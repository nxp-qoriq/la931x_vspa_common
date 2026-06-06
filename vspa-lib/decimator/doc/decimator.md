---
kernel: decimator
precision: [half_fixed, half, single, double]
status: sim_verified

inputs:
  - name: inp
    shape: [N]
    dtype: complex_variable
    description: "Complex time-domain input vector; precision controlled by input_prec"
  - name: flt
    shape: [M]
    dtype: real_variable
    description: "Real FIR filter coefficients in natural order [h0, h1, ...]"

outputs:
  - name: out
    shape: [N/factor]
    dtype: complex_variable
    description: "Decimated complex output; precision controlled by output_prec"
  - name: next_hist
    shape: "[M-1, log2(factor)]"
    dtype: complex_variable
    description: "History state for streaming block processing"

parameters:
  N:
    description: "Number of input samples"
    valid_values: []
    default: null
  M:
    description: "Filter length (number of coefficients)"
    valid_values: []
    default: null
  input_prec:
    description: "Input precision"
    valid_values: [half_fixed, half, single, double]
    default: half_fixed
  filter_prec:
    description: "Filter coefficient precision"
    valid_values: [half_fixed, half, single, double]
    default: half_fixed
  output_prec:
    description: "Output precision"
    valid_values: [half_fixed, half, single, double]
    default: half_fixed
  factor:
    description: "Decimation factor (2=2x, 4=4x cascaded 2x, 8=8x cascaded 2x)"
    valid_values: [2, 4, 8]
    default: 2

matlab_source:  submodules/la931x_vspa_common/vspa-lib/decimator/matlab/r_decimator.m
c_source:       [submodules/la931x_vspa_common/vspa-lib/decimator/src/decimator_c.c]
sx_source:      [submodules/la931x_vspa_common/vspa-lib/decimator/src/decimator_asm.sx]
python_model:   framework/vspa_model/filter.py::r_decimator
test_dir:       tests/decimator/
doc:
  - submodules/la931x_vspa_common/vspa-lib/decimator/doc/Decimator_Implementation_Plan.pdf
  - submodules/la931x_vspa_common/vspa-lib/decimator/doc/Decimator_Testplan.xlsx

depends_on: []

perf:
  cycles: 54
  au_config: vspa2_16au
  notes: "Factor=2 decimation with half_fixed I/O; re-measured runsim 2026-05-27"
---

# decimator — Time-Domain Downsampling

> Performs time-domain downsampling (decimation) by factors 2x, 4x, or 8x 
> using cascaded FIR filters to prevent spectrum aliasing.

## Algorithm

Implements cascaded 2x decimators with polyphase filtering. Each 2x stage 
outputs every other sample after FIR filtering. Factors 4x and 8x are 
implemented by cascading 2x stages while reusing the same filter for each stage.

## Function API

```c
void r_decimator(complex_t *inp, real_t *flt, ctrl_t *ctrl, 
                 complex_t *out, complex_t *next_hist);
```

## Memory Requirements

| Buffer | Size | Alignment | Notes |
|--------|------|-----------|-------|
| Input | N words | 1 | Complex samples (interleaved Re/Im) |
| Filter | M words | 1 | Real coefficients in natural order |
| Output | N/factor words | 1 | Decimated result |
| History | (M−1)×log₂(factor) | 1 | Streaming history state |

## Known Constraints

- History matrix must be preserved between successive block calls for 
  continuous decimation.
- First block should initialize history to zero.
- All four precision modes (half_fixed, half, single, double) are supported 
  independently for input, filter, and output.

## References

- MATLAB oracle: `submodules/la931x_vspa_common/vspa-lib/decimator/matlab/r_decimator.m`
- C implementation: `submodules/la931x_vspa_common/vspa-lib/decimator/src/decimator_c.c`
- VSPA assembly: `submodules/la931x_vspa_common/vspa-lib/decimator/src/decimator_asm.sx`
- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/decimator/`
