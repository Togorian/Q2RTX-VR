#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# Design/direction by Togorian; implementation with Codex assistance.
# Modified 11 November 2026: explicit release inputs, manifests and scoped privacy evidence.
"""Prepare build input hashes, record built products, or assemble a runtime package (private by default).

Python standard library only. Never builds/executes products or discovers user data.
See docs/PACKAGING.md. Input hashes bind observed files, not a build attestation.
"""
import argparse
import datetime
import hashlib
import io
import json
import os
from pathlib import Path, PurePosixPath
import re
import stat
import struct
import sys
import tarfile
import zipfile

PRODUCTS = ("q2rtx.exe", "baseq2/gamex86_64.dll", "Q2RTX VR.exe")
LIBRARIES = ("libcurl.lib", "OpenAL32.lib", "SDL2-static.lib", "SDL2main.lib", "zlibstatic.lib")
VERSION_RE = re.compile(r"(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(?:-rc\.([1-9][0-9]*))?")
SOURCE_LIST = "packaging/source-files.json"
MAX_FILE = 64 * 1024 * 1024
MAX_ARCHIVE_EXPANDED = 512 * 1024 * 1024
PERSONAL = re.compile(r"(?:[A-Za-z]:[\\/]+Users[\\/]+[^\x00\r\n\"<>]{1,180}|/" r"Users/[^\x00\r\n\"<>]{1,180}|/" r"home/[^\x00\r\n\"<>]{1,180}|\.codex[\\/]+worktrees[\\/]+[^\x00\r\n\"<>]{1,180})", re.I)
NEUTRAL = re.compile(r"[A-Za-z]:[\\/]+(?:Codex|build|work)[\\/]+[^\x00\r\n\"<>]{1,180}", re.I)

class Refusal(ValueError):
    pass

def require(condition, message):
    if not condition:
        raise Refusal(message)

def digest(data):
    return hashlib.sha256(data).hexdigest()

def encoded(value):
    return (json.dumps(value, ensure_ascii=False, indent=2, sort_keys=True) + "\n").encode()

def relative(name):
    require(isinstance(name, str) and "\\" not in name and ":" not in name, "Non-portable member path")
    require(not any(ord(c) < 32 for c in name), "Control character in member path")
    p = PurePosixPath(name)
    require(name and not p.is_absolute() and all(x not in ("", ".", "..") for x in name.split("/")), "Member path escapes root")
    require(all(not x.endswith((" ", ".")) for x in p.parts), "Ambiguous Windows member path")
    require(all(not re.fullmatch(r"(?i)(con|prn|aux|nul|com[1-9]|lpt[1-9])(?:\..*)?", x) for x in p.parts), "Reserved Windows member path")
    return p

def ordinary(path, file=True):
    path = Path(path).absolute()
    for p in (path, *path.parents):
        if p.exists() or p.is_symlink():
            st = p.lstat()
            require(not stat.S_ISLNK(st.st_mode) and not (getattr(st, "st_file_attributes", 0) & 0x400), "Reparse/symlink input: " + p.name)
    if file:
        require(path.is_file(), "Missing input: " + path.name)
        require(path.stat().st_size <= MAX_FILE, "Oversized input: " + path.name)
    return path

def load(path):
    return json.loads(ordinary(path).read_bytes())

def read_member(root, name):
    relative(name)
    return ordinary(Path(root) / name).read_bytes()

def unique(names):
    require(all(isinstance(n, str) for n in names), "Member names must be strings")
    require(len(names) == len(set(n.casefold() for n in names)), "Duplicate/case-colliding members")

def version(source):
    value = read_member(source, "VERSION").decode("ascii").strip()
    m = VERSION_RE.fullmatch(value)
    require(m is not None and all(int(x or 0) <= 65535 for x in m.groups()), "Invalid VERSION")
    return value, tuple(int(x or 0) for x in m.groups())

def disjoint(output, inputs):
    out = ordinary(output, file=False).resolve()
    for value in inputs:
        p = ordinary(value, file=False).resolve()
        require(not (out == p or out in p.parents or p in out.parents), "Output overlaps an input tree")
    require(not out.exists() or (out.is_dir() and not any(out.iterdir())), "Output is not empty")
    return out

