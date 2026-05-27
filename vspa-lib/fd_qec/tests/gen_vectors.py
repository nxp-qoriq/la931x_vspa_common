#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate vectors for fd_qec using local python/model.py."""

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

from model import quantise_complex_sm16, r_fd_qec
from utils.hex_io import write_hex_u16
from vspa.io import _float_to_sm16

OUTDIR = _TESTS_DIR / 'vectors'
BUF_LEN = 128


def interleave_sm16(c):
    re_ = _float_to_sm16(np.asarray(c).real)
    im_ = _float_to_sm16(np.asarray(c).imag)
    out = np.empty(2 * len(re_), dtype=np.uint16)
    out[0::2] = re_
    out[1::2] = im_
    return out


def main() -> None:
    rng = np.random.default_rng(42)

    def rand_cplx():
        return rng.uniform(-0.7, 0.7, BUF_LEN) + 1j * rng.uniform(-0.7, 0.7, BUF_LEN)

    x = quantise_complex_sm16(rand_cplx())
    x_mir = quantise_complex_sm16(x[::-1])
    weights_a = quantise_complex_sm16(rand_cplx())
    weights_b = quantise_complex_sm16(rand_cplx())

    y = r_fd_qec(x, x_mir, weights_a, weights_b)

    OUTDIR.mkdir(parents=True, exist_ok=True)
    blob = np.concatenate([x, x_mir, weights_a, weights_b])
    write_hex_u16(interleave_sm16(blob), str(OUTDIR / 'input.hex'))
    write_hex_u16(interleave_sm16(y), str(OUTDIR / 'ref.hex'))

    print(f'Generated fd_qec vectors (BUF_LEN={BUF_LEN})')


if __name__ == '__main__':
    main()
