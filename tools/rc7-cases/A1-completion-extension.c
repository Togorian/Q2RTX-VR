// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Retained CPU fixture: selected production spans are verified by rc7-regression.py.
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

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
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define B2C_MAX_AGE_MS 100
#define B2B_CROUCH 1
#define PM_NORMAL 1
#define Com_Printf(...) ((void)0)
typedef struct {bool ready;unsigned serial,generation,xr_generation,capture_ms,head_ms;float head_position[3],head_orientation[4];} b2c_snapshot_t;
static unsigned now=1000,generation=7;static bool interaction=true;
static unsigned Sys_Milliseconds(void){return now;}
static bool B2XR_InteractionReady(void){return interaction;}
static unsigned B2XR_InputGeneration(void){return generation;}
static int B2B_Hand(void){return 0;}
static float crouch;static float B2B_Value(const b2c_snapshot_t*p,int a,int h,int i){return crouch;}
static struct {bool recenter_pending,handoff_position_pending,origin_latched,walking,walk_ready,walk_release_required,stopped,recenter_crouch_held;float controller_reference_height;struct{bool dead;}combat_state;float reference_origin[3],reference_yaw;unsigned recenter_epoch;}b2_client;
static struct {bool requested;}b2_controller;
static struct {struct{struct{struct{int pm_type;}pmove;}ps;}frame;}cl;
static bool B2RS_HeightValid(float h){return isfinite(h)&&h>0&&h<3;}
static bool b2_recenter_reference(void*f,float*o,float*y){o[0]=9;o[1]=1.8f;o[2]=8;*y=90;return true;}
static bool b2_handoff_reference(void*f,float*o){o[0]=8;return true;}
static void b2rs_reference(float*o){}
static bool b2c_sample_valid(const b2c_snapshot_t *p,uint32_t now)
{
    if(!p->ready || !p->serial || !p->generation ||
       (uint32_t)(now-p->capture_ms)>B2C_MAX_AGE_MS ||
       (uint32_t)(now-p->head_ms)>B2C_MAX_AGE_MS)return false;
    // Actions and poses are admitted independently by the native reader.
    // The shared envelope remains bound to the actual finite tracked head.
    float norm=0;
    for(unsigned i=0;i<3;i++)if(!isfinite(p->head_position[i]))return false;
    for(unsigned i=0;i<4;i++){if(!isfinite(p->head_orientation[i]))return false;norm+=p->head_orientation[i]*p->head_orientation[i];}
    return isfinite(norm) && fabsf(norm-1.f)<=.02f;
}
static bool b2_recenter_sample488(const b2c_snapshot_t *sample,bool *held)
{
    if (!B2XR_InteractionReady() || !sample->xr_generation ||
        sample->xr_generation!=B2XR_InputGeneration() ||
        !b2c_sample_valid(sample,Sys_Milliseconds())) return false;
    float value=B2B_Value(sample,B2B_CROUCH,B2B_Hand(),0);
    if (!isfinite(value)) return false;
    *held=value>.35f; // retain the existing crouch release threshold
    return true;
}

static int height_commits,retirements,reads;static float last_origin[3],last_yaw;
static bool B2XR_ReadControllerInput(b2c_snapshot_t*p){reads++;*p=(b2c_snapshot_t){.ready=true,.serial=reads,.generation=2,.xr_generation=7,.capture_ms=now,.head_ms=now,.head_orientation={0,0,0,1}};return true;}
static void b2rs_retire(void){retirements++;}static void b2_reference_height_committed480(void){}
static void b2_recenter_height484(const float*o,bool held){height_commits++;b2_client.controller_reference_height=o[1];b2_client.recenter_crouch_held=held;}
static int Cvar_VariableInteger(const char*p){return 0;}static bool R_B2SettingsRejected580(void){return false;}
static bool stop_requested;static bool B2XR_StopRequested(void){return stop_requested;}
static void b2_stop_at(const char*s,int line){b2_client.stopped=true;}

