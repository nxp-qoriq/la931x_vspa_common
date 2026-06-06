// SPDX-License-Identifier: BSD-3-Clause
// qam modulator end-to-end test harness — Python-oracle driven.
//
// The constellation is selected at compile time via -DQAM_MODE=<token>;
// the matching gen_vectors.py invocation produces input.hex (random
// bits) and ref.hex (cfloat16 oracle reference).  Tokens (and the asm
// entry point each maps to):
//
//      QAM_MODE         N_LINES   asm entry        input_words  out_symbols
//      ---------------- --------- ---------------- ------------ -----------
//      QAM_MODE_BPSK         32   qamModBpsk           32           1024
//      QAM_MODE_QPSK         16   qamModQpsk           32            512
//      QAM_MODE_16QAM         8   qamMod16             32            256
//      QAM_MODE_64QAM        32   qamMod64            192           1024
//      QAM_MODE_256QAM        4   qamMod256            32            128
//      QAM_MODE_1024QAM      32   qamMod1024          320           1024
//
// (input_words = N_LINES * 32 * M / 32 = N_LINES * M)
//
// Each output cfloat16 packs as (imag_fp16<<16) | real_fp16 in one
// uint32; we compare uint32-wise against ref.hex via vspa_array_cmp.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

// asm entry points (signatures from
// submodules/.../vspa-lib/qam/include/qam.h).
extern void qamModBpsk(unsigned int *bitIn, void *qamOut, unsigned int N);
extern void qamModQpsk(unsigned int *bitIn, void *qamOut, unsigned int N);
extern void qamMod16  (unsigned int *bitIn, void *qamOut, unsigned int N);
extern void qamMod64  (unsigned int *bitIn, void *qamOut, unsigned int N);
extern void qamMod256 (unsigned int *bitIn, void *qamOut, unsigned int N);
extern void qamMod1024(unsigned int *bitIn, void *qamOut, unsigned int N);

// Mode tokens — mirror QAM_MODE values produced by the Makefile -D.
#define QAM_MODE_BPSK     1
#define QAM_MODE_QPSK     2
#define QAM_MODE_16QAM    4
#define QAM_MODE_64QAM    6
#define QAM_MODE_256QAM   8
#define QAM_MODE_1024QAM 10

#ifndef QAM_MODE
#define QAM_MODE QAM_MODE_BPSK
#endif

// Per-mode geometry: must match gen_vectors.py N_LINES_PER_MODE.
#if QAM_MODE == QAM_MODE_BPSK
#  define N_LINES         32
#  define N_INPUT_WORDS   32
#  define N_OUT_SYMBOLS   1024
#  define QAM_MOD_FN      qamModBpsk
#elif QAM_MODE == QAM_MODE_QPSK
#  define N_LINES         16
#  define N_INPUT_WORDS   32
#  define N_OUT_SYMBOLS    512
#  define QAM_MOD_FN      qamModQpsk
#elif QAM_MODE == QAM_MODE_16QAM
#  define N_LINES          8
#  define N_INPUT_WORDS   32
#  define N_OUT_SYMBOLS   256
#  define QAM_MOD_FN      qamMod16
#elif QAM_MODE == QAM_MODE_64QAM
#  define N_LINES         32
#  define N_INPUT_WORDS  192
#  define N_OUT_SYMBOLS  1024
#  define QAM_MOD_FN      qamMod64
#elif QAM_MODE == QAM_MODE_256QAM
#  define N_LINES          4
#  define N_INPUT_WORDS   32
#  define N_OUT_SYMBOLS   128
#  define QAM_MOD_FN      qamMod256
#elif QAM_MODE == QAM_MODE_1024QAM
#  define N_LINES         32
#  define N_INPUT_WORDS  320
#  define N_OUT_SYMBOLS 1024
#  define QAM_MOD_FN      qamMod1024
#else
#  error "Unknown QAM_MODE"
#endif

#define OUT_BUF_WORDS  1056   // matches cwproj's qamOut[1056] envelope

// Compile-time-baked vectors from gen_vectors.py.
static const unsigned int INPUT_DATA[N_INPUT_WORDS] = {
#include "vectors/input.hex"
};

static const unsigned int REF_DATA[N_OUT_SYMBOLS] = {
#include "vectors/ref.hex"
};

// cwproj-aligned BSS buffers — vector-aligned, zero on entry to main().
_VSPA_VECTOR_ALIGN static unsigned int bitIn[320];
_VSPA_VECTOR_ALIGN static unsigned int qamOut[OUT_BUF_WORDS];

int main(void)
{
    int i;

    for (i = 0; i < N_INPUT_WORDS; i++)
        bitIn[i] = INPUT_DATA[i];

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    QAM_MOD_FN(bitIn, (void *)qamOut, (unsigned int)N_LINES);
    KCYC_STOP_PRINT();

    return vspa_array_cmp(qamOut, REF_DATA, N_OUT_SYMBOLS);
}
