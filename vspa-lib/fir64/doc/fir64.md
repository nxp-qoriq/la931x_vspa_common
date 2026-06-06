---
kernel: fir64
precision: [half_fixed]
status: sim_verified

inputs:
  - name: input
    shape: [num_samples]
    dtype: hfx
    alignment_words: 128      # 128-byte (64-word) aligned
    description: "Input signal samples (half-fixed Q15)"

outputs:
  - name: output
    shape: [num_samples]
    dtype: hfx
    alignment_words: 128      # 128-byte aligned
    description: "Filtered output samples (half-fixed Q15)"

scratch:
  - name: history
    size_words: 256           # SIZE_FIR_HISTORY
    alignment_words: 128      # 128-byte aligned
  - name: filter_taps
    size_words: 252           # (NUM_FIR_TAPS * 4), max 63 taps
    alignment_words: 128      # 128-byte aligned

parameters:
  num_samples:
    description: "Number of input samples"
    valid_values: []          # VSPA2: multiple of 32, min 128
    default: 128
  NUM_FIR_TAPS:
    description: "Number of filter taps"
    valid_values: []          # VSPA2: 40-64
    default: 63

matlab_source:  "submodules/la931x_vspa_common/vspa-lib/fir64/matlab/"
c_source:
  - submodules/la931x_vspa_common/vspa-lib/fir64/src/fir_filter.sx
  - submodules/la931x_vspa_common/vspa-lib/fir64/include/fir_filter.h
python_model:   framework/vspa_model/filter.py::r_decimator
test_dir:       tests/fir64/
doc:
  - submodules/la931x_vspa_common/vspa-lib/fir64/doc/fir64_implementation_plan.pdf

depends_on: []

test_cases:
  - id: TC001
    params: {num_samples: 128, NUM_FIR_TAPS: 0}
    notes: "Zero-tap smoke test (all taps=0 → output=0)"
  - id: TC002
    params: {num_samples: 128, NUM_FIR_TAPS: 63}
    notes: "Full 63-tap filter with randomized coefficient vector"

perf:
  target_efficiency: null
  cycles: 343
  au_config: vspa2_16au
  notes: "PASS with zero-tap smoke harness (N=128). Full functional coverage with nontrivial tap vectors needed.; re-measured runsim 2026-05-27 (median of 3)"
---

# fir64

> High-performance FIR polyphase filter for multirate signal processing (interpolation, decimation, resampling).

## Algorithm

The fir64 kernel implements a polyphase FIR filter optimized for VSPA. The filter operates on the current input block and a history buffer containing previous samples:

1. **History management:** Maintain a sliding window of past samples
2. **Convolution:** Compute output = Σ(input[k] * taps[k]) for each sample
3. **Polyphase structure:** Support multi-rate operations via tap scheduling

## Function API

```c
void fir_filter(__fx16 *output, __fx16 *input, unsigned int num_samples, __fx16 *history, float *filter_taps);
```

## Memory Requirements

| Buffer      | Min size (words) | Alignment (words) | Allocated by |
|-------------|-----------------|-------------------|---------------|
| input       | num_samples     | 128 (128 bytes)   | Caller       |
| output      | num_samples     | 128 (128 bytes)   | Caller       |
| history     | 256             | 128               | Caller       |
| filter_taps | 252 (63 taps)   | 128               | Caller       |

_Note: VSPA2 requires num_samples ≥ 128 and multiple of 32. History must be cleared to zero at initialization._

## Input/Output Layout

| Port | Shape | Type      | Notes |
|------|-------|-----------|-------|
| input  | [num_samples] | hfx | Linear buffer |
| output | [num_samples] | hfx | Linear buffer, can be same as input |

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/fir64/`
- MATLAB reference: `r_firFilter` in vspa_model.py
