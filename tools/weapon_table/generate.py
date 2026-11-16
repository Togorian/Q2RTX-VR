# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# SPDX-License-Identifier: GPL-2.0-or-later
# Modified for Q2RTX VR on 2026-11-10: counts/calibration only; raw topology stays in owned assets.
"""Optional weapon-table regeneration from an explicit user-owned asset tree.

Emission follows the original table-01.py generator. See origins.json for the original identities
and README.md for the retained runtime-reference names and flare correction.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path, PurePosixPath
import re
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from model_geometry import load_model

REFERENCES = ('muzzle', 'grip', 'forwardFront', 'forwardBack', 'upper',
              'upperCentre', 'left', 'right')
MODES = {'rim-upper': 0, 'twin-prongs': 1, 'preserve-barrel-spin': 2,
         'grenade-body-convention': 3}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def asset_entry(value):
    require(isinstance(value, str), 'asset entry must be a relative POSIX path')
    path = PurePosixPath(value)
    require(bool(value) and not path.is_absolute() and '..' not in path.parts
            and '\\' not in value and ':' not in value,
            'asset entry must stay within the explicit asset directory')
    return path


def read_asset(c, resolve_asset):
    asset = c['asset']
    asset_entry(asset['entry'])
    require(type(asset['bytes']) is int and asset['bytes'] > 0, 'invalid asset byte length')
    require(isinstance(asset['sha256'], str) and
            re.fullmatch('[0-9a-f]{64}', asset['sha256']), 'invalid asset SHA-256')
    data = resolve_asset(asset['entry'])
    require(isinstance(data, bytes), 'asset reader must return bytes')
    require(len(data) == asset['bytes'] and hashlib.sha256(data).hexdigest() == asset['sha256'],
            f'{asset["entry"]}: exact asset identity mismatch')
    require(asset['format'] in ('md2', 'md3'), 'unsupported asset format')
    require(data[:4] == {'md2': b'IDP2', 'md3': b'IDP3'}[asset['format']],
            f'{asset["entry"]}: format mismatch')
    try:
        model = load_model(data)
    except (AssertionError, struct.error, IndexError, UnicodeError) as error:
        raise ValueError(f'{asset["entry"]}: invalid model format or bounds') from error
    require(model['format'] == asset['format'] and model['frames'] > 0 and model['meshes'],
            f'{asset["entry"]}: invalid frames or meshes')
    require(type(c['readyFrame']) is int and 0 <= c['readyFrame'] < model['frames'],
            f'{asset["entry"]}: ready frame outside model')
    require(c['basisMode'] in MODES, 'unknown basis mode')
    require(isinstance(c['scale'], (int, float)) and math.isfinite(c['scale']) and c['scale'] > 0,
            'scale must be finite and positive')
    for mesh_id, mesh in enumerate(model['meshes']):
        positions = mesh['positions']
        require(mesh['id'] == mesh_id and len(positions) == model['frames']
                and positions and positions[0], 'invalid mesh frame layout')
        nv = len(positions[0])
        require(nv < 65535 and all(len(frame) == nv for frame in positions),
                'invalid mesh vertex bounds')
        require(all(len(t) == 3 and all(type(v) is int and 0 <= v < min(nv, 65535)
                    for v in t) for t in mesh['triangles']), 'triangle index outside mesh')
    require(set(c['references']).issubset(REFERENCES), 'unknown reference group')
    for groups in c['references'].values():
        for group in groups:
            mesh_id = group['mesh']
            require(type(mesh_id) is int and 0 <= mesh_id < len(model['meshes']),
                    'reference mesh outside model')
            nv = len(model['meshes'][mesh_id]['positions'][0])
            require(group['vertices'] and all(type(v) is int and 0 <= v < min(nv, 65535)
                    for v in group['vertices']), 'reference vertex outside mesh')
    return model


def generate_table(calibration, resolve_asset):
    require(__debug__, 'Run without -O: the original parser uses assertions')
    models = calibration['models']
    require(len(models) == 12 and [c['slot'] for c in models] == list(range(12)),
            'expected the twelve ordered model slots')
    lines = ['// SPDX-License-Identifier: GPL-2.0-or-later',
             '// Modified for Q2RTX VR on 2026-11-10: authored references and counts only; no copied triangle arrays.']
    specs = []
    for c in models:
        slot = c['slot']; asset = c['asset']; model = read_asset(c, resolve_asset); groups = []
        for name in REFERENCES:
            entries = []
            for j, g in enumerate(c['references'].get(name, [])):
                symbol = f'b2wm_{slot}_{name}_{j}'; values = g['vertices']
                lines.append('static const uint16_t '+symbol+'[] = {'+','.join(map(str,values))+'};')
                entries.append('{'+f"{g['mesh']},{len(values)},{symbol}"+'}')
            if entries:
                symbol = f'b2wm_{slot}_{name}'
                lines.append('static const b2wm_group_t '+symbol+'[] = {'+','.join(entries)+'};')
                groups.append('{'+f'{len(entries)},{symbol}'+'}')
            else:
                groups.append('{0,NULL}')
        topology = []
        for m in model['meshes']:
            topology.append('{'+f'{len(m["positions"][0])},{len(m["triangles"])}'+'}')
        symbol = f'b2wm_{slot}_meshes'
        lines.append('static const b2wm_mesh_spec_t '+symbol+'[] = {'+','.join(topology)+'};')
        mode = MODES[c['basisMode']]
        digest = '{'+','.join('0x'+asset['sha256'][i:i+2] for i in range(0,64,2))+'}'
        # Runtime names are compatibility references, distinct from MD3 input entries.
        # calibration.json retains .md2 for slots 0-10 and the selected flare .md3 fix.
        model_reference = c['modelReference']
        asset_entry(model_reference)
        specs.append('{'+','.join([json.dumps(model_reference),str(slot),str(asset['bytes']),str(model['frames']),str(c['readyFrame']),str(len(model['meshes'])),str(mode),repr(float(c['scale']))+'f',digest,symbol,'{'+','.join(groups)+'}'])+'}')
    lines.append('static const b2wm_spec_t b2wm_specs[12] = {\n'+',\n'.join(specs)+'\n};')
    return ('\n'.join(lines)+'\n').encode('utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--assets-dir', type=Path, required=True,
                        help='user-owned directory containing models/weapons/...')
    parser.add_argument('--calibration', type=Path, default=Path(__file__).with_name('calibration.json'))
    parser.add_argument('--output', type=Path, required=True, help='new header; parent directory must exist')
    args = parser.parse_args()
    try:
        root = args.assets_dir.resolve(strict=True)
        require(root.is_dir(), 'assets directory must be a directory')
        require(not args.output.exists(), 'output exists; choose a new header path')
        calibration = json.loads(args.calibration.read_bytes())
        def resolve_asset(entry):
            path = (root / asset_entry(entry)).resolve(strict=True)
            require(path.is_relative_to(root), 'asset path escapes the explicit asset directory')
            # Modified 11 November 2026: reject a wrong size before reading the owned file.
            expected = next(c["asset"]["bytes"] for c in calibration["models"]
                            if c["asset"]["entry"] == entry)
            require(path.stat().st_size == expected, f"{entry}: exact asset size mismatch")
            return path.read_bytes()
        output = generate_table(calibration, resolve_asset)
        with args.output.open('xb') as target:
            target.write(output)
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.error(str(error))
    print(f'Generated twelve calibration/count tables: {len(output)} bytes')


if __name__ == '__main__':
    main()
