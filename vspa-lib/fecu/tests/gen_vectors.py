#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate test vectors for the fecu_p2p kernel test harness.

Produces vectors/input_tcN.hex and vectors/ref_tcN.hex for TC0..TC3.

The .hex format is one 32-bit word per line as a C initializer token
(0xDEADBEEF,) so the files can be #include-d directly inside a C array
initializer -- same convention used by crc8 and other vspa-lib kernels.

The reference output for a noiseless encode+decode round-trip is the
original input data, so ref_tcN.hex == input_tcN.hex for all TCs.

Run:
    python3 gen_vectors.py
"""

import os
import sys
import random

# Make the sibling python/ reference model importable
_HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(_HERE, '..', 'python'))
import model  # noqa: E402

OUT_DIR = os.path.join(_HERE, 'vectors')
os.makedirs(OUT_DIR, exist_ok=True)

# Buffer size must match DMEM_BUF_WORDS in test_fecu.c
WORDS_PER_BUF = 128



def bits_to_words(bits, num_words):
    """Pack a list of bits into num_words 32-bit integers (LSB first, zero-padded).

    The FECU DMEM interface reads and writes bits LSB-first: bit 0 of the data
    stream occupies bit 0 of word 0, bit 1 occupies bit 1 of word 0, etc.
    Input/reference vectors must use the same LSB-first convention so that an
    encode->decode round-trip recovers the original data unchanged.
    """
    words = []
    for w in range(num_words):
        val = 0
        for b in range(32):
            idx = w * 32 + b
            if idx < len(bits):
                val |= bits[idx] << b
        words.append(val)
    return words


def write_hex(path, words):
    """Write 32-bit words as C initializer tokens (0xHHHHHHHH,), one per line."""
    with open(path, 'w') as f:
        for w in words:
            f.write('0x{:08X},\n'.format(w & 0xFFFFFFFF))


# Coding-rate name -> model rate index (matches fecuRateT in fecu.h)
_RATE = {'1/2': 0, '2/3': 1, '3/4': 2, '5/6': 3}


def encode_pipeline(data_bits, use_ldpc, rate, cw_sz=None,
                    bypass_scrambler=True, sc_lfsr_init=0x5B,
                    bypass_interleaver=True, channel=None, modulation=None):
    """Run the reference encode pipeline and return the packed coded bits.

    Mirrors the FECU encode datapath used by fecu_p2p_encode():
      (optional) scramble -> BCC-encode+puncture  OR  LDPC-encode
      -> (optional) interleave / tone-map.
    Returns a flat list of coded bits (0/1), LSB-first output ordering.
    """
    bits = list(data_bits)
    if not bypass_scrambler:
        bits = model.scramble(bits, sc_lfsr_init)
    if use_ldpc:
        coded = model.ldpc_encode(bits, cw_sz=cw_sz, rate=rate)
    else:
        coded = model.bcc_encode(bits, rate=rate)

    # Apply the frequency-domain interleave/tone-map stage when not bypassed,
    # dispatching exactly like cFecuInterleaver::interleave():
    #   LDPC path -> ldpcToneMapping (per-OFDM-symbol tone permutation)
    #   BCC  path -> bccInterleave   (nRow x nCol matrix interleave)
    # This reproduces the 80 MHz / 64-QAM datapath used by TC1 (BCC) and TC2
    # (LDPC) so the packed encoder output matches ENC_OUT_BUF bit-for-bit.
    if not bypass_interleaver and channel and modulation:
        if use_ldpc:
            coded = model.ldpc_tone_map(coded, channel, modulation)
        else:
            nrow, ncol, s = model.bcc_interleave_params(channel, modulation)
            if nrow * ncol == len(coded):
                coded = model.bcc_interleave(coded, nrow, ncol, s)
    return coded


def write_tc(tc_id, data_bits, use_ldpc, rate, cw_sz=None,
             bypass_scrambler=True, sc_lfsr_init=0x5B,
             bypass_interleaver=True, channel=None, modulation=None):

    """Write input, reference (round-trip) and encoder-output hex files.

    - input_tcN.hex : raw data bits (LSB-first packed)
    - ref_tcN.hex   : recovered data after a noiseless round-trip (== input)
    - enc_tcN.hex   : expected packed encoder output (ENC_OUT_BUF contents)
    """
    in_words = bits_to_words(data_bits, WORDS_PER_BUF)
    write_hex(os.path.join(OUT_DIR, 'input_tc{}.hex'.format(tc_id)), in_words)
    write_hex(os.path.join(OUT_DIR, 'ref_tc{}.hex'.format(tc_id)),   in_words)

    coded = encode_pipeline(data_bits, use_ldpc, rate, cw_sz,
                            bypass_scrambler, sc_lfsr_init,
                            bypass_interleaver, channel, modulation)

    enc_words = bits_to_words(coded, WORDS_PER_BUF)
    write_hex(os.path.join(OUT_DIR, 'enc_tc{}.hex'.format(tc_id)), enc_words)

    print('TC{}: {} data bits -> {} coded bits'.format(
        tc_id, len(data_bits), len(coded)))


def write_tc5():
    """TC5: multi-block LDPC 648 rate-1/2 encode of all-ones data.

    Three systematic codewords (648 bits each) laid out at an output stride of
    672 bits (84 bytes) per block, matching the FECU multi-block placement:
      block 0 -> output bit    0..647
      block 1 -> output bit  672..1319
      block 2 -> output bit 1344..1991
    A single enc_tc5.hex describes the whole ENC_OUT_BUF (128 words).
    """
    info = [1] * 324
    cw = model.ldpc_encode(info, cw_sz=648, rate=0)  # 648-bit codeword
    out_bits = [0] * (WORDS_PER_BUF * 32)
    for blk in range(3):
        base = blk * 672
        for i, b in enumerate(cw):
            out_bits[base + i] = b
    enc_words = bits_to_words(out_bits, WORDS_PER_BUF)
    write_hex(os.path.join(OUT_DIR, 'enc_tc5.hex'), enc_words)
    # Report the systematic info bit 323 of each block for quick inspection.
    b = [out_bits[blk * 672 + 323] for blk in range(3)]
    print('TC5: 3 x 648-bit LDPC blocks, info bit 323 = '
          '{} {} {} (all should be 1)'.format(b[0], b[1], b[2]))


random.seed(0x1234ABCD)

# TC0: BCC 1/2, BPSK, 20 MHz, 24 data bits, scrambler + interleaver active
write_tc(0, [random.randint(0, 1) for _ in range(24)],
         use_ldpc=False, rate=_RATE['1/2'],
         bypass_scrambler=False, sc_lfsr_init=0x5B,
         bypass_interleaver=False, channel='BW_20MHZ_11a', modulation='BPSK')


# TC1: BCC 3/4, 64-QAM, 80 MHz, 216 data bits, scrambler active
write_tc(1, [random.randint(0, 1) for _ in range(216)],
         use_ldpc=False, rate=_RATE['3/4'],
         bypass_scrambler=False, sc_lfsr_init=0x5B,
         bypass_interleaver=False, channel='BW_80MHZ_11ac', modulation='64QAM')

# TC2: LDPC 1944-bit CW, rate 3/4, 64-QAM, 80 MHz, 1458 data bits, scrambler active
write_tc(2, [random.randint(0, 1) for _ in range(1458)],
         use_ldpc=True, rate=_RATE['3/4'], cw_sz=1944,
         bypass_scrambler=False, sc_lfsr_init=0x5B,
         bypass_interleaver=False, channel='BW_80MHZ_11ac', modulation='64QAM')

# TC3: BCC 1/2, BPSK, 20 MHz, scrambler + interleaver bypassed, 24 data bits
write_tc(3, [random.randint(0, 1) for _ in range(24)],
         use_ldpc=False, rate=_RATE['1/2'],
         bypass_scrambler=True)

# TC4: LDPC 648-bit CW, rate 1/2, 324 data bits, scrambler + interleaver bypassed.
# Matches ldpc648_LA9310_FECU reference (codelength=648, coderate=1/2, max_iter=20).
write_tc(4, [random.randint(0, 1) for _ in range(324)],
         use_ldpc=True, rate=_RATE['1/2'], cw_sz=648,
         bypass_scrambler=True)

# TC5: multi-block LDPC 648 rate-1/2 encode reproducer (all-ones)
write_tc5()

print('All vectors written to', OUT_DIR)

