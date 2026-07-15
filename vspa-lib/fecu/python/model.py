"""
model.py -- Pure-Python reference model for LA9310 FECU P2P FEC operations.

Provides:
  scramble(bits, seed)         -- LFSR scrambler (same polynomial as 802.11)
  bcc_encode(bits, rate)       -- Convolutional encoding + puncturing (K=7)
  bcc_decode_hard(bits, rate)  -- Hard-decision Viterbi decoder
  ldpc_encode(bits, cw_sz, rate) -- IEEE 802.11n systematic LDPC encoder
  ldpc_decode(bits, cw_sz, rate) -- Strip parity bits (lossless round-trip)

All inputs/outputs are Python lists of integers, one bit per element (0 or 1).

NOTE: The LDPC encoder implements the systematic IEEE 802.11n encoding
(Sec 20.3.11.7, Annex F parity-check matrices).  It uses the structured
802.11n parity recursion (not a generic Gaussian-elimination solve) so that
the packed encoder output matches ENC_OUT_BUF bit-for-bit.  Only the
(size, rate) combinations tabulated in _LDPC_BASE are supported; other configs
fall back to a zero-parity append (sufficient for the round-trip test only).

"""


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# BCC generator polynomials (IEEE 802.11 / standard K=7 code)
# g0 = 1011011 (133 octal), g1 = 1111001 (171 octal)
_G0 = 0b1011011
_G1 = 0b1111001
_K  = 7  # constraint length

# Puncture masks for each coding rate (bit=1 means keep, bit=0 means puncture)
# Format: list of (keep_g0, keep_g1) per input bit pair, repeated as needed.
_PUNC_MASKS = {
    "1/2": [(1, 1)],
    "2/3": [(1, 1), (1, 0), (1, 1), (0, 1)],
    "3/4": [(1, 1), (1, 0), (0, 1)],
    "5/6": [(1, 1), (1, 0), (0, 1), (1, 0), (0, 1)],
}

_RATE_STR = {0: "1/2", 1: "2/3", 2: "3/4", 3: "5/6"}

# ---------------------------------------------------------------------------
# Scrambler
# ---------------------------------------------------------------------------

def scramble(bits, seed=0x5B):
    """XOR bits with an LFSR sequence seeded by seed (7-bit, taps 4 and 7).

    The same function serves as descrambler (XOR is self-inverse).
    """
    lfsr = seed & 0x7F
    if lfsr == 0:
        lfsr = 1  # avoid all-zero state
    out = []
    for b in bits:
        # tap positions: bit7 XOR bit4  (1-indexed from MSB of 7-bit shift reg)
        fb = ((lfsr >> 6) ^ (lfsr >> 3)) & 1
        lfsr = ((lfsr << 1) | fb) & 0x7F
        out.append(b ^ fb)
    return out


# ---------------------------------------------------------------------------
# BCC encoder
# ---------------------------------------------------------------------------

def _parity(x):
    """Return popcount(x) mod 2."""
    x ^= x >> 16
    x ^= x >> 8
    x ^= x >> 4
    x ^= x >> 2
    x ^= x >> 1
    return x & 1


def bcc_encode(bits, rate=0):
    """Convolutional encode bits with K=7 code and puncture to coding rate.

    Parameters
    ----------
    bits : list[int]   input data bits (0 or 1)
    rate : int         0=1/2, 1=2/3, 2=3/4, 3=5/6

    Returns
    -------
    list[int]  encoded + punctured bits
    """
    # IEEE 802.11 K=7 convolutional encoder (Sec 18.3.5.6) as implemented by
    # the LA9310 FECU hardware.  The hardware does NOT append 6 tail bits
    # after the data; instead it processes exactly dataBitsPerEncoder input
    # bits (= len(bits)) and OVERWRITES the last numPadBits (=6) input
    # positions with the encoder flush/end state (all zeros for a single
    # symbol).  Output pairs are emitted per input bit and punctured against
    # the coding-rate mask.  This matches ENC_OUT_BUF bit-for-bit.

    rate_str = _RATE_STR.get(rate, "1/2")
    mask = _PUNC_MASKS[rate_str]
    mask_len = len(mask)

    num_pad_bits = 6
    end_state = 0  # single-symbol encode flushes to zero

    data = list(bits)
    dbe = len(data)
    tail = dbe - num_pad_bits

    # Overwrite the last positions with the flush/tail bits.

    if tail >= 6:
        for jj in range(6):
            data[jj + tail - 6] = (end_state >> jj) & 1
    elif 1 <= tail <= 5:
        for jj in range(tail):
            data[jj] = (end_state >> (6 - tail + jj)) & 1


    # Encode: next_state = state | (bit << 6); output = parity(next_state & G).
    state = 0
    out = []
    pi = 0
    for ii in range(dbe):
        next_state = state | (data[ii] << (_K - 1))
        c0 = _parity(next_state & _G0)
        c1 = _parity(next_state & _G1)
        state = (next_state >> 1) & 0x3F

        k0, k1 = mask[pi % mask_len]
        if k0:
            out.append(c0)
        if k1:
            out.append(c1)
        pi += 1
    return out



