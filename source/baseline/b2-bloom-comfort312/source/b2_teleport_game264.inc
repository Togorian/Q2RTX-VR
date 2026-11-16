// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 1997-2001 Id Software, Inc.
// Copyright (C) 2026 Togorian
//
// VR teleport landing for Q2RTX VR; telefrag handling follows id Software's code.
// Original VR functions; see PROVENANCE.md.
// Modified 14 November 2026: authorship clarified; behaviour unchanged.
#include "b2_teleport_bridge.h"
static b2tp_trace_t b2tp_game_trace(void *ctx,const float start[3],const float mins[3],
    const float maxs[3],const float end[3],bool actors)
{
    edict_t *ent=ctx;
    trace_t t=gi.trace(start,mins,maxs,end,ent,actors?MASK_PLAYERSOLID:MASK_DEADSOLID);
    b2tp_trace_t out={0};out.fraction=t.fraction;out.startsolid=t.startsolid;out.allsolid=t.allsolid;
    out.sky=t.surface && (t.surface->flags&SURF_SKY);
    out.actor=t.ent && t.ent!=g_edicts && t.ent->solid!=SOLID_BSP;
    out.ground=(uintptr_t)t.ent;VectorCopy(t.endpos,out.end);VectorCopy(t.plane.normal,out.normal);
    return out;
}
static bool b2tp_overlap(const edict_t *hit,const float origin[3],const float mins[3],const float maxs[3])
{
    for(unsigned i=0;i<3;i++)if(hit->absmin[i]>=origin[i]+maxs[i] || hit->absmax[i]<=origin[i]+mins[i])return false;
    return true;
}
// Cosmetic fly callbacks do not move or resurrect a completed corpse.
void M_FliesOn(edict_t *self);
void M_FliesOff(edict_t *self);
static bool b2tp_settled_corpse(const edict_t *hit)
{
    // DEADMONSTER is published by the death-complete callback, not at health0.
    // Preplaced misc_deadsoldier uses NONE; completed monsters use TOSS.
    return !hit->client && (hit->svflags&SVF_DEADMONSTER) && hit->deadflag==DEAD_DEAD &&
        (!hit->nextthink || hit->think==M_FliesOn || hit->think==M_FliesOff) && (hit->movetype==MOVETYPE_NONE ||
        (hit->movetype==MOVETYPE_TOSS && hit->groundentity)) &&
        VectorLengthSquared(hit->velocity)==0 && VectorLengthSquared(hit->avelocity)==0;
}
static int b2tp_game_occupants(void *ctx,const float origin[3],const float mins[3],const float maxs[3],bool damage)
{
    edict_t *ent=ctx,*list[MAX_EDICTS];vec3_t lo,hi;
    // Ordinary water remains traversable. Reject hazardous destination volumes
    // for the entire hull at preview and every commit classification, before damage.
    trace_t hazard=gi.trace(origin,mins,maxs,origin,ent,CONTENTS_LAVA|CONTENTS_SLIME);
    if(hazard.startsolid || hazard.allsolid || hazard.fraction!=1.f)return -1;
    // Brush abs bounds are padded by SV_LinkEdict for broad-phase discovery.
    // They can overlap a clear landing .125 QU above a lift. Let the full
    // hull/playerclip trace classify brush geometry before classifying actors.
    b2tp_trace_t solid=b2tp_game_trace(ctx,origin,mins,maxs,origin,false);
    if(!b2tp_trace_valid(&solid) || solid.fraction!=1)return -1;
    for(unsigned i=0;i<3;i++){lo[i]=origin[i]+mins[i];hi[i]=origin[i]+maxs[i];}
    int count=gi.BoxEdicts(lo,hi,list,MAX_EDICTS,AREA_SOLID),eligible=0;
    if(count<0 || count>=MAX_EDICTS)return -1;
    for(int i=0;i<count;i++){
        edict_t *hit=list[i];
        if(!hit || hit==ent || !hit->inuse || hit->solid==SOLID_NOT ||
           hit->solid==SOLID_TRIGGER || hit->solid==SOLID_BSP || !b2tp_overlap(hit,origin,mins,maxs))continue;
        if(b2tp_settled_corpse(hit))continue;
        // Do not damage moving completed corpses or arbitrary damageable props.
        // A monster still in its death animation has not set DEADMONSTER yet.
        if((hit->svflags&SVF_DEADMONSTER) || !(hit->client || (hit->svflags&SVF_MONSTER)) ||
           !hit->takedamage)return -1;
        eligible++;
    }
    // Re-read at preview AND commit, including after a pending request. Complete
    // classification above still refuses unsupported occupants before any damage.
    if(eligible && gi.cvar("b2_telefrag","0",CVAR_ARCHIVE)->value!=1.f)return -1;
    if(damage)for(int i=0;i<count;i++){
        edict_t *hit=list[i];
        if(!hit || hit==ent || !hit->inuse || hit->solid==SOLID_NOT ||
           hit->solid==SOLID_TRIGGER || hit->solid==SOLID_BSP || !b2tp_overlap(hit,origin,mins,maxs))continue;
        if(b2tp_settled_corpse(hit))continue;
        vec3_t point;VectorCopy(origin,point);
        T_Damage(hit,ent,ent,vec3_origin,point,vec3_origin,100000,0,DAMAGE_NO_PROTECTION,MOD_TELEFRAG);
    }
    return eligible;
}
static bool b2tp_game_preview(const b2w_packet_t *aim,float range,b2tp_preview_t *out)
{
    if(!out)return false;memset(out,0,sizeof(*out));out->status=B2TP_INVALID;
    edict_t *ent=b2w_live_player();
    if(!b2w_host.teleport_enabled || !b2w_host.teleport_enabled() ||
       !b2w_game_packet(aim,ent) || aim->attack || aim->selection || level.intermission_framenum)return false;
    vec3_t corrected;
    if(!b2wn_clip(ent,aim->pose.origin,corrected) ||
       b2tp_distance2(corrected,aim->pose.origin)>.0001f)return false;
    b2tp_hull_t h;VectorCopy(ent->s.origin,h.origin);VectorCopy(ent->mins,h.mins);VectorCopy(ent->maxs,h.maxs);
    B2TP_Arc(&aim->pose,&h,range,b2tp_game_trace,b2tp_game_occupants,ent,out);
    return out->count>0;
}
static bool b2tp_use_allowed(edict_t *ent)
{
    b2w_packet_t weapon;
    return ent==b2w_live_player() && b2w_host.teleport_enabled && b2w_host.teleport_enabled() &&
        b2w_host.current && b2w_host.current(&weapon) && b2w_game_packet(&weapon,ent);
}
static bool b2tp_game_owner(const b2tp_request_t *r,edict_t *ent)
{
    b2w_packet_t weapon;
    return b2tp_use_allowed(ent) && b2w_host.current(&weapon) &&
        r->aim.pose.hand<=1u && r->aim.pose.serial==weapon.pose.serial &&
        r->aim.pose.pose_time==weapon.pose.pose_time && r->aim.game_epoch==weapon.game_epoch &&
        !memcmp(&r->aim.pose.epochs,&weapon.pose.epochs,sizeof(weapon.pose.epochs));
}
void B2W_TeleportMove(edict_t *ent,pmove_t *pm)
{
    b2tp_request_t r;
    if(!b2w_host.teleport_current || !b2w_host.teleport_result ||
       !b2w_host.teleport_current(&r))return;
    bool accepted=false;float origin[3]={0};uintptr_t ground=0;
    if(ent==b2w_live_player() && pm && pm->s.pm_type==PM_NORMAL && !pm->s.pm_time &&
       !level.intermission_framenum && b2w_host.teleport_enabled && b2w_host.teleport_enabled() &&
       b2w_game_packet(&r.aim,ent) && b2tp_game_owner(&r,ent) && !r.aim.attack && !r.aim.selection &&
       r.number && r.generation && r.executed && !r.acknowledged &&
       b2tp_finite3(r.head) && isfinite(r.range)){
        b2tp_hull_t h;
        for(unsigned i=0;i<3;i++){h.origin[i]=pm->s.origin[i]*.125f;h.mins[i]=pm->mins[i];h.maxs[i]=pm->maxs[i];}
        unsigned result=B2TP_Landing(&h,r.target,r.range,b2tp_game_trace,b2tp_game_occupants,ent,true,origin,&ground);
        accepted=result==B2TP_EMPTY || result==B2TP_OCCUPIED;
        // Damage callbacks can invalidate the requester or the floor; position has not yet changed.
        if(accepted && ent!=b2w_live_player())accepted=false;
    }
    if(accepted){
        for(unsigned i=0;i<3;i++){pm->s.origin[i]=(short)lroundf(origin[i]*8.f);pm->s.velocity[i]=0;}
        pm->s.pm_flags&=~(PMF_TIME_WATERJUMP|PMF_TIME_LAND);
        pm->s.pm_flags|=PMF_TIME_TELEPORT|PMF_ON_GROUND;pm->s.pm_time=2;
        pm->groundentity=(edict_t *)ground;pm->numtouch=0;
        pm->waterlevel=pm->watertype=0;
        float point[3]={origin[0],origin[1],origin[2]+pm->mins[2]+1};
        int contents=gi.pointcontents(point);
        if(contents&MASK_WATER){
            pm->watertype=contents;pm->waterlevel=1;point[2]=origin[2]+(pm->mins[2]+pm->viewheight)*.5f;
            if(gi.pointcontents(point)&MASK_WATER){pm->waterlevel=2;point[2]=origin[2]+pm->viewheight;
                if(gi.pointcontents(point)&MASK_WATER)pm->waterlevel=3;}
        }
        ent->s.event=EV_PLAYER_TELEPORT;
        // Pmove.applied owns origin/velocity. Only this authorised timer is admitted early.
        obs_trace.last.pm.pm_flags=pm->s.pm_flags;obs_trace.last.pm.pm_time=pm->s.pm_time;
        b2tp_moved=true;
    }
    if(!b2w_host.teleport_result(r.number,accepted,origin,accepted?pm->s.pm_flags:0,accepted?pm->s.pm_time:0))
        observer_walk_fault("teleport_acknowledgement_rejected",ent);
}
