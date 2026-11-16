# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# SPDX-License-Identifier: GPL-2.0-or-later
# Inspection-only parser for the accepted MD2/MD3 archive members.
import struct,math
def load_model(data):
 if data[:4]==b'IDP3':
  h=struct.unpack_from('<4si64s9i',data);_,version,name,flags,nf,tags,nm,ns,of,ot,om,end=h
  assert version==15 and tags==0 and end<=len(data)
  meshes=[];off=om
  for mi in range(nm):
   _,name,flags,frames,skins,nv,nt,tri,skin,uv,verts,size=struct.unpack_from('<4s64s10i',data,off)
   assert frames==nf and off+size<=end and nv>0
   positions=[[[v/64 for v in struct.unpack_from('<3h',data,off+verts+(f*nv+i)*8)] for i in range(nv)] for f in range(nf)]
   triangles=[list(struct.unpack_from('<3i',data,off+tri+i*12)) for i in range(nt)]
   assert all(0<=v<nv for t in triangles for v in t)
   shaders=[data[off+skin+i*68:off+skin+i*68+64].split(b'\0')[0].decode() for i in range(skins)]
   meshes.append(dict(id=mi,name=name.split(b'\0')[0].decode(),positions=positions,triangles=triangles,shaders=shaders))
   off+=size
  return dict(format='md3',frames=nf,meshes=meshes,axisScale='signed XYZ shorts /64; no baked axis permutation',frameNames=[data[of+f*56+40:of+f*56+56].split(b'\0')[0].decode() for f in range(nf)])
 assert data[:4]==b'IDP2'
 _,ver,sw,sh,fs,skins,nv,nst,nt,ng,nf,os,ost,ot,of,og,end=struct.unpack_from('<17i',data);assert ver==8 and end<=len(data)
 triangles=[list(struct.unpack_from('<3H',data,ot+i*12)) for i in range(nt)]
 positions=[];frame_names=[]
 for f in range(nf):
  off=of+f*fs;s=struct.unpack_from('<3f',data,off);t=struct.unpack_from('<3f',data,off+12)
  frame_names.append(data[off+24:off+40].split(b'\0')[0].decode())
  positions.append([[data[off+40+i*4+k]*s[k]+t[k] for k in range(3)] for i in range(nv)])
 return dict(format='md2',frames=nf,meshes=[dict(id=0,name='md2-original-vertex-order',positions=positions,triangles=triangles,shaders=[data[os+i*64:os+(i+1)*64].split(b'\0')[0].decode() for i in range(skins)])],axisScale='per-frame byteXYZ * scaleXYZ + translateXYZ; UV seam expansion does not change positions',frameNames=frame_names)
def dot(a,b):return sum(x*y for x,y in zip(a,b))
def sub(a,b):return [x-y for x,y in zip(a,b)]
def add(a,b):return [x+y for x,y in zip(a,b)]
def mul(a,s):return [x*s for x in a]
def cross(a,b):return [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]]
def length(a):return math.sqrt(dot(a,a))
def unit(a):
 n=length(a);assert math.isfinite(n) and n>1e-8;return mul(a,1/n)
def centroid(vertices):return [sum(v[i] for v in vertices)/len(vertices) for i in range(3)]
def bounds(vertices):return dict(min=[min(v[i] for v in vertices) for i in range(3)],max=[max(v[i] for v in vertices) for i in range(3)])