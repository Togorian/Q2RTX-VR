# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
"""Retained inert CPU boundaries. Run from an x64 MSVC developer shell.
Use --output ABSENT_DIRECTORY; no private Git history is required.
Generated functions are exact source text; external engine/XR calls are seams.
No game, graphics, headset or player files are opened. All outputs are retained.
"""
from pathlib import Path
import argparse,subprocess,json,re,hashlib,time
ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
S=Path(__file__).resolve().parents[1];out=a.output.resolve()
assert not out.exists() and not out.is_relative_to(S) and not S.is_relative_to(out);out.mkdir(parents=True)
client='source/current/palace-fresh514/source/engine_037/reference_b_b2_polish93-r2.c'
xr='source/current/upper-palace500/source/engine_087/reference_b_b2_xr.c'
ref='source/current/upper-palace500/source/b2_realign_client500.inc'
inp='source/current/right-menu409/source/b2_controller_impl409.inc';proof=[]
def take(path,name,stop=None):
 text=(S/path).read_text(encoding='utf-8')
 m=re.search(r'^(?:static )?(?:inline )?(?:bool|void) '+name+r'\([^;]*?\)\s*\{',text,re.M);assert m,name
 end=m.end();depth=1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 fn=text[m.start():end]
 if stop:fn=fn[:fn.index(stop)]+'}\n'
 proof.append(dict(source=path,function=name,stopBefore=stop,sha256=hashlib.sha256(fn.encode()).hexdigest(),origin='current-selected-source'))
 return fn+'\n'
code=''.join([take(inp,'b2c_sample_valid'),take(xr,'B2XR_ReadHeadInput'),take(client,'b2rs_retire'),take(client,'b2_recenter_height484'),take(client,'b2rs_filter','    if(b2_roomscale->integer!=1)return;'),take(client,'b2_realign_stance391'),take(client,'b2_finish_interruption256'),take(ref,'b2_reference_stage397'),take(xr,'b2_xr_continuity_offset'),take('source/baseline/b2-consolidated215/inputs/origin-006/src/common/pmove.c','PM_CheckDuck')])
template=(S/'tools/rc8-crouch-resume.c.in').read_text();assert template.count('/* PRODUCTION */')==1
(out/'case.c').write_text(template.replace('/* PRODUCTION */',code),encoding='utf-8')
(out/'EXTRACTIONS.json').write_text(json.dumps(proof,indent=2)+'\n')
def run(name,args):
 tick=time.monotonic();p=subprocess.Popen([str(x) for x in args],cwd=out,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 try:raw=p.communicate(timeout=90)[0]
 except subprocess.TimeoutExpired:
  subprocess.run(['taskkill','/PID',str(p.pid),'/T','/F'],capture_output=True,timeout=20);raw=p.communicate(timeout=10)[0];(out/(name+'-timeout.txt')).write_bytes(raw);raise
 (out/(name+'.txt')).write_bytes(raw);(out/(name+'.json')).write_text(json.dumps(dict(argv=[str(x) for x in args],pid=p.pid,exit=p.returncode,seconds=time.monotonic()-tick,terminal=True),indent=2)+'\n')
 assert p.returncode==0,raw.decode(errors='replace');print(raw.decode(errors='replace'),flush=True)
run('compile',['cl.exe','/nologo','/W3','/O1','/MT','/I'+str(S)]+['case.c','/Fecase.exe','/Focase.obj','/link','/INCREMENTAL:NO'])
run('run',[out/'case.exe'])
(out/'COMPLETE.json').write_text(json.dumps(dict(status='RC8_BOUNDARIES_PASSED',nativeOrHeadsetExecution=False,scope='Exact selected functions and room-scale prefix before horizontal movement; external interaction/hold/neutral/commit ownership calls are controlled seams, not an end-to-end runtime test.'))+'\n')
