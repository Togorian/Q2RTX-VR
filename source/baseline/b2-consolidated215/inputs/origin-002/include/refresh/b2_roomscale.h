// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 1997-2001 Id Software, Inc.
// Copyright (C) 2026 Togorian
//
// Room-scale movement and physical crouch for Q2RTX VR.
// Collision stepping follows id Software's Quake II player movement (pmove.c).
// Physical crouch originated separately. These VR functions are original
// project work designed by Togorian and implemented with Codex; see PROVENANCE.md.
// Modified 14 November 2026: authorship and separate crouch origin clarified.
#ifndef B2_ROOMSCALE_H
#define B2_ROOMSCALE_H
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

typedef struct {
    bool calibrated, candidate_valid, clock_valid, crouched;
    float baseline, candidate_min, candidate_max, last_height;
    uint32_t last_ms, stable_ms, transition_ms;
} b2rs_stance_t;

// LOCAL height is origin-relative metres, not a measured floor distance.
// Admit by representable resolution, not distance from the runtime origin.
// Each adjacent float gap must be <= one eighth of the existing 0.1 mm
// horizontal deadband. This also leaves ample margin below the 30 mm stance
// stability window. Test both neighbours at exponent boundaries.
static inline bool B2RS_CoordinateValid(float value)
{
    if (!isfinite(value)) return false;
    const double gap_limit=0.0001/8.0;
    double above=(double)nextafterf(value, INFINITY)-(double)value;
    double below=(double)value-(double)nextafterf(value, -INFINITY);
    return above<=gap_limit && below<=gap_limit;
}
// Widen before addition; finite float endpoints cannot overflow this sum.
static inline float B2RS_Midpoint(float a, float b)
{
    return (float)(((double)a+(double)b)*0.5);
}
static inline bool B2RS_HeightValid(float h) { return B2RS_CoordinateValid(h); }
static inline void B2RS_Suspend(b2rs_stance_t *s)
{
    s->clock_valid=false;s->candidate_valid=false;s->stable_ms=0;
    s->transition_ms=0;s->crouched=false; // Baseline survives suspension.
}
// No inferred standing baseline is established while the stock hull is ducked.
static inline bool B2RS_Stance(b2rs_stance_t *s, bool eligible, bool tracked,
    float h, uint32_t sample_ms, bool stock_ducked, bool manual)
{
    if (!eligible || !tracked || !B2RS_HeightValid(h)) { B2RS_Suspend(s);return false; }
    if (s->clock_valid && sample_ms==s->last_ms) return s->crouched;
    uint32_t dt=s->clock_valid ? sample_ms-s->last_ms : 0;
    s->last_ms=sample_ms;s->last_height=h;s->clock_valid=true;
    if (!dt || dt>100) {
        s->transition_ms=0;s->candidate_valid=false;s->stable_ms=0;
        if (dt>100) s->crouched=false;
        return s->crouched;
    }
    if (!s->calibrated) {
        if (stock_ducked || manual) {s->candidate_valid=false;s->stable_ms=0;return false;}
        if (!s->candidate_valid || h<s->candidate_min-.03f || h>s->candidate_max+.03f ||
            fmaxf(s->candidate_max,h)-fminf(s->candidate_min,h)>.03f) {
            s->candidate_min=s->candidate_max=h;s->candidate_valid=true;s->stable_ms=0;
            return false;
        }
        s->candidate_min=fminf(s->candidate_min,h);s->candidate_max=fmaxf(s->candidate_max,h);
        s->stable_ms+=dt;
        if (s->stable_ms>=500) {
            s->baseline=B2RS_Midpoint(s->candidate_min,s->candidate_max);s->calibrated=true;
        }
        return false;
    }
    // Physical crouch threshold and debounce with fixed settings.
    // Compare heights (rather than subtracting then comparing decimal drops).
    bool target=s->crouched;
    if (!s->crouched && h<=s->baseline-.35f) target=true;
    else if (s->crouched && h>=s->baseline-.25f) target=false;
    if (target==s->crouched) s->transition_ms=0;
    else if ((s->transition_ms+=dt)>=100) {s->crouched=target;s->transition_ms=0;}
    return s->crouched;
}
// Physical duck-view offset follows id Software's 24-unit duck drop. Compensation contains
// only the stock stance component, not bob, damage kick or tracked HMD height.
static inline float B2RS_DuckDrop(bool owned, bool old_duck, bool duck, float lerp)
{
    if (!owned) return 0;
    if (!isfinite(lerp) || lerp<0) lerp=0;
    if (lerp>1) lerp=1;
    return 24.f*((old_duck?1.f:0.f)+lerp*((duck?1.f:0.f)-(old_duck?1.f:0.f)));
}
typedef struct { bool owned,manual;float weight;uint32_t last_ms; } b2rs_eye_t;
static inline float B2RS_EyeStep(b2rs_eye_t *e,bool physical,bool manual,
    bool old_duck,bool duck,uint32_t now)
{
    uint32_t dt=e->last_ms?now-e->last_ms:0;e->last_ms=now;
    if(physical){
        // From standing the stock drop starts at zero. A pre-existing manual
        // drop hands over gradually instead of removing 24QU in one frame.
        if(!e->owned && !e->manual && !old_duck && !duck)e->weight=1;
        e->owned=true;
    }
    if(!physical && !old_duck && !duck)e->owned=false;
    float target=e->owned && (physical || !manual) ? 1.f:0.f;
    if(dt<=100){
        float step=dt*.01f;
        e->weight=target>e->weight?fminf(target,e->weight+step):fmaxf(target,e->weight-step);
    }
    e->manual=manual;return e->weight;
}

