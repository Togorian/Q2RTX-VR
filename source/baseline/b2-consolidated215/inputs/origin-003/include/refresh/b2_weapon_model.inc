// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
// Modified for Q2RTX VR on 2026-11-10: validate topology from the exact owned raw asset.
// Owned by the actual VKPT model loader; opaque bindings avoid ABI layout edits.
#include "b2_weapon_model.h"
#include <float.h>
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#endif
typedef struct { unsigned mesh, count; const uint16_t *vertices; } b2wm_group_t;
typedef struct { unsigned count; const b2wm_group_t *groups; } b2wm_reference_t;
typedef struct { unsigned vertices, triangles; } b2wm_mesh_spec_t;
typedef struct {
    const char *name; unsigned slot, bytes, frames, ready, meshes, mode;
    float scale; unsigned char digest[32]; const b2wm_mesh_spec_t *topology;
    b2wm_reference_t refs[8];
} b2wm_spec_t;
#include "b2_weapon_model_data.inc"
enum { B2WM_BINDINGS=64 };
typedef struct {
    int vertices, triangles, indices;
    const vec3_t *positions;
    const int *elements;
} b2wm_mesh_binding_t;
typedef struct {
    const model_t *model;
    const maliasmesh_t *meshes;
    const maliasframe_t *frames;
    b2wm_mesh_binding_t layout[MD3_MAX_MESHES];
    const b2wm_spec_t *spec;
    b2wm_info_t info;
    uint16_t original_to_loaded[MD2_MAX_VERTS];
} b2wm_binding_t;
static b2wm_binding_t b2wm_bindings[B2WM_BINDINGS];
static uint64_t b2wm_generation;

void B2WM_Forget(const void *model)
{
    for (unsigned i=0;i<B2WM_BINDINGS;i++)
        if (b2wm_bindings[i].model==model) memset(b2wm_bindings+i,0,sizeof(b2wm_bindings[i]));
}
static bool b2wm_digest(const void *data, unsigned length, unsigned char digest[32])
{
#ifdef _WIN32
    // In-memory adaptation of accepted reference_b_b0u.c's BCrypt SHA-256 pattern.
    BCRYPT_ALG_HANDLE algorithm=NULL; BCRYPT_HASH_HANDLE hash=NULL;
    unsigned char *object=NULL; DWORD size=0, used=0; bool ok=false;
    if (!data || BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,NULL,0)<0) goto done;
    if (BCryptGetProperty(algorithm,BCRYPT_OBJECT_LENGTH,(PUCHAR)&size,sizeof(size),&used,0)<0 || !size) goto done;
    object=malloc(size); if (!object) goto done;
    if (BCryptCreateHash(algorithm,&hash,object,size,NULL,0,0)<0 ||
        BCryptHashData(hash,(PUCHAR)data,length,0)<0 || BCryptFinishHash(hash,digest,32,0)<0) goto done;
    ok=true;
done:
    if (hash) BCryptDestroyHash(hash);
    if (algorithm) BCryptCloseAlgorithmProvider(algorithm,0);
    free(object); return ok;
#else
    (void)data;(void)length;(void)digest;return false;
