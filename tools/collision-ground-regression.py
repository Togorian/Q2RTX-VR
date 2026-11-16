# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
"""Exercise stock overlap-ground jumps and the selected movement guard.
Run from an x64 MSVC developer shell with --output ABSENT_DIRECTORY outside source.
No game, graphics, headset, saved game or runtime is opened. Outputs are retained.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import time

ap = argparse.ArgumentParser(description=__doc__)
ap.add_argument('--output', type=Path, required=True)
a = ap.parse_args()
source = Path(__file__).resolve().parents[1]
output = a.output.resolve()
assert not output.exists() and not output.is_relative_to(source) and not source.is_relative_to(output)
output.mkdir(parents=True)
guard_path = 'source/current/rc-cleanup457/source/game_003/g_combat.c'
pmove_path = 'source/baseline/b2-consolidated215/inputs/origin-006/src/common/pmove.c'
math_path = 'source/baseline/b2-consolidated215/inputs/origin-006/src/shared/shared.c'
proof = []


def record(path, name, text):
    proof.append(dict(source=path, selection=name, sha256=hashlib.sha256(text.encode()).hexdigest()))
    return text


def take(path, name):
    text = (source / path).read_text(encoding='utf-8')
    match = re.search(r'^(?:static )?(?:bool|void|int|vec_t) ' + name + r'\([^;]*?\)\s*\{', text, re.M)
    assert match, name
    end, depth = match.end(), 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return record(path, name, text[match.start():end]) + '\n'


guard_text = (source / guard_path).read_text(encoding='utf-8')
begin = guard_text.index('typedef struct {\n    vec3_t origin, velocity, mins, maxs;')
end = guard_text.index('} obs_trace;', begin) + len('} obs_trace;')
state = record(guard_path, 'observer_state_t and obs_trace', guard_text[begin:end])
helper = take(guard_path, 'observer_stock_snap_fallback')
validator = take(guard_path, 'observer_check_walk_pmove')
callbacks = ''.join(take(guard_path, name) for name in (
    'G_ObserverPmove', 'G_ObserverPointContents', 'G_ObserverCollision'))
math_code = ''.join(take(math_path, name) for name in ('AngleVectors', 'VectorNormalize'))
pmove_code = record(pmove_path, 'complete selected translation unit', (source / pmove_path).read_text(encoding='utf-8'))
old_ground = 'trace->fraction < 1.f && trace->plane.normal[2] >= .7f;'
new_ground = '(trace->plane.normal[2] >= .7f || trace->startsolid);'
assert callbacks.count(new_ground) == 1
old_callbacks = callbacks.replace(new_ground, old_ground)
template = (source / 'tools/collision-ground-case.c.in').read_text(encoding='utf-8')
for marker, text in [('/* OBSERVER_STATE */', state), ('/* PMOVE */', math_code + pmove_code),
                     ('/* VALIDATOR */', validator)]:
    assert template.count(marker) == 1
    template = template.replace(marker, text)
assert template.count('/* SNAP_HELPER */') == 1
(output / 'current.c').write_text(template.replace('/* SNAP_HELPER */', helper).replace('/* CALLBACKS */', callbacks), encoding='utf-8')
(output / 'before.c').write_text('#define EXPECT_PREVIOUS_REFUSAL 1\n' + template.replace('/* SNAP_HELPER */', helper).replace('/* CALLBACKS */', old_callbacks), encoding='utf-8')
(output / 'EXTRACTIONS.json').write_text(json.dumps(proof, indent=2) + '\n', encoding='utf-8')


def run(name, command):
    start = time.monotonic()
    child = subprocess.Popen([str(x) for x in command], cwd=output, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        raw = child.communicate(timeout=90)[0]
    except subprocess.TimeoutExpired:
        subprocess.run(['taskkill', '/PID', str(child.pid), '/T', '/F'], capture_output=True, timeout=20)
        raw = child.communicate(timeout=10)[0]
        (output / (name + '-timeout.txt')).write_bytes(raw)
        raise
    (output / (name + '.txt')).write_bytes(raw)
    (output / (name + '.json')).write_text(json.dumps(dict(argv=[str(x) for x in command], pid=child.pid,
        exit=child.returncode, seconds=time.monotonic() - start, terminal=True), indent=2) + '\n', encoding='utf-8')
    print(raw.decode(errors='replace'), flush=True)
    assert child.returncode == 0, name


include = source / 'source/baseline/b2-consolidated215/inputs/origin-046/upstream/inc'
for stem in ('before', 'current'):
    run(stem + '-compile', ['cl.exe', '/nologo', '/W3', '/O1', '/MT', '/std:c11', '/D_CRT_SECURE_NO_WARNINGS',
        '/I' + str(include), stem + '.c', '/Fe' + stem + '.exe', '/Fo' + stem + '.obj', '/link', '/INCREMENTAL:NO'])
    run(stem + '-run', [output / (stem + '.exe')])
(output / 'COMPLETE.json').write_text(json.dumps(dict(status='COLLISION_GROUND_REGRESSION_PASSED',
    scope='Actual selected Pmove, mathematics, capture callbacks and movement validator; collision and point-content results are controlled seams. Pre-correction refusal is reproduced. No end-to-end headset or BSP collision claim.',
    gameOrHeadsetExecution=False), indent=2) + '\n', encoding='utf-8')
