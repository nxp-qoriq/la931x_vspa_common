#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate vectors for vec_mult_64chp using local python/model.py."""

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

from model import r_freq_domain_corr
from utils.hex_io import write_hex_u32
from vspa.arith import r_half_flt

OUTDIR = _TESTS_DIR / 'vectors'
N = 64
PHASE_RAMP = 59
PHASE_INIT = int(np.array([-122], dtype=np.int32).view(np.uint32)[0])


def complex_f16_to_packed_u32(c):
    c = np.asarray(c)
    re_bits = np.frombuffer(c.real.astype(np.float16).tobytes(), dtype=np.uint16).astype(np.uint32)
    im_bits = np.frombuffer(c.imag.astype(np.float16).tobytes(), dtype=np.uint16).astype(np.uint32)
    return (im_bits << 16) | re_bits


def main() -> None:
    rng = np.random.default_rng(42)
    vec = r_half_flt(rng.uniform(-0.7, 0.7, N) + 1j * rng.uniform(-0.7, 0.7, N))
    gain = complex(np.float32(rng.uniform(-0.7, 0.7)), np.float32(rng.uniform(-0.7, 0.7)))

    corr, out = r_freq_domain_corr(vec, gain, PHASE_RAMP, PHASE_INIT)

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_hex_u32(complex_f16_to_packed_u32(vec), str(OUTDIR / 'input.hex'))
    write_hex_u32(complex_f16_to_packed_u32(corr), str(OUTDIR / 'corr.hex'))
    write_hex_u32(complex_f16_to_packed_u32(out), str(OUTDIR / 'ref.hex'))

    print('Generated freq_domain_corr vectors')


if __name__ == '__main__':
    main()
