#!/usr/bin/env python3
"""sigcond smoke-test vector generator (no-op sentinel)."""

from __future__ import annotations

from pathlib import Path
import sys

_TESTS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(_TESTS_DIR.parent / 'python'))

from model import ensure_smoke_sentinel


def main() -> None:
    ensure_smoke_sentinel(_TESTS_DIR)
    print('sigcond: smoke test (no external vectors required)')


if __name__ == '__main__':
    main()
