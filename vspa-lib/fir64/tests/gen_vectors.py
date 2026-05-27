#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate test vectors for fir64 using local python/model.py."""

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

from model import r_firFilter64
from utils.hex_io import write_hex_u32
from vspa.io import _float_to_sm16

OUTDIR = _TESTS_DIR / 'vectors'
NUM_INPUT_SAMPLES = 128
NUM_FIR_TAPS = 63


def design_lowpass_taps(num_taps: int, cutoff_norm: float = 0.25) -> np.ndarray:
    if num_taps % 2 == 0:
        raise ValueError('design_lowpass_taps requires odd num_taps')
    n = np.arange(num_taps) - (num_taps - 1) / 2.0
    h = cutoff_norm * np.sinc(cutoff_norm * n)
    w = 0.54 - 0.46 * np.cos(2 * np.pi * np.arange(num_taps) / (num_taps - 1))
    h = h * w
    h = h / np.sum(h)
    return h.astype(np.float64)


def complex_to_u32_sm16(c):
    c = np.asarray(c)
    re = _float_to_sm16(c.real).astype(np.uint32)
    im = _float_to_sm16(c.imag).astype(np.uint32)
    return (im << 16) | re


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument('--seed', type=int, default=42)
    p.add_argument('--mode', choices=('random', 'impulse', 'dcgain'), default='random')
    p.add_argument('--cutoff', type=float, default=0.25)
    args = p.parse_args()

    rng = np.random.default_rng(args.seed)
    taps_f32 = design_lowpass_taps(NUM_FIR_TAPS, cutoff_norm=args.cutoff).astype(np.float32).astype(np.float64)

    if args.mode == 'random':
        x = rng.uniform(-0.7, 0.7, NUM_INPUT_SAMPLES) + 1j * rng.uniform(-0.7, 0.7, NUM_INPUT_SAMPLES)
    elif args.mode == 'impulse':
        x = np.zeros(NUM_INPUT_SAMPLES, dtype=np.complex128)
        x[0] = 0.5 + 0.25j
    else:
        x = np.full(NUM_INPUT_SAMPLES, 0.5 - 0.25j, dtype=np.complex128)

    y_ref, _ = r_firFilter64(
        taps_f32, x,
        inp_hist=None,
        input_prec='half_fixed', filter_prec='single', output_prec='half_fixed')

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_hex_u32(complex_to_u32_sm16(x), str(OUTDIR / 'input.hex'))
    write_hex_u32(complex_to_u32_sm16(y_ref), str(OUTDIR / 'ref.hex'))
    write_hex_u32(np.frombuffer(np.asarray(taps_f32, dtype=np.float32).tobytes(), dtype='<u4').copy(), str(OUTDIR / 'taps.hex'))

    print(f'Generated fir64 vectors (mode={args.mode}, seed={args.seed})')


if __name__ == '__main__':
    main()
