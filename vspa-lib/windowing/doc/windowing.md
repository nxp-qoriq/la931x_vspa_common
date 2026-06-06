---
kernel: windowing
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Complex input time-domain samples"
outputs:
  - name: y
    shape: [N]
    dtype: complex_hfx
    description: "Windowed complex output"
parameters: {}

matlab_source:  []
c_source:       []
python_model:   ""
test_dir:       tests/windowing/
doc:            []

depends_on: []

perf:
  cycles: 23
  au_config: vspa2_16au
  notes: "re-measured runsim 2026-05-27"
---

# windowing

> _TODO: fill in description._

## Algorithm

Applies a window function element-wise to time-domain complex samples to reduce 
spectral leakage in FFT processing.

For each sample `x[k]`, multiplies by a pre-computed window coefficient `w[k]`:
```
y[k] = x[k] · w[k]
```

Supported window types include Hamming, Hann (Hanning), Blackman, and Kaiser, 
selected at runtime via parameter. Window coefficients are typically precomputed 
and stored in ROM or DMEM.

**Use case**: OFDM burst processing, spectral analysis, and other applications where 
windowing improves frequency resolution and reduces out-of-band emissions.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/windowing/`
