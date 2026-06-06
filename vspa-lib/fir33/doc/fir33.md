---
kernel: fir33
precision: [half_fixed]
status: sim_verified

inputs:
  - name: pIn
    shape: [M]
    dtype: complex_hfx
    alignment_words: 1
    description: "Complex half-fixed input data samples (interleaved Re/Im 16-bit)"
  - name: pTaps
    shape: [L]
    dtype: real_hfx
    alignment_words: 1
    description: "Real FIR tap coefficients in reversed order [h(L-1)…h(0)] (LS to MS)"

outputs:
  - name: pOut
    shape: [M]
    dtype: complex_hfx
    alignment_words: 1
    description: "Filtered output, same length as input"

scratch: []

parameters:
  M:
    description: "Number of input data samples; must be a multiple of 64"
    valid_values: []
    default: null
  L:
    description: "Number of FIR taps (filter order + 1); valid range 12–33"
    valid_values: []
    default: 33
  itDat:
    description: "Data iteration count = ceil(M / 64)"
    valid_values: []
    default: null
  itTaps:
    description: "Tap iteration count = L − 11"
    valid_values: []
    default: null

matlab_source:  submodules/la931x_vspa_common/vspa-lib/fir33/fir33_cwproj/matlab/r_firFilter.m
c_source:
  - submodules/la931x_vspa_common/vspa-lib/fir33/src/firFilterReal.sx
python_model:   framework/vspa_model/filter.py::r_decimator
test_dir:       tests/fir33/
doc:            []

depends_on: []

test_cases:
  - id: TC000
    params: {M: 64, L: 33, itDat: 1, itTaps: 22}
    notes: "Minimum M, maximum L"
  - id: TC001
    params: {M: 128, L: 16, itDat: 2, itTaps: 5}
    notes: "Mid-range taps"
  - id: TC002
    params: {M: 64, L: 12, itDat: 1, itTaps: 1}
    notes: "Minimum L"

perf:
  target_efficiency: null
  cycles: 2135
  au_config: vspa2_16au
  notes: "TC000 (M=64, L=33); 100% bit-exact; half_fixed complex input, real taps; re-measured runsim 2026-05-27 (median of 3)"
---

# fir33 — Real-Tap FIR Filter (half-fixed, 12–33 taps)

> Applies a 1-D FIR filter with real-valued taps to a complex half-fixed
> input vector; supports 12–33 taps.

## Algorithm

`firFilterReal` performs a direct-form FIR convolution:

$$y[n] = \sum_{k=0}^{L-1} h[k] \cdot x[n-k]$$

The VSPA implementation processes 64 complex samples per data-iteration
(`itDat`) and unrolls the tap loop in `itTaps = L − 11` steps, keeping
coefficients in the IPPU coefficient RAM to avoid repeated DDR fetches.

Taps must be provided in **reversed order**: `[h(L-1), h(L-2), …, h(0)]`
(least-significant to most-significant in time).

## Function API

```c
// From submodules/la931x_vspa_common/vspa-lib/fir33/include/fir.h
void firFilterReal(
    void     *pIn,     // input pointer to data sample x(0)
    void     *pOut,    // output buffer address
    void     *pTaps,   // pointer to reversed filter taps [h(L-1)…h(0)]
    uint32_t  itDat,   // data iteration count = ceil(M / 64)
    uint32_t  itTaps   // tap iteration count  = L - 11
);
```

## Memory Requirements

| Buffer | Min size (words) | Alignment (words) | Allocated by |
|--------|-----------------|-------------------|--------------|
| pIn    | M               | 1                 | Caller       |
| pOut   | M               | 1                 | Caller       |
| pTaps  | L               | 1                 | Caller       |

No scratch buffer required.

## Input/Output Layout

| Port   | Shape | Type        | Notes                              |
|--------|-------|-------------|-------------------------------------|
| pIn    | [M]   | complex_hfx | interleaved Re/Im; M multiple of 64 |
| pTaps  | [L]   | real_hfx    | reversed tap order; L ∈ [12, 33]   |
| pOut   | [M]   | complex_hfx | filtered output                     |

## Precision Modes

| Mode         | Description                               |
|--------------|-------------------------------------------|
| `half_fixed` | 16-bit fixed-point only; no SP variant    |

## Test Cases

| ID    | M   | L  | itDat | itTaps | Notes            |
|-------|-----|----|-------|--------|-----------------|
| TC000 | 64  | 33 | 1     | 22     | Max taps         |
| TC001 | 128 | 16 | 2     | 5      | Mid-range        |
| TC002 | 64  | 12 | 1     | 1      | Min taps         |

## Known Constraints

- L must be in the range [12, 33]. For L > 33 use `fir64`.
- M must be a multiple of 64.
- Complex input / real taps only — use a separate kernel for complex taps.
- `itDat = ceil(M / 64)`, `itTaps = L − 11` — these are **derived** parameters,
  not independently variable.

## References

- Public header: `submodules/la931x_vspa_common/vspa-lib/fir33/include/fir.h`
- MATLAB oracle: `submodules/la931x_vspa_common/vspa-lib/fir33/fir33_cwproj/matlab/r_firFilter.m`
- Python model: `tests/framework/vspa_model.py::r_firFilter`
