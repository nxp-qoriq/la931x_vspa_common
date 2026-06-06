// SPDX-License-Identifier: BSD-3-Clause
// crc8_encode end-to-end harness.

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

static const uint32_t DATA1[CASES] = {
#include "vectors/data1.hex"
};
static const uint32_t DATA2[CASES] = {
#include "vectors/data2.hex"
};
static const uint32_t SIZE_BITS[CASES] = {
#include "vectors/size.hex"
};
static const uint32_t REF_CRC[CASES] = {
#include "vectors/ref.hex"
};

extern uint_fast8_t crc8_encode(uint32_t data1,
                                uint32_t data2,
                                uint_fast8_t size);

int main(void)
{
    int failures = 0;
    int i;

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    for (i = 0; i < CASES; i++) {
        uint32_t got = (uint32_t)crc8_encode(DATA1[i], DATA2[i], (uint_fast8_t)SIZE_BITS[i]);
        uint32_t exp = REF_CRC[i] & 0xFFu;

        if (got != exp) {
            if (failures == 0) {
                printf("FIRST MISMATCH idx=%d got=0x%08X exp=0x%08X\n", i, got, exp);
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