typedef struct {
    float fraction, end[3], normal[3];
    bool startsolid, allsolid;
    uintptr_t ground;
} b2rs_trace_t;
typedef b2rs_trace_t (*b2rs_trace_fn)(void *, const float[3], const float[3],
    const float[3], const float[3]);
typedef struct {
    float origin[3], mins[3], maxs[3];
    uintptr_t ground;
} b2rs_hull_t;
static inline bool b2rs_finite3(const float *v)
{ return isfinite(v[0]) && isfinite(v[1]) && isfinite(v[2]); }
static inline bool b2rs_trace_valid(const b2rs_trace_t *t)
{
    return isfinite(t->fraction) && t->fraction>=0 && t->fraction<=1 &&
        b2rs_finite3(t->end) && b2rs_finite3(t->normal);
}
// Room-scale slide: callback carries the existing player hull and mask.
// Reject startsolid and non-finite traces; retain a bounded multi-plane clip.
static inline bool B2RS_Slide(const b2rs_hull_t *h, const float start[3],
    const float movement[3], b2rs_trace_fn trace, void *opaque, float result[3])
{
    float current[3],remaining[3],planes[4][3];unsigned count=0;
    memcpy(current,start,sizeof(current));memcpy(remaining,movement,sizeof(remaining));
    remaining[2]=0;
    for(unsigned bump=0;bump<4;bump++) {
        float end[3];for(unsigned i=0;i<3;i++)end[i]=current[i]+remaining[i];
        b2rs_trace_t t=trace(opaque,current,h->mins,h->maxs,end);
        if(!b2rs_trace_valid(&t) || t.startsolid || t.allsolid) return false;
        if(t.fraction>0)memcpy(current,t.end,sizeof(current));
        if(t.fraction==1)break;
        float n=t.normal[0]*t.normal[0]+t.normal[1]*t.normal[1]+t.normal[2]*t.normal[2];
        if(fabsf(n-1.f)>.02f)return false;
        memcpy(planes[count++],t.normal,sizeof(t.normal));
        for(unsigned i=0;i<3;i++)remaining[i]*=1-t.fraction;
        // Horizontal physical motion may slide along walls; stairs/slopes are
        // resolved by the separate supported step path, not a vertical impulse.
        for(unsigned p=0;p<count;p++) {
            float d=remaining[0]*planes[p][0]+remaining[1]*planes[p][1];
            float nxy=planes[p][0]*planes[p][0]+planes[p][1]*planes[p][1];
            if(d<0 && nxy>1e-6f) {
                remaining[0]-=d*planes[p][0]/nxy;remaining[1]-=d*planes[p][1]/nxy;
            }
        }
        for(unsigned p=0;p<count;p++)
            if(remaining[0]*planes[p][0]+remaining[1]*planes[p][1]<-.0001f)
                remaining[0]=remaining[1]=0;
        if(remaining[0]*remaining[0]+remaining[1]*remaining[1]<.0001f)break;
    }
    memcpy(result,current,sizeof(current));return true;
}
static inline float b2rs_dist2(const float a[3],const float b[3])
{return (a[0]-b[0])*(a[0]-b[0])+(a[1]-b[1])*(a[1]-b[1]);}
// Room-scale movement follows id Software's step/slide limits; no cvar transport.
// Returns exact eighth-unit accepted travel; validates snapped hull before commit.
static inline bool B2RS_Move(b2rs_hull_t *h,const float move[3],
    b2rs_trace_fn trace,void *opaque,float accepted[3])
{
    memset(accepted,0,3*sizeof(float));
    if(!h || !trace || !b2rs_finite3(h->origin) || !b2rs_finite3(h->mins) ||
       !b2rs_finite3(h->maxs) || !b2rs_finite3(move) || move[2]!=0 ||
       move[0]*move[0]+move[1]*move[1]>64.f*64.f)return false;
    float start[3],flat[3];memcpy(start,h->origin,sizeof(start));
    if(!B2RS_Slide(h,start,move,trace,opaque,flat))return false;
    uintptr_t support=h->ground;
    if(h->ground && b2rs_dist2(flat,start)+.01f<move[0]*move[0]+move[1]*move[1]) {
        float up[3]={start[0],start[1],start[2]+18.f};
        b2rs_trace_t t=trace(opaque,start,h->mins,h->maxs,up);
        if(b2rs_trace_valid(&t) && !t.startsolid && !t.allsolid && t.fraction==1) {
            float stepped[3];
            if(B2RS_Slide(h,up,move,trace,opaque,stepped)) {
                float down[3]={stepped[0],stepped[1],stepped[2]-18.f};
                t=trace(opaque,stepped,h->mins,h->maxs,down);
                if(b2rs_trace_valid(&t) && !t.startsolid && !t.allsolid &&
                   t.fraction<1 && t.normal[2]>=.7f && b2rs_dist2(t.end,start)>b2rs_dist2(flat,start)+.01f) {
                    memcpy(flat,t.end,sizeof(flat));support=t.ground;
                }
            }
        }
    }
    float snap[3];
    for(unsigned i=0;i<3;i++) {
        if(fabsf(flat[i])>4095.f)return false;
        snap[i]=roundf(flat[i]*8.f)*.125f;
    }
    b2rs_trace_t valid=trace(opaque,snap,h->mins,h->maxs,snap);
    if(!b2rs_trace_valid(&valid))return false;
    if(valid.startsolid || valid.allsolid) {
        // A downward collision trace stops DIST_EPSILON above the step. Nearest
        // eighth rounding can put that hull exactly back inside the brush.
        // Keep every already-clear original snap; otherwise use the stock
        // PM_SnapPosition checked jitter order (Id Software, GPL-2.0-or-later).
        static const unsigned jitter[8]={0,4,1,2,3,5,6,7};
        float base[3],sign[3];bool clear=false;
        for(unsigned i=0;i<3;i++) {
            base[i]=truncf(flat[i]*8.f)*.125f;
            sign[i]=base[i]==flat[i]?0.f:(flat[i]>=0?.125f:-.125f);
        }
        for(unsigned j=0;j<8;j++) {
            for(unsigned i=0;i<3;i++)snap[i]=base[i]+((jitter[j]&(1u<<i))?sign[i]:0.f);
            valid=trace(opaque,snap,h->mins,h->maxs,snap);
            if(!b2rs_trace_valid(&valid))return false;
            if(!valid.startsolid && !valid.allsolid){clear=true;break;}
        }
        if(!clear)return false;
    }
    for(unsigned i=0;i<3;i++){accepted[i]=snap[i]-start[i];h->origin[i]=snap[i];}
    h->ground=support;return true;
}
#endif
