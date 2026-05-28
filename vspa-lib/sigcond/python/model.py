# SPDX-License-Identifier: BSD-3-Clause
"""Minimal Python helper for sigcond smoke tests."""

from __future__ import annotations

from pathlib import Path


def ensure_smoke_sentinel(tests_dir: Path) -> Path:
    vectors = tests_dir / 'vectors'
    vectors.mkdir(parents=True, exist_ok=True)
    sentinel = vectors / 'input.hex'
    sentinel.write_text('// no external vectors needed for the zero-input smoke test\n')
    return sentinel
