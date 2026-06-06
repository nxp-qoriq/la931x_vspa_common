// SPDX-License-Identifier: BSD-3-Clause
// qec_opt_asm smoke test harness.
//
// Why this layout: there is no `qec_cwproj/src/main.c` upstream — only the
// kernel sources at submodules/la931x_vspa_common/vspa-lib/qec/{inc,src}/.
// We therefore mirror the closest available pattern (the fd_qec_cwproj
// layout): aligned BSS-resident input/output buffers, a parameter struct
// pre-populated by gen_vectors.py, and a single bulk call that processes
// `num_samples` complex points.
//
// The active asm is qec_opt.sx with -DVSPA2 (selecting the post-#else
// "2nd round optimization" body — see lines 124..165 of qec_opt.sx).
// Inside that body:
//     g0 -> num_samples; sr g0,g0,7  => num_samples must be a multiple
//                                       of 128 (=> 256 halfwords = 8 lines
//                                       on 16-AU, which the inner loop
//                                       walks exactly once per outer iter)
//     a2 -> qec_params_opt_t*  (one VRA line, vector-aligned)
//     a1 -> input  cfixed16_t* (vector-aligned)
//     a0 -> output cfixed16_t* (vector-aligned, may alias input)
//
// Smoke configuration: zero input + zero parameters -> zero output.
// This exercises buffer wiring, alignment, num_samples scaling, and the
// link of the real asm without needing a Python oracle for the QEC
// imbalance-correction equation (which has no MATLAB sibling in nxp/).

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

// We deliberately avoid #include "vcpu.h" / "qec_opt.h": vcpu.h declares
// `extern void main(void) __noreturn;` which collides with our `int main`.
// The asm only cares about memory layout, so we treat all pointers as
// raw void/int16_t — same convention used in tests/decimator/test_decimator.c.
//
// Param block size: qec_params_opt_t in qec_opt.h is sized to one VRA line
// (32 halfwords on 16-AU = 64 bytes).  We allocate a uint32_t scratch
// large enough for any AU configuration up to 32 (= 64 halfwords =
// 16 uint32 doublewords).
#define QEC_PARAM_DWORDS 32

// Smoke test: 128 complex samples = exactly one outer-loop iteration of
// _qec_opt_asm (sr g0, g0, 7 => g0/128).
#define N_SAMPLES 128

// Initialiser source: gen_vectors.py emits 2*N_SAMPLES halfwords (re,im,
// SM16-truncated) and the matching reference (also 2*N_SAMPLES halfwords).
static const int16_t INPUT_DATA[2 * N_SAMPLES] = {
#include "vectors/input.hex"
};

static const int16_t REF_DATA[2 * N_SAMPLES] = {
#include "vectors/ref.hex"
};

// Aligned BSS-resident IO buffers (mirror fd_qec_cwproj/src/main.c rule 1).
_VSPA_VECTOR_ALIGN static int16_t QEC_INP_BUFF[2 * N_SAMPLES];
_VSPA_VECTOR_ALIGN static int16_t QEC_OUT_BUFF[2 * N_SAMPLES];

// Parameter block: must be vector-aligned (the asm does ld.laddr [a2]
// which is a full VRA line load).  We over-allocate so the same harness
// covers AU=8..32 without recompiling against qec_opt.h's exact layout.
_VSPA_VECTOR_ALIGN static uint32_t QEC_PARAMS[QEC_PARAM_DWORDS];

extern void qec_opt_asm(void *output, void *input,
                        unsigned int num_samples,
                        void *qec_para);

int main(void)
{
    int i;

    // Stage input from .hex into the aligned buffer (BSS-zero on entry).
    for (i = 0; i < 2 * N_SAMPLES; i++)
        QEC_INP_BUFF[i] = INPUT_DATA[i];

    // QEC_PARAMS is BSS-zero already; gen_vectors guarantees the matching
    // reference was computed under the all-zero parameter config.

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    qec_opt_asm((void *)QEC_OUT_BUFF,
                (void *)QEC_INP_BUFF,
                (unsigned int)N_SAMPLES,
                (void *)QEC_PARAMS);
    KCYC_STOP_PRINT();

    return vspa_array_cmp((const unsigned *)QEC_OUT_BUFF,
                          (const unsigned *)REF_DATA,
                          N_SAMPLES);  // N_SAMPLES uint32 = 2*N_SAMPLES int16
}
