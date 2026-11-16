# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian; implementation with Codex assistance.
"""CPU-only selected snapshot-time block plus actual game publication getter.

Use --output with a new directory outside the source tree, from an x64 MSVC
developer prompt. No game/runtime/GPU is started. The retained pre-fix block
must reject the valid-publication positive case.
"""
from pathlib import Path
import argparse, hashlib, json, runpy, subprocess

CLIENT='source/current/palace-fresh514/source/engine_037/reference_b_b2_polish93-r2.c'
GAME='source/current/rc-cleanup457/source/game_003/g_combat.c'
CASE='tools/publication-gap-case.c.in'
BEFORE='tools/publication-gap-before.c.in'

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
    root=Path(__file__).resolve().parents[1];out=a.output.resolve()
    if out.exists() or out.is_relative_to(root) or root.is_relative_to(out):raise ValueError('Use a new, disjoint output directory')
    helpers=runpy.run_path(str(root/'tools/rc7-regression.py'));run=helpers['run'];function=helpers['function']
    client=(root/CLIENT).read_text(encoding='utf-8');scene=function(client,'B2_ObserveScene')
    walking=scene[scene.index('    if (b2_client.walking) {'):scene.index('    if (initial) {')]
    prefix=scene[:scene.index('    if (b2_client.walking) {')]
    assert 'b2_client.history.frame_open' in prefix and '!cl.frame.valid' in prefix and 'if (!B2_CombatSnapshot()) return false;' in prefix
    combat=function(client,'B2_CombatSnapshot');assert '!SV_B2CombatState(&cl.frame.ps, &next)' in combat
    game=(root/GAME).read_text(encoding='utf-8');getter=game[game.index('q_exported bool GetB2CombatState('):];getter=getter[:getter.index('\n}')+2]
    template=(root/CASE).read_text(encoding='utf-8');before=(root/BEFORE).read_text(encoding='utf-8');before=before[before.index('    if (b2_client.walking) {'):]
    out.mkdir(parents=True);results=[]
    for label,block in [('current',walking),('pre-fix-negative',before)]:
        work=out/label;work.mkdir();code=template.replace('/* @GETTER@ */',getter).replace('/* @WALKING@ */',block)
        (work/'case.c').write_text(code,encoding='utf-8')
        spans=[(CLIENT,'B2_ObserveScene admission',prefix),(CLIENT,'B2_CombatSnapshot',combat),(GAME,'GetB2CombatState',getter),(CLIENT if label=='current' else BEFORE,'walking block',block)]
        (work/'EXTRACTIONS.json').write_text(json.dumps([dict(source=p,span=n,sha256=hashlib.sha256(t.encode()).hexdigest()) for p,n,t in spans],indent=2)+'\n')
        run(['cl.exe','/nologo','/W3','/O1','/MT','case.c','/Fetest.exe','/Fotest.obj','/link','/INCREMENTAL:NO'],work,work/'compile.txt')
        if label=='current':run([work/'test.exe'],work,work/'run.txt',30);results.append(dict(case=label,exit=0))
        else:
            p=subprocess.run([str(work/'test.exe')],cwd=work,capture_output=True,timeout=30);(work/'run.txt').write_bytes(p.stdout+p.stderr)
            assert p.returncode==86 and b'observe(&fd)' in p.stderr,('Old bound must fail valid publication case',p.returncode)
            results.append(dict(case=label,exit=p.returncode,expectedFailure=True))
        print(label,'PASS',flush=True)
    (out/'COMPLETE.json').write_text(json.dumps(dict(status='PASSED',cases=results,headsetVerification=False),indent=2)+'\n')

if __name__=='__main__':main()
