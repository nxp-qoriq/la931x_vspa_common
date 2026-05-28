# SPDX-License-Identifier: BSD-3-Clause
"""Python model for selected utility kernels."""

from __future__ import annotations

from pathlib import Path
import sys

import numpy as np

_COMMON = Path(__file__).resolve().parents[2] / 'common' / 'python'
if str(_COMMON) not in sys.path:
    sys.path.insert(0, str(_COMMON))

from vspa.arith import r_smad


def mpy_f32_f32_model(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    a64 = np.asarray(a, dtype=np.float64)
    b64 = np.asarray(b, dtype=np.float64)
    return r_smad(a64, b64, np.zeros_like(a64)).astype(np.float64)
