// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 14 November 2026: historical RC6 fixture regenerated from the selected
// Q2RTX VR/Quake II RTX source spans listed in R6.json; external effects are seams.
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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "current/right-menu409/source/b2_controller409.h"
#define KEY_GAME 1
#define KEY_MENU 2
#define ACT_ACTIVATED 1
#define ACT_MINIMIZED 2
#define PM_NORMAL 0
#define STAT_HEALTH 0
#define ca_active 1
#define K_PAUSE 19
static struct {bool running,focused,tracking,failed,reference_refused,exit_requested,loss_pending;} xr;
static struct {int active,state,key_dest;} cls;
static struct {struct {bool valid;struct {struct {int pm_type;} pmove;int stats[2];} ps;} frame;} cl;
static struct {bool ui_interrupted,ui_resume_request,ui_recovery_ready,ui_manual_resume582,ui_manual_pause,stopped,walk_ready,world_waiting,world_rebinding,headset_waiting,recenter_pending,handoff_position_pending,walk_release_required,audio_pending;
unsigned ui_good_frames582;uint64_t ui_good_serial582;uint32_t ui_good_capture582,ui_good_input582,ui_good_actions582,ui_xr_epoch;
struct {bool dead;} combat_state;struct {bool valid;} history;} b2_client;
static b2c_state_t b2_controller;
static b2c_snapshot_t b2_controller_sample;
static struct {bool pending;} b2_resume;
static bool b2_menu_transition,realign,hold_valid=true,read_ok=true,key_down,head_ok=true;
static uint32_t b2_menu_interruption,epoch,now_ms,read_advance;
static int mode=1,release_calls,pause_calls;
typedef struct {uint64_t serial;uint32_t captured_ms,generation;bool valid;} b2rs_head_t;
static b2rs_head_t head;
static int Cvar_VariableInteger(const char *s){return !strcmp(s,"b2_input_mode")?mode:0;}
static uint32_t Sys_Milliseconds(void){return now_ms;}
static bool B2XR_Requested(void){return true;}
static uint32_t B2XR_InputGeneration(void){return head.generation;}
static uint32_t B2XR_InterruptionEpoch(void){return epoch;}
static bool B2XR_StopRequested(void){return xr.failed||xr.exit_requested||xr.loss_pending;}
static bool B2_RealignActive391(void){return realign;}
static bool B2_UIEnabled(void){return true;}
static bool B2_UISurfaceActive(void){return false;}
static bool B2_KeepFramePump(void){return true;}
static void B2XR_PollEvents(void){}
static void b2_realign_begin391(void){}
static void b2_stop_at(const char *s,int line){b2_client.stopped=true;}
static void B2B_Invalidate(void){}
static void b2_headset_wait_unavailable(void){}
static void b2_resume_reset(void){b2_resume.pending=false;}
static void B2H_Reset(void){}
static void b2_clear_walk_input(void){memset(&b2_controller.intent,0,sizeof(b2_controller.intent));}
static void CL_B2ReleaseButtons(void){release_calls++;}
static void b2_ui_apply_pause(void){pause_calls++;}
static bool b2_ui_validate_hold(void){return hold_valid;}
static bool b2_controller_enabled(void){return true;}
static bool b2_controller_resume_commit_ready(uint32_t n){return true;}
static bool Key_IsDown(int key){return key_down&&key==1;}
static bool B2XR_ReadHeadInput(b2rs_head_t *p){*p=head;return head_ok;}
static bool B2XR_ReadControllerInput(b2c_snapshot_t *p){now_ms+=read_advance;*p=b2_controller_sample;p->capture_ms=now_ms;return read_ok;}
#define Com_Printf(...) ((void)0)

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

bool B2XR_Focused582(void)
{
    return xr.running && xr.focused && !xr.failed && !xr.reference_refused &&
        !xr.exit_requested && !xr.loss_pending;
}

bool B2XR_InteractionReady(void)
{
    return xr.running && xr.focused && xr.tracking && !xr.failed && !xr.reference_refused &&
        !xr.exit_requested && !xr.loss_pending;
}

bool B2_ControllerFocusMode556(void)
{
    return B2XR_Requested() && Cvar_VariableInteger("b2_input_mode")==1;
}

bool B2_InputFocused556(void)
{
    return B2_ControllerFocusMode556()?B2XR_InteractionReady():cls.active==ACT_ACTIVATED;
}

