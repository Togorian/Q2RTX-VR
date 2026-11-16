#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# Design/direction by Togorian; implementation with Codex assistance.
# Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
# Modified 11 November 2026: provenance/notice clarification only. See PROVENANCE.md.
"""Emit editable C arrays from explicitly supplied SPIR-V modules."""
import argparse
from pathlib import Path
import re
import struct

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--array', action='append', nargs=2, required=True, metavar=('NAME','SPV'))
    args = parser.parse_args()
    lines = ['// SPDX-License-Identifier: GPL-2.0-or-later',
             '// Generated from the explicitly named SPIR-V inputs.', '#pragma once', '#include <stdint.h>']
    names = set()
    for name, filename in args.array:
        if not re.fullmatch(r'[A-Za-z_][A-Za-z_0-9]*', name) or name in names:
            parser.error('array names must be unique C identifiers')
        names.add(name)
        data = Path(filename).read_bytes()
        if len(data) < 20 or len(data) % 4 or data[:4] != b'\x03\x02\x23\x07':
            parser.error(f'{filename}: not a little-endian SPIR-V module')
        words = struct.unpack('<' + 'I' * (len(data)//4), data)
        lines.append(f'static const uint32_t {name}[] = {{')
        for i in range(0,len(words),8):
            lines.append('    ' + ','.join(f'0x{x:08x}u' for x in words[i:i+8]) + ',')
        lines.append('};')
    args.output.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')

if __name__ == '__main__':
    main()
