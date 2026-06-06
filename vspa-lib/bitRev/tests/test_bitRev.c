// SPDX-License-Identifier: BSD-3-Clause
// bitRev64 end-to-end bit-accurate test harness.
//
// Root cause of the historical "all-zero output" failure:
//
//   The submodule's bitRev64Invoke() calls
//       ippu_arg_base((uint32_t)ippu_args);
//   passing the *VCPU-space* address of the ippu_args[] global. In the LCF
//   shipped with the runsim simulator (vspa2_<N>au_sp.lcf) `ippu_dram` is
//   placed at a non-zero base inside the shared physical DRAM, so the IPPU
//   sees ippu_args at offset (vcpu_addr - ippu_dmem_base()), not at the
//   raw VCPU address. Without IPPU_OFFSET() the IPPU's `ld as0,+1`
//   instruction reads garbage instead of the input pointer.
//
//   The cwproj LCF (la9310_sp.lcf) places ippu_dram lower in the shared
//   DRAM (just past VCPU dmem) and works against real silicon, but the
//   simulator's LCF still requires the explicit IPPU_OFFSET translation.
//
// Fix (test-side only): bypass bitRev64Invoke() and program ippu_args /
// ippu_arg_base directly with IPPU_OFFSET() applied. The asm kernel itself
// is untouched.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
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

#define main vcpu_sdk_main
#include "vcpu.h"
#undef main
#include "ippu.h"
#include "bitRev.h"

extern uint32_t volatile ippu_args[];
extern void bitRev64(void);

#define N 64

_VSPA_VECTOR_ALIGN static unsigned input[N] = {
#include "vectors/input.hex"
};

_VSPA_VECTOR_ALIGN static unsigned out[N];

static const unsigned ref[N] = {
#include "vectors/ref.hex"
};

static int vspa_array_cmp_u32(const unsigned *actual, const unsigned *expected, int n)
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

int main(void)
{
    int i;

    for (i = 0; i < N; i++)
        out[i] = 0;

    if (ippu_is_busy()) {
        printf("IPPU busy\n");
        printf("FAIL\n");
        return 1;
    }

    /* Mirror bitRev64Invoke() but use IPPU_OFFSET() for the arg base so
       the IPPU sees ippu_args[] correctly under the simulator LCF. */
    ippu_args[0] = (uint32_t)((void *)out);
    ippu_args[1] = (uint32_t)((void *)input);
    ippu_arg_base(IPPU_OFFSET((uint32_t)ippu_args));
    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    ippu_enable((ippu_proc_t)bitRev64, IPPU_PEND_NONE | IPPU_MODE_16BIT);

    __asm volatile("fnop .asmvol");
    __asm volatile("fnop .asmvol");
    __asm volatile("fnop .asmvol");

    while (!ippu_is_done()) {
    }
    KCYC_STOP_PRINT();

    return vspa_array_cmp_u32(out, ref, N);
}
