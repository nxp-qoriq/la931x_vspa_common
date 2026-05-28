#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate deterministic crc8 vectors."""

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

from model import crc8_encode
from utils.hex_io import write_hex_u32

OUTDIR = _TESTS_DIR / 'vectors'
CASES = 64
SEED = 42
VALID_SIZES = np.array([20, 21, 23, 34], dtype=np.uint32)


def main() -> None:
    rng = np.random.default_rng(SEED)

    data1 = rng.integers(0, 1 << 32, size=CASES, dtype=np.uint32)
    data2 = np.zeros(CASES, dtype=np.uint32)
    size = rng.choice(VALID_SIZES, size=CASES, replace=True).astype(np.uint32)

    # For size==34, only bit[1:0] are consumed by the C kernel.
    mask34 = size == 34
    data2[mask34] = rng.integers(0, 4, size=int(mask34.sum()), dtype=np.uint32)

    ref = np.zeros(CASES, dtype=np.uint32)
    for i in range(CASES):
        ref[i] = np.uint32(crc8_encode(int(data1[i]), int(data2[i]), int(size[i])))

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_hex_u32(data1, str(OUTDIR / 'data1.hex'))
    write_hex_u32(data2, str(OUTDIR / 'data2.hex'))
    write_hex_u32(size, str(OUTDIR / 'size.hex'))
    write_hex_u32(ref, str(OUTDIR / 'ref.hex'))

    print(f'Generated crc8 vectors: cases={CASES}, seed={SEED}')


if __name__ == '__main__':
    main()
