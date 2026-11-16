// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian
// OpenXR controller input for Q2RTX VR.
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifdef B2C_REDUCER
// Original project deadzone with finite/range checks; vector normalisation is below.
static float b2c_axis(float value, float deadzone)
{
    if (!isfinite(value) || !isfinite(deadzone)) return 0;
    if (deadzone < 0) deadzone=0;
    if (deadzone > .9f) deadzone=.9f;
    if (value < -1) value=-1;
    if (value > 1) value=1;
    float magnitude=fabsf(value);
    if (magnitude<=deadzone) return 0;
    return (value<0 ? -1.f : 1.f)*((magnitude-deadzone)/(1.f-deadzone));
}

static void b2c_clear(b2c_state_t *s)
{
    memset(&s->intent,0,sizeof(s->intent));
    s->wait_neutral=true; s->snap_ready=s->menu_ready=s->attack=s->crouch=false;
    s->previous_primary=s->previous_secondary=s->previous_menu=s->previous_click=false;
    s->valid=false;
    s->turn_clock_valid=false; s->turn_ms=0;
}

static bool b2c_sample_valid(const b2c_snapshot_t *p, uint32_t now)
{
    if (!p->ready || !p->serial || !p->generation ||
        (uint32_t)(now-p->capture_ms)>B2C_MAX_AGE_MS ||
        (uint32_t)(now-p->head_ms)>B2C_MAX_AGE_MS) return false;
    for (unsigned i=0;i<2;i++) {
        const b2c_hand_t *h=&p->hands[i];
        if (!h->controls_active || !h->aim_valid || !isfinite(h->trigger) ||
            h->trigger<0 || h->trigger>1 || !h->squeeze_active || !isfinite(h->squeeze) ||
            h->squeeze<0 || h->squeeze>1) return false;
        for (unsigned j=0;j<2;j++) if (!isfinite(h->stick[j]) || fabsf(h->stick[j])>1) return false;
        float norm=0;
        for (unsigned j=0;j<3;j++) if (!isfinite(h->aim_position[j])) return false;
        for (unsigned j=0;j<4;j++) {
            if (!isfinite(h->aim_orientation[j])) return false;
            norm+=h->aim_orientation[j]*h->aim_orientation[j];
        }
        if (fabsf(norm-1.f)>.02f) return false;
    }
    return true;
}

// Called only on the existing valid-owner loss edge, after its decision.
static const char *b2c_sample_reason(const b2c_snapshot_t *p, uint32_t now)
{
    if (!p->ready) return "not-ready";
    if (!p->serial || !p->generation) return "identity";
    if ((uint32_t)(now-p->capture_ms)>B2C_MAX_AGE_MS) return "capture-age";
    if ((uint32_t)(now-p->head_ms)>B2C_MAX_AGE_MS) return "head-age";
    for (unsigned i=0;i<2;i++) {
        const b2c_hand_t *h=&p->hands[i];
        if (!h->controls_active || !h->squeeze_active) return "controls-inactive";
        if (!h->aim_valid) return "aim-invalid";
        if (!isfinite(h->trigger) || h->trigger<0 || h->trigger>1 ||
            !isfinite(h->squeeze) || h->squeeze<0 || h->squeeze>1) return "trigger-grip-range";
        for (unsigned j=0;j<2;j++)
            if (!isfinite(h->stick[j]) || fabsf(h->stick[j])>1) return "stick-range";
        float norm=0;
        for (unsigned j=0;j<3;j++) if (!isfinite(h->aim_position[j])) return "aim-position";
        for (unsigned j=0;j<4;j++) {
            if (!isfinite(h->aim_orientation[j])) return "aim-orientation";
            norm+=h->aim_orientation[j]*h->aim_orientation[j];
        }
        if (fabsf(norm-1.f)>.02f) return "aim-norm";
    }
    return "valid";
}

static bool b2c_neutral(const b2c_snapshot_t *p)
{
    for (unsigned i=0;i<2;i++) {
        const b2c_hand_t *h=&p->hands[i];
        if (fabsf(h->stick[0])>=.25f || fabsf(h->stick[1])>=.25f ||
            h->trigger>=.45f || h->squeeze>=.35f || h->primary || h->secondary || h->menu || h->click) return false;
    }
    return true;
}

