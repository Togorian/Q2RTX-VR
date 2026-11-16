// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 1997-2001 Id Software, Inc.
// Copyright (C) 2026 Togorian
//
// VR teleport landing for Q2RTX VR; telefrag handling follows id Software's code.
// Original VR functions; see PROVENANCE.md.
// Modified 14 November 2026: authorship clarified; behaviour unchanged.
#ifndef B2_TELEPORT_H
#define B2_TELEPORT_H
#include "b2_weapon.h"
#include <math.h>
#include <string.h>
enum { B2TP_POINTS=34 };
enum { B2TP_INVALID, B2TP_EMPTY, B2TP_OCCUPIED };
typedef struct {
    bool configured,armed,aiming;
    unsigned hand;
    uint64_t serial;
    b2w_epochs_t epochs;
} b2tp_input_t;
typedef struct {
    float fraction,end[3],normal[3];
    bool startsolid,allsolid,sky,actor;
    uintptr_t ground;
} b2tp_trace_t;
typedef b2tp_trace_t (*b2tp_trace_fn)(void *,const float[3],const float[3],
    const float[3],const float[3],bool);
typedef int (*b2tp_occupants_fn)(void *,const float[3],const float[3],const float[3],bool);
typedef struct {
    float origin[3],mins[3],maxs[3];
} b2tp_hull_t;
typedef struct b2tp_preview_s {
    float points[B2TP_POINTS][3],landing[3];
    unsigned count,status;
    uintptr_t ground;
} b2tp_preview_t;
static bool b2tp_finite3(const float v[3])
{return v && isfinite(v[0]) && isfinite(v[1]) && isfinite(v[2]);}
static float B2TP_Range(float value)
{return !isfinite(value)?768.f:fminf(2048.f,fmaxf(64.f,value));}
static float b2tp_distance2(const float a[3],const float b[3])
{float x=a[0]-b[0],y=a[1]-b[1],z=a[2]-b[2];return x*x+y*y+z*z;}
static bool b2tp_trace_valid(const b2tp_trace_t *t)
{return isfinite(t->fraction) && t->fraction>=0 && t->fraction<=1 &&
    !t->startsolid && !t->allsolid && b2tp_finite3(t->end);}
static void B2TP_CancelInput(b2tp_input_t *s)
{memset(s,0,sizeof(*s));}
// Return one release edge only. The caller supplies the existing owner/pose gates.
static bool B2TP_Input(b2tp_input_t *s,const b2c_snapshot_t *p,const b2w_epochs_t *e,
    uint32_t now,unsigned weapon_hand,bool eligible,bool pose_current)
{
    if(!s || !p || !e)return false;
    if(!eligible || !pose_current || weapon_hand>1 || !p->ready || !p->serial ||
       !B2W_EpochsEqual(e,e) || p->generation!=e->action || p->xr_generation!=e->xr ||
       !p->generation || !p->xr_generation || (uint32_t)(now-p->capture_ms)>100 ||
       (uint32_t)(now-p->head_ms)>100 || !b2tp_finite3(p->head_position)){
        B2TP_CancelInput(s);return false;
    }
    unsigned hand=weapon_hand^1u;
    const b2c_hand_t *h=&p->hands[hand];
    if(!h->controls_active || !h->aim_valid || !isfinite(h->trigger) ||
       h->trigger<0 || h->trigger>1){B2TP_CancelInput(s);return false;}
    if(!s->configured || s->hand!=hand || memcmp(&s->epochs,e,sizeof(*e))){
        B2TP_CancelInput(s);s->configured=true;s->hand=hand;s->epochs=*e;
    }
    if(p->serial<=s->serial)return false;
    s->serial=p->serial;
    bool held=h->trigger>.55f;
    if(!s->armed){if(!held)s->armed=true;return false;}
    if(held){s->aiming=true;return false;}
    bool release=s->aiming;s->aiming=false;return release;
}
static bool B2TP_Use(const b2c_snapshot_t *p,unsigned weapon_hand,bool enabled)
{return enabled && p && weapon_hand<=1 &&
    (weapon_hand==1?p->hands[0].primary:p->hands[1].secondary);}
