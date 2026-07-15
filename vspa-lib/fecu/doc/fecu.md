# fecu - Forward Error Correction Unit (FECU) Kernel

## Overview

The `fecu` kernel provides a protocol-agnostic driver for the LA9310 **Forward Error
Correction Unit (FECU)** hardware IP block, together with a high-level P2P API and a
simulation/test harness.

The FECU runs at 614.4 MHz and offloads the two main FEC pipelines from the VSPA core:

| Path | Engine | Coding rates |
|------|--------|--------------|
| Transmit | BCC K=7 convolutional encoder + scrambler + interleaver | 1/2, 2/3, 3/4, 5/6 |
| Receive  | Viterbi decoder (BCC) or LDPC decoder + de-interleaver + de-scrambler | 1/2, 2/3, 3/4, 5/6 |

LDPC block sizes of 648, 1296, and 1944 bits are supported.

The driver is intentionally **not tied to 802.11**. Although the register model was
originally designed for WiFi PHY, every field is also usable in a generic point-to-point
link; the `fecu_p2p` layer exposes only the parameters that matter for P2P operation.

---

## Repository Layout

```
vspa-lib/fecu/
  include/
    ipreg_fecu.h      -- FECU IP register map (byte/word addresses, bitfield structs, masks)
    fecu.h            -- Low-level inline accessor API and type definitions
    fecu_p2p.h        -- High-level P2P encode/decode API
  src/
    fecu.c            -- fecuInit() implementation
    fecu_p2p.c        -- P2P encode/decode implementation
  tests/
    test_fecu.c       -- VSPA test harness (TC0-TC4 + TC5 multi-block reproducer)
    Makefile          -- Build + simulation targets
    gen_vectors.py    -- Generate input/reference hex vectors
    run_sim.py        -- Invoke simulator and check pass/fail
  fecu_cwproj/
    src/
      main.c          -- Standalone CW debug harness with cycle measurement
  python/
    model.py          -- Pure-Python BCC/LDPC reference model
  doc/
    fecu.md           -- This file
```

---

## API Reference

### Types (`fecu.h`)

```c
typedef enum { FECU_BW_20MHZ_802a = 0, ..., FECU_BW_RU996_802ax = 0xD } fecuBwT;
typedef enum { FECU_MT_BPSK = 0, FECU_MT_QPSK, FECU_MT_16QAM,
               FECU_MT_64QAM, FECU_MT_256QAM, FECU_MT_1024QAM } fecuModT;
typedef enum { FECU_LDPC_CW_648 = 0, FECU_LDPC_CW_1296, FECU_LDPC_CW_1944 } fecuLdpcCwSzT;
typedef enum { FEC_RATE_1_2 = 0, FEC_RATE_2_3, FEC_RATE_3_4, FEC_RATE_5_6 } fecuRateT;
```

### Configuration structure (`fecu_p2p.h`)

```c
typedef struct {
    bool           use_ldpc;         // false = BCC, true = LDPC
    fecuBwT        bw;               // Channel bandwidth token
    fecuModT       mod;              // Modulation order
    fecuRateT      rate;             // Coding rate
    uint16_t       data_bits;        // Payload size in bits (before encoding)
    uint16_t       coded_bits;       // Coded size in bits (after encoding / puncturing)
    uint16_t       num_pad_bits;     // Tail/padding bits (carried separately, not in coded_bits)
    uint32_t       bcc_punc_mask;    // BCC puncture pattern (ignored when use_ldpc=true)
    uint8_t        bcc_punc_len;     // Length of BCC puncture mask in bits
    fecuLdpcCwSzT  ldpc_cw_sz;      // LDPC codeword size (ignored when use_ldpc=false)
    uint8_t        ldpc_max_iter;    // Maximum LDPC decoder iterations
    uint16_t       ldpc_num_short;   // Number of shortened LDPC codewords
    uint16_t       ldpc_num_rep;     // Number of repeated LDPC codewords
    uint8_t        sc_lfsr_init;     // Scrambler LFSR seed (7-bit value)
    bool           bypass_scrambler;    // Set true to disable scrambler/de-scrambler
    bool           bypass_interleaver;  // Set true to disable interleaver/de-interleaver
} fecuP2pCfgT;
```

