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
static struct {bool recenter_pending,handoff_position_pending,origin_latched;struct{bool dead;}combat_state;float reference_origin[3],reference_yaw;unsigned recenter_epoch;}b2_client;
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
static void test(unsigned age,bool handoff_pending,bool ready,bool valid_generation,bool focused){

int frame=0;float reference_origin[3]={1,1.5f,2},reference_yaw=17;
bool accepted_pair=true,initial_reference_ready=true,recenter=false,handoff=false;
b2_client.recenter_pending=true;b2_client.handoff_position_pending=handoff_pending;b2_client.origin_latched=true;
b2_controller.requested=true;cl.frame.ps.pmove.pm_type=PM_NORMAL;
b2c_snapshot_t recenter_sample488={.ready=true,.serial=1,.generation=2,.xr_generation=7,.capture_ms=1000,.head_ms=1000,.head_orientation={0,0,0,1}};
bool recenter_sample_ready488=ready;now=1000+age;generation=valid_generation?7:8;interaction=focused;
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

bool usable=age<=100&&ready&&valid_generation&&focused;
assert(accepted_pair);assert(recenter==usable);assert(reset_controller_height==usable);
if(!usable){assert(reference_origin[0]==1&&reference_origin[1]==1.5f&&reference_origin[2]==2&&reference_yaw==17);assert(!handoff&&!reset_held_crouch);}
assert(b2_client.recenter_pending==handoff_pending);assert(!b2_client.recenter_epoch);
}
int main(void){for(int h=0;h<2;h++){test(100,h,true,true,true);test(101,h,true,true,true);test(500,h,true,true,true);test(0,h,false,true,true);test(0,h,true,false,true);test(0,h,true,true,false);test(0,h,true,true,true);}puts("actual recentre proposal after successful wait: age boundary, local pose rollback, fresh retry, generation/focus refusal PASS");}
