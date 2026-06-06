---
kernel: nco_phase_ramp
precision: [single]
status: sim_verified

inputs:
  - name: ctrl
    shape: scalar
    dtype: struct
    description: "NCO control: phase increment (normalized frequency)"
outputs:
  - name: phase
    shape: [N]
    dtype: real_sfl
    description: "Output phase ramp in single precision"
parameters: {}

matlab_source:  []
c_source:       []
python_model:   framework/vspa_model/nco.py::r_nco
test_dir:       tests/nco_phase_ramp/
doc:
  - submodules/la931x_vspa_common/vspa-lib/nco_phase_ramp/doc/implementation_plan.pdf

depends_on: []

perf:
  cycles: 25
  au_config: vspa2_16au
  notes: "re-measured runsim 2026-05-27"
---

# nco_phase_ramp

> _TODO: fill in description._

## Algorithm

Generates a phase ramp for Numerically Controlled Oscillator (NCO) frequency shifting.

For each output sample k, computes:
```
phase[k] = phase_init + k * phase_increment
```

where `phase_increment` is the normalized frequency (typically in range [−π, π] or 
equivalent fixed-point encoding). The phase ramp is later used to modulate a complex 
sinusoid for frequency translation in mixing stages.

**Use case**: Correcting carrier frequency offset (CFO) in receivers by generating 
the conjugate phase ramp and multiplying received symbols.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/nco_phase_ramp/`
