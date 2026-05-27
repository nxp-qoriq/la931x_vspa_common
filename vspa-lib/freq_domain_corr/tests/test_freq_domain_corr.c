// SPDX-License-Identifier: BSD-3-Clause
// freq_domain_corr / vec_mult_64chp end-to-end test harness.
//
// Wiring mirrors NXP's cwproj driver
//   submodules/.../freq_domain_corr/freq_domain_corr_cwproj/src/main.c:
//     - both buffers are vector-aligned and live in BSS (zero-init), so
//       any read beyond the populated region returns zero (which is the
//       correction `corr*0 = 0` outcome the Python oracle assumes);
//     - the input vector is staged into the start of the BSS-resident
//       INP_OUT buffer at runtime via a small loop (cwproj's `mem_copy`).
//
// The cwproj driver calls `freq_domain_corr_64sbc(INP_OUT, &GAIN, SCRATCH,
// phase_ramp, phase_init, num_streams)` which, internally, builds the
// scratch correction via `phase_ramp_gen` (NCO * gain, half-truncated)
// and then forwards to `vec_mult_64chp(scratch, INP_OUT, num_streams)`.
// We pre-compute the scratch in the Python oracle (gen_vectors.py) so
// this harness exercises only the asm `vec_mult_64chp`, not the
// `phase_ramp_gen` C+IPPU dependency chain.
//
// TC000 parameters (cwproj/vector/in/TC000):
//   num_sbc      = 64
//   num_streams  = 1
//   phase_ramp   = 59
//   phase_init   = -122

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

#define NUM_SBC_PER_LINE    32
#define NUM_LINES_64SBC     2                                   // 64 sbc / 32 per line
#define N                   (NUM_SBC_PER_LINE * NUM_LINES_64SBC) // 64 cfloat16
#define NUM_STREAMS         1

// cwproj sizes — match exactly so the asm sees the same wrap/stride layout.
#define TEST_MAX_NUM_LINES  16
#define TEST_MAX_NUM_NSS    4
#define INP_BUFF_COMPLEX    (NUM_SBC_PER_LINE * TEST_MAX_NUM_LINES * TEST_MAX_NUM_NSS)
                                // = 2048 cfloat16  (matches cwproj's INP_OUT)
#define CORR_BUFF_COMPLEX   (NUM_SBC_PER_LINE * TEST_MAX_NUM_LINES)
                                // = 512  cfloat16  (matches cwproj's SCRATCH)

// Each cfloat16 packs (im<<16)|re into a single uint32 — that's the layout
// gen_vectors.py emits via complex_f16_to_packed_u32().
static const unsigned INPUT_DATA[N] = {
#include "vectors/input.hex"
};

static const unsigned CORR_DATA[N] = {
#include "vectors/corr.hex"
};

static const unsigned REF_DATA[N] = {
#include "vectors/ref.hex"
};

// BSS-resident, vector-aligned, zero-initialised by default. cwproj puts
// SCRATCH in `.ibss` to be sure; in this build the LCF places `static`
// uninitialised globals in `.bss` which the runtime clears identically.
_VSPA_VECTOR_ALIGN static unsigned FREQ_DOMAIN_CORR_INP_OUT[INP_BUFF_COMPLEX];
_VSPA_VECTOR_ALIGN static unsigned FREQ_DOMAIN_CORR_SCRATCH[CORR_BUFF_COMPLEX];

// Asm entry point — declared as void* so the int16 vs cfloat16 type
// mismatch doesn't fight the compiler; only the bit pattern matters.
extern void vec_mult_64chp(void *corr_p, void *vec_p, unsigned num_vec);

int main(void)
{
    int i;

    // Stage corr and input into the start of the BSS-zeroed buffers.
    // Mirrors cwproj's mem_copy at TC start; the rest stays zero.
    for (i = 0; i < N; i++) {
        FREQ_DOMAIN_CORR_SCRATCH[i] = CORR_DATA[i];
        FREQ_DOMAIN_CORR_INP_OUT[i] = INPUT_DATA[i];
    }

    vec_mult_64chp((void *)FREQ_DOMAIN_CORR_SCRATCH,
                   (void *)FREQ_DOMAIN_CORR_INP_OUT,
                   (unsigned)NUM_STREAMS);

    return vspa_array_cmp((const unsigned *)FREQ_DOMAIN_CORR_INP_OUT,
                          (const unsigned *)REF_DATA,
                          N);
}