#endif
}
static b2wm_binding_t *b2wm_find(const void *pointer, const b2wm_key_t *key)
{
    if (!pointer) return NULL;
    for (unsigned i=0;i<B2WM_BINDINGS;i++) {
        b2wm_binding_t *b=b2wm_bindings+i;
        if (b->model!=pointer) continue;
        const model_t *m=b->model;
        if (m->type!=MOD_ALIAS || m->meshes!=b->meshes || m->numframes!=(int)b->spec->frames ||
            m->nummeshes!=(int)b->spec->meshes || m->frames!=b->frames ||
            m->iqmData || m->num_light_polys!=0 ||
            strncmp(m->name,b->spec->name,sizeof(m->name))) return NULL;
        // Loader-owned arrays remain immutable until Forget. Refuse replaced storage/layout
        // before using a saved persistent vertex index or frame stride.
        for (unsigned j=0;j<b->spec->meshes;j++) {
            const maliasmesh_t *mesh=m->meshes+j;
            const b2wm_mesh_binding_t *saved=b->layout+j;
            if (mesh->numverts!=saved->vertices || mesh->numtris!=saved->triangles ||
                mesh->numindices!=saved->indices || mesh->positions!=saved->positions ||
                mesh->indices!=saved->elements) return NULL;
        }
        if (key && (key->slot!=b->info.key.slot || key->calibration!=B2WM_CALIBRATION ||
            key->generation!=b->info.key.generation || memcmp(key->sha256,b->info.key.sha256,32))) return NULL;
        return b;
    }
    return NULL;
}
static double b2wm_dot(const double a[3], const double b[3])
{ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }
static void b2wm_cross(const double a[3], const double b[3], double out[3])
{
    double v[3]={a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};
    memcpy(out,v,sizeof(v));
}
static bool b2wm_unit(double v[3], double floor)
{
    double n=sqrt(b2wm_dot(v,v));
    if (!isfinite(n) || n<=floor) return false;
    for (unsigned i=0;i<3;i++) v[i]/=n;
    return true;
}
static bool b2wm_point(const b2wm_binding_t *b, unsigned ref, int frame, int old, double lerp, double out[3])
{
    const b2wm_reference_t *r=b->spec->refs+ref;
    if (!r->count) return false;
    memset(out,0,sizeof(double)*3);
    for (unsigned group=0;group<r->count;group++) {
        const b2wm_group_t *g=r->groups+group;
        if (g->mesh>=b->spec->meshes || !g->count) return false;
        const maliasmesh_t *mesh=b->meshes+g->mesh;
        if (!mesh->positions || mesh->numverts<=0) return false;
        double mean[3]={0};
        for (unsigned j=0;j<g->count;j++) {
            unsigned v=g->vertices[j];
            if (b->spec->slot==5) {
                if (v>=MD2_MAX_VERTS) return false;
                v=b->original_to_loaded[v];
            }
            if (v>=(unsigned)mesh->numverts) return false;
            const float *p=mesh->positions[(size_t)frame*mesh->numverts+v];
            const float *q=mesh->positions[(size_t)old*mesh->numverts+v];
            for (unsigned k=0;k<3;k++) {
                if (!isfinite(p[k]) || !isfinite(q[k])) return false;
                mean[k]+=(1.0-lerp)*p[k]+lerp*q[k];
            }
        }
        for (unsigned k=0;k<3;k++) out[k]+=mean[k]/g->count/r->count;
    }
    return true;
}
static bool b2wm_evaluate(const b2wm_binding_t *b, int frame, int old, float lerp, b2wm_datum_t *out)
{
    if (!out || frame<0 || old<0 || !isfinite(lerp) || lerp<0 || lerp>1) return false;
    // Exactly the effective frame rule in fill_model_instance, before all reads.
    if (frame>=(int)b->spec->frames) frame=0;
    if (old>=(int)b->spec->frames) old=0;
    b2wm_datum_t d={0};d.frame=frame;d.oldframe=old;
    if (!b2wm_point(b,0,frame,old,lerp,d.muzzle) || !b2wm_point(b,1,frame,old,lerp,d.grip)) return false;
    if (b->spec->mode==3) {
        for (unsigned k=0;k<3;k++) d.basis[k][k]=1;
    } else {
        double front[3],back[3],up[3],base[3],resolution=1;
        if (!b2wm_point(b,2,frame,old,lerp,front) || !b2wm_point(b,3,frame,old,lerp,back)) return false;
        for (unsigned k=0;k<3;k++) {
            d.basis[0][k]=front[k]-back[k];
            resolution=fmax(resolution,fmax(fabs(front[k]),fabs(back[k])));
        }
        // A relative float-coordinate cancellation bound, not a physical-fit tolerance.
        double floor=32.0*FLT_EPSILON*resolution;
        if (!b2wm_unit(d.basis[0],floor)) return false;
        if (b->spec->mode==2) { up[0]=up[1]=0;up[2]=1; }
        else if (b->spec->mode==1) {
            if (!b2wm_point(b,6,frame,old,lerp,up) || !b2wm_point(b,7,frame,old,lerp,base)) return false;
            for (unsigned k=0;k<3;k++) up[k]-=base[k];
            b2wm_cross(d.basis[0],up,up);
        } else {
            if (!b2wm_point(b,4,frame,old,lerp,up) || !b2wm_point(b,5,frame,old,lerp,base)) return false;
            for (unsigned k=0;k<3;k++) {
                resolution=fmax(resolution,fmax(fabs(up[k]),fabs(base[k])));
                up[k]-=base[k];
            }
        }
        double along=b2wm_dot(up,d.basis[0]);
        for (unsigned k=0;k<3;k++) up[k]-=along*d.basis[0][k];
        if (!b2wm_unit(up,32.0*FLT_EPSILON*resolution)) return false;
        b2wm_cross(up,d.basis[0],d.basis[1]);
        if (!b2wm_unit(d.basis[1],32.0*FLT_EPSILON)) return false;
        b2wm_cross(d.basis[0],d.basis[1],d.basis[2]);
    }
    *out=d;return true;
}
// Raw spans are checked before any index/position access. Full SHA256 admission
// still precedes this helper; no arbitrary model or external topology is accepted.
typedef struct { const byte *indices; size_t stride; } b2wm_raw_mesh_t;
static bool b2wm_span535(size_t bytes, size_t offset, size_t count, size_t stride)
{
    return stride && offset <= bytes && count <= (bytes-offset)/stride;
}
static bool b2wm_raw_topology535(const b2wm_spec_t *s, const void *raw,
    size_t bytes, b2wm_raw_mesh_t out[MD3_MAX_MESHES])
{
    if (!raw || !s || !s->frames || s->frames > MD3_MAX_FRAMES ||
        !s->meshes || s->meshes > MD3_MAX_MESHES) return false;
    if (s->slot == 5) {
        if (bytes < sizeof(dmd2header_t) || s->meshes != 1) return false;
        const dmd2header_t *h = raw;
        if (h->ident != MD2_IDENT || h->version != MD2_VERSION || h->ofs_end != bytes ||
            h->num_frames != s->frames || h->num_xyz != s->topology[0].vertices ||
            h->num_tris != s->topology[0].triangles || !h->num_xyz || h->num_xyz > MD2_MAX_VERTS ||
            !h->num_tris || h->num_tris > MD2_MAX_TRIANGLES ||
            h->framesize < offsetof(dmd2frame_t,verts) + h->num_xyz*sizeof(dmd2trivertx_t) ||
            !b2wm_span535(bytes,h->ofs_frames,h->num_frames,h->framesize) ||
            !b2wm_span535(bytes,h->ofs_tris,h->num_tris,sizeof(dmd2triangle_t))) return false;
        out[0] = (b2wm_raw_mesh_t){(const byte *)raw+h->ofs_tris,sizeof(dmd2triangle_t)};
        return true;
    }
    if (bytes < sizeof(dmd3header_t)) return false;
    const dmd3header_t *h = raw;
    if (h->ident != MD3_IDENT || h->version != MD3_VERSION || h->ofs_end != bytes ||
        h->num_frames != s->frames || h->num_meshes != s->meshes ||
        !b2wm_span535(bytes,h->ofs_frames,s->frames,sizeof(dmd3frame_t))) return false;
    size_t offset = h->ofs_meshes;
    for (unsigned mi=0;mi<s->meshes;mi++) {
        if (!b2wm_span535(bytes,offset,1,sizeof(dmd3mesh_t))) return false;
        const dmd3mesh_t *m = (const void *)((const byte *)raw+offset);
        if (m->ident != MD3_IDENT || m->num_frames != s->frames ||
            m->num_verts != s->topology[mi].vertices || m->num_tris != s->topology[mi].triangles ||
            !m->num_verts || m->num_verts > MD3_MAX_VERTS || !m->num_tris || m->num_tris > MD3_MAX_TRIANGLES ||
            m->meshsize < sizeof(*m) || m->meshsize > bytes-offset ||
            !b2wm_span535(m->meshsize,m->ofs_indexes,m->num_tris,3*sizeof(uint32_t)) ||
            !b2wm_span535(m->meshsize,m->ofs_verts,(size_t)m->num_verts*s->frames,sizeof(dmd3vertex_t))) return false;
        out[mi] = (b2wm_raw_mesh_t){(const byte *)m+m->ofs_indexes,3*sizeof(uint32_t)};
        offset += m->meshsize;
    }
    return offset <= bytes;
}
static unsigned b2wm_raw_index535(const b2wm_raw_mesh_t *mesh, unsigned index, bool md2)
{
    const byte *p=mesh->indices+(size_t)(index/3)*mesh->stride;
    if (md2) { uint16_t v; memcpy(&v,p+(index%3)*sizeof(v),sizeof(v)); return v; }
    uint32_t v; memcpy(&v,p+(index%3)*sizeof(v),sizeof(v)); return v;
}

