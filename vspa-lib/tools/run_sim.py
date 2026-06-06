#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Standalone VSPA ELF runner for submodule tests.

Usage:
    python3 run_sim.py <elf_path> <au_config> [--cycles]

Arguments:
    elf_path   — path to .elf file (absolute or relative to CWD)
    au_config  — device string, e.g. vspa2_16au, vspa2_32au
    --cycles   — pass -showpc to runsim and report cycle count (slow)

Resolves runsim in order:
    1. SIMULATOR_PATH environment variable
    2. /opt/ccssim2/bin/runsim                     (container symlink)
    3. <repo_root>/environment/ccssim2/bin/runsim  (repo-relative, 4 parents up)

Exits 0 on PASS, 1 on FAIL or no PASS/FAIL line.
"""
from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path


def _find_runsim() -> str:
    """Resolve the native Linux runsim binary."""
    # 1. SIMULATOR_PATH env var
    sp = os.environ.get("SIMULATOR_PATH", "")
    if sp and not sp.endswith(".exe") and Path(sp).exists():
        return sp

    # 2. /opt fallback (container symlink)
    if Path("/opt/ccssim2/bin/runsim").exists():
        return "/opt/ccssim2/bin/runsim"

    # 3. Repo-relative: tools/ -> vspa-lib/ -> la931x_vspa_common/ -> submodules/ -> repo_root
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent.parent.parent.parent
    local = repo_root / "environment" / "ccssim2" / "bin" / "runsim"
    if local.exists():
        return str(local)

    return "/opt/ccssim2/bin/runsim"


def main() -> int:
    parser = argparse.ArgumentParser(description="Run a VSPA ELF and report PASS/FAIL.")
    parser.add_argument("elf_path", help="Path to .elf file")
    parser.add_argument("au_config", help="Device string, e.g. vspa2_16au")
    parser.add_argument("--cycles", action="store_true",
                        help="Pass -showpc to runsim and report cycle count (slow)")
    args = parser.parse_args()

    elf_path = Path(args.elf_path).resolve()
    au_config = args.au_config

    if not elf_path.exists():
        print(f"ERROR: ELF not found: {elf_path}", file=sys.stderr)
        return 2

    runsim = _find_runsim()
    elf_dir = str(elf_path.parent)
    elf_name = elf_path.name
    showpc_flag = "-showpc" if args.cycles else ""

    print(f"  ELF    : {elf_name}")
    print(f"  Target : {au_config}")
    print(f"  Runner : {runsim}")

    sim_cmd = f"cd {elf_dir} && {runsim} -d {au_config} {showpc_flag} {elf_name} 2>/dev/null"
    try:
        result = subprocess.run(
            ["bash", "-c", sim_cmd.strip()],
            capture_output=True,
            text=True,
            timeout=150,
        )
    except subprocess.TimeoutExpired:
        print("RESULT : ERROR (timeout)")
        return 1

    _PC_RE = re.compile(r"^PC:\s*\d+")
    program_lines: list[str] = []
    total_cycles: int | None = None   # whole-program (showpc CYC: trace)
    kernel_cycles: int | None = None  # kernel body (KERNEL_CYCLES: printf)

    for line in result.stdout.splitlines():
        if _PC_RE.match(line):
            m = re.search(r'CYC:\s*(\d+)', line)
            if m:
                total_cycles = int(m.group(1))
        else:
            stripped = line.strip()
            if stripped:
                program_lines.append(stripped)
                km = re.search(r'KERNEL_CYCLES:\s*(-?\d+)', stripped)
                if km:
                    kernel_cycles = int(km.group(1))

    # Prefer the honest kernel-body count (KCYC_* macros) over the
    # whole-program total. The kernel count is available without -showpc.
    cycles = kernel_cycles if kernel_cycles is not None else total_cycles


    passed: bool | None = None
    for line in program_lines:
        upper = line.upper()
        if "PASS" in upper and "FAIL" not in upper:
            passed = True
        elif "FAIL" in upper:
            passed = False
            break

    print("=" * 60)
    if passed is True:
        cycle_str = f"  ({cycles} cycles)" if cycles is not None else ""
        print(f"RESULT : PASS ✓{cycle_str}")
        return 0

    if passed is False:
        print("RESULT : FAIL ✗")
        for line in program_lines:
            print(f"  {line}")
        return 1

    print("RESULT : ERROR (no PASS/FAIL line in output)")
    for line in program_lines:
        print(f"  {line}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
