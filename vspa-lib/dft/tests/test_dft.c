// SPDX-License-Identifier: BSD-3-Clause
// dft (mini_dft_hfx_hfl_asm) end-to-end test harness.
// Wiring mirrors NXP's cwproj driver:
//   submodules/la931x_vspa_common/vspa-lib/dft/dft_cwproj/src/main.c
//
// cwproj allocates three vector-aligned buffers in BSS, copies test data
// into INP_BUFF at runtime, and calls mini_dft_hfx_hfl_asm(INP, OUT, N).
// We replicate that minimum here.  No scratch buffer is needed for the
// `mini_*` entry points (only the full `dft_*` variants take scratch).

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "test_utils.h"

#define N 96
#define MAX_LEN 512   // matches cwproj `#define MAX_LEN 512`

extern void mini_dft_hfx_hfl_asm(void *inp, void *out, int n);

// Hex-included input/ref. Each line is `(im_u16 << 16) | re_u16` = one
// complex sample per word. Input encoded as SM16 (half_fixed), ref as
// truncated IEEE f16 (half), per gen_vectors.py.
// `unsigned` matches `vspa_array_cmp` from test_utils.h.
static const unsigned INPUT_DATA[N] = {
#include "vectors/input.hex"
};

static const unsigned REF_DATA[N] = {
#include "vectors/ref.hex"
};

// Match cwproj's BSS-resident, vector-aligned buffers.
_VSPA_VECTOR_ALIGN static unsigned INP_BUFF[2 * MAX_LEN];
_VSPA_VECTOR_ALIGN static unsigned OUT_BUFF[2 * MAX_LEN];

int main(void)
{
    int i;

    // Stage input into the start of INP_BUFF; tail stays BSS-zero.
    for (i = 0; i < N; i++)
        INP_BUFF[i] = INPUT_DATA[i];

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    mini_dft_hfx_hfl_asm((void *)INP_BUFF, (void *)OUT_BUFF, N);
    KCYC_STOP_PRINT();

    return vspa_array_cmp(OUT_BUFF, REF_DATA, N);
}