static bool b2_resume_neutral256(uint32_t now)
{
    for (int key=0;key<256;key++) if (key!=K_PAUSE && Key_IsDown(key)) return false;
    if (!b2_controller.requested) return true;
    const b2c_snapshot_t *p=&b2_controller_sample;
    return b2_controller_enabled() && b2c_sample_valid(p,now) && b2c_neutral(p) &&
        p->serial==b2_controller.serial && p->generation==b2_controller.generation &&
        p->xr_generation==B2XR_InputGeneration();
}

static void b2_interrupt_kind582(const char *reason, bool manual)
{
    B2B_Invalidate();
    b2_headset_wait_unavailable();
    b2_resume_reset();
    B2H_Reset();
    if (!B2_UIEnabled() || !b2_client.walk_ready || b2_client.stopped) return;
    b2_client.ui_resume_request = b2_client.ui_recovery_ready = false;
    if (!b2_client.ui_interrupted) b2_client.ui_manual_resume582 = false;
    b2_client.ui_manual_resume582 |= manual; // A stronger reason always wins.
    b2_client.ui_good_frames582 = 0;
    if (!b2_client.ui_interrupted) {
        // An interruption is a new input-ownership boundary even when a
        // manual/menu hold already exists. Keep its snapshot, discard input.
        b2_clear_walk_input(); CL_B2ReleaseButtons();
        b2_client.walk_release_required = true;
        b2_client.ui_interrupted = true;
        b2_client.history.valid = false;
        b2_client.audio_pending = false;
        Com_Printf("B2_INTERRUPTION hold reason=%s; %s Manual pause/menu stays unchanged.\n", reason,
            manual ? "Restore focus/headset, release controls, then use the bound resume control or Pause." :
                     "Waiting for three fresh stable frames and released controls to resume automatically.");
    }
    b2_ui_apply_pause(); // validates an existing hold; never recaptures it
}

static void b2_interrupt(const char *reason) { b2_interrupt_kind582(reason, true); }

static bool b2_manual_interruption582(bool epoch_changed)
{
    return epoch_changed || !B2XR_Focused582() ||
        (!B2_ControllerFocusMode556() && cls.active != ACT_ACTIVATED);
}

void B2_ProcessInterruptions(void)
{
    if (b2_menu_transition && (!B2_InputFocused556() ||
        b2_menu_interruption!=B2XR_InterruptionEpoch() || !B2XR_InteractionReady())) b2_menu_transition=false;
    if (B2_UISurfaceActive()) return;
    if (!B2_KeepFramePump()) return;
    B2XR_PollEvents();
    if (B2XR_StopRequested()) { b2_stop_at(__func__, __LINE__); return; }
    b2_realign_begin391();
    if(b2_client.stopped)return;
    if (b2_client.world_waiting || b2_client.world_rebinding) return;
    uint32_t epoch = B2XR_InterruptionEpoch();
    bool interrupted = epoch != b2_client.ui_xr_epoch;
    b2_client.ui_xr_epoch = epoch;
    if (!B2_InputFocused556() || !B2XR_InteractionReady() || interrupted) {
        bool manual = b2_manual_interruption582(interrupted);
        b2_interrupt_kind582(manual ? "focus-session-or-reference" : "tracking-unavailable", manual);
    }
}