static bool b2tp_hull_valid(const b2tp_hull_t *h)
{
    if(!h || !b2tp_finite3(h->origin) || !b2tp_finite3(h->mins) || !b2tp_finite3(h->maxs))return false;
    for(unsigned i=0;i<3;i++)if(h->mins[i]>=h->maxs[i] || h->mins[i]<-128 || h->maxs[i]>128)return false;
    return true;
}
// Revalidate the final quantised hull, including floor and player-clip geometry.
// Occupants are classified in a complete first pass before any damage callback.
static unsigned B2TP_Landing(const b2tp_hull_t *h,const float target[3],float range,
    b2tp_trace_fn trace,b2tp_occupants_fn occupants,void *ctx,bool commit,
    float result[3],uintptr_t *ground)
{
    if(!b2tp_hull_valid(h) || !b2tp_finite3(target) || !trace || !occupants || !result || !ground)return B2TP_INVALID;
    range=B2TP_Range(range);
    if(b2tp_distance2(h->origin,target)>range*range)return B2TP_INVALID;
    float start[3],end[3],q[3];
    memcpy(start,target,sizeof(start));memcpy(end,target,sizeof(end));
    start[2]+=2;end[2]-=10;
    b2tp_trace_t t=trace(ctx,start,h->mins,h->maxs,end,false);
    // A point on a tread can put the full hull through the next riser.
    // Retry at most one stock step higher; final full-hull checks still govern.
    if(t.startsolid || t.allsolid){
        start[2]=target[2]+18.125f;
        t=trace(ctx,start,h->mins,h->maxs,end,false);
    }
    if(!b2tp_trace_valid(&t) || t.fraction>=1 || t.sky || !t.ground ||
       !b2tp_finite3(t.normal) || t.normal[2]<.70f)return B2TP_INVALID;
    float length=t.normal[0]*t.normal[0]+t.normal[1]*t.normal[1]+t.normal[2]*t.normal[2];
    if(fabsf(length-1.f)>.02f)return B2TP_INVALID;
    for(unsigned i=0;i<3;i++){
        float v=t.end[i]+(i==2?.125f:0);
        if(!isfinite(v) || v< -4096.f || v>4095.875f)return B2TP_INVALID;
        q[i]=roundf(v*8.f)*.125f;
    }
    if(b2tp_distance2(h->origin,q)>range*range)return B2TP_INVALID;
    t=trace(ctx,q,h->mins,h->maxs,q,false);
    if(!b2tp_trace_valid(&t) || t.fraction!=1)return B2TP_INVALID;
    memcpy(end,q,sizeof(end));end[2]-=.25f;
    t=trace(ctx,q,h->mins,h->maxs,end,false);
    if(!b2tp_trace_valid(&t) || t.fraction>=1 || t.sky || !t.ground ||
       !b2tp_finite3(t.normal) || t.normal[2]<.70f)return B2TP_INVALID;
    length=t.normal[0]*t.normal[0]+t.normal[1]*t.normal[1]+t.normal[2]*t.normal[2];
    if(fabsf(length-1.f)>.02f)return B2TP_INVALID;
    uintptr_t floor=t.ground;
    int occupied=occupants(ctx,q,h->mins,h->maxs,false);
    if(occupied<0)return B2TP_INVALID;
    if(commit){
        if(occupied && occupants(ctx,q,h->mins,h->maxs,true)<0)return B2TP_INVALID;
        // Includes surviving/new actors as well as world geometry.
        t=trace(ctx,q,h->mins,h->maxs,q,true);
        if(!b2tp_trace_valid(&t) || t.fraction!=1 ||
           occupants(ctx,q,h->mins,h->maxs,false)!=0)return B2TP_INVALID;
        // A death callback can remove or change the support after classification.
        t=trace(ctx,q,h->mins,h->maxs,end,false);
        if(!b2tp_trace_valid(&t) || t.fraction>=1 || t.sky || !t.ground ||
           !b2tp_finite3(t.normal) || t.normal[2]<.70f)return B2TP_INVALID;
        length=t.normal[0]*t.normal[0]+t.normal[1]*t.normal[1]+t.normal[2]*t.normal[2];
        if(fabsf(length-1.f)>.02f)return B2TP_INVALID;
        floor=t.ground;
    }
    memcpy(result,q,sizeof(q));*ground=floor;
    return occupied?B2TP_OCCUPIED:B2TP_EMPTY;
}
static void B2TP_Arc(const b2w_pose_t *aim,const b2tp_hull_t *h,float range,
    b2tp_trace_fn trace,b2tp_occupants_fn occupants,void *ctx,b2tp_preview_t *out)
{
    memset(out,0,sizeof(*out));out->status=B2TP_INVALID;
    if(!aim || !b2tp_hull_valid(h) || !b2tp_finite3(aim->origin) ||
       !b2tp_finite3(aim->forward) || !trace || !occupants)return;
    float n=0;for(unsigned i=0;i<3;i++)n+=aim->forward[i]*aim->forward[i];
    if(fabsf(n-1.f)>.02f)return;
    range=B2TP_Range(range);memcpy(out->points[0],aim->origin,sizeof(aim->origin));out->count=1;
    const float zero[3]={0};
    for(unsigned i=1;i<B2TP_POINTS;i++){
        float time=(float)i*.05f,end[3];
        for(unsigned a=0;a<3;a++)end[a]=aim->origin[a]+650.f*aim->forward[a]*time;
        end[2]+=180.f*time-400.f*time*time;
        b2tp_trace_t t=trace(ctx,out->points[i-1],zero,zero,end,true);
        if(!b2tp_trace_valid(&t))return;
        memcpy(out->points[i],t.end,sizeof(t.end));out->count++;
        if(b2tp_distance2(h->origin,t.end)>range*range)return;
        if(t.fraction<1){
            float floor[3];memcpy(floor,t.end,sizeof(floor));
            if(t.actor){
                float above[3],below[3];memcpy(above,floor,sizeof(above));memcpy(below,floor,sizeof(below));
                above[2]+=2;below[2]-=128;
                t=trace(ctx,above,zero,zero,below,false);
                if(!b2tp_trace_valid(&t) || t.fraction>=1)return;
                memcpy(floor,t.end,sizeof(floor));
            }
            if(t.sky || !b2tp_finite3(t.normal))return;
            if(fabsf(t.normal[2])<.01f){
                // A vertical stair face may have a walkable top within 18 QU.
                // Move only the point probe just across that face, then validate
                // the full player hull. Tall walls and steep ramps still fail.
                float above[3],below[3];
                for(unsigned a=0;a<3;a++)above[a]=below[a]=floor[a]-t.normal[a]*.125f;
                above[2]+=18.125f;below[2]-=.125f;
                t=trace(ctx,above,zero,zero,below,false);
                if(!b2tp_trace_valid(&t) || t.fraction>=1 || t.sky ||
                   !b2tp_finite3(t.normal))return;
                memcpy(floor,t.end,sizeof(floor));
            }
            if(t.normal[2]<.70f)return;
            // Lift the origin by the plane support of the horizontal footprint.
            // A point-floor height alone intersects an uphill hull corner.
            for(unsigned a=0;a<2;a++)
                floor[2]-=t.normal[a]*(t.normal[a]>=0?h->mins[a]:h->maxs[a])/t.normal[2];
            floor[2]-=h->mins[2];floor[2]+=.125f;
            out->status=B2TP_Landing(h,floor,range,trace,occupants,ctx,false,out->landing,&out->ground);
            return;
        }
    }
}
static float B2TP_Comfort(uint32_t now,uint32_t accepted_ms,bool pulsing,bool enabled,float strength)
{
    if(!pulsing || !enabled || !isfinite(strength))return 0;
    uint32_t elapsed=now-accepted_ms;
    return elapsed>=280?0:fminf(1,fmaxf(0,strength))*(1.f-(float)elapsed/280.f);
}
#endif
