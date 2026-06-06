---
kernel: logarithm
precision: [half]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: real_hfx
    description: "Real half-fixed input samples (positive magnitude values)"

outputs:
  - name: y
    shape: [N]
    dtype: real_hfx
    description: "Natural logarithm output in half-fixed precision"

parameters: {}

matlab_source:  submodules/la931x_vspa_common/vspa-lib/logarithm/matlab/r_log.m
c_source:       []
  - submodules/la931x_vspa_common/vspa-lib/logarithm/src/log_asm.sx
python_model:   framework/vspa_model/log.py::r_log
test_dir:       tests/logarithm/
doc:
  - submodules/la931x_vspa_common/vspa-lib/logarithm/doc/Log_Implementation_Plan.pdf
  - submodules/la931x_vspa_common/vspa-lib/logarithm/doc/Log_Testplan.xlsx

depends_on: []

perf:
  cycles: 1056
  au_config: vspa2_16au
  notes: "log_asm PASS (N=32, log(0.5)); re-measured runsim 2026-05-27"
---

# logarithm

> Element-wise natural logarithm for power analysis and magnitude scaling.

## Algorithm

Element-wise natural logarithm using fixed-point polynomial approximation.

For each positive real input `x[k]`, computes output `y[k] = ln(x[k])` via:

1. **Normalization**: Scale input to canonical range [1, 2) using dyadic shifts 
   (extract exponent and mantissa).

2. **Polynomial Approximation**: Apply a rational or polynomial fit (typical degree 2–4) 
   to compute log over the normalized domain.

3. **Scale Back**: Adjust result by the extracted exponent to recover full-range logarithm.

**Use case**: Power spectral estimation, dynamic range compression, and probability 
computations in communication receivers. Fixed-point implementation avoids costly 
division instructions.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/logarithm/`
