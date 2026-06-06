// SPDX-License-Identifier: BSD-3-Clause
// utility mpy_f32_f32 end-to-end harness.

#include <stdint.h>
#include <stdio.h>

// ---------------------------------------------------------------------------
// Kernel cycle measurement (KCYC_*) - self-contained copy for harnesses that
// do not include common/include/test_utils.h. Measures the kernel body only
// (honest cycle count), not the whole-program runsim total.
// ---------------------------------------------------------------------------
#include <time.h>
static clock_t _kcyc_start, _kcyc_end, _kcyc_overhead;
__attribute__((unused, optimize("O0")))
static void _kcyc_pipe_flush(void)
{
    __asm("fnop;");
    __asm("fnop;");
    __asm("fnop;");
}
#define KCYC_INIT()                                               \
    do {                                                          \
        _kcyc_start    = clock();                                 \
        _kcyc_end      = clock();                                 \
        _kcyc_overhead = _kcyc_end - _kcyc_start;                 \
    } while (0)
#define KCYC_START()                                              \
    do {                                                          \
        _kcyc_pipe_flush();                                       \
        _kcyc_start = clock();                                    \
    } while (0)
#define KCYC_STOP_PRINT()                                         \
    do {                                                          \
        _kcyc_end = clock();                                      \
        _kcyc_pipe_flush();                                       \
        printf("KERNEL_CYCLES: %ld\n",                           \
               (long)(_kcyc_end - _kcyc_start - _kcyc_overhead)); \
    } while (0)

#define CASES 64

typedef union {
    uint32_t u;
    float f;
} f32_u32;

static const uint32_t A_BITS[CASES] = {
#include "vectors/a.hex"
};
static const uint32_t B_BITS[CASES] = {
#include "vectors/b.hex"
};
static const uint32_t REF_BITS[CASES] = {
#include "vectors/ref.hex"
};

extern float mpy_f32_f32(float f32_1, float f32_2);

int main(void)
{
    int i;
    int failures = 0;

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    for (i = 0; i < CASES; i++) {
        f32_u32 a;
        f32_u32 b;
        f32_u32 got;

        a.u = A_BITS[i];
        b.u = B_BITS[i];
        got.f = mpy_f32_f32(a.f, b.f);

        if (got.u != REF_BITS[i]) {
            if (failures == 0) {
                printf("FIRST MISMATCH idx=%d got=0x%08X exp=0x%08X\n", i, got.u, REF_BITS[i]);
            }
            failures++;
        }
    }
    KCYC_STOP_PRINT();

    if (failures > 0) {
        printf("TOTAL MISMATCHES: %d / %d\n", failures, CASES);
    }
    printf("%s\n", failures == 0 ? "PASS" : "FAIL");
    return failures;
}
