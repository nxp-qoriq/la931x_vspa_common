# SPDX-License-Identifier: BSD-3-Clause
"""Python model for crc8_encode kernel."""

from __future__ import annotations


def crc8_encode(data1: int, data2: int, size: int) -> int:
    gen_poly = 0xE0

    if size == 34:
        data_temp = (data1 ^ 0x000000FF) & 0xFFFFFFFF
        for i in range(2):
            lsb = data_temp & 0x00000001
            data_temp >>= 1
            if ((data2 >> i) & 0x00000001) != 0:
                data_temp = (data_temp + 0x80000000) & 0xFFFFFFFF
            if lsb != 0:
                data_temp ^= gen_poly
        for _ in range(32):
            lsb = data_temp & 0x00000001
            data_temp >>= 1
            if lsb != 0:
                data_temp ^= gen_poly
    else:
        data_temp = (data1 ^ 0x000000FF) & 0xFFFFFFFF
        for _ in range(size):
            lsb = data_temp & 0x00000001
            data_temp >>= 1
            if lsb != 0:
                data_temp ^= gen_poly

    crc = data_temp & 0xFF
    crc ^= 0xFF
    return crc & 0xFF
