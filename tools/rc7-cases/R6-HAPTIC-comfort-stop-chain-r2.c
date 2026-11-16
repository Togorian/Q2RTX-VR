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
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#define XR_NO_PROTOTYPES
#include <openxr/openxr.h>
#include "../../source/current/runtime-recovery555/source/b2_runtime555.h"
static char b2_failure_message555[256];
static void Q_strlcpy(char *dst,const char *src,size_t n){snprintf(dst,n,"%s",src);}
static void b2_xr_note_failure555(XrResult result,const char *operation){(void)result;(void)operation;}
typedef int b2c_diagnostic_t;
static int logs,terminal;
#define Com_Printf(...) (++logs)
static void B2XR_RequestStop(void);
#define B2C_XR_FUNCTIONS(X) \
 X(xrStringToPath) X(xrCreateActionSet) X(xrDestroyActionSet) X(xrCreateAction) \
 X(xrSuggestInteractionProfileBindings) X(xrCreateActionSpace) X(xrDestroySpace) \
 X(xrAttachSessionActionSets) X(xrSyncActions) X(xrGetActionStateBoolean) \
 X(xrGetActionStateFloat) X(xrGetActionStateVector2f) X(xrGetActionStatePose) \
 X(xrLocateSpace) X(xrGetCurrentInteractionProfile)
enum { B2CA_AIM, B2CA_GRIP, B2CA_TRIGGER, B2CA_STICK, B2CA_PRIMARY,
    B2CA_SECONDARY, B2CA_MENU, B2CA_CLICK, B2CA_SQUEEZE, B2CA_COUNT };
typedef struct {
#define B2C_DECLARE(n) PFN_##n n;
    B2C_XR_FUNCTIONS(B2C_DECLARE)
#undef B2C_DECLARE
    XrInstance instance;
    XrSession session;
    XrActionSet set;
    XrAction actions[B2CA_COUNT];
    XrAction haptic;
    PFN_xrApplyHapticFeedback xrApplyHapticFeedback;
    PFN_xrStopHapticFeedback xrStopHapticFeedback;
    XrPath haptic_previous[2];
    bool input_enabled, haptic_suggested[4], haptic_available[2], haptic_active[2], haptic_failed;
    unsigned haptic_errors, haptic_reported, haptic_result_records582, haptic_error_records594;
    bool haptic_focused582, haptic_rearm582;
    uint64_t haptic_apply_attempts[2], haptic_apply_success[2];
    uint64_t haptic_stop_attempts[2], haptic_stop_success[2];
    XrResult haptic_last_result[2];
    XrSpace spaces[2][2];
    XrPath hands[2], profiles[4], current[2];
    bool menu_active316[2],hp_extension316;
    bool suggested[4], attached, cleanup_failed, exhausted;
    uint32_t generation;
    uint64_t serial;
    b2c_diagnostic_t diagnostic;
    unsigned armed556[2], profile_kind582[2]; // Zero unknown; otherwise profile index + 1.
    bool pose_seen556[2][2], pose_tracked556[2][2], sample_seen556;
    uint32_t pose_ms556[2][2], sample_ms556;
} b2c_native_t;

// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
// Optional output on the one B2 action set. Failures never enter renderer error handling.
// Expected focus/session transitions are temporary. Other failures disable
// optional output once; session/instance loss also retires the XR session.
static bool b2ha_result582(b2c_native_t *a, XrResult result, unsigned bit, const char *operation)
{
    if (result == XR_SUCCESS) return true;
    bool transient = result == XR_SESSION_NOT_FOCUSED || result == XR_ERROR_SESSION_NOT_RUNNING;
    // Sixteen general records plus the first genuine negative result for
    // each operation. This mask is not reset when focus rearms output.
    bool first_error=result<0 && !transient && !(a->haptic_error_records594&bit);
    if (first_error) a->haptic_error_records594|=bit;
    if (first_error || a->haptic_result_records582 < 16) {
        if (a->haptic_result_records582 < 16) ++a->haptic_result_records582;
        Com_Printf("B2_HAPTIC_RESULT operation=%s result=%d transient=%d general_record=%u/16 first_error=%d\n",
            operation, (int)result, transient, a->haptic_result_records582,first_error);
    }
    a->haptic_rearm582 = true;
    if (transient) {
        a->haptic_focused582 = false;
        memset(a->haptic_active, 0, sizeof(a->haptic_active));
        memset(a->haptic_available, 0, sizeof(a->haptic_available));
    } else {
        a->haptic_errors |= bit;
        a->haptic_failed = true;
        if (result == XR_SESSION_LOSS_PENDING || result == XR_ERROR_SESSION_LOST || result == XR_ERROR_INSTANCE_LOST) {
            b2_xr_note_failure555(result,operation);
            // RequestStop calls the client reset, which calls HapticStop again.
            // Retire uncertain output ownership before that re-entrant chain.
            memset(a->haptic_active, 0, sizeof(a->haptic_active));
            memset(a->haptic_available, 0, sizeof(a->haptic_available));
            a->haptic_focused582 = false;
            B2XR_RequestStop();
        }
    }
    return false;
}
static void b2ha_stop(b2c_native_t *a)
{
    for (unsigned hand=0;hand<2;hand++) {
        if (a->haptic_focused582 && a->haptic && a->attached && a->xrStopHapticFeedback && a->haptic_active[hand]) {
            XrHapticActionInfo info={.type=XR_TYPE_HAPTIC_ACTION_INFO,
                .action=a->haptic,.subactionPath=a->hands[hand]};
            a->haptic_active[hand]=false; // Retire before a re-entrant error path.
            XrResult result=a->xrStopHapticFeedback(a->session,&info);
            if (a->haptic_stop_attempts[hand]!=UINT64_MAX) a->haptic_stop_attempts[hand]++;
            if (result==XR_SUCCESS && a->haptic_stop_success[hand]!=UINT64_MAX) a->haptic_stop_success[hand]++;
            a->haptic_last_result[hand]=result;
            b2ha_result582(a,result,16u,"stop");
        }
        a->haptic_active[hand]=false;
    }
}
static void b2ha_create(b2c_native_t *a, PFN_xrGetInstanceProcAddr get)
{
    if (get(a->instance,"xrApplyHapticFeedback",(PFN_xrVoidFunction*)&a->xrApplyHapticFeedback)!=XR_SUCCESS ||
        get(a->instance,"xrStopHapticFeedback",(PFN_xrVoidFunction*)&a->xrStopHapticFeedback)!=XR_SUCCESS ||
        !a->xrApplyHapticFeedback || !a->xrStopHapticFeedback) {
        a->xrApplyHapticFeedback=NULL; a->xrStopHapticFeedback=NULL; return;
    }
    XrActionCreateInfo info={.type=XR_TYPE_ACTION_CREATE_INFO,
        .actionType=XR_ACTION_TYPE_VIBRATION_OUTPUT,.countSubactionPaths=2,.subactionPaths=a->hands};
    strcpy(info.actionName,"haptic"); strcpy(info.localizedActionName,"B2 haptic feedback");
    if (a->xrCreateAction(a->set,&info,&a->haptic)!=XR_SUCCESS) {
        // The set retains any partially returned action until ordinary set cleanup.
        a->haptic=NULL; a->haptic_errors|=1u;
    }
}
static unsigned b2ha_bindings(b2c_native_t *a, XrActionSuggestedBinding *bindings, unsigned count)
{
    if (!a->haptic) return count;
    XrPath paths[2];
    if (a->xrStringToPath(a->instance,"/user/hand/left/output/haptic",&paths[0])!=XR_SUCCESS ||
        a->xrStringToPath(a->instance,"/user/hand/right/output/haptic",&paths[1])!=XR_SUCCESS) {
        a->haptic_errors|=2u; return count;
    }
    for (unsigned h=0;h<2;h++) bindings[count++]=(XrActionSuggestedBinding){a->haptic,paths[h]};
    return count;
}
static bool b2ha_prepare(b2c_native_t *a)
{
    if (!a->attached || !a->haptic || !a->xrApplyHapticFeedback ||
        !a->xrStopHapticFeedback || a->haptic_failed || !a->haptic_focused582) return false;
    XrActiveActionSet active={a->set,XR_NULL_PATH};
    XrActionsSyncInfo sync={.type=XR_TYPE_ACTIONS_SYNC_INFO,.countActiveActionSets=1,.activeActionSets=&active};
    XrResult result = a->xrSyncActions(a->session,&sync);
    if (!b2ha_result582(a,result,4u,"sync")) { b2ha_stop(a); return false; }
    bool changed=false,available=false;
    for (unsigned h=0;h<2;h++) {
        XrInteractionProfileState p={.type=XR_TYPE_INTERACTION_PROFILE_STATE};
        result = a->xrGetCurrentInteractionProfile(a->session,a->hands[h],&p);
        if (!b2ha_result582(a,result,8u,"profile")) { b2ha_stop(a); return false; }
        changed|=p.interactionProfile!=a->haptic_previous[h];
        a->haptic_previous[h]=p.interactionProfile;
        a->haptic_available[h]=false;
        for (unsigned n=0;n<4;n++) a->haptic_available[h]|=
            a->haptic_suggested[n] && p.interactionProfile==a->profiles[n];
        available|=a->haptic_available[h];
    }
    if (changed || !available || a->haptic_rearm582) {
        a->haptic_rearm582 = false;
        b2ha_stop(a); return false; // Caller clears queued pulses before rearming.
    }
    return true;
}
static void b2ha_deliver(b2c_native_t *a, const float amplitude[2],const uint32_t duration[2])
{
    if (!a->attached || !a->haptic || !a->xrApplyHapticFeedback ||
        !a->xrStopHapticFeedback || a->haptic_failed || !a->haptic_focused582) return;
    for (unsigned h=0;h<2;h++) {
        XrHapticActionInfo info={.type=XR_TYPE_HAPTIC_ACTION_INFO,
            .action=a->haptic,.subactionPath=a->hands[h]};
        float value=amplitude[h]; XrResult result;
        if (a->haptic_available[h] && isfinite(value) && value>.001f && value<=1.f && duration[h]>0 && duration[h]<=50) {
            XrHapticVibration vibration={.type=XR_TYPE_HAPTIC_VIBRATION,
                .duration=(XrDuration)duration[h]*1000000,.frequency=XR_FREQUENCY_UNSPECIFIED,.amplitude=value};
            result=a->xrApplyHapticFeedback(a->session,&info,(const XrHapticBaseHeader*)&vibration);
            if (a->haptic_apply_attempts[h]!=UINT64_MAX) a->haptic_apply_attempts[h]++;
            if (result==XR_SUCCESS && a->haptic_apply_success[h]!=UINT64_MAX) a->haptic_apply_success[h]++;
            a->haptic_last_result[h]=result;
            a->haptic_active[h]=true; // Also stop an uncertain failed submission.
        } else {
            if (!a->haptic_active[h]) continue; // No redundant inactive stop.
            result=a->xrStopHapticFeedback(a->session,&info);
            if (a->haptic_stop_attempts[h]!=UINT64_MAX) a->haptic_stop_attempts[h]++;
            if (result==XR_SUCCESS && a->haptic_stop_success[h]!=UINT64_MAX) a->haptic_stop_success[h]++;
            a->haptic_last_result[h]=result;
            a->haptic_active[h]=false;
        }
        if (!b2ha_result582(a,result,32u,"deliver")) { b2ha_stop(a); return; }
    }
}
static XrResult sync_result=XR_SUCCESS,apply_result=XR_SUCCESS,stop_result=XR_SUCCESS;
static int syncs,applies,stops;
static XrResult XRAPI_PTR syncfn(XrSession s,const XrActionsSyncInfo *i){syncs++;return sync_result;}
static XrResult XRAPI_PTR profilefn(XrSession s,XrPath h,XrInteractionProfileState *p){p->interactionProfile=10;return XR_SUCCESS;}
static XrResult XRAPI_PTR applyfn(XrSession s,const XrHapticActionInfo *i,const XrHapticBaseHeader *h){applies++;return apply_result;}
static XrResult XRAPI_PTR stopfn(XrSession s,const XrHapticActionInfo *i){stops++;return stop_result;}
static b2c_native_t setup(void){b2c_native_t a={0};a.attached=true;a.haptic=(XrAction)1;a.xrSyncActions=syncfn;a.xrGetCurrentInteractionProfile=profilefn;a.xrApplyHapticFeedback=applyfn;a.xrStopHapticFeedback=stopfn;a.haptic_focused582=true;a.profiles[0]=10;a.haptic_suggested[0]=true;return a;}
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_COMFORT_RELEASE308_H
#define B2_COMFORT_RELEASE308_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
typedef struct {
    bool valid,moving,fading;
    uint64_t frame;
    uint32_t now,released,owner[9];
    float strength,value;
} b2_comfort_release308_t;
static void B2Comfort_Reset308(b2_comfort_release308_t *s){memset(s,0,sizeof(*s));}
// Called outside the eye loop, with the existing logical frame and millisecond
// clock. Only ordinary locomotion release gets this tail; teleport is separate.
static float B2Comfort_Release308(b2_comfort_release308_t *s,uint64_t frame,
    uint32_t now,const uint32_t owner[9],bool valid,bool moving,float strength)
{
    if(!valid || !isfinite(strength) || strength<=0){B2Comfort_Reset308(s);return 0;}
    strength=fminf(1.f,strength);
    if(s->valid && (memcmp(s->owner,owner,sizeof(s->owner)) ||
       s->strength!=strength || (uint32_t)(now-s->now)>100 || frame<s->frame))
        B2Comfort_Reset308(s);
    if(s->valid && frame==s->frame)return s->value;
    if(moving){s->moving=true;s->fading=false;s->value=strength;}
    else {
        if(s->moving){s->moving=false;s->fading=true;s->released=now;}
        uint32_t age=now-s->released;
        if(s->fading && age<150){
            float t=(float)age/150.f;
            s->value=strength*(1.f-t*t*(3.f-2.f*t));
        }else {s->fading=false;s->value=0;}
    }
    s->valid=true;s->frame=frame;s->now=now;s->strength=strength;
    memcpy(s->owner,owner,sizeof(s->owner));return s->value;
}
#endif

// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// B2 implementation of the user-supplied event/tuning specification.
// Private deterministic state; no gameplay, renderer or runtime dependency.
#ifndef B2_HAPTIC_STATE_H
#define B2_HAPTIC_STATE_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
enum { B2H_BLASTER, B2H_SHOTGUN, B2H_SSHOTGUN, B2H_MACHINEGUN,
    B2H_CHAINGUN, B2H_GRENADE_LAUNCHER, B2H_ROCKET, B2H_HYPERBLASTER,
    B2H_RAILGUN, B2H_PROFILE_COUNT };
enum { B2H_MODEL_OTHER, B2H_MODEL_GRENADE, B2H_MODEL_BFG };
typedef struct { uint32_t start, duration; float amplitude; bool fade; } b2h_envelope_t;
typedef struct {
    bool active, weapons, bfg, damage, armour, controller;
    unsigned hand;
} b2h_config_t;
typedef struct {
    bool valid;
    int number, server, player, health, armour, flashes, model, gunframe, selected;
    unsigned world;
} b2h_snapshot_t;
typedef struct {
    b2h_config_t config;
    b2h_snapshot_t previous;
    bool baseline, weapon_baseline, charge;
    b2h_envelope_t health, armour, ordinary[2], fire[2];
} b2h_state_t;
// Diagnostic total survives state cancellation; never affects an envelope.
static uint64_t b2h_created;
static void b2h_reset(b2h_state_t *s) { memset(s,0,sizeof(*s)); }
static void b2h_weapon_reset(b2h_state_t *s) {
    memset(s->ordinary,0,sizeof(s->ordinary)); memset(s->fire,0,sizeof(s->fire));
    s->charge=false; s->weapon_baseline=false;
}
static void b2h_configure(b2h_state_t *s, b2h_config_t c) {
    if (!c.active || c.hand>1) { b2h_reset(s); return; }
    if (!s->config.active || c.hand!=s->config.hand ||
        c.controller!=s->config.controller) b2h_weapon_reset(s);
    if (!c.controller || !c.weapons) {
        memset(s->ordinary,0,sizeof(s->ordinary));
    }
    if (!c.controller || !c.bfg) {
        memset(s->fire,0,sizeof(s->fire)); s->charge=false;
    }
    if (c.weapons!=s->config.weapons || c.bfg!=s->config.bfg) s->weapon_baseline=false;
    if (!c.damage) memset(&s->health,0,sizeof(s->health));
    if (!c.armour) memset(&s->armour,0,sizeof(s->armour));
    s->config=c;
}
static unsigned b2h_hand(int hand, float dominant) {
    if (hand==0) return 1;
    if (hand==1) return 0;
    return isfinite(dominant) && dominant<.5f ? 0u : 1u;
}
static void b2h_start(b2h_envelope_t *e, uint32_t now, uint32_t ms, float amplitude, bool fade) {
    if (!ms || ms>3000 || !isfinite(amplitude) || amplitude<=0 || amplitude>1) {
        memset(e,0,sizeof(*e)); return;
    }
    if (b2h_created!=UINT64_MAX) b2h_created++;
    *e=(b2h_envelope_t){now,ms,amplitude,fade};
}
static float b2h_value(b2h_envelope_t *e, uint32_t now) {
    uint32_t elapsed=now-e->start;
    if (!e->duration || elapsed>=e->duration || !isfinite(e->amplitude) ||
        e->amplitude<0 || e->amplitude>1) { memset(e,0,sizeof(*e)); return 0; }
    return e->amplitude*(e->fade ? 1.f-(float)elapsed/(float)e->duration : 1.f);
}
static bool b2h_profile(unsigned profile, uint32_t *ms, float *amplitude) {
    static const uint32_t durations[B2H_PROFILE_COUNT]={60,110,160,80,120,120,140,90,90};
    static const float amplitudes[B2H_PROFILE_COUNT]={.05f,.15f,.35f,.08f,.05f,.22f,.30f,.06f,.35f};
    if (profile>=B2H_PROFILE_COUNT) return false;
    *ms=durations[profile]; *amplitude=amplitudes[profile]; return true;
}
static void b2h_muzzle(b2h_state_t *s, unsigned profile, uint32_t now) {
    uint32_t ms; float amplitude;
    if (!s->config.active || !s->config.controller || !s->config.weapons ||
        !b2h_profile(profile,&ms,&amplitude)) return;
    b2h_start(&s->ordinary[s->config.hand],now,ms,amplitude,false);
}
static float b2h_capped(float base, int loss, float cap) {
    float value=base+(float)loss/200.f; return value<cap ? value : cap;
}
static void b2h_snapshot(b2h_state_t *s, const b2h_snapshot_t *p, uint32_t now) {
    if (!s->config.active) return;
    if (!p->valid || !p->world || p->number<0 || p->player<0 ||
        p->health<-32768 || p->health>32767 || p->armour<0 || p->armour>32767 ||
        p->selected<0 || p->model<0 || p->model>B2H_MODEL_BFG || p->gunframe<0 || p->gunframe>255) {
        b2h_config_t c=s->config; b2h_reset(s); s->config=c; return;
    }
    bool epoch=!s->baseline || p->server!=s->previous.server ||
        p->world!=s->previous.world || p->player!=s->previous.player;
    if (!epoch && p->number<=s->previous.number) {
        if (p->number<s->previous.number) {
            b2h_weapon_reset(s);
            memset(&s->health,0,sizeof(s->health)); memset(&s->armour,0,sizeof(s->armour));
        }
        return;
    }
    bool respawn=!epoch && s->previous.health<=0 && p->health>0;
    if (epoch || respawn) {
        b2h_config_t c=s->config; b2h_reset(s); s->config=c;
    } else {
        int loss=s->previous.health-p->health;
        // STAT_FLASHES blood bit distinguishes damage from passive overheal decay.
        // Lethal damage can return before damage_blood is accumulated: keep death feedback.
        if (s->config.damage && s->previous.health>0 && loss>0 &&
            (p->health<=0 || (p->flashes&1)))
            b2h_start(&s->health,now,p->health<=0?3000u:250u,
                b2h_capped(.20f,p->health<=0?s->previous.health:loss,.70f),p->health<=0);
        loss=s->previous.armour-p->armour;
        if (s->config.armour && s->previous.health>0 && loss>0 && (p->flashes&2))
            b2h_start(&s->armour,now,100,b2h_capped(.10f,loss,.60f),false);
    }
    bool same=s->weapon_baseline && p->model==s->previous.model && p->selected==s->previous.selected;
    if (!same || p->health<=0) b2h_weapon_reset(s);
    if (s->config.controller && p->health>0) {
        if (s->config.weapons && same && p->model==B2H_MODEL_GRENADE) {
            int old=s->previous.gunframe;
            if (old<6 && p->gunframe>=6 && p->gunframe<=11)
                b2h_start(&s->ordinary[s->config.hand],now,50,.06f,false);
            else if (old>=6 && old<=12 && p->gunframe>=13)
                b2h_start(&s->ordinary[s->config.hand],now,80,.18f,false);
        }
        if (s->config.bfg && p->model==B2H_MODEL_BFG) {
            s->charge=p->gunframe>=9 && p->gunframe<=16;
            if (same && s->previous.gunframe>=9 && s->previous.gunframe<=16 && p->gunframe>=17)
                b2h_start(&s->fire[s->config.hand],now,180,1.f,false);
        } else s->charge=false;
        s->weapon_baseline=true;
    }
    s->previous=*p; s->baseline=true;
}
static void b2h_compose(b2h_state_t *s, uint32_t now, float out[2]) {
    out[0]=out[1]=0;
    if (!s->config.active) return;
    float health=b2h_value(&s->health,now),armour=b2h_value(&s->armour,now);
    for (unsigned hand=0;hand<2;hand++) {
        float amplitude=health>armour?health:armour;
        float ordinary=b2h_value(&s->ordinary[hand],now),fire=b2h_value(&s->fire[hand],now);
        if (ordinary>amplitude) amplitude=ordinary;
        if (fire>amplitude) amplitude=fire;
        if (s->charge && s->config.controller && s->config.bfg && hand==s->config.hand && amplitude<.12f)
            amplitude=.12f;
        out[hand]=isfinite(amplitude) && amplitude>0 && amplitude<=1 ? amplitude : 0;
    }
}
#endif


