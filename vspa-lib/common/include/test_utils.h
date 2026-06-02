// SPDX-License-Identifier: BSD-3-Clause
// VSPA Kernel Test Utilities
//
// Usage pattern in a test harness:
//   return vspa_array_cmp(y_out, y_ref, N);

#ifndef VSPA_TEST_UTILS_H
#define VSPA_TEST_UTILS_H

#include <vspa/intrinsics.h>
#include <stdio.h>

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
