// SPDX-License-Identifier: BSD-3-Clause
// VSPA Kernel Test Utilities
//
// Usage pattern in a test harness:
//   return vspa_array_cmp(y_out, y_ref, N);

#ifndef VSPA_TEST_UTILS_H
#define VSPA_TEST_UTILS_H

#include <vspa/intrinsics.h>
#include <stdio.h>
#include <time.h>

// ---------------------------------------------------------------------------
// Kernel cycle measurement (KCYC_*)
// ---------------------------------------------------------------------------
// Measures the cycle cost of the kernel body ONLY, not the whole program.
// This is the honest "kernel cycles" number (e.g. ~64 for atan TC001), as
// opposed to the runsim total program CYC: count (~2665) which also includes
// startup, vector copies and printf.
//
// Mechanism follows NXP's own cwproj pattern (e.g. vspa-lib/dft .../main.c):
// read the on-chip free-running cycle counter via clock(), bracket the kernel
// call, and subtract the back-to-back clock() call overhead. A short pipeline
// flush (fnop x3) before/after stops out-of-order pipeline effects from
// leaking into the measured window.
//
// Usage in a harness:
//     KCYC_INIT();                 // once, before the measured region
//     KCYC_START();
//     my_kernel_asm(in, out, n);   // the kernel call under test
//     KCYC_STOP_PRINT();           // prints "KERNEL_CYCLES: <n>"
//
// The runner (run_test.py / vspa_runner.py / run_sim.py) parses the
// "KERNEL_CYCLES: <n>" line; -showpc is no longer required for cycle counts.

static clock_t _kcyc_start, _kcyc_end, _kcyc_overhead;

// Flush the VSPA pipeline so clock() brackets only the kernel body.
// O0 + unused attribute keep it intact and warning-free in every TU.
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
        printf("KERNEL_CYCLES: %ld\n",                            \
               (long)(_kcyc_end - _kcyc_start - _kcyc_overhead)); \
    } while (0)


// Exact uint32 comparison (bit-accurate).

#define VSPA_ASSERT_EQ(actual, expected, fail_count)  \
    do { if ((actual) != (expected)) (*fail_count)++; } while (0)

// Compare two uint32 arrays exactly.
// On the first mismatch, prints the index and both values via printf so the
// simulator forwards it to stdout — visible in run_test.py HARNESS OUTPUT.
// Returns total mismatch count (== main() return code seen by run_test.py).
static int vspa_array_cmp(const unsigned *actual, const unsigned *expected, int n)
{
    int i, failures = 0;
    for (i = 0; i < n; i++) {
        if (actual[i] != expected[i]) {
            if (failures == 0)
                printf("FIRST MISMATCH idx=%d  actual=0x%08X  expected=0x%08X\n",
                       i, actual[i], expected[i]);
            failures++;
        }
    }
    if (failures > 0)
        printf("TOTAL MISMATCHES: %d / %d\n", failures, n);
    printf("%s\n", failures == 0 ? "PASS" : "FAIL");
    return failures;
}

#endif /* VSPA_TEST_UTILS_H */