def source_names(source):
    paths = load(Path(source) / SOURCE_LIST)["paths"]
    require(isinstance(paths, list), "Source member list must be an array")
    names = paths + [SOURCE_LIST]
    unique(names)
    for name in names:
        p = relative(name)
        require(p.parts[0].casefold() not in (".git", ".codex", ".claude", "audit", "reports", "home", "saves", "logs", "before", "build", "build-deps"), "Private/build member selected")
        require(not name.lower().endswith((".log", ".pdb", ".obj", ".exe", ".dll", ".lib", ".spv")), "Build/runtime output selected as source")
        require(Path(name).name.upper() != "START-Q2RTX-VR.CMD", "Old CMD launcher selected")
        ordinary(Path(source) / name)
    return sorted(names)

def identity(name, data):
    return {"path": name, "bytes": len(data), "sha256": digest(data)}

def input_record(source, libraries):
    v, _ = version(source)
    files = [identity(n, read_member(source, n)) for n in source_names(source)]
    libs = [identity(n, read_member(libraries, n)) for n in LIBRARIES]
    sdk = identity("vulkan-1.lib", read_member(libraries, "vulkan-1.lib"))
    return {"schema": 1, "version": v, "source": files, "libraries": libs, "externalSdkImport": sdk}

def write_new(path, data):
    path = Path(path)
    ordinary(path.parent, file=False)
    require(not path.exists(), "Refusing to replace " + path.name)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("xb") as f:
        f.write(data)

def prepare(source, build, libraries):
    build = disjoint(build, (source, libraries))
    record = input_record(source, libraries)
    write_new(build / "release-inputs.json", encoded(record))
    return {"status": "INPUTS_SNAPSHOTTED_NOT_BUILT", "version": record["version"], "sourceFiles": len(record["source"])}

def pe_version(data):
    """Read AMD64 PE32+ version resources without loading or executing the image."""
    require(len(data) >= 256 and data[:2] == b"MZ", "Not a PE product")
    pe = struct.unpack_from("<I", data, 0x3c)[0]
    require(pe + 264 <= len(data) and data[pe:pe+4] == b"PE\0\0", "Invalid PE header")
    machine, count = struct.unpack_from("<HH", data, pe+4)
    optional_size = struct.unpack_from("<H", data, pe+20)[0]
    opt = pe+24
    require(machine == 0x8664 and 0 < count <= 96 and optional_size >= 136 and struct.unpack_from("<H", data, opt)[0] == 0x20b, "Expected AMD64 PE32+")
    resource_rva, resource_size = struct.unpack_from("<II", data, opt+128)
    sections = opt+optional_size
    def offset(rva, size):
        for i in range(count):
            p = sections+40*i
            require(p+40 <= len(data), "Truncated PE sections")
            vs, va, raw, at = struct.unpack_from("<IIII", data, p+8)
            if va <= rva and rva+size <= va+raw:
                require(at+(rva-va)+size <= len(data), "Truncated PE resource")
                return at+rva-va
        raise Refusal("Unmapped PE resource")
    root = offset(resource_rva, resource_size)
    def entries(at):
        require(0 <= at and at+16 <= resource_size, "Invalid resource directory")
        named, ids = struct.unpack_from("<HH", data, root+at+12)
        require(named+ids <= 256 and at+16+8*(named+ids) <= resource_size, "Oversized resource directory")
        return [struct.unpack_from("<II", data, root+at+16+8*i) for i in range(named+ids)]
    types = [to for key,to in entries(0) if key == 16]
    require(len(types) == 1 and types[0] & 0x80000000, "Missing version resource")
    leaves = []
    for _,name in entries(types[0] & 0x7fffffff):
        require(name & 0x80000000, "Invalid version name")
        for _,leaf in entries(name & 0x7fffffff):
            require(not leaf & 0x80000000 and leaf+16 <= resource_size, "Invalid version leaf")
            rva, size = struct.unpack_from("<II", data, root+leaf)
            require(size <= 65536, "Oversized version resource")
            leaves.append(data[offset(rva,size):offset(rva,size)+size])
    require(len(leaves) == 1, "Ambiguous version resources")
    buf = leaves[0]; strings = {}; fixed = None
    def block(at, end, depth):
        nonlocal fixed
        require(depth <= 6 and at+6 <= end, "Invalid version block")
        length, values, kind = struct.unpack_from("<HHH", buf, at)
        require(length >= 6 and at+length <= end, "Truncated version block")
        k = at+6
        while k+2 <= at+length and buf[k:k+2] != b"\0\0": k += 2
        require(k+2 <= at+length, "Unterminated version key")
        key = buf[at+6:k].decode("utf-16le")
        start = (k+2+3)&~3; size = values*2 if kind else values
        require(start+size <= at+length, "Invalid version value")
        value = buf[start:start+size]
        if key == "VS_VERSION_INFO":
            require(len(value) >= 52 and struct.unpack_from("<I",value)[0] == 0xfeef04bd, "Missing fixed version")
            fixed = struct.unpack_from("<IIII",value,8)
            strings["_FileFlags"] = struct.unpack_from("<I",value,28)[0]
        elif key in ("FileVersion","ProductVersion","ProductName","CompanyName","LegalCopyright"):
            strings[key] = value.decode("utf-16le").rstrip("\0")
        child = (start+size+3)&~3
        while child+6 <= at+length:
            n = block(child,at+length,depth+1)
            child = (child+n+3)&~3
        return length
    block(0,len(buf),0)
    require(fixed is not None, "Missing version data")
    return strings, ((fixed[0]>>16,fixed[0]&65535,fixed[1]>>16,fixed[1]&65535),
                     (fixed[2]>>16,fixed[2]&65535,fixed[3]>>16,fixed[3]&65535))