### BCC puncture helper

```c
void fecu_p2p_bcc_punc_from_rate(fecuRateT rate,
                                  uint32_t *mask_out,
                                  uint8_t  *len_out);
```

Populates `bcc_punc_mask` and `bcc_punc_len` from the rate enum. Built-in patterns:

| Rate | Mask   | Length |
|------|--------|--------|
| 1/2  | 0x03   | 2      |
| 2/3  | 0x07   | 3      |
| 3/4  | 0x27   | 6      |
| 5/6  | 0x267  | 10     |

### Public functions

```c
// Initialize FECU hardware (clear pending status + error flags).
void fecu_p2p_init(void);

// Encode src DMEM buffer -> dst DMEM buffer.
// src/dst are half-word (16-bit) DMEM addresses; on VSPA a data pointer is
// already such an address, so pass the buffer address directly (no shifting).
void fecu_p2p_encode(const fecuP2pCfgT *cfg, uint32_t src, uint32_t dst);

// Decode src DMEM buffer -> dst DMEM buffer; returns true on success.
// (false = LDPC decoder reported uncorrectable error)
bool fecu_p2p_decode(const fecuP2pCfgT *cfg, uint32_t src, uint32_t dst);

// Spin until FECU finishes the current operation.
void fecu_p2p_wait_done(void);
```

---

## Usage Example

```c
#include "fecu_p2p.h"

/* Aligned DMEM buffers.  On VSPA, a data pointer is already a half-word
 * (16-bit) DMEM address, which is exactly what the FECU DMA expects -- no
 * shifting or conversion is required. */
static uint32_t tx_buf[64] _VSPA_VECTOR_ALIGN;  /* 64 words = 2048 bits input  */
static uint32_t enc_buf[64] _VSPA_VECTOR_ALIGN; /* encoded output              */
static uint32_t dec_buf[64] _VSPA_VECTOR_ALIGN; /* decoded output              */

void p2p_link_example(void)
{
    fecuP2pCfgT cfg = {0};
    cfg.use_ldpc         = false;
    cfg.bw               = FECU_BW_20MHZ_802a;
    cfg.mod              = FECU_MT_BPSK;
    cfg.rate             = FEC_RATE_1_2;
    cfg.data_bits        = 24;
    cfg.coded_bits       = 48;   /* 24 * 2 for rate-1/2 */
    cfg.num_pad_bits     = 6;    /* tail bits, carried separately (not in coded_bits) */
    cfg.sc_lfsr_init     = 0x5B;
    fecu_p2p_bcc_punc_from_rate(FEC_RATE_1_2, &cfg.bcc_punc_mask, &cfg.bcc_punc_len);

    fecu_p2p_init();

    /* Transmit side: fill tx_buf, then encode.  Pass buffer addresses
     * directly -- they are already half-word DMEM addresses. */
    fecu_p2p_encode(&cfg, (uint32_t)tx_buf, (uint32_t)enc_buf);
    fecu_p2p_wait_done();

    /* Receive side: decode enc_buf (possibly after channel) */
    bool ok = fecu_p2p_decode(&cfg, (uint32_t)enc_buf, (uint32_t)dec_buf);
    /* ok == true  -> dec_buf matches tx_buf */
}
```

---

## Test Cases

The test harness (`tests/test_fecu.c`) runs five test cases (TC0-TC4) automatically on
the VSPA simulator. Each TC encodes a known input and then verifies the result. The
primary, deterministic check is the packed **encoder output** (`ENC_OUT_BUF`) compared
bit-for-bit against the pure-Python reference model (`python/model.py` via
`gen_vectors.py`). Where the datapath is losslessly recoverable, the harness also runs
a full encode -> decode round-trip and compares the recovered data against the original.