// Returns loss of a previously valid controller owner; caller owns interruption.

// Terminal continuity refusal has a separate one-shot Quit gesture. It never
// produces gameplay/menu/resume intents: fresh neutral, then hold both face
// buttons on the configured dominant hand for 1.2 seconds.
typedef struct {
    uint64_t serial;
    uint32_t generation,xr,last_ms,held_ms;
    unsigned hand;
    bool configured,armed,holding;
} b2cq_state_t;
static bool b2c_quit_step(b2cq_state_t *s,const b2c_snapshot_t *p,
    uint32_t now,unsigned hand,bool eligible)
{
    if(!eligible || hand>1 || !b2c_sample_valid(p,now) || !p->xr_generation){
        memset(s,0,sizeof(*s));return false;
    }
    if(!s->configured || s->generation!=p->generation || s->xr!=p->xr_generation ||
       s->hand!=hand || p->serial<s->serial){
        memset(s,0,sizeof(*s));s->configured=true;s->generation=p->generation;
        s->xr=p->xr_generation;s->hand=hand;s->serial=p->serial;s->last_ms=p->capture_ms;
        return false;
    }
    if(p->serial==s->serial)return false;
    uint32_t dt=p->capture_ms-s->last_ms;s->serial=p->serial;s->last_ms=p->capture_ms;
    if(!dt || dt>B2C_MAX_AGE_MS){s->armed=s->holding=false;s->held_ms=0;return false;}
    if(b2c_neutral(p)){s->armed=true;s->holding=false;s->held_ms=0;return false;}
    bool chord=p->hands[hand].primary && p->hands[hand].secondary;
    b2c_snapshot_t other=*p;other.hands[hand].primary=other.hands[hand].secondary=false;
    if(!s->armed || !chord || !b2c_neutral(&other)){
        s->armed=s->holding=false;s->held_ms=0;return false;
    }
    if(!s->holding){s->holding=true;s->held_ms=0;return false;}
    s->held_ms+=dt;
    if(s->held_ms<1200)return false;
    s->armed=s->holding=false;s->held_ms=0;return true;
}

