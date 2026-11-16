# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
"""Run retained RC7 CPU cases from an x64 MSVC developer command prompt.

python -I -B -S tools/rc7-regression.py --output C:/scratch/rc7-checks
The output directory must be absent and outside the source tree. No game or GPU
is started. Extracted production spans must still match before compilation.
"""
from pathlib import Path
import argparse,json,re,subprocess,time

def function(text,name):
    match=re.search(r'^(?:static )?(?:inline )?(?:void|bool|int|uint32_t|VkImage|VkResult|int64_t|uint64_t|float|qboolean)[\s\n]+\b'+re.escape(name)+r'\([^;]*?\)\s*\{',text,re.M)
    if not match:raise ValueError('Missing selected function: '+name)
    end=match.end();depth=1
    while depth:
        if text[end]=='{':depth+=1
        elif text[end]=='}':depth-=1
        end+=1
    return text[match.start():end]

def member(root,name):
    path=root/name
    if not path.resolve().is_relative_to(root.resolve()):raise ValueError('Path escapes root')
    return path

def run(args,cwd,log,timeout=90):
    started=time.monotonic();p=subprocess.Popen([str(x) for x in args],cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    try:raw=p.communicate(timeout=timeout)[0]
    except subprocess.TimeoutExpired:
        subprocess.run(['taskkill','/PID',str(p.pid),'/T','/F'],capture_output=True,timeout=20)
        raw=p.communicate(timeout=10)[0];log.write_bytes(raw);raise
    log.write_bytes(raw)
    result=dict(argv=[str(x) for x in args],exit=p.returncode,seconds=time.monotonic()-started,terminal=True)
    log.with_suffix('.json').write_text(json.dumps(result,indent=2)+'\n')
    if p.returncode:raise RuntimeError(str(log))

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--finding');ap.add_argument('--case');a=ap.parse_args()
    source=Path(__file__).resolve().parents[1];out=a.output.resolve()
    if out.exists() or out.is_relative_to(source) or source.is_relative_to(out):raise ValueError('Use a new, disjoint output directory')
    out.mkdir(parents=True);cases=source/'tools/rc7-cases';executed=[]
    for manifest in sorted(cases.glob('*.json')):
        if a.finding and manifest.stem!=a.finding:continue
        plan=json.loads(manifest.read_text())
        if plan.get('readme'):
            if a.case:continue
            phrase="Defaults are controllers, right-handed with Togorian's preferred bindings"
            version=(source/'VERSION').read_text().strip();text=(source/'packaging/README.runtime.md').read_text()
            assert phrase in (source/'README.md').read_text() and phrase in text
            substitutions={'@Q2RTX_VERSION@':version,'@Q2RTX_TAG@':'v'+version,'@Q2RTX_LOG_FILE@':'console'+version+'.log'}
            assert 'for old,new in substitutions.items():t=t.replace(old,new)' in (source/'packaging/package.py').read_text()
            for old,new in substitutions.items():text=text.replace(old,new)
            assert '@Q2RTX_' not in text
            (out/'A8-runtime-preview.md').write_text(text,encoding='utf-8');executed.append('A8-readme');print('A8 PASS');continue
        for case in plan['cases']:
            if a.case and case['file']!=a.case:continue
            code=member(cases,case['file']).read_text(encoding='utf-8')
            for check in case['production']:
                text=member(source,check['source']).read_text(encoding='utf-8')
                if 'function' in check:text=function(text,check['function'])
                if 'start' in check:text=text[text.index(check['start']):]
                if 'end' in check:text=text[:text.index(check['end'])]
                if 'length' in check:text=text[:check['length']]
                assert text.strip() in code,(case['file'],check)
            work=out/Path(case['file']).stem;work.mkdir()
            if case.get('productVersion'):
                run(['cmake','-DQ2RTX_VERSION_OUTPUT='+str(work),'-P',source/'recipes/product-version.cmake'],work,work/'version.txt')
            setup=case.get('setup',{})
            for name in setup.get('dirs',[]):member(work,name).mkdir(parents=True,exist_ok=True)
            for name,value in setup.get('files',{}).items():member(work,name).write_text(value,encoding='ascii')
            for i,(link,target) in enumerate(setup.get('junctions',[])):
                link=member(work,link);target=member(work,target)
                quote=lambda p:"'"+str(p).replace("'","''")+"'"
                command='New-Item -ItemType Junction -Path '+quote(link)+' -Target '+quote(target)+' | Select-Object FullName,LinkType'
                run(['powershell.exe','-NoProfile','-Command',command],work,work/('junction-'+str(i)+'.txt'),30)
            args=['cl.exe','/nologo','/W3','/O1','/MT','/I'+str(work)]+['/I'+str(member(source,x)) for x in case.get('includes',[])]+[str(member(cases,case['file'])),'/Fetest.exe','/Fotest.obj','/link','/INCREMENTAL:NO']
            run(args,work,work/'compile.txt');run([work/'test.exe'],work,work/'run.txt',60)
            executed.append(case['file'])
            print(case['file'],'PASS',flush=True)
    if not executed:raise ValueError('No cases matched the requested finding/case selection')
    (out/'COMPLETE.json').write_text(json.dumps(dict(status='CPU_CASES_PASSED',headsetVerification=False,finding=a.finding,executedCases=executed,count=len(executed)))+'\n')

if __name__=='__main__':main()
