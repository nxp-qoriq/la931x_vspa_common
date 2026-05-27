# SPDX-License-Identifier: BSD-3-Clause
"""Python model for fir64 kernel."""

from __future__ import annotations

from pathlib import Path
import sys

import numpy as np

_COMMON = Path(__file__).resolve().parents[2] / 'common' / 'python'
if str(_COMMON) not in sys.path:
    sys.path.insert(0, str(_COMMON))

from vspa.arith import r_convert, r_smad


def r_firFilter64(h, x, inp_hist=None,
                  input_prec='half_fixed', filter_prec='single',
                  output_prec='half_fixed'):
    """Bit-exact FIR filter model for fir64 kernel."""
    h = np.asarray(h, dtype=np.float64).reshape(-1)
    x = np.asarray(x, dtype=np.complex128).reshape(-1)
    k_len = len(h)
    n_len = len(x)
    hist_len = k_len - 1

    if inp_hist is None:
        inp_hist = np.zeros(hist_len, dtype=np.complex128)
    inp_hist = np.asarray(inp_hist, dtype=np.complex128).reshape(-1)
    if len(inp_hist) != hist_len:
        raise ValueError(f'r_firFilter64: inp_hist must have {hist_len} samples, got {len(inp_hist)}')

    h_q = r_convert(h, filter_prec)
    x_q = r_convert(x, input_prec)
    hist_q = r_convert(inp_hist, input_prec)

    x_ext = np.concatenate([hist_q, x_q])

    acc = np.zeros(n_len, dtype=np.complex128)
    for k in range(k_len - 1, -1, -1):
        start = (k_len - 1) - k
        acc = r_smad(h_q[k], x_ext[start:start + n_len], acc)

    re_q = r_convert(acc.real, output_prec)
    im_q = r_convert(acc.imag, output_prec)
    y_out = np.empty(n_len, dtype=np.complex128)
    y_out.real = re_q
    y_out.imag = im_q

    next_hist = x_q[-hist_len:] if hist_len > 0 else np.empty(0, dtype=np.complex128)
    return y_out, next_hist
