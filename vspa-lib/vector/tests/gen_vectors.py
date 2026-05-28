#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate vectors for vector rhf_rhf_rhf_vAddSclr_asm test."""

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

from model import r_vaddsclr_half_fixed
from utils.hex_io import write_hex_u16

OUTDIR = _TESTS_DIR / 'vectors'
L = 2
N = L * 64


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument('--seed', type=int, default=42)
    args = p.parse_args()

    rng = np.random.default_rng(args.seed)
    x_f = rng.uniform(-0.8, 0.8, size=N)
    alpha_f = -0.125

    x_sm16, alpha_sm16, y_sm16 = r_vaddsclr_half_fixed(x_f, alpha_f)
    alpha_arr = np.array([0, alpha_sm16, 0], dtype=np.uint16)

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_hex_u16(x_sm16, str(OUTDIR / 'input.hex'))
    write_hex_u16(alpha_arr, str(OUTDIR / 'alpha.hex'))
    write_hex_u16(y_sm16, str(OUTDIR / 'ref.hex'))

    print(f'Generated vector vAddSclr vectors: L={L}, N={N}, seed={args.seed}')


if __name__ == '__main__':
    main()
