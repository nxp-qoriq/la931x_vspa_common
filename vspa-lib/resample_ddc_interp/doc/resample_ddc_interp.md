---
kernel: resample_ddc_interp
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Complex input samples"
outputs:
  - name: y
    shape: [N_out]
    dtype: complex_hfx
    description: "Resampled and interpolated complex output"
parameters: {}

matlab_source:  []
c_source:       []
python_model:   ""
test_dir:       tests/resample_ddc_interp/
doc:            []

depends_on: []

perf:
  cycles: 51
  au_config: vspa2_16au
  notes: "re-measured runsim 2026-05-27"
---

# resample_ddc_interp

> _TODO: fill in description._

## Algorithm

Cascaded Digital Down Converter (DDC) with polyphase interpolation for 
flexible sample-rate conversion and frequency shifting.

1. **NCO Mixer**: Complex multiply input by NCO to shift carrier frequency.
2. **Decimating FIR**: Anti-aliasing polyphase FIR filter, decimate by 2x or 4x.
3. **Interpolation**: Upsample back to target rate with polyphase synthesis filter.

Combined effect: Narrows bandwidth while shifting frequency, useful for waveform 
resampling with frequency translation (e.g., DDC in multi-channel receivers).

Supports 2x and 4x interpolation factors for flexible output rate configuration.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/resample_ddc_interp/`