def product_records(source, build):
    v, nums = version(source); records = []
    for name in PRODUCTS:
        data = read_member(Path(build)/"bin",name)
        strings, numeric = pe_version(data)
        require(strings.get("FileVersion") == v and strings.get("ProductVersion") == v and strings.get("ProductName") == "Q2RTX VR" and numeric == (nums,nums), "Product version mismatch: "+name)
        require(strings.get("CompanyName") == "Togorian" and "Copyright (C) 2026 Togorian" in strings.get("LegalCopyright", ""), "Product attribution mismatch: "+name)
        require(name == "Q2RTX VR.exe" or "NVIDIA CORPORATION" in strings.get("LegalCopyright", ""), "Missing upstream product notice: "+name)
        require(bool(strings["_FileFlags"] & 2) == bool(nums[3]), "Product prerelease flag mismatch: "+name)
        records.append(identity(name,data))
    return records

def build_context(build, snapshot):
    """Validate the operator's completed-run observations and retained evidence.

    Values come from the actual invocation, Git and tool output. This is a
    checked observation record, not independent compiler attestation.
    """
    context = load(Path(build)/"release-context.json")
    require(context.get("schema") == 1, "Unsupported build context")
    require(context.get("inputsSha256") == digest(encoded(snapshot)), "Context input snapshot mismatch")
    require(re.fullmatch(r"[0-9a-f]{40}", context.get("sourceCommit", "")), "Missing source commit")
    def utc(value):
        require(isinstance(value, str) and value.endswith("Z"), "Expected UTC timestamp")
        return datetime.datetime.fromisoformat(value[:-1]+"+00:00")
    utc(context["sourceCommitUtc"])
    started = utc(context["buildStartedUtc"])
    completed = utc(context["buildCompletedUtc"])
    require(started <= completed, "Reversed build timestamps")
    for key in ("compilerVersion", "toolsetVersion", "windowsSdkVersion", "cmakeVersion", "shaderCompilerVersion"):
        require(isinstance(context["tools"].get(key), str) and context["tools"][key].strip(), "Missing observed tool version: "+key)
    commands = context["commands"]
    require(isinstance(commands, list) and commands and {c["phase"] for c in commands} >= {"native", "shaders"}, "Missing completed build commands")
    for command in commands:
        require(command["exitCode"] == 0, "Build command did not succeed")
        require(isinstance(command["argv"], list) and command["argv"] and all(isinstance(x,str) and x for x in command["argv"]), "Invalid actual command")
        require(isinstance(command["cwd"], str) and command["cwd"], "Missing command working directory")
        require(started <= utc(command["startedUtc"]) <= utc(command["completedUtc"]) <= completed, "Command outside build interval")
    logs = context["logs"]
    require(isinstance(logs, list) and logs, "Missing retained build evidence")
    unique([p["path"] for p in logs])
    for pin in logs:
        require(identity(pin["path"], read_member(build, pin["path"])) == pin, "Changed build evidence: "+pin["path"])
    distributed = context["distributedLogs"]
    require(isinstance(distributed, list) and distributed, "Missing distributed shader compile log")
    unique([p["path"] for p in distributed])
    require(any(p.get("purpose") == "shader-compile" for p in distributed), "Missing distributed shader compile log")
    require(all(p["path"] in {log["path"] for log in logs} for p in distributed), "Distributed log has no retained evidence pin")
    return context