| TC | FEC  | Rate | Modulation | BW     | data_bits | Notes                                  |
|----|------|------|------------|--------|-----------|----------------------------------------|
|  0 | BCC  | 1/2  | BPSK       | 20 MHz | 24        | Baseline BCC; encoder-output check     |
|  1 | BCC  | 3/4  | 64-QAM     | 80 MHz | 216       | Higher rate + wider BW; BCC frequency-domain scatter not modeled, encoder check skipped (encode-completed sanity only) |
|  2 | LDPC | 3/4  | 64-QAM     | 80 MHz | 1458      | 1944-bit LDPC + tone mapping; encoder-output check (trailing partial OFDM symbol dropped, so no lossless round-trip) |
|  3 | BCC  | 1/2  | BPSK       | 20 MHz | 24        | Scrambler + interleaver bypassed; encoder-output check |
|  4 | LDPC | 1/2  | BPSK       | 20 MHz | 324       | 648-bit LDPC, interleaver/tone-mapper bypassed; full encode -> decode round-trip |

Notes on verification scope:

- Only the LDPC path with the tone mapper bypassed (TC4) is a fully lossless decode
  round-trip; that case checks both the encoder output and the recovered data.
- BCC paths (TC0, TC1, TC3) and the LDPC+tone-mapping path (TC2) are validated by the
  bit-exact encoder-output comparison; the decode round-trip is skipped for those.

A separate multi-block LDPC encode reproducer, `run_tc5_multiblock_ones()` ("TC5"),
exercises the manual FIRST / ONE_BEFORE_FINAL / FINAL control sequence and compares the
three-codeword output against `enc_tc5.hex`. It is compiled out by default (guarded by
`#if 0`); enable it to reproduce/regression-test the multi-block encoder path.

The harness prints `PASS` / `FAIL` and returns the failure count from `main()`.

---

## Build Instructions

```bash
cd vspa-lib/fecu/tests

# 1. Generate stimulus and reference vectors (requires Python 3)
make generate

# 2. Build VSPA binary and run simulator
make test

# 3. Clean build artefacts and vectors
make clean
```

The Makefile requires `VSPA_TOOL` to point to the VSPA toolchain installation, e.g.:

```bash
export VSPA_TOOL=/opt/vspa_sdk
make test
```

For the standalone CW debug harness (cycle-accurate measurements):

```bash
cd vspa-lib/fecu/fecu_cwproj
# build with your CW Makefile / IDE project
```

---

## Register Map Summary

All registers are accessed through the VSPA `__ip_read` / `__ip_write` intrinsics using
the word-address constants defined in `include/ipreg_fecu.h`.

Each register constant is defined as `byte_offset >> 2` (see `ipreg_fecu.h`).

| Register              | Byte offset | Word address | Purpose                                   |
|-----------------------|:-----------:|:------------:|-------------------------------------------|
| FECU_CONFIG           | 0x300       | 0xC0         | FEC type, BW, modulation, rate, direction |
| FECU_SIZES            | 0x304       | 0xC1         | data_bits, coded_bits                     |
| FECU_NUM_PAD          | 0x308       | 0xC2         | Number of tail/padding bits               |
| FECU_BCC_PUNC_MASK    | 0x30C       | 0xC3         | BCC puncture bit mask                      |
| FECU_BCC_CONFIG       | 0x310       | 0xC4         | BCC puncture length, enc start/end state  |
| FECU_LDPC_CONFIG      | 0x314       | 0xC5         | LDPC codeword size, rate, iterations, repeat |
| FECU_LDPC_SIZES       | 0x318       | 0xC6         | LDPC shortening / repetition bits         |
| FECU_BYPASS           | 0x324       | 0xC9         | Bypass flags (int/vd/ce/sc/le/ld/di)      |
| FECU_SC_CONFIG        | 0x328       | 0xCA         | Scrambler LFSR seed, 802.11b mode         |
| FECU_DMEM_READ_COUNT  | 0x32C       | 0xCB         | Number of items to read from DMEM         |
| FECU_DMEM_SRC_ADDR    | 0x330       | 0xCC         | DMEM source half-word address (+keep bits)|
| FECU_DMEM_DST_ADDR    | 0x334       | 0xCD         | DMEM destination half-word address (+rep) |
| FECU_CTRL             | 0x358       | 0xD6         | Trigger (start type, first/last symbol)   |
| FECU_STATUS           | 0x364       | 0xD9         | Busy, pending, cmd-fifo, start-error      |
| FECU_LDPC_DEC_STATUS  | 0x378       | 0xDE         | LDPC decode error flag + good/bad CW count|