static bool b2c_step_mode(b2c_state_t *s, const b2c_snapshot_t *p, uint32_t now,
    unsigned destination, unsigned dominant, float speed, unsigned turn_mode)
{
    bool was_valid=s->valid;
    s->intent.events=s->intent.navigation=0; s->intent.yaw=0;
    if (!s->requested) { b2c_clear(s); return false; }
    if (!b2c_sample_valid(p,now) || dominant>1 || turn_mode>B2C_TURN_SMOOTH || !isfinite(speed)) {
        b2c_clear(s); return was_valid;
    }
    bool changed=s->generation && s->generation!=p->generation;
    if (s->configured && (s->dominant!=dominant || s->turn_mode!=turn_mode)) b2c_clear(s);
    s->configured=true; s->dominant=dominant; s->turn_mode=turn_mode;
    if (changed || s->destination!=destination || destination==B2C_BLOCKED) b2c_clear(s);
    s->generation=p->generation; s->destination=destination;
    if (changed) { s->serial=p->serial; return was_valid; }
    if (p->serial<s->serial) { b2c_clear(s); return was_valid; }
    if (p->serial==s->serial) return false;
    s->serial=p->serial; s->valid=true;
    memset(&s->intent,0,sizeof(s->intent));
    if (destination==B2C_BLOCKED) return false;
    if (s->wait_neutral) {
        if (b2c_neutral(p)) { s->wait_neutral=false; s->snap_ready=s->menu_ready=true; }
        return false;
    }
    const b2c_hand_t *d=&p->hands[dominant], *o=&p->hands[1-dominant];
    bool menu=p->hands[0].menu || p->hands[1].menu;
    bool back=(d->secondary && !s->previous_secondary) || (menu && !s->previous_menu);
    bool primary=d->primary && !s->previous_primary;
    bool click=d->click && !s->previous_click;
    s->previous_primary=d->primary; s->previous_secondary=d->secondary;
    s->previous_menu=menu; s->previous_click=d->click;
    if (destination==B2C_MENU || destination==B2C_CONSOLE) {
        if (back) s->intent.events|=B2C_BACK;
        if (destination==B2C_CONSOLE) return false;
        if (primary) s->intent.events|=B2C_CONFIRM;
        float x=d->stick[0], y=d->stick[1];
        if (s->menu_ready && (fabsf(x)>.65f || fabsf(y)>.65f)) {
            s->intent.navigation=fabsf(x)>fabsf(y) ? (x>0?B2C_RIGHT:B2C_LEFT) : (y>0?B2C_UP:B2C_DOWN);
            s->menu_ready=false;
        } else if (fabsf(x)<.35f && fabsf(y)<.35f) s->menu_ready=true;
        return false;
    }
    if (back) { s->intent.events=B2C_OPEN_MENU; return false; }
    // A separate destination reuses the normal release/freshness/hand gates,
    // but intermission permits only deliberate fire (and the menu above).
    if (destination==B2C_INTERMISSION) {
        if (d->trigger>.55f) s->attack=true;
        else if (d->trigger<.45f) s->attack=false;
        s->intent.attack=s->attack;
        return false;
    }
    if (click) { s->intent.events=B2C_RECENTRE; return false; }
    float x=b2c_axis(o->stick[0],.25f), y=b2c_axis(o->stick[1],.25f);
    float length=sqrtf(x*x+y*y);
    if (length>1) { x/=length; y/=length; }
    if (speed<0) speed=0;
    if (speed>400) speed=400;
    s->intent.forward=(short)(y*speed); s->intent.side=(short)(x*speed);
    if (o->squeeze>.65f) s->crouch=true;
    else if (o->squeeze<.35f) s->crouch=false;
    s->intent.up=200*((int)p->hands[1].primary-(int)s->crouch);
    if (d->trigger>.55f) s->attack=true;
    else if (d->trigger<.45f) s->attack=false;
    s->intent.attack=s->attack;
    if (turn_mode==B2C_TURN_SMOOTH) {
        if (!s->turn_clock_valid) {
            s->turn_clock_valid=true; s->turn_ms=now;
        } else {
            uint32_t elapsed=now-s->turn_ms;
            s->turn_ms=now;
            // Never convert an inactive interval or stall into a catch-up turn.
            if (elapsed>50) return false; // Fresh movement/fire remain valid; discard only overdue yaw.
            s->intent.yaw=-180.f*b2c_axis(d->stick[0],.25f)*(elapsed*.001f);
        }
    } else if (s->snap_ready && fabsf(d->stick[0])>.65f) {
        s->intent.yaw=d->stick[0]>0 ? -30.f : 30.f; s->snap_ready=false;
    } else if (fabsf(d->stick[0])<.25f) s->snap_ready=true;
    return false;
}

// Preserve the accepted snap-only fixture/call contract for regression checks.
static bool b2c_step(b2c_state_t *s, const b2c_snapshot_t *p, uint32_t now,
    unsigned destination, unsigned dominant, float speed)
{
    return b2c_step_mode(s,p,now,destination,dominant,speed,B2C_TURN_SNAP);
}
#endif

#ifdef B2C_NATIVE
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

#include "../../../baseline/b2-menu-defaults320/source/refresh/b2_haptics_xr.inc"

static bool b2ca_destroy(b2c_native_t *a)
{
    b2ha_stop(a);
    bool ok=true;
    for (unsigned h=0;h<2;h++) for (unsigned p=0;p<2;p++) {
        if (a->spaces[h][p] && a->xrDestroySpace && a->xrDestroySpace(a->spaces[h][p])!=XR_SUCCESS) ok=false;
        a->spaces[h][p]=XR_NULL_HANDLE;
    }
    if (a->set && a->xrDestroyActionSet && a->xrDestroyActionSet(a->set)!=XR_SUCCESS) ok=false;
    memset(a,0,sizeof(*a));
    return ok;
}

