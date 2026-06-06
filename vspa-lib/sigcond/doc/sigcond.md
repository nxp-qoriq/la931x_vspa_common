---
kernel: sigcond
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Complex input samples at 160 Msps in half-fixed precision"
  - name: bw
    shape: scalar
    dtype: int
    description: "Output bandwidth selector: 160, 80, 40, or 20 (Msps)"
  - name: decim_taps
    shape: "struct{h1:[L1], h2:[L2], h3:[L3]}"
    dtype: real_hfx
    description: "3-stage decimation FIR tap coefficients"
  - name: ipstruct
    shape: "struct"
    dtype: mixed
    description: "Configuration parameters: normfreq, normphase, fegain, dcoff, iqimb_*, etc."

outputs:
  - name: y_out
    shape: "[N/decimation_factor]"
    dtype: complex_hfx
    description: "Decimated complex output in half-fixed precision"
  - name: opstruct
    shape: struct
    dtype: mixed
    description: "Intermediate stage outputs for debugging"

parameters:
  N:
    description: "Number of input samples"
    valid_values: []
    default: null
  bw:
    description: "Output bandwidth (Msps)"
    valid_values: [160, 80, 40, 20]
    default: 160
  decimation_factor:
    description: "Decimation ratio = 160 / bw"
    valid_values: [1, 2, 4, 8]
    default: 1

matlab_source:  submodules/la931x_vspa_common/vspa-lib/sigcond/matlab/r_customsigcond2.m
c_source:       []
sx_source:      [submodules/la931x_vspa_common/vspa-lib/sigcond/src/sigcond.sx]
python_model:   ""
test_dir:       tests/sigcond/
doc:
  - submodules/la931x_vspa_common/vspa-lib/sigcond/doc/customsigcond_function_description.pdf
  - submodules/la931x_vspa_common/vspa-lib/sigcond/doc/customsigcond2_function_description.pdf

depends_on: []

perf:
  cycles: 598
  au_config: vspa2_16au
  notes: "160→20 Msps (8x decimation) configuration; re-measured runsim 2026-05-27"
---

# sigcond — Multi-Stage Signal Conditioning Chain

> Custom 160 Msps input signal conditioning with IQ imbalance compensation 
> and cascaded decimation down to 20, 40, 80, or 160 Msps.

## Algorithm

Multi-stage signal conditioning pipeline:

1. **IQ Imbalance Compensation**: Fractional-delay filtering to correct I/Q 
   channel imbalance (amplitude and phase errors).

2. **Mixer Stage** (BW ≤ 80 Msps): Complex multiply with NCO for frequency shift.

3. **Cascaded Decimation** (up to 3 stages):
   - Stage 1: 2x decimation (160→80 Msps)
   - Stage 2: 2x decimation (80→40 Msps)  
   - Stage 3: 2x decimation (40→20 Msps)

## Function API

```c
void r_customsigcond2(complex_hfx *x, int bw, struct decim_taps *taps, 
                      struct config *ipstruct, complex_hfx *y_out);
```

## Memory Requirements

| Buffer | Size | Notes |
|--------|------|-------|
| Input | N | Complex half-fixed samples |
| FIR taps | L1 + L2 + L3 | 3-stage tap coefficients |
| Output | N / factor | Decimated result |
| History | 2×(L1+L2+L3) | Streaming history buffers |

## Precision Modes

- **Fixed precision**: Half-fixed (Q15) throughout the chain
- **Config parameters**: Mixer frequency, phase, gain, and I/Q imbalance 
  factors supplied via ipstruct

## Known Constraints

- Input assumed to be 160 Msps complex stream
- Supported output rates: 160, 80, 40, 20 Msps (decimation: 1, 2, 4, 8)
- FIR tap lengths vary by stage; see implementation plan for details
- I/Q imbalance correction requires integer and fractional delay components

## References

- MATLAB oracle: `submodules/la931x_vspa_common/vspa-lib/sigcond/matlab/r_customsigcond2.m`
- VSPA assembly: `submodules/la931x_vspa_common/vspa-lib/sigcond/src/sigcond.sx`
- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/sigcond/`