---

## Python Reference Model

`python/model.py` provides a pure-Python BCC/LDPC reference model used by
`tests/gen_vectors.py` to produce the reference hex files:

- **`scramble(bits, seed)`** - 7-bit LFSR, taps bit-7 XOR bit-4, self-inverse.
- **`bcc_encode(bits, rate)`** - K=7 convolutional encoder (G0=0b1011011, G1=0b1111001),
  the IEEE 802.11 BCC encoder (Sec 18.3.5.6) as implemented by the LA9310 FECU hardware.
  The hardware does not append 6 tail bits; instead it overwrites the last
  `num_pad_bits` (=6) input positions with the encoder flush/end state before encoding,
  then punctures per the rate table.
- **`bcc_decode_hard(bits, rate, num_data_bits)`** - Hard-decision Viterbi with
  64-state trellis; de-punctures with erasure value 2 before ACS.
- **`bcc_interleave(...)` / `bcc_interleave_params(...)`** - IEEE 802.11 BCC bit
  interleaver (Sec 18.3.5.7), single-subcarrier simplification (the frequency-rotation
  term is omitted because the hardware supports a single subcarrier only).
- **`ldpc_tone_map(...)`** - IEEE 802.11 LDPC frequency-domain tone mapping
  (Sec 20.3.11.8), applied per full OFDM symbol.
- **`ldpc_encode(bits, cw_sz, rate)`** - Structured IEEE 802.11n systematic encoder
  (Sec 20.3.11.7, Annex F parity-check matrices) as implemented by the LA9310 FECU
  hardware, tabulated for (648, rate 1/2) and (1944, rate 3/4); other (size, rate)
  combinations fall back to a zero-parity append (sufficient for the round-trip test only).
- **`ldpc_decode(codeword, cw_sz, rate)`** - Parity-strip helper that returns the
  systematic message bits; real soft-decision LDPC decoding is done in hardware.

---

## Notes and Limitations

1. **DMEM addressing** - The FECU DMA engine uses *half-word* (16-bit) addresses. On
   VSPA a data pointer is natively a half-word address, so pass the buffer address
   directly to `src`/`dst`; no shift or `>> 1` conversion is required. Bits [31:24] of
   the source/destination address registers carry the keep-read / repeat-write bit
   counts used by the multi-block path.

2. **Buffer alignment** - Input and output DMEM buffers must be aligned to a VSPA
   vector boundary. Declare them with `_VSPA_VECTOR_ALIGN`.

3. **Single-block trigger** - `fecu_p2p_encode` and `fecu_p2p_decode` both use the
   `FECU_CTRL_SINGLE_BLOCK` convenience macro which asserts first-symbol, last-symbol,
   and immediate-start simultaneously. This is appropriate for single-packet P2P
   operation. Multi-symbol streaming requires a different control sequence, driving the
   FIRST / ONE_BEFORE_FINAL / FINAL symbol flags manually as demonstrated by the TC5
   multi-block LDPC reproducer in `test_fecu.c`.

4. **LDPC model** - The Python model implements a real, bit-exact LDPC encoder:
   `ldpc_encode()` is the structured IEEE 802.11n systematic encoder (Sec 20.3.11.7,
   Annex F parity-check matrices) as implemented by the LA9310 FECU hardware, tabulated
   for (648, rate 1/2) and (1944, rate 3/4) with a zero-parity fallback for untabulated
   (size, rate) combinations. `ldpc_decode()` is a parity-strip helper (returns the
   systematic message bits) used for the round-trip check; actual soft-decision LDPC
   decoding is performed by the hardware.

5. **Coding rate for BCC** - The `coded_bits` field must be set by the caller to match
   the punctured output length. The tail/flush bits are carried via `num_pad_bits` and
   are NOT counted in `coded_bits` (they do not appear in the `cbps` field). For a
   rate `num/denom` code:
   `coded_bits = data_bits * denom / num`
   (e.g. 24 -> 48 at rate 1/2; 216 -> 288 at rate 3/4). The
   `fecu_p2p_bcc_punc_from_rate()` helper provides the puncture mask/length but not the
   coded-bits calculation.