static bool b2_auto_resume582(bool ready, uint32_t now)
{
    b2rs_head_t head;
    b2c_snapshot_t sample = {0};
    if (!ready || b2_client.ui_manual_resume582 || b2_client.ui_manual_pause ||
        cls.key_dest != KEY_GAME || b2_client.stopped || !b2_client.walk_ready ||
        b2_client.world_waiting || b2_client.world_rebinding || b2_client.headset_waiting ||
        b2_client.recenter_pending || b2_client.handoff_position_pending || B2_RealignActive391() ||
        b2_client.combat_state.dead || cls.state != ca_active || !cl.frame.valid ||
        cl.frame.ps.pmove.pm_type != PM_NORMAL || cl.frame.ps.stats[STAT_HEALTH] <= 0 ||
        !B2XR_ReadHeadInput(&head) || !head.valid || !head.serial ||
        (uint32_t)(now-head.captured_ms) > B2C_MAX_AGE_MS) goto reset;
    for (int key=0; key<256; ++key) if (Key_IsDown(key)) goto reset;
    if (b2_controller.requested) {
        if (!b2_controller_enabled() || !B2XR_ReadControllerInput(&sample)) goto reset;
        now = Sys_Milliseconds(); // XR action reads can cross a clock tick.
        if (!b2c_sample_valid(&sample,now) || !b2c_neutral(&sample) ||
            sample.xr_generation != B2XR_InputGeneration() || sample.pose_time != (int64_t)head.serial)
            goto reset;
        for (unsigned h=0; h<2; ++h)
            if (!sample.hands[h].aim_tracked556 || !b2c_aim_usable556(&sample,h,now)) goto reset;
    }
    if ((uint32_t)(now-head.captured_ms) > B2C_MAX_AGE_MS) goto reset;
    if (b2_client.ui_good_frames582 &&
        (head.generation != b2_client.ui_good_input582 || sample.generation != b2_client.ui_good_actions582 ||
         head.serial < b2_client.ui_good_serial582 ||
         (uint32_t)(head.captured_ms-b2_client.ui_good_capture582) > B2C_MAX_AGE_MS))
        b2_client.ui_good_frames582 = 0;
    if (b2_client.ui_good_frames582 && (head.serial == b2_client.ui_good_serial582 ||
        head.captured_ms == b2_client.ui_good_capture582)) return false;
    b2_client.ui_good_serial582 = head.serial;
    b2_client.ui_good_capture582 = head.captured_ms;
    b2_client.ui_good_input582 = head.generation;
    b2_client.ui_good_actions582 = sample.generation;
    if (b2_client.ui_good_frames582 < 3) ++b2_client.ui_good_frames582;
    return b2_client.ui_good_frames582 == 3;
reset:
    b2_client.ui_good_frames582 = 0;
    return false;
}

static void b2_finish_interruption256(bool accepted_pair,bool ended,bool renderer_committed,bool history_committed)
{
    if (B2_KeepFramePump() && b2_client.ui_interrupted && !b2_client.headset_waiting && !B2_RealignActive391()) {
        bool ready = accepted_pair && ended && renderer_committed && history_committed &&
            B2_InputFocused556() && B2XR_InteractionReady();
        if (ready && !b2_client.ui_recovery_ready && b2_client.ui_manual_resume582)
            Com_Printf("B2_INTERRUPTION ready; release controls, then press and release your bound resume control shown in the headset, or press Pause. Manual pause/menu stays unchanged.\n");
        b2_client.ui_recovery_ready = ready;
        bool automatic = b2_auto_resume582(ready,Sys_Milliseconds());
        bool resume = automatic || (ready && b2_client.ui_resume_request && b2_resume_neutral256(Sys_Milliseconds()));
        b2_client.ui_resume_request = false; // never queue a future auto-resume
        bool controller_request=!automatic && b2_resume.pending;
        b2_resume.pending=false;
        bool accepted=resume && (!controller_request || b2_controller_resume_commit_ready(Sys_Milliseconds())) && b2_ui_validate_hold();
        if (controller_request && (!accepted || Cvar_VariableInteger("developer") > 0))
            Com_Printf("B2_CONTROLLER_RESUME completion=%s; manual/menu intent unchanged.\n",accepted?"accepted":"refused");
        if (accepted) {
            b2_client.ui_interrupted = false;
            b2_client.ui_good_frames582 = 0;
            b2_client.ui_manual_resume582 = false;
            if (automatic) Com_Printf("VR tracking recovered; controls released, resuming.\n");
            b2_clear_walk_input(); CL_B2ReleaseButtons();
            b2_client.walk_release_required = true;
            b2_ui_apply_pause();
            if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_INTERRUPTION returned; manual_pause=%d destination=%d release_required=1\n", b2_client.ui_manual_pause,cls.key_dest);
        }
    }
}

