// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Include after the canonical pmove_t/trace_t declarations, engine or game.
#include "b2_roomscale_bridge.h"
static b2rs_trace_t b2rs_pm_trace(void *opaque,const float start[3],const float mins[3],
    const float maxs[3],const float end[3])
{
    pmove_t *pm=opaque;trace_t t=pm->trace(start,mins,maxs,end);
    b2rs_trace_t r={0};r.fraction=t.fraction;r.startsolid=t.startsolid;r.allsolid=t.allsolid;
    memcpy(r.end,t.endpos,sizeof(r.end));memcpy(r.normal,t.plane.normal,sizeof(r.normal));
    r.ground=(uintptr_t)t.ent;
    if(t.fraction<1 && t.ent && pm->numtouch<MAXTOUCH){
        bool seen=false;for(int i=0;i<pm->numtouch;i++)if(pm->touchents[i]==t.ent)seen=true;
        if(!seen)pm->touchents[pm->numtouch++]=t.ent;
    }
    return r;
}
static bool b2rs_apply_pmove(pmove_t *pm,const b2rm_request_t *request,float accepted[3])
{
    memset(accepted,0,3*sizeof(float));
    if(pm->s.pm_type!=PM_NORMAL || pm->s.pm_time || !pm->trace || !pm->pointcontents)return false;
    b2rs_hull_t h={0};h.ground=(uintptr_t)pm->groundentity;
    for(unsigned i=0;i<3;i++){h.origin[i]=pm->s.origin[i]*.125f;h.mins[i]=pm->mins[i];h.maxs[i]=pm->maxs[i];}
    if(!B2RS_Move(&h,request->movement,b2rs_pm_trace,pm,accepted))return false;
    // Categorise the newly accepted position; no extra acceleration, velocity,
    // jump, duck, timers or command integration is performed.
    float down[3]={h.origin[0],h.origin[1],h.origin[2]-.25f};
    b2rs_trace_t ground=b2rs_pm_trace(pm,h.origin,h.mins,h.maxs,down);
    h.ground=pm->s.velocity[2]<=1440 && b2rs_trace_valid(&ground) && !ground.startsolid && !ground.allsolid &&
        ground.fraction<1 && ground.normal[2]>=.7f ? ground.ground : 0;
    for(unsigned i=0;i<3;i++)pm->s.origin[i]=(short)lroundf(h.origin[i]*8.f);
    pm->groundentity=(struct edict_s *)h.ground;
    if(h.ground)pm->s.pm_flags|=PMF_ON_GROUND;else pm->s.pm_flags&=~PMF_ON_GROUND;
    pm->waterlevel=pm->watertype=0;
    float point[3]={h.origin[0],h.origin[1],h.origin[2]+h.mins[2]+1};
    int contents=pm->pointcontents(point);
    if(contents&MASK_WATER){
        pm->watertype=contents;pm->waterlevel=1;
        point[2]=h.origin[2]+h.mins[2]+(pm->viewheight-h.mins[2])*.5f;
        if(pm->pointcontents(point)&MASK_WATER){
            pm->waterlevel=2;point[2]=h.origin[2]+pm->viewheight;
            if(pm->pointcontents(point)&MASK_WATER)pm->waterlevel=3;
        }
    }
    return true;
}
