// SPDX-License-Identifier: BSD-3-Clause
// sigcond end-to-end test harness — buffer layout mirrors NXP's cwproj
// driver (submodules/.../sigcond/sigcond_cwproj/Sources/main.c) so the
// asm sees identical inputs and a clean (BSS-zeroed) state.
//
// Why the previous harness was broken (silent failure):
//   - Input buffer was sized to 32 u32 entries, but the asm function
//     `customsigcond_ddc1x_N2560_4t` reads up to
//     CUSTOMSIGCOND_INPUTCIRCBUFFERSIZE_N2560 = 5120 32-bit words inside
//     the circular range (set.range g2, ...). That out-of-bounds access
//     triggered the simulator's "MemArray reading data that is out of
//     bounds" errors visible in the previous baseline run.
//   - It used a local cmp_u32() that only set the exit code; it never
//     printed PASS/FAIL, so scripts/run_test.py fell through to the
//     "no PASS/FAIL line" branch.
//
// What this harness does:
//   - Allocates input/output buffers at the cwproj sizes (5120 complex
//     each, BSS-zeroed via static linkage so they boot to zero).
//   - Allocates structSigCondParams with inCircBuffBase pointing to the
//     input buffer base (the only field the asm dereferences from
//     pConfig that needs an explicit value; everything else is zero).
//   - With zero input, zero filter taps and zero configuration, the
//     signal-conditioning chain (Gain*0 + DCOffset 0 -> IQImb*0 ftaps)
//     emits all zeros. We use that as a smoke test and switch the
//     comparator to `vspa_array_cmp` so PASS/FAIL is printed.
//
// Note: this is intentionally a smoke test, not a numerical-accuracy
// regression. There is no Python oracle for sigcond yet; a richer
// harness would load NXP's cwproj test_vectors/test2_N256_BW80_*.hex
// against the customsigcond2 entry points (different .sx symbol set,
// requires CUSTOMSIGCOND_IQSSFILT_NUMTAPS == 5 build).

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

#include "sigcond.h"

// Per CUSTOMSIGCOND_INPUTCIRCBUFFERSIZE_N2560 = 5120 32-bit words.
// Each complex_fixed16 is 4 bytes (32 bits) -> 5120 complex samples.
#define N_COMPLEX 5120

// BSS-resident, zero-initialised by default.
__attribute__((aligned(64))) static vspa_complex_fixed16 input_buffer[N_COMPLEX];
__attribute__((aligned(64))) static vspa_complex_fixed16 output_buffer[N_COMPLEX];
__attribute__((aligned(64))) static structSigCondParams  sigcond_cfg;

// Required by the customsigcond assembly implementation. Both default
// to zero in BSS, matching cwproj's clean-boot state. Alignment attrs
// match the extern declarations in <sigcond.h>.
vspa_pair_fixed16    customsigcond_FilterTaps[13];
vspa_complex_fixed16 customsigcond_ScratchMem[2592] __attribute__((aligned(64)));

int main(void)
{
    // Mirror cwproj main.c: only field the kernel reads directly via
    // a pointer dereference is inCircBuffBase. With zero input + zero
    // taps + zero gain/DC/IQ-imb-coeffs, the chain output is all zero.
    sigcond_cfg.inCircBuffBase = input_buffer;

    // Measure only the kernel body (honest cycle count, not whole-program).
    KCYC_INIT();
    KCYC_START();
    customsigcond_ddc1x_N2560_4t(input_buffer, output_buffer, &sigcond_cfg);
    KCYC_STOP_PRINT();

    // Expected: N_COMPLEX zero complex samples. Compare as 32-bit words
    // (one cfixed16 == one u32). vspa_array_cmp prints PASS/FAIL.
    static const unsigned zeros[N_COMPLEX] = {0};

    return vspa_array_cmp((const unsigned *)output_buffer,
                          zeros,
                          N_COMPLEX);
}