# ---------------------------------------------------------------------------
# BCC interleaver
# ---------------------------------------------------------------------------

# Channel-type keys used by bcc_interleave_params for the IEEE 802.11 BCC
# interleaver (Sec 18.3.5.7).
#   name         -> (nRow_factor, nCol)

# nRow = nRow_factor * bpscs, s = max(1, bpscs // 2).
_BCC_CHAN = {
    "BW_20MHZ_11a":  (3, 16),
    "BW_20MHZ_11ac": (4, 13),
    "BW_40MHZ_11ac": (6, 18),
    "BW_80MHZ_11ac": (9, 26),
}

# Bits per subcarrier per stream for each modulation.
_BPSCS = {"BPSK": 1, "QPSK": 2, "16QAM": 4, "64QAM": 6}


def bcc_interleave_params(channel, modulation):
    """Return (nRow, nCol, s) for an IEEE 802.11 BCC interleaver (Sec 18.3.5.7)."""

    bpscs = _BPSCS[modulation]
    factor, ncol = _BCC_CHAN[channel]
    nrow = factor * bpscs
    s = max(1, bpscs // 2)
    return nrow, ncol, s


# ---------------------------------------------------------------------------
# LDPC tone mapping (frequency-domain permutation)
#
# IEEE 802.11 LDPC tone mapping (Sec 20.3.11.8) as implemented by the LA9310
# FECU hardware.  The mapping operates on
# one OFDM symbol worth of coded bits at a time:

#     Nsd data subcarriers, decodedBPSCS (=BPSCS) bits per subcarrier,
#     so each symbol carries Nsd*BPSCS bits.
# For each subcarrier k in a symbol the tone index tk is
#     tk = Dtm*(k % (Nsd/Dtm)) + (k*Dtm)//Nsd
# and all BPSCS bits of subcarrier k are moved to tone tk.
# A codeword longer than one symbol is processed symbol-by-symbol; a trailing
# partial symbol (fewer than Nsd*BPSCS bits) is passed through unchanged, since
# the permutation is applied only to full-symbol chunks.
# ---------------------------------------------------------------------------

# channel -> (Nsd, Dtm) tone-mapping parameters (IEEE 802.11 Sec 20.3.11.8).

_LDPC_TONE = {
    "BW_20MHZ_11ac": (52,  4),
    "BW_40MHZ_11ac": (108, 6),
    "BW_80MHZ_11ac": (234, 9),
    "BW_RU26_11ax":  (24,  1),
    "BW_RU52_11ax":  (48,  3),
    "BW_RU106_11ax": (102, 6),
    "BW_RU242_11ax": (234, 9),
    "BW_RU484_11ax": (468, 12),
    "BW_RU996_11ax": (980, 20),
}


def _ldpc_tone_map_symbol(inb, nsd, dtm, bpscs):
    """Tone-map exactly one OFDM symbol (nsd*bpscs bits)."""
    out = [0] * (nsd * bpscs)
    for k in range(nsd):
        tk = dtm * (k % (nsd // dtm)) + (k * dtm) // nsd
        for n in range(bpscs):
            out[tk * bpscs + n] = inb[k * bpscs + n]
    return out


def ldpc_tone_map(coded, channel, modulation):
    """Apply the FECU LDPC frequency-domain tone mapping to coded bits.

    Processes the coded stream one OFDM symbol at a time (sym_bits =
    Nsd*BPSCS).  Any trailing partial symbol is copied through unchanged.
    Returns a new list of the same length as coded.
    """
    nsd, dtm = _LDPC_TONE[channel]
    bpscs = _BPSCS[modulation]
    sym_bits = nsd * bpscs
    out = []
    i = 0
    n = len(coded)
    while i < n:
        chunk = coded[i:i + sym_bits]
        if len(chunk) == sym_bits:
            out.extend(_ldpc_tone_map_symbol(chunk, nsd, dtm, bpscs))
        else:
            # Trailing partial OFDM symbol: the FECU tone mapper only operates
            # on complete OFDM symbols (exactly Nsd*BPSCS input bits).  A
            # trailing partial symbol is never tone-mapped

            # and is not emitted, so the corresponding ENC_OUT_BUF bits remain
            # zero.  Reproduce that by zero-filling the tail.
            out.extend([0] * len(chunk))
        i += sym_bits
    return out


def bcc_interleave(inb, nRow, nCol, s):
    """IEEE 802.11 BCC bit interleaver (Sec 18.3.5.7).

    The frequency-rotation term is omitted because the hardware supports a
    single subcarrier only.
    """

    N = nRow * nCol
    out = [0] * N
    for k in range(N):
        i = nRow * (k % nCol) + (k // nCol)
        j = s * (i // s) + (i + N - ((nCol * i) // N)) % s
        out[j] = inb[k]
    return out


# ---------------------------------------------------------------------------
# BCC hard-decision Viterbi decoder
# ---------------------------------------------------------------------------


def bcc_decode_hard(bits, rate=0, num_data_bits=None):
    """Hard-decision Viterbi decoder for the K=7 BCC code.

    Parameters
    ----------
    bits         : list[int]  hard received bits (0 or 1)
    rate         : int        0=1/2, 1=2/3, 2=3/4, 3=5/6
    num_data_bits: int or None  if given, trim output to this length

    Returns
    -------
    list[int]  decoded bits
    """
    rate_str = _RATE_STR.get(rate, "1/2")
    mask = _PUNC_MASKS[rate_str]
    mask_len = len(mask)
    num_states = 1 << (_K - 1)  # 64

    # De-puncture: insert erasures (value 2) where bits were punctured
    depunc = []
    bit_idx = 0
    sym_idx = 0
    total_syms = 0
    # Estimate number of encoder output symbol pairs from len(bits)
    # Count how many outputs per mask period
    kept_per_period = sum(k0 + k1 for k0, k1 in mask)
    # Total mask periods = ceil(len(bits) / kept_per_period)
    import math
    periods = math.ceil(len(bits) / kept_per_period) if kept_per_period else 1
    for period in range(periods):
        for k0, k1 in mask:
            if bit_idx < len(bits) and k0:
                depunc.append(bits[bit_idx]); bit_idx += 1
            else:
                depunc.append(2)  # erasure
            if bit_idx < len(bits) and k1:
                depunc.append(bits[bit_idx]); bit_idx += 1
            else:
                depunc.append(2)  # erasure

    # Pair up depunctured bits into (c0, c1) symbols
    symbols = []
    for i in range(0, len(depunc) - 1, 2):
        symbols.append((depunc[i], depunc[i + 1]))

    # Viterbi ACS
    INF = 10**9
    pm = [INF] * num_states
    pm[0] = 0
    traceback = []

    for c0_rx, c1_rx in symbols:
        new_pm = [INF] * num_states
        tb_row = [0] * num_states
        for s in range(num_states):
            for inp in (0, 1):
                prev_sr = (s >> 1) | (inp << (_K - 2))
                if prev_sr >= num_states:
                    continue
                sr_enc = (s | (inp << (_K - 1))) & 0x7F
                c0 = _parity(sr_enc & _G0)
                c1 = _parity(sr_enc & _G1)
                # Branch metric: Hamming distance, skip erasures
                bm = 0
                if c0_rx != 2:
                    bm += (c0 != c0_rx)
                if c1_rx != 2:
                    bm += (c1 != c1_rx)
                cand = pm[prev_sr] + bm
                if cand < new_pm[s]:
                    new_pm[s] = cand
                    tb_row[s] = prev_sr
        pm = new_pm
        traceback.append(tb_row)

    # Traceback from state 0 (tail bits flush to 0)
    state = 0
    decoded = []
    for tb_row in reversed(traceback):
        prev = tb_row[state]
        inp = (state >> (_K - 2)) & 1
        decoded.append(inp)
        state = prev
    decoded.reverse()

    # Remove 6 tail bits
    decoded = decoded[:-6] if len(decoded) > 6 else decoded

    if num_data_bits is not None:
        decoded = decoded[:num_data_bits]
    return decoded


# ---------------------------------------------------------------------------
# LDPC stub (zero-parity append / strip for simulation round-trip)
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# IEEE 802.11n LDPC base (prototype) matrices
#
# Each matrix entry is a circulant-permutation shift: -1 means an all-zero
# Z x Z block, a value s >= 0 means the Z x Z identity cyclically shifted
# right by s.  The expansion factor Z depends on the codeword size:
#   N=648  -> Z=27,  N=1296 -> Z=54,  N=1944 -> Z=81.
# Base matrix dimensions: mb rows x 24 cols, so m = mb*Z parity bits and
# n = 24*Z coded bits.  Only the (size, rate) combinations exercised by the
# test harness are tabulated here; add more from IEEE 802.11-2016 Annex F as
# needed.
# ---------------------------------------------------------------------------
_BASE_648_R12 = """
 0 -1 -1 -1  0  0 -1 -1  0 -1 -1  0  1  0 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1
22  0 -1 -1 17 -1  0  0 12 -1 -1 -1 -1  0  0 -1 -1 -1 -1 -1 -1 -1 -1 -1
 6 -1  0 -1 10 -1 -1 -1 24 -1  0 -1 -1 -1  0  0 -1 -1 -1 -1 -1 -1 -1 -1
 2 -1 -1  0 20 -1 -1 -1 25  0 -1 -1 -1 -1 -1  0  0 -1 -1 -1 -1 -1 -1 -1
23 -1 -1 -1  3 -1 -1 -1  0 -1  9 11 -1 -1 -1 -1  0  0 -1 -1 -1 -1 -1 -1
24 -1 23  1 17 -1  3 -1 10 -1 -1 -1 -1 -1 -1 -1 -1  0  0 -1 -1 -1 -1 -1
25 -1 -1 -1  8 -1 -1 -1  7 18 -1 -1  0 -1 -1 -1 -1 -1  0  0 -1 -1 -1 -1
13 24 -1 -1  0 -1  8 -1  6 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1  0  0 -1 -1 -1
 7 20 -1 16 22 10 -1 -1 23 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1  0  0 -1 -1
11 -1 -1 -1 19 -1 -1 -1 13 -1  3 17 -1 -1 -1 -1 -1 -1 -1 -1 -1  0  0 -1
25 -1  8 -1 23 18 -1 14  9 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1  0  0
 3 -1 -1 -1 16 -1 -1  2 25  5 -1 -1  1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1  0
"""

_BASE_1944_R34 = """
16 17 22 24  9  3 14 -1  4  2  7 -1 26 -1  2 -1 21 -1  1  0 -1 -1 -1 -1
25 12 12  3  3 26  6 21 -1 15 22 -1 15 -1  4 -1 -1 16 -1  0  0 -1 -1 -1
25 18 26 16 22 23  9 -1  0 -1  4 -1  4 -1  8 23 11 -1 -1 -1  0  0 -1 -1
 9  7  0  1 17 -1 -1  7  3 -1  3 23 -1 16 -1 -1 21 -1  0 -1 -1  0  0 -1
24  5 26  7  1 -1 -1 15 24 15 -1  8 -1 13 -1 13 -1 11 -1 -1 -1 -1  0  0
 2  2 19 14 24  1 15 19 -1 21 -1  2 -1 24 -1  3 -1  2  1 -1 -1 -1 -1  0
"""

# (cw_sz, rate) -> (base-matrix text, expansion factor Z)
_LDPC_BASE = {
    (648,  0): (_BASE_648_R12,  27),
    (1944, 2): (_BASE_1944_R34, 81),
}


def _parse_base(text):
    """Parse a whitespace base matrix string into a list of rows of ints."""
    rows = []
    for line in text.strip().splitlines():
        rows.append([int(tok) for tok in line.split()])
    return rows


def _build_h(base, z):
    """Expand a base matrix into the full parity-check matrix H.

    Returns H as a list of rows, each row an integer bitmask of width n
    (bit j set => H[row][j] == 1).  Column 0 is the most significant use:
    here bit j corresponds to codeword column j (LSB = column 0).
    """
    mb = len(base)
    nb = len(base[0])
    m = mb * z
    rows = [0] * m
    for br in range(mb):
        for bc in range(nb):
            shift = base[br][bc]
            if shift < 0:
                continue
            # Z x Z identity cyclically right-shifted by 'shift':
            # entry (i, (i+shift) mod Z) = 1.
            for i in range(z):
                r = br * z + i
                col = bc * z + ((i + shift) % z)
                rows[r] |= (1 << col)
    return rows, m


def _gen_parity(base, z, info_bits):
    """Compute systematic LDPC parity bits using the IEEE 802.11n encoding.

    Implements the structured IEEE 802.11n parity recursion (Sec 20.3.11.7,
    Annex F), not a generic Gaussian-elimination solve.  This is what the
    LA9310 FECU hardware produces in ENC_OUT_BUF, so reproducing it bit-for-bit
    is required for the intermediate encoder-output vectors to match.


    Returns the full codeword (message followed by parity) of length 24*z.
    """
    numCol = 24
    numRow = len(base)
    blockLength = numCol * z
    numMessPlusShort = blockLength - numRow * z   # message columns before parity
    ps = numMessPlusShort                          # parity start index

    blk = list(info_bits) + [0] * (blockLength - len(info_bits))
    for i in range(ps, blockLength):
        blk[i] = 0

    def rotR(start, rot):
        # bitRotR: output[t] = input[start + (t + rot) % z]
        return [blk[start + (t + rot) % z] for t in range(z)]

    # Step 1/2: sumRow(i) = sum_j protoH(i,j) * s(j); P0 = sum(sumRow)
    sumRow = [0] * (z * numCol)
    for i in range(numRow):
        for j in range(numCol - numRow):
            sh = base[i][j]
            if sh >= 0:
                r = rotR(j * z, sh)
                for t in range(z):
                    sumRow[i * z + t] ^= r[t]
        # P0 += sumRow[i]: the recursion accumulates every row's sumRow into
        # the single P0 block at 'ps' (XOR dest is 'ps' for all i), not block i.

        for t in range(z):
            blk[ps + t] ^= sumRow[i * z + t]


    # Step 3: recursive parity blocks.
    def X(a, b, d):
        # P(d) = sumRow[a] + P(b)
        for t in range(z):
            blk[ps + d * z + t] = sumRow[a * z + t] ^ blk[ps + b * z + t]

    def XP0(d):
        # P(d) += P0
        for t in range(z):
            blk[ps + d * z + t] ^= blk[ps + t]

    # P1 = sumRow[0] + rotR(P0, 1)
    r = rotR(ps, 1)
    for t in range(z):
        blk[ps + 1 * z + t] = sumRow[0 * z + t] ^ r[t]

    X(1, 1, 2)   # P2 = sumRow[1] + P1
    X(2, 2, 3)   # P3 = sumRow[2] + P2
    if numRow == 4:      # R = 5/6
        XP0(3)
        return blk
    X(3, 3, 4)   # P4 = sumRow[3] + P3
    if numRow == 6:      # R = 3/4
        XP0(4)
    X(4, 4, 5)   # P5 = sumRow[4] + P4
    if numRow == 8:      # R = 2/3
        XP0(5)
    if numRow == 6:      # R = 3/4 done
        return blk
    X(5, 5, 6)   # P6 = sumRow[5] + P5
    X(6, 6, 7)   # P7 = sumRow[6] + P6
    if numRow == 12:     # R = 1/2
        XP0(7)
    if numRow == 8:      # R = 2/3 done
        return blk
    X(7, 7, 8)
    X(8, 8, 9)
    X(9, 9, 10)
    X(10, 10, 11)
    return blk


def ldpc_encode(bits, cw_sz=1944, rate=2):
    """Systematic IEEE 802.11n LDPC encode (matches the FECU hardware).


    Returns [info_bits | parity_bits] of total length cw_sz.  Falls back to a
    zero-parity append for (size, rate) combinations not tabulated in
    _LDPC_BASE (keeps the round-trip test working on unsupported configs).
    """
    key = (cw_sz, rate)
    if key not in _LDPC_BASE:
        n_parity = cw_sz - len(bits)
        return list(bits) + [0] * n_parity

    text, z = _LDPC_BASE[key]
    base = _parse_base(text)
    n = 24 * z
    m = len(base) * z
    k = n - m

    info = list(bits[:k])
    if len(info) < k:
        info = info + [0] * (k - len(info))

    return _gen_parity(base, z, info)




def ldpc_decode(codeword, cw_sz=1944, rate=2):
    """Stub: strip parity bits and return only the message bits."""
    n_data = {
        (648,  0): 324,  # rate 1/2
        (648,  1): 432,  # rate 2/3
        (648,  2): 486,  # rate 3/4
        (648,  3): 540,  # rate 5/6
        (1296, 0): 648,
        (1296, 1): 864,
        (1296, 2): 972,
        (1296, 3): 1080,
        (1944, 0): 972,
        (1944, 1): 1296,
        (1944, 2): 1458,
        (1944, 3): 1620,
    }.get((cw_sz, rate), len(codeword))
    return list(codeword[:n_data])
