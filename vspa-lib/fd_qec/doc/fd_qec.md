---
kernel: fd_qec
precision: [half]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Frequency-domain received symbols (pilot + data)"

outputs:
  - name: y
    shape: [N]
    dtype: complex_hfx
    description: "Quantization-error-cancelled symbols"

parameters: {}

matlab_source:  ""
c_source:
  - submodules/la931x_vspa_common/vspa-lib/fd_qec/src/fd_qec.sx
python_model:   ""
test_dir:       tests/fd_qec/
doc:
  - submodules/la931x_vspa_common/vspa-lib/fd_qec/doc/fd_qec_Implementation_plan.pdf
  - submodules/la931x_vspa_common/vspa-lib/fd_qec/doc/fd_qec_Testplan.xlsx

depends_on: []

perf:
  cycles: 56
  au_config: vspa2_16au
  notes: "fd_qec PASS (N=32, zero-input); re-measured runsim 2026-05-27"
---

# fd_qec

> Frequency-domain Quantization Error Cancellation for OFDM equalization.

## Algorithm

Frequency-domain Quantization Error Cancellation to reduce inter-subcarrier 
interference from FFT fixed-point quantization noise.

Operates on frequency-domain OFDM symbols:

1. **Estimate Channel**: Extract pilot subcarrier estimates for per-subcarrier 
   channel gain magnitude.

2. **Predict Quantization Noise**: Model quantization error power as a function of 
   signal magnitude and FFT bit width. High-level signals → larger quantization 
   noise floor.

3. **Linear Cancellation**: Apply Wiener-optimal (or approximated) filter per 
   subcarrier to attenuate predicted noise.

**Benefit**: Improved SNR especially for small-magnitude subcarriers that are 
obscured by quantization noise from neighboring high-power subcarriers. Critical 
for 16-bit FFT in wideband OFDM.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/fd_qec/`
