// SPDX-License-Identifier: BSD-3-Clause
// crc8_encode end-to-end harness.

#include <stdint.h>
#include <stdio.h>

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

    if (failures > 0) {
        printf("TOTAL MISMATCHES: %d / %d\n", failures, CASES);
    }
    printf("%s\n", failures == 0 ? "PASS" : "FAIL");
    return failures;
}