// Called only after exact accepted raw-byte and topology verification. These are
// the same float conversions as the real loaders; every loaded position must agree.
static bool b2wm_loaded_vertices(const b2wm_binding_t *b, const void *raw,
    const uint16_t *original, const uint16_t *loaded, unsigned corners)
{
    if (b->spec->slot==5) {
        const dmd2header_t *h=raw;
        for (unsigned f=0;f<b->spec->frames;f++) {
            const dmd2frame_t *frame=(const void *)((const byte *)raw+h->ofs_frames+(size_t)f*h->framesize);
            for (unsigned j=0;j<corners;j++) for (unsigned k=0;k<3;k++) {
                float expected=frame->verts[original[j]].v[k]*frame->scale[k]+frame->translate[k];
                float actual=b->meshes[0].positions[(size_t)f*b->meshes[0].numverts+loaded[j]][k];
                if (!isfinite(actual) || actual!=expected) return false;
            }
        }
    } else {
        const dmd3header_t *h=raw;const byte *cursor=(const byte *)raw+h->ofs_meshes;
        for (unsigned mi=0;mi<b->spec->meshes;mi++) {
            const dmd3mesh_t *mesh=(const void *)cursor;
            const dmd3vertex_t *vertices=(const void *)(cursor+mesh->ofs_verts);
            for (size_t j=0;j<(size_t)mesh->num_verts*b->spec->frames;j++) for (unsigned k=0;k<3;k++) {
                float actual=b->meshes[mi].positions[j][k];
                if (!isfinite(actual) || actual!=(float)vertices[j].point[k]/64.f) return false;
            }
            cursor+=mesh->meshsize;
        }
    }
    return true;
}
static void b2wm_register(model_t *model, const void *raw, size_t bytes,
    const uint16_t *original, const uint16_t *loaded, unsigned corners)
{
    if (!model) return;
    B2WM_Forget(model);
    const b2wm_spec_t *s=NULL;
    for (unsigned i=0;i<12;i++) if (!strcmp(model->name,b2wm_specs[i].name) && bytes==b2wm_specs[i].bytes) s=b2wm_specs+i;
    if (!s || b2wm_generation==UINT64_MAX || model->type!=MOD_ALIAS ||
        model->numframes!=(int)s->frames || model->nummeshes!=(int)s->meshes || model->num_light_polys!=0 ||
        !model->meshes || !model->frames || model->iqmData || s->meshes>MD3_MAX_MESHES) return;
    unsigned char digest[32];
    if (!b2wm_digest(raw,s->bytes,digest) || memcmp(digest,s->digest,32)) return;
    b2wm_raw_mesh_t raw_meshes[MD3_MAX_MESHES];
    if (!b2wm_raw_topology535(s,raw,bytes,raw_meshes)) return;
    b2wm_binding_t candidate={0};candidate.model=model;candidate.meshes=model->meshes;candidate.frames=model->frames;candidate.spec=s;
    memset(candidate.original_to_loaded,0xff,sizeof(candidate.original_to_loaded));
    for (unsigned mi=0;mi<s->meshes;mi++) {
        const maliasmesh_t *mesh=model->meshes+mi;const b2wm_mesh_spec_t *expected=s->topology+mi;
        if (!mesh->positions || !mesh->indices || mesh->numverts<=0 || mesh->numtris!=(int)expected->triangles ||
            mesh->numindices!=(int)(expected->triangles*3)) return;
        if (s->slot!=5 && mesh->numverts!=(int)expected->vertices) return;
        candidate.layout[mi]=(b2wm_mesh_binding_t){mesh->numverts,mesh->numtris,
            mesh->numindices,mesh->positions,mesh->indices};
        for (unsigned i=0;i<expected->triangles*3;i++) {
            unsigned triangle=i/3,corner=i%3;
            unsigned raw_index=b2wm_raw_index535(raw_meshes+mi,i,s->slot==5);
            if (raw_index >= expected->vertices) return;
            if (s->slot==5) {
                if (!original || !loaded || corners!=expected->triangles*3 || original[i]!=raw_index ||
                    original[i]>=MD2_MAX_VERTS || loaded[i]>=(unsigned)mesh->numverts ||
                    mesh->indices[triangle*3+(corner==0?0:3-corner)]!=loaded[i]) return;
                uint16_t *mapped=candidate.original_to_loaded+original[i];
                if (*mapped==UINT16_MAX) *mapped=loaded[i];
                for (unsigned f=0;f<s->frames;f++)
                    if (memcmp(mesh->positions[f*mesh->numverts+*mapped],mesh->positions[f*mesh->numverts+loaded[i]],sizeof(vec3_t))) return;
            } else if (mesh->indices[triangle*3+(2-corner)]!=raw_index) return;
        }
    }
    if (!b2wm_loaded_vertices(&candidate,raw,original,loaded,corners)) return;
    b2wm_datum_t ready;
    if (!b2wm_evaluate(&candidate,s->ready,s->ready,0,&ready)) return;
    candidate.info.size=sizeof(candidate.info);candidate.info.version=B2WM_VERSION;
    candidate.info.key.slot=s->slot;candidate.info.key.calibration=B2WM_CALIBRATION;
    memcpy(candidate.info.key.sha256,digest,32);candidate.info.frames=s->frames;candidate.info.ready=s->ready;
    candidate.info.scale=s->scale;candidate.info.requires_body_partition=s->slot==5;
    double delta[3];for (unsigned k=0;k<3;k++) delta[k]=ready.grip[k]-ready.muzzle[k];
    for (unsigned k=0;k<3;k++) candidate.info.grip_from_muzzle[k]=b2wm_dot(ready.basis[k],delta);
    for (unsigned i=0;i<B2WM_BINDINGS;i++) if (!b2wm_bindings[i].model) {
        candidate.info.key.generation=++b2wm_generation;b2wm_bindings[i]=candidate;return;
    }
}
bool B2WM_Query(const void *model, unsigned version, unsigned size, b2wm_info_t *out)
{
    if (!out || version!=B2WM_VERSION || size!=sizeof(*out)) return false;
    memset(out,0,sizeof(*out));b2wm_binding_t *b=b2wm_find(model,NULL);
    if (!b) return false;*out=b->info;return true;
}
bool B2WM_Datum(const void *model, const b2wm_key_t *key, int frame, int old, float lerp, b2wm_datum_t *out)
{
    if (out) memset(out,0,sizeof(*out));
    b2wm_binding_t *b=key?b2wm_find(model,key):NULL;
    return b && b2wm_evaluate(b,frame,old,lerp,out);
}
static bool b2wm_aim(const b2w_pose_t *aim, double world[3][3])
{
    if (!aim || aim->hand>1) return false;
    for (unsigned k=0;k<3;k++) {
        if (!isfinite(aim->origin[k]) || !isfinite(aim->forward[k]) || !isfinite(aim->right[k]) || !isfinite(aim->up[k])) return false;
        world[0][k]=aim->forward[k];world[1][k]=-aim->right[k];world[2][k]=aim->up[k];
    }
    // Same geometric admission as B2W_PoseCurrent; freshness remains its owner's job.
    for (unsigned a=0;a<3;a++) for (unsigned b=0;b<3;b++)
        if (fabs(b2wm_dot(world[a],world[b])-(a==b?1.0:0.0))>.001) return false;
    double cross[3];b2wm_cross(world[0],world[1],cross);
    return b2wm_dot(cross,world[2])>.999;
}
bool B2WM_Desired(const void *model, const b2wm_key_t *key, const b2w_pose_t *aim, float desired[3])
{
    if (desired) memset(desired,0,sizeof(float)*3);
    b2wm_binding_t *b=key?b2wm_find(model,key):NULL;double w[3][3];
    if (!b || !desired || !b2wm_aim(aim,w)) return false;
    float result[3];double hand=aim->hand==0?-1:1;
    for (unsigned i=0;i<3;i++) {
        double x=aim->origin[i];
        for (unsigned k=0;k<3;k++) x-=b->spec->scale*w[k][i]*(k==1?hand:1)*b->info.grip_from_muzzle[k];
        result[i]=(float)x;if (!isfinite(result[i])) return false;
    }
    memcpy(desired,result,sizeof(result));return true;
}
// Hyperblaster's authored ready and firing poses have different sideways
// aim and muzzle positions. Remove only that model-local yaw/lateral motion;
// longitudinal/vertical recoil, pitch, roll and relative barrel spin remain.
// The same rigid correction precedes both drawing and nominal muzzle sampling.
bool B2WM_HyperStabilise(const void *model, const b2wm_key_t *key,
    int frame, int old, float lerp, bool tracked, float matrix[16])
{
    b2wm_binding_t *b=key?b2wm_find(model,key):NULL;b2wm_datum_t d,cal;
    if (!b || !matrix || b->spec->slot!=8 ||
        !b2wm_evaluate(b,frame,old,lerp,&d) ||
        !b2wm_evaluate(b,b->spec->ready,b->spec->ready,0,&cal)) return false;
    // Raise/lower animation is unchanged, with a continuous render transition
    // into and out of the ready/firing range (stock frames 6 through 50).
    double weight=(d.frame>=6 && d.frame<=50?1.0:0.0)*(1-lerp)+
        (d.oldframe>=6 && d.oldframe<=50?1.0:0.0)*lerp;
    if (!weight) return true;
    // Yaw is measured around the input owner's up axis: ready-model up for
    // the calibrated tracked transform, ordinary model Z for the KBM entity.
    double reference[3][3];memcpy(reference,cal.basis,sizeof(reference));
    if (!tracked) {
        reference[0][2]=0;
        if (!b2wm_unit(reference[0],32.0*FLT_EPSILON)) return false;
        reference[2][0]=reference[2][1]=0;reference[2][2]=1;
        b2wm_cross(reference[2],reference[0],reference[1]);
    }
    double yaw=atan2(b2wm_dot(d.basis[0],reference[1]),
        b2wm_dot(d.basis[0],reference[0]));
    double c=cos(-yaw*weight),s=sin(-yaw*weight),r[3][3],t[3],delta[3];
    const double *up=reference[2];
    for (unsigned i=0;i<3;i++) for (unsigned j=0;j<3;j++)
        r[i][j]=(i==j?c:0)+(1-c)*up[i]*up[j];
    r[0][1]-=s*up[2];r[0][2]+=s*up[1];
    r[1][0]+=s*up[2];r[1][2]-=s*up[0];
    r[2][0]-=s*up[1];r[2][1]+=s*up[0];
    for (unsigned i=0;i<3;i++) {
        double muzzle=d.grip[i];
        for (unsigned j=0;j<3;j++) muzzle+=r[i][j]*(d.muzzle[j]-d.grip[j]);
        delta[i]=cal.muzzle[i]-muzzle;
    }
    double lateral=b2wm_dot(delta,reference[1])*weight;
    for (unsigned i=0;i<3;i++) {
        t[i]=d.grip[i]+lateral*reference[1][i];
        for (unsigned j=0;j<3;j++) t[i]-=r[i][j]*d.grip[j];
    }
    float result[16];memcpy(result,matrix,sizeof(result));
    for (unsigned i=0;i<3;i++) {
        double origin=matrix[12+i];
        for (unsigned j=0;j<3;j++) {
            double v=0;for (unsigned k=0;k<3;k++) v+=matrix[k*4+i]*r[k][j];
            result[j*4+i]=(float)v;origin+=matrix[j*4+i]*t[j];
        }
        result[12+i]=(float)origin;
    }
    for (unsigned i=0;i<16;i++) if (!isfinite(result[i])) return false;
    memcpy(matrix,result,sizeof(result));return true;
}

