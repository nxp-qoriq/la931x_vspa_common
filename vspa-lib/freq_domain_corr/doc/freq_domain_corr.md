---
kernel: freq_domain_corr
precision: [half_fixed]
status: sim_verified

inputs:
  - name: x
    shape: [N]
    dtype: complex_hfx
    description: "Complex frequency-domain input samples"
outputs:
  - name: y
    shape: [N]
    dtype: complex_hfx
    description: "Correlated output samples"
parameters: {}

matlab_source:  submodules/la931x_vspa_common/vspa-lib/freq_domain_corr/matlab/r_freq_domain_corr.m
c_source:       [submodules/la931x_vspa_common/vspa-lib/freq_domain_corr/src/freq_domain_corr.c]
python_model:   ""
test_dir:       tests/freq_domain_corr/
doc:
  - submodules/la931x_vspa_common/vspa-lib/freq_domain_corr/doc/Freq_Domain_Corr_Implementation_Plan.pdf
  - submodules/la931x_vspa_common/vspa-lib/freq_domain_corr/doc/Freq_Domain_Corr_Testplan.xlsx

depends_on: []

perf:
  cycles: 24
  au_config: vspa2_16au
  notes: "re-measured runsim 2026-05-27"
---

# freq_domain_corr

> _TODO: fill in description._

## Algorithm

Frequency-domain channel correction via two-stage pipeline:

1. **Complex Gain Application**: Each symbol is multiplied by a fixed complex gain 
   to equalize amplitude and phase distortions introduced by the channel.

2. **Phase Ramp Correction**: A linear phase ramp is applied to each symbol for 
   fractional timing offset correction. The ramp is defined by initial phase and 
   per-symbol phase increment, allowing fine tuning of sample timing without 
   resampling.

Combined effect: simultaneous channel gain equalization and fractional-sample 
timing correction in frequency domain.

## References

- NXP vspa-lib: `submodules/la931x_vspa_common/vspa-lib/freq_domain_corr/`
