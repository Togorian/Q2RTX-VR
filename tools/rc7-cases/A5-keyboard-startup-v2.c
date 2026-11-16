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
#include "current/right-menu409/source/b2_controller409.h"
#define Com_Printf(...) ((void)0)
enum {KEY_GAME=1,KEY_CONSOLE=2,KEY_MENU=4,PM_NORMAL=1,STAT_HEALTH=0,ca_active=1,B2B_RESUME=0,UIMENU_GAME=1,K_PAUSE=130};

#include "current/runtime-recovery555/source/b2_runtime555.h"
static struct {bool input_enabled,attached,exhausted;unsigned generation;bool menu_active316[2];}b2_actions={true,true,false,3,{false,false}};
static int event_at;
uint32_t B2XR_ActionGeneration594(void)
{
    // Profile events can advance this while the client's last snapshots stay
    // unchanged. Never use a cached snapshot as proof of the live contract.
    return b2_actions.input_enabled && b2_actions.attached && !b2_actions.exhausted &&
        b2_actions.generation!=UINT32_MAX ? b2_actions.generation : 0;
}
enum {XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED=1};
static void profile_event(void){switch(XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED){
        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
            memset(b2_actions.menu_active316,0,sizeof(b2_actions.menu_active316));
            if(b2_actions.generation==UINT32_MAX)b2_actions.exhausted=true;
            else ++b2_actions.generation;
            break;
}}

