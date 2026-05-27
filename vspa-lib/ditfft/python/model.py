# SPDX-License-Identifier: BSD-3-Clause
"""Python model for ditfft kernels."""

from __future__ import annotations

from pathlib import Path
import sys

import numpy as np

_COMMON = Path(__file__).resolve().parents[2] / 'common' / 'python'
if str(_COMMON) not in sys.path:
    sys.path.insert(0, str(_COMMON))

from vspa.arith import r_half, r_single, r_smad

_NCO_M = 8
_NCO_Q = 17


def _build_nco_tables():
    m, q = _NCO_M, _NCO_Q
    n_pts = 2 ** m + 1
    x1 = np.arange(n_pts, dtype=np.float64) / 2 ** m
    cos_v = np.cos(np.pi * x1 / 4)
    sin_v = np.sin(np.pi * x1 / 4)
    y_cos = np.round(cos_v / 2.0 ** (-q)).astype(np.int64)
    y_sin = np.round(sin_v / 2.0 ** (-q)).astype(np.int64)
    cos_ext = np.append(cos_v, np.sqrt(2) / 2.0)
    sin_ext = np.append(sin_v, np.sqrt(2) / 2.0)
    m_cos = np.round(np.diff(cos_ext) / 2.0 ** (-q)).astype(np.int64)
    m_sin = np.round(np.diff(sin_ext) / 2.0 ** (-q)).astype(np.int64)
    return y_cos, y_sin, m_cos, m_sin


_NCO_Y_COS, _NCO_Y_SIN, _NCO_M_COS, _NCO_M_SIN = _build_nco_tables()


def r_nco(f, length, phase_count=0):
    """Bit-exact VSPA NCO model."""
    m, q = _NCO_M, _NCO_Q

    f_v = np.atleast_1d(np.asarray(f, dtype=np.float64)).ravel()
    f_mapped = f_v % 1.0
    n_f = len(f_v)

    delta_q = np.floor(f_mapped * 2 ** 32).astype(np.int64)

    pc = np.atleast_1d(np.asarray(phase_count, dtype=np.int64)).ravel()
    if len(pc) == 1:
        pc = np.tile(pc, n_f)
    mask32 = np.int64(0xFFFFFFFF)
    dq_lo = delta_q & np.int64(0x0000FFFF)
    dq_hi = delta_q & np.int64(0xFFFF0000)
    pc_lo = pc & np.int64(0x0000FFFF)
    pc_hi = pc & np.int64(0xFFFF0000)
    del_phase = (dq_lo * pc_hi + dq_hi * pc_lo + dq_lo * pc_lo) & mask32

    k = np.arange(length, dtype=np.int64)
    raw = del_phase[:, None].astype(np.float64) + delta_q[:, None].astype(np.float64) * k[None, :]
    x = (raw / float(2 ** 32)) % 1.0

    val = np.round(x * float(2 ** (q + 3))).astype(np.int64)
    oct_idx = (val >> q) & np.int64(7)
    phase_lsb = val & np.int64((1 << q) - 1)
    phase_refl = np.int64(1 << q) - phase_lsb

    def _lut(y_lut, m_lut, ph):
        idx = ph >> (q - m)
        frac = ph & np.int64((1 << (q - m)) - 1)
        return (np.int64(2 ** (q - m)) * y_lut[idx] + m_lut[idx] * frac) / float(2 ** (2 * q - m))

    cos_d = _lut(_NCO_Y_COS, _NCO_M_COS, phase_lsb)
    sin_d = _lut(_NCO_Y_SIN, _NCO_M_SIN, phase_lsb)
    cos_r = _lut(_NCO_Y_COS, _NCO_M_COS, phase_refl)
    sin_r = _lut(_NCO_Y_SIN, _NCO_M_SIN, phase_refl)

    o = oct_idx
    i_arr = np.select(
        [o == 0, o == 1, o == 2, o == 3, o == 4, o == 5, o == 6, o == 7],
        [cos_d, sin_r, -sin_d, -cos_r, -cos_d, -sin_r, sin_d, cos_r],
    )
    q_arr = np.select(
        [o == 0, o == 1, o == 2, o == 3, o == 4, o == 5, o == 6, o == 7],
        [sin_d, cos_r, cos_d, sin_r, -sin_d, -cos_r, -cos_d, -sin_r],
    )

    y = i_arr - 1j * q_arr
    if n_f == 1:
        return y[0]
    return y.T


def bit_rev_order(n):
    """Return bit-reversal permutation array of length n."""
    n_bits = int(round(np.log2(n)))
    return np.array([int(f'{i:0{n_bits}b}'[::-1], 2) for i in range(n)], dtype=np.int32)


def r_dit_fft(x, inv_flag=0, prec_type='half_fixed', scale_out=0):
    """VSPA DIT-FFT behavioral model."""
    x = np.asarray(x, dtype=np.complex128)
    n_len = len(x)
    n_stages = int(round(np.log2(n_len)))

    scale_fact = 2 if (prec_type == 'half_fixed' and scale_out == 1) else 1

    br_idx = bit_rev_order(n_len)
    y = x[br_idx].copy()

    n_samples_grp = 1
    n_groups = n_len // 2
    freq = 1.0 / n_len

    for _ in range(1, n_stages + 1):
        if not inv_flag:
            twf = r_nco(freq * n_groups, n_samples_grp, 0)
        else:
            twf = r_nco(-freq * n_groups, n_samples_grp, 0)

        yy = y.reshape(n_groups, 2, n_samples_grp)
        t1 = yy[:, 0, :].copy()
        t2 = yy[:, 1, :].copy()
        yy[:, 0, :] = r_smad(t2, twf, t1)
        yy[:, 1, :] = r_smad(-t2, twf, t1)
        y = yy.reshape(n_len)

        if prec_type == 'half_fixed':
            y = r_half(y / scale_fact)
        else:
            y = r_single(y)

        n_samples_grp *= 2
        n_groups //= 2

    return y
