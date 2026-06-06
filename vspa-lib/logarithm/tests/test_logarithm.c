// SPDX-License-Identifier: BSD-3-Clause
// log_asm end-to-end test harness.
//
// log_asm is a *scalar* float32 -> float32 routine:
//     float32_t log_asm(float32_t x, LOG_FACT_T fact);
//
// We mirror the cwproj-aligned pattern from `submodules/.../logarithm/
// log_cwproj/src/main.c`: BSS-allocated input/output float32 buffers,
// runtime copy of the input vector, scalar loop calling log_asm() per
// sample, then a uint32 bit-exact compare via framework/test_utils.h
// `vspa_array_cmp`, which prints FIRST MISMATCH and the trailing
// PASS/FAIL token that scripts/run_test.py greps for.
//
// The factor (LOG2x1 / LOG10x10 / LOG10x20) is selected at compile time
// via -DLOG_FACT=<enum>; gen_vectors.py picks the matching factor and
// computes a Python-oracle reference that is bit-exact against the asm
// for inputs constructed as  x = 2^k * (1 + j/2^M)  (LUT-knot mantissa,
// M=5).  See framework/vspa_model/log.py for the oracle.

#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

// log.h pulls in float32_t via vcpu.h, but vcpu.h also redeclares
// `void main(void)` (cwproj-style entry point) which conflicts with our
// `int main(void)` test harness. Provide the typedef locally before
// including log.h, then guard out vcpu.h.
typedef float float32_t;
#define __VCPU_H__   /* skip vcpu.h: it redeclares void main(void) */
#include "log.h"

#ifndef LOG_FACT
#define LOG_FACT LOG2x1
#endif

#define N 32

// Input is float32 IEEE-754 bit patterns from vectors/input.hex
static const uint32_t INPUT_DATA[N] = {
#include "vectors/input.hex"
};

static const uint32_t REF_DATA[N] = {
#include "vectors/ref.hex"
};

// BSS-zeroed runtime buffers, vector-aligned (cwproj pattern).
_VSPA_VECTOR_ALIGN static float32_t LOG_INP_BUFF[N];
_VSPA_VECTOR_ALIGN static float32_t LOG_OUT_BUFF[N];

int main(void)
{
    int i;

    // Stage live input into LOG_INP_BUFF (reinterpret uint32 as float32).
    for (i = 0; i < N; i++) {
        union { uint32_t u; float32_t f; } cvt;
        cvt.u = INPUT_DATA[i];
        LOG_INP_BUFF[i] = cvt.f;
        LOG_OUT_BUFF[i] = 0.0f;
    }

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    // Scalar log loop, mirroring cwproj/src/main.c.
    for (i = 0; i < N; i++) {
        LOG_OUT_BUFF[i] = log_asm(LOG_INP_BUFF[i], LOG_FACT);
    }
    KCYC_STOP_PRINT();

    // Bit-exact compare via uint32 reinterpretation.
    return vspa_array_cmp((const unsigned *)LOG_OUT_BUFF,
                          (const unsigned *)REF_DATA,
                          N);
}
