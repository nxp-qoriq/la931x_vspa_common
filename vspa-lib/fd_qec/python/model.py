# SPDX-License-Identifier: BSD-3-Clause
"""Python model for fd_qec kernel."""

from __future__ import annotations

from pathlib import Path
import sys

import numpy as np

_COMMON = Path(__file__).resolve().parents[2] / 'common' / 'python'
if str(_COMMON) not in sys.path:
    sys.path.insert(0, str(_COMMON))

from vspa.arith import r_smad
from vspa.io import _float_to_sm16, _sm16_to_float


def quantise_complex_sm16(c):
    """Round-trip complex values through SM16 to match DMEM quantization."""
    c = np.asarray(c, dtype=np.complex128)
    re = _sm16_to_float(_float_to_sm16(c.real))
    im = _sm16_to_float(_float_to_sm16(c.imag))
    return re + 1j * im


def r_fd_qec(x, x_mirror, weights_a, weights_b):
    """Bit-exact fd_qec chain: y = r_smad(x_mirror,b,r_smad(x,a,0))."""
    y_tmp = r_smad(x, weights_a, np.zeros(len(x), dtype=np.complex128))
    return r_smad(x_mirror, weights_b, y_tmp)
