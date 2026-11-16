#!/usr/bin/env python3
# Copyright (C) 2026 Togorian, to the extent of rights held.
# SPDX-License-Identifier: GPL-2.0-or-later
# Original placeholder geometry by Togorian with Codex assistance. No external image.
"""Generate the Q2RTX VR charcoal/two-eye icon deterministically (standard library)."""
import struct
from pathlib import Path

def pixel(x, y, size):
    eye = size * 5 // 16 <= y < size * 10 // 16 and (size * 2 // 16 <= x < size * 7 // 16 or size * 9 // 16 <= x < size * 14 // 16)
    return (40, 209, 224, 255) if eye else (38, 42, 46, 255)

def icon():
    images = []
    for size in (16, 32, 48, 64):
        pixels = bytearray()
        for y in range(size - 1, -1, -1):
            for x in range(size):
                r, g, b, a = pixel(x, y, size)
                pixels.extend((b, g, r, a))
        mask = bytes(((size + 31) // 32) * 4 * size)
        images.append((size, struct.pack('<IiiHHIIiiII', 40, size, size * 2, 1, 32, 0, len(pixels), 0, 0, 0, 0) + pixels + mask))
    offset = 6 + 16 * len(images)
    directory = bytearray(struct.pack('<HHH', 0, 1, len(images)))
    for size, data in images:
        directory.extend(struct.pack('<BBBBHHII', size, size, 0, 0, 1, 32, len(data), offset))
        offset += len(data)
    return bytes(directory) + b''.join(data for _, data in images)

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    with args.output.open('xb') as stream:
        stream.write(icon())