static unsigned now=1000,xr_generation=7,epoch=2,b2w_owner_epoch=1;
static bool focused=true,interaction=true,stop,hold_valid=true;
static struct {bool headset_waiting,frozen,ui_recovery_ready,ui_manual_pause,stopped,world_waiting,world_rebinding,recenter_pending,handoff_position_pending,lifecycle,combat,kbm_height_reference,origin_latched,ui_interrupted,ui_resume_request,walk_ready,walk_release_required;unsigned world_epoch,ui_xr_epoch,recenter_epoch;float reference_origin[3],reference_yaw;}b2_client;
static b2c_state_t b2_controller;static b2c_snapshot_t b2_controller_sample;
static struct {int key_dest,state;}cls;
static struct {struct{bool valid;struct{struct{int pm_type;}pmove;int stats[1];}ps;}frame;}cl;
typedef struct {int integer;float value;} cv;
static cv mode={1},dominant={1},turn={0},smooth={0},snap={0},tracked={0},teleport={0};
static cv *b2_input_mode484=&mode,*b2_controller_dominant=&dominant,*b2_controller_turn_mode=&turn,*b2_controller_smooth_speed=&smooth,*b2_controller_snap_angle=&snap,*b2_tracked_weapon=&tracked,*b2_teleport=&teleport;
static bool b2_menu_transition;static unsigned b2_menu_interruption;
static int b2rs_client,b2rs_stance;
static bool B2_HeadsetWaiting(void){return true;}static bool B2_UIEnabled(void){return true;}static bool B2_UIPaused(void){return true;}
static bool B2_InputFocused556(void){return focused;}static bool B2XR_InteractionReady(void){return interaction;}static bool B2XR_StopRequested(void){return stop;}
static unsigned B2XR_InputGeneration(void){return xr_generation;}static unsigned B2XR_InterruptionEpoch(void){return epoch;}
static bool b2_controller_enabled(void){return true;}static unsigned Sys_Milliseconds(void){return now;}
static int Key_IsDown(unsigned key){return 1;}static bool b2_ui_validate_hold(void){return hold_valid;}
static void b2_clear_walk_input(void){}static void B2RM_Reset(void){}static void b2_ui_apply_pause(void){}static int Cvar_VariableInteger(const char*n){return 0;}
static void b2_menu_cancel253(void){}static void b2_menu_route320(const b2c_snapshot_t*p,const char*s){}static void UI_OpenMenu(int n){}static void Con_Close(bool b){}
static void b2_menu_keys253(b2c_state_t*s,const b2c_snapshot_t*p,unsigned n,int d){}
static void b2c_step_turn(b2c_state_t*s,const b2c_snapshot_t*p,unsigned n,unsigned d,int hand,int a,int turn,float speed,float angle){}
// No controller resume binding, as with the Simple profile.
static float B2B_Value(const b2c_snapshot_t*p,int a,int h,int n){return 0;}static void B2B_Clear(b2c_snapshot_t*p,int a,int h){}
static struct {
    bool configured, armed, previous, pressed, pending, keyboard_request;
    uint32_t sample_epoch;
    uint64_t serial;
    uint32_t generation, xr_generation, owner, world, interruption;
    int dominant, turn, tracked, teleport;
    b2c_state_t ui;
    b2c_snapshot_t sample;
} b2_start;

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
static bool b2c_neutral(const b2c_snapshot_t *p)
{
    for (unsigned i=0;i<2;i++) {
        const b2c_hand_t *h=&p->hands[i];
        if (h->blocked556 || fabsf(h->stick[0])>=.25f || fabsf(h->stick[1])>=.25f ||
            h->trigger>=.45f || h->squeeze>=.35f || h->primary || h->secondary || h->menu || h->click) return false;
    }
    return true;
}
static void b2c_clear(b2c_state_t *s)
{
    memset(&s->menu_release409,0,sizeof(s->menu_release409));
    memset(&s->intent,0,sizeof(s->intent));
    s->wait_neutral=true; s->snap_ready=s->menu_ready=s->attack=s->crouch=false;
    s->previous_primary=s->previous_secondary=s->previous_menu=s->previous_click=false;
    s->valid=false;s->fire_ready556=false;
    s->turn_clock_valid=false; s->turn_ms=0;
}
static bool b2_headset_start_valid(uint32_t now)
{
    const b2c_snapshot_t *p=&b2_controller_sample;
    return b2_client.headset_waiting && B2_HeadsetWaiting() && B2_UIEnabled() &&
        b2_client.frozen && B2_UIPaused() && b2_client.ui_recovery_ready &&
        !b2_client.ui_manual_pause && !b2_client.stopped && !b2_controller.requested &&
        !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.recenter_pending && !b2_client.handoff_position_pending &&
        cls.state==ca_active && B2_InputFocused556() && cls.key_dest==KEY_GAME &&
        cl.frame.valid && cl.frame.ps.pmove.pm_type==PM_NORMAL && cl.frame.ps.stats[STAT_HEALTH]>0 &&
        b2_controller_enabled() && b2_controller_turn_mode->integer>=0 &&
        b2_controller_turn_mode->integer<=B2C_TURN_SMOOTH &&
        b2_client.lifecycle && b2_client.combat && b2w_owner_epoch && b2_client.world_epoch &&
        B2XR_InteractionReady() && !B2XR_StopRequested() && b2c_sample_valid(p,now) &&
        p->generation==B2XR_ActionGeneration594() && p->xr_generation && p->xr_generation==B2XR_InputGeneration() &&
        p->pose_time>0;
}
static bool b2_headset_start_sample594(uint32_t now)
{
    return b2_headset_start_valid(now) && b2c_sample_valid(&b2_start.sample,now) &&
        b2_start.sample.generation==b2_controller_sample.generation &&
        b2_start.sample.generation==B2XR_ActionGeneration594() &&
        b2_start.sample.xr_generation==B2XR_InputGeneration() &&
        b2_start.sample_epoch==B2XR_InterruptionEpoch() &&
        b2c_neutral(&b2_start.sample) && b2c_neutral(&b2_controller_sample);
}
static bool b2_headset_reference(const b2c_snapshot_t *p, float origin[3], float *yaw)
{
    const float *q=p->head_orientation; float norm=0;
    for (unsigned i=0;i<3;i++) if (!isfinite(p->head_position[i])) return false;
    for (unsigned i=0;i<4;i++) { if (!isfinite(q[i])) return false; norm+=q[i]*q[i]; }
    if (!isfinite(norm) || fabsf(norm-1.f)>.02f) return false;
    // Same horizontal heading geometry as b2_recenter_reference; no floor claim.
    float fx=-2.f*(q[0]*q[2]+q[1]*q[3])/norm;
    float fz=-(1.f-2.f*(q[0]*q[0]+q[1]*q[1])/norm);
    if (fx*fx+fz*fz<.0001f) return false;
    memcpy(origin,p->head_position,sizeof(float)*3);
    *yaw=atan2f(-fx,-fz);
    return true;
}
static void b2_headset_start_tick(uint32_t now)
{
    bool keyboard_request=b2_start.keyboard_request;
    b2_start.keyboard_request=false;
    const b2c_snapshot_t *p=&b2_controller_sample;
    b2_start.pending=false;
    // A separate UI reducer can open/navigate/close the ordinary menu while
    // main controller gameplay ownership remains false.
    b2_start.ui.requested=true;
    unsigned dest=(cls.key_dest&KEY_CONSOLE)?B2C_CONSOLE:
        (cls.key_dest&KEY_MENU)?B2C_MENU:B2C_GAME;
    b2c_step_turn(&b2_start.ui,p,now,dest,b2_controller_dominant->integer,0,
        b2_controller_turn_mode->integer,b2_controller_smooth_speed->value,b2_controller_snap_angle->value);
    if (dest!=B2C_MENU || !B2_InputFocused556() || !B2XR_InteractionReady()) b2_menu_cancel253();
    b2c_intent_t ui=b2_start.ui.intent;
    if (B2_InputFocused556() && B2XR_InteractionReady()) {
        if (ui.events&B2C_OPEN_MENU) {b2_menu_route320(p,"startup-game-menu");UI_OpenMenu(UIMENU_GAME);}
        else if (dest==B2C_CONSOLE && (ui.events&B2C_BACK)) Con_Close(true);
        else if (dest==B2C_MENU) {
            b2_menu_keys253(&b2_start.ui,p,now,2);
        }
    }
    if (!b2_headset_start_valid(now)) {
        b2_start.configured=b2_start.armed=b2_start.pressed=false;return;
    }
    if (keyboard_request && b2_headset_start_sample594(now) && !b2_start.ui.wait_neutral) {
        b2_start.pending=true;b2_start.sample=*p;b2_start.sample_epoch=B2XR_InterruptionEpoch();return;
    }
    if (b2_menu_transition && b2_menu_interruption==B2XR_InterruptionEpoch() &&
        b2c_neutral(p) && !b2_start.ui.wait_neutral) {
        b2_start.pending=true;b2_start.sample=*p;b2_start.sample_epoch=B2XR_InterruptionEpoch();return;
    }
    int hand=b2_controller_dominant->integer,turn=b2_controller_turn_mode->integer;
    int tracked=b2_tracked_weapon->integer,teleport=b2_teleport->integer;
    bool pressed=B2B_Value(p,B2B_RESUME,hand,0)>.5f;
    if (!b2_start.configured || b2_start.generation!=p->generation ||
        b2_start.xr_generation!=p->xr_generation || b2_start.owner!=b2w_owner_epoch ||
        b2_start.world!=b2_client.world_epoch || b2_start.interruption!=b2_client.ui_xr_epoch ||
        b2_start.dominant!=hand || b2_start.turn!=turn || b2_start.tracked!=tracked ||
        b2_start.teleport!=teleport || p->serial<b2_start.serial) {
        b2_start.configured=true;b2_start.armed=b2_start.pressed=false;
        b2_start.generation=p->generation;b2_start.xr_generation=p->xr_generation;
        b2_start.owner=b2w_owner_epoch;b2_start.world=b2_client.world_epoch;
        b2_start.interruption=b2_client.ui_xr_epoch;
        b2_start.dominant=hand;b2_start.turn=turn;b2_start.tracked=tracked;b2_start.teleport=teleport;
        b2_start.serial=p->serial;b2_start.previous=pressed;return;
    }
    if (p->serial==b2_start.serial) return;
    b2_start.serial=p->serial;
    b2c_snapshot_t released=*p;B2B_Clear(&released,B2B_RESUME,hand);
    bool neutral=b2c_neutral(&released);
    if (pressed && !b2_start.previous) {
        b2_start.pressed=b2_start.armed && neutral;b2_start.armed=false;
    } else if (!pressed) {
        b2_start.pending=b2_start.previous && b2_start.pressed && neutral;
        if (b2_start.pending) {b2_start.sample=*p;b2_start.sample_epoch=B2XR_InterruptionEpoch();}
        b2_start.pressed=false;b2_start.armed=neutral;
    } else if (!neutral) b2_start.pressed=false;
    b2_start.previous=pressed;
}
static void b2_headset_wait_unavailable(void)
{
    if (!b2_client.headset_waiting) return;
    b2_client.ui_recovery_ready=false;
    b2_start.configured=b2_start.armed=b2_start.pressed=b2_start.pending=b2_start.keyboard_request=false;
    b2c_clear(&b2_start.ui);
}
static bool key_request(bool down){unsigned key=K_PAUSE;
    if (key == K_PAUSE && b2_client.headset_waiting) {
        if (down && Key_IsDown(key)==1) {
            // Deliberate, one-use startup request. Unavailable presses cannot
            // replay after recovery; the frame still owns the final commit.
            b2_start.keyboard_request=b2_headset_start_valid(Sys_Milliseconds()) &&
                b2c_neutral(&b2_controller_sample);
            if (b2_start.keyboard_request) {
                b2_start.sample=b2_controller_sample;
                b2_start.sample_epoch=B2XR_InterruptionEpoch();
            }
        }
        return true;
    }
return false;}

