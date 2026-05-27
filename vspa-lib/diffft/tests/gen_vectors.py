#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Generate diffft vectors by replaying NXP cwproj golden vector set."""

from __future__ import annotations

from pathlib import Path
import sys

_TESTS_DIR = Path(__file__).resolve().parent
_KERNEL_DIR = _TESTS_DIR.parent
if str(_KERNEL_DIR / 'python') not in sys.path:
    sys.path.insert(0, str(_KERNEL_DIR / 'python'))

from model import cwproj_vector_dir

OUTDIR = _TESTS_DIR / 'vectors'
CWPROJ_DIR = cwproj_vector_dir()

INPUT_HEX = CWPROJ_DIR / 'fft512_type1_input_x.hex'
REF_HEX = CWPROJ_DIR / 'fft512_type1_output_y_ref.hex'
CFG_HEX = CWPROJ_DIR / 'fft512_type1_input_config.hex'

N = 512
CBUFF_SIZE = 4096 + 32
INPUT_OFFSET = 4096 + 32 - 65


def read_words(path: Path):
    out = []
    with path.open() as f:
        for line in f:
            line = line.strip().rstrip(',').strip()
            if line:
                out.append(int(line, 16) & 0xFFFFFFFF)
    return out


def write_c_include(words, path: Path):
    with path.open('w') as f:
        for v in words:
            f.write(f'0x{v:08X},\n')


def main() -> None:
    cbuf = read_words(INPUT_HEX)
    ref = read_words(REF_HEX)
    cfg = read_words(CFG_HEX)

    if len(cbuf) != CBUFF_SIZE:
        raise SystemExit(f'input_x.hex: expected {CBUFF_SIZE} words, got {len(cbuf)}')
    if len(ref) != N:
        raise SystemExit(f'output_y_ref.hex: expected {N} words, got {len(ref)}')
    if len(cfg) != 4:
        raise SystemExit(f'input_config.hex: expected 4 words, got {len(cfg)}')

    ncfg, inv, prec, off = cfg
    if ncfg != N or inv != 0 or prec != 1 or off != INPUT_OFFSET:
        raise SystemExit('config mismatch for fft512_type1 golden vectors')

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_c_include(cbuf, OUTDIR / 'input.hex')
    write_c_include(ref, OUTDIR / 'ref.hex')

    print('Generated diffft vectors from cwproj fft512_type1 (hfx_hfx, scaleout=1)')


if __name__ == '__main__':
    main()