// Touch exposes only left Menu, Index exposes system, HP exposes Menu on both hands.
static bool b2ca_has_menu316(int profile,unsigned hand)
{
    return hand<2 && (profile==2 || profile==3 || (profile==0 && hand==0));
}

static bool b2ca_setup_mode(b2c_native_t *a, bool enabled, bool input, XrInstance instance,
    XrSession session, PFN_xrGetInstanceProcAddr get)
{
    if (!enabled) return true; // No extra resolution or XR call when disabled.
    if (a->set || a->attached) return false;
    a->instance=instance; a->session=session; a->generation=1; a->input_enabled=input;
#define B2C_LOAD(n) if (get(instance,#n,(PFN_xrVoidFunction*)&a->n)!=XR_SUCCESS || !a->n) goto fail;
    if (input) { B2C_XR_FUNCTIONS(B2C_LOAD) }
    else {
        B2C_LOAD(xrStringToPath) B2C_LOAD(xrCreateActionSet) B2C_LOAD(xrDestroyActionSet)
        B2C_LOAD(xrCreateAction) B2C_LOAD(xrSuggestInteractionProfileBindings)
        B2C_LOAD(xrAttachSessionActionSets) B2C_LOAD(xrSyncActions) B2C_LOAD(xrGetCurrentInteractionProfile)
    }
#undef B2C_LOAD
    if (a->xrStringToPath(instance,"/user/hand/left",&a->hands[0])!=XR_SUCCESS ||
        a->xrStringToPath(instance,"/user/hand/right",&a->hands[1])!=XR_SUCCESS) goto fail;
    XrActionSetCreateInfo set={.type=XR_TYPE_ACTION_SET_CREATE_INFO};
    strcpy(set.actionSetName,"b2_controller"); strcpy(set.localizedActionSetName,"B2 controller input");
    if (a->xrCreateActionSet(instance,&set,&a->set)!=XR_SUCCESS) goto fail;
    const char *names[B2CA_COUNT]={"aim_pose","grip_pose","trigger","stick","primary","secondary","menu","stick_click","squeeze"};
    const XrActionType types[B2CA_COUNT]={XR_ACTION_TYPE_POSE_INPUT,XR_ACTION_TYPE_POSE_INPUT,
        XR_ACTION_TYPE_FLOAT_INPUT,XR_ACTION_TYPE_VECTOR2F_INPUT,XR_ACTION_TYPE_BOOLEAN_INPUT,
        XR_ACTION_TYPE_BOOLEAN_INPUT,XR_ACTION_TYPE_BOOLEAN_INPUT,XR_ACTION_TYPE_BOOLEAN_INPUT,XR_ACTION_TYPE_FLOAT_INPUT};
    for (unsigned i=0;input && i<B2CA_COUNT;i++) {
        XrActionCreateInfo info={.type=XR_TYPE_ACTION_CREATE_INFO,.actionType=types[i],.countSubactionPaths=2,.subactionPaths=a->hands};
        strcpy(info.actionName,names[i]); strcpy(info.localizedActionName,names[i]);
        if (a->xrCreateAction(a->set,&info,&a->actions[i])!=XR_SUCCESS) goto fail;
    }
    b2ha_create(a,get);
    if (!input && !a->haptic) goto fail;
    const char *profiles[4]={"/interaction_profiles/oculus/touch_controller","/interaction_profiles/valve/index_controller","/interaction_profiles/hp/mixed_reality_controller","/interaction_profiles/khr/simple_controller"};
    for (unsigned profile=0;profile<4;profile++) {
        if (profile==2 && !a->hp_extension316) continue;
        if (a->xrStringToPath(instance,profiles[profile],&a->profiles[profile])!=XR_SUCCESS) continue;
        XrActionSuggestedBinding bindings[2*B2CA_COUNT+2]; unsigned count=0;
        for (unsigned hand=0;hand<2;hand++) for (unsigned action=0;input && action<B2CA_COUNT;action++) {
            if (action==B2CA_MENU && !b2ca_has_menu316(profile,hand)) continue;
            // Core simple-controller offers poses, select and menu, not analogue controls.
            if (profile==3 && action!=B2CA_AIM && action!=B2CA_GRIP &&
                action!=B2CA_PRIMARY && action!=B2CA_MENU) continue;
            const char *component[]={"aim/pose","grip/pose","trigger/value","thumbstick",NULL,NULL,"menu/click","thumbstick/click","squeeze/value"};
            const char *part=component[action];
            if (action==B2CA_PRIMARY) part=profile==3?"select/click":(profile!=1 && !hand)?"x/click":"a/click";
            if (action==B2CA_SECONDARY) part=(profile!=1 && !hand)?"y/click":"b/click";
            char path[128]; snprintf(path,sizeof(path),"/user/hand/%s/input/%s",hand?"right":"left",part);
            XrPath binding=XR_NULL_PATH;
            if (a->xrStringToPath(instance,path,&binding)!=XR_SUCCESS) goto fail;
            bindings[count++]=(XrActionSuggestedBinding){a->actions[action],binding};
        }
        unsigned input_count=count;
        count=b2ha_bindings(a,bindings,count);
        XrInteractionProfileSuggestedBinding info={.type=XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING,
            .interactionProfile=a->profiles[profile],.countSuggestedBindings=count,.suggestedBindings=bindings};
        a->suggested[profile]=count && a->xrSuggestInteractionProfileBindings(instance,&info)==XR_SUCCESS;
        a->haptic_suggested[profile]=a->suggested[profile] && count>input_count;
        if (!a->suggested[profile] && input && count>input_count) {
            info.countSuggestedBindings=input_count;
            a->suggested[profile]=a->xrSuggestInteractionProfileBindings(instance,&info)==XR_SUCCESS;
            a->haptic_errors|=2u;
        }
    }
    if (!a->suggested[0] && !a->suggested[1] && !a->suggested[2] && !a->suggested[3]) goto fail;
    for (unsigned hand=0;hand<2;hand++) for (unsigned pose=0;input && pose<2;pose++) {
        XrActionSpaceCreateInfo info={.type=XR_TYPE_ACTION_SPACE_CREATE_INFO,.action=a->actions[pose],.subactionPath=a->hands[hand],.poseInActionSpace.orientation.w=1};
        if (a->xrCreateActionSpace(session,&info,&a->spaces[hand][pose])!=XR_SUCCESS) goto fail;
    }
    XrSessionActionSetsAttachInfo attach={.type=XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO,.countActionSets=1,.actionSets=&a->set};
    if (a->xrAttachSessionActionSets(session,&attach)!=XR_SUCCESS) goto fail;
    a->attached=true; return true;
fail:
    a->cleanup_failed=!b2ca_destroy(a); return false;
}