static bool b2wm_matrix(const void *model, const b2wm_key_t *key, int frame, int old, float lerp,
    const b2w_pose_t *aim, const float corrected[3], float matrix[16], bool partition)
{
    if (matrix) memset(matrix,0,sizeof(float)*16);
    b2wm_binding_t *b=key?b2wm_find(model,key):NULL;b2wm_datum_t d,cal;double w[3][3];
    // The ordinary path still refuses grenade geometry; only the exact partition may use it.
    if (!b || !matrix || !corrected || (b->info.requires_body_partition && !partition) || !b2wm_aim(aim,w) ||
        !b2wm_evaluate(b,frame,old,lerp,&d) ||
        !b2wm_evaluate(b,b->spec->ready,b->spec->ready,0,&cal)) return false;
    double hand=aim->hand==0?-1:1;float result[16]={0};result[15]=1;
    for (unsigned i=0;i<3;i++) {
        if (!isfinite(corrected[i])) return false;
        double origin=corrected[i];
        for (unsigned j=0;j<3;j++) {
            double r=0;
            for (unsigned k=0;k<3;k++) r+=b->spec->scale*w[k][i]*(k==1?hand:1)*cal.basis[k][j];
            result[j*4+i]=(float)r;origin-=r*cal.muzzle[j];
        }
        result[12+i]=(float)origin;if (!isfinite(result[12+i])) return false;
    }
    memcpy(matrix,result,sizeof(result));
    return b->spec->slot!=8 || B2WM_HyperStabilise(model,key,frame,old,lerp,true,matrix);
}

