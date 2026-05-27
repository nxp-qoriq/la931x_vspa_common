// SPDX-License-Identifier: BSD-3-Clause
// fd_qec end-to-end test harness — buffer wiring mirrors NXP's cwproj
// driver (submodules/.../fd_qec/fd_qec_cwproj/src/main.c) so the asm
// sees identical inputs to MATLAB.
//
// Kernel signature (from fd_qec.h):
//   void fd_qec(cfixed16_t *out_p,
//               cfixed16_t *inp_p0,   // x        (input)
//               cfixed16_t *inp_p1,   // x_mirror
//               cfixed16_t *inp_p2,   // weights a
//                                     // weights b are at inp_p2 + size
//               uint32_t   size);     // bufLen in complex samples
//
// cwproj layout (single contiguous buffer FD_QEC_INP_BUFF[]):
//   [ 0 .. size )           : x          → inp_p0 = base
//   [ size .. 2*size )      : x_mirror   → inp_p1 = base +   size
//   [ 2*size .. 3*size )    : weights_a  → inp_p2 = base + 2*size
//   [ 3*size .. 4*size )    : weights_b  → asm computes inp_p2 + size
// (units above are complex samples)
//
// Asm constraints:
//   sr g1, g0, 5;  sub g1, g1, 2;   ⇒  size must be a multiple of 32
//                                       AND size >= 64.
// We pick size = 64 (= 2 loop iterations), the smallest legal size.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

#define BUF_LEN          128                      // complex samples per buffer
#define NUM_INP_BUFFERS  4                        // x, x_mirror, a, b
#define INP_TOTAL_CPLX   (BUF_LEN * NUM_INP_BUFFERS)  // 256 complex
#define OUT_LEN          BUF_LEN                  // 64 complex outputs

// Initialiser source: gen_vectors.py writes 2*INP_TOTAL_CPLX = 512 halfwords
// of input (interleaved SM16: re,im,re,im,...) into the start of FD_QEC_INP_BUFF
// at runtime; rest of buffer stays BSS-zeroed (unused by the asm here).
static const int16_t INPUT_DATA[2 * INP_TOTAL_CPLX] = {
#include "vectors/input.hex"
};

static const int16_t REF_DATA[2 * OUT_LEN] = {
#include "vectors/ref.hex"
};
// Honesty check toggle: when -DHONESTY_CHECK_CORRUPT is passed, corrupt
// REF_DATA[0] post-init to flip what the comparator sees → must FAIL.
#ifdef HONESTY_CHECK_CORRUPT
static int16_t REF_DATA_MUT[2 * OUT_LEN];
#endif

// cwproj-style: aligned, BSS-resident, zero-initialised by default.
_VSPA_VECTOR_ALIGN static int16_t FD_QEC_INP_BUFF[2 * INP_TOTAL_CPLX];
_VSPA_VECTOR_ALIGN static int16_t FD_QEC_OUT_BUFF[2 * OUT_LEN];

// Asm entry point. cfixed16_t* in the prototype but our int16_t* arrays
// have the same memory layout — only the bit pattern matters.
extern void fd_qec(void *out_p,
                   void *inp_p0, void *inp_p1, void *inp_p2,
                   unsigned size);

int main(void)
{
    int i;

    // Stage all four input slabs at the start of FD_QEC_INP_BUFF.
    for (i = 0; i < 2 * INP_TOTAL_CPLX; i++)
        FD_QEC_INP_BUFF[i] = INPUT_DATA[i];

    fd_qec(
        (void *)FD_QEC_OUT_BUFF,
        (void *)&FD_QEC_INP_BUFF[0],                // x
        (void *)&FD_QEC_INP_BUFF[2 * BUF_LEN],      // x_mirror   (1*BUF_LEN cplx = 2*BUF_LEN halfwords)
        (void *)&FD_QEC_INP_BUFF[4 * BUF_LEN],      // weights_a  (asm computes b = a + size)
        (unsigned)BUF_LEN);

#ifdef HONESTY_CHECK_CORRUPT
    for (i = 0; i < 2 * OUT_LEN; i++) REF_DATA_MUT[i] = REF_DATA[i];
    REF_DATA_MUT[0] ^= (int16_t)0xBEEF;     // deliberate corruption
    return vspa_array_cmp((const unsigned *)FD_QEC_OUT_BUFF,
                          (const unsigned *)REF_DATA_MUT,
                          OUT_LEN);
#else
    return vspa_array_cmp((const unsigned *)FD_QEC_OUT_BUFF,
                          (const unsigned *)REF_DATA,
                          OUT_LEN);
#endif
}
