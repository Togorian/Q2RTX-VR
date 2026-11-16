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
typedef int b2c_diagnostic_t;
static int logs,terminal;
#define Com_Printf(...) (++logs)
static void B2XR_RequestStop(void){terminal++;}
static void b2_xr_note_failure555(XrResult result,const char *operation){(void)result;(void)operation;}
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
int main(void){b2c_native_t a=setup();float pulse[2]={.5f,0},zero[2]={0,0};uint32_t duration[2]={20,0};
assert(!b2ha_prepare(&a));assert(!a.haptic_failed);assert(b2ha_prepare(&a));
for(int i=0;i<100;i++)b2ha_deliver(&a,zero,duration);assert(!stops&&!applies);
b2ha_deliver(&a,pulse,duration);assert(applies==1&&a.haptic_active[0]);b2ha_deliver(&a,zero,duration);assert(stops==1&&!a.haptic_active[0]);
sync_result=XR_SESSION_NOT_FOCUSED;assert(!b2ha_prepare(&a));assert(!a.haptic_failed&&!a.haptic_errors&&!a.haptic_focused582&&a.haptic_rearm582);
int before=syncs;b2ha_deliver(&a,pulse,duration);assert(applies==1);assert(!b2ha_prepare(&a));assert(syncs==before);
sync_result=XR_SUCCESS;a.haptic_focused582=true;assert(!b2ha_prepare(&a));assert(b2ha_prepare(&a));b2ha_deliver(&a,pulse,duration);assert(applies==2);
apply_result=XR_SESSION_NOT_FOCUSED;b2ha_deliver(&a,pulse,duration);assert(!a.haptic_failed&&!a.haptic_errors);
a=setup();apply_result=XR_SUCCESS;sync_result=XR_ERROR_RUNTIME_FAILURE;assert(!b2ha_prepare(&a));assert(a.haptic_failed&&a.haptic_errors==4);before=syncs;for(int i=0;i<100;i++)assert(!b2ha_prepare(&a));assert(syncs==before);
a=setup();sync_result=XR_SESSION_LOSS_PENDING;assert(!b2ha_prepare(&a));assert(a.haptic_failed&&terminal==1);
puts("actual haptics: inactive no-op, transient no-focus, safe rearm, no delayed replay, latched real error and session loss pass");return 0;}
