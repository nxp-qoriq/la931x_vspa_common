# SPDX-License-Identifier: BSD-3-Clause
"""Helpers for txiqcomp cwproj-vector replay."""

from __future__ import annotations

from pathlib import Path


def read_words(path: Path):
    out = []
    for line in path.read_text().splitlines():
        tok = line.strip().rstrip(',').strip()
        if not tok:
            continue
        out.append(int(tok, 16) & 0xFFFFFFFF)
    return out
