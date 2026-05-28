#!/usr/bin/env python3
"""Generate txiqcomp vectors by replaying NXP cwproj golden vectors."""

from __future__ import annotations

from pathlib import Path
import sys

_TESTS_DIR = Path(__file__).resolve().parent
_KERNEL_DIR = _TESTS_DIR.parent
sys.path.insert(0, str(_KERNEL_DIR / 'python'))

from model import read_words

CWPROJ_DIR = _KERNEL_DIR / 'txiqcomp_cwproj' / 'test_vectors'
OUTDIR = _TESTS_DIR / 'vectors'

INPUT_HEX = CWPROJ_DIR / 'generic_input_x_batch1.hex'
REF_HEX = CWPROJ_DIR / 'generic_output_y_batch1_ref.hex'
CFG_HEX = CWPROJ_DIR / 'generic_input_txiqcompstruct.hex'

N_SAMPLES = 512
N_CFG = 6


def write_c_include(words, path: Path):
    with path.open('w') as f:
        for v in words:
            f.write(f'0x{v:08X},\n')


def main() -> None:
    inp = read_words(INPUT_HEX)
    ref = read_words(REF_HEX)
    cfg = read_words(CFG_HEX)

    if len(inp) != N_SAMPLES:
        raise SystemExit(f'input.hex: expected {N_SAMPLES} words, got {len(inp)}')
    if len(ref) != N_SAMPLES:
        raise SystemExit(f'ref.hex: expected {N_SAMPLES} words, got {len(ref)}')
    if len(cfg) != N_CFG:
        raise SystemExit(f'cfg.hex: expected {N_CFG} words, got {len(cfg)}')

    OUTDIR.mkdir(parents=True, exist_ok=True)
    write_c_include(inp, OUTDIR / 'input.hex')
    write_c_include(ref, OUTDIR / 'ref.hex')
    write_c_include(cfg, OUTDIR / 'cfg.hex')

    print('Generated txiqcomp vectors from cwproj generic batch 1')


if __name__ == '__main__':
    main()
