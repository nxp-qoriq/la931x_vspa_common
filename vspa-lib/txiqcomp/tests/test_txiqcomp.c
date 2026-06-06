// SPDX-License-Identifier: BSD-3-Clause
// txiqcomp end-to-end test harness — buffer layout mirrors NXP's cwproj
// driver (submodules/.../txiqcomp/txiqcomp_cwproj/Sources/main.c) so the
// asm sees identical inputs and a clean (BSS-zeroed) wrap-around region.
//
// Reference data sourced directly from cwproj golden test vectors
// (generic_input_x_batch1.hex / generic_output_y_batch1_ref.hex,
//  generic_input_txiqcompstruct.hex). This bypasses any Python oracle
// uncertainty: we replay the MATLAB-generated stimulus and compare to
// the MATLAB-generated bit-exact reference.
//
// Function 1 path: txiqcomp(in, out, &cfg, n_linepairs).
//   - n_linepairs = N / 64;  cwproj uses N=512 → n_linepairs=8
//   - in/out are complex half-fixed (1 word = re/im halfwords)
//   - cfg is a structTXIQCompParams = { vspa_complex_float32 dcOffset;
//                                       float IQImb_ftaps[4]; }
//   - cwproj's "txiqcompstruct" hex is already laid out as 6 floats:
//       [-di, -dq, 0, ci2q, ci, cq]
//     i.e. dcOffset.re, dcOffset.im, then taps[0..3].
//
// All buffers are vector-aligned and reside in .bss (zero-initialised).
// We pre-load the input and config into the buffers at runtime to mirror
// cwproj's sim_dram_load (which writes the same regions before `go`).
//
// We use the same VSPA2 pipeline-flush idiom (assign-back of an unused
// scalar) that cwproj uses, plus an unconditional printf so the
// simulator stdout pipe definitely shows our PASS/FAIL line.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

#define N_SAMPLES        512                 // complex samples per batch (cwproj)
// Per-iteration output of the asm loop is one VRA of cfixed16:
//   VR = __AU_COUNT__ * 4 halfwords = AU*2 cfixed16 = AU*2 complex.
// At AU=16 this is 32 complex per iteration. The header doc claims
// "n_linepairs = N/64" but cwproj's main computes n_lines = batch_size>>5
// (= N/32) and passes that as the loop count — empirically that's what
// matches: 1 line of 32 complex per iteration. Stick with cwproj's value.
#define N_LINEPAIRS      (N_SAMPLES / 32)    // 16 lines for N=512 at AU=16

// Input/output buffers: cwproj declares input_buffer[2048], output_buffer[2048]
// of vspa_complex_fixed16 (1 cfixed16 = 2 halfwords = 1 uint32 worth of bits).
// Our N_SAMPLES batch only fills the first half of the slot; the rest stays
// BSS-zeroed.  Use uint32_t for clean hex-file initialisation: each line of
// cwproj's hex file is one packed re/im halfword pair.
#define BUFF_WORDS       (2 * N_SAMPLES)     // double-buffer headroom (1024 words)

// Static const initialisers populated from cwproj golden hex files.
static const uint32_t INPUT_DATA[N_SAMPLES] = {
#include "vectors/input.hex"
};

static const uint32_t REF_DATA[N_SAMPLES] = {
#include "vectors/ref.hex"
};

// 6 floats from the cwproj struct hex, packed as raw u32 bit patterns.
// We splice these into the start of the cfg struct at runtime; the struct
// itself sits in BSS so any padding bytes are zero.
static const uint32_t CFG_DATA[6] = {
#include "vectors/cfg.hex"
};

// Match cwproj: aligned, BSS-resident, zero-initialised by default.
// The asm reads/writes via int16/halfword indexing; using uint32_t storage
// just makes the .hex include cleaner (1 line per complex half-fixed sample).
_VSPA_VECTOR_ALIGN static uint32_t TXIQ_INP_BUFF[BUFF_WORDS];
_VSPA_VECTOR_ALIGN static uint32_t TXIQ_OUT_BUFF[BUFF_WORDS];

// structTXIQCompParams = { vspa_complex_float32 dcOffset; float IQImb_ftaps[4]; }
// 6 floats total; we reserve 8 floats (32B) for vector alignment safety.
_VSPA_VECTOR_ALIGN static uint32_t TXIQ_CFG[8];

// Asm entry point. Prototype matches txiqcomp.h; signature uses void* here
// to avoid the cfixed16_t header dependency in this minimal harness.
extern void txiqcomp(const void *pIn, void *pOut, void *pConfig,
                     unsigned int n_linepairs);

int main(void)
{
    int i;

    // Stage input + config into BSS-zeroed buffers — mirrors cwproj's
    // sim_dram_load calls before the simulator's `go`.
    for (i = 0; i < N_SAMPLES; i++)
        TXIQ_INP_BUFF[i] = INPUT_DATA[i];
    for (i = 0; i < 6; i++)
        TXIQ_CFG[i] = CFG_DATA[i];

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    txiqcomp((const void *)TXIQ_INP_BUFF,
             (void *)TXIQ_OUT_BUFF,
             (void *)TXIQ_CFG,
             (unsigned)N_LINEPAIRS);
    KCYC_STOP_PRINT();

    // cwproj's main.c flushes the VSPA2 pipeline by reading back a config
    // word; do the equivalent here so the final stores are visible before
    // we read TXIQ_OUT_BUFF.
    volatile uint32_t flush = TXIQ_CFG[0];
    (void)flush;

    return vspa_array_cmp((const unsigned *)TXIQ_OUT_BUFF,
                          (const unsigned *)REF_DATA,
                          N_SAMPLES);
}