def decoded_array(data, name):
    text = data.decode("utf-8")
    matches = re.findall(r"\b"+re.escape(name)+r"\s*\[\s*\]\s*=\s*\{([^}]+)\}\s*;", text, re.S)
    require(len(matches) == 1, "Missing or ambiguous embedded array: "+name)
    tokens = [x.strip() for x in matches[0].split(",") if x.strip()]
    require(tokens and all(re.fullmatch(r"0x[0-9a-fA-F]{8}[uU]?", x) for x in tokens), "Unexpected embedded array format")
    return b"".join(struct.pack("<I", int(x.rstrip("uU"), 16)) for x in tokens)


def shader_records(source, shaders):
    """Bind actual fresh outputs to selected pins and compiled-in C arrays."""
    plan = load(Path(source)/"recipes/shader-plan.json")
    runtime = []; embedded = []
    def input_pin(item):
        data = read_member(source, item["source"])
        size = item.get("currentSourceBytes", item.get("sourceBytes"))
        value = item.get("currentSourceSha256", item.get("sourceSha256"))
        require((len(data), digest(data)) == (size, value), "Selected shader source pin mismatch: "+item["source"])
    for item in plan["shipped"]:
        input_pin(item)
        name = "shaders/"+item["output"]
        data = read_member(shaders, name)
        require((len(data), digest(data)) == (item["referenceBytes"], item["referenceSha256"]), "Fresh runtime shader differs from selected pin: "+name)
        runtime.append(identity(name, data))
    unique([p["path"] for p in runtime])
    for header in plan["embedded"]:
        generated = "generated/"+header["header"]
        generated_data = read_member(shaders, generated)
        require(header["selectedCopies"], "No selected embedded header")
        for item in header["arrays"]:
            input_pin(item)
            name = "embedded/"+item["array"]+".spv"
            data = read_member(shaders, name)
            require((len(data), digest(data)) == (item["bytes"], item["sha256"]), "Fresh embedded shader differs from selected pin: "+name)
            require(decoded_array(generated_data,item["array"]) == data, "Generated header differs from fresh module")
            selected = []
            for path in header["selectedCopies"]:
                original = read_member(source,path)
                require(decoded_array(original,item["array"]) == data, "Selected native header differs from fresh module: "+path)
                selected.append(identity(path, original))
            embedded.append({**identity(name,data),"selectedHeaders":selected,"generatedHeader":identity(generated,generated_data)})
    unique([p["path"] for p in embedded])
    return {"runtime":runtime,"embedded":embedded,"scope":"Exact output/reference and decoded-array equality, not semantic, GPU or headset validation. Compile origin is documented by retained completed-run context/logs."}


def record_build(source, build, libraries, *, shaders=None):
    snapshot = load(Path(build)/"release-inputs.json")
    require(snapshot == input_record(source,libraries), "Build inputs changed since snapshot")
    receipt = {"schema":2,"scope":"Observed completed-run context, evidence and input/product identities; not independent compiler attestation, binary reproducibility or a headset claim.",
               "context":build_context(build,snapshot),"inputs":snapshot,"products":product_records(source,build),
               "shaders":shader_records(source,shaders or Path(build)/"shader-output")}
    write_new(Path(build)/"release-build.json", encoded(receipt))
    return {"status":"BUILD_BYTES_RECORDED_NOT_RUNTIME_TESTED","products":receipt["products"]}

