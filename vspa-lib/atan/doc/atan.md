---
kernel: atan
precision: [half_fixed, single]
status: sim_verified

inputs:
  - name: inp
    shape: [N]
    dtype: complex_hfx        # or complex_sfl depending on inp_prec
    description: "Complex input vector; precision set via ctrl.inp_prec"

outputs:
  - name: phase
    shape: [N]
    dtype: real_hfx            # or real_sfl depending on out_prec
    description: "Full-circle phase angle per element (radians or normalised to π)"

parameters:
  N:
    description: "Number of complex input samples"
    valid_values: []           # arbitrary; must be a multiple of VSPA vector width
    default: null
  inp_prec:
    description: "Input precision (half_fixed or single)"
    valid_values: [half_fixed, single]
    default: half_fixed
  coeff_prec:
    description: "Coefficient precision for polynomial approximation"
    valid_values: [half_fixed, single]
    default: half_fixed
  out_prec:
    description: "Output precision"
    valid_values: [half_fixed, single]
    default: half_fixed
  num_coeff:
    description: "Number of non-zero polynomial coefficients (min 3)"
    valid_values: []
    default: 4
  norm:
    description: "false → radians; true → normalised to π"
    valid_values: [true, false]
    default: true

matlab_source:  submodules/la931x_vspa_common/vspa-lib/atan/matlab/r_atan2.m
c_source:       []             # assembly only — see vspa-lib atan/src/
python_model:   framework/vspa_model/trig.py::r_atan2
test_dir:       tests/atan/
doc:
  - submodules/la931x_vspa_common/vspa-lib/atan/doc/Atan_Implementation_Plan.pdf
  - submodules/la931x_vspa_common/vspa-lib/atan/doc/Atan_Testplan.xlsx

depends_on: []

perf:
  target_efficiency: null
  cycles: 69
  au_config: vspa2_16au
  notes: "TC001, default config, vspa2_16au; 56 NXP test cases bit-exact; re-measured runsim 2026-05-27 (median of 3)"
---

# atan — Full-Circle Phase Extraction (atan2)

> Computes the phase angle of each element of a complex input vector,
> using a polynomial approximation matched to VSPA fixed-point arithmetic.

## Algorithm

For each complex sample `z = re + j·im` the kernel evaluates
`atan2(im, re)` over the full [−π, π] circle.  A piecewise polynomial
approximation (degree determined by `num_coeff`) is used so that no
division instruction is required — only multiplications and additions on
the VSPA half-fixed multiplier units.

When `norm=true` the output is scaled by 1/π, mapping the range to [−1, 1].

## Input/Output Layout

| Port  | Shape | Type | Notes |
|-------|-------|------|-------|
| inp   | [N]   | complex_hfx | interleaved Re/Im |
| phase | [N]   | real_hfx    | one angle per input sample |

## Precision Modes

| Mode | Description |
|------|-------------|
| `half_fixed` | 16-bit fixed-point I/O and coefficients |
| `single`     | 32-bit float I/O and coefficients |

## Known Constraints

- N must be a multiple of the VSPA vector width (16 for vspa2_16au).
- Minimum `num_coeff` is 3; values above 8 give diminishing accuracy returns.
- Coefficient tables are precomputed offline and stored as `.dat` files in
  the vspa-lib submodule.

## References

- MATLAB oracle: `submodules/la931x_vspa_common/vspa-lib/atan/matlab/r_atan2.m`
- VSPA python model: `tests/framework/vspa_model.py::r_atan2`
- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/atan/`
