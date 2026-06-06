// SPDX-License-Identifier: BSD-3-Clause
// decimator end-to-end test harness — buffer layout mirrors NXP's cwproj
// driver (submodules/.../decimator/decimator_cwproj/src/main.c) so the
// asm sees identical inputs and a clean (BSS-zeroed) wrap-around region.
//
// Why this layout matters: `decimator_2x_32hf` reads filter history by
// going backward (DECIM_FLT_LEN-1) complex samples from inp_p, with
// circular wrap-around within [inp_circ_p, inp_circ_p + inp_circ_size).
// In cwproj, DECIM_INP_BUFF is double-buffered (NUM_INP_BUFFERS=2) so
// for the first block (inp_block_idx=0, inp_p = &BUFF[0]) the wrap goes
// into the second half of the buffer — which is BSS-zeroed and yields
// the all-zeros history that the MATLAB model assumes.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

#define DECIM_FACT          2
#define SAMP_PER_LINE       32        // 32 complex outputs per asm call
#define NUM_INP_BUFFERS     2         // cwproj double-buffering
#define MAX_OUT_LINES       1
#define MAX_FACT            8         // cwproj's DECIM_TEST_MAX_FACT

#define OUT_LEN             SAMP_PER_LINE
#define INP_LEN             (OUT_LEN * DECIM_FACT)            // 64 complex
#define INP_BUFF_COMPLEX    (MAX_OUT_LINES * SAMP_PER_LINE * NUM_INP_BUFFERS * MAX_FACT)
                                                              // 512 complex
#define OUT_BUFF_COMPLEX    (MAX_OUT_LINES * SAMP_PER_LINE)   // 32 complex

// inp_circ_size: cwproj passes
//   out_num_lines * SAMP_PER_LINE * NUM_INP_BUFFERS * decim_fact
//                                            * sizeof(cfixed16_t)
// In VSPA2, sizeof(cfixed16_t) is 2 halfwords (MAU=halfword), so for our
// settings this is 1 * 32 * 2 * 2 * 2 = 128 halfwords.
#define INP_CIRC_SIZE_HW    (MAX_OUT_LINES * SAMP_PER_LINE * NUM_INP_BUFFERS \
                             * DECIM_FACT * 2)

// Initialiser source: gen_vectors.py writes INP_LEN complex (= 2*INP_LEN
// halfwords) of input. We splice that into the start of DECIM_INP_BUFF
// at runtime; the rest of the buffer stays BSS-zeroed.
static const int16_t INPUT_DATA[2 * INP_LEN] = {
#include "vectors/input.hex"
};

static const int16_t REF_DATA[2 * OUT_LEN] = {
#include "vectors/ref.hex"
};

// Match cwproj: aligned, BSS-resident, zero-initialised by default.
_VSPA_VECTOR_ALIGN static int16_t DECIM_INP_BUFF[2 * INP_BUFF_COMPLEX];
_VSPA_VECTOR_ALIGN static int16_t DECIM_OUT_BUFF[2 * OUT_BUFF_COMPLEX];

// Asm entry point. cfixed16_t* in the prototype but our int16_t* arrays
// have the same memory layout — only the bit pattern matters.
extern void decimator_2x_32hf(void *inp_p, void *out_p,
                              void *inp_circ_p, unsigned inp_circ_size);

int main(void)
{
    int i;

    // Stage input into the first slot of the double-buffered region.
    // This mirrors cwproj's `mem_copy` for `test_block_idx = 0`.
    for (i = 0; i < 2 * INP_LEN; i++)
        DECIM_INP_BUFF[i] = INPUT_DATA[i];

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    decimator_2x_32hf(
        (void *)DECIM_INP_BUFF,             // inp_p = &BUFF[0]
        (void *)DECIM_OUT_BUFF,             // out_p
        (void *)DECIM_INP_BUFF,             // inp_circ_p = base
        (unsigned)INP_CIRC_SIZE_HW);        // 128 halfwords
    KCYC_STOP_PRINT();

    return vspa_array_cmp((const unsigned *)DECIM_OUT_BUFF,
                          (const unsigned *)REF_DATA,
                          OUT_LEN);
}
