// SPDX-License-Identifier: BSD-3-Clause
// ATAN2 kernel bit-accurate test harness.
//
// Embeds NXP golden-reference test vectors prepared by prepare_golden.py
// via #include.  Returns 0 on PASS, number of word mismatches on FAIL.
//
// Build: see tests/atan/Makefile
// Run:   wine runsim.exe -d vspa2_16au test_atan_TC001.elf  → exit code 0 = PASS

// Use <vspa/intrinsics.h> instead of vcpu.h to avoid the conflicting
// "extern void main(void)" declaration that vcpu.h contains.
#include <vspa/intrinsics.h>
#include "test_utils.h"

// tc_params.h is located at vectors/$(TC)/ via -I in the Makefile.
// Defines: TC_INPUT_LEN, TC_NUM_COEFF, TC_INP_PREC, TC_OUT_PREC,
//          TC_N_INP_WORDS, TC_N_REF_WORDS
#include "tc_params.h"

// ---------------------------------------------------------------------------
// VSPA types (subset from vcpu.h — reproduced here to avoid pulling in vcpu.h)
// ---------------------------------------------------------------------------
typedef __fx16 fixed16_t;
typedef __fp16 float16_t;
typedef float  float32_t;

typedef struct { fixed16_t real; fixed16_t imag; } cfixed16_t;
typedef struct { float16_t real; float16_t imag; } cfloat16_t;
typedef struct { float32_t real; float32_t imag; } cfloat32_t;

// ---------------------------------------------------------------------------
// Forward-declare the kernel variant selected by this TC's precision combo.
// (All variants are defined in atan_asm.sx, linked in from that object.)
// ---------------------------------------------------------------------------
#if   (TC_INP_PREC == 0 && TC_OUT_PREC == 0)
extern void atan2_x64_chf_hf_asm(cfixed16_t *inp_p, fixed16_t  *out_p, unsigned count);
#elif (TC_INP_PREC == 0 && TC_OUT_PREC == 2)
extern void atan2_x64_chf_sp_asm(cfixed16_t *inp_p, float32_t  *out_p, unsigned count);
#elif (TC_INP_PREC == 1 && TC_OUT_PREC == 1)
extern void atan2_x64_chp_hp_asm(cfloat16_t *inp_p, float16_t  *out_p, unsigned count);
#elif (TC_INP_PREC == 1 && TC_OUT_PREC == 2)
extern void atan2_x64_chp_sp_asm(cfloat16_t *inp_p, float32_t  *out_p, unsigned count);
#elif (TC_INP_PREC == 2 && TC_OUT_PREC == 0)
extern void atan2_x64_csp_hf_asm(cfloat32_t *inp_p, fixed16_t  *out_p, unsigned count);
#elif (TC_INP_PREC == 2 && TC_OUT_PREC == 1)
extern void atan2_x64_csp_hp_asm(cfloat32_t *inp_p, float16_t  *out_p, unsigned count);
#elif (TC_INP_PREC == 2 && TC_OUT_PREC == 2)
extern void atan2_x64_csp_sp_asm(cfloat32_t *inp_p, float32_t  *out_p, unsigned count);
#else
#error "Unsupported precision combination (TC_INP_PREC, TC_OUT_PREC)"
#endif

// ---------------------------------------------------------------------------
// Number of 64-sample kernel iterations
// ---------------------------------------------------------------------------
#define ATAN_COUNT  (TC_INPUT_LEN / 64)

// ---------------------------------------------------------------------------
// Embedded test vectors (raw uint32 bit-patterns from prepare_golden.py)
// ---------------------------------------------------------------------------
static unsigned inp_words[TC_N_INP_WORDS] = {
#include "input.hex"
};

static const unsigned ref_words[TC_N_REF_WORDS] = {
#include "reference.hex"
};

// Output buffer — MUST be vector-aligned for VSPA store instructions
_VSPA_VECTOR_ALIGN unsigned out_words[TC_N_REF_WORDS];

// ---------------------------------------------------------------------------
// Test entry point
// ---------------------------------------------------------------------------
int main(void)
{
    int i;

    for (i = 0; i < TC_N_REF_WORDS; i++)
        out_words[i] = 0;

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();

    // Dispatch to correct kernel variant (selected at compile time)
#if   (TC_INP_PREC == 0 && TC_OUT_PREC == 0)

    atan2_x64_chf_hf_asm((cfixed16_t *)inp_words, (fixed16_t  *)out_words, ATAN_COUNT);
#elif (TC_INP_PREC == 0 && TC_OUT_PREC == 2)
    atan2_x64_chf_sp_asm((cfixed16_t *)inp_words, (float32_t  *)out_words, ATAN_COUNT);
#elif (TC_INP_PREC == 1 && TC_OUT_PREC == 1)
    atan2_x64_chp_hp_asm((cfloat16_t *)inp_words, (float16_t  *)out_words, ATAN_COUNT);
#elif (TC_INP_PREC == 1 && TC_OUT_PREC == 2)
    atan2_x64_chp_sp_asm((cfloat16_t *)inp_words, (float32_t  *)out_words, ATAN_COUNT);
#elif (TC_INP_PREC == 2 && TC_OUT_PREC == 0)
    atan2_x64_csp_hf_asm((cfloat32_t *)inp_words, (fixed16_t  *)out_words, ATAN_COUNT);
#elif (TC_INP_PREC == 2 && TC_OUT_PREC == 1)
    atan2_x64_csp_hp_asm((cfloat32_t *)inp_words, (float16_t  *)out_words, ATAN_COUNT);
#elif (TC_INP_PREC == 2 && TC_OUT_PREC == 2)
    atan2_x64_csp_sp_asm((cfloat32_t *)inp_words, (float32_t  *)out_words, ATAN_COUNT);
#endif

    KCYC_STOP_PRINT();

    return vspa_array_cmp(out_words, ref_words, TC_N_REF_WORDS);

}
