// SPDX-License-Identifier: BSD-3-Clause
// comp16_12b end-to-end harness.

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
#include <vspa/intrinsics.h>

#define NB 2
#define IN_WORDS  (NB * 24 * 64)
#define OUT_WORDS (NB * 18 * 64)

_VSPA_VECTOR_ALIGN static uint16_t INP[IN_WORDS];
_VSPA_VECTOR_ALIGN static uint16_t OUT[OUT_WORDS];

static const uint16_t INPUT_DATA[IN_WORDS] = {
#include "vectors/input.hex"
};

static const uint16_t REF_DATA[OUT_WORDS] = {
#include "vectors/ref.hex"
};

extern void comp16_12b(unsigned short *buf_12b, unsigned short *buf_16b, unsigned int nb);

int main(void)
{
    int i;
    int failures = 0;

    for (i = 0; i < IN_WORDS; i++) {
        INP[i] = INPUT_DATA[i];
    }
    for (i = 0; i < OUT_WORDS; i++) {
        OUT[i] = 0;
    }

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    comp16_12b((unsigned short *)OUT, (unsigned short *)INP, NB);
    KCYC_STOP_PRINT();

    for (i = 0; i < OUT_WORDS; i++) {
        uint32_t got = (uint32_t)OUT[i];
        uint32_t exp = (uint32_t)REF_DATA[i];
        if (got != exp) {
            if (failures == 0) {
                printf("FIRST MISMATCH idx=%d got=0x%04X exp=0x%04X\n", i, (unsigned)got, (unsigned)exp);
            }
            failures++;
        }
    }

    if (failures > 0) {
        printf("TOTAL MISMATCHES: %d / %d\n", failures, OUT_WORDS);
    }
    printf("%s\n", failures == 0 ? "PASS" : "FAIL");
    return failures;
}
