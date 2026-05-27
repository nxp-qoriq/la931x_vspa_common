#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate vectors for mini_dft_hfx_hfl_asm (N=96) using local python/model.py."""

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

from model import r_dft
from utils.hex_io import write_hex_u32
from vspa.arith import _f32_to_f16_trunc
from vspa.io import _float_to_sm16

OUTDIR = _TESTS_DIR / 'vectors'
N = 96


def pack_complex_sm16(c: np.ndarray) -> np.ndarray:
    re = _float_to_sm16(np.asarray(c).real).astype(np.uint32)
    im = _float_to_sm16(np.asarray(c).imag).astype(np.uint32)
    return (im << 16) | re


def pack_complex_half_trunc(c: np.ndarray) -> np.ndarray:
    re_f64 = np.asarray(c).real.astype(np.float64)
    im_f64 = np.asarray(c).imag.astype(np.float64)
    re_f16_bits = _f32_to_f16_trunc(re_f64.astype(np.float32))
    im_f16_bits = _f32_to_f16_trunc(im_f64.astype(np.float32))
    re_u16 = np.frombuffer(re_f16_bits.astype(np.float16).tobytes(), dtype=np.uint16).astype(np.uint32)
    im_u16 = np.frombuffer(im_f16_bits.astype(np.float16).tobytes(), dtype=np.uint16).astype(np.uint32)
    return (im_u16 << 16) | re_u16


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('--seed', type=int, default=42)
    args = parser.parse_args()

    rng = np.random.default_rng(args.seed)
    x = rng.uniform(-0.9, 0.9, N) + 1j * rng.uniform(-0.9, 0.9, N)

    y = r_dft(x, inp_prec='half_fixed', out_prec='half')

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_hex_u32(pack_complex_sm16(x), str(OUTDIR / 'input.hex'))
    write_hex_u32(pack_complex_half_trunc(y), str(OUTDIR / 'ref.hex'))

    print(f'Generated dft vectors: N={N}, seed={args.seed}')


if __name__ == '__main__':
    main()
