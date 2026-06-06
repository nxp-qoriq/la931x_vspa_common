---
kernel: phase_compensation
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Complex input samples"
  - name: phase_shift
    shape: scalar
    dtype: real_hfx
    description: "Phase shift angle (radians or normalized)"
outputs:
  - name: y
    shape: [N]
    dtype: complex_hfx
    description: "Phase-compensated complex output"
parameters: {}

matlab_source:  []
c_source:       []
python_model:   ""
test_dir:       tests/phase_compensation/
doc:            []

depends_on: []

perf:
  cycles: 25
  au_config: vspa2_16au
  notes: "re-measured runsim 2026-05-27"
---

# phase_compensation

> _TODO: fill in description._

## Algorithm

Applies a constant phase rotation to each complex sample.

For each input symbol `x[k] = I[k] + j·Q[k]`, computes:
```
y[k] = x[k] · exp(j·phase_shift)
```

Implemented efficiently as a single complex multiplication using two real 
multiplications and two additions (via pre-computed cos/sin table or direct arithmetic).

**Use case**: Correcting residual phase offset after frequency synchronization 
or applying known phase references in pilot-based channel estimation.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/phase_compensation/`
