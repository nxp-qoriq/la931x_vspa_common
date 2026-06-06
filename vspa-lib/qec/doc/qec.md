---
kernel: qec
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Complex received symbols"
outputs:
  - name: y
    shape: [N]
    dtype: complex_hfx
    description: "Equalized complex output"
parameters: {}

matlab_source:  []
c_source:       []
python_model:   ""
test_dir:       tests/qec/
doc:            []

depends_on: []

perf:
  cycles: 31
  au_config: vspa2_16au
  notes: "re-measured runsim 2026-05-27"
---

# qec

> _TODO: fill in description._

## Algorithm

Quantization Error Cancellation (QEC) in frequency domain for OFDM reception.

Reduces inter-subcarrier interference caused by fixed-point quantization noise in 
the FFT output. Operates on frequency-domain pilot and data subcarriers:

1. Extract channel estimate from pilot subcarriers.
2. Predict quantization error magnitude for each subcarrier based on signal level.
3. Apply adaptive linear filter (Wiener-type) to cancel predicted error.

Result: Improved constellation SNR by mitigating quantization noise leakage across 
subcarrier boundaries, particularly valuable in 16-bit fixed-point FFT processors.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/qec/`
