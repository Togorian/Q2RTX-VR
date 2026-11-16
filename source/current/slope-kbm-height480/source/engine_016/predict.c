/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/* Modified by Togorian with AI assistance, 12 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */


#include "client.h"
#include "reference_b_b2.h"
#ifdef REFERENCE_B_B2
#include "../../include/refresh/b2_roomscale_pmove.inc"
#include "../../include/refresh/b2_teleport_bridge.h"
#include "../../include/refresh/b2_wall_query.h"
#endif

/*
===================
CL_CheckPredictionError
===================
*/
void CL_CheckPredictionError(void)
{
#ifdef REFERENCE_B_B2
    if (!B2_CombatSnapshot()) return;
#endif
    int         frame;
    int         delta[3];
    unsigned    cmd;
    int         len;

    if (cls.demo.playback) {
        return;
    }

    if (sv_paused->integer) {
        VectorClear(cl.prediction_error);
        return;
    }

    if (!cl_predict->integer || (cl.frame.ps.pmove.pm_flags & PMF_NO_PREDICTION))
        return;

    // calculate the last usercmd_t we sent that the server has processed
    frame = cls.netchan.incoming_acknowledged & CMD_MASK;
    cmd = cl.history[frame].cmdNumber;

    // compare what the server returned with what we had predicted it to be
    VectorSubtract(cl.frame.ps.pmove.origin, cl.predicted_origins[cmd & CMD_MASK], delta);

    // save the prediction error for interpolation
    len = abs(delta[0]) + abs(delta[1]) + abs(delta[2]);
    if (len < 1 || len > 640) {
        // > 80 world units is a teleport or something
        VectorClear(cl.prediction_error);
        return;
    }

    SHOWMISS("prediction miss on %i: %i (%d %d %d)\n",
             cl.frame.number, len, delta[0], delta[1], delta[2]);

    // don't predict steps against server returned data
    if (cl.predicted_step_frame <= cmd)
        cl.predicted_step_frame = cmd + 1;

    VectorCopy(cl.frame.ps.pmove.origin, cl.predicted_origins[cmd & CMD_MASK]);

    // save for error interpolation
    VectorScale(delta, 0.125f, cl.prediction_error);
}

/*
====================
CL_ClipMoveToEntities
====================
*/
static void CL_ClipMoveToEntities(trace_t *tr, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int contentmask)
{
    int         i;
    trace_t     trace;
    mnode_t     *headnode;
    centity_t   *ent;
    mmodel_t    *cmodel;

    for (i = 0; i < cl.numSolidEntities; i++) {
        ent = cl.solidEntities[i];

        if (ent->current.solid == PACKED_BSP) {
            // special value for bmodel
            cmodel = cl.model_clip[ent->current.modelindex];
            if (!cmodel)
                continue;
            headnode = cmodel->headnode;
        } else {
            headnode = CM_HeadnodeForBox(ent->mins, ent->maxs);
        }

        if (tr->allsolid)
            return;

        CM_TransformedBoxTrace(&trace, start, end,
                               mins, maxs, headnode, contentmask,
                               ent->current.origin, ent->current.angles);

        CM_ClipEntity(tr, &trace, (struct edict_s *)ent);
    }
}

/*
================
CL_Trace
================
*/
void CL_Trace(trace_t *tr, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int contentmask)
{
    // check against world
    CM_BoxTrace(tr, start, end, mins, maxs, cl.bsp->nodes, contentmask);
    if (tr->fraction < 1.0f)
        tr->ent = (struct edict_s *)cl_entities;

    // check all other solid models
    CL_ClipMoveToEntities(tr, start, mins, maxs, end, contentmask);
}

