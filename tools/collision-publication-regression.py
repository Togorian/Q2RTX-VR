# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
"""Exercise the selected combat publisher and query with inert game seams.
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
begin = guard_text.index('static struct {', guard_text.index('static cvar_t *b2_combat;'))
end = guard_text.index('} combat;', begin) + len('} combat;')
state = record(guard_path, 'combat publication state', guard_text[begin:end])
begin = guard_text.index('void G_CombatPublish(')
end = guard_text.index('\nvoid T_Damage(', begin)
publication = record(guard_path, 'complete publisher and query', guard_text[begin:end])
old_path = 'tools/collision-publication-before.c.in'
previous = record(old_path, 'retained pre-correction publisher and query', (source / old_path).read_text())
template = (source / 'tools/collision-publication-case.c.in').read_text(encoding='utf-8')
template = template.replace('/* COMBAT_STATE */', 'enum { B2_COMBAT_SERIAL_HIGH_STAT = B2_COMBAT_STAT - 1 };\n' + state)
(output / 'current.c').write_text(template.replace('/* PUBLICATION */', publication), encoding='utf-8')
(output / 'before.c').write_text('#define EXPECT_PREVIOUS_REFUSAL 1\n' + template.replace('/* PUBLICATION */', previous), encoding='utf-8')
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
        '/I' + str(include), '/I' + str(source / 'source/baseline/b2-consolidated215/inputs/origin-068/inc'), stem + '.c', '/Fe' + stem + '.exe', '/Fo' + stem + '.obj', '/link', '/INCREMENTAL:NO'])
    run(stem + '-run', [output / (stem + '.exe')])
(output / 'COMPLETE.json').write_text(json.dumps(dict(status='COLLISION_PUBLICATION_REGRESSION_PASSED',
    scope='Actual selected publisher and state query; inert game/observer seams. Pre-correction 32768-publication stop reproduced. No end-to-end headset claim.',
    gameOrHeadsetExecution=False), indent=2) + '\n', encoding='utf-8')