def scan_blob(name, data, exceptions, evidence, depth=0):
    """ASCII and both UTF-16LE alignments; bounded retained dependency archives too."""
    require(len(data) <= MAX_FILE, "Oversized scan member")
    key = digest(data); seen = set()
    for pos in range(0,len(data),1024*1024):
        chunk = data[max(0,pos-1024):pos+1024*1024]
        texts = [chunk.decode("latin1"),chunk[:len(chunk)//2*2].decode("utf-16le","ignore"),chunk[1:1+(len(chunk)-1)//2*2].decode("utf-16le","ignore")]
        for text in texts:
            for match in PERSONAL.finditer(text):
                value=match.group(0)
                if ("personal",value) in seen:continue
                seen.add(("personal",value))
                exception = next((e for e in exceptions if e["path"]==name and e["sha256"]==key and digest(value[:e["characters"]].encode())==e["matchPrefixSha256"]),None)
                evidence["findings"].append({"path":name,"sha256":key,"kind":exception.get("kind","vendor-example") if exception else "personal-path","match":value[:240],"reason":exception["reason"] if exception else "Unclassified user/worktree path"})
            for match in NEUTRAL.finditer(text):
                value=match.group(0)
                if ("neutral",value) not in seen:
                    seen.add(("neutral",value));evidence["neutralBuildPathOccurrences"] += 1
                    evidence.setdefault("neutralBuildPaths",[]).append({"path":name,"sha256":key,"match":value[:240]})
    evidence["scannedFiles"] += 1;evidence["scannedBytes"] += len(data)
    if name.lower().endswith((".tar.gz",".tgz",".zip")):
        require(depth < 2, "Nested archive review depth exceeded: "+name)
        total=0
        if name.lower().endswith(".zip"):
            with zipfile.ZipFile(io.BytesIO(data)) as z:
                unique([i.filename.rstrip("/") for i in z.infolist()])
                for item in z.infolist():
                    relative(item.filename.rstrip("/"))
                    require(not stat.S_ISLNK(item.external_attr>>16), "Archive symlink")
                    if item.is_dir():continue
                    total+=item.file_size;require(total<=MAX_ARCHIVE_EXPANDED and item.file_size<=MAX_FILE,"Archive scan size bound")
                    scan_blob(name+"!/"+item.filename,z.read(item),exceptions,evidence,depth+1)
        else:
            with tarfile.open(fileobj=io.BytesIO(data),mode="r:gz") as t:
                for item in t:
                    relative(item.name.rstrip("/"))
                    if item.isdir():continue
                    require(item.isfile(), "Archive link/special member")
                    total+=item.size;require(total<=MAX_ARCHIVE_EXPANDED and item.size<=MAX_FILE,"Archive scan size bound")
                    scan_blob(name+"!/"+item.name,t.extractfile(item).read(),exceptions,evidence,depth+1)

def privacy_result(evidence):
    rejected=[f for f in evidence["findings"] if f["kind"]=="personal-path"]
    upstream=[f for f in evidence["findings"] if f["kind"]=="upstream-author-path"]
    evidence["unclassifiedPersonalPathFindings"]=len(rejected)
    evidence["retainedUpstreamAuthorPathFindings"]=len(upstream)
    evidence["personalDataIncluded"]=bool(rejected or upstream)
    evidence["personalDataMeaning"]="True includes retained, exactly classified upstream author paths; examples, CI paths, tokens and neutral build paths are recorded separately. Unknown user/worktree paths refuse packaging."
    require(not rejected,"Privacy scan refused: "+json.dumps(rejected[:8],ensure_ascii=False))
    return evidence


def write_zip(path, members):
    unique(list(members))
    with zipfile.ZipFile(path,"x",compression=zipfile.ZIP_DEFLATED,compresslevel=9,allowZip64=True) as z:
        for name in sorted(members):
            relative(name);data=members[name]() if callable(members[name]) else members[name]
            info=zipfile.ZipInfo(name,(1980,1,1,0,0,0));info.create_system=3;info.external_attr=0o100644<<16
            info.compress_type=zipfile.ZIP_DEFLATED
            z.writestr(info,data,compress_type=zipfile.ZIP_DEFLATED,compresslevel=9)
    with zipfile.ZipFile(path) as z:
        require(z.namelist()==sorted(members),"Archive membership mismatch")
        for info in z.infolist():
            expected=members[info.filename]() if callable(members[info.filename]) else members[info.filename]
            require(z.read(info)==expected,"Archive byte mismatch: "+info.filename)

def assemble(source, build, libraries, assets, output, *, release=False, shaders=None):
    source=Path(source);build=Path(build);libraries=Path(libraries);assets=Path(assets)
    output=disjoint(output,(source,build,libraries,assets,shaders or build/"shader-output"))
    v,_=version(source);snapshot=input_record(source,libraries)
    receipt=load(build/"release-build.json")
    require(receipt["inputs"]==snapshot,"Source/library mismatch against build receipt")
    require(receipt["products"]==product_records(source,build),"Product mismatch against build receipt")
    require(receipt.get("schema")==2 and receipt["context"]==build_context(build,snapshot),"Build context mismatch against receipt")
    shaders=Path(shaders) if shaders is not None else build/"shader-output"
    current_shaders=shader_records(source,shaders)
    require(receipt["shaders"]==current_shaders,"Shader output mismatch against receipt")
    plan=load(source/"packaging/payload-plan.json")
    selected_runtime={"home/baseq2/shader_vkpt/"+p["path"].removeprefix("shaders/"):p for p in current_shaders["runtime"]}
    require(set(selected_runtime)=={f["path"] for f in plan["files"] if f.get("ship",True) and f["path"].endswith(".spv")},"Runtime shader selection mismatch")
    exceptions=load(source/"packaging/privacy-exceptions.json")["exceptions"]
    evidence={"scope":"Selected input bytes, ASCII and both UTF-16LE alignments; source/package archive members, including bounded nested archives. No user-home/network discovery. Scoped patterns are not universal secret detection.",
              "patterns":["Windows Users paths","Unix user-home paths",".codex/worktrees paths"],"scannedFiles":0,"scannedBytes":0,"neutralBuildPathOccurrences":0,"findings":[]}
    source_members={}
    def checked(root,name,pin):
        b=read_member(root,name);require(identity(name,b)==pin,"Input changed while packaging: "+name);return b
    for pin in snapshot["source"]:
        name=pin["path"]
        source_members["Q2RTX-VR-source/"+name]=lambda n=name,p=pin:checked(source,n,p)
    for pin in snapshot["libraries"]:
        name=pin["path"]
        source_members["relinking-libraries/"+name]=lambda n=name,p=pin:checked(libraries,n,p)
    source_members["BUILD-RECORD.json"]=encoded(receipt)
    for entry in receipt["context"]["distributedLogs"]:
        name=entry["path"];relative(name)
        pin=next(p for p in receipt["context"]["logs"] if p["path"]==name)
        source_members["build-evidence/"+name]=lambda n=name,p=pin:checked(build,n,p)
    source_members["relinking-libraries/SDK-PREREQUISITE.txt"]=b"Supply the compatible Vulkan SDK vulkan-1.lib separately; it is not redistributed here.\n"
    source_manifest=[]
    for name,provider in source_members.items():
        b=provider() if callable(provider) else provider
        source_manifest.append(identity(name,b))
        scan_blob(name,b,exceptions,evidence)
    source_members["SOURCE-MANIFEST.json"]=encoded({"schema":1,"version":v,"members":sorted(source_manifest,key=lambda x:x["path"]),"selfExcluded":True})
    runtime={}
    for f in plan["files"]:
        if not f.get("ship",True):continue
        name=f["path"];relative(name)
        require(name.casefold() != "home/baseq2/q2config.cfg", "Player q2config.cfg must never be packaged")
        if name in PRODUCTS:
            pin=next(p for p in receipt["products"] if p["path"]==name)
            b=checked(build/"bin",name,pin)
        else:
            if name.startswith("home/baseq2/pics/b2/"):root=source;src="assets/pics/b2/"+Path(name).name
            elif name.startswith("home/baseq2/") and name.endswith(".cfg"):root=source;src="packaging/defaults/"+name.removeprefix("home/")
            elif name in selected_runtime:root=shaders;src=selected_runtime[name]["path"]
            else:root=assets;src=name
            b=read_member(root,src)
            require((len(b),digest(b))==(f["bytes"],f["sha256"]),"Accepted asset/default mismatch: "+name)
        require(name not in runtime,"Duplicate runtime member");runtime[name]=b
    substitutions={"@Q2RTX_VERSION@":v,"@Q2RTX_TAG@":"v"+v,"@Q2RTX_LOG_FILE@":"console"+v+".log"}
    known=[x["path"] for x in snapshot["source"]]
    for dest,src in plan["runtimeTemplates"].items():
        pairs=[(dest+n[len(src):],n) for n in known if n.startswith(src)] if src.endswith("/") else [(dest,src)]
        require(pairs,"Empty documentation mapping")
        for dest,src in pairs:
            relative(dest);require(src in known and dest not in runtime,"Invalid/duplicate documentation mapping")
            b=read_member(source,src)
            if src.endswith(".md"):
                t=b.decode("utf-8")
                for old,new in substitutions.items():t=t.replace(old,new)
                require(not re.search(r"@Q2RTX_[A-Z_]+@",t),"Unresolved template variable")
                b=t.encode()
            runtime[dest]=b
    for name,b in runtime.items():scan_blob("Q2RTX VR/"+name,b,exceptions,evidence)
    privacy_result(evidence)
    # Only now create an absent/empty output. Failures are retained, never cleaned
    # over another candidate; source/build inputs remain read-only.
    output=disjoint(output,(source,build,libraries,assets,shaders or build/"shader-output"))
    output.mkdir(parents=True,exist_ok=True);runtime_root=output/"Q2RTX VR";runtime_root.mkdir()
    for name,b in runtime.items():write_new(runtime_root/name,b)
    source_name="corresponding-source-v"+v+".zip"
    write_zip(runtime_root/source_name,source_members)
    runtime[source_name]=(runtime_root/source_name).read_bytes()
    members=[identity(n,b) for n,b in sorted(runtime.items())]
    manifest={"schema":1,"version":v,"privateCandidate":not release,"sourceArchive":source_name,"members":members,"privacy":evidence,
              "selfExcluded":["RELEASE-MANIFEST.json","SHA256SUMS.txt"],"reproducibility":"Sorted members, fixed ZIP metadata and compression level. Product bit-identical reproduction and headset acceptance are not claimed."}
    manifest_bytes=encoded(manifest);write_new(runtime_root/"RELEASE-MANIFEST.json",manifest_bytes)
    runtime["RELEASE-MANIFEST.json"]=manifest_bytes
    sums="".join(digest(b)+"  "+n+"\n" for n,b in sorted(runtime.items())).encode()
    write_new(runtime_root/"SHA256SUMS.txt",sums);runtime["SHA256SUMS.txt"]=sums
    outer=output/("Q2RTX-VR-v"+v+(".zip" if release else "-audit.zip"))
    write_zip(outer,{"Q2RTX VR/"+n:b for n,b in runtime.items()})
    write_new(output/(outer.name+".sha256"),(digest(outer.read_bytes())+"  "+outer.name+"\n").encode())
    return {"status":("RELEASE_PACKAGE_BYTES_VERIFIED_NO_RUNTIME_ACCEPTANCE" if release else "PRIVATE_PACKAGE_BYTES_VERIFIED_NO_RUNTIME_ACCEPTANCE"),"version":v,"runtimeFiles":len(runtime),"sourceMembers":len(source_members),"privacy":evidence,"archive":outer.name,"archiveBytes":outer.stat().st_size,"archiveSha256":digest(outer.read_bytes())}

def main(argv=None):
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("action",choices=("prepare","record","package","verify-shaders"))
    p.add_argument("--source",type=Path,default=Path(__file__).resolve().parents[1])
    p.add_argument("--build",type=Path,required=True)
    p.add_argument("--libraries",type=Path,required=True)
    p.add_argument("--assets",type=Path);p.add_argument("--output",type=Path)
    p.add_argument("--shaders",type=Path,help="Fresh shader-only build outputs; defaults to <build>/shader-output")
    p.add_argument("--release",action="store_true",help="Explicit release package identity; package action only. Default remains private.")
    a=p.parse_args(argv)
    try:
        require(not a.release or a.action=="package", "--release is only valid for package")
        if a.action=="prepare":result=prepare(a.source,a.build,a.libraries)
        elif a.action=="record":result=record_build(a.source,a.build,a.libraries,shaders=a.shaders)
        elif a.action=="verify-shaders":result=shader_records(a.source,a.shaders or a.build/"shader-output")
        else:
            require(a.assets is not None and a.output is not None,"package needs --assets and --output")
            result=assemble(a.source,a.build,a.libraries,a.assets,a.output,release=a.release,shaders=a.shaders)
        print(json.dumps(result,ensure_ascii=False,sort_keys=True))
        return 0
    except (Refusal,OSError,ValueError,KeyError,zipfile.BadZipFile,tarfile.TarError,struct.error) as e:
        print(json.dumps({"status":"REFUSED","reason":str(e)},ensure_ascii=False),file=sys.stderr)
        return 2

if __name__=="__main__":
    raise SystemExit(main())