bool B2WM_Matrix(const void *model, const b2wm_key_t *key, int frame, int old, float lerp,
    const b2w_pose_t *aim, const float corrected[3], float matrix[16])
{ return b2wm_matrix(model,key,frame,old,lerp,aim,corrected,matrix,false); }
// Exact v_handgr MD2 hand geometry. The 73 hand vertices exclude the four
// far sleeve-end vertices (208..211) and wrist extension (212). This anchors
// the preferred hand rather than the grenade held in the other hand.
static bool b2wm_grenade_hand(const b2wm_binding_t *b, int frame, int old,
    float lerp, double centre[3], double basis[3][3])
{
    double p[3][3];memset(centre,0,3*sizeof(double));
    const maliasmesh_t *mesh=b->meshes;
    for (unsigned v=135;v<=207;v++) {
        unsigned loaded=b->original_to_loaded[v];
        if (loaded>=(unsigned)mesh->numverts) return false;
        const float *a=mesh->positions[(size_t)frame*mesh->numverts+loaded];
        const float *q=mesh->positions[(size_t)old*mesh->numverts+loaded];
        for (unsigned k=0;k<3;k++) {
            double x=(1-lerp)*a[k]+lerp*q[k];if (!isfinite(x)) return false;
            centre[k]+=x/73;
            if (v==135) p[0][k]=x;
            if (v==183) p[1][k]=x;
            if (v==185) p[2][k]=x;
        }
    }
    for (unsigned k=0;k<3;k++) {
        basis[0][k]=p[1][k]-p[0][k];basis[1][k]=p[2][k]-p[0][k];
    }
    if (!b2wm_unit(basis[0],32*FLT_EPSILON)) return false;
    b2wm_cross(basis[0],basis[1],basis[2]);
    if (!b2wm_unit(basis[2],32*FLT_EPSILON)) return false;
    b2wm_cross(basis[2],basis[0],basis[1]);return true;
}
static int b2wm_grenade_body_frame(int frame)
{
    // Stock hides the grenade by collapsing it during the cocked hold and
    // recovery. Retain the last intact throwing-hand pose until emission.
    return (frame>=10 && frame<=12) || frame==14 || frame==15 ? 9 : frame;
}
bool B2WM_GrenadeParts(const void *model, const b2wm_key_t *key,
    int frame, int old, float lerp, const b2w_pose_t *aim,
    const float corrected[3], bool held, b2wm_part_t parts[3])
{
    if (!parts) return false;
    memset(parts,0,sizeof(*parts)*3);
    b2wm_binding_t *b=key?b2wm_find(model,key):NULL;
    b2wm_datum_t d,cal,body;double palm[3],basis[3][3],source_palm[3],source_basis[3][3];
    float matrix[16];
    if (!b || b->spec->slot!=5 || b->spec->ready!=16 ||
        b->spec->meshes!=1 || b->meshes[0].numtris!=406 ||
        !b2wm_evaluate(b,frame,old,lerp,&d) ||
        !b2wm_evaluate(b,16,16,0,&cal) ||
        !b2wm_matrix(model,key,frame,old,lerp,aim,corrected,matrix,true) ||
        !b2wm_grenade_hand(b,d.frame,d.oldframe,lerp,palm,basis)) return false;
    // Keep the stock hand animation and its relative two-hand spacing, but
    // place the preferred hand at the sole simulation-owned corrected anchor.
    for (unsigned i=0;i<3;i++)
        for (unsigned j=0;j<3;j++) matrix[12+i]+=(float)(matrix[j*4+i]*(cal.muzzle[j]-palm[j]));
    // Exact original MD2 primitive order: other hand, whole grenade including
    // its attached detail, preferred hand. Every triangle is submitted once.
    const unsigned first[3]={168,0,254},count[3]={86,168,152};
    b2wm_part_t result[3]={0};
    for (unsigned i=0;i<3;i++) {
        b2wm_part_t *p=result+i;p->id=i;p->first=first[i];p->count=count[i];
        p->frame=d.frame;p->oldframe=d.oldframe;p->backlerp=lerp;p->visible=i!=1||held;
        memcpy(p->matrix,matrix,sizeof(matrix));
    }
    b2wm_part_t *p=result+1;
    p->frame=b2wm_grenade_body_frame(d.frame);p->oldframe=b2wm_grenade_body_frame(d.oldframe);
    if (p->frame!=d.frame || p->oldframe!=d.oldframe) {
        if (!b2wm_evaluate(b,p->frame,p->oldframe,lerp,&body) ||
            !b2wm_grenade_hand(b,p->frame,p->oldframe,lerp,source_palm,source_basis)) return false;
        // Restore only collapsed grenade geometry. Its authored centre follows
        // the actual frame datum (also used by nominal emission and reticle);
        // orientation follows the palm. Hands remain on their actual frames.
        double rotation[3][3],translation[3];
        for (unsigned i=0;i<3;i++) for (unsigned j=0;j<3;j++) {
            rotation[i][j]=0;
            for (unsigned k=0;k<3;k++) rotation[i][j]+=basis[k][i]*source_basis[k][j];
        }
        for (unsigned i=0;i<3;i++) {
            translation[i]=d.muzzle[i];
            for (unsigned j=0;j<3;j++) translation[i]-=rotation[i][j]*body.muzzle[j];
        }
        for (unsigned i=0;i<3;i++) {
            double origin=matrix[12+i];
            for (unsigned j=0;j<3;j++) {
                double x=0;for (unsigned k=0;k<3;k++) x+=matrix[k*4+i]*rotation[k][j];
                p->matrix[j*4+i]=(float)x;origin+=matrix[j*4+i]*translation[j];
            }
            p->matrix[12+i]=(float)origin;
        }
    }
    for (unsigned i=0;i<3;i++) for (unsigned j=0;j<16;j++)
        if (!isfinite(result[i].matrix[j])) return false;
    memcpy(parts,result,sizeof(result));return true;
}
// Fixed ready calibration preserves actual model animation. Recoil is supplied
// by the simulation; this function neither samples input nor authorises a shot.
bool B2WM_Recoil(const b2w_pose_t *aim, const float angles[3], b2w_pose_t *out)
{
    double world[3][3];if (!out || !angles || !b2wm_aim(aim,world)) return false;
    for (unsigned k=0;k<3;k++) if (!isfinite(angles[k]) || fabsf(angles[k])>720) return false;
    vec3_t f,r,u;AngleVectors(angles,f,r,u);b2w_pose_t result=*aim;
    for (unsigned k=0;k<3;k++) {
        result.forward[k]=f[0]*aim->forward[k]-f[1]*aim->right[k]+f[2]*aim->up[k];
        result.right[k]=r[0]*aim->forward[k]-r[1]*aim->right[k]+r[2]*aim->up[k];
        result.up[k]=u[0]*aim->forward[k]-u[1]*aim->right[k]+u[2]*aim->up[k];
    }
    if (!b2wm_aim(&result,world)) return false;*out=result;return true;
}
bool B2WM_Muzzle(const b2wm_datum_t *d, const float matrix[16], int hand, b2w_pose_t *out)
{
    if (!d || !matrix || !out || (hand!=1 && hand!=-1)) return false;
    b2w_pose_t p={0};
    for (unsigned row=0;row<3;row++) {
        p.origin[row]=matrix[12+row];
        for (unsigned col=0;col<3;col++) {
            p.origin[row]+=(float)d->muzzle[col]*matrix[col*4+row];
            p.forward[row]+=(float)d->basis[0][col]*matrix[col*4+row];
            p.right[row]-=(float)d->basis[1][col]*matrix[col*4+row]*hand;
            p.up[row]+=(float)d->basis[2][col]*matrix[col*4+row];
        }
    }
    if (VectorNormalize(p.forward)<=0 || VectorNormalize(p.right)<=0 || VectorNormalize(p.up)<=0) return false;
    double w[3][3];if (!b2wm_aim(&p,w)) return false;*out=p;return true;
}