static void fresh(unsigned delta){now_ms+=delta;head.serial++;head.captured_ms=now_ms;head.generation=1;head.valid=true;
memset(&b2_controller_sample,0,sizeof(b2_controller_sample));b2_controller_sample.ready=true;b2_controller_sample.serial=head.serial;b2_controller_sample.generation=1;b2_controller_sample.xr_generation=1;b2_controller_sample.pose_time=head.serial;b2_controller_sample.capture_ms=b2_controller_sample.head_ms=now_ms;b2_controller_sample.head_orientation[3]=1;
for(int h=0;h<2;h++){b2_controller_sample.hands[h].aim_active556=true;b2_controller_sample.hands[h].aim_valid=true;b2_controller_sample.hands[h].aim_tracked556=true;b2_controller_sample.hands[h].aim_orientation[3]=1;}
b2_controller.serial=b2_controller_sample.serial;b2_controller.generation=1;}
static void init(void){memset(&b2_client,0,sizeof(b2_client));memset(&b2_controller,0,sizeof(b2_controller));memset(&head,0,sizeof(head));memset(&xr,0,sizeof(xr));xr.running=xr.focused=xr.tracking=true;cls.active=ACT_ACTIVATED;cls.state=ca_active;cls.key_dest=KEY_GAME;cl.frame.valid=true;cl.frame.ps.pmove.pm_type=PM_NORMAL;cl.frame.ps.stats[0]=100;b2_client.walk_ready=true;b2_controller.requested=true;now_ms=0;epoch=0;realign=key_down=false;hold_valid=read_ok=head_ok=true;read_advance=0;mode=1;fresh(20);}
static void finish(void){b2_finish_interruption256(true,true,true,true);}
static void stable(int n){for(int i=0;i<n;i++){fresh(20);finish();}}
int main(void){init();b2_interrupt_kind582("tracking",false);stable(2);assert(b2_client.ui_interrupted&&b2_client.ui_good_frames582==2);finish();assert(b2_client.ui_good_frames582==2);stable(1);assert(!b2_client.ui_interrupted&&b2_client.walk_release_required&&!b2_controller.intent.attack);
init();b2_interrupt_kind582("tracking",false);stable(1);b2_interrupt("focus");b2_interrupt_kind582("timeout",false);stable(5);assert(b2_client.ui_interrupted&&b2_client.ui_manual_resume582);b2_client.ui_resume_request=true;finish();assert(!b2_client.ui_interrupted);
init();xr.tracking=false;B2_ProcessInterruptions();assert(b2_client.ui_interrupted&&!b2_client.ui_manual_resume582);xr.focused=false;B2_ProcessInterruptions();assert(b2_client.ui_manual_resume582);
init();mode=0;cls.active=ACT_MINIMIZED;B2_ProcessInterruptions();assert(b2_client.ui_manual_resume582);
init();epoch=1;B2_ProcessInterruptions();assert(b2_client.ui_manual_resume582);
init();b2_interrupt_kind582("timeout",false);fresh(20);finish();fresh(101);finish();assert(b2_client.ui_good_frames582==1);fresh(100);finish();assert(b2_client.ui_good_frames582==2);fresh(100);finish();assert(!b2_client.ui_interrupted);
init();b2_interrupt_kind582("age",false);stable(2);b2_finish_interruption256(true,false,true,true);assert(!b2_client.ui_good_frames582);stable(2);head.captured_ms=now_ms-101;finish();assert(!b2_client.ui_good_frames582);
init();b2_interrupt_kind582("age",false);fresh(20);read_advance=1;finish();assert(b2_client.ui_good_frames582==1);read_advance=101;fresh(20);finish();assert(!b2_client.ui_good_frames582);read_advance=0;
init();b2_interrupt_kind582("tracking",false);stable(2);fresh(20);b2_controller_sample.hands[0].trigger=.6f;finish();assert(!b2_client.ui_good_frames582);fresh(20);b2_controller_sample.hands[0].aim_tracked556=false;finish();assert(!b2_client.ui_good_frames582);
bool *gates[]={&b2_client.ui_manual_pause,&b2_client.combat_state.dead,&b2_client.recenter_pending,&b2_client.handoff_position_pending,&b2_client.world_waiting,&b2_client.world_rebinding,&b2_client.headset_waiting,&realign,&key_down};
for(int i=0;i<sizeof(gates)/sizeof(gates[0]);i++){init();b2_interrupt_kind582("tracking",false);*gates[i]=true;stable(4);assert(b2_client.ui_interrupted&&!b2_client.ui_good_frames582&&*gates[i]);}
init();b2_interrupt_kind582("tracking",false);cls.key_dest=KEY_MENU;stable(4);assert(b2_client.ui_interrupted&&cls.key_dest==KEY_MENU);cls.key_dest=KEY_GAME;stable(3);assert(!b2_client.ui_interrupted);
init();b2_controller_sample.head_ms=now_ms-100;assert(b2c_sample_valid(&b2_controller_sample,now_ms));b2_controller_sample.head_ms--;assert(!b2c_sample_valid(&b2_controller_sample,now_ms));
puts("actual interruption/process/finish gates: transient three fresh frames, duplicate/gap/stale refusal, manual escalation, focus classification, neutral/lifecycle/manual/menu protection pass");return 0;}
