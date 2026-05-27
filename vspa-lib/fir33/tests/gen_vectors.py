#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate vectors for fir33 using local python/model.py."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

_TESTS_DIR = Path(__file__).resolve().parent
_KERNEL_DIR = _TESTS_DIR.parent
_COMMON_PY = _KERNEL_DIR.parent / 'common' / 'python'
for p in (str(_COMMON_PY), str(_KERNEL_DIR / 'python')):
    if p not in sys.path:
        sys.path.insert(0, p)

from model import r_firFilter
from utils.hex_io import write_hex_u16, write_hex_u32
from vspa.io import _float_to_sm16

OUTDIR = _TESTS_DIR / 'vectors'
M = 2048
L = 33
L_PAD = 34


def complex_to_u32_sm16(c):
    c = np.asarray(c)
    re = _float_to_sm16(c.real).astype(np.uint32)
    im = _float_to_sm16(c.imag).astype(np.uint32)
    return (im << 16) | re


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('--seed', type=int, default=42)
    args = parser.parse_args()

    rng = np.random.default_rng(args.seed)
    h_raw = rng.uniform(-0.9, 0.9, L)
    x_raw = rng.uniform(-0.9, 0.9, M + L - 1) + 1j * rng.uniform(-0.9, 0.9, M + L - 1)

    y_ref = r_firFilter(h_raw, x_raw, prec='half_fixed')

    h_rev_sm16 = _float_to_sm16(np.asarray(h_raw, dtype=np.float64)[::-1])
    taps_u16 = np.zeros(L_PAD, dtype=np.uint16)
    taps_u16[:L] = h_rev_sm16

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_hex_u16(taps_u16, str(OUTDIR / 'taps.hex'))
    write_hex_u32(complex_to_u32_sm16(x_raw), str(OUTDIR / 'x_input.hex'))
    write_hex_u32(complex_to_u32_sm16(y_ref), str(OUTDIR / 'y_ref.hex'))

    print(f'Generated fir33 vectors (seed={args.seed})')


if __name__ == '__main__':
    main()