static void frame_commit(bool accepted_pair,bool ended,bool renderer_committed,bool history_committed,unsigned wait){bool start_requested=b2_start.pending;b2_start.pending=false;bool initial_reference_ready=true,start_commit=false;float reference_origin[3]={0},reference_yaw=0;now+=wait;if(event_at==1)profile_event();
    if (b2_client.headset_waiting && start_requested && initial_reference_ready &&
        b2_headset_start_sample594(Sys_Milliseconds())) {
        start_commit=b2_headset_reference(&b2_start.sample,reference_origin,&reference_yaw);
    }
    if(event_at==2)profile_event();
    if (start_commit && accepted_pair && ended && renderer_committed && history_committed &&
        b2_headset_start_sample594(Sys_Milliseconds()) && b2_ui_validate_hold()) {
        // Position/yaw come from the one consumed release sample. No travel,
        // calibrated height or body displacement from pre-play survives.
        b2_clear_walk_input(); B2RM_Reset();
        memset(&b2rs_client,0,sizeof(b2rs_client));
        memset(&b2rs_stance,0,sizeof(b2rs_stance));
        b2_client.kbm_height_reference=false;
        memcpy(b2_client.reference_origin,reference_origin,sizeof(reference_origin));
        b2_client.reference_yaw=reference_yaw;
        b2_client.origin_latched=true; b2_client.recenter_epoch++;
        b2_client.headset_waiting=false; b2_client.ui_interrupted=false;
        b2_menu_transition=false;
        b2_client.ui_resume_request=false; b2_client.ui_recovery_ready=false;
        b2_client.walk_ready=true; b2_client.walk_release_required=true;
        b2_controller.requested=b2_input_mode484->integer!=0;
        memset(&b2_start,0,sizeof(b2_start));
        b2_ui_apply_pause();
        { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_HEADSET_START accepted owner=selected-input-mode reference=consumed-request both_eyes=1 body_unchanged=1 LOCAL_relative=1\n"); }
    }
}

static void reset(void){event_at=0;b2_actions.input_enabled=b2_actions.attached=true;b2_actions.exhausted=false;b2_actions.generation=3;memset(&b2_client,0,sizeof(b2_client));memset(&b2_start,0,sizeof(b2_start));memset(&b2_controller,0,sizeof(b2_controller));now=1000;epoch=2;xr_generation=7;focused=interaction=hold_valid=true;stop=false;b2_menu_transition=false;
b2_client.headset_waiting=b2_client.frozen=b2_client.ui_recovery_ready=b2_client.lifecycle=b2_client.combat=true;b2_client.world_epoch=1;b2_client.ui_xr_epoch=epoch;
cls.key_dest=KEY_GAME;cls.state=ca_active;cl.frame.valid=true;cl.frame.ps.pmove.pm_type=PM_NORMAL;cl.frame.ps.stats[0]=100;
b2_controller_sample=(b2c_snapshot_t){.ready=true,.serial=1,.generation=3,.xr_generation=7,.capture_ms=1000,.head_ms=1000,.pose_time=1,.head_position={2,1.8f,4},.head_orientation={0,0,0,1}};}
static void request(void){assert(key_request(true));b2_headset_start_tick(now);}
int main(void){for(int m=0;m<2;m++){reset();mode.integer=m;request();assert(b2_start.pending&&!b2_client.walk_ready);frame_commit(true,true,true,true,0);assert(!b2_client.headset_waiting&&b2_client.walk_ready&&b2_client.origin_latched&&b2_client.recenter_epoch==1);assert(b2_controller.requested==(m!=0));assert(b2_client.reference_origin[0]==2&&b2_client.reference_origin[1]==1.8f);}
for(int fail=0;fail<4;fail++){reset();request();frame_commit(fail!=0,fail!=1,fail!=2,fail!=3,0);assert(b2_client.headset_waiting&&!b2_client.walk_ready&&!b2_client.origin_latched&&!b2_client.recenter_epoch&&!b2_start.pending);}
reset();request();frame_commit(true,true,true,true,101);assert(b2_client.headset_waiting&&!b2_client.recenter_epoch);
for(int bad=0;bad<6;bad++){reset();if(bad==0)focused=false;if(bad==1)interaction=false;if(bad==2)b2_client.ui_recovery_ready=false;if(bad==3)now+=101;if(bad==4)b2_controller_sample.hands[0].trigger=.8f;if(bad==5)b2_controller_sample.xr_generation=8;
request();assert(!b2_start.pending);focused=interaction=true;b2_client.ui_recovery_ready=true;now=1000;b2_controller_sample.hands[0].trigger=0;b2_controller_sample.xr_generation=7;b2_headset_start_tick(now);assert(!b2_start.pending);}
reset();assert(key_request(true));b2_controller_sample.generation++;b2_headset_start_tick(now);assert(!b2_start.pending);
reset();assert(key_request(true));epoch++;b2_headset_start_tick(now);assert(!b2_start.pending);
reset();assert(key_request(true));b2_headset_wait_unavailable();assert(!b2_start.keyboard_request);b2_client.ui_recovery_ready=true;b2_headset_start_tick(now);assert(!b2_start.pending);
reset();assert(key_request(true));focused=false;b2_headset_start_tick(now);focused=true;b2_headset_start_tick(now);assert(!b2_start.pending);
reset();request();b2_controller_sample.generation++;frame_commit(true,true,true,true,0);assert(b2_client.headset_waiting);

for(int when=1;when<=2;when++){reset();request();assert(b2_start.pending);event_at=when;frame_commit(true,true,true,true,0);
assert(b2_start.sample.generation==3&&b2_controller_sample.generation==3&&b2_actions.generation==4);assert(now==1000&&focused&&interaction&&xr_generation==7&&epoch==2);
assert(b2_client.headset_waiting&&!b2_client.walk_ready&&!b2_client.origin_latched&&!b2_client.recenter_epoch&&!b2_controller.requested&&!b2_start.pending);
b2_controller_sample.generation=4;event_at=0;b2_headset_start_tick(now);assert(!b2_start.pending&&!b2_start.keyboard_request);}
for(int invalid=0;invalid<4;invalid++){reset();request();if(invalid==0)b2_actions.generation=0;if(invalid==1)b2_actions.exhausted=true;if(invalid==2)b2_actions.attached=false;if(invalid==3){b2_actions.generation=UINT32_MAX;profile_event();assert(b2_actions.exhausted);}
assert(B2XR_ActionGeneration594()==0);frame_commit(true,true,true,true,0);assert(b2_client.headset_waiting&&!b2_client.recenter_epoch&&!b2_start.pending);}

puts("actual live native profile-event generation and Pause/startup validity/request/commit: Simple profile, input mode preserved, four commit failures, stale/generation/non-neutral/unavailable/refocus no replay PASS");}
