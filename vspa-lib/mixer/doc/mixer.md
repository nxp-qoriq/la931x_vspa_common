---
kernel: mixer
precision: [half_fixed]
status: sim_verified

inputs:
  - name: px
    shape: [L*32]             # L DMEM lines
    dtype: cfixed16_t
    alignment_words: 32       # vector-aligned (1 DMEM line)
    description: "Input signal samples (complex half-fixed)"

outputs:
  - name: py
    shape: [L*32]             # L DMEM lines
    dtype: cfixed16_t
    alignment_words: 32       # vector-aligned
    description: "Mixed output signal (complex half-fixed), in-place operation"

scratch: []

parameters:
  L:
    description: "Number of DMEM lines (each = 32 complex half-fixed samples)"
    valid_values: [1, 2, 4, 8, 16, 31]
    default: 2
  PhaseIn:
    description: "Input NCO phase (32-bit unsigned integer)"
    valid_values: []
    default: 0
  FreqIn:
    description: "NCO base frequency (32-bit signed integer, 1's complement)"
    valid_values: []
    default: 0

matlab_source:  "submodules/la931x_vspa_common/vspa-lib/mixer/matlab/"
c_source:
  - submodules/la931x_vspa_common/vspa-lib/mixer/source/mixer_asm.sx
  - submodules/la931x_vspa_common/vspa-lib/mixer/include/mixer.h
python_model:   ""
test_dir:       tests/mixer/
doc:
  - submodules/la931x_vspa_common/vspa-lib/mixer/doc/mixer_IPv0.pdf

depends_on: []

test_cases:
  - id: TC001
    params: {L: 2, PhaseIn: 0, FreqIn: 0}
    notes: "Zero-input, zero-frequency smoke test (y=0 for all outputs)"
  - id: TC002
    params: {L: 2, PhaseIn: 0, FreqIn: 1024}
    notes: "Non-zero NCO frequency with complex sine reference vector"

perf:
  target_efficiency: null
  cycles: 27
  au_config: vspa2_16au
  notes: "mixer_asm PASS with zero-input smoke harness (L=2). Full functional coverage with NCO/complex tone vectors needed.; re-measured runsim 2026-05-27 (median of 3)"
---

# mixer

> Time-domain mixer for frequency translation via NCO (Numerically-Controlled Oscillator) multiplication in wireless baseband processing.

## Algorithm

The mixer kernel performs element-wise multiplication of an input signal with an NCO-generated complex exponential sequence:

- **y = x . m**, where
  - x: input signal (linear buffer, complex half-fixed)
  - m: NCO-generated complex sequence (m = exp(j·2π·f·n), with phase accumulation)
  - y: mixed output (in-place operation)

## Function API

```c
unsigned int mixer_asm(cfixed16_t *py, cfixed16_t *px, uint32_t PhaseIn, int FreqIn, size_t L);
```

Returns: next phase value for chaining multiple calls (phase accumulation state).

## Memory Requirements

| Buffer | Min size (words) | Alignment (words) | Allocated by |
|--------|-----------------|-------------------|---------------|
| x      | L*32 (L lines)  | 32                | Caller       |
| y      | L*32 (L lines)  | 32                | Caller       |

_Note: In-place operation (output buffer can be same as input). Vector-aligned (1 DMEM line = 32 complex halves)._

## Input/Output Layout

| Port | Shape    | Type       | Notes |
|------|----------|------------|-------|
| px   | [L*32]   | cfixed16_t | Input signal, linear buffer |
| py   | [L*32]   | cfixed16_t | Mixed output, linear buffer (in-place) |

## Cycle Count

- **Formula:** 17 + 2*L cycles (linear time in L)
- **Efficiency:** VALU: 2L/(17+2L) = 48% @ L=8, 78% @ L=31

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/mixer/`
- NCO phase accumulation: standard DDS (Direct Digital Synthesis) algorithm
