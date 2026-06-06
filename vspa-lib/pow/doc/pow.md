---
kernel: pow
precision: [half_fixed, single]
status: sim_verified

inputs:
  - name: sample_in
    shape: "[nL, 32]"
    dtype: complex_hfx
    description: "Complex samples in half-fixed format; nL lines of 32 samples each"

outputs:
  - name: acc_inout
    shape: "[32]"
    dtype: real_sfl
    description: "32 accumulated power values in single-precision float"

parameters:
  nL:
    description: "Number of input sample lines"
    valid_values: []
    default: null

matlab_source:  []
c_source:       []
sx_source:      [submodules/la931x_vspa_common/vspa-lib/pow/src/pow_acc_asm.sx]
python_model:   ""
test_dir:       tests/pow/
doc:            []

depends_on: []

perf:
  cycles: 27
  au_config: vspa2_16au
  notes: "pow_acc_asm; assembly-only implementation; re-measured runsim 2026-05-27"
---

# pow — Power (Magnitude-Squared) Accumulation

> Computes magnitude-squared (power) of complex input samples and accumulates 
> results into a 32-element floating-point vector.

## Algorithm

Implements fixed-to-float power accumulation pipeline:
1. Read complex half-fixed input samples in groups
2. Compute magnitude-squared per sample via multiply-accumulate
3. Accumulate 32 power values in single-precision float

Supports batch processing via `pow_sum_asm` to sum the 32 accumulated 
power values into a single output.

## Function API

```c
void pow_acc_asm(void *sample_in, void *acc_inout, unsigned int nL);
float pow_sum_asm(void *acc_in, unsigned int num_points);
```

## Memory Requirements

| Buffer | Size | Notes |
|--------|------|-------|
| Input | nL × 32 words | Complex half-fixed (interleaved Re/Im) |
| Accumulator | 32 words | Float; initialized to zero on first call |

## Known Constraints

- Accumulator must be initialized to zero before first call
- Output is 32-element vector of float (single-precision)
- Input must be half-fixed complex in interleaved format

## References

- VSPA assembly: `submodules/la931x_vspa_common/vspa-lib/pow/src/pow_acc_asm.sx`
- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/pow/`
