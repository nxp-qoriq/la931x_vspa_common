---
kernel: crc8
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: real_hfx
    description: "Input byte/word stream"
outputs:
  - name: crc
    shape: scalar
    dtype: real_hfx
    description: "8-bit CRC result"
parameters: {}

matlab_source:  []
c_source:       [submodules/la931x_vspa_common/vspa-lib/crc8/src/CRC8.c]
python_model:   ""
test_dir:       tests/crc8/
doc:
  - submodules/la931x_vspa_common/vspa-lib/crc8/doc/CRC8_Implementation_plan.pdf

depends_on: []

perf:
  cycles: 15419
  au_config: vspa2_16au
  notes: ""
---

# crc8

> _TODO: fill in description._

## Algorithm

Bitwise 8-bit Cyclic Redundancy Check (CRC) using polynomial 0xE0 (generator polynomial 
p(x) = x⁸ + x⁷ + x⁵ + 1).

For input data of variable bit width (8 to 34 bits per call):

1. **Initialize CRC**: XOR data with 0xFF.
2. **Shift & Divide Loop**: Process each data bit; if LSB is 1, XOR with 0xE0; 
   then right-shift.
3. **Finalize**: XOR result with 0xFF to produce 8-bit checksum.

Optimized for bit-serial computation on word-aligned data. Final CRC is appended 
to transmitted message; receiver recomputes CRC and compares for error detection.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/crc8/`