#ifdef REFERENCE_B_B2
// SPDX-License-Identifier: GPL-2.0-or-later
float CL_B2WallDepth(const vec3_t start,const vec3_t end)
{
    if(!B2WQ_VectorFinite(start)||!B2WQ_VectorFinite(end)||VectorCompare(start,end))return 0;
    b2_wall_query_t q={0};q.enter=1;q.valid=true;
    const vec3_t zero={0};
    CM_B2WallObstruction(&q,start,end,cl.bsp->nodes,MASK_PLAYERSOLID,zero,zero);
    // Same current solid-entity list and model/box selection as CL_Trace.
    // The earliest obstructing entry is independent of the camera's current leaf.
    for(int i=0;i<cl.numSolidEntities && q.valid;i++) {
        centity_t *ent=cl.solidEntities[i];
        mnode_t *headnode;
        if(ent->current.solid==PACKED_BSP) {
            mmodel_t *model=cl.model_clip[ent->current.modelindex];
            if(!model)continue;
            headnode=model->headnode;
        } else headnode=CM_HeadnodeForBox(ent->mins,ent->maxs);
        CM_B2WallObstruction(&q,start,end,headnode,MASK_PLAYERSOLID,
            ent->current.origin,ent->current.angles);
    }
    // Normal distance beyond the first blocking entry face includes open space
    // beyond its far face. No previous hit, facing or total-distance latch.
    return q.valid && isfinite(q.depth) && q.depth>0 && q.depth<=FLT_MAX?(float)q.depth:0;
}
#endif

static trace_t q_gameabi CL_PMTrace(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end)
{
    trace_t t;
    CL_Trace(&t, start, mins, maxs, end, MASK_PLAYERSOLID);
    return t;
}

static int CL_PointContents(const vec3_t point)
{
    int         i;
    centity_t   *ent;
    mmodel_t    *cmodel;
    int         contents;

    contents = CM_PointContents(point, cl.bsp->nodes);

    for (i = 0; i < cl.numSolidEntities; i++) {
        ent = cl.solidEntities[i];

        if (ent->current.solid != PACKED_BSP) // special value for bmodel
            continue;

        cmodel = cl.model_clip[ent->current.modelindex];
        if (!cmodel)
            continue;

        contents |= CM_TransformedPointContents(
                        point, cmodel->headnode,
                        ent->current.origin,
                        ent->current.angles);
    }

    return contents;
}

/*
=================
CL_PredictMovement

Sets cl.predicted_origin and cl.predicted_angles
=================
*/
void CL_PredictAngles(void)
{
#ifdef REFERENCE_B_B2
    // Player lerp can request angles during packet parsing, before the usual
    // error-check callback and render-driven world bind.
    if (!B2_CombatSnapshot()) return;
#endif
    cl.predicted_angles[0] = cl.viewangles[0] + SHORT2ANGLE(cl.frame.ps.pmove.delta_angles[0]);
    cl.predicted_angles[1] = cl.viewangles[1] + SHORT2ANGLE(cl.frame.ps.pmove.delta_angles[1]);
    cl.predicted_angles[2] = cl.viewangles[2] + SHORT2ANGLE(cl.frame.ps.pmove.delta_angles[2]);
}

#ifdef REFERENCE_B_B2
// Retained command origins can span enough continuous uphill travel to pass the
// legacy 8-unit stair threshold. Do not restart stair smoothing for that rise.
// Both hull supports must lie on the same sloped plane; real risers, lifts and
// unsupported displacement keep the existing step path. Physics is untouched.
static bool CL_B2ContinuousSlope(const pmove_t *move, const short *old_origin)
{
    if (!B2_AdvancingWorld() ||
        move->s.pm_type != PM_NORMAL)
        return false;

    vec3_t old, current, down;
    VectorScale(old_origin, 0.125f, old);
    VectorScale(move->s.origin, 0.125f, current);
    VectorCopy(old, down);
    down[2] -= 1.f;
    trace_t a = CL_PMTrace(old, move->mins, move->maxs, down);
    VectorCopy(current, down);
    down[2] -= 1.f;
    trace_t b = CL_PMTrace(current, move->mins, move->maxs, down);
    if (a.startsolid || a.allsolid || b.startsolid || b.allsolid ||
        a.fraction == 1.f || b.fraction == 1.f || a.ent != b.ent ||
        a.plane.normal[2] < 0.7f || a.plane.normal[2] >= 0.9999f ||
        fabsf(a.plane.dist - b.plane.dist) > 0.0001f)
        return false;
    for (int i = 0; i < 3; ++i)
        if (fabsf(a.plane.normal[i] - b.plane.normal[i]) > 0.0001f)
            return false;
    float rise = -(a.plane.normal[0] * (current[0] - old[0]) +
                   a.plane.normal[1] * (current[1] - old[1])) / a.plane.normal[2];
    // Each snapped origin component is quantized to one eighth of a unit.
    return fabsf((current[2] - old[2]) - rise) <= 0.25f;
}
#endif

