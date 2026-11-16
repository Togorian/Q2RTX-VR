#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Project CPU fixtures, Togorian with Codex assistance; rights held GPLv2-or-later.
"""Small inert fixtures for the actual packager. No compiler/product execution."""
import argparse
import importlib.util
import io
import json
from pathlib import Path
import struct
import traceback
import zipfile

def module():
    spec=importlib.util.spec_from_file_location("q2rtx_packager",Path(__file__).with_name("package.py"))
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

# Historical version fixtures, retained from 12 November 2026; these inert PE-shaped
# bytes deliberately exercise prerelease identity and an older-version refusal.
def inert_pe(v="0.8.4-rc.3",nums=(0,8,4,3)):
    """Version-resource-only PE-shaped bytes; no executable code or imports."""
    def block(key,value=b"",kind=1,children=()):
        b=bytearray(b"\0"*6+key.encode("utf-16le")+b"\0\0")
        b+=b"\0"*((-len(b))%4);b+=value
        for child in children:b+=b"\0"*((-len(b))%4);b+=child
        struct.pack_into("<HHH",b,0,len(b),len(value)//2 if kind else len(value),kind)
        return bytes(b)
    strings=[block(k,(val+"\0").encode("utf-16le")) for k,val in
             [("FileVersion",v),("ProductVersion",v),("ProductName","Q2RTX VR"),("CompanyName","Togorian"),("LegalCopyright","Copyright (C) 2019-2025, NVIDIA CORPORATION. All rights reserved. Copyright (C) 2026 Togorian"),("Comments","INERT_FIXTURE_ONLY_DO_NOT_EXECUTE")]]
    fixed=struct.pack("<13I",0xfeef04bd,0x10000,(nums[0]<<16)|nums[1],(nums[2]<<16)|nums[3],
                      (nums[0]<<16)|nums[1],(nums[2]<<16)|nums[3],0x3f,2 if nums[3] else 0,0x40004,1,0,0,0)
    version=block("VS_VERSION_INFO",fixed,0,[block("StringFileInfo",children=[block("040904b0",children=strings)])])
    rsrc=bytearray(96)
    for at,key,to in [(0,16,0x80000000|24),(24,1,0x80000000|48),(48,0x409,72)]:
        struct.pack_into("<HH",rsrc,at+12,0,1);struct.pack_into("<II",rsrc,at+16,key,to)
    struct.pack_into("<IIII",rsrc,72,0x1000+96,len(version),0,0);rsrc+=version
    raw=(len(rsrc)+511)//512*512
    header=bytearray(512);header[:2]=b"MZ";struct.pack_into("<I",header,0x3c,128)
    header[128:132]=b"PE\0\0";struct.pack_into("<HHIIIHH",header,132,0x8664,1,0,0,0,240,0x22)
    opt=152;struct.pack_into("<H",header,opt,0x20b);struct.pack_into("<I",header,opt+108,16)
    struct.pack_into("<II",header,opt+128,0x1000,len(rsrc))
    section=opt+240;header[section:section+8]=b".rsrc\0\0\0"
    struct.pack_into("<IIII",header,section+8,len(rsrc),0x1000,raw,512)
    return bytes(header+rsrc+b"\0"*(raw-len(rsrc)))

def run(output):
    m=module();output=Path(output);assert not output.exists()
    output.mkdir(parents=True);results=[]
    def case(name,callback):
        try:
            callback();results.append({"case":name,"passed":True})
        except Exception:
            results.append({"case":name,"passed":False,"error":traceback.format_exc()});raise
    def fixture(name,private=None,bad_asset=False,bad_source=False,archive_link=False):
        root=output/name;s=root/"source";b=root/"build";libs=root/"libraries";assets=root/"assets"
        def put(path,value):m.write_new(path,value)
        for path,value in {"VERSION":b"0.8.4-rc.3\n","README.md":b"INERT FIXTURE ONLY; no real source/products/assets.\n",
          "packaging/README.runtime.md":b"# Inert fixture @Q2RTX_VERSION@\nLog @Q2RTX_LOG_FILE@\n",
          "docs/guide.md":b"INERT USER GUIDE\n", "docs/engineering.md":b"INERT SOURCE-ONLY NOTE\n",
          "packaging/privacy-exceptions.json":m.encoded({"exceptions":[]})}.items():put(s/path,value)
        payload={"files":[{"path":p,"ship":True} for p in m.PRODUCTS]+[{"path":"openxr_loader.dll","bytes":6,"sha256":m.digest(b"loader" if not bad_asset else b"wrong!")}],
                 "runtimeTemplates":{"README.md":"packaging/README.runtime.md","docs/guide.md":"docs/guide.md"}}
        for dest,src,value in [
            ("home/baseq2/autoexec.cfg","packaging/defaults/baseq2/autoexec.cfg",b"// INERT CLEAN DEFAULT\n"),
            ("home/baseq2/default-vr.cfg","packaging/defaults/baseq2/default-vr.cfg",b"// INERT IMMUTABLE DEFAULT\n"),
            ("home/baseq2/pics/b2/arrow.png","assets/pics/b2/arrow.png",b"INERT ARTWORK MAPPING")
        ]:
            put(s/src,value)
            payload["files"].append({"path":dest,"bytes":len(value),"sha256":m.digest(value)})
        # Header-shaped inert fixture only; no SPIR-V instructions or execution.
        module_bytes=struct.pack("<5I",0x07230203,0x00010500,0,1,0)
        glsl=b"// INERT FIXTURE; never compiled\n"
        header=b"static const uint32_t fixture_spv[] = {0x07230203u,0x00010500u,0x00000000u,0x00000001u,0x00000000u};\n"
        put(s/"fixture.comp",glsl);put(s/"fixture_spv.h",header)
        pin={"source":"fixture.comp","sourceBytes":len(glsl),"sourceSha256":m.digest(glsl)}
        put(s/"recipes/shader-plan.json",m.encoded({"shipped":[dict(pin,output="fixture.comp.spv",referenceBytes=len(module_bytes),referenceSha256=m.digest(module_bytes))],
            "embedded":[{"header":"fixture_spv.h","selectedCopies":["fixture_spv.h"],"arrays":[dict(pin,array="fixture_spv",bytes=len(module_bytes),sha256=m.digest(module_bytes))]}]}))
        # Keep fixture outputs outside the absent native build used by prepare.
        put(root/"shader-fixture/shaders/fixture.comp.spv",module_bytes)
        put(root/"shader-fixture/embedded/fixture_spv.spv",module_bytes)
        put(root/"shader-fixture/generated/fixture_spv.h",header)
        payload["files"].append({"path":"home/baseq2/shader_vkpt/fixture.comp.spv","bytes":len(module_bytes),"sha256":m.digest(module_bytes)})
        put(s/"packaging/payload-plan.json",m.encoded(payload))
        paths=["VERSION","README.md","packaging/README.runtime.md","packaging/privacy-exceptions.json","packaging/payload-plan.json"]
        paths += ["docs/guide.md","docs/engineering.md","packaging/defaults/baseq2/autoexec.cfg","packaging/defaults/baseq2/default-vr.cfg","assets/pics/b2/arrow.png"]
        paths += ["fixture.comp","fixture_spv.h","recipes/shader-plan.json"]
        if archive_link:
            buffer=io.BytesIO()
            with zipfile.ZipFile(buffer,"w") as z:
                zi=zipfile.ZipInfo("unsafe-link");zi.create_system=3;zi.external_attr=0o120777<<16;z.writestr(zi,"target")
            put(s/"fixture.zip",buffer.getvalue());paths.append("fixture.zip")
        if bad_source:paths.append("../escape.txt")
        put(s/m.SOURCE_LIST,m.encoded({"paths":paths}))
        for n in (*m.LIBRARIES,"vulkan-1.lib"):put(libs/n,b"INERT_LIBRARY"+(private if private and n==m.LIBRARIES[0] else b""))
        put(assets/"openxr_loader.dll",b"loader")
        return s,b,libs,assets,root
    def built(parts,old=False):
        s,b,l,a,r=parts;m.prepare(s,b,l)
        for path in (r/"shader-fixture").rglob("*"):
            if path.is_file():m.write_new(b/"shader-output"/path.relative_to(r/"shader-fixture"),path.read_bytes())
        for n in m.PRODUCTS:m.write_new(b/"bin"/n,inert_pe("0.8.4-rc.2",(0,8,4,2)) if old else inert_pe())
        stamp="2000-01-01T00:00:00Z"
        evidence=b"INERT FIXTURE ONLY: no compiler or build ran.\n"
        m.write_new(b/"fixture-evidence.txt",evidence)
        context={"schema":1,"sourceCommit":"0"*40,"sourceCommitUtc":stamp,
                 "inputsSha256":m.digest(m.encoded(m.input_record(s,l))),"buildStartedUtc":stamp,"buildCompletedUtc":stamp,
                 "tools":{k:"INERT FIXTURE ONLY" for k in ("compilerVersion","toolsetVersion","windowsSdkVersion","cmakeVersion","shaderCompilerVersion")},
                 "commands":[{"phase":p,"argv":["INERT_NOT_EXECUTED"],"cwd":"INERT_FIXTURE_ONLY","startedUtc":stamp,"completedUtc":stamp,"exitCode":0} for p in ("native","shaders")],
                 "logs":[m.identity("fixture-evidence.txt",evidence)],
                 "distributedLogs":[{"path":"fixture-evidence.txt","purpose":"shader-compile"}]}
        m.write_new(b/"release-context.json",m.encoded(context))
        m.record_build(s,b,l)
    def refuses(action,phrase):
        try:action()
        except m.Refusal as e:assert phrase in str(e),(phrase,str(e))
        else:raise AssertionError("Expected refusal: "+phrase)
    try:
        normal=fixture("normal");built(normal);s,b,l,a,r=normal
        case("normal actual implementation / inert bytes",lambda: m.assemble(s,b,l,a,r/"out"))
        def mappings():
            assert (r/"out/Q2RTX VR"/"home/baseq2/autoexec.cfg").read_bytes()==(s/"packaging/defaults/baseq2/autoexec.cfg").read_bytes()
            assert (r/"out/Q2RTX VR"/"home/baseq2/default-vr.cfg").read_bytes()==(s/"packaging/defaults/baseq2/default-vr.cfg").read_bytes()
            assert not (r/"out/Q2RTX VR"/"home/baseq2/q2config.cfg").exists()
            assert (r/"out/Q2RTX VR"/"home/baseq2/pics/b2/arrow.png").read_bytes()==(s/"assets/pics/b2/arrow.png").read_bytes()
        case("actual clean-default and artwork source layout",mappings)

        def doc_selection():
            runtime=r/"out/Q2RTX VR"
            assert (runtime/"docs/guide.md").read_bytes()==(s/"docs/guide.md").read_bytes()
            assert not (runtime/"docs/engineering.md").exists()
            manifest=m.load(runtime/"RELEASE-MANIFEST.json")
            assert manifest["privateCandidate"] is True
            with zipfile.ZipFile(runtime/manifest["sourceArchive"]) as z:
                assert z.read("Q2RTX-VR-source/docs/engineering.md")== (s/"docs/engineering.md").read_bytes()
                assert z.read("Q2RTX-VR-source/docs/guide.md")== (s/"docs/guide.md").read_bytes()
                assert z.read("build-evidence/fixture-evidence.txt")== (b/"fixture-evidence.txt").read_bytes()
        case("explicit user guides and source-only engineering notes",doc_selection)
        def release_identity():
            args=["package","--source",str(s),"--build",str(b),"--libraries",str(l),"--assets",str(a),"--output",str(r/"release"),"--release"]
            assert m.main(args)==0
            manifest=m.load(r/"release/Q2RTX VR/RELEASE-MANIFEST.json")
            assert manifest["privateCandidate"] is False
            assert (r/"release/Q2RTX-VR-v0.8.4-rc.3.zip").exists()
            assert not (r/"release/Q2RTX-VR-v0.8.4-rc.3-audit.zip").exists()
        case("explicit release identity through CLI with inert inputs",release_identity)
        def release_action_refused():
            for action in ("prepare","record"):
                target=r/("invalid-"+action)
                assert m.main([action,"--source",str(s),"--build",str(target),"--libraries",str(l),"--release"])==2
                assert not target.exists()
        case("release flag refused for non-package actions",release_action_refused)
        def changed_doc_receipt():
            parts=fixture("doc-receipt");built(parts);ss,bb,ll,aa,rr=parts
            with (ss/"docs/guide.md").open("ab") as f:f.write(b"changed prose")
            for release in (False,True):
                target=rr/("release" if release else "private")
                refuses(lambda:m.assemble(ss,bb,ll,aa,target,release=release),"Source/library mismatch against build receipt")
                assert not target.exists()
        case("documentation change refuses old receipt in both modes",changed_doc_receipt)
        case("output collision",lambda:refuses(lambda:m.assemble(s,b,l,a,r/"out"),"not empty"))
        case("source overlap",lambda:refuses(lambda:m.assemble(s,b,l,a,s/"out"),"overlaps"))
        def deterministic():
            m.assemble(s,b,l,a,r/"out2")
            assert (r/"out/Q2RTX-VR-v0.8.4-rc.3-audit.zip").read_bytes()==(r/"out2/Q2RTX-VR-v0.8.4-rc.3-audit.zip").read_bytes()
        case("deterministic archive metadata/order",deterministic)
        def source_changed():
            p=fixture("source-changed");ss,bb,ll,aa,rr=p;m.prepare(ss,bb,ll)
            with (ss/"README.md").open("ab") as f:f.write(b"changed")
            refuses(lambda:m.record_build(ss,bb,ll),"inputs changed")
        case("source mismatch since snapshot",source_changed)
        def incomplete():
            ss,bb,ll,aa,rr=fixture("incomplete");m.prepare(ss,bb,ll)
            m.write_new(bb/"bin"/m.PRODUCTS[0],inert_pe())
            refuses(lambda:m.record_build(ss,bb,ll),"Missing input")
        case("incomplete products",incomplete)
        case("old product resource version",lambda:refuses(lambda:built(fixture("old-version"),old=True),"version mismatch"))
        def escaped():
            ss,bb,ll,aa,rr=fixture("traversal",bad_source=True)
            refuses(lambda:m.prepare(ss,bb,ll),"escapes root")
        case("source selection traversal",escaped)
        def asset():
            p=fixture("asset",bad_asset=True);built(p);ss,bb,ll,aa,rr=p
            refuses(lambda:m.assemble(ss,bb,ll,aa,rr/"out"),"asset/default mismatch")
            assert not (rr/"out").exists()
        case("asset mismatch before output",asset)
        path="C:/"+ "Users/"+"SyntheticFixture/private"
        for name,value in [("ascii",path.encode()),("utf16-offset1",b"x"+path.encode("utf-16le"))]:
            def privacy(name=name,value=value):
                p=fixture("privacy-"+name,private=value);built(p);ss,bb,ll,aa,rr=p
                refuses(lambda:m.assemble(ss,bb,ll,aa,rr/"out"),"Privacy scan refused")
                assert not (rr/"out").exists()
            case("privacy "+name,privacy)
        def archive():
            p=fixture("archive-link",archive_link=True);built(p);ss,bb,ll,aa,rr=p
            refuses(lambda:m.assemble(ss,bb,ll,aa,rr/"out"),"Archive symlink")
            assert not (rr/"out").exists()
        case("serialized archive link refusal",archive)
        def player_config_refused():
            p=fixture("player-config-refused");ss,bb,ll,aa,rr=p
            data=b"// INERT PLAYER CONFIG MUST NOT SHIP\n"
            member="packaging/defaults/baseq2/q2config.cfg"
            m.write_new(ss/member,data)
            selected=m.load(ss/m.SOURCE_LIST);selected["paths"].append(member)
            (ss/m.SOURCE_LIST).write_bytes(m.encoded(selected))
            plan=m.load(ss/"packaging/payload-plan.json")
            plan["files"].append({"path":"home/baseq2/q2config.cfg","bytes":len(data),"sha256":m.digest(data)})
            (ss/"packaging/payload-plan.json").write_bytes(m.encoded(plan))
            built(p)
            refuses(lambda:m.assemble(ss,bb,ll,aa,rr/"out"),"Player q2config.cfg must never be packaged")
        case("player settings cannot enter package",player_config_refused)
        def product_changed():
            p=fixture("changed-product");built(p);ss,bb,ll,aa,rr=p
            with (bb/"bin/q2rtx.exe").open("ab") as f:f.write(b"changed")
            refuses(lambda:m.assemble(ss,bb,ll,aa,rr/"out"),"Product mismatch")
        case("product mismatch against receipt",product_changed)
        def classification():
            raw=("C:/"+ "Users/"+"PublicExample/library").encode()
            exceptions=[{"path":"example","sha256":m.digest(raw),"characters":len(raw),"matchPrefixSha256":m.digest(raw),"reason":"Labelled inert example"}]
            ev={"scannedFiles":0,"scannedBytes":0,"neutralBuildPathOccurrences":0,"findings":[]}
            m.scan_blob("example",raw,exceptions,ev);assert ev["findings"][0]["kind"]=="vendor-example"
            ev["findings"]=[]
            m.scan_blob("example",raw+b"changed",exceptions,ev);assert ev["findings"][0]["kind"]=="personal-path"
        case("exact exception and changed-file refusal",classification)
        def author_classification():
            raw=("C:/"+ "Users/"+"UpstreamAuthor/example").encode()
            e=[{"path":"example","sha256":m.digest(raw),"characters":len(raw),"matchPrefixSha256":m.digest(raw),"kind":"upstream-author-path","reason":"Inert labelled author-path case"}]
            ev={"scannedFiles":0,"scannedBytes":0,"neutralBuildPathOccurrences":0,"findings":[]}
            m.scan_blob("example",raw,e,ev);m.privacy_result(ev)
            assert ev["personalDataIncluded"] and ev["retainedUpstreamAuthorPathFindings"]==1 and not ev["unclassifiedPersonalPathFindings"]
            ev["findings"]=[]
            m.scan_blob("example",raw+b"changed",e,ev)
            refuses(lambda:m.privacy_result(ev),"Privacy scan refused")
        case("retained author path remains personal data",author_classification)

        case("non-string source member",lambda:refuses(lambda:m.unique([1]),"must be strings"))
        case("control character member",lambda:refuses(lambda:m.relative("bad"+chr(10)+"name"),"Control character"))

    finally:
        m.write_new(output/"RESULTS.json",m.encoded({"scope":"Small inert labelled inputs through actual packager functions, no substituted returns, no compiler/product execution. No OS reparse point created; actual path guard source-reviewed and archive-link refusal exercised.",
          "results":results,"productionSourceOrProductsUsed":False,"passed":len(results)==23 and all(r["passed"] for r in results)}))
    print(json.dumps({"fixtureCases":len(results),"passed":all(r["passed"] for r in results),"report":str(output/"RESULTS.json")}))

if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__);p.add_argument("--output",required=True,type=Path)
    run(p.parse_args().output)
