---
kernel: utility
precision: [mixed]
status: sim_verified
inputs: []
outputs: []
scratch: []
parameters: {}
matlab_source: ""
c_source:
  - "submodules/la931x_vspa_common/vspa-lib/utility/src/"
sx_source: []
src: ""
python_model: ""
test_dir: ""
doc: []
etsi_refs: []
depends_on: []
test_cases: []
perf:
  target_efficiency: null
  cycles: 2880
  c_cycles: null
  sx_cycles: null
  au_config: vspa2_16au
  notes: "Container for utility primitives; define per-function specs as needed."
---

# utility

> Collection of utility primitives used by higher-level kernels.

## Algorithm

This folder groups helper primitives and does not currently map to one unique kernel algorithm.

## Function API

No single canonical API at folder level.

## Memory Requirements

Function-dependent; define in per-function specs when split out.

## Input/Output Layout

Function-dependent; define in per-function specs when split out.

## Precision Modes

Mixed, function-dependent.

## Test Cases

No standalone folder-level test cases yet.

## Known Constraints

- Intended as a shared utility area; migration may split this into dedicated kernel specs.

## References

- Sources: submodules/la931x_vspa_common/vspa-lib/utility/src/
