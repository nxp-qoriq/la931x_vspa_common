// SPDX-License-Identifier: BSD-3-Clause
// pow_acc_asm end-to-end test harness.
//
// pow_acc_asm has no NXP cwproj sample driver; we adopt the same buffer-
// padding pattern used by the cwproj-aligned decimator harness:
//   - input buffer is generously oversized and lives in .bss so the asm's
//     pipelined prefetch reads land in zero-initialised memory rather than
//     off the end of the static input[] (which crashes the simulator with
//     "DMem out of range");
//   - acc buffer is line-aligned and zero-initialised on entry, matching
//     pow.h's contract;
//   - compare uses framework/test_utils.h `vspa_array_cmp`, which prints
//     the FIRST MISMATCH line (if any) and the trailing PASS/FAIL token
//     that scripts/run_test.py greps for.

#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

#define N_PER_LINE   32     // AU=16: 32 complex samples per line
// nL must be >= 2: the asm prologue does `sub g0, 1; set.loop g0, 1` and
// `set.loop` with agX=0 is interpreted by the HW as 65536 iterations
// (see docs/isa/03g_vcpu_insn_gp_loops_flow_cond_nco.md, agX semantics).
#define N_LINES      8
#define INP_LINES    24     // headroom for asm pipeline prefetch

#define INP_COMPLEX  (N_PER_LINE * INP_LINES)
#define ACC_SLOTS    N_PER_LINE

// gen_vectors.py emits N_PER_LINE * N_LINES uint32 (= 32 packed cfixed16).
static const uint32_t INPUT_DATA[N_PER_LINE * N_LINES] = {
#include "vectors/input.hex"
};

static const uint32_t REF_DATA[ACC_SLOTS] = {
#include "vectors/ref.hex"
};

// Padded input buffer in BSS — first N_LINES lines hold real input, the
// rest are zero so the asm's prefetched (but unused) loads are harmless.
// uint32 elements; pow_acc_asm dereferences as cfixed16_t* (2 halfwords).
_VSPA_VECTOR_ALIGN static uint32_t POW_INP_BUFF[INP_COMPLEX];
_VSPA_VECTOR_ALIGN static uint32_t POW_ACC[ACC_SLOTS];

extern void pow_acc_asm(void *sample_in, void *acc_inout, unsigned int nL);

int main(void)
{
    int i;

    // Stage live input into the start of POW_INP_BUFF; rest stays zero.
    for (i = 0; i < N_PER_LINE * N_LINES; i++)
        POW_INP_BUFF[i] = INPUT_DATA[i];
    for (i = 0; i < ACC_SLOTS; i++)
        POW_ACC[i] = 0;

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    pow_acc_asm((void *)POW_INP_BUFF, (void *)POW_ACC, (unsigned)N_LINES);
    KCYC_STOP_PRINT();

    return vspa_array_cmp((const unsigned *)POW_ACC,
                          (const unsigned *)REF_DATA,
                          ACC_SLOTS);
}
