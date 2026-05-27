// SPDX-License-Identifier: BSD-3-Clause
// fir64 (fir_filter) end-to-end test harness — buffer layout mirrors NXP's
// cwproj driver (submodules/.../fir64/fir64_cwproj/Sources/main.c).
//
// Wiring rules (cwproj-aligned, see /nxp-kernel-test):
//   - input[]   : NUM_INPUT_SAMPLES uint32 = NUM_INPUT_SAMPLES complex cfixed16
//   - output[]  : NUM_INPUT_SAMPLES uint32, vector-aligned, BSS-zeroed
//   - history[SIZE_FIR_HISTORY/4] uint32, vector-aligned, BSS-zeroed
//                                                (kernel reads as filter history)
//   - taps[NUM_FIR_TAPS] float, vector-aligned   (IEEE-754 float32)
//   - num_samples = complex sample count (multiple of 32, minimum 128 on VSPA2)
//
// Real (non-trivial) test:  taps are a Hamming-windowed lowpass with
// DC gain == 1.  History is BSS-zero (mirroring cwproj's first-block call).
// The Python oracle r_firFilter64 (filter_prec='single',
// input_prec=output_prec='half_fixed') generates the bit-exact reference.
//
// Build:  make -C tests/fir64 AU=16
// Run:    python3 scripts/run_test.py --kernel fir64 --au 16

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vspa/intrinsics.h>

#include "fir_filter.h"
#include "test_utils.h"

#define NUM_INPUT_SAMPLES   128                        // complex samples (>= 128, % 32 == 0)
#define HIST_NWORDS         (SIZE_FIR_HISTORY / 4)     // unsigned-int count, == 64
#define TAPS_NWORDS         NUM_FIR_TAPS               // 63 floats

// Match cwproj: vector-aligned, declared at file scope so .bss boots zeroed.
_VSPA_VECTOR_ALIGN static uint32_t fir64_input  [NUM_INPUT_SAMPLES];
_VSPA_VECTOR_ALIGN static uint32_t fir64_output [NUM_INPUT_SAMPLES];
_VSPA_VECTOR_ALIGN static uint32_t fir64_history[HIST_NWORDS];
_VSPA_VECTOR_ALIGN static float    fir64_taps   [TAPS_NWORDS];

// Initializer source from gen_vectors.py — packed cfixed16 (im<<16)|re per word.
static const uint32_t INPUT_DATA[NUM_INPUT_SAMPLES] = {
#include "vectors/input.hex"
};

// Reference output (cfixed16 SM16 packed) computed by Python oracle.
static const uint32_t REF_DATA[NUM_INPUT_SAMPLES] = {
#include "vectors/ref.hex"
};

// Filter taps as raw IEEE-754 float32 bit patterns.  Stored as uint32 here
// and memcpy'd into `fir64_taps` at runtime so neither the C compiler nor the
// linker can re-quantise the constants en route to DMEM.
static const uint32_t TAPS_BITS[TAPS_NWORDS] = {
#include "vectors/taps.hex"
};

int main(void)
{
    int i;

    // Stage input into the asm's input buffer at runtime.
    for (i = 0; i < NUM_INPUT_SAMPLES; i++)
        fir64_input[i] = INPUT_DATA[i];

    // Stage taps as bit-exact float32 bit patterns into the kernel's tap buffer.
    // Using memcpy avoids any uint→float widening conversion the compiler
    // might otherwise insert; the 32-bit storage layout is identical.
    memcpy(fir64_taps, TAPS_BITS, sizeof(fir64_taps));

    // history stays BSS-zeroed; the Python oracle was called with
    // inp_hist=None (== zeros), so the asm and oracle see matching state.
    fir_filter((__fx16 *)fir64_output,
               (__fx16 *)fir64_input,
               NUM_INPUT_SAMPLES,
               (__fx16 *)fir64_history,
               fir64_taps);

    return vspa_array_cmp((const unsigned *)fir64_output,
                          (const unsigned *)REF_DATA,
                          NUM_INPUT_SAMPLES);
}
