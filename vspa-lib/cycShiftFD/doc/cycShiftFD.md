---
kernel: cycShiftFD
precision: [half]
status: sim_verified

inputs:
  - name: in_p
    shape: [L*32]
    dtype: cfloat16_t
    alignment_words: 32

outputs:
  - name: out_p
    shape: [L*32]
    dtype: cfloat16_t
    alignment_words: 32

scratch: []

parameters:
  shift:
    description: "Cyclic shift amount"
    valid_values: []
    default: 0
  length:
    description: "Total number of samples"
    valid_values: []
    default: 64

matlab_source:  submodules/la931x_vspa_common/vspa-lib/cycShiftFD/matlab/m_cycShiftFD.m
c_source:       []
  - submodules/la931x_vspa_common/vspa-lib/cycShiftFD/src/cycShiftFD.sx
python_model:   ""
test_dir:       tests/cycShiftFD/
doc:
  - submodules/la931x_vspa_common/vspa-lib/cycShiftFD/doc/cycShiftFD.pdf
  - submodules/la931x_vspa_common/vspa-lib/cycShiftFD/doc/cycShiftFD_Testplan.xlsx

depends_on: []

perf:
  cycles: 46
  au_config: vspa2_16au
  notes: "cycShiftFD_asm PASS (shift=0, L=2); re-measured runsim 2026-05-27 (median of 3)"
---

# cycShiftFD

> Cyclic frequency-domain shift for OFDM subcarrier rotation.

## Algorithm

Element-wise cyclic rotation: **y[k] = x[(k - shift) mod length]**

## Function API

```c
void cycShiftFD_asm(cfloat16_t *in_p, cfloat16_t *out_p, int shift, int length);
```

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/cycShiftFD/`
