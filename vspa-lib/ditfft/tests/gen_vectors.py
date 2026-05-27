#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate vectors for fftDIT512_hfl using local python/model.py."""

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

from model import bit_rev_order, r_dit_fft
from utils.hex_io import write_hex_u32
from vspa.arith import r_half_flt
from vspa.io import _float_to_sm16

OUTDIR = _TESTS_DIR / 'vectors'
N = 512


def complex_to_packed_f16(c):
    c = np.asarray(c)
    re_bits = np.frombuffer(c.real.astype(np.float16).tobytes(), dtype=np.uint16).astype(np.uint32)
    im_bits = np.frombuffer(c.imag.astype(np.float16).tobytes(), dtype=np.uint16).astype(np.uint32)
    return (im_bits << 16) | re_bits


def complex_to_u32_sm16(c):
    c = np.asarray(c)
    re = _float_to_sm16(c.real).astype(np.uint32)
    im = _float_to_sm16(c.imag).astype(np.uint32)
    return (im << 16) | re


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('--seed', type=int, default=11)
    args = parser.parse_args()

    rng = np.random.default_rng(args.seed)
    x_raw = rng.uniform(-0.9, 0.9, N) + 1j * rng.uniform(-0.9, 0.9, N)
    x_f16 = r_half_flt(x_raw)

    y_ref = r_dit_fft(x_f16, inv_flag=0, prec_type='half_fixed', scale_out=1)

    br_idx = bit_rev_order(N)
    x_br = x_f16[br_idx]

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_hex_u32(complex_to_packed_f16(x_br), str(OUTDIR / 'input.hex'))
    write_hex_u32(complex_to_u32_sm16(y_ref), str(OUTDIR / 'ref.hex'))
    print(f'Generated ditfft vectors: N={N}, seed={args.seed}')


if __name__ == '__main__':
    main()
