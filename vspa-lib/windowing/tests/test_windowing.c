// SPDX-License-Identifier: BSD-3-Clause
// txWindowing_w16_vecaligned end-to-end test harness.
//
// The asm kernel performs W-OFDM overlap-add at a symbol edge with W=16:
//   - Reads the CP head  : inp_out[0..15]   (W complex)
//   - Reads the symbol tail: inp_out[CPlen..CPlen+15]   (W complex)
//   - Reads old history  : history[0..15]              (W complex)
//   - Writes new history : history[0..15] = (1 - win) * tail
//   - Writes faded head  : inp_out[0..15] = win * head + old_hist
//
// ABI (txWindowing.sx, _txWindowing_w16_vecaligned):
//   a0: inp_out_p, vec aligned (cfixed16_t*)
//   a1: history_p, vec aligned (cfixed16_t*)
//   a2: real_window_p          (float16_t*)
//   g0: CPlen, in complex samples
//
// All three buffers MUST be vec-aligned because the asm uses .laddr / vector
// loads.  Without _VSPA_VECTOR_ALIGN the simulator crashes silently.
//
// Buffer layout on VSPA2 (sizeof(uint32_t) = 2 halfwords = 1 complex):
//   - input[N=32]  : 32 complex SM16 samples (Im<<16 | Re per uint32)
//   - hist[N=32]   : zeros on entry; first W=16 entries are written by asm
//   - win[N=32]    : 16 float16 values packed in first 8 uint32 entries
//                    (low halfword = f16[2k], high halfword = f16[2k+1])
//   - ref[N=32]    : expected inp_out after the kernel call

#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>

#include "test_utils.h"

#define N      32
#define CPLEN  16

// Match cwproj wiring: vec-aligned, BSS-resident, zero by default.
_VSPA_VECTOR_ALIGN static uint32_t input[N];
_VSPA_VECTOR_ALIGN static uint32_t hist [N];
_VSPA_VECTOR_ALIGN static uint32_t win  [N];

static const uint32_t INPUT_DATA[N] = {
#include "vectors/input.hex"
};
static const uint32_t HIST_DATA[N] = {
#include "vectors/hist_in.hex"
};
static const uint32_t WIN_DATA[N] = {
#include "vectors/win.hex"
};
static const uint32_t REF_DATA[N] = {
#include "vectors/ref.hex"
};

extern void txWindowing_w16_vecaligned(void *inp_out_p,
                                       void *history_p,
                                       void *real_window_p,
                                       uint32_t CPlen);

int main(void)
{
    int i;

    for (i = 0; i < N; i++) {
        input[i] = INPUT_DATA[i];
        hist[i]  = HIST_DATA[i];
        win[i]   = WIN_DATA[i];
    }

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    txWindowing_w16_vecaligned((void *)input, (void *)hist, (void *)win, CPLEN);
    KCYC_STOP_PRINT();

    return vspa_array_cmp((const unsigned *)input,
                          (const unsigned *)REF_DATA,
                          N);
}
