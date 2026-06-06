---
kernel: txiqcomp
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Complex TX I/Q input samples"
outputs:
  - name: y
    shape: [N]
    dtype: complex_hfx
    description: "Compensated I/Q output samples"
parameters: {}

matlab_source:  submodules/la931x_vspa_common/vspa-lib/txiqcomp/matlab/r_txiqcomp2.m
c_source:       []
python_model:   ""
test_dir:       tests/txiqcomp/
doc:
  - submodules/la931x_vspa_common/vspa-lib/txiqcomp/doc/txiqcomp_function_description.pdf

depends_on: []

perf:
  cycles: 56
  au_config: vspa2_16au
  notes: "re-measured runsim 2026-05-27"
---

# txiqcomp

> _TODO: fill in description._

## Algorithm

Transmitter I/Q compensation chain mitigating transmit-path impairments:

1. **DC Offset Subtraction**: Removes DC bias on both I and Q channels.

2. **Gain Normalization**: Applies wideband gain to equalize average transmit power.

3. **I/Q Imbalance Correction**: Compensates for amplitude and phase mismatch between 
   I and Q channels using two independent scaling factors (f1, f2) and fractional-delay 
   filtering (polyphase filter + integer delay).

The correction operates in time domain on TX baseband I/Q samples. Typical I/Q 
imbalance arises from analog front-end component tolerance; this algorithm 
uses calibration parameters (gain, DC offset, imbalance factors) to restore 
I/Q orthogonality.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/txiqcomp/`
