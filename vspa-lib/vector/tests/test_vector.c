// SPDX-License-Identifier: BSD-3-Clause
// vector rhf_rhf_rhf_vAddSclr_asm end-to-end test harness.
// Wiring mirrors NXP's cwproj driver:
//   submodules/la931x_vspa_common/vspa-lib/vector/vSclr_cwproj/Sources/main.c
//
// cwproj allocates `__fx16 alpha[3] _VSPA_VECTOR_ALIGN` and passes
// `&alpha[1]` to the asm. The asm reads the scalar plus its two
// neighbours (lane gather), so we reserve 3 slots and place the value
// at index 1. The data buffer is over-allocated to L_MAX * 64 lanes so
// vector-load prefetch always lands in BSS-zero — matches cwproj.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

/* Forward-declare the asm kernel directly — including `vector.h` would
   pull in `vcpu.h` which redeclares `main` as `void main(void) __noreturn`
   and clashes with the `int main(void)` we need here. */
extern void rhf_rhf_rhf_vAddSclr_asm(int16_t *py, int16_t *px,
                                     int16_t *px2, size_t L);

/* Halfword-granularity comparator (`vspa_array_cmp` from test_utils.h
   takes 32-bit words; vAddSclr operates on real half_fixed = 16 bits per
   sample, and casting a uint16-aligned array to `unsigned *` produces
   misaligned reads on VSPA). Prints the same PASS/FAIL terminator that
   scripts/run_test.py parses. */
static int vspa_array_cmp_u16(const unsigned short *actual,
                              const unsigned short *expected, int n)
{
    int i, failures = 0;
    for (i = 0; i < n; i++) {
        if (actual[i] != expected[i]) {
            if (failures == 0)
                printf("FIRST MISMATCH idx=%d  actual=0x%04X  expected=0x%04X\n",
                       i, actual[i], expected[i]);
            failures++;
        }
    }
    if (failures > 0)
        printf("TOTAL MISMATCHES: %d / %d\n", failures, n);
    printf("%s\n", failures == 0 ? "PASS" : "FAIL");
    return failures;
}

#define L       2
#define L_MAX   8
#define BUF_LEN (L_MAX * 64)   // cwproj L_MAX * (__AU_COUNT__ * 4) for AU=16
#define DATA_N  (L * 64)       // actual operand length

// Compile-time inputs (from gen_vectors.py).
static const unsigned short INPUT_DATA[DATA_N] = {
#include "vectors/input.hex"
};

// Three alpha slots: [0]=guard, [1]=scalar, [2]=guard.
static const unsigned short ALPHA_DATA[3] = {
#include "vectors/alpha.hex"
};

static const unsigned short REF_DATA[DATA_N] = {
#include "vectors/ref.hex"
};

// cwproj-aligned BSS buffers — vector-aligned, zero on entry.
_VSPA_VECTOR_ALIGN static unsigned short x_buf[BUF_LEN];
_VSPA_VECTOR_ALIGN static unsigned short y_buf[BUF_LEN];
_VSPA_VECTOR_ALIGN static unsigned short alpha_buf[3];

int main(void)
{
    int i;

    for (i = 0; i < DATA_N; i++)
        x_buf[i] = INPUT_DATA[i];
    for (i = 0; i < 3; i++)
        alpha_buf[i] = ALPHA_DATA[i];

    rhf_rhf_rhf_vAddSclr_asm(
        (int16_t *)y_buf,
        (int16_t *)x_buf,
        (int16_t *)&alpha_buf[1],   // cwproj passes &alpha[1]
        (size_t)L);

    return vspa_array_cmp_u16(y_buf, REF_DATA, DATA_N);
}