static struct {int64_t display_time,haptic_time;bool running,focused,failed,exit_requested,loss_pending;int loader;XrInstance instance;XrSession session;PFN_xrRequestExitSession xrRequestExitSession;} xr;
static b2c_native_t b2_actions;
static b2_comfort_release308_t b2_comfort_release308;
static b2h_state_t b2_haptics;
static b2h_envelope_t b2_ui_pulse304[2],b2_pickup_pulse304[2];
static bool b2_ui_ready304,b2_ui_rate304,b2_pickup_baseline304,b2_ui_pending308[2],b2_pickup_pending308[2];
static int mode=1,outputs,exit_calls;static bool enabled=true;
static bool B2XR_InteractionReady(void){return xr.running&&xr.focused&&!xr.failed&&!xr.exit_requested&&!xr.loss_pending;}
static uint32_t Sys_Milliseconds(void){return (uint32_t)xr.display_time;}
static int Cvar_VariableInteger(const char *s){return !strcmp(s,"b2_input_mode")?mode:0;}
static bool B2H_TakeStop316(void){return false;}
static bool B2H_Enabled316(void){return enabled;}
static void B2H_Output308(uint32_t now,float a[2],uint32_t d[2]){outputs++;a[0]=b2h_value(&b2_haptics.health,now);a[1]=0;d[0]=20;d[1]=0;}
static void B2XR_InvalidateHistory(const char *s){}
static bool b2_xr_result(XrResult r,const char *s){return r==XR_SUCCESS;}
static XrResult XRAPI_PTR exitfn(XrSession s){exit_calls++;assert(exit_calls<3);return XR_SUCCESS;}
#define Com_WPrintf(...) (++logs)
static void b2_feedback_reset304(void)
{
 memset(b2_ui_pulse304,0,sizeof(b2_ui_pulse304));memset(b2_pickup_pulse304,0,sizeof(b2_pickup_pulse304));
 b2_ui_ready304=b2_ui_rate304=b2_pickup_baseline304=false;
 memset(b2_ui_pending308,0,sizeof(b2_ui_pending308));memset(b2_pickup_pending308,0,sizeof(b2_pickup_pending308));
}
void B2XR_HapticStop(void)
{
    b2_actions.haptic_focused582 = xr.running && xr.focused && !xr.failed &&
        !xr.exit_requested && !xr.loss_pending;
    b2ha_stop(&b2_actions);
}
void B2H_Discard582(void)
{
    b2_feedback_reset304();
    b2h_reset(&b2_haptics);
    B2XR_HapticStop();
}
void B2H_Reset(void)
{
    B2Comfort_Reset308(&b2_comfort_release308);
    B2H_Discard582();
}
void B2XR_RequestStop(void)
{
    // Preserve the initiating cause before cleanup or our own exit request can
    // produce a later session-loss event. A normal quit caches this text only:
    // it neither emits a diagnostic nor displays a stopped-session dialog.
    if ((xr.loader || xr.instance) && !xr.exit_requested && !b2_failure_message555[0])
        Q_strlcpy(b2_failure_message555,B2_InternalStopMessage642(),sizeof(b2_failure_message555));
    B2H_Reset();
    if ((!xr.loader && !xr.instance) || xr.exit_requested)
        return;
    xr.exit_requested = true;
    B2XR_InvalidateHistory("operator-stop");
    if (xr.running && xr.xrRequestExitSession)
        b2_xr_result(xr.xrRequestExitSession(xr.session), "request-exit-session");
}
static void b2_xr_haptic_update(void)
{
    if (xr.display_time<=0 || xr.display_time<=xr.haptic_time) return;
    xr.haptic_time=xr.display_time;
    b2_actions.haptic_focused582 = xr.running && xr.focused && !xr.failed &&
        !xr.exit_requested && !xr.loss_pending;
    if(B2H_TakeStop316())B2XR_HapticStop();
    if(!B2H_Enabled316() || Cvar_VariableInteger("b2_input_mode") != 1) {
        b2_actions.haptic_rearm582 = true; B2H_Discard582(); return;
    }
    if (!B2XR_InteractionReady()) {
        b2_actions.haptic_rearm582 = true; B2H_Discard582();
    } else if (!b2ha_prepare(&b2_actions)) {
        B2H_Discard582();
    } else {
        float amplitude[2];uint32_t duration[2];
        B2H_Output308(Sys_Milliseconds(),amplitude,duration);
        b2ha_deliver(&b2_actions,amplitude,duration);
        if (b2_actions.haptic_failed) B2H_Discard582();
    }
    // Bounded logical-update diagnostics, outside the eye loop.
    uint32_t now=Sys_Milliseconds();
    unsigned state=(b2_actions.haptic!=XR_NULL_HANDLE)|(b2_actions.attached<<1)|
        (b2_actions.haptic_available[0]<<2)|(b2_actions.haptic_available[1]<<3)|
        (b2_actions.haptic_active[0]<<4)|(b2_actions.haptic_active[1]<<5)|
        (b2_actions.haptic_failed<<6);
    static unsigned previous=UINT_MAX,records;static uint32_t logged;
    static uint64_t seen;
    uint64_t attempts=b2_actions.haptic_apply_attempts[0]+b2_actions.haptic_apply_attempts[1];
    if (records<64 && (!records || ((uint32_t)(now-logged)>=1000 &&
        (state!=previous || (seen!=attempts && (uint32_t)(now-logged)>=5000))))) {
        previous=state;logged=now;seen=attempts;records++;
        if ((Cvar_VariableInteger("developer") > 0) || b2_actions.haptic_failed) Com_Printf("B2_HAPTIC_OUTPUT state=%u apply=%llu,%llu accepted=%llu,%llu stop=%llu,%llu stopped=%llu,%llu last_result=%d,%d errors=%u record=%u/64\n",
            state,(unsigned long long)b2_actions.haptic_apply_attempts[0],
            (unsigned long long)b2_actions.haptic_apply_attempts[1],
            (unsigned long long)b2_actions.haptic_apply_success[0],
            (unsigned long long)b2_actions.haptic_apply_success[1],
            (unsigned long long)b2_actions.haptic_stop_attempts[0],
            (unsigned long long)b2_actions.haptic_stop_attempts[1],
            (unsigned long long)b2_actions.haptic_stop_success[0],
            (unsigned long long)b2_actions.haptic_stop_success[1],
            b2_actions.haptic_last_result[0],b2_actions.haptic_last_result[1],
            b2_actions.haptic_errors,records);
    }
    if (b2_actions.haptic_errors!=b2_actions.haptic_reported) {
        b2_actions.haptic_reported=b2_actions.haptic_errors;
        Com_WPrintf("B2_HAPTIC optional output error_mask=%u disabled=%d\n",
            b2_actions.haptic_errors,b2_actions.haptic_failed);
    }
}
static void tick(void){xr.display_time+=20;b2_xr_haptic_update();}
static void init(void){b2_actions=setup();memset(&xr,0,sizeof(xr));xr.loader=1;xr.instance=(XrInstance)1;xr.running=xr.focused=true;xr.xrRequestExitSession=exitfn;sync_result=apply_result=stop_result=XR_SUCCESS;syncs=applies=stops=exit_calls=0;mode=1;enabled=true;b2h_reset(&b2_haptics);B2Comfort_Reset308(&b2_comfort_release308);}
int main(void){uint32_t owner[9]={0};init();enabled=false;
assert(B2Comfort_Release308(&b2_comfort_release308,1,0,owner,true,true,1)==1);tick();
assert(B2Comfort_Release308(&b2_comfort_release308,2,20,owner,true,false,1)==1);tick();
float fade=B2Comfort_Release308(&b2_comfort_release308,3,40,owner,true,false,1);assert(fade>0&&fade<1);tick();
assert(B2Comfort_Release308(&b2_comfort_release308,4,100,owner,true,false,1)>0);tick();
assert(B2Comfort_Release308(&b2_comfort_release308,5,170,owner,true,false,1)==0);assert(!applies&&!syncs);
init();tick();tick();b2h_start(&b2_haptics.health,(uint32_t)xr.display_time,50,.5f,false);tick();assert(applies==1);
sync_result=XR_SESSION_NOT_FOCUSED;tick();assert(!b2_haptics.health.duration&&!b2_actions.haptic_failed);sync_result=XR_SUCCESS;tick();tick();b2h_start(&b2_haptics.health,(uint32_t)xr.display_time,50,.5f,false);tick();assert(applies==2);
XrResult losses[]={XR_SESSION_LOSS_PENDING,XR_ERROR_SESSION_LOST,XR_ERROR_INSTANCE_LOST};
for(int op=0;op<3;op++)for(int e=0;e<3;e++){init();tick();tick();b2_actions.haptic_active[0]=b2_actions.haptic_active[1]=true;b2_ui_pending308[0]=true;b2h_start(&b2_haptics.health,(uint32_t)xr.display_time,50,.5f,false);
 if(op==0){sync_result=losses[e];tick();}else if(op==1){apply_result=losses[e];tick();}else{stop_result=losses[e];B2XR_HapticStop();}
 assert(xr.exit_requested&&exit_calls==1&&b2_actions.haptic_failed);assert(!b2_actions.haptic_active[0]&&!b2_actions.haptic_active[1]);assert(!b2_haptics.health.duration&&!b2_ui_pending308[0]);assert(stops<=1);int s=stops;B2H_Reset();assert(stops==s&&exit_calls==1);
}
puts("actual update/reset/stop chain: comfort fade retained; focus recovery; all three loss statuses at sync/apply/stop finite and terminal");return 0;}
