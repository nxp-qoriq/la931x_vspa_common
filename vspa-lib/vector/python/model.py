# SPDX-License-Identifier: BSD-3-Clause
"""Python model for vector rhf_rhf_rhf_vAddSclr_asm."""

from __future__ import annotations

from pathlib import Path
import sys

import numpy as np

_COMMON = Path(__file__).resolve().parents[2] / 'common' / 'python'
if str(_COMMON) not in sys.path:
    sys.path.insert(0, str(_COMMON))

from vspa.io import _float_to_sm16, _sm16_to_float


def r_vaddsclr_half_fixed(x_f: np.ndarray, alpha_f: float):
    x_sm16 = _float_to_sm16(np.asarray(x_f, dtype=np.float64))
    alpha_sm16 = _float_to_sm16(np.array([alpha_f], dtype=np.float64))[0]
    y_f = _sm16_to_float(x_sm16) + _sm16_to_float(np.array([alpha_sm16], dtype=np.uint16))[0]
    y_sm16 = _float_to_sm16(y_f)
    return x_sm16, np.uint16(alpha_sm16), y_sm16
