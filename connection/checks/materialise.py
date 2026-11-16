# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Test glue: Togorian with Codex assistance, 11 November 2026.
"""Pin exact selected C spans; CMake compiles full cvar/shared/utils directly."""
from pathlib import Path
import argparse,json,hashlib,re
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
sha=lambda b:hashlib.sha256(b).hexdigest()
plan=json.loads((a.source/'recipes/build-plan.json').read_bytes())
units={r['unit']:r for r in plan['C'] if r['target']=='engine'}
records=[]
def selected(unit):
    rel=units[unit]['source'];b=(a.source/rel).read_bytes()
    records.append({'unit':unit,'path':rel,'bytes':len(b),'sha256':sha(b)})
    return rel,b.decode('utf-8')
commonRel,common=selected('engine_042');serverRel,server=selected('engine_078')
for u in ['engine_043','engine_054','engine_065','engine_012']:selected(u)
spans=[]
def capture(label,rel,text,pattern):
    found=list(re.finditer(pattern,text,re.M));assert len(found)==1,label
    m=found[0];value=m[0]
    spans.append({'label':label,'path':rel,'startLine':text[:m.start()].count('\n')+1,'bytes':len(value.encode()),'sha256':sha(value.encode()),'exactText':value})
    return value
display=capture('display declaration',commonRel,common,r'^const char  com_version_string\[\] =\r?\n    [^\r\n]+;')
wire=capture('client identifier declaration',commonRel,common,r'^static const char com_client_version_string\[\] =\r?\n    [^\r\n]+;')
registration=capture('actual Cvar_Get registration',commonRel,common,r'^    com_version = Cvar_Get\("version", com_client_version_string, CVAR_SERVERINFO \| CVAR_USERINFO \| CVAR_ROM\);')
restriction=capture('unchanged server restriction block',serverRel,server,r'^\tif \(sv_restrict_rtx->integer\)\r?\n\t\{\r?\n\t\ts = Info_ValueForKey\(info, "version"\);\r?\n\t\tif \(strncmp\(s, "q2rtx", 5\) != 0\)\r?\n\t\t\{\r?\n\t\t\treturn reject\("This server is only available to Q2RTX clients\.\\n"\);\r?\n\t\t\}\r?\n\t\}')
client=(a.source/units['engine_012']['source']).read_text(encoding='utf-8')
assert 'Cvar_BitInfo(userinfo, CVAR_USERINFO);\n    Netchan_OutOfBand' in client
out='/* Exact production spans; see production-spans.json and original notices. */\n'+display+'\n'+wire+'\nstatic void register_actual_version(void)\n{\n'+registration+'\n}\nstatic bool actual_server_version_restriction(const char *info)\n{\n    char *s;\n'+restriction+'\n    return true;\n}\n'
a.output.mkdir(parents=True,exist_ok=True)
for name,data in [('production-spans.inc',out.encode()),('production-spans.json',(json.dumps({'selectedFiles':records,'spans':spans,'clientConnectUsesActualCvarBitInfo':True,'fullCvarSharedUtilsCompiledDirectly':True,'networkProtocolOrPasswordChecksChanged':False},indent=2)+'\n').encode())]:
    with (a.output/name).open('xb') as f:f.write(data)
print('Pinned exact producer registration and unchanged server predicate; full cvar/shared/utils sources selected.')
