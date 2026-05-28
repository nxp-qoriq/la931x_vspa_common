#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate vectors for utility mpy_f32_f32."""

from __future__ import annotations

from pathlib import Path
import sys

import numpy as np

_TESTS_DIR = Path(__file__).resolve().parent
_KERNEL_DIR = _TESTS_DIR.parent
_COMMON_PY = _KERNEL_DIR.parent / 'common' / 'python'

for p in (str(_COMMON_PY), str(_KERNEL_DIR / 'python')):
    if p not in sys.path:
        sys.path.insert(0, p)

from model import mpy_f32_f32_model
from utils.hex_io import write_hex_u32

OUTDIR = _TESTS_DIR / 'vectors'
CASES = 64
SEED = 42


def f32_to_u32(x: np.ndarray) -> np.ndarray:
    return np.asarray(x, dtype=np.float32).view(np.uint32)


def main() -> None:
    rng = np.random.default_rng(SEED)

    # Keep magnitudes moderate to avoid Inf/NaN corner cases in the first pass.
    a = rng.uniform(-8.0, 8.0, size=CASES).astype(np.float32)
    b = rng.uniform(-8.0, 8.0, size=CASES).astype(np.float32)
    ref = mpy_f32_f32_model(a, b)

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_hex_u32(f32_to_u32(a), str(OUTDIR / 'a.hex'))
    write_hex_u32(f32_to_u32(b), str(OUTDIR / 'b.hex'))
    write_hex_u32(f32_to_u32(ref), str(OUTDIR / 'ref.hex'))

    print(f'Generated utility mpy_f32_f32 vectors: cases={CASES}, seed={SEED}')


if __name__ == '__main__':
    main()
