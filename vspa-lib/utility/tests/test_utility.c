// SPDX-License-Identifier: BSD-3-Clause
// utility mpy_f32_f32 end-to-end harness.

#include <stdint.h>
#include <stdio.h>

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

    if (failures > 0) {
        printf("TOTAL MISMATCHES: %d / %d\n", failures, CASES);
    }
    printf("%s\n", failures == 0 ? "PASS" : "FAIL");
    return failures;
}
