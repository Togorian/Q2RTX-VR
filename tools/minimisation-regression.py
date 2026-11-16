# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
"""Check headset presentation admission while the desktop mirror is minimised.

Run from an x64 MSVC developer prompt with --output pointing to a new directory
outside the source tree. No game, graphics, headset or player files are opened.
The actual frame-refusal block is extracted, and the retained interruption case
must match its selected production functions. External effects remain seams.
The pre-fix variant restores only the former minimised-KBM condition and must fail.
"""
from pathlib import Path
import argparse
import hashlib
import json
import runpy
import subprocess
import time

CLIENT = 'source/current/palace-fresh514/source/engine_037/reference_b_b2_polish93-r2.c'
XR = 'source/current/upper-palace500/source/engine_087/reference_b_b2_xr.c'
RENDERER = 'source/current/key-render-crash492/source/engine_112/main.c'
HARNESS = 'tools/rc7-cases/R6-S5-interruption-caller.c'
TEMPLATE = 'tools/minimisation-case.c.in'
OLD_CLAUSE = '        (!B2_ControllerFocusMode556() && cls.active==ACT_MINIMIZED) ||\n'


def digest(text):
    return hashlib.sha256(text.encode()).hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    root = Path(__file__).resolve().parents[1]
    out = args.output.resolve()
    if out.exists() or out.is_relative_to(root) or root.is_relative_to(out):
        raise ValueError('Use a new, disjoint output directory')
    helpers = runpy.run_path(str(root/'tools/rc7-regression.py'))
    function, run = helpers['function'], helpers['run']
    client = (root/CLIENT).read_text(encoding='utf-8')
    renderer = (root/RENDERER).read_text(encoding='utf-8')
    xr = (root/XR).read_text(encoding='utf-8')
    harness = (root/HARNESS).read_text(encoding='utf-8')
    plan = json.loads((root/'tools/rc7-cases/R6.json').read_text())
    case = next(c for c in plan['cases'] if c['file'] == Path(HARNESS).name)
    proof = []
    for check in case['production']:
        span = function((root/check['source']).read_text(encoding='utf-8'), check['function'])
        assert span.strip() in harness, ('Retained interruption fixture drifted', check)
        proof.append(dict(source=check['source'], function=check['function'], sha256=digest(span)))

    start = '    if (B2_KeepFramePump() && (!frame.should_render || !frame.tracking_valid ||\n'
    end = '    initial_reference_ready = B2XR_InitialReferenceReady();'
    assert client.count(start) == 1
    block = client[client.index(start):client.index(end, client.index(start))]
    assert OLD_CLAUSE not in block and 'ended = B2XR_EndFrame(false);' in block
    assert block.count('return true;') == 1
    proof.append(dict(source=CLIENT, span='B2_RenderStereo post-begin refusal block', sha256=digest(block)))

    # Static corroboration of the XR/desktop ownership routes. These checks do
    # not emulate a compositor or prove native GPU execution while minimised.
    end_frame = function(xr, 'B2XR_EndFrame')
    assert 'bool complete = rendered_pair && !xr.failed && !xr.loss_pending' in end_frame
    assert '.layerCount = complete ? 1 : 0, .layers = complete ? layers : NULL' in end_frame
    begin_render = function(renderer, 'R_BeginFrame_RTX')
    end_render = function(renderer, 'R_EndFrame_RTX')
    assert 'b2_desktop_drawable.width < 2 || b2_desktop_drawable.height < 2' in begin_render
    assert 'if (b2_mirror.no_image534) b2_mirror.wait_acquire = false;' in begin_render
    assert 'b2_desktop_enabled() ? 0 :' in begin_render
    assert end_render.index('Desktop blit; XR copy below is independent of presentation.') < end_render.index('B2XR_CopyEye(')
    assert 'b2_mirror.wait_acquire ? LENGTH(wait_semaphores) : 0' in end_render
    assert '(b2_mirror.no_image534 || (b2_eye && !b2_last)) ? 0 : qvk.device_count' in end_render
    assert 'return; // No desktop image/wait/signal/present; completed XR work remains valid.' in end_render
    startup = (root/'launcher/accepted-startup.h').read_text(encoding='utf-8')
    assert 'L"b2_desktop_mirror",\nL"1",' in startup
    for path, name, span in [(XR, 'B2XR_EndFrame', end_frame),
                             (RENDERER, 'R_BeginFrame_RTX', begin_render),
                             (RENDERER, 'R_EndFrame_RTX', end_render)]:
        proof.append(dict(source=path, function=name, sha256=digest(span), check='static route corroboration'))
    proof.append(dict(source='launcher/accepted-startup.h', span='whole-header', sha256=digest(startup)))

    # Keep the existing selected-production functions intact. Add only fields
    # and external-effect seams needed by the newly extracted refusal block.
    replacements = [
        ('#include <assert.h>', '#include <assert.h>\n#include <stdlib.h>\n#undef assert\n#define assert(x) do { if (!(x)) { fprintf(stderr,"FAIL %s:%d: %s\\n",__FILE__,__LINE__,#x); exit(86); } } while(0)'),
        (' frame;} cl;', ' frame;int refdef;} cl;'),
        ('struct {bool valid;} history;} b2_client;', 'struct {bool valid;} history;int scene;} b2_client;'),
        ('int main(void)', 'static int retained_interruption_cases(void)'),
    ]
    for old, new in replacements:
        assert harness.count(old) == 1, old
        harness = harness.replace(old, new)
    template = (root/TEMPLATE).read_text(encoding='utf-8')
    assert template.count('/* HARNESS */') == template.count('/* FRAME-REFUSAL */') == 1
    out.mkdir(parents=True)
    results = []
    for label, previous in [('current', False), ('pre-fix-negative', True)]:
        work = out/label
        work.mkdir()
        selected = block.replace(start, start+OLD_CLAUSE) if previous else block
        code = template.replace('/* HARNESS */', harness).replace('/* FRAME-REFUSAL */', selected)
        (work/'case.c').write_text(code, encoding='utf-8')
        records = proof+[dict(source=CLIENT, span='compiled refusal block',
                            variant=label, restoredCondition=OLD_CLAUSE.strip() if previous else None,
                            sha256=digest(selected))]
        (work/'EXTRACTIONS.json').write_text(json.dumps(records, indent=2)+'\n')
        run(['cl.exe', '/nologo', '/W3', '/O1', '/MT', '/I'+str(root/'source'),
             'case.c', '/Fetest.exe', '/Fotest.obj', '/link', '/INCREMENTAL:NO'], work, work/'compile.txt')
        if previous:
            tick = time.monotonic()
            p = subprocess.run([str(work/'test.exe')], cwd=work, capture_output=True, timeout=30)
            (work/'run.txt').write_bytes(p.stdout+p.stderr)
            receipt = dict(argv=[str(work/'test.exe')], exit=p.returncode,
                           seconds=time.monotonic()-tick, terminal=True, expectedFailure=True)
            (work/'run.json').write_text(json.dumps(receipt, indent=2)+'\n')
            assert p.returncode == 86 and b'minimised KBM must retain headset rendering' in p.stderr, \
                ('Pre-fix must fail the minimised presentation case', p.returncode)
            results.append(dict(case=label, exit=p.returncode, expectedFailure=True))
        else:
            run([work/'test.exe'], work, work/'run.txt', 30)
            results.append(dict(case=label, exit=0))
        print(label, 'PASS', flush=True)
    (out/'COMPLETE.json').write_text(json.dumps(dict(
        status='PASSED', cases=results, headsetVerification=False,
        scope='Actual frame-refusal block and source-checked interruption functions; external presentation/hold effects are seams. XR zero-layer and desktop-independent ownership paths are statically checked. Native minimise/restore comfort requires a headset test.'), indent=2)+'\n')


if __name__ == '__main__':
    main()
