---
kernel: qam
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Complex received samples (IQ demodulation input)"

outputs:
  - name: llr
    shape: [N*bits_per_symbol]
    dtype: real_hfx
    description: "Log-likelihood ratio (LLR) hard-bit stream"

parameters:
  N:
    description: "Number of input symbols"
    valid_values: []
    default: null
  constellation:
    description: "QAM constellation order (e.g., 4=QPSK, 16=16QAM, 64=64QAM)"
    valid_values: [4, 16, 64, 256]
    default: null

matlab_source:  submodules/la931x_vspa_common/vspa-lib/qam/matlab/llr_reorder.m
c_source:       []
python_model:   framework/vspa_model/qam.py::r_qam_mod
test_dir:       tests/qam/
doc:
  - submodules/la931x_vspa_common/vspa-lib/qam/doc/QAM_Implementation_plan.pdf

depends_on: []

perf:
  cycles: 88
  au_config: vspa2_16au
  notes: "re-measured runsim 2026-05-27"
---

# qam

> _TODO: fill in description._

## Algorithm

_Not yet documented._

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/qam/`
