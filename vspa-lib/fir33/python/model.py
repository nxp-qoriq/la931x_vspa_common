# SPDX-License-Identifier: BSD-3-Clause
"""Python model for fir33 kernel."""

from __future__ import annotations

from pathlib import Path
import sys

import numpy as np

_COMMON = Path(__file__).resolve().parents[2] / 'common' / 'python'
if str(_COMMON) not in sys.path:
    sys.path.insert(0, str(_COMMON))

from vspa.arith import r_half, r_half_flt, r_smac


def r_firFilter(h, x, prec='half_fixed'):
    """Bit-exact FIR filter model for fir33."""
    h = np.asarray(h, dtype=np.float64)
    x = np.asarray(x, dtype=np.complex128)
    k_len = len(h)
    n_len = len(x) - k_len + 1
    if n_len <= 0:
        raise ValueError(f'Input too short: len(x)={len(x)} < K={k_len}')

    s1 = np.zeros((k_len, n_len), dtype=np.complex128)
    for k in range(k_len):
        s1[k, :] = x[k:n_len + k]

    h_rev = h[k_len - 1::-1]
    s0 = np.tile(h_rev.reshape(-1, 1), (1, n_len))

    if prec == 'half':
        yr = r_half_flt(r_smac(r_half_flt(s0), r_half_flt(s1), np.zeros(n_len, dtype=np.complex128)))
    else:
        yr = r_half(r_smac(r_half(s0), r_half(s1), np.zeros(n_len, dtype=np.complex128)))

    return yr