static void step(unsigned wait,bool ended,bool renderer_committed,bool history_committed){
struct{bool should_render,tracking_valid;unsigned logical_index;}frame={true,true,9};
float reference_origin[3];memcpy(reference_origin,b2_client.reference_origin,sizeof(reference_origin));float reference_yaw=b2_client.reference_yaw;
bool initial_reference_ready=true,accepted_pair=true,recenter=false,handoff=false;
    b2c_snapshot_t recenter_sample488={0};
    bool recenter_sample_ready488=false;
    if (b2_client.recenter_pending && b2_controller.requested &&
        !b2_client.combat_state.dead && cl.frame.ps.pmove.pm_type==PM_NORMAL)
        recenter_sample_ready488=B2XR_ReadControllerInput(&recenter_sample488);

now+=wait; // successful XR wait seam, unchanged generation/focus/epoch
    float previous_reference_origin[3];
    memcpy(previous_reference_origin,reference_origin,sizeof(previous_reference_origin));
    float previous_reference_yaw=reference_yaw;
    bool recenter_deferred=false;
    if (b2_client.recenter_pending) {
        // A coincident Home owns heading. Keep it pending with the handoff
        // until the entire frame commits, including during keyboard recovery interruptions.
        if (!b2_client.handoff_position_pending) b2_client.recenter_pending = false;
        recenter = initial_reference_ready && b2_client.origin_latched &&
            b2_recenter_reference(&frame, reference_origin, &reference_yaw);
        if (!recenter) Com_Printf("B2RECENTER rejected valid_horizontal_pose_required=1\n");
        handoff = b2_client.handoff_position_pending && recenter;
    } else if (b2_client.handoff_position_pending) {
        handoff = initial_reference_ready && b2_client.origin_latched &&
            b2_handoff_reference(&frame, reference_origin);
    }
    bool reset_controller_height=recenter && b2_controller.requested &&
        !b2_client.combat_state.dead && cl.frame.ps.pmove.pm_type==PM_NORMAL;
    bool reset_held_crouch=false;
    if (reset_controller_height && (!B2RS_HeightValid(reference_origin[1]) ||
        !recenter_sample_ready488 || !b2_recenter_sample488(&recenter_sample488,&reset_held_crouch))) {
        // A successful frame wait may age the captured actions. Cancel only
        // this proposal, including its local pose; no height/epoch is committed.
        // A coincident handoff stays held and captures fresh actions next frame.
        memcpy(reference_origin,previous_reference_origin,sizeof(previous_reference_origin));
        reference_yaw=previous_reference_yaw;
        recenter=handoff=reset_controller_height=reset_held_crouch=false;
        recenter_deferred=true;
    }
    if (b2_client.handoff_position_pending && !handoff && !recenter_deferred) accepted_pair=false;
    if(b2_client.origin_latched && !recenter && !handoff)b2rs_reference(reference_origin);

memcpy(last_origin,reference_origin,sizeof(last_origin));last_yaw=reference_yaw;
    if ((recenter || handoff) && accepted_pair && ended && renderer_committed && history_committed) {
        // The new reference already absorbs the tracked displacement.
        // Retire against the old reference before overwriting it.
        b2rs_retire();
        b2_reference_height_committed480();
        if (reset_controller_height) b2_recenter_height484(reference_origin,reset_held_crouch);
        memcpy(b2_client.reference_origin, reference_origin, sizeof(reference_origin));
        b2_client.reference_yaw = reference_yaw;
        b2_client.recenter_epoch++;
        if (handoff) {
            b2_client.handoff_position_pending=false;
            b2_client.recenter_pending=false;
            b2_client.walk_release_required=true;
        }
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2REFERENCE committed home=%d handoff=%d pair=%u epoch=%u reference=%.9g,%.9g,%.9g yaw=%.9g both_eyes=1 history_seeded=1 body_unchanged=1\n",
            recenter,handoff,frame.logical_index, b2_client.recenter_epoch, reference_origin[0],
            reference_origin[1], reference_origin[2], reference_yaw);
    }
    if (b2_client.walking) {
        if (accepted_pair && ended && renderer_committed) {
            if (b2_client.origin_latched) {
                if (!b2_client.walk_ready)
                    b2_client.walk_release_required = true;
                b2_client.walk_ready = true;
            }
        } else if (!(R_B2SettingsRejected580() && ended && !B2XR_StopRequested()) &&
            (b2_client.walk_ready || (frame.should_render && frame.tracking_valid) ||
            !ended || B2XR_StopRequested())) {
            Com_Printf("B2WALK: XR pose/presentation pair unavailable; terminal stop.\n");
            Com_Printf("B2PAIR refused pair=%d ended=%d renderer=%d history=%d recenter=%d handoff=%d reset_height=%d sample_ready=%d sample_generation=%u current_generation=%u\n",
                accepted_pair,ended,renderer_committed,history_committed,recenter,handoff,
                reset_controller_height,recenter_sample_ready488,recenter_sample488.xr_generation,B2XR_InputGeneration());
            b2_stop_at(__func__, __LINE__);
        }
        // Before activation only: ordinary runtime no-render/not-tracked cycles
        // may wait under the native deadline. No eye was rendered in this path;
        // malformed views, failed eye ownership/end-frame and XR faults stop.
    }

}
static void reset(void){memset(&b2_client,0,sizeof(b2_client));b2_client.walking=b2_client.walk_ready=b2_client.origin_latched=b2_client.recenter_pending=b2_client.handoff_position_pending=true;
b2_client.reference_origin[0]=1;b2_client.reference_origin[1]=1.5f;b2_client.reference_origin[2]=2;b2_client.reference_yaw=17;b2_client.controller_reference_height=1.5f;b2_client.recenter_crouch_held=false;
b2_controller.requested=true;cl.frame.ps.pmove.pm_type=PM_NORMAL;now=1000;generation=7;interaction=true;stop_requested=false;height_commits=retirements=reads=0;crouch=.8f;}
int main(void){reset();step(101,true,true,true);assert(reads==1&&!b2_client.stopped);assert(b2_client.recenter_pending&&b2_client.handoff_position_pending);assert(!b2_client.recenter_epoch&&!height_commits&&!retirements&&!b2_client.recenter_crouch_held&&b2_client.controller_reference_height==1.5f);
assert(last_origin[0]==1&&last_origin[1]==1.5f&&last_origin[2]==2&&last_yaw==17);assert(!memcmp(last_origin,b2_client.reference_origin,sizeof(last_origin))&&b2_client.reference_yaw==17);
step(0,true,true,true);assert(reads==2&&!b2_client.stopped&&height_commits==1&&retirements==1&&b2_client.recenter_epoch==1);assert(!b2_client.recenter_pending&&!b2_client.handoff_position_pending);assert(b2_client.recenter_crouch_held&&b2_client.controller_reference_height==1.8f);assert(b2_client.reference_origin[0]==9&&b2_client.reference_yaw==90);
for(int failure=0;failure<3;failure++){reset();step(0,failure!=0,failure!=1,failure!=2);assert(!height_commits&&!retirements&&!b2_client.recenter_epoch&&!b2_client.recenter_crouch_held);assert(b2_client.reference_yaw==17&&b2_client.controller_reference_height==1.5f);if(failure<2)assert(b2_client.stopped);}
puts("actual capture/proposal/commit/terminal spans: sequential stale-to-fresh handoff, crouch/height/local/persistent reference, failed XR/renderer commit still stops PASS");}
