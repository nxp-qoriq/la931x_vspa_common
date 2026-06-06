---
kernel: compression16_12b
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: real_hfx
    description: "16-bit fixed-point input samples"
outputs:
  - name: y
    shape: [N]
    dtype: real_hfx
    description: "12-bit compressed output samples"
parameters: {}

matlab_source:  []
c_source:       []
python_model:   ""
test_dir:       tests/compression16_12b/
doc:            []

depends_on: []

perf:
  cycles: 381
  au_config: vspa2_16au
  notes: ""
---

# compression16_12b

> _TODO: fill in description._

## Algorithm

Lossy compression reducing 16-bit fixed-point samples to 12-bit representation via 
midrise quantization with optional saturation.

For each 16-bit signed input, maps to 12-bit range [−2048, 2047] by:

1. **Scale & Shift**: Divide by 16 (or use barrel shifter) to compress dynamic range.
2. **Saturate**: Clamp to [−2048, 2047] to prevent overflow.
3. **Output**: 12-bit result (often packed into 16-bit aligned storage).

**Use case**: Reducing memory bandwidth in streaming DSP systems or data logging 
where signal SNR permits the loss of 4 LSBs.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/compression16_12b/`
