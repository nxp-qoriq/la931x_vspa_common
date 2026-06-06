---
kernel: bitRev
precision: [half]
status: sim_verified

inputs:
  - name: pIn
    shape: [N]
    dtype: complex_hfl
    alignment_words: 32       # 32-bit aligned for bitRev64Invoke
    description: "Input buffer in VCPU DMEM, sub-carriers in bit-reversed order"

outputs:
  - name: pOut
    shape: [N]
    dtype: complex_hfl
    alignment_words: 1        # dmem aligned
    description: "Output buffer in VCPU DMEM, sub-carriers in linear order (after bit-reversal and DC shift)"

scratch: []

parameters:
  N:
    description: "Number of sub-carriers (FFT size)"
    valid_values: [64, 128, 256, 1024]
    default: 64

matlab_source:  submodules/la931x_vspa_common/vspa-lib/bitRev/matlab/r_bitRev1024.m
c_source:       [submodules/la931x_vspa_common/vspa-lib/bitRev/src/bitRev.c]
  - submodules/la931x_vspa_common/vspa-lib/bitRev/include/bitRev.h
python_model:   ""
test_dir:       tests/bitRev/
doc:
  - submodules/la931x_vspa_common/vspa-lib/bitRev/doc/bitRev64.pdf

depends_on: []

test_cases:
  - id: TC001
    params: {N: 64}
    notes: "64-point bit-reversal with DC shift via IPPU"

perf:
  target_efficiency: null
  cycles: 103
  au_config: vspa2_16au
  notes: "bitRev64Invoke PASS using MATLAB r_bitRev64 behavior (bitrevorder permutation + [33:64; 1:32] shift); IPPU microcode runs end-to-end in runsim.exe — canonical IPPU-in-cosim example; re-measured runsim 2026-05-27 (median of 3)"
---

# bitRev

> Bit-reversal and DC shift kernel for FFT outputs in OFDM and wireless signal processing.

## Algorithm

The bit-reversal kernel reorders sub-carriers from bit-reversed order (natural FFT output) to linear order, with a simultaneous DC shift that places the DC component at the center of the output. For N=64:

1. **Bit-reversal:** permute input[k] → output[bitrevorder(k)]
2. **DC shift:** cyclic shift [33:64; 1:32], moving DC from position 0 to position 33

This combination is typical in OFDM receivers after FFT, where the DC subcarrier must be centered for signal processing.

## Function API

```c
bool bitRev64Invoke(vspa_complex_float16 *pOut, vspa_complex_float16 const *pIn);
```

## Memory Requirements

| Buffer | Min size (words) | Alignment (words) | Allocated by |
|--------|-----------------|-------------------|---------------|
| Input  | 2N (64 samples) | 32                | Caller       |
| Output | 2N (64 samples) | 1                 | Caller       |

_Note: Input buffer must be 32-bit aligned; output is dmem-aligned. IPPU synchronization required._

## Input/Output Layout

| Port | Shape | Type            | Notes |
|------|-------|-----------------|-------|
| pIn  | [64]  | complex_float16 | Bit-reversed order |
| pOut | [64]  | complex_float16 | Linear order |

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/bitRev/`
- MATLAB reference: `r_bitRev64` in bitRev/matlab/