static bool b2ca_setup(b2c_native_t *a, bool enabled, XrInstance instance,
    XrSession session, PFN_xrGetInstanceProcAddr get)
{
    return b2ca_setup_mode(a,enabled,true,instance,session,get);
}

static void b2ca_invalidate556(b2c_native_t *a)
{
    memset(a->armed556,0,sizeof(a->armed556));
    memset(a->pose_seen556,0,sizeof(a->pose_seen556));
    a->sample_seen556=false;
    memset(a->profile_kind582,0,sizeof(a->profile_kind582));
}

static bool b2ca_pose556(b2c_native_t *a,unsigned hand,unsigned pose,XrSpace base,
    XrTime time,uint32_t now,float position[3],float orientation[4],
    bool *active,bool *tracked,uint32_t *tracked_ms)
{
    *active=*tracked=false;*tracked_ms=0;a->pose_tracked556[hand][pose]=false;
    XrActionStateGetInfo get={.type=XR_TYPE_ACTION_STATE_GET_INFO,.action=a->actions[pose],.subactionPath=a->hands[hand]};
    XrActionStatePose state={.type=XR_TYPE_ACTION_STATE_POSE};
    XrSpaceLocation location={.type=XR_TYPE_SPACE_LOCATION};
    if(a->xrGetActionStatePose(a->session,&get,&state)!=XR_SUCCESS || !state.isActive)goto invalid;
    *active=true;
    if(a->xrLocateSpace(a->spaces[hand][pose],base,time,&location)!=XR_SUCCESS)goto invalid;
    XrSpaceLocationFlags valid=XR_SPACE_LOCATION_POSITION_VALID_BIT|XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
    XrSpaceLocationFlags tracking=XR_SPACE_LOCATION_POSITION_TRACKED_BIT|XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
    if((location.locationFlags&valid)!=valid)goto invalid;
    float p[3]={location.pose.position.x,location.pose.position.y,location.pose.position.z};
    float q[4]={location.pose.orientation.x,location.pose.orientation.y,location.pose.orientation.z,location.pose.orientation.w};
    float norm=0;
    for(unsigned i=0;i<3;i++)if(!isfinite(p[i]))goto invalid;
    for(unsigned i=0;i<4;i++){if(!isfinite(q[i]))goto invalid;norm+=q[i]*q[i];}
    if(!isfinite(norm) || fabsf(norm-1.f)>.02f)goto invalid;
    *tracked=(location.locationFlags&tracking)==tracking;
    a->pose_tracked556[hand][pose]=*tracked;
    if(*tracked){a->pose_seen556[hand][pose]=true;a->pose_ms556[hand][pose]=now;}
    *tracked_ms=a->pose_ms556[hand][pose];
    if(!a->pose_seen556[hand][pose] || (uint32_t)(now-*tracked_ms)>B2C_POSE_GRACE_MS556)return false;
    memcpy(position,p,sizeof(p));memcpy(orientation,q,sizeof(q));return true;
invalid:
    a->pose_seen556[hand][pose]=false;
    return false;
}

