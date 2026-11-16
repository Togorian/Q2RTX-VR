# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
"""Exercise campaign-ending New Game with selected lifecycle and cinematic code.
Run from an x64 MSVC developer shell with --output ABSENT_DIRECTORY outside source.
Engine rendering, resource and stock command dispatch calls are inert seams.
No game, runtime, player saves or settings are opened. Outputs are retained.
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
client = 'source/current/palace-fresh514/source/engine_037/reference_b_b2_polish93-r2.c'
menu = 'source/current/rc-cleanup457/source/b2_menu151_client457.inc'
cinematic = 'source/baseline/b2-consolidated215/inputs/origin-040/src/client/cin.c'
proof = []


def record(path, name, text):
    proof.append(dict(source=path, selection=name, sha256=hashlib.sha256(text.encode()).hexdigest()))
    return text


def take(path, name):
    text = (source / path).read_text(encoding='utf-8')
    match = re.search(r'^(?:static )?(?:bool|void) ' + name + r'\([^;]*?\)\s*\{', text, re.M)
    assert match, name
    end, depth = match.end(), 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return record(path, name, text[match.start():end]) + '\n'


production = ''.join(take(path, name) for path, name in (
    (menu, 'B2_MenuSession'), (menu, 'B2_CinematicBoundary'),
    (menu, 'B2_UISurfaceActive'), (client, 'B2_BeginWorldChange'),
    (client, 'b2_world_wait_valid'), (cinematic, 'SCR_PlayCinematic')))
current = take(menu, 'B2_MenuCommand')
newgame = take(client, 'b2_newgame_f')
old_path = 'tools/ending-newgame-before.c.in'
before = record(old_path, 'retained pre-correction B2_MenuCommand', (source / old_path).read_text(encoding='utf-8'))
template = (source / 'tools/ending-newgame-case.c.in').read_text(encoding='utf-8')
for marker, value in [('/* PRODUCTION */', production), ('/* NEWGAME */', newgame)]:
    assert template.count(marker) == 1
    template = template.replace(marker, value)
assert template.count('/* MENU_COMMAND */') == 1
for stem, gate in [('before', before), ('current', current)]:
    (output / (stem + '.c')).write_text(('#define EXPECT_PREVIOUS_REFUSAL 1\n' if stem == 'before' else '') +
        template.replace('/* MENU_COMMAND */', gate), encoding='utf-8')
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


for stem in ('before', 'current'):
    run(stem + '-compile', ['cl.exe', '/nologo', '/W3', '/O1', '/MT', '/std:c11', '/D_CRT_SECURE_NO_WARNINGS',
        stem + '.c', '/Fe' + stem + '.exe', '/Fo' + stem + '.obj', '/link', '/INCREMENTAL:NO'])
    run(stem + '-run', [output / (stem + '.exe')])
(output / 'COMPLETE.json').write_text(json.dumps(dict(status='ENDING_NEWGAME_REGRESSION_PASSED',
    scope='Actual selected world retirement, cinematic activation, menu session/ownership gates and New Game dispatch; renderer/XR/resource/stock-command calls are controlled seams. Before-correction refusal reproduced. No runtime, filesystem-save or end-to-end campaign claim.',
    gameOrHeadsetExecution=False), indent=2) + '\n', encoding='utf-8')
