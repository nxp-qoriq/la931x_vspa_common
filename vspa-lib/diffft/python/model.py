# SPDX-License-Identifier: BSD-3-Clause
"""Helpers for diffft test-vector replay.

The migrated diffft test currently replays NXP cwproj golden vectors.
"""

from __future__ import annotations

from pathlib import Path


def cwproj_vector_dir() -> Path:
    """Return canonical cwproj test-vector directory for diffft."""
    return Path(__file__).resolve().parents[1] / 'diffft_cwproj' / 'test_vectors'