// Keep the visual caller contract, with the same bounded current-pose policy.
static bool b2ca_pose(b2c_native_t *a,unsigned hand,unsigned pose,XrSpace base,
    XrTime time,float position[3],float orientation[4])
{
    bool active,tracked;uint32_t tracked_ms;
    return b2ca_pose556(a,hand,pose,base,time,Sys_Milliseconds(),position,orientation,
        &active,&tracked,&tracked_ms);
}

static void b2ca_controls556(b2c_native_t *a,unsigned hand,b2c_hand_t *h)
{
    unsigned neutral=0;
    if(fabsf(h->stick[0])<.25f && fabsf(h->stick[1])<.25f)neutral|=B2C_STICK556;
    if(h->trigger<.45f)neutral|=B2C_TRIGGER556;
    if(h->squeeze<.35f)neutral|=B2C_GRIP556;
    if(!h->primary)neutral|=B2C_PRIMARY556;
    if(!h->secondary)neutral|=B2C_SECONDARY556;
    if(!h->click)neutral|=B2C_CLICK556;
    if(!h->menu)neutral|=B2C_MENU556;
    a->armed556[hand]=(a->armed556[hand]|neutral)&h->active556;
    h->ready556=a->armed556[hand];
    h->blocked556=h->active556&~h->ready556;
    h->controls_active=h->active556!=0;
    if(!(h->ready556&B2C_STICK556))h->stick[0]=h->stick[1]=0;
    if(!(h->ready556&B2C_TRIGGER556))h->trigger=0;
    if(!(h->ready556&B2C_GRIP556))h->squeeze=0;
    if(!(h->ready556&B2C_PRIMARY556))h->primary=false;
    if(!(h->ready556&B2C_SECONDARY556))h->secondary=false;
    if(!(h->ready556&B2C_CLICK556))h->click=false;
    if(!(h->ready556&B2C_MENU556))h->menu=false;
}