void CL_PredictMovement(void)
{
#ifdef REFERENCE_B_B2
    // Packet error checking and per-refresh movement prediction share the
    // same publication boundary; waiting is a no-prediction result.
    if (!B2_CombatSnapshot()) return;
#endif
    unsigned    ack, current, frame;
    pmove_t     pm;
    int         step, oldz;

    if (cls.state != ca_active) {
        return;
    }

    if (cls.demo.playback) {
        return;
    }

    if (sv_paused->integer) {
        return;
    }

    if (!cl_predict->integer || (cl.frame.ps.pmove.pm_flags & PMF_NO_PREDICTION)) {
        // just set angles
        CL_PredictAngles();
        return;
    }

    ack = cl.history[cls.netchan.incoming_acknowledged & CMD_MASK].cmdNumber;
    current = cl.cmdNumber;
#ifdef REFERENCE_B_B2
    B2RM_PredictionBegin(ack);B2TP_PredictionBegin(ack);
#endif

    // if we are too far out of date, just freeze
    if (current - ack > CMD_BACKUP - 1) {
        SHOWMISS("%i: exceeded CMD_BACKUP\n", cl.frame.number);
        return;
    }

    if (!cl.cmd.msec && current == ack) {
        SHOWMISS("%i: not moved\n", cl.frame.number);
        return;
    }

    // copy current state to pmove
    memset(&pm, 0, sizeof(pm));
    pm.trace = CL_PMTrace;
    pm.pointcontents = CL_PointContents;
    pm.s = cl.frame.ps.pmove;

    // run frames
    while (++ack <= current) {
        pm.cmd = cl.cmds[ack & CMD_MASK];
        Pmove(&pm, &cl.pmp);
#ifdef REFERENCE_B_B2
        b2rm_request_t room_request;
        if(B2RM_PredictionRequest(ack,&room_request)){
            float accepted[3]={0};b2rs_apply_pmove(&pm,&room_request,accepted);
            B2RM_Predicted(ack,accepted);
        }
        b2tp_request_t teleport;
        if(pm.s.pm_type==PM_NORMAL && B2TP_Prediction(ack,&teleport)){
            for(unsigned i=0;i<3;i++){pm.s.origin[i]=(short)lroundf(teleport.accepted_origin[i]*8.f);pm.s.velocity[i]=0;}
            pm.s.pm_flags=teleport.accepted_flags;pm.s.pm_time=teleport.accepted_time;
        }
#endif

        // save for debug checking
        VectorCopy(pm.s.origin, cl.predicted_origins[ack & CMD_MASK]);
    }

    // run pending cmd
    if (cl.cmd.msec) {
        pm.cmd = cl.cmd;
        pm.cmd.forwardmove = cl.localmove[0];
        pm.cmd.sidemove = cl.localmove[1];
        pm.cmd.upmove = cl.localmove[2];
        Pmove(&pm, &cl.pmp);
        frame = current;

        // save for debug checking
        VectorCopy(pm.s.origin, cl.predicted_origins[(current + 1) & CMD_MASK]);
    } else {
        frame = current - 1;
    }

    if (pm.s.pm_type != PM_SPECTATOR && (pm.s.pm_flags & PMF_ON_GROUND)) {
        oldz = cl.predicted_origins[cl.predicted_step_frame & CMD_MASK][2];
        step = pm.s.origin[2] - oldz;
        if (step > 63 && step < 160
#ifdef REFERENCE_B_B2
            && !CL_B2ContinuousSlope(&pm, cl.predicted_origins[cl.predicted_step_frame & CMD_MASK])
#endif
        ) {
            cl.predicted_step = step * 0.125f;
            cl.predicted_step_time = cls.realtime;
            cl.predicted_step_frame = frame + 1;    // don't double step
        }
    }

    if (cl.predicted_step_frame < frame) {
        cl.predicted_step_frame = frame;
    }

    // copy results out for rendering
    VectorScale(pm.s.origin, 0.125f, cl.predicted_origin);
    VectorScale(pm.s.velocity, 0.125f, cl.predicted_velocity);
    VectorCopy(pm.viewangles, cl.predicted_angles);
}