// Bounded raw observations; profile names identify contracts, not hardware.
static void b2ca_menu_note320(const int kind[2],const bool active[2],
    const bool down[2],const b2c_snapshot_t *sample)
{
    static const char *const profiles[]={"Touch","Index","HP","Simple"};
    static int previous=-1;
    static bool held[2];
    static unsigned states,edges[2];
    int state=((kind[0]+1)*5+kind[1]+1)*4+active[0]*2+active[1];
    if(state!=previous && states<6){
        B2D_Record(0,"MENU_STATE L=%s active=%u R=%s active=%u",
            kind[0]>=0?profiles[kind[0]]:"unavailable",(unsigned)active[0],kind[1]>=0?profiles[kind[1]]:"unavailable",(unsigned)active[1]);
        states++;
    }
    for(unsigned h=0;h<2;h++){
        if(down[h]!=held[h] && edges[h]<6){
            B2D_Record(0,"MENU_EDGE serial=%llu hand=%u profile=%s raw=%u active=%u admitted=%u",
                (unsigned long long)sample->serial,h,kind[h]>=0?profiles[kind[h]]:"unavailable",(unsigned)down[h],
                (unsigned)active[h],(unsigned)sample->hands[h].menu);
            edges[h]++;
        }
        held[h]=down[h];
    }
    previous=state;
}
static bool b2ca_read(b2c_native_t *a, bool focused, XrSpace base, XrTime time,
    uint32_t now, uint32_t head_ms, bool head_valid, b2c_snapshot_t *out)
{
    memset(out,0,sizeof(*out));
    bool active_menu[2]={false,false},raw_active[2]={false,false},raw_down[2]={false,false};
    int kinds[2]={-1,-1};
    memset(a->profile_kind582,0,sizeof(a->profile_kind582));
    memset(a->menu_active316,0,sizeof(a->menu_active316));
    a->diagnostic=(b2c_diagnostic_t){!a->attached?"actions-detached":
        a->exhausted?"generation-exhausted":!focused?"not-focused":
        !head_valid?"head-invalid":time<=0?"head-time":"head-age",now,head_ms,-1};
    if (!a->input_enabled || !a->attached || a->exhausted || !focused || !head_valid || time<=0 ||
        (uint32_t)(now-head_ms)>B2C_MAX_AGE_MS) { b2ca_invalidate556(a);return false; }
    XrActiveActionSet active={a->set,XR_NULL_PATH};
    XrActionsSyncInfo sync={.type=XR_TYPE_ACTIONS_SYNC_INFO,.countActiveActionSets=1,.activeActionSets=&active};
    a->diagnostic.reason="action-sync";
    if (a->xrSyncActions(a->session,&sync)!=XR_SUCCESS) { b2ca_invalidate556(a);return false; }
    if(!a->sample_seen556 || (uint32_t)(now-a->sample_ms556)>B2C_MAX_AGE_MS)b2ca_invalidate556(a);
    a->sample_seen556=true;a->sample_ms556=now;
    b2c_snapshot_t p={0};
    a->diagnostic.reason="serial-exhausted";
    if (a->serial==UINT64_MAX) return false;
    p.serial=++a->serial; p.capture_ms=now; p.head_ms=head_ms; p.pose_time=time;
    for (unsigned h=0;h<2;h++) {
        XrInteractionProfileState profile={.type=XR_TYPE_INTERACTION_PROFILE_STATE};
        a->diagnostic.hand=(int)h;a->diagnostic.reason="profile-read";
        bool profile_ok=a->xrGetCurrentInteractionProfile(a->session,a->hands[h],&profile)==XR_SUCCESS;
        int kind=-1;
        for(int i=0;i<4;i++)if(profile_ok && a->suggested[i] && profile.interactionProfile==a->profiles[i])kind=i;
        a->diagnostic.reason="profile-unsupported";
        if (kind<0){
            static bool reported[2];
            if(!reported[h]){
                B2D_Record(0,"MENU_PROFILE hand=%u unavailable; other hand remains independent",h);
                reported[h]=true;
            }
            a->armed556[h]=0;memset(a->pose_seen556[h],0,sizeof(a->pose_seen556[h]));
            continue;
        }
        if(profile.interactionProfile!=a->current[h]){
            a->current[h]=profile.interactionProfile;a->armed556[h]=0;
            memset(a->pose_seen556[h],0,sizeof(a->pose_seen556[h]));
            if(a->generation==UINT32_MAX){a->exhausted=true;b2ca_invalidate556(a);return false;}
            ++a->generation; // A different binding contract remains an ownership boundary.
        }
        kinds[h]=kind;a->profile_kind582[h]=(unsigned)kind+1;
        b2c_hand_t *hand=&p.hands[h];
        hand->aim_valid=b2ca_pose556(a,h,0,base,time,now,hand->aim_position,hand->aim_orientation,
            &hand->aim_active556,&hand->aim_tracked556,&hand->aim_tracked_ms556);
        hand->grip_valid=b2ca_pose556(a,h,1,base,time,now,hand->grip_position,hand->grip_orientation,
            &hand->grip_active556,&hand->grip_tracked556,&hand->grip_tracked_ms556);
        XrActionStateGetInfo get={.type=XR_TYPE_ACTION_STATE_GET_INFO,.subactionPath=a->hands[h]};
        if (kind!=3) {
        XrActionStateFloat trigger={.type=XR_TYPE_ACTION_STATE_FLOAT};
        get.action=a->actions[B2CA_TRIGGER];
        a->diagnostic.reason="trigger-read-or-inactive";
        if(a->xrGetActionStateFloat(a->session,&get,&trigger)==XR_SUCCESS && trigger.isActive &&
           isfinite(trigger.currentState) && trigger.currentState>=0 && trigger.currentState<=1){
            hand->active556|=B2C_TRIGGER556;hand->trigger=trigger.currentState;
        }
        XrActionStateFloat squeeze={.type=XR_TYPE_ACTION_STATE_FLOAT};
        get.action=a->actions[B2CA_SQUEEZE];
        a->diagnostic.reason="squeeze-read";
        if(a->xrGetActionStateFloat(a->session,&get,&squeeze)==XR_SUCCESS && squeeze.isActive &&
           isfinite(squeeze.currentState) && squeeze.currentState>=0 && squeeze.currentState<=1){
            hand->active556|=B2C_GRIP556;hand->squeeze_active=true;hand->squeeze=squeeze.currentState;
        }
        XrActionStateVector2f stick={.type=XR_TYPE_ACTION_STATE_VECTOR2F};
        get.action=a->actions[B2CA_STICK];
        a->diagnostic.reason="stick-read-or-inactive";
        if(a->xrGetActionStateVector2f(a->session,&get,&stick)==XR_SUCCESS && stick.isActive &&
           isfinite(stick.currentState.x) && isfinite(stick.currentState.y) &&
           fabsf(stick.currentState.x)<=1 && fabsf(stick.currentState.y)<=1){
            hand->active556|=B2C_STICK556;hand->stick[0]=stick.currentState.x;hand->stick[1]=stick.currentState.y;
        }
        } // Simple profile never claims analogue trigger, squeeze or stick capabilities.
        bool *buttons[]={&hand->primary,&hand->secondary,&hand->menu,&hand->click};
        for (unsigned b=0;b<4;b++) {
            if (kind==3 && b!=0 && b!=2) continue;
            XrActionStateBoolean state={.type=XR_TYPE_ACTION_STATE_BOOLEAN};
            get.action=a->actions[B2CA_PRIMARY+b];
            a->diagnostic.reason="button-read-or-inactive";
            bool read_ok=a->xrGetActionStateBoolean(a->session,&get,&state)==XR_SUCCESS;
            // Suggestions must use profile-defined paths. On a supported
            // non-Touch profile the runtime may supply an ordinary Menu action
            // through a user binding even without a suggested menu/click path.
            bool active=read_ok && state.isActive;
            if(b==2){
                raw_active[h]=active;
                raw_down[h]=active && state.currentState;
                if(kind==0 && h==1)active=false;
                active_menu[h]=active;
            }
            static const unsigned bits[]={B2C_PRIMARY556,B2C_SECONDARY556,B2C_MENU556,B2C_CLICK556};
            if(active)hand->active556|=bits[b];
            *buttons[b]=active && state.currentState;
        }
        b2ca_controls556(a,h,hand);
    }
    a->diagnostic.reason="read-complete";a->diagnostic.hand=-1;
    b2ca_menu_note320(kinds,raw_active,raw_down,&p);
    memcpy(a->menu_active316,active_menu,sizeof(active_menu));
    p.generation=a->generation; p.ready=true; *out=p; return true;
}
#endif
