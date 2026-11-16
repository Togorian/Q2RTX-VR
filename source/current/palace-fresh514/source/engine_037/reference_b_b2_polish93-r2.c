// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 14 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
#include "../../../runtime-recovery555/source/b2_runtime555.h"
#include "current/input-comfort556/source/b2_client556.h"
#include "../../../../baseline/b2-controls-ai293/source/b2_graphics270.h"
// SPDX-License-Identifier: GPL-2.0-or-later
#include "../../../../baseline/b2-consolidated215/inputs/origin-058/quad-views/source/src/client/client.h"
#include "../b2_fresh_profile514.h"
#include "../../../../baseline/b2-denoiser-state233/source/b2_denoiser_state.h"
#include "../../../../baseline/b2-denoiser-state233/source/b2_denoiser_history.inc"
#include "../../../../baseline/b2-consolidated215/inputs/origin-040/include/refresh/b2_menu151.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/b2/reference_b_b2_lifecycle.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-058/quad-views/source/inc/refresh/b2_world.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-058/quad-views/source/src/client/reference_b_b2.h"
#include "../../../../baseline/b2-vr-resolution273/include/engine_037/refresh/reference_b_b2.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-058/quad-views/source/inc/shared/b2_combat.h"
#include "../../../right-menu409/source/b2_controller409.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_weapon_bridge.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_roomscale_bridge.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_wall_return.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_wall_query.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_teleport_bridge.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_weapon_presentation.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_weapon_display.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_weapon_visual.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_weapon_observation.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-058/quad-views/source/inc/refresh/models.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_haptics.h"
#include "../../../megahealth426/source/engine_037/b2_haptic_state.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_reticle.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-084/include/refresh/b2_weapon_ui.h"
#include "../../../recentre347/final/source/b2_bindings287.h"
#define B2C_REDUCER
#include "../../../right-menu409/source/b2_controller_impl409.inc"
#include "../../../recentre347/final/source/b2_bindings_state287.inc"
#include "../../../../baseline/b2-menu-polish253/source/b2_cinematic_skip253.inc"
#include "../../../../baseline/b2-comfort-feedback304/source/b2_vr_repeat253.h"
#include "../../../../baseline/b2-resume-cue261/source/b2_resume_cue261.h"
static struct {
    bool pending, frozen, stopped, origin_latched, advancing;
    bool walking, turning, firing, target, vertical, walk_ready, walk_release_required;
    bool combat, respawn_sent;
    bool lifecycle, world_waiting, world_rebinding, headset_waiting;
    unsigned world_epoch;
    int retired_servercount;
    bool ui_enabled, ui_manual_pause, ui_paused;
    bool ui_interrupted, ui_resume_request, ui_recovery_ready;
    bool ui_manual_resume582;
    unsigned ui_good_frames582;
    uint64_t ui_good_serial582;
    uint32_t ui_good_capture582, ui_good_input582, ui_good_actions582;
    uint32_t ui_xr_epoch;
    int ui_destination;
    int ui_server_frame, ui_client_time, ui_server_time;
    player_state_t ui_player;
    vec3_t ui_player_origin;
    b2_combat_state_t combat_state;
    refdef_t scene;
    entity_t *entities;
    particle_t *particles;
    dlight_t *dlights;
    lightstyle_t *lightstyles;
    byte *areabits;
    b2_history_t history;
    float reference_origin[3];
    float reference_yaw;
    float controller_reference_height;
    bool kbm_height_reference;
    bool recenter_crouch_held;
    bool recenter_held, recenter_pending, handoff_position_pending;
    unsigned recenter_epoch;
    bool audio_pending;
    vec3_t audio_origin, audio_angles;
    int servercount, frame_number, client_time, server_time;
    int prior_pause;
    qhandle_t demo_file;
    int64_t demo_offset;
    char mapname[MAX_QPATH];
    vec3_t anchor_origin, anchor_angles, input_angles;
    pmove_state_t anchor_player;
    pmove_state_t walk_previous;
    int walk_previous_time;
    int anchor_health, anchor_gunindex;
    int anchor_frame, anchor_client_time, anchor_server_time;
    uint32_t scene_serial, entities_hash;
    int reticle_count,teleport_count;
    int entity_base,entity_capacity;
    bool reticle_tracked,weapon_base_valid;
    vec3_t weapon_base_muzzle;
    float reticle_aim[12];
    uint32_t particles_hash;
    uint64_t particle_allocations;
} b2_client;
static bool b2_menu_transition;
static uint32_t b2_menu_interruption;
static b2c_state_t b2_controller;
static unsigned b2_screen_phase338;
static b2c_snapshot_t b2_controller_sample;
static cvar_t *b2_controller_input, *b2_controller_dominant;
static cvar_t *b2_input_mode484;
static cvar_t *b2_controller_turn_mode, *b2_controller_resume;
#include "../../../reference-realignment397/source/b2_realign397.h"
static struct {
    bool active,visible,armed,pressed,previous,key_pressed,pending,duck;
    uint32_t epoch,generation,xr_generation,owner,world,revision;
    unsigned hand;
    uint64_t serial;
    b2c_snapshot_t sample;
} b2_realign;
static bool b2_realign_duck,b2_realign_duck_seen,b2_realign_duck_standing602;
static unsigned b2_realign_duck_world;
static void b2_reference_retire397(void);

static cvar_t *b2_controller_smooth_speed, *b2_controller_snap_angle;
// Private interruption edge state; no public, save, network or game ABI.
static struct {
    bool configured, armed, previous, pending, pressed;
    uint64_t serial;
    uint32_t generation, xr_generation, owner, world, interruption;
    int dominant, turn, tracked, teleport;
} b2_resume;
static void b2_resume_reset(void);
static void b2_release_scene(void);
static void b2_menu_cancel253(void);
static bool b2_inventory_feedback342(void);
static void b2_inventory_cancel342(void);
// Pre-play UI input has no gameplay ownership.
static cvar_t *b2_headset_start, *b2_give_all;
static bool b2_headset_bootstrapping;
static struct {
    bool configured, armed, previous, pressed, pending, keyboard_request;
    uint32_t sample_epoch;
    uint64_t serial;
    uint32_t generation, xr_generation, owner, world, interruption;
    int dominant, turn, tracked, teleport;
    b2c_state_t ui;
    b2c_snapshot_t sample;
} b2_start;
extern uint32_t B2XR_InputGeneration(void);
extern bool CL_B2ShuttingDown457(void);
static bool b2_controller_enabled(void);
static void b2_headset_wait_unavailable(void)
{
    if (!b2_client.headset_waiting) return;
    b2_client.ui_recovery_ready=false;
    b2_start.configured=b2_start.armed=b2_start.pressed=b2_start.pending=b2_start.keyboard_request=false;
    b2c_clear(&b2_start.ui);
}
static bool b2_headset_start_valid(uint32_t now);
static bool b2_headset_start_sample594(uint32_t now);
static bool b2_headset_reference(const b2c_snapshot_t *p, float origin[3], float *yaw);
static void b2_headset_start_tick(uint32_t now);
bool B2_HeadsetWaiting(void)
{
    return b2_headset_start && b2_headset_start->integer==1 && !b2_client.stopped &&
        (!b2_client.world_epoch || b2_client.headset_waiting);
}
static cvar_t *b2_tracked_weapon;
static bool b2move_eligible301(uint32_t now);
#include "../../../../baseline/b2-feedback-vignette308/source/b2_comfort_release308.h"
static b2_comfort_release308_t b2_comfort_release308;
static bool b2_smooth_published304;
static uint32_t b2_smooth_ms304;
static uint32_t b2_turn_ms556;
static bool b2_turn_published556;
static cvar_t *b2_teleport,*b2_teleport_range,*b2_comfort_vignette,*b2_comfort_strength;
static bool b2move_active301(uint32_t now);
static b2tp_input_t b2tp_input;
static b2tp_preview_t b2tp_preview;
static b2tp_hull_t b2tp_preview_hull;
enum { B2TP_CIRCLE_SEGMENTS=32, B2TP_GUIDE_ENTITIES=2*(B2TP_POINTS-1)+B2TP_CIRCLE_SEGMENTS };
static uint32_t b2tp_preview_ms,b2tp_accepted_ms;
static bool b2tp_use,b2tp_pulsing;
static bool b2tp_enabled(void);
static void b2tp_cancel(void);
static int b2tp_geometry(entity_t beams[B2TP_GUIDE_ENTITIES]);
static cvar_t *b2_world_reticle, *b2_world_reticle_scale;
static uint32_t b2w_owner_epoch=1;
static int b2w_config_hand=-1, b2w_config_turn=-1;
static uint64_t b2w_published_serial;
static b2w_inventory_t b2w_published_inventory;
static b2w_selector_t b2w_selector;
static b2w_hud_t b2w_hud;
static bool b2w_actions_suppressed=true, b2w_config_feature;
static uint64_t b2w_token_counter, b2w_pending_token;
static uint32_t b2w_pending_ms, b2w_pending_game_epoch;
static int b2w_pending_slot=-1;
static bool b2w_pending_sent;
static bool b2w_waiting_input;
static int b2w_last_refusal;
static struct {
    bool active;
    b2w_pose_t pose;
    b2wm_key_t model;
    uint32_t handle;
    b2w_inventory_t inventory;
} b2w_blocked_preview;
static bool b2w_move_only_ready(uint32_t now);
static bool b2w_input_move_only556(uint32_t now);
static bool b2_controller_locomotion556(uint32_t now);
static uint64_t b2w_input_move_serial556;
static b2w_epochs_t b2w_input_move_epochs556;
static b2w_epochs_t b2w_pending_epochs;
static bool b2w_move_latched;
static float b2w_move_cos=1.f, b2w_move_sin;
static void b2_controller_handoff_f(void);
static void b2_controller_status_f(void);
static void b2_recenter_down_f(void);
static void b2_recenter_up_f(void);
extern bool UI_B2KeyboardInput530(void);
static cvar_t *b2_units_per_metre;
static cvar_t *b2_advance_world;
static cvar_t *b2_keyboard_locomotion, *b2_walk_fault, *b2_turning_mouselook;
static cvar_t *b2_jump_crouch;
static cvar_t *b2_combat;
static cvar_t *b2_same_map_ui;
static cvar_t *b2_lifecycle;
static cvar_t *b2_blaster_fire, *b2_visual_target, *b2_target_fault;
static void b2_stop_f(void);
static void b2_stop_at(const char *source, int line);
static bool b2aw_owned_hold(void);
static void b2aw_abort(void);
static void b2_clear_walk_input(void);
// One existing tracked head sample, independent of controller actions.
static cvar_t *b2_roomscale,*b2_physical_crouch,*b2_wall_return;
static b2rs_stance_t b2rs_stance;
static struct {
    bool active,head_seen,reproject;
    b2rs_eye_t eye;
    uint32_t world,recentre,owner,xr,sample_ms,activation_ms;
    uint64_t serial;
    float anchor[3],last_head[3];
} b2rs_client;
static void b2rs_retire(void)
{
    if(b2rs_client.active){
        float raw[3];B2RM_AcknowledgedOffset(raw);
        b2_client.reference_origin[0]+=raw[0];b2_client.reference_origin[2]+=raw[2];
    }
    if(b2rs_client.active)B2RM_Reset();b2rs_client.active=false;b2rs_client.serial=0;b2rs_client.reproject=false;
    B2RS_Suspend(&b2rs_stance);
}
// Retire an exhausted movement generation while the tracking context remains
// admitted. Fold only accepted travel into both accounting anchors, so blocked
// displacement survives the guard and physical return cannot become new travel.
static void b2rs_retire_movement(uint32_t now)
{
    float raw[3];B2RM_AcknowledgedOffset(raw);
    b2_client.reference_origin[0]+=raw[0];b2_client.reference_origin[2]+=raw[2];
    b2rs_client.anchor[0]+=raw[0];b2rs_client.anchor[2]+=raw[2];
    B2RM_Reset();b2rs_client.serial=0;b2rs_client.reproject=false;
    b2rs_client.activation_ms=now;
    B2RS_Suspend(&b2rs_stance);
}
static void b2rs_filter(void);
static void b2rs_turn(void)
{
    if(!b2rs_client.active)return;
    float raw[3];B2RM_AcknowledgedOffset(raw);
    b2_client.reference_origin[0]+=raw[0];b2_client.reference_origin[2]+=raw[2];
    b2rs_client.anchor[0]+=raw[0];b2rs_client.anchor[2]+=raw[2];
    uint32_t last_request=B2RM_LastRequestMs();
    if(last_request)b2rs_client.activation_ms=last_request;
    B2RM_Reset();
    b2rs_client.serial=0;b2rs_client.reproject=true;
    // Called after the heading commit. Cancel the old command identity and
    // reproject only the unaccepted residual under the new basis. Keep the
    // actual travel budget and stance clock, including continuous turning.
    b2rs_filter();
}

static void b2rs_reference(float out[3])
{
    float raw[3];B2RM_Offset(raw);
    VectorCopy(b2_client.reference_origin,out);
    out[0]+=raw[0];out[2]+=raw[2];
}

// Both aim preview and the later stereo view use this single proposed basis.
static bool b2rs_turn_space(float yaw,b2w_space_t *space)
{
    if(!yaw || b2_controller.turn_mode!=B2C_TURN_SMOOTH)return true;
    b2w_space_t old=*space;
    for(unsigned i=0;i<3;i++)old.anchor_angles[i]=SHORT2ANGLE((short)(
        ANGLE2SHORT(b2_client.input_angles[i])+b2_client.anchor_player.delta_angles[i]));
    float retired[3]={0};
    if(b2rs_client.active){
        float ack[3],predicted[3];B2RM_AcknowledgedOffset(ack);B2RM_Offset(predicted);
        for(unsigned i=0;i<3;i++)retired[i]=ack[i]-predicted[i];
    }
    return B2W_TurnSpace(&old,b2_controller_sample.head_position,
        space->anchor_angles,retired,space);
}
static void b2rs_commit_turn(float yaw,const b2w_space_t *space)
{
    if(!yaw)return;
    if(b2_controller.turn_mode!=B2C_TURN_SMOOTH){b2rs_turn();return;}
    if(b2rs_client.active){
        for(unsigned i=0;i<3;i++)
            b2rs_client.anchor[i]+=space->reference_origin[i]-b2_client.reference_origin[i];
        uint32_t last=B2RM_LastRequestMs();if(last)b2rs_client.activation_ms=last;
        B2RM_Reset();b2rs_client.serial=0;b2rs_client.reproject=true;
    }
    VectorCopy(space->reference_origin,b2_client.reference_origin);
    if(b2rs_client.active)b2rs_filter();
}

// Read actions only outside an XR frame; revalidate the captured sample at use.
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

static bool b2_recenter_crouch484(bool *held)
{
    b2c_snapshot_t sample;
    return B2XR_ReadControllerInput(&sample) && b2_recenter_sample488(&sample,held);
}

static void b2_recenter_height484(const float origin[3],bool held)
{
    memset(&b2rs_stance,0,sizeof(b2rs_stance));
    b2rs_stance.calibrated=true;
    b2rs_stance.baseline=b2rs_stance.last_height=origin[1];
    memset(&b2rs_client.eye,0,sizeof(b2rs_client.eye));
    // Remove only an old stock stance drop until the authoritative hull releases.
    b2rs_client.eye.owned=!held;b2rs_client.eye.weight=held?0.f:1.f;
    b2rs_client.eye.last_ms=Sys_Milliseconds();
    b2_client.recenter_crouch_held=held;
    b2_realign_duck=false;b2_realign_duck_seen=false;
}

static float b2rs_eye(void)
{
    if(!b2_client.walking || b2_client.combat_state.dead || cl.frame.ps.pmove.pm_type!=PM_NORMAL)
        return 0;
    if (b2_client.recenter_crouch_held)
        return B2RS_DuckDrop(true,(cl.oldframe.ps.pmove.pm_flags&PMF_DUCKED)!=0,
            (cl.frame.ps.pmove.pm_flags&PMF_DUCKED)!=0,cl.lerpfrac)-24.f;
    return b2rs_client.eye.weight*B2RS_DuckDrop(true,
        (cl.oldframe.ps.pmove.pm_flags&PMF_DUCKED)!=0,(cl.frame.ps.pmove.pm_flags&PMF_DUCKED)!=0,cl.lerpfrac);
}
static void b2rs_weapon_origin(const float origin[3],float out[3])
{
    // Use the same physical-crouch compensation as the published tracked pose.
    // The local player view height has already followed the authoritative duck.
    VectorCopy(origin,out);out[2]+=b2rs_eye();
}
static void b2rs_diagnostic(unsigned reason)
{
    static unsigned previous=UINT_MAX,records;
    static uint32_t logged;
    uint32_t now=Sys_Milliseconds();
    unsigned state=reason | (b2rs_client.active?32u:0u) |
        (b2rs_stance.calibrated?64u:0u) | (b2rs_stance.crouched?128u:0u);
    if(records>=128 || state==previous || (records && (uint32_t)(now-logged)<250))return;
    previous=state;logged=now;records++;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2ROOM reason=%u active=%d calibrated=%d physical=%d roomscale=%d crouch=%d local_height=%.6g baseline=%.6g record=%u/128\n",
        reason,b2rs_client.active,b2rs_stance.calibrated,b2rs_stance.crouched,
        b2_roomscale?b2_roomscale->integer:0,b2_physical_crouch?b2_physical_crouch->integer:0,
        b2rs_stance.last_height,b2rs_stance.baseline,records);
}
static void b2rs_filter(void)
{
    uint32_t now=Sys_Milliseconds();b2rs_head_t head;b2w_game_api_t api;
    bool enabled=(b2_roomscale && b2_roomscale->integer==1) ||
        (b2_physical_crouch && b2_physical_crouch->integer==1);
    bool eligible=enabled && b2_client.walking && b2_client.vertical && b2_client.combat &&
        b2_client.walk_ready && !b2_client.walk_release_required && b2_client.origin_latched &&
        !b2_client.stopped && !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.recenter_pending && !b2_client.handoff_position_pending &&
        !b2_client.ui_interrupted && !B2_UISimulationHeld() && B2_InputFocused556() &&
        cls.key_dest==KEY_GAME && cls.state==ca_active && cl.frame.valid &&
        cl.frame.ps.pmove.pm_type==PM_NORMAL && cl.frame.ps.stats[STAT_HEALTH]>0 &&
        cl_predict->integer==1 && !(cl.frame.ps.pmove.pm_flags&PMF_NO_PREDICTION) &&
        SV_B2WeaponAPI(&api) && B2XR_ReadHeadInput(&head);
    if(b2_controller.requested)eligible=eligible && b2_controller.valid &&
        !b2_controller.wait_neutral && b2_controller.destination==B2C_GAME &&
        b2c_sample_valid(&b2_controller_sample,now);
    bool ducked=(cl.frame.ps.pmove.pm_flags&PMF_DUCKED)!=0;
    bool manual=b2_controller.requested ? b2_controller.intent.up<0 : Key_IsDown('c')!=0;
    if(!eligible){
        unsigned reason=!enabled?1u:
            (!b2_client.walking || !b2_client.vertical || !b2_client.combat ||
             !b2_client.walk_ready || !b2_client.origin_latched)?2u:
            (b2_client.stopped || b2_client.world_waiting || b2_client.world_rebinding ||
             b2_client.recenter_pending || b2_client.handoff_position_pending ||
             b2_client.walk_release_required)?3u:
            (b2_client.ui_interrupted || B2_UISimulationHeld() ||
             !B2_InputFocused556() || cls.key_dest!=KEY_GAME)?4u:
            (cls.state!=ca_active || !cl.frame.valid ||
             cl.frame.ps.pmove.pm_type!=PM_NORMAL || cl.frame.ps.stats[STAT_HEALTH]<=0)?5u:
            (cl_predict->integer!=1 || (cl.frame.ps.pmove.pm_flags&PMF_NO_PREDICTION))?6u:
            (b2_controller.requested && (!b2_controller.valid || b2_controller.wait_neutral ||
             b2_controller.destination!=B2C_GAME || !b2c_sample_valid(&b2_controller_sample,now)))?7u:8u;
        b2rs_retire();b2rs_diagnostic(reason);return;
    }
    // Input generations also change on ordinary focus/tracking loss. Keep the
    // calibrated height; genuine reference changes use the separate continuity
    // or explicitly confirmed fresh-basis recovery path.
    if(!b2rs_client.active || b2rs_client.world!=b2_client.world_epoch ||
       b2rs_client.recentre!=b2_client.recenter_epoch || b2rs_client.owner!=b2w_owner_epoch ||
       b2rs_client.xr!=head.generation){
        b2rs_retire();B2RM_Reset();b2rs_client.active=true;
        b2rs_client.world=b2_client.world_epoch;b2rs_client.recentre=b2_client.recenter_epoch;
        b2rs_client.owner=b2w_owner_epoch;b2rs_client.xr=head.generation;
        VectorCopy(head.position,b2rs_client.anchor);b2rs_client.sample_ms=head.captured_ms;b2rs_client.activation_ms=now;
    }
    bool physical=B2RS_Stance(&b2rs_stance,b2_controller.requested && b2_physical_crouch->integer==1,
        head.valid,head.position[1],head.captured_ms,ducked,manual);
    if(physical){cl.cmd.upmove=-200;cl.localmove[2]=-200;}
    B2RS_EyeStep(&b2rs_client.eye,physical,manual,
        (cl.oldframe.ps.pmove.pm_flags&PMF_DUCKED)!=0,ducked,now);
    b2rs_diagnostic(0);
    bool reproject=b2rs_client.reproject;b2rs_client.reproject=false;
    if(b2rs_client.serial==head.serial && !reproject)return;
    uint32_t dt=head.captured_ms-b2rs_client.sample_ms;
    b2rs_client.sample_ms=head.captured_ms;b2rs_client.serial=head.serial;
    VectorCopy(head.position,b2rs_client.last_head);b2rs_client.head_seen=true;
    if(!dt && !reproject)return;
    if(dt>100){b2rs_retire();b2rs_diagnostic(9);return;}
    if(B2RM_Pending(now)){
        if((uint32_t)(now-B2RM_LastRequestMs())>100){b2rs_retire_movement(now);b2rs_diagnostic(10);}
        return;
    }
    if(b2_roomscale->integer!=1)return;
    float raw[3];B2RM_Offset(raw);
    float dx=head.position[0]-b2rs_client.anchor[0]-raw[0];
    float dz=head.position[2]-b2rs_client.anchor[2]-raw[2];
    float length=sqrtf(dx*dx+dz*dz);
    if(!isfinite(length)){b2rs_retire();b2rs_diagnostic(11);return;}
    if(length>1.f){b2rs_retire_movement(now);b2rs_diagnostic(11);return;}
    uint32_t last_request=B2RM_LastRequestMs();
    uint32_t travel_dt=now-(last_request?last_request:b2rs_client.activation_ms);
    if(travel_dt>100)travel_dt=100;
    float max_distance=2.f*travel_dt*.001f; // <=2m/s across command cadence, <=0.2m.
    if(length>max_distance){dx*=max_distance/length;dz*=max_distance/length;}
    if(fabsf(dx)+fabsf(dz)<.0001f)return;
    // Use the accepted position transform, projected to the collision plane.
    float yaw=SHORT2ANGLE((short)(ANGLE2SHORT(b2_client.input_angles[YAW])+b2_client.anchor_player.delta_angles[YAW]))*3.14159265358979323846f/180.f;
    float pitch=SHORT2ANGLE((short)(ANGLE2SHORT(b2_client.input_angles[PITCH])+b2_client.anchor_player.delta_angles[PITCH]))*3.14159265358979323846f/180.f;
    float roll=SHORT2ANGLE((short)(ANGLE2SHORT(b2_client.input_angles[ROLL])+b2_client.anchor_player.delta_angles[ROLL]))*3.14159265358979323846f/180.f;
    float c=cosf(b2_client.reference_yaw),s=sinf(b2_client.reference_yaw),u=b2_units_per_metre->value;
    float fx=cosf(pitch)*cosf(yaw),fy=cosf(pitch)*sinf(yaw);
    float rx=-sinf(roll)*sinf(pitch)*cosf(yaw)+cosf(roll)*sinf(yaw);
    float ry=-sinf(roll)*sinf(pitch)*sinf(yaw)-cosf(roll)*cosf(yaw);
    float basis[4]={u*(rx*c-fx*s),u*(-rx*s-fx*c),u*(ry*c-fy*s),u*(-ry*s-fy*c)};
    float move[3]={basis[0]*dx+basis[1]*dz,basis[2]*dx+basis[3]*dz,0};
    B2RM_Publish(head.serial,now,move,basis);
}


// Haptic events consume received server data. Never called by an eye render.
static b2h_state_t b2_haptics;
static bool b2_haptic_attract;
static uint64_t b2h_muzzle_received, b2h_snapshot_received, b2h_compositions, b2h_nonzero;
static cvar_t *b2_haptics_master, *b2_haptics_damage, *b2_haptics_armour;
static cvar_t *b2_haptics_bfg, *b2_haptics_weapons;

#include "../../../../baseline/b2-menu-bindings316/source/b2_haptic_output308.h"
#include "../../../inventory342/source/b2_feedback304.inc"

// Optional haptic cancellation must not reset the independent vignette fade.
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
void B2H_ServerData(bool attract)
{
    B2H_Reset();
    b2_haptic_attract=attract;
}
static b2h_config_t b2h_client_config(void)
{
    bool active=b2_haptics_master && b2_haptics_master->integer!=0 &&
        B2XR_Requested() && B2XR_InteractionReady() && !B2XR_StopRequested() &&
        B2_AdvancingWorld() && !b2_client.world_rebinding && !b2_client.ui_interrupted &&
        !b2_client.ui_paused && cls.state==ca_active && B2_InputFocused556() &&
        cls.key_dest==KEY_GAME && cl.frame.valid && !cls.demo.playback && !cls.demo.seeking &&
        !b2_haptic_attract && cl.frame.clientNum==cl.clientNum &&
        (cl.frame.ps.pmove.pm_type==PM_NORMAL || cl.frame.ps.pmove.pm_type==PM_DEAD ||
         cl.frame.ps.pmove.pm_type==PM_GIB);
    // Damage output is independent of this controller gameplay-owner predicate.
    bool controller=b2_controller_input && b2_controller_input->integer==1 &&
        b2_controller.requested && b2_controller.valid && !b2_controller.wait_neutral &&
        !b2_client.handoff_position_pending && !b2_client.recenter_pending &&
        cl.frame.ps.stats[STAT_HEALTH]>0 &&
        b2c_sample_valid(&b2_controller_sample,Sys_Milliseconds());
    unsigned hand=b2h_hand(info_hand?info_hand->integer:0,
        b2_controller_dominant?b2_controller_dominant->value:1.f);
    return (b2h_config_t){active,b2_haptics_weapons && b2_haptics_weapons->integer!=0,
        b2_haptics_bfg && b2_haptics_bfg->integer!=0,b2_haptics_damage && b2_haptics_damage->integer!=0,
        b2_haptics_armour && b2_haptics_armour->integer!=0,controller,hand};
}
static bool b2h_stop_pending316;
bool B2H_Enabled316(void)
{
    return b2_haptics_master && b2_haptics_master->integer!=0;
}
bool B2H_TakeStop316(void)
{
    bool stop=b2h_stop_pending316;
    b2h_stop_pending316=false;
    return stop;
}
static void b2h_options_changed(cvar_t *var)
{
    b2_feedback_reset304();
    if(var==b2_haptics_master){
        b2h_reset(&b2_haptics);
        b2h_stop_pending316=true;
        return;
    }
    // Apply cancellation even if a switch is toggled off/on between XR updates.
    // Keep the other logical categories; their maximum resumes next update.
    b2h_configure(&b2_haptics,b2h_client_config());
    B2XR_HapticStop();
}
static int b2h_muzzle_profile(int weapon)
{
    switch (weapon & ~MZ_SILENCED) {
    case MZ_BLASTER: case MZ_BLASTER2: return B2H_BLASTER;
    case MZ_SHOTGUN: case MZ_SHOTGUN2: return B2H_SHOTGUN;
    case MZ_SSHOTGUN: return B2H_SSHOTGUN;
    case MZ_MACHINEGUN: return B2H_MACHINEGUN;
    case MZ_CHAINGUN1: case MZ_CHAINGUN2: case MZ_CHAINGUN3: return B2H_CHAINGUN;
    case MZ_GRENADE: return B2H_GRENADE_LAUNCHER;
    case MZ_ROCKET: return B2H_ROCKET;
    case MZ_HYPERBLASTER: case MZ_BLUEHYPERBLASTER: return B2H_HYPERBLASTER;
    case MZ_RAILGUN: return B2H_RAILGUN;
    default: return -1; // No invented wire values for absent protocol extensions.
    }
}
void B2H_Muzzle(int entity, int weapon)
{
    b2h_configure(&b2_haptics,b2h_client_config());
    if (entity!=cl.clientNum+1 || entity!=cl.frame.clientNum+1) return;
    if (b2h_muzzle_received!=UINT64_MAX) b2h_muzzle_received++;
    int profile=b2h_muzzle_profile(weapon);
    if (profile>=0) b2h_muzzle(&b2_haptics,(unsigned)profile,Sys_Milliseconds());
}
void B2H_Snapshot(void)
{
    if (b2h_snapshot_received!=UINT64_MAX) b2h_snapshot_received++;
    b2h_configure(&b2_haptics,b2h_client_config());
    unsigned model=cl.frame.ps.gunindex&GUNINDEX_MASK;
    const char *name=model && model<cl.csr.max_models ? cl.configstrings[cl.csr.models+model] : "";
    int kind=!strcmp(name,"models/weapons/v_handgr/tris.md2")?B2H_MODEL_GRENADE:
        !strcmp(name,"models/weapons/v_bfg/tris.md2")?B2H_MODEL_BFG:B2H_MODEL_OTHER;
    b2h_snapshot_t snapshot={cl.frame.valid,cl.frame.number,cl.servercount,cl.frame.clientNum,
        cl.frame.ps.stats[STAT_HEALTH],cl.frame.ps.stats[STAT_ARMOR],cl.frame.ps.stats[STAT_FLASHES],
        kind,cl.frame.ps.gunframe,(int)model,b2_client.world_epoch};
    b2h_snapshot(&b2_haptics,&snapshot,Sys_Milliseconds());
    b2_pickup_poll304(Sys_Milliseconds(),b2_haptics.config.active && b2_haptics.config.controller,b2_haptics.config.hand);
}
void B2H_Amplitudes(uint32_t now, float out[2])
{
    b2h_configure(&b2_haptics,b2h_client_config());
    b2h_compose(&b2_haptics,now,out);
    b2_feedback_compose304(now,out);
    if (b2h_compositions!=UINT64_MAX) b2h_compositions++;
    if ((out[0]>0 || out[1]>0) && b2h_nonzero!=UINT64_MAX) b2h_nonzero++;
    b2h_config_t c=b2_haptics.config;
    unsigned state=c.active|(c.controller<<1)|(c.weapons<<2)|(c.bfg<<3)|(c.damage<<4)|
        (c.armour<<5)|(c.hand<<6)|((out[0]>0)<<7)|((out[1]>0)<<8);
    static unsigned previous=UINT_MAX,records;static uint32_t logged;
    static uint64_t seen;
    if (records<64 && (!records || ((uint32_t)(now-logged)>=1000 &&
        (state!=previous || (seen!=b2h_created && (uint32_t)(now-logged)>=5000))))) {
        previous=state;logged=now;seen=b2h_created;records++;
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_HAPTIC_EVENTS state=%u local_muzzle=%llu snapshots=%llu envelopes=%llu compositions=%llu nonzero=%llu amplitude=%.3g,%.3g record=%u/64\n",
            state,(unsigned long long)b2h_muzzle_received,(unsigned long long)b2h_snapshot_received,
            (unsigned long long)b2h_created,(unsigned long long)b2h_compositions,
            (unsigned long long)b2h_nonzero,out[0],out[1],records);
    }
}


void B2H_Output308(uint32_t now,float out[2],uint32_t duration[2])
{
    B2H_Amplitudes(now,out);
    b2_feedback_duration308(now,out,duration);
}

bool B2_LifecycleEnabled(void) { return b2_client.lifecycle; }

unsigned B2_WorldMaintenanceEpoch(void)
{
    return b2_client.lifecycle && b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.stopped && !b2_client.history.frame_open && b2_client.world_epoch &&
        b2_lifecycle->integer == 1 && b2_client.combat && b2_client.ui_enabled &&
        B2XR_Requested() && B2XR_InteractionReady() && !B2XR_StopRequested() ? b2_client.world_epoch : 0;
}

bool B2_WorldBound(unsigned epoch)
{
    return epoch && b2_client.lifecycle && !b2_client.stopped && !b2_client.world_waiting &&
        b2_client.world_epoch == epoch && b2_client.servercount != b2_client.retired_servercount;
}

// Only ordinary engine lifecycle callers may retire a world. Never inside a pair.
void B2_BeginWorldChange(const char *reason)
{
    B2H_Reset();
    R_B2UIReset();
    if (!B2_LifecycleEnabled() || b2_client.stopped || b2_client.world_waiting) return;
    if (b2_client.history.frame_open) { b2_stop_at(__func__, __LINE__); return; }
    R_B2PairFinish(false);
    if (B2XR_StopRequested()) { b2_stop_at(__func__, __LINE__); return; }
    // Ordinary accepted map/load/progression is not a focus interruption.
    // Preserve an already authenticated transition across its repeated callers.
    if (!b2_menu_transition && !b2_client.ui_interrupted &&
        !b2_client.ui_manual_pause && B2_InputFocused556() &&
        B2XR_InteractionReady()) {
        b2_menu_transition=true;
        b2_menu_interruption=B2XR_InterruptionEpoch();
    }
    b2_clear_walk_input(); CL_B2ReleaseButtons();
    Z_Free(b2_client.entities); b2_client.entities=NULL;
    Z_Free(b2_client.particles); b2_client.particles=NULL;
    Z_Free(b2_client.dlights); b2_client.dlights=NULL;
    Z_Free(b2_client.lightstyles); b2_client.lightstyles=NULL;
    Z_Free(b2_client.areabits); b2_client.areabits=NULL;
    memset(&b2_client.scene,0,sizeof(b2_client.scene));
    memset(&cl.refdef,0,sizeof(cl.refdef));
    b2_client.retired_servercount=cl.servercount;
    b2_client.world_waiting=true; b2_client.world_rebinding=false;
    if (b2_client.world_epoch == UINT_MAX) { b2_stop_at(__func__, __LINE__); return; }
    B2TP_WorldReset();
    b2_client.world_epoch++; b2_client.walk_ready=false;
    b2_reference_retire397();
    b2_client.walk_release_required=true;
    b2_client.ui_paused=false; b2_client.ui_interrupted=true;
    b2_client.ui_resume_request=b2_client.ui_recovery_ready=false;
    b2_client.audio_pending=false; b2_client.history.valid=false;
    b2_client.origin_latched=false; b2_client.reference_yaw=0;
    b2_client.kbm_height_reference = false;
    b2_client.handoff_position_pending=false;
    b2_client.recenter_pending=false;
    Cvar_SetByVar(cl_paused,"0",FROM_CODE); Cvar_Set("sv_paused","0");
    B2XR_InvalidateHistory("world-lifecycle");
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_LIFECYCLE retire epoch=%u reason=%s old_scene_released=1 pair_closed=1 normal_transition_release_rearm=1\n",b2_client.world_epoch,reason); }
}

static bool b2_drop_menu555;
bool B2_DropMenu555(void) { return b2_drop_menu555 && !b2_client.stopped && !B2XR_StopRequested(); }

void B2_DropToMenu555(void)
{
    b2_drop_menu555=false;
    if (!B2XR_Requested()) return;
    unsigned next=b2_client.world_epoch;
    if (B2_LifecycleEnabled() && !b2_client.world_waiting) {
        if (next==UINT_MAX) { b2_stop_at(__func__,__LINE__);return; }
        ++next;
    }
    if (!R_B2DropRetire555(&next)) {
        B2XR_AbortFrame555();b2_stop_at(__func__,__LINE__);return;
    }
    if (!B2XR_AbortFrame555()) { b2_stop_at(__func__,__LINE__);return; }
    b2_client.history.frame_open=false;b2_client.history.valid=false;
    if (next) B2_BeginWorldChange("recoverable-game-error");
    else b2_release_scene(); // Before first world: ordinary initial bootstrap.
    if (b2_client.stopped || B2XR_StopRequested()) return;
    b2_clear_walk_input();CL_B2ReleaseButtons();UI_B2VRRepeatCancel();
    b2_menu_cancel253();b2_resume_reset();memset(&b2_start,0,sizeof(b2_start));
    b2tp_cancel();B2H_Reset();b2_reference_retire397();
    b2_client.walk_ready=false;b2_client.walk_release_required=true;
    b2_client.audio_pending=false;b2_client.world_rebinding=false;
    b2_client.handoff_position_pending=false;b2_client.recenter_pending=false;
    memset(&cl.refdef,0,sizeof(cl.refdef));
    B2_CinematicBoundary(false);R_B2UIReset();
    B2XR_InvalidateHistory("game-error-menu");
    b2_drop_menu555=true;
    Com_Printf("The game returned to the menu after an error. Existing saves are unchanged. Start a new game or load a normal save.\n");
}

void B2_LifecycleDisconnect(bool terminal)
{
    B2H_Reset();
    if (!B2_LifecycleEnabled()) return;
    if (terminal) b2_stop_at(__func__, __LINE__);
    else B2_BeginWorldChange("reconnect");
}

// Retirement is an expected gap, not an admitted gameplay publication.
// Validate permanent ownership/settings even while no new frame is available.
static bool b2_exclusive(void);
static bool b2_world_wait_valid(void)
{
    if (!b2_client.world_waiting) return true;
    if (b2_client.stopped) return false;
    if (!b2_client.lifecycle || !b2_client.world_epoch || b2_client.world_rebinding ||
        b2_client.history.frame_open || b2_client.stopped || !B2XR_Requested() ||
        B2XR_StopRequested() || !b2_exclusive() ||
        (!B2_UISurfaceActive() && (cls.serverAddress.type != NA_LOOPBACK || !sv_running->integer)) ||
        cls.demo.playback || (cl.frame.valid && cl.maxclients != 1) ||
        b2_lifecycle->integer != 1 || !b2_client.combat || !b2_client.ui_enabled ||
        b2_combat->integer != 1 || b2_same_map_ui->integer != 1 ||
        b2_advance_world->integer != (int)b2_client.advancing ||
        b2_keyboard_locomotion->integer != (int)b2_client.walking ||
        b2_turning_mouselook->integer != (int)b2_client.turning ||
        b2_blaster_fire->integer != (int)b2_client.firing ||
        b2_jump_crouch->integer != (int)b2_client.vertical ||
        b2_visual_target->integer != (int)b2_client.target ||
        b2_units_per_metre->modified || b2_target_fault->string[0] || b2_walk_fault->string[0]) {
        Com_Printf("B2_LIFECYCLE rejected waiting ownership/runtime/settings boundary\n");
        b2_stop_at(__func__, __LINE__); return false;
    }
    return true;
}

static bool b2_bind_new_world(void)
{
    if (!b2_world_wait_valid()) return false;
    b2_combat_state_t state;
    if (cls.state!=ca_active || !cl.frame.valid || !cl.mapname[0] ||
        cl.servercount==b2_client.retired_servercount) return false;
    bool local = cls.serverAddress.type==NA_LOOPBACK && sv_running->integer && cl.maxclients==1 && !cls.demo.playback;
    bool publication = local && SV_B2CombatState(&cl.frame.ps,&state);
    if (!local || !publication || b2_walk_fault->string[0]) {
        Com_Printf("B2_BIND_REJECT local=%d publication=%d movement_fault=%d frame=%d publication_id=%d\n",
            local,publication,b2_walk_fault->string[0]!=0,cl.frame.number,cl.frame.ps.stats[31]);
        b2_stop_at(__func__, __LINE__); return false;
    }
    // Discover UI and the current HUD/layout media before sealing the owned
    // world registration. These are CPU draw queues only; the next ordinary
    // R_BeginFrame clears them before either eye renders its real HUD.
    if (!SCR_B2PrepareUI()) { b2_stop_at(__func__, __LINE__); return false; }
    SCR_DrawB2HUD();
    if (!R_B2WorldReady(b2_client.world_epoch)) return false;
    b2_client.anchor_player=b2_client.walk_previous=cl.frame.ps.pmove;
    b2_client.walk_previous_time=cl.servertime;
    b2_client.combat_state=state; b2_client.respawn_sent=false;
    b2_client.servercount=cl.servercount;
    b2_client.frame_number=cl.frame.number; b2_client.client_time=cl.time;
    b2_client.server_time=cl.servertime;
    Q_strlcpy(b2_client.mapname,cl.mapname,sizeof(b2_client.mapname));
    VectorCopy(cl.viewangles,b2_client.input_angles);
    for(unsigned i=0;i<3;i++) if(!isfinite(cl.viewangles[i])) { b2_stop_at(__func__, __LINE__); return false; }
    b2_client.input_angles[ROLL]=-SHORT2ANGLE(cl.frame.ps.pmove.delta_angles[ROLL]);
    VectorCopy(b2_client.input_angles,cl.viewangles);
    b2_clear_walk_input(); CL_B2ReleaseButtons();
    cl.oldframe.ps=cl.frame.ps; VectorClear(cl.prediction_error); cl.predicted_step=0;
    b2_client.ui_destination=cls.key_dest;
    b2_client.world_waiting=false; b2_client.world_rebinding=true;
    b2_drop_menu555=false;
    if (b2_menu_transition && b2_menu_interruption==B2XR_InterruptionEpoch()) {
        b2_client.headset_waiting=true;b2_client.ui_manual_pause=false;
        b2_client.ui_xr_epoch=b2_menu_interruption;
        b2_controller.requested=false; // Retire gameplay owner before the startup gate.
        b2_client.ui_recovery_ready=false;memset(&b2_start,0,sizeof(b2_start));
    }
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_LIFECYCLE bind epoch=%u map=%s servercount=%d fresh_publication=%u\n",b2_client.world_epoch,cl.mapname,cl.servercount,state.serial); }
    return true;
}

bool B2_ControllerFocusMode556(void)
{
    return B2XR_Requested() && Cvar_VariableInteger("b2_input_mode")==1;
}
bool B2_InputFocused556(void)
{
    return B2_ControllerFocusMode556()?B2XR_InteractionReady():cls.active==ACT_ACTIVATED;
}

bool B2_KeyboardMode(void)
{
    return b2_client.walking || (b2_keyboard_locomotion && b2_keyboard_locomotion->integer == 1 &&
        b2_advance_world->integer == 1 && B2XR_Requested());
}

bool B2_TurningMode(void)
{
    return b2_client.turning || (B2_KeyboardMode() && b2_turning_mouselook &&
        b2_turning_mouselook->integer == 1);
}

void B2_UpdateLook(int msec, float mouse_yaw, float mouse_pitch)
{
    // Validate the previous owner before accepting intentional input. Never
    // silently adopt console/game writes to the client's orientation.
    B2_FilterInput();
    if (b2_controller.requested) return;
    if (!b2_client.turning || b2_client.stopped || B2_UIPaused() || !b2_client.walk_ready ||
        b2_client.walk_release_required || (b2_client.combat && b2_client.combat_state.dead))
        return;
    if (msec < 0 || !isfinite(mouse_yaw) || !isfinite(mouse_pitch)) {
        b2_stop_at(__func__, __LINE__); return;
    }
    if (msec > 250)
        Com_Printf("B2LOOK: long input frame msec=%d; yaw integrates elapsed input time, existing command physics timing retained.\n", msec);
    float yaw = b2_client.input_angles[YAW] + mouse_yaw + .03f * msec *
        ((Key_IsDown(K_LEFTARROW) != 0) - (Key_IsDown(K_RIGHTARROW) != 0));
    float delta_pitch = SHORT2ANGLE(b2_client.anchor_player.delta_angles[PITCH]);
    float pitch = b2_client.input_angles[PITCH] + delta_pitch + mouse_pitch;
    if (!isfinite(yaw) || !isfinite(pitch)) { b2_stop_at(__func__, __LINE__); return; }
    bool room_turn=yaw!=b2_client.input_angles[YAW] ||
       Q_clipf(pitch,-89.f,89.f)-delta_pitch!=b2_client.input_angles[PITCH];
    b2_client.input_angles[YAW] = remainderf(yaw, 360.f);
    b2_client.input_angles[PITCH] = Q_clipf(pitch, -89.f, 89.f) - delta_pitch;
    VectorCopy(b2_client.input_angles, cl.viewangles);
    if(room_turn)b2rs_turn();
}

static void b2_resume_reset(void)
{
    if (b2_resume.pending) {
        b2_client.ui_resume_request=false;
        Com_Printf("B2_CONTROLLER_RESUME refused; input or interruption boundary changed.\n");
    }
    memset(&b2_resume,0,sizeof(b2_resume));
}

// Local KBM display offsets only. Neither the body/view pose nor emission
// uses this state. CL_AddViewWeapon evaluates it once before shared-eye copies.
static struct {
    bool valid;
    uint32_t ms,game,world,recentre,owner;
    int slot,model;
    float xy[2],yaw;
} b2wn_visual;
static b2cp_state_t b2_pause;

static void b2_clear_walk_input(void)
{
    b2_client.recenter_crouch_held=false;
    b2_screen_phase338=0;
    b2_inventory_cancel342();
    UI_B2VRRepeatCancel(); b2_menu_cancel253();
    b2wn_visual.valid=false;memset(&b2_pause,0,sizeof(b2_pause));
    b2_resume_reset();
    b2tp_cancel();
    b2rs_retire();
    // A retired owner identity is never recycled within this process.
    if (b2w_owner_epoch) b2w_owner_epoch=b2w_owner_epoch==UINT32_MAX ? 0 : b2w_owner_epoch+1;
    b2w_published_serial=0;
    B2W_CancelSelector(&b2w_selector);b2w_selector.configured=false;
    b2w_actions_suppressed=true;b2w_pending_token=0;b2w_pending_slot=-1;
    b2w_pending_sent=false;b2w_move_latched=false;b2w_waiting_input=false;b2w_blocked_preview.active=false;
    memset(&b2w_hud,0,sizeof(b2w_hud));
    B2W_Reset();
    b2c_clear(&b2_controller);
    memset(&b2_controller_sample,0,sizeof(b2_controller_sample));
    if (b2_controller_input && b2_controller_input->integer==1 && !b2_client.handoff_position_pending)
        b2_client.recenter_pending=false;
    if (b2_client.turning) CL_B2DiscardMouse();
    VectorClear(cl.localmove);
    cl.mousemove[0] = cl.mousemove[1] = 0;
    cl.cmd.forwardmove = cl.cmd.sidemove = cl.cmd.upmove = 0;
    cl.cmd.buttons = cl.cmd.impulse = 0;
    // Discard queued gameplay input on stop or an input ownership transition,
    // preserving command numbering/timestamps.
    for (unsigned i = 0; i < CMD_BACKUP; ++i) {
        cl.cmds[i].forwardmove = cl.cmds[i].sidemove = cl.cmds[i].upmove = 0;
        cl.cmds[i].buttons = cl.cmds[i].impulse = 0;
        if (b2_client.turning)
            for (unsigned j = 0; j < 3; ++j)
                cl.cmds[i].angles[j] = ANGLE2SHORT(b2_client.input_angles[j]);
    }
}


static void b2b_reset_client(void)
{b2_clear_walk_input();memset(&b2_start,0,sizeof(b2_start));b2tp_cancel();}

bool B2_UIEnabled(void)
{
    return B2_MenuSession() || (b2_client.ui_enabled && (b2_client.pending || b2_client.frozen || b2_client.stopped));
}

// Only the opted-in same-space headset route may preserve a validated LOCAL
// reference change. UI/manual intent and the existing player hold are untouched.
bool B2_ReferenceRecoveryEnabled(void)
{
    return b2_headset_start && b2_headset_start->integer==1 &&
        b2_controller_resume && b2_controller_resume->integer==1 &&
        b2_controller_enabled() && b2_controller.requested && B2_UIEnabled() &&
        b2_client.frozen && b2_client.walk_ready && b2_client.origin_latched &&
        !b2_client.stopped && !b2_client.headset_waiting && !b2_client.world_waiting &&
        !b2_client.world_rebinding && !b2_client.recenter_pending && !b2_client.handoff_position_pending;
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

static bool b2_controller_resume_commit_ready(uint32_t now)
{
    const b2c_snapshot_t *p=&b2_controller_sample;
    return b2_resume.configured && b2_controller.requested && b2_controller_enabled() &&
        b2_controller_resume && b2_controller_resume->integer==1 &&
        b2_resume.owner==b2w_owner_epoch && b2_resume.world==b2_client.world_epoch &&
        b2_resume.interruption==b2_client.ui_xr_epoch &&
        b2_resume.generation==p->generation && b2_resume.xr_generation==p->xr_generation &&
        p->xr_generation==B2XR_InputGeneration() && b2c_sample_valid(p,now) &&
        p->serial==b2_controller.serial && p->generation==b2_controller.generation &&
        b2_resume.dominant==b2_controller_dominant->integer &&
        b2_controller_turn_mode && b2_resume.turn==b2_controller_turn_mode->integer &&
        b2_resume.tracked==(b2_tracked_weapon ? b2_tracked_weapon->integer : 0) &&
        b2_resume.teleport==(b2_teleport ? b2_teleport->integer : 0) &&
        (!b2_headset_start || b2_headset_start->integer!=1 || b2c_neutral(p));
}

bool B2_UIPaused(void) { return B2_UIEnabled() && b2_client.ui_paused; }
bool B2_UISimulationHeld(void) { return B2_UIEnabled() && (b2_client.ui_paused || b2_client.stopped); }





static bool b2_ui_destination(int dest)
{
    return dest == KEY_GAME || dest == KEY_MENU || dest == KEY_CONSOLE || dest == (KEY_MENU | KEY_CONSOLE);
}

static bool b2_ui_validate_hold(void)
{
    if (!B2_UIPaused()) return true;
    int server_frame; player_state_t player; vec3_t origin;
    bool available=SV_B2PauseState(&server_frame,&player,origin);
    bool frame_changed=available && server_frame!=b2_client.ui_server_frame;
    bool player_changed=available && memcmp(&player,&b2_client.ui_player,sizeof(player));
    bool origin_changed=available && memcmp(origin,b2_client.ui_player_origin,sizeof(origin));
    bool client_clock_changed=cl.time!=b2_client.ui_client_time;
    bool server_clock_changed=cl.servertime!=b2_client.ui_server_time;
    if (!available || frame_changed || player_changed || origin_changed ||
        client_clock_changed || server_clock_changed) {
        Com_Printf("B2_UI_HOLD_MISMATCH available=%d server_frame=%d player=%d origin=%d client_clock=%d server_clock=%d\n",
            available,frame_changed,player_changed,origin_changed,client_clock_changed,server_clock_changed);
        Com_Printf("B2UI: simulation/player/clock changed while paused; terminal stop.\n"); b2_stop_at(__func__, __LINE__); return false;
    }
    return true;
}

// This boundary owns only the stock give-all pickup HUD delta. The held
// player, simulation clocks and world remain subject to the ordinary checks.
static bool b2_ui_give_all_available(void)
{
    return B2_UIPaused() && b2_client.lifecycle && b2_client.combat &&
        b2_client.frozen && b2_client.walk_ready && !b2_client.stopped &&
        !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.ui_interrupted && !b2_client.history.frame_open &&
        B2_InputFocused556() && B2XR_InteractionReady() &&
        (cls.key_dest & KEY_CONSOLE) && cls.serverAddress.type == NA_LOOPBACK &&
        sv_running->integer && cl.maxclients == 1;
}

bool B2_UIBeginGiveAll(player_state_t *before)
{
    b2_combat_state_t state;
    if (!before || !b2_ui_give_all_available() || !b2_ui_validate_hold()) return false;
    if (!SV_B2CombatState(&cl.frame.ps, &state)) {
        b2_stop_at(__func__, __LINE__); return false;
    }
    // The received frame owns publication; held live pmove may be newer.
    *before = b2_client.ui_player;
    b2_clear_walk_input(); CL_B2ReleaseButtons();
    b2_client.walk_release_required = true;
    return true;
}

void B2_UIEndGiveAll(const player_state_t *before)
{
    int frame; player_state_t player, expected; vec3_t origin;
    b2_combat_state_t state;
    if (!before || !b2_ui_give_all_available() ||
        memcmp(before, &b2_client.ui_player, sizeof(*before)) ||
        !SV_B2PauseState(&frame, &player, origin) ||
        frame != b2_client.ui_server_frame ||
        memcmp(origin, b2_client.ui_player_origin, sizeof(origin)) ||
        cl.time != b2_client.ui_client_time || cl.servertime != b2_client.ui_server_time)
        goto invalid;
    expected = *before;
    expected.stats[STAT_PICKUP_ICON] = player.stats[STAT_PICKUP_ICON];
    expected.stats[STAT_PICKUP_STRING] = player.stats[STAT_PICKUP_STRING];
    expected.stats[STAT_SELECTED_ITEM] = player.stats[STAT_SELECTED_ITEM];
    if (memcmp(&expected, &player, sizeof(player)) || !SV_B2CombatState(&cl.frame.ps, &state))
        goto invalid;
    // No whole-player recapture: only these stock pickup fields are owned here.
    b2_client.ui_player.stats[STAT_PICKUP_ICON] = player.stats[STAT_PICKUP_ICON];
    b2_client.ui_player.stats[STAT_PICKUP_STRING] = player.stats[STAT_PICKUP_STRING];
    b2_client.ui_player.stats[STAT_SELECTED_ITEM] = player.stats[STAT_SELECTED_ITEM];
    return;
invalid:
    Com_Printf("B2UI: give-all changed an unowned hold field; terminal stop.\n");
    b2_stop_at(__func__, __LINE__);
}

static void b2_ui_apply_pause(void)
{
    if ((!b2_client.frozen && !b2_client.pending) || B2_UISurfaceActive() || b2_client.stopped || b2_client.world_waiting || !b2_ui_validate_hold()) return;
    bool paused = b2aw_owned_hold() || b2_client.headset_waiting || b2_client.ui_interrupted || b2_client.ui_manual_pause || cls.key_dest != KEY_GAME;
    bool entering = paused && !b2_client.ui_paused;
    int value = b2_client.ui_manual_pause ? 2 : paused ? 1 : 0;
    if (paused != b2_client.ui_paused) {
        b2_clear_walk_input(); CL_B2ReleaseButtons();
        b2_client.walk_release_required = true;
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2UI: %s. Head tracking stays live.\n", paused ? "Paused" : "Resumed - release gameplay keys before moving");
    }
    if (entering && !CL_B2PreparePause()) {
        // A drained packet may retire this world. Its lifecycle owns the next hold.
        if (b2_client.world_waiting || b2_client.world_rebinding) return;
        Com_Printf("B2UI: pre-pause client/scene boundary unavailable; terminal stop.\n"); b2_stop_at(__func__, __LINE__); return;
    }
    b2_client.ui_paused = paused;
    Cvar_SetByVar(cl_paused, va("%d", value), FROM_CODE);
    // Pending local packets, interpolation and the held scene are complete.
    // Establish one hold here; never align or recapture an existing hold.
    Cvar_Set("sv_paused", paused ? "1" : "0");
    if (entering) {
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_UI_PAUSE_BOUNDARY client_frame=%d client_time=%d server_time=%d\n", cl.frame.number,cl.time,cl.servertime);
        b2_client.ui_client_time=cl.time; b2_client.ui_server_time=cl.servertime;
        if (!SV_B2PauseState(&b2_client.ui_server_frame,&b2_client.ui_player,b2_client.ui_player_origin)) {
            Com_Printf("B2UI: local pause snapshot unavailable; terminal stop.\n"); b2_stop_at(__func__, __LINE__);
        }
    }
}

// This holds the last coherent world without changing manual/menu intent.
// Call only outside an XR/renderer frame; pause preparation may parse packets.
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

#include "../../../upper-palace500/source/b2_realign_client500.inc"

bool B2_KeepFramePump(void)
{
    return B2_UISurfaceActive() || (B2_UIEnabled() && (b2_client.headset_waiting || b2_client.walk_ready || b2_client.world_waiting || b2_client.world_rebinding) && !b2_client.stopped);
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

void B2_UIFocusChanged(void)
{
    if (!B2_UIEnabled() || b2_client.stopped) return;
    if (!b2_client.frozen || B2_UISurfaceActive()) {
        b2_client.ui_destination=cls.key_dest;b2_clear_walk_input();CL_B2ReleaseButtons();return;
    }
    if (b2_client.world_waiting || b2_client.world_rebinding) { b2_client.ui_destination=cls.key_dest; return; }
    if (!b2_client.frozen || (!b2_client.walk_ready && !b2_client.headset_waiting) || !b2_ui_destination(cls.key_dest)) {
        Com_Printf("B2UI: unsupported interface transition; terminal stop.\n"); b2_stop_at(__func__, __LINE__); return;
    }
    b2_client.ui_destination = cls.key_dest;
    b2_clear_walk_input(); CL_B2ReleaseButtons(); b2_client.walk_release_required = true;
    b2_ui_apply_pause();
}

void B2_UITogglePause(void)
{
    if (!B2_UIEnabled()) return;
    if (b2_client.stopped || !b2_client.walk_ready) {
        Com_Printf("B2UI: pause/resume unavailable after terminal stop or before startup completes. Close the game window to quit.\n"); return;
    }
    b2_client.ui_manual_pause = !b2_client.ui_manual_pause;
    b2_ui_apply_pause();
}

void B2_UIResume(void)
{
    if (!B2_UIEnabled()) return;
    if (b2_client.stopped || (!b2_client.walk_ready && !b2_client.headset_waiting)) {
        Com_Printf("B2UI: terminal stop cannot resume. Close the game window to quit.\n"); return;
    }
    b2_client.ui_manual_pause = false;
    UI_OpenMenu(UIMENU_NONE); Con_Close(true); b2_ui_apply_pause();
}

// Both physical Pause and the opt-in controller edge request this same gate.
// A request never substitutes for the subsequent committed stereo pair.
static void b2_request_interruption_resume(bool controller)
{
    b2_client.ui_resume_request = !B2_RealignActive391() && B2_UIEnabled() && b2_client.ui_interrupted &&
        !b2_client.stopped && B2_InputFocused556() && b2_client.walk_ready &&
        !b2_client.world_waiting && !b2_client.world_rebinding;
    b2_resume.pending = controller && b2_client.ui_resume_request;
    if (controller) {
        if (!b2_client.ui_resume_request || Cvar_VariableInteger("developer") > 0) Com_Printf("B2_CONTROLLER_RESUME request=%s; fresh focused/tracked committed stereo pair required.\n",
            b2_client.ui_resume_request ? "accepted" : "refused");
    } else
        Com_Printf("B2_INTERRUPTION Pause requested; a fresh focused/tracked stereo pair is required.\n");
}

// Controller display pose B is standard. Handoff remains an explicit console command.














bool B2_UIKey(unsigned key, bool down)
{
    // Track the physical press, even if its binding or destination changes.
    // Release must also work after focus loss, realignment or VR shutdown.
    static bool recenter_keys530[256];
    if (!down && recenter_keys530[key]) {
        recenter_keys530[key]=false;
        for (unsigned i=0;i<256;i++) if (recenter_keys530[i]) return true;
        b2_recenter_up_f();
        return true;
    }
    if (down && recenter_keys530[key]) return true;
    if (!B2_UIEnabled()) return false;
    if(B2_RealignActive391() && key==K_PAUSE){b2_realign_key391(down);return true;}
    if(B2_RealignActive391())return true;
    const char *binding=Key_GetBindingForKey(key);
    if (binding && !Q_stricmp(binding,"+b2_recenter") &&
        !(cls.key_dest&(KEY_CONSOLE|KEY_MESSAGE)) && !UI_B2KeyboardInput530() &&
        key!=K_ESCAPE && key!='`' && key!='~' && key!=K_PAUSE &&
        !(key==K_ENTER && Key_IsDown(K_ALT))) {
        // Consume the ordinary +command route, which would skip a cinematic.
        // The existing request function still owns every focus/pose/state gate.
        if (down && Key_IsDown(key)==1) {
            recenter_keys530[key]=true;
            b2_recenter_down_f();
        }
        return true;
    }
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
    if (key == K_PAUSE && b2_client.ui_interrupted) {
        if (down && Key_IsDown(key) == 1 && !b2_client.stopped) {
            b2_request_interruption_resume(false);
        }
        return true;
    }
    if (key == K_PAUSE) {
        if (down && Key_IsDown(key) == 1) B2_UITogglePause();
        return true;
    }
    if (key == K_ENTER && Key_IsDown(K_ALT)) {
        if (down && Key_IsDown(key) == 1) Com_Printf("B2UI: display changes are unavailable in this session.\n");
        return true;
    }
    // Stock F-key actions and user non-button rebindings use ordinary dispatch.
    // Held gameplay button bindings still remain behind the simulation hold.
    if (key >= K_F1 && key <= K_F12) {
        const char *binding=Key_GetBindingForKey(key);
        return B2_UISimulationHeld() && binding && binding[0]=='+';
    }
    if (B2_UISimulationHeld() && cls.key_dest == KEY_GAME &&
        key != K_ESCAPE && key != '`' && key != '~' &&
        !(binding && !Q_stricmp(binding,"b2_controller_handoff"))) return true;
    return false;
}

// Ordinary local maps only. The stock parser owns validation, saves and
// spawning; do not retire this world until its target has loaded successfully.
static bool b2_ui_gamemap(const char *text, size_t length)
{
    if (length < 7 || strncmp(text,"gamemap",7) ||
        (length > 7 && text[7]!=' ' && text[7]!='\t')) return false;
    size_t first=7;
    while (first<length && (text[first]==' ' || text[first]=='\t')) first++;
    size_t count=length-first;
    bool valid=count>0 && count<=MAX_QPATH-10;
    for (size_t i=first;i<length;i++) {
        char c=text[i];
        if (!((c>='a'&&c<='z') || (c>='0'&&c<='9') || c=='_' || c=='-')) valid=false;
    }
    if (!valid) {
        Com_Printf("Usage: gamemap NAME (1-54 lowercase letters, digits, _ or -; e.g. gamemap city1). No paths, extensions, quotes, *, $, + or chained commands.\n");
        return true;
    }
    if (!B2_UIPaused() || !b2_client.lifecycle || !b2_client.combat ||
        !b2_client.frozen || !b2_client.walk_ready || b2_client.stopped ||
        b2_client.world_waiting || b2_client.world_rebinding || b2_client.ui_interrupted ||
        b2_client.history.frame_open || !b2_client.world_epoch || b2_client.world_epoch==UINT_MAX ||
        cls.state!=ca_active || !cl.frame.valid || !B2_InputFocused556() ||
        !(cls.key_dest&KEY_CONSOLE) || cls.serverAddress.type!=NA_LOOPBACK || cls.demo.playback ||
        !sv_running->integer || cl.maxclients!=1 || !b2_exclusive() ||
        !B2XR_Requested() || !B2XR_InteractionReady() || B2XR_StopRequested() ||
        b2_lifecycle->integer!=1 || b2_combat->integer!=1 || b2_same_map_ui->integer!=1 ||
        b2_advance_world->integer!=(int)b2_client.advancing ||
        b2_keyboard_locomotion->integer!=(int)b2_client.walking ||
        b2_turning_mouselook->integer!=(int)b2_client.turning ||
        b2_blaster_fire->integer!=(int)b2_client.firing ||
        b2_jump_crouch->integer!=(int)b2_client.vertical ||
        b2_visual_target->integer!=(int)b2_client.target ||
        b2_units_per_metre->modified || b2_target_fault->string[0] || b2_walk_fault->string[0] ||
        !SV_B2GameMapAllowed() || !b2_ui_validate_hold()) {
        Com_Printf("gamemap unavailable: requires a focused, paused local baseq2 console with lifecycle and save ownership intact; no stopped or pending transition.\n");
        return true;
    }
    char command[MAX_QPATH];
    memcpy(command,"gamemap ",8); memcpy(command+8,text+first,count); command[8+count]=0;
    Cmd_ExecuteString(&cmd_buffer,command);
    return true;
}

// Stock difficulty-menu actions and the local console share the same deliberate
// transition. Validate before retiring the world; keep the cinematic chain.
static void b2_newgame_f(void)
{
    if (!B2_MenuCommand("map *ntro.cin+base1 force"))
        Com_Printf("New game unavailable: restore focus/tracking and release controls.\n");
}

// Fixed diagnostic filenames only; no general console or path access.
static bool b2_ui_condump(const char *text, size_t length)
{
    static const char *const commands[] = {
        "condump b2_eye_on_224_20261101a",
        "condump b2_eye_off_224_20261101a",
        "condump b2_prestart_227"
    };
    for (unsigned i=0;i<3;i++) {
        if (length!=strlen(commands[i]) || memcmp(text,commands[i],length)) continue;
        bool prestart=i==2;
        bool phase=prestart ? b2_client.headset_waiting && !b2_client.origin_latched :
            !b2_client.headset_waiting && b2_client.origin_latched &&
            b2_client.walk_ready && !b2_client.ui_interrupted;
        if (!phase || !b2_client.frozen || b2_client.pending || b2_client.stopped ||
            b2_client.world_waiting || b2_client.world_rebinding || b2_client.history.frame_open ||
            !B2_UIPaused() || cls.state!=ca_active || !B2_InputFocused556() ||
            !(cls.key_dest&KEY_CONSOLE) || cls.serverAddress.type!=NA_LOOPBACK ||
            cls.demo.playback || !sv_running->integer || cl.maxclients!=1) {
            Com_Printf("Diagnostic dump unavailable in this session state. Prestart requires the initial headset wait; eye comparisons require completed start.\n");
            return true;
        }
        Com_Printf("B2DENOISE233 request waiting=%d origin_latched=%d walk_ready=%d interrupted=%d\n",
            b2_client.headset_waiting, b2_client.origin_latched,
            b2_client.walk_ready, b2_client.ui_interrupted);
        b2ds_print(b2_client.history.logical_index, b2_client.history.generation,
            Sys_Milliseconds());
        Cmd_ExecuteString(&cmd_buffer,commands[i]);
        return true;
    }
    return false;
}



bool B2_UIConsole(const char *text)
{
    if (!B2_UIEnabled()) return false;
    char command[32]; size_t n;
    while (*text == ' ' || *text == '\t') ++text;
    if (*text == '/' || *text == '\\') ++text;
    n = strlen(text); while (n && (text[n-1] == ' ' || text[n-1] == '\t')) --n;
    if (b2_ui_condump(text,n)) return true;
    if (b2_ui_gamemap(text,n)) return true;
    if (n >= sizeof(command)) goto unsupported;
    memcpy(command, text, n); command[n] = 0;
    if (!strcmp(command,"b2_controller_handoff")) { b2_controller_handoff_f(); return true; }
    if (!strcmp(command,"b2_controller_status")) { b2_controller_status_f(); return true; }
    if (!strcmp(command,"exec RESET-KEYS476.cfg")) {
        if (b2_client.stopped || b2_client.history.frame_open || !b2_ui_validate_hold()) goto unsupported;
        return false; // Ordinary console exec; reviewed fragment lives in own home.
    }
    if (!strcmp(command,"give all")) {
        if (!b2_ui_give_all_available() || !b2_ui_validate_hold()) goto unsupported;
        // Preserve ordinary buffering/forwarding. The server rechecks the hold
        // when this command arrives, including focus lost after it was queued.
        Cbuf_AddText(&cmd_buffer, "give all\n");
        return true;
    }
    if (!strcmp(command,"newgame") && b2_client.lifecycle) {
        b2_newgame_f();
        return true;
    }
    if (b2_client.lifecycle && (!strncmp(command,"save ",5) || !strncmp(command,"load ",5))) {
        const char *slot=command+5;
        bool valid=!strncmp(slot,"life-",5) || (!strncmp(command,"load ",5) && !strcmp(slot,"save0"));
        for(const char *p=slot;*p;p++) if(!((*p>='a'&&*p<='z')||(*p>='0'&&*p<='9')||*p=='-')) valid=false;
        if(!valid || !slot[0] || b2_client.stopped || b2_client.world_waiting || b2_client.history.frame_open) goto unsupported;
        Cmd_ExecuteString(&cmd_buffer,command);
        return true;
    }
    if (!strcmp(command,"help") || !strcmp(command,"status")) {
        Com_Printf("Session console: help, status, pause, resume, menu, clear, stop, quit. Local equipment: give all. Lifecycle: gamemap NAME / newgame / save life-NAME / load life-NAME / load save0.\n");
        Com_Printf("gamemap city1 keeps stock map/save semantics; ordinary names only (no *, $, +, paths or cinematics). After loading, release controls and press Pause in a focused tracked session.\n");
        Com_Printf("State: %s; map %s; head tracking live while ordinarily paused. Esc/backtick closes console. Pause toggles pause;\n",
            b2_client.stopped ? "TERMINALLY STOPPED" : B2_UIPaused() ? "PAUSED" : "PLAYING", cl.mapname);
    } else if (!strcmp(command,"pause")) {
        if (!b2_client.stopped && b2_client.walk_ready) { b2_client.ui_manual_pause=true; b2_ui_apply_pause(); }
    } else if (!strcmp(command,"resume")) B2_UIResume();
    else if (!strcmp(command,"menu")) UI_OpenMenu(UIMENU_GAME);
    else if (!strcmp(command,"stop")) { Com_Printf("B2_UI_STOP_COMMAND\n"); B2XR_RequestStop(); b2_stop_at(__func__, __LINE__); }
    else if (!strcmp(command,"quit") || !strcmp(command,"clear")) {
        if (!strcmp(command,"quit")) Com_Printf("B2_UI_QUIT_COMMAND\n");
        Cmd_ExecuteString(&cmd_buffer,command);
    }
    else goto unsupported;
    return true;
unsupported:
    Com_Printf("This command is unavailable right now. Type help for supported commands.\n");
    return true;
}

bool B2_CombatSnapshot(void)
{
    if (b2_client.world_waiting) {
        b2_world_wait_valid();
        return false; // Do not predict or adopt an old/new publication before bind.
    }
    if (!b2_client.combat || (!b2_client.pending && !b2_client.frozen)) return true;
    b2_combat_state_t next;
    if (b2_client.stopped || !cl.frame.valid || cls.serverAddress.type != NA_LOOPBACK ||
        !sv_running->integer || cl.maxclients != 1 || cl.servercount != b2_client.servercount ||
        cl.frame.ps.pmove.gravity != b2_client.anchor_player.gravity ||
        (cl.frame.ps.pmove.pm_flags & (PMF_NO_PREDICTION | PMF_TELEPORT_BIT)) ||
        !SV_B2CombatState(&cl.frame.ps, &next) || next.serial < b2_client.combat_state.serial ||
        next.epoch < b2_client.combat_state.epoch || next.relocations < b2_client.combat_state.relocations ||
        (next.relocations != b2_client.combat_state.relocations && next.epoch == b2_client.combat_state.epoch) || next.returns < b2_client.combat_state.returns ||
        next.returns > b2_client.combat_state.returns + 1 ||
        (next.returns != b2_client.combat_state.returns &&
         (!b2_client.respawn_sent || !b2_client.combat_state.dead))) {
        if (!b2_client.stopped || Cvar_VariableInteger("developer") > 0) Com_Printf("B2_COMBAT rejected unverified publication/lifecycle\n"); b2_stop_at(__func__, __LINE__); return false;
    }
    if (b2_client.combat_state.serial && next.epoch != b2_client.combat_state.epoch) {
        if (b2_client.history.frame_open) { b2_stop_at(__func__, __LINE__); return false; }
        bool returned = next.returns != b2_client.combat_state.returns;
        bool relocated = next.relocations != b2_client.combat_state.relocations;
        if (returned || relocated) {
            if (b2_client.lifecycle && cl.frame.ps.pmove.pm_type==PM_FREEZE) {
                for(unsigned i=0;i<3;i++) b2_client.input_angles[i]=cl.frame.ps.viewangles[i]-SHORT2ANGLE(cl.frame.ps.pmove.delta_angles[i]);
                VectorCopy(b2_client.input_angles,cl.viewangles);
            }
            b2_client.anchor_player = cl.frame.ps.pmove;
            b2_client.respawn_sent = false;
        }
        b2_clear_walk_input();
        b2_client.walk_release_required = true;
        cl.oldframe.ps = cl.frame.ps;
#if USE_FPS
        cl.keyframe.ps = cl.oldkeyframe.ps = cl.frame.ps;
#endif
        VectorClear(cl.prediction_error);
        cl.predicted_step = 0;
        VectorScale(cl.frame.ps.pmove.origin, .125f, cl.predicted_origin);
        for (unsigned i = 0; i < CMD_BACKUP; i++)
            VectorCopy(cl.frame.ps.pmove.origin, cl.predicted_origins[i]);
        for (unsigned i = 0; i < 3; i++)
            cl.predicted_angles[i] = b2_client.input_angles[i] + SHORT2ANGLE(cl.frame.ps.pmove.delta_angles[i]);
        // The live renderer owns a monotonically increasing pair cadence.
        // Invalidate old images/poses without restarting that logical counter.
        b2_client.history.valid = false;
        B2XR_InvalidateHistory(returned ? "combat_same_area_respawn" : relocated ? "stock_teleport" : "combat_death");
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_COMBAT snapshot serial=%u epoch=%u returns=%u dead=%d history_reset=1 head_reference_preserved=1 release_required=1\n",
            next.serial, next.epoch, next.returns, next.dead);
    }
    b2_client.combat_state = next;
    return true;
}

void B2_InputFocusLost(void)
{
    extern void UI_B2LinksFocusLost(void);
    UI_B2LinksFocusLost(); // Desktop link/capture state still follows its window.
    if(B2_ControllerFocusMode556())return;
    B2B_Invalidate();
    B2H_Reset();
    if (B2_KeepFramePump()) { b2_interrupt("window-focus"); return; }
    if (b2_client.walking && !b2_client.stopped) {
        Com_Printf("B2WALK: input focus lost; terminal stop.\n");
        b2_stop_at(__func__, __LINE__);
    }
}

static bool b2_exclusive(void)
{
    if (cls.state == ca_active && !cls.demo.playback && B2_FreshRequested() &&
        !B2_FreshClient(&cl.csr, cls.serverProtocol, cls.protocolVersion,
            cl.esFlags, cl.psFlags, cls.serverAddress.type == NA_LOOPBACK))
        return false;
    return true;
}

static void b2_release_scene(void)
{
    b2_resume_reset();
    memset(&b2_start,0,sizeof(b2_start));
    b2tp_cancel();B2TP_WorldReset();
    B2H_Reset();
    Z_Free(b2_client.entities);
    Z_Free(b2_client.particles);
    Z_Free(b2_client.dlights);
    Z_Free(b2_client.lightstyles);
    Z_Free(b2_client.areabits);
    b2rs_retire();memset(&b2rs_client,0,sizeof(b2rs_client));
    memset(&b2_client, 0, sizeof(b2_client));
    memset(&b2_controller,0,sizeof(b2_controller));
    memset(&b2_controller_sample,0,sizeof(b2_controller_sample));
}

bool B2_AdvancingWorld(void)
{
    return b2_client.advancing && !b2_client.stopped &&
        (b2_client.pending || b2_client.frozen) && !b2_client.world_waiting;
}

// Angle ownership is exact for finite values, except IEEE signed zero.
// Stock centerview and pitch arithmetic can change zero sign without motion.
static bool b2_input_angles_match488(void)
{
    for (unsigned i=0;i<3;i++) {
        float a=cl.viewangles[i],b=b2_client.input_angles[i];
        uint32_t ab,bb;memcpy(&ab,&a,sizeof(ab));memcpy(&bb,&b,sizeof(bb));
        if (!isfinite(a) || !isfinite(b) ||
            (ab!=bb && ((ab&0x7fffffffU)!=0 || (bb&0x7fffffffU)!=0))) return false;
    }
    return true;
}

static void b2_report_input_boundary(void)
{
    uint32_t actual[3], expected[3];
    memcpy(actual, cl.viewangles, sizeof(actual));
    memcpy(expected, b2_client.input_angles, sizeof(expected));
    // Observation only, after the unchanged guard has failed. StopRequested
    // is a read-only flag query; no event polling or input collection here.
    Com_Printf("B2INPUT failed window=%d key_dest=%d walk=%d combat=%d turn=%d fire=%d vertical=%d target=%d target_fault=%d angle_owner=%d walk_fault=%d xr_stop=%d\n",
        !B2_InputFocused556(), cls.key_dest != KEY_GAME,
        b2_keyboard_locomotion->integer != 1,
        b2_combat->integer != (int)b2_client.combat,
        b2_turning_mouselook->integer != (int)b2_client.turning,
        b2_blaster_fire->integer != (int)b2_client.firing,
        b2_jump_crouch->integer != (int)b2_client.vertical,
        b2_visual_target->integer != (int)b2_client.target,
        b2_target_fault->string[0] != 0,
        b2_client.turning && !b2_input_angles_match488() != 0,
        b2_walk_fault->string[0] != 0, B2XR_StopRequested());
    Com_Printf("B2INPUT state window=%d required_window=%d key_dest=%d required_key_dest=%d pending=%d frozen=%d walk_ready=%d release_required=%d pause=%d server_pause=%d walk_fault=%.160s target_fault=%.160s\n",
        cls.active, ACT_ACTIVATED, cls.key_dest, KEY_GAME,
        b2_client.pending, b2_client.frozen, b2_client.walk_ready,
        b2_client.walk_release_required, cl_paused->integer, sv_paused->integer,
        b2_walk_fault->string, b2_target_fault->string);
    Com_Printf("B2INPUT angle_bits actual=%08x,%08x,%08x expected=%08x,%08x,%08x\n",
        (unsigned)actual[0], (unsigned)actual[1], (unsigned)actual[2],
        (unsigned)expected[0], (unsigned)expected[1], (unsigned)expected[2]);
}

// Intermission uses the existing attack command, independently of weapon muzzle
// publication. The distinct reducer destination requires release on each entry.
static bool b2_controller_intermission_attack(uint32_t now)
{
    return b2_client.lifecycle && cl.frame.ps.pmove.pm_type==PM_FREEZE &&
        b2_controller_enabled() && b2_controller.requested && b2_controller.valid &&
        !b2_controller.wait_neutral && b2_controller.destination==B2C_INTERMISSION &&
        b2_controller.dominant==(unsigned)b2_controller_dominant->integer &&
        b2_controller.turn_mode==(unsigned)b2_controller_turn_mode->integer &&
        b2_client.walk_ready && !b2_client.walk_release_required && b2_client.origin_latched &&
        !b2_client.stopped && !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.recenter_pending && !b2_client.handoff_position_pending &&
        !b2_client.history.frame_open && !b2_client.ui_interrupted &&
        !b2_client.combat_state.dead && !B2_UISimulationHeld() &&
        cls.state==ca_active && cl.frame.valid && B2_InputFocused556() &&
        cls.key_dest==KEY_GAME && B2XR_InteractionReady() &&
        b2c_sample_valid(&b2_controller_sample,now) &&
        b2_controller_sample.serial==b2_controller.serial &&
        b2_controller_sample.generation==b2_controller.generation &&
        b2_controller_sample.xr_generation==B2XR_InputGeneration() &&
        b2_controller.intent.attack;
}

// Stock death input; no weapon pose, movement, same-area return or synthetic key.
bool B2_ControllerDeadAttack(uint32_t now)
{
    return b2_client.lifecycle && (cl.frame.ps.pmove.pm_type==PM_DEAD || cl.frame.ps.pmove.pm_type==PM_GIB) &&
        b2_controller_enabled() && b2_controller.requested && b2_controller.valid &&
        !b2_controller.wait_neutral && b2_controller.destination==B2C_DEAD &&
        b2_controller.dominant==(unsigned)b2_controller_dominant->integer &&
        b2_controller.turn_mode==(unsigned)b2_controller_turn_mode->integer &&
        b2_client.walk_ready && !b2_client.walk_release_required && b2_client.origin_latched &&
        !b2_client.stopped && !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.recenter_pending && !b2_client.handoff_position_pending &&
        !b2_client.history.frame_open && !b2_client.ui_interrupted &&
        b2_client.combat && b2_client.combat_state.dead && cl.frame.ps.stats[STAT_HEALTH]<=0 && !B2_UISimulationHeld() &&
        cls.state==ca_active && cl.frame.valid && B2_InputFocused556() &&
        cls.key_dest==KEY_GAME && B2XR_InteractionReady() &&
        b2c_sample_valid(&b2_controller_sample,now) &&
        b2_controller_sample.serial==b2_controller.serial &&
        b2_controller_sample.generation==b2_controller.generation &&
        b2_controller_sample.xr_generation==B2XR_InputGeneration() &&
        b2_controller.intent.attack;
}

// Recheck current input admission when the local server consumes a queued
// intermission command; a prior admitted bit cannot outlive tracking or ownership.
bool B2_ControllerIntermissionAttack(uint32_t now)
{
    return b2_controller_intermission_attack(now);
}


static void b2_filter_input(void)
{
    bool keyboard = B2_KeyboardMode();
    if (b2_client.world_waiting) {
        b2_world_wait_valid(); b2_clear_walk_input(); return;
    }
    if (b2_client.world_rebinding) { b2_clear_walk_input(); return; }
    if (!keyboard && (!b2_client.advancing ||
        (!b2_client.pending && !b2_client.frozen && !b2_client.stopped)))
        return;
    if (b2_client.walking && !b2_client.stopped &&
        ((!B2_InputFocused556() && !b2_client.ui_interrupted) ||
         (B2_UIEnabled() ? (cls.key_dest != b2_client.ui_destination || !b2_ui_destination(cls.key_dest)) : cls.key_dest != KEY_GAME) ||
         b2_keyboard_locomotion->integer != 1 ||
         b2_combat->integer != (int)b2_client.combat ||
         b2_turning_mouselook->integer != (int)b2_client.turning ||
         b2_blaster_fire->integer != (int)b2_client.firing ||
         b2_jump_crouch->integer != (int)b2_client.vertical ||
         b2_visual_target->integer != (int)b2_client.target || b2_target_fault->string[0] ||
         (b2_client.turning && !b2_input_angles_match488()) || b2_walk_fault->string[0] || B2XR_StopRequested())) {
        b2_report_input_boundary();
        Com_Printf("B2WALK: input/session/settings boundary or game movement fault=%s; terminal stop.\n", b2_walk_fault->string);
        b2_stop_at(__func__, __LINE__);
    }
    // Filter after both angle collection and final button/movement assembly.
    // Keep command timing, console commands and window/key processing intact.
    if (b2_client.advancing)
        VectorCopy(b2_client.input_angles, cl.viewangles);
    VectorClear(cl.localmove);
    cl.mousemove[0] = cl.mousemove[1] = 0;
    for (unsigned i = 0; i < 3; ++i)
        cl.cmd.angles[i] = ANGLE2SHORT(cl.viewangles[i]);
    cl.cmd.forwardmove = cl.cmd.sidemove = cl.cmd.upmove = 0;
    cl.cmd.buttons = cl.cmd.impulse = 0;
    if (B2_UISimulationHeld() || b2_client.handoff_position_pending) {
        b2_client.walk_release_required = true; return;
    }
    if (b2_client.walking && !b2_client.stopped) {
        bool w = Key_IsDown('w') != 0, s = Key_IsDown('s') != 0;
        bool a = Key_IsDown('a') != 0, d = Key_IsDown('d') != 0;
        bool jump = b2_client.vertical && Key_IsDown(K_SPACE), duck = b2_client.vertical && Key_IsDown('c');
        bool respawn = b2_client.combat && Key_IsDown('r');
        bool shift = Key_IsDown(K_SHIFT) != 0;
        if (!w && !s && !a && !d && !jump && !duck && !respawn && !shift && (!b2_client.firing || !Key_IsDown(K_MOUSE1)) && (!b2_client.turning ||
            (!Key_IsDown(K_LEFTARROW) && !Key_IsDown(K_RIGHTARROW))))
            b2_client.walk_release_required = false;
        if (b2_controller.requested) {
            if (b2_client.combat && b2_client.combat_state.dead) {
                if (B2_ControllerDeadAttack(Sys_Milliseconds())) cl.cmd.buttons=BUTTON_ATTACK;
                return;
            }
            if (b2_client.lifecycle && cl.frame.ps.pmove.pm_type==PM_FREEZE) {
                if (b2_controller_intermission_attack(Sys_Milliseconds())) cl.cmd.buttons=BUTTON_ATTACK;
                return;
            }
            if (b2_tracked_weapon && b2_tracked_weapon->integer &&
                !B2W_Ready(Sys_Milliseconds()) && !b2w_move_only_ready(Sys_Milliseconds()) &&
                !b2w_input_move_only556(Sys_Milliseconds())) return;
            if (b2_client.walk_ready && !b2_client.walk_release_required && b2_controller.valid &&
                !b2_controller.wait_neutral && b2_controller.destination==B2C_GAME &&
                b2c_sample_valid(&b2_controller_sample,Sys_Milliseconds()) && B2XR_InteractionReady() &&
                !(b2_client.combat && b2_client.combat_state.dead) && cl.frame.ps.pmove.pm_type!=PM_FREEZE) {
                float forward=b2_controller.intent.forward,side=b2_controller.intent.side;
                if (b2_tracked_weapon && b2_tracked_weapon->integer && b2w_move_latched) {
                    cl.cmd.forwardmove=forward*b2w_move_cos+side*b2w_move_sin;
                    cl.cmd.sidemove=side*b2w_move_cos-forward*b2w_move_sin;
                } else {
                    cl.cmd.forwardmove=forward;cl.cmd.sidemove=side;
                }
                cl.cmd.upmove=b2_client.vertical ? b2_controller.intent.up : 0;
                cl.localmove[0]=cl.cmd.forwardmove; cl.localmove[1]=cl.cmd.sidemove; cl.localmove[2]=cl.cmd.upmove;
                if (b2_client.firing && (!(b2_tracked_weapon && b2_tracked_weapon->integer) || !b2w_actions_suppressed) && b2_controller.intent.attack &&
                    b2c_aim_usable556(&b2_controller_sample,b2_controller_dominant->integer,Sys_Milliseconds())) cl.cmd.buttons=BUTTON_ATTACK;
                if(b2tp_enabled() && b2tp_use && !b2w_actions_suppressed && !b2w_selector.open && !b2w_selector.rearm && !b2w_pending_token)cl.cmd.buttons|=BUTTON_USE;
            }
            return;
        }
        if (b2_client.walk_ready && !b2_client.walk_release_required) {
            if (b2_client.lifecycle && cl.frame.ps.pmove.pm_type==PM_FREEZE) {
                if (Key_IsDown(K_MOUSE1)) cl.cmd.buttons=BUTTON_ATTACK;
                return;
            }
            if (b2_client.combat && b2_client.combat_state.dead) {
                if (b2_client.lifecycle && Key_IsDown(K_MOUSE1) && !respawn && !w && !s && !a && !d) { cl.cmd.buttons=BUTTON_ATTACK; return; }
                if (respawn && !w && !s && !a && !d && !jump && !duck && !Key_IsDown(K_MOUSE1) &&
                    !Key_IsDown(K_LEFTARROW) && !Key_IsDown(K_RIGHTARROW)) {
                    cl.cmd.buttons = BUTTON_USE;
                    b2_client.respawn_sent = true;
                }
                return;
            }
            float forward = (int)w - (int)s, side = (int)d - (int)a;
            cvar_t *run = Cvar_FindVar("cl_run");
            float speed = ((run && run->integer != 0) ^ shift) ? 400.f : 200.f;
            if (forward && side) speed /= sqrtf(2.f);
            // Quantize once so prediction and the network receive identical
            // input. Stock cl_run/Shift selects run or walk; mouse aims only.
            cl.cmd.forwardmove = (short)(forward * speed);
            cl.cmd.sidemove = (short)(side * speed);
            cl.localmove[0] = cl.cmd.forwardmove;
            cl.localmove[1] = cl.cmd.sidemove;
            cl.cmd.upmove = 200 * ((int)jump - (int)duck); // opposed physical keys cancel
            cl.localmove[2] = cl.cmd.upmove;
            if (b2_client.firing && Key_IsDown(K_MOUSE1))
                cl.cmd.buttons = BUTTON_ATTACK; // only the physical left mouse button
        }
    }
}
// At most 128 transition records per process, never more than four per second.
// Observes predicates and the final filtered command; no game/pose publication.
static void b2_realign_stance391(void)
{
    if (b2_client.recenter_crouch_held) {
        bool held=false;
        bool valid=b2_controller.requested && !b2_client.stopped &&
            !b2_client.world_waiting && !b2_client.world_rebinding &&
            !b2_client.combat_state.dead && cl.frame.ps.pmove.pm_type==PM_NORMAL &&
            b2_recenter_crouch484(&held);
        if (!valid || !held) b2_client.recenter_crouch_held=false;
        else if (!B2_UISimulationHeld() && B2_InputFocused556() &&
            cls.key_dest==KEY_GAME && b2_client.walk_ready) {
            // Keep only the already-held crouch; other input still needs neutral.
            cl.cmd.upmove=-200;cl.localmove[2]=-200;
        }
    }
    if(b2_realign_duck){
        if(b2_realign_duck_world!=b2_client.world_epoch || b2_client.combat_state.dead ||
            cl.frame.ps.pmove.pm_type!=PM_NORMAL)b2_realign_duck=false;
        else if(!B2_UISimulationHeld() && !b2_client.stopped){
            bool usable=B2_InputFocused556() && cls.key_dest==KEY_GAME &&
                !b2_client.walk_release_required && (!b2_controller.requested ||
                (b2_controller.valid && !b2_controller.wait_neutral &&
                 b2c_sample_valid(&b2_controller_sample,Sys_Milliseconds())));
            bool duck=(b2_controller.requested ? b2_controller.intent.up<0 : Key_IsDown('c')!=0) ||
                b2rs_stance.crouched;
            // A fresh standing posture can retire the old recovery-only duck
            // latch without requiring a new crouch press. Never alter PMF_DUCKED;
            // normal Pmove/headroom still owns the authoritative hull.
            b2rs_head_t head;
            bool fresh=usable && b2_client.walk_ready && B2XR_ReadHeadInput(&head) && head.valid &&
                (!b2_controller.requested || b2_controller_sample.xr_generation==head.generation);
            bool standing=fresh && !duck && b2_realign_duck_standing602 &&
                (!(b2_controller.requested && b2_physical_crouch && b2_physical_crouch->integer==1) ||
                 (b2rs_stance.calibrated && B2RS_HeightValid(b2rs_stance.baseline) &&
                  B2RS_HeightValid(head.position[1]) &&
                  head.position[1]>=b2rs_stance.baseline-.25f));
            if(usable && (duck || standing))b2_realign_duck_seen=true;
            if(fresh && !duck && b2_realign_duck_seen)b2_realign_duck=false;
            if(b2_realign_duck){cl.cmd.upmove=-200;cl.localmove[2]=-200;}
        }
    }
}

void B2_FilterInput(void)
{
    b2_filter_input();
    b2rs_filter();
    b2_realign_stance391();
    if (!b2_controller.requested || !b2_tracked_weapon || b2_tracked_weapon->integer!=1) return;
    uint32_t now=Sys_Milliseconds();
    unsigned mask=(b2_client.world_waiting?1u:0)|(b2_client.world_rebinding?2u:0)|
        (b2_client.stopped?4u:0)|(B2_UISimulationHeld()?8u:0)|
        (b2_client.handoff_position_pending?16u:0)|(!b2_client.walk_ready?32u:0)|
        (b2_client.walk_release_required?64u:0)|(!b2_controller.valid?128u:0)|
        (b2_controller.wait_neutral?256u:0)|(!b2c_sample_valid(&b2_controller_sample,now)?512u:0)|
        (!B2XR_InteractionReady()?1024u:0)|(b2_client.combat_state.dead?2048u:0)|
        (b2_controller.destination!=B2C_GAME?4096u:0)|(!B2W_Ready(now)?8192u:0)|
        (b2w_blocked_preview.active?16384u:0)|(b2w_actions_suppressed?32768u:0)|
        (b2_client.recenter_pending?65536u:0)|(cls.key_dest!=KEY_GAME?131072u:0)|
        (!B2_InputFocused556()?262144u:0)|(cl.frame.ps.pmove.pm_type!=PM_NORMAL?524288u:0);
    static unsigned previous=UINT_MAX, records;static int refusal=-1;static uint32_t logged;
    if (records>=128 || (records && (uint32_t)(now-logged)<250) ||
        (mask==previous && refusal==b2w_last_refusal)) return;
    previous=mask;refusal=b2w_last_refusal;logged=now;records++;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_WEAPON_STATE mask=%u refusal_line=%d serial=%llu age=%u/%u blocked_serial=%llu waiting=%d move=%d,%d,%d intent=%d,%d attack=%u model=%u frame=%d record=%u/128\n",
        mask,refusal,(unsigned long long)b2_controller_sample.serial,
        now-b2_controller_sample.capture_ms,now-b2_controller_sample.head_ms,
        (unsigned long long)b2w_blocked_preview.pose.serial,b2w_waiting_input,
        cl.cmd.forwardmove,cl.cmd.sidemove,cl.cmd.upmove,
        b2_controller.intent.forward,b2_controller.intent.side,(unsigned)cl.cmd.buttons,
        (unsigned)(cl.frame.ps.gunindex&GUNINDEX_MASK),cl.frame.ps.gunframe,records);
}


static uint32_t b2_scene_entities_hash(const refdef_t *fd)
{
    // Audit sampled entity data separately from scene time; never a history key.
    const byte *data = (const byte *)fd->entities;
    size_t bytes = sizeof(*fd->entities) * (size_t)fd->num_entities;
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < bytes; ++i)
        hash = (hash ^ data[i]) * 16777619u;
    return hash;
}

static bool b2_particles_valid(const refdef_t *fd)
{
    if (fd->num_particles < 0 || fd->num_particles > MAX_PARTICLES ||
        (fd->num_particles && !fd->particles)) return false;
    for (int i = 0; i < fd->num_particles; i++) {
        const particle_t *p = fd->particles + i;
        if (!isfinite(p->origin[0]) || !isfinite(p->origin[1]) || !isfinite(p->origin[2]) ||
            !isfinite(p->alpha) || p->alpha < 0.f || p->alpha > 1.f ||
            !isfinite(p->brightness) || p->brightness < 0.f ||
            !isfinite(p->radius) || p->radius < 0.f) return false;
    }
    return true;
}

static uint32_t b2_scene_particles_hash(const refdef_t *fd)
{
    // Snapshot audit only: particles keep their stock lifetime and no history ID.
    const byte *data = (const byte *)fd->particles;
    size_t bytes = sizeof(*fd->particles) * (size_t)fd->num_particles;
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < bytes; i++) hash = (hash ^ data[i]) * 16777619u;
    return hash;
}

static void b2_log_player_anchor(const char *phase)
{
    const pmove_state_t *expected = &b2_client.anchor_player;
    const pmove_state_t *actual = &cl.frame.ps.pmove;
    // One complete record, before stopping or changing pause/runtime state.
    // pmove origin/velocity are raw eighth-unit integers; angles are shorts.
    { if ((strcmp(phase,"armed") || Cvar_VariableInteger("developer") > 0)) Com_Printf("B2WORLD_ANCHOR phase=%s scene=%u servercount=%d "
        "expected_frame=%d expected_clienttime=%d expected_servertime=%d "
        "actual_frame=%d actual_clienttime=%d actual_servertime=%d "
        "expected_origin=%d,%d,%d actual_origin=%d,%d,%d "
        "expected_velocity=%d,%d,%d actual_velocity=%d,%d,%d "
        "expected_delta=%d,%d,%d actual_delta=%d,%d,%d "
        "expected_type=%d actual_type=%d expected_health=%d actual_health=%d "
        "expected_gun=%d actual_gun=%d expected_flags=%u actual_flags=%u "
        "expected_pmtime=%u actual_pmtime=%u expected_gravity=%d actual_gravity=%d "
        "changed_origin=%d changed_delta=%d changed_type=%d changed_health=%d changed_gun=%d "
        "cmd_angles=%d,%d,%d cmd_move=%d,%d,%d cmd_buttons=%u cmd_impulse=%u cmd_msec=%u\n",
        phase, b2_client.scene_serial, cl.servercount,
        b2_client.anchor_frame, b2_client.anchor_client_time, b2_client.anchor_server_time,
        cl.frame.number, cl.time, cl.servertime,
        expected->origin[0], expected->origin[1], expected->origin[2],
        actual->origin[0], actual->origin[1], actual->origin[2],
        expected->velocity[0], expected->velocity[1], expected->velocity[2],
        actual->velocity[0], actual->velocity[1], actual->velocity[2],
        expected->delta_angles[0], expected->delta_angles[1], expected->delta_angles[2],
        actual->delta_angles[0], actual->delta_angles[1], actual->delta_angles[2],
        (int)expected->pm_type, (int)actual->pm_type,
        b2_client.anchor_health, cl.frame.ps.stats[STAT_HEALTH],
        b2_client.anchor_gunindex, cl.frame.ps.gunindex,
        (unsigned)expected->pm_flags, (unsigned)actual->pm_flags,
        (unsigned)expected->pm_time, (unsigned)actual->pm_time,
        expected->gravity, actual->gravity,
        memcmp(expected->origin, actual->origin, sizeof(expected->origin)) != 0,
        memcmp(expected->delta_angles, actual->delta_angles, sizeof(expected->delta_angles)) != 0,
        expected->pm_type != actual->pm_type,
        b2_client.anchor_health != cl.frame.ps.stats[STAT_HEALTH],
        b2_client.anchor_gunindex != cl.frame.ps.gunindex,
        cl.cmd.angles[0], cl.cmd.angles[1], cl.cmd.angles[2],
        cl.cmd.forwardmove, cl.cmd.sidemove, cl.cmd.upmove,
        (unsigned)cl.cmd.buttons, (unsigned)cl.cmd.impulse, (unsigned)cl.cmd.msec); }
}

static void b2_stop_f(void) { B2XR_RequestStop(); b2_stop_at("operator-command", 0); }

static void b2_stop_at(const char *source, int line)
{
    b2aw_abort();
    R_B2UIReset();
    if (b2_client.stopped) return;
    if (!B2XR_Requested() && !b2_client.pending && !b2_client.frozen && !b2_client.stopped)
        return;
    if (!CL_B2ShuttingDown457() || Cvar_VariableInteger("developer") > 0) Com_Printf("B2_STOP first source=%s line=%d waiting=%d rebinding=%d epoch=%u servercount=%d retired=%d frame=%d publication=%d xr_stop=%d\n",
        source,line,b2_client.world_waiting,b2_client.world_rebinding,b2_client.world_epoch,
        cl.servercount,b2_client.retired_servercount,cl.frame.number,cl.frame.ps.stats[31],B2XR_StopRequested());
    if (B2XR_Requested() && !B2XR_QuitOnly())
        B2XR_RequestStop();
    b2_client.pending = false;
    b2_client.stopped = true;
    CL_B2ReleaseButtons(); UI_B2VRRepeatCancel(); b2_menu_cancel253();
    b2_client.handoff_position_pending = false;
    if (b2_client.walking) {
        b2_client.walk_ready = false;
        b2_clear_walk_input();
    }
    cl.refdef.b2_active = false;
    cl.refdef.b2_frame = NULL;
    if (cls.state == ca_active) {
        Cvar_SetByVar(cl_paused, "2", FROM_CODE);
        CL_CheckForPause();
        if (!cls.demo.playback && cls.serverAddress.type == NA_LOOPBACK &&
            sv_running->integer && cl.maxclients == 1)
            Cvar_Set("sv_paused", "1");
    }
    // Temporal descriptors remain owned by the B2 renderer until shutdown.
    // Keep the scene and prevent an ordinary world render from reusing them.
    if (!CL_B2ShuttingDown457() || Cvar_VariableInteger("developer") > 0) Com_Printf("B2: session cannot safely resume. Close the game window to quit cleanly, restart Q2RTX VR and load a saved game. Existing saves are retained; unsaved progress is not recovered. Focus/Pause cannot override this stop.\n");
}

// One bounded report per explicit activation command. This observes the same
// inputs as the guards below; it must never replace or relax those decisions.
// Normal menu/lifecycle entry may restore ordinary saved motion/stance.
// The diagnostic/manual freeze route still requires a settled player.
static bool b2_normal_start(void)
{
    return b2_headset_bootstrapping && b2_menu_transition &&
        b2_lifecycle->integer == 1 && !b2_client.ui_manual_pause &&
        !b2_client.stopped && !B2XR_StopRequested() &&
        B2_InputFocused556() && B2XR_InteractionReady() &&
        b2_menu_interruption == B2XR_InterruptionEpoch();
}

static void b2_report_activation(void)
{
    cvar_t *projection = Cvar_FindVar("pt_projection");
    cvar_t *geometry = Cvar_FindVar("vid_geometry");
    cvar_t *protection = Cvar_FindVar("b2_observer_protect");
    bool requested = B2XR_Requested(), b1 = false, b0u = false;
    bool local_single = cls.serverAddress.type == NA_LOOPBACK &&
        sv_running->integer && cl.maxclients == 1;
    bool advancing = b2_advance_world->integer == 1;
    bool walking = b2_keyboard_locomotion->integer == 1;
    bool turning = b2_turning_mouselook->integer == 1;
    bool vertical = b2_jump_crouch->integer == 1;
    bool firing = b2_blaster_fire->integer == 1, target = b2_visual_target->integer == 1;
    char failures[2048] = "", later[128] = "";
#define B2_START_FAIL(name, condition) do { if (condition) Q_strlcat(failures, #name ",", sizeof(failures)); } while (0)
    B2_START_FAIL(b2_not_requested, !requested);
    B2_START_FAIL(not_exclusive, b1 || b0u);
    B2_START_FAIL(extended_local_profile, !cls.demo.playback && B2_FreshRequested() &&
        (!local_single || !B2_FreshClient(&cl.csr, cls.serverProtocol,
            cls.protocolVersion, cl.esFlags, cl.psFlags, cls.serverAddress.type == NA_LOOPBACK)));
    B2_START_FAIL(renderer, cls.ref_type != REF_TYPE_VKPT);
    B2_START_FAIL(client_inactive, cls.state != ca_active);
    B2_START_FAIL(frame_invalid, !cl.frame.valid);
    B2_START_FAIL(not_local_or_demo, !cls.demo.playback && !local_single);
    B2_START_FAIL(timedemo, com_timedemo->integer);
    B2_START_FAIL(demo_seeking, cls.demo.seeking);
    B2_START_FAIL(demo_eof, cls.demo.eof);
    B2_START_FAIL(pending, b2_client.pending);
    B2_START_FAIL(frozen, b2_client.frozen);
    B2_START_FAIL(stopped, b2_client.stopped);
    B2_START_FAIL(projection, !projection || projection->integer != 0);
    B2_START_FAIL(scale, !isfinite(b2_units_per_metre->value) || b2_units_per_metre->value <= 0.f);
    B2_START_FAIL(width, r_config.width != B2G_Width());
    B2_START_FAIL(height, r_config.height != B2G_Height());
    B2_START_FAIL(advance_mode, b2_advance_world->integer != 0 && !advancing);
    B2_START_FAIL(walk_mode, b2_keyboard_locomotion->integer != 0 && !walking);
    B2_START_FAIL(turn_mode, b2_turning_mouselook->integer != 0 && !turning);
    B2_START_FAIL(turn_needs_walk, turning && !walking);
    B2_START_FAIL(vertical_mode, b2_jump_crouch->integer != 0 && !vertical);
    B2_START_FAIL(vertical_needs_walk, vertical && !walking);
    B2_START_FAIL(vertical_flags, vertical && !b2_normal_start() && (cl.frame.ps.pmove.pm_flags &
        (PMF_DUCKED | PMF_JUMP_HELD | PMF_TIME_TELEPORT | PMF_TIME_WATERJUMP)));
    B2_START_FAIL(fire_mode, b2_blaster_fire->integer != 0 && !firing);
    B2_START_FAIL(fire_needs_turn, firing && !turning);
    B2_START_FAIL(target_mode, b2_visual_target->integer != 0 && !target);
    B2_START_FAIL(target_needs_walk, target && !walking);
    B2_START_FAIL(target_fault, b2_target_fault->string[0]);
    B2_START_FAIL(walk_needs_advance, walking && !advancing);
    B2_START_FAIL(observer_protection, walking && (!protection || protection->integer != (b2_combat->integer == 1 ? 0 : 1)));
    B2_START_FAIL(window_inactive, walking && !B2_InputFocused556());
    B2_START_FAIL(key_destination, walking && cls.key_dest != KEY_GAME);
    B2_START_FAIL(walk_fault, walking && b2_walk_fault->string[0]);
    B2_START_FAIL(advance_needs_local, advancing && !local_single);
    B2_START_FAIL(advance_demo, advancing && cls.demo.playback);
    B2_START_FAIL(player_type, advancing && cl.frame.ps.pmove.pm_type != PM_NORMAL);
    B2_START_FAIL(player_health, advancing && cl.frame.ps.stats[STAT_HEALTH] <= 0);
#undef B2_START_FAIL
    if (advancing && !b2_normal_start() && (cl.frame.ps.pmove.velocity[0] || cl.frame.ps.pmove.velocity[1] || cl.frame.ps.pmove.velocity[2]))
        Q_strlcat(later, "stationary_velocity,", sizeof(later));
    if (advancing && turning && (!isfinite(cl.viewangles[0]) || !isfinite(cl.viewangles[1]) || !isfinite(cl.viewangles[2])))
        Q_strlcat(later, "viewangles_finite,", sizeof(later));
    if (!b2_headset_bootstrapping || failures[0] || later[0] || Cvar_VariableInteger("developer") > 0) {
    Com_Printf("B2START failures=%s later_checks=%s\n", failures[0] ? failures : "none", later[0] ? later : "none");
    Com_Printf("B2START geometry=%.96s actual=%dx%d required=%dx%d renderer=%d required_renderer=%d b2=%d b1=%d b0u=%d\n",
        geometry ? geometry->string : "<missing>", r_config.width, r_config.height, B2G_Width(), B2G_Height(),
        cls.ref_type, REF_TYPE_VKPT, requested, b1, b0u);
    Com_Printf("B2START client=%d required_client=%d frame_valid=%d frame=%d address=%d loopback=%d server=%d maxclients=%d local_single=%d demo=%d seeking=%d eof=%d timedemo=%d\n",
        cls.state, ca_active, cl.frame.valid, cl.frame.number, cls.serverAddress.type, NA_LOOPBACK,
        sv_running->integer, cl.maxclients, local_single, cls.demo.playback, cls.demo.seeking, cls.demo.eof, com_timedemo->integer);
    Com_Printf("B2START window=%d required_window=%d key_dest=%d required_key_dest=%d projection_present=%d projection=%d scale=%.9g pending=%d frozen=%d stopped=%d pause=%d server_pause=%d\n",
        cls.active, ACT_ACTIVATED, cls.key_dest, KEY_GAME, projection != NULL, projection ? projection->integer : -1,
        b2_units_per_metre->value, b2_client.pending, b2_client.frozen, b2_client.stopped, cl_paused->integer, sv_paused->integer);
    Com_Printf("B2START advance=%d walk=%d turn=%d vertical=%d fire=%d target=%d protection_present=%d protection=%d walk_fault=%.160s target_fault=%.160s\n",
        b2_advance_world->integer, b2_keyboard_locomotion->integer, b2_turning_mouselook->integer,
        b2_jump_crouch->integer, b2_blaster_fire->integer, b2_visual_target->integer,
        protection != NULL, protection ? protection->integer : -1, b2_walk_fault->string, b2_target_fault->string);
    Com_Printf("B2START player_type=%d required_type=%d health=%d flags=0x%x forbidden_vertical_flags=0x%x velocity=%d,%d,%d viewangles=%.9g,%.9g,%.9g\n",
        cl.frame.ps.pmove.pm_type, PM_NORMAL, cl.frame.ps.stats[STAT_HEALTH], cl.frame.ps.pmove.pm_flags,
        PMF_DUCKED | PMF_JUMP_HELD | PMF_TIME_TELEPORT | PMF_TIME_WATERJUMP,
        cl.frame.ps.pmove.velocity[0], cl.frame.ps.pmove.velocity[1], cl.frame.ps.pmove.velocity[2],
        cl.viewangles[0], cl.viewangles[1], cl.viewangles[2]);
    }
}

static void b2_freeze_f(void)
{
    b2_report_activation();
    cvar_t *projection = Cvar_FindVar("pt_projection");
    bool local_single = cls.serverAddress.type == NA_LOOPBACK &&
        sv_running->integer && cl.maxclients == 1;
    bool advancing = b2_advance_world->integer == 1;
    bool walking = b2_keyboard_locomotion->integer == 1;
    bool turning = b2_turning_mouselook->integer == 1;
    bool vertical = b2_jump_crouch->integer == 1;
    bool firing = b2_blaster_fire->integer == 1, target = b2_visual_target->integer == 1;
    cvar_t *protection = Cvar_FindVar("b2_observer_protect");
    bool combat = b2_combat->integer == 1;
    if (b2_lifecycle->integer != 0 &&
        (b2_lifecycle->integer != 1 || !combat || b2_same_map_ui->integer != 1)) {
        Com_Printf("B2LIFE_ARM requested=%d armed=0 failures=lifecycle_prerequisites\n", b2_lifecycle->integer);
        return;
    }
    if (b2_same_map_ui->integer != 0 && (b2_same_map_ui->integer != 1 || !combat || !local_single)) {
        Com_Printf("B2UI: same-map controls require local single-player combat.\n"); return;
    }
    b2_combat_state_t admitted = {0};
    if ((b2_combat->integer != 0 && !combat) || (combat &&
        (!walking || !turning || !vertical || !firing || !advancing || !protection || protection->integer ||
         (b2_lifecycle->integer != 1 && strcmp(cl.mapname, "base1") && strcmp(cl.mapname, "command") && strcmp(cl.mapname, "mintro") && strcmp(cl.mapname, "fact2")) || !SV_B2CombatState(&cl.frame.ps, &admitted)))) {
        Com_Printf("B2START failures=combat_admission\n"); return;
    }
    if (!B2XR_Requested() || !b2_exclusive() || cls.ref_type != REF_TYPE_VKPT ||
        cls.state != ca_active || !cl.frame.valid ||
        (!cls.demo.playback && !local_single) || com_timedemo->integer ||
        cls.demo.seeking || cls.demo.eof || b2_client.pending || b2_client.frozen || b2_client.stopped ||
        !projection || projection->integer != 0 ||
        !isfinite(b2_units_per_metre->value) || b2_units_per_metre->value <= 0.f ||
        r_config.width != B2G_Width() || r_config.height != B2G_Height() ||
        (b2_advance_world->integer != 0 && !advancing) ||
        (b2_keyboard_locomotion->integer != 0 && !walking) ||
        (b2_turning_mouselook->integer != 0 && !turning) || (turning && !walking) ||
        (b2_jump_crouch->integer != 0 && !vertical) || (vertical && (!walking ||
            (!b2_normal_start() && (cl.frame.ps.pmove.pm_flags & (PMF_DUCKED | PMF_JUMP_HELD | PMF_TIME_TELEPORT | PMF_TIME_WATERJUMP))))) ||
        (b2_blaster_fire->integer != 0 && !firing) || (firing && !turning) ||
        (b2_visual_target->integer != 0 && !target) || (target && !walking) || b2_target_fault->string[0] ||
        (walking && (!advancing || !protection || protection->integer != (combat ? 0 : 1) ||
            (!b2_headset_bootstrapping && (!B2_InputFocused556() || cls.key_dest != KEY_GAME)) || b2_walk_fault->string[0])) ||
        (advancing && (!local_single || cls.demo.playback ||
            cl.frame.ps.pmove.pm_type != PM_NORMAL || cl.frame.ps.stats[STAT_HEALTH] <= 0))) {
        Com_Printf("B2: freeze requires exclusive B2, active local single-player or ordinary demo, "
            "flat projection, fixed 1948x1563 output and a valid scale.\n");
        return;
    }
    if (advancing) {
        if (b2_same_map_ui->integer == 1 && !SCR_B2PrepareUI()) {
            Com_Printf("B2UI: startup interface resources unavailable.\n"); return;
        }
        // Preserve a normal saved player state. Manual/diagnostic admission
        // still requires a settled player; neither route resets saved physics.
        for (unsigned i = 0; i < 3; ++i) {
            if (!b2_normal_start() && cl.frame.ps.pmove.velocity[i]) {
                Com_Printf("B2WORLD: player must be stationary before arming.\n");
                return;
            }
        }
        b2_client.headset_waiting = b2_headset_bootstrapping;
        b2_client.ui_manual_pause = b2_headset_bootstrapping &&
            !b2_menu_transition && cl_paused->integer == 2;
        b2_client.ui_interrupted = b2_headset_bootstrapping;
        if (b2_menu_transition) b2_client.ui_xr_epoch=b2_menu_interruption;
        b2_client.advancing = true;
        b2_client.ui_enabled = b2_same_map_ui->integer == 1;
        b2_client.ui_destination = cls.key_dest;
        b2_client.walking = walking;
        b2_client.turning = turning;
        b2_client.firing = firing; b2_client.target = target; b2_client.vertical = vertical;
        b2_client.walk_release_required = true;
        b2_client.walk_previous = cl.frame.ps.pmove;
        b2_client.walk_previous_time = cl.servertime;
        b2_client.anchor_player = cl.frame.ps.pmove;
        b2_client.combat = combat;
        b2_client.lifecycle = b2_lifecycle->integer == 1 && combat && b2_same_map_ui->integer == 1;
        b2_client.combat_state = admitted;
        b2_client.servercount = cl.servercount; // owner exists before prediction/first scene
        b2_client.anchor_health = cl.frame.ps.stats[STAT_HEALTH];
        b2_client.anchor_gunindex = cl.frame.ps.gunindex;
        b2_client.anchor_frame = cl.frame.number;
        b2_client.anchor_client_time = cl.time;
        b2_client.anchor_server_time = cl.servertime;
        VectorCopy(cl.viewangles, b2_client.input_angles);
        if (turning) {
            for (unsigned i = 0; i < 3; ++i) {
                if (!isfinite(cl.viewangles[i])) { b2_stop_at(__func__, __LINE__); return; }
            }
            float dp = SHORT2ANGLE(b2_client.anchor_player.delta_angles[PITCH]);
            b2_client.input_angles[PITCH] = Q_clipf(remainderf(cl.viewangles[PITCH] + dp, 360.f), -89.f, 89.f) - dp;
            b2_client.input_angles[YAW] = remainderf(cl.viewangles[YAW], 360.f);
            b2_client.input_angles[ROLL] = -SHORT2ANGLE(b2_client.anchor_player.delta_angles[ROLL]);
            VectorCopy(b2_client.input_angles, cl.viewangles);
            CL_B2DiscardMouse();
        }
        // Zero denotes no owned world. The initial admitted session needs an
        // identity just like subsequent lifecycle binds, before tracked input.
        if (!b2_client.world_epoch) b2_client.world_epoch = 1;
        b2_client.pending = true;
        B2_FilterInput();
        b2_log_player_anchor("armed");
        { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2LIFE_ARM requested=%d armed=%d combat=%d same_map_ui=%d\n",
            b2_lifecycle->integer, b2_client.lifecycle, combat, b2_same_map_ui->integer); }
        if (b2_client.headset_waiting) {
            b2_ui_apply_pause(); // Drain/publish once, then hold both clocks.
            { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_HEADSET_WAIT paused=1 owner=none; release controls, press and release off-hand top button in the play space.\n"); }
        } else {
            Cvar_SetByVar(cl_paused, "0", FROM_CODE);
            CL_CheckForPause();
            Cvar_Set("sv_paused", "0");
        }
        b2_units_per_metre->modified = false;
        { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2WORLD: armed local advancing simulation, fixed input/player anchor, stock particles sampled once per scene.\n"); }
        if (vertical) { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2VERTICAL: physical Space jump, hold C crouch, opposed cancel; release before activation; ordinary Pmove/headroom.\n"); }
        if (firing) {
            if (combat) { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2FIRE: physical left mouse blaster; combat damage/knockback/death enabled. Release all controls after death, then press R after one second for a clear same-area return; release again to move/fire. Body heading stays level, live HMD tracking retained.\n"); }
            else { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2FIRE: opt-in physical left mouse blaster, release before activation; normal weapon/projectile effects, observer protected.\n"); }
        }
        if (turning) { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2LOOK: arrows 30 deg/s, raw mouse yaw/pitch using sensitivity*m_yaw/m_pitch, non-inverted, pitch +/-89; live head pose relative to body.\n"); }
        else if (walking) Com_Printf("B2WALK: opt-in W/S forward/back A/D strafe, fixed launch heading; "
            "200-unit walk input, normalized diagonal, no mouse/run/jump/fire. Release all keys; first committed XR pair enables input.\n");
        return;
    }
    b2_client.prior_pause = cl_paused->integer;
    Cvar_SetByVar(cl_paused, "2", FROM_CODE);
    CL_CheckForPause();
    // Local single-player's server pause check observes cl_paused on its next
    // tick. Stop client interpolation immediately at this same command boundary.
    if (local_single && !cls.demo.playback)
        Cvar_Set("sv_paused", "1");
    if (!sv_paused->integer) {
        Cvar_SetByVar(cl_paused, va("%d", b2_client.prior_pause), FROM_CODE);
        CL_CheckForPause();
        Com_Printf("B2: simulation could not be paused; freeze rejected.\n");
        return;
    }
    b2_client.pending = true;
    b2_units_per_metre->modified = false;
    Com_Printf("B2: freeze armed for one natural scene including stock particles, no locomotion or gameplay.\n");
}


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
// Stereo view geometry adapted from the earlier project src/client/view.c.
// Original Id Software / NVIDIA GPL-2.0-or-later notices: reference provenance.
typedef struct { vec3_t origin, forward, right, up; } b2_reticle_aim_t;

static bool b2_reticle_vector(const float v[3])
{
    return isfinite(v[0]) && isfinite(v[1]) && isfinite(v[2]);
}

static bool b2_reticle_aim(const refdef_t *fd, const b2wv_visual_t *visual, const b2_view_t *view,
    b2_reticle_aim_t *out)
{
    if (!fd || !out || !b2_world_reticle || b2_world_reticle->integer!=1 ||
        !B2_AdvancingWorld() || !b2_client.combat || !b2_client.origin_latched ||
        !b2_client.walk_ready || b2_client.walk_release_required ||
        b2_client.stopped || b2_client.world_waiting || b2_client.world_rebinding ||
        b2_client.recenter_pending || b2_client.handoff_position_pending ||
        b2_client.ui_interrupted || b2_client.ui_manual_pause || B2_UISimulationHeld() ||
        cls.state!=ca_active || !B2_InputFocused556() || cls.key_dest!=KEY_GAME ||
        !cl.frame.valid || cl.frame.ps.pmove.pm_type!=PM_NORMAL ||
        cl.frame.ps.stats[STAT_HEALTH]<=0 || cl.thirdPersonView ||
        (cl.frame.ps.stats[STAT_LAYOUTS]&(LAYOUTS_HIDE_HUD|LAYOUTS_HIDE_CROSSHAIR)) ||
        !B2XR_InteractionReady()) return false;
    memset(out,0,sizeof(*out));
    const entity_t *gun=NULL;
    if (fd->num_entities<0 || fd->num_entities>MAX_ENTITIES ||
        (fd->num_entities && !fd->entities)) return false;
    for (int i=0;i<fd->num_entities;i++)
        if (fd->entities[i].flags&RF_WEAPONMODEL) { gun=&fd->entities[i];break; }
    if (!gun) return false;
    model_t *model=MOD_ForHandle(gun->model);
    b2wm_info_t info;b2wm_datum_t datum;float matrix[16]={0};int hand=1;
    if (!B2WM_Query(model,B2WM_VERSION,sizeof(info),&info) ||
        !B2WM_Datum(model,&info.key,gun->frame,gun->oldframe,gun->backlerp,&datum)) return false;
    if (!b2_controller.requested && (info.key.slot==6 || info.key.slot==11)) {
        // Shared cyclopean view, prepared once before either stereo/quad eye.
        // This visual ray never becomes a gameplay aim or a weapon transform.
        if (!view || !b2_reticle_vector(view->origin) || !b2_reticle_vector(view->angles)) return false;
        VectorCopy(view->origin,out->origin);
        AngleVectors(view->angles,out->forward,out->right,out->up);
        return b2_reticle_vector(out->forward) && b2_reticle_vector(out->right) && b2_reticle_vector(out->up);
    }
    if (b2_controller.requested) {
        if (b2w_actions_suppressed || b2w_selector.open || b2w_selector.rearm ||
            b2w_pending_token || !visual || visual->handle!=(uint32_t)gun->model ||
            visual->model!=model || visual->frame!=gun->frame ||
            visual->oldframe!=gun->oldframe || visual->backlerp!=gun->backlerp) return false;
        memcpy(matrix,visual->matrix,sizeof(matrix));
        hand=visual->owner.source.aim.hand==0?-1:1;
    } else {
        // CL_AddViewWeapon already included the enabled view kick and the
        // interpolated gunangles. Use that displayed basis once, not the
        // scene's overwritten body angles or a second addition of recoil.
        if (!b2_reticle_vector(gun->angles) || !b2_reticle_vector(gun->origin) ||
            !b2_reticle_vector(gun->oldorigin) || !isfinite(gun->scale) ||
            !isfinite(gun->backlerp) || gun->backlerp<0 || gun->backlerp>1) return false;
        vec3_t axes[3];AnglesToAxis(gun->angles,axes);
        float scale=gun->scale>0?gun->scale:1;
        hand=info_hand && info_hand->integer==1?-1:1;
        // Same physical B2 entity convention as internal_create_entity_matrix:
        // forward/left/up columns, optional left-hand mirror, no gun-FOV warp.
        for (unsigned row=0;row<3;row++) {
            for (unsigned col=0;col<3;col++)
                matrix[col*4+row]=axes[col][row]*scale*(col==1?hand:1);
            matrix[12+row]=(1-gun->backlerp)*gun->origin[row]+gun->backlerp*gun->oldorigin[row];
        }
        matrix[15]=1;
        if (info.key.slot==8 && !B2WM_HyperStabilise(model,&info.key,
                gun->frame,gun->oldframe,gun->backlerp,false,matrix)) return false;
    }
    // The current interpolated model datum follows the displayed barrel,
    // including animation. Origin is its actual visual muzzle (with parallax),
    // not a new ballistic preview. The private game contract samples the same
    // model transform at its discrete emission state; rendering never authorises it.
    for (unsigned row=0;row<3;row++) {
        out->origin[row]=matrix[12+row];
        for (unsigned col=0;col<3;col++) {
            out->origin[row]+=(float)datum.muzzle[col]*matrix[col*4+row];
            out->forward[row]+=(float)datum.basis[0][col]*matrix[col*4+row];
            out->right[row]-=(float)datum.basis[1][col]*matrix[col*4+row]*hand;
            out->up[row]+=(float)datum.basis[2][col]*matrix[col*4+row];
        }
    }
    if (VectorNormalize(out->forward)<=0 || VectorNormalize(out->right)<=0 ||
        VectorNormalize(out->up)<=0) return false;
    if (!b2_reticle_vector(out->origin) || !b2_reticle_vector(out->forward) ||
        !b2_reticle_vector(out->right) || !b2_reticle_vector(out->up)) return false;
    return fabsf(DotProduct(out->forward,out->forward)-1.f)<.02f &&
        fabsf(DotProduct(out->right,out->right)-1.f)<.02f &&
        fabsf(DotProduct(out->up,out->up)-1.f)<.02f &&
        fabsf(DotProduct(out->forward,out->right))<.02f &&
        fabsf(DotProduct(out->forward,out->up))<.02f &&
        fabsf(DotProduct(out->right,out->up))<.02f;
}

// Visual water placement only; an opaque/entity hit caps every ray.
// At most three CL_Trace calls per ray: centre plus 24 arm endpoints = 75.
// No per-frame cache. Reverse tracing supports an ordinary single water span;
// disconnected pools / intervening air gaps are not a first-exit guarantee.
static bool b2_reticle_trace(const vec3_t origin, const vec3_t end, trace_t *trace)
{
    if (!b2_reticle_vector(origin) || !b2_reticle_vector(end)) return false;
    CL_Trace(trace,origin,vec3_origin,vec3_origin,end,MASK_SHOT);
    if (trace->startsolid || trace->allsolid || !isfinite(trace->fraction) ||
        trace->fraction<0.f || trace->fraction>1.f || !b2_reticle_vector(trace->endpos)) return false;
    trace_t water;
    CL_Trace(&water,origin,vec3_origin,vec3_origin,end,CONTENTS_WATER);
    if (!water.startsolid && !water.allsolid) {
        if (isfinite(water.fraction) && water.fraction>=0.f && water.fraction<trace->fraction)
            *trace=water;
    } else {
        // The existing collision interface reports entry, not exit, from a
        // wet origin. Reverse from the nearest opaque hit (or ray end). A dry
        // endpoint gives the ordinary exit plane; a wet endpoint falls back
        // to the opaque result. Never search beyond that result.
        CL_Trace(&water,trace->endpos,vec3_origin,vec3_origin,origin,CONTENTS_WATER);
        if (!water.startsolid && !water.allsolid && isfinite(water.fraction) &&
            water.fraction>=0.f && water.fraction<1.f && b2_reticle_vector(water.endpos)) {
            float fraction=trace->fraction*(1.f-water.fraction);
            if (fraction>=0.f && fraction<trace->fraction) {
                water.fraction=fraction;
                VectorNegate(water.plane.normal,water.plane.normal);
                *trace=water;
            }
        }
    }
    return trace->fraction<1.f && b2_reticle_vector(trace->endpos);
}

static bool b2_reticle_project(const vec3_t origin, const vec3_t centre,
    const vec3_t axis, float offset, float max_span, vec3_t projected)
{
    vec3_t target,direction,far_point;
    trace_t trace;
    VectorMA(centre,offset,axis,target);
    VectorSubtract(target,origin,direction);
    float length=VectorLength(direction);
    if (!isfinite(length) || length<=.000001f) return false;
    VectorScale(direction,1.f/length,direction);
    VectorMA(origin,8192.f,direction,far_point);
    if (!b2_reticle_trace(origin,far_point,&trace) ||
        Distance(trace.endpos,centre)>max_span) return false;
    if (trace.contents&CONTENTS_WATER)
        VectorMA(trace.endpos,.35f,trace.plane.normal,projected);
    else VectorMA(trace.endpos,-.35f,direction,projected);
    return b2_reticle_vector(projected);
}

static int b2_reticle_geometry(const b2_reticle_aim_t *aim, float scale, entity_t beams[12])
{
    vec3_t end,centre;
    trace_t trace;
    if (!aim || !beams || !isfinite(scale)) return 0;
    scale=Q_clipf(scale,.1f,4.f);
    VectorMA(aim->origin,8192.f,aim->forward,end);
    if (!b2_reticle_trace(aim->origin,end,&trace)) return 0;
    VectorCopy(trace.endpos,centre);
    float raw_distance=Distance(aim->origin,centre);
    if (!isfinite(raw_distance) || raw_distance<=.000001f) return 0;
    float distance=max(4.f,raw_distance);
    float near_scale=min(1.f,distance/256.f);
    int diameter=max(1,min(3,(int)floorf(3.f*near_scale+.5f)));
    float alpha=.95f*min(1.f,3.f*near_scale/(float)diameter);
    float outer=min(4.f,tanf(DEG2RAD(.75f*scale))*distance),inner=outer*.80f;
    float max_surface=max(2.f,outer*4.f),max_segment=max(.75f,((outer-inner)/3.f)*6.f);
    const float *axes[4]={aim->right,aim->right,aim->up,aim->up};
    const float signs[4]={1.f,-1.f,1.f,-1.f};
    int count=0;
    for (int arm=0;arm<4;arm++) for (int segment=0;segment<3;segment++) {
        vec3_t start,finish;
        float offset0=signs[arm]*(inner+(outer-inner)*(float)segment/3.f);
        float offset1=signs[arm]*(inner+(outer-inner)*(float)(segment+1)/3.f);
        if (!b2_reticle_project(aim->origin,centre,axes[arm],offset0,max_surface,start) ||
            !b2_reticle_project(aim->origin,centre,axes[arm],offset1,max_surface,finish)) continue;
        float span=Distance(start,finish);
        if (!isfinite(span) || span<=.000001f || span>max_segment) continue;
        entity_t *beam=&beams[count++];
        memset(beam,0,sizeof(*beam));
        beam->flags=RF_TRANSLUCENT|RF_BEAM|B2_RF_WORLD_RETICLE;
        beam->skinnum=-1;
        for (unsigned i=0;i<4;i++) beam->rgba.u8[i]=255;
        beam->alpha=alpha;beam->frame=diameter;
        VectorCopy(start,beam->origin);VectorCopy(finish,beam->oldorigin);
    }
    return count;
}

// Retire only the appended tail at a shared pre-pair boundary. No world
// advance or per-eye mutation; ordinary scene entities remain byte-identical.
static void b2_reticle_retire(void)
{
    if (!b2_client.reticle_count) return;
    b2_reticle_aim_t current;
    if (b2_reticle_aim(&b2_client.scene,NULL,NULL,&current) &&
        !memcmp(&current,b2_client.reticle_aim,sizeof(current))) return;
    b2_client.scene.num_entities-=b2_client.reticle_count;
    b2_client.reticle_count=0;
}

// Only the reserved tail changes. Ordinary entities and teleport keep their order.
static void b2_reticle_clear(void)
{
    b2_client.scene.num_entities=b2_client.entity_base+b2_client.teleport_count;
    b2_client.reticle_count=0;
}

static void b2_reticle_prepare(const b2_frame_t *frame, const b2_view_t *head)
{
    if (!b2_client.entities || b2_client.entity_base<0 || b2_client.entity_base>MAX_ENTITIES) return;
    if (!b2_controller.requested) {
        // All other KBM weapons retain their original cached barrel reticle.
        bool centred=false;
        for (int i=0;i<b2_client.entity_base;i++) if (b2_client.entities[i].flags&RF_WEAPONMODEL) {
            b2wm_info_t info;
            centred=B2WM_Query(MOD_ForHandle(b2_client.entities[i].model),B2WM_VERSION,sizeof(info),&info) &&
                (info.key.slot==6 || info.key.slot==11);
            break;
        }
        if (!centred) return;
    }
    b2_reticle_clear();b2_client.reticle_tracked=true;
    int first=b2_client.entity_base+b2_client.teleport_count;
    if (!frame || !frame->should_render || !frame->tracking_valid ||
        frame->predicted_time_ns<=0 || first<0 || first>MAX_ENTITIES-12 ||
        b2_client.entity_capacity<first+12 || !b2_client.entities) return;
    const entity_t *gun=NULL;
    for (int i=0;i<b2_client.entity_base;i++)
        if (b2_client.entities[i].flags&RF_WEAPONMODEL) { gun=&b2_client.entities[i];break; }
    if (!gun) return;
    b2wv_visual_t visual;b2_reticle_aim_t aim;
    const b2wv_visual_t *current=NULL;
    if (b2_controller.requested) {
        if (!B2WP_CopyVisual((uint32_t)gun->model,MOD_ForHandle(gun->model),
            gun->frame,gun->oldframe,gun->backlerp,frame->predicted_time_ns,
            Sys_Milliseconds(),&visual)) return;
        current=&visual;
    }
    if (!b2_reticle_aim(&b2_client.scene,current,head,&aim)) return;
    b2_client.reticle_count=b2_reticle_geometry(&aim,b2_world_reticle_scale->value,
        b2_client.entities+first);
    b2_client.scene.num_entities=first+b2_client.reticle_count;
    memcpy(b2_client.reticle_aim,&aim,sizeof(aim));
}

bool B2_ObserveScene(const refdef_t *fd)
{
    if (b2_client.world_waiting) { b2_world_wait_valid(); return false; }
    bool initial = b2_client.pending;
    if (!initial && !B2_AdvancingWorld())
        return !b2_client.stopped;
    if (!fd || b2_client.history.frame_open || !cl.frame.valid || cls.state != ca_active ||
        fd->rdflags & RDF_NOWORLDMODEL || !fd->lightstyles ||
        fd->num_entities < 0 || fd->num_entities > MAX_ENTITIES ||
        fd->num_dlights < 0 || fd->num_dlights > MAX_DLIGHTS ||
        !b2_particles_valid(fd) ||
        cl.frame.areabytes < 0 || cl.frame.areabytes > MAX_MAP_AREA_BYTES ||
        (fd->num_entities && !fd->entities) || (fd->num_dlights && !fd->dlights) ||
        (cl.frame.areabytes && !fd->areabits)) {
        b2_stop_at(__func__, __LINE__);
        Com_Printf("B2: natural scene unavailable or malformed; freeze rejected.\n");
        return false;
    }
    if (!B2_CombatSnapshot()) return false;
    if (b2_client.advancing && !b2_client.combat &&
        ((!b2_client.walking && memcmp(cl.frame.ps.pmove.origin, b2_client.anchor_player.origin,
            sizeof(b2_client.anchor_player.origin))) ||
         memcmp(cl.frame.ps.pmove.delta_angles, b2_client.anchor_player.delta_angles,
            sizeof(b2_client.anchor_player.delta_angles)) ||
         cl.frame.ps.pmove.pm_type != b2_client.anchor_player.pm_type ||
         cl.frame.ps.stats[STAT_HEALTH] != b2_client.anchor_health ||
         cl.frame.ps.gunindex != b2_client.anchor_gunindex)) {
        b2_log_player_anchor("guard-stop");
        Com_Printf("B2WORLD: player position/health/state/weapon changed; terminal stop, gameplay interaction requires review.\n");
        b2_stop_at(__func__, __LINE__);
        return false;
    }
    if (b2_client.walking) {
        const pmove_state_t *p = &cl.frame.ps.pmove, *old = &b2_client.walk_previous;
        int64_t elapsed_ms = (int64_t)cl.servertime - (int64_t)b2_client.walk_previous_time;
        float dt = (float)elapsed_ms * .001f;
        float dx = (p->origin[0] - old->origin[0]) * .125f;
        float dy = (p->origin[1] - old->origin[1]) * .125f;
        float dz = (p->origin[2] - old->origin[2]) * .125f;
        // Server snapshot time is not accumulated user-command integration
        // time. A new authenticated combat publication may follow skipped
        // snapshots. Pmove already checked each integration; retire visual
        // history instead of treating that publication interval as movement.
        // B2_CombatSnapshot above retains its publication age/identity checks.
        bool publication_gap = elapsed_ms > 1000 && b2_client.combat &&
            b2_client.lifecycle && b2_client.advancing && b2_client.frozen &&
            !initial && cl.frame.number > b2_client.frame_number;
        if (b2_walk_fault->string[0] || elapsed_ms < 0 || (elapsed_ms > 1000 && !publication_gap) ||
            p->gravity != b2_client.anchor_player.gravity ||
            p->pm_flags & ((b2_client.combat ? 0 : PMF_TIME_TELEPORT | PMF_TIME_WATERJUMP) |
                (b2_client.vertical ? 0 : PMF_DUCKED | PMF_JUMP_HELD)) ||
            !b2_input_angles_match488() ||
            !isfinite(fd->vieworg[0]) || !isfinite(fd->vieworg[1]) || !isfinite(fd->vieworg[2])) {
            b2_log_player_anchor("walk-guard-stop");
            Com_Printf("B2WALK: movement discontinuity/unsupported state fault=%s dt=%.3f delta=%.3f,%.3f,%.3f; terminal stop.\n",
                b2_walk_fault->string, dt, dx, dy, dz);
            b2_stop_at(__func__, __LINE__); return false;
        }
        if (publication_gap) {
            // The entry guard rejected an open frame; both history and input
            // generations are retired before any new XR/eye work is admitted.
            b2_client.history.valid = false;
            B2XR_InvalidateHistory("server-publication-gap");
            Com_Printf("VR: refreshed view history after a delayed gameplay update (%.3f s).\n", dt);
        }
        b2_client.walk_previous = *p;
        b2_client.walk_previous_time = cl.servertime;
    }
    if (initial) {
        VectorCopy(fd->vieworg, b2_client.anchor_origin);
        VectorCopy(fd->viewangles, b2_client.anchor_angles);
    }
    // Replace only sampled scene storage, never per-eye history or XR origin.
    Z_Free(b2_client.entities); b2_client.entities = NULL;
    Z_Free(b2_client.particles); b2_client.particles = NULL;
    Z_Free(b2_client.dlights); b2_client.dlights = NULL;
    Z_Free(b2_client.lightstyles); b2_client.lightstyles = NULL;
    Z_Free(b2_client.areabits); b2_client.areabits = NULL;
    b2_client.scene = *fd;
    if (b2_client.advancing) {
        if (!b2_client.walking)
            VectorCopy(b2_client.anchor_origin, b2_client.scene.vieworg);
        if (b2_client.turning) {
            // One shared body orientation; B2_BuildView composes the relative
            // head/eye transform once. Never feed an already-composed eye back.
            for (unsigned i = 0; i < 3; ++i)
                b2_client.scene.viewangles[i] = b2_client.input_angles[i] +
                    SHORT2ANGLE(b2_client.anchor_player.delta_angles[i]);
        } else VectorCopy(b2_client.anchor_angles, b2_client.scene.viewangles);
    }
    entity_t reticle[12],teleport[B2TP_GUIDE_ENTITIES];
    b2_client.teleport_count=fd->num_entities<=MAX_ENTITIES-(B2TP_GUIDE_ENTITIES+12)?b2tp_geometry(teleport):0;
    b2_reticle_aim_t reticle_aim;
    b2_client.reticle_count=0;b2_client.reticle_tracked=false;
    b2_client.weapon_base_valid=false;b2_client.entity_base=fd->num_entities;
    int base=fd->num_entities+b2_client.teleport_count;
    b2_client.entity_capacity=base+(base<=MAX_ENTITIES-12?12:0);
    // Reserve all twelve slots before tracing; tracked geometry is prepared
    // once after the current display transform, never from last frame's overlay.
    if (!b2_controller.requested && base<=MAX_ENTITIES-12 &&
        b2_reticle_aim(&b2_client.scene,NULL,NULL,&reticle_aim))
        b2_client.reticle_count=b2_reticle_geometry(&reticle_aim,
            b2_world_reticle_scale->value,reticle);
    b2_client.scene.num_entities=fd->num_entities+b2_client.teleport_count+b2_client.reticle_count;
    if (b2_client.entity_capacity) {
        size_t ordinary=sizeof(*fd->entities)*(size_t)fd->num_entities;
        b2_client.entities=Z_Malloc(sizeof(*fd->entities)*(size_t)b2_client.entity_capacity);
        if (ordinary) memcpy(b2_client.entities,fd->entities,ordinary);
        if(b2_client.teleport_count)memcpy(b2_client.entities+fd->num_entities,teleport,sizeof(*teleport)*(size_t)b2_client.teleport_count);
        if (b2_client.reticle_count) {
            memcpy(b2_client.entities+fd->num_entities+b2_client.teleport_count,reticle,sizeof(*reticle)*(size_t)b2_client.reticle_count);
            memcpy(b2_client.reticle_aim,&reticle_aim,sizeof(reticle_aim));
        }
    }
    if (fd->num_dlights) {
        size_t bytes = sizeof(*fd->dlights) * (size_t)fd->num_dlights;
        b2_client.dlights = Z_Malloc(bytes);
        memcpy(b2_client.dlights, fd->dlights, bytes);
    }
    b2_client.lightstyles = Z_Malloc(sizeof(*fd->lightstyles) * MAX_LIGHTSTYLES);
    memcpy(b2_client.lightstyles, fd->lightstyles, sizeof(*fd->lightstyles) * MAX_LIGHTSTYLES);
    if (cl.frame.areabytes) {
        b2_client.areabits = Z_Malloc((size_t)cl.frame.areabytes);
        memcpy(b2_client.areabits, fd->areabits, (size_t)cl.frame.areabytes);
    }
    b2_client.scene.entities = b2_client.entities;
    if (B2WP_ViewRequested()) {
        vec3_t muzzle,tracked_origin;
        b2rs_weapon_origin(b2_client.scene.vieworg,tracked_origin);
        if (B2WP_RebaseView(tracked_origin,b2_client.scene.viewangles,Sys_Milliseconds(),muzzle)) {
            VectorCopy(muzzle,b2_client.weapon_base_muzzle);b2_client.weapon_base_valid=true;
            for (int i=0;i<b2_client.scene.num_entities;i++) {
                entity_t *e=&b2_client.entities[i];
                if (e->flags&RF_WEAPONMODEL) { VectorCopy(muzzle,e->origin);VectorCopy(muzzle,e->oldorigin); }
            }
        } else B2WP_RetireView();
    }
    if (fd->num_particles) {
        size_t bytes = sizeof(*fd->particles) * (size_t)fd->num_particles;
        b2_client.particles = Z_Malloc(bytes);
        memcpy(b2_client.particles, fd->particles, bytes);
    }
    b2_client.scene.dlights = b2_client.dlights;
    b2_client.scene.lightstyles = b2_client.lightstyles;
    b2_client.scene.areabits = b2_client.areabits;
    b2_client.scene.particles = b2_client.particles;
    b2_client.scene.b2_active = false;
    b2_client.scene.b2_frame = NULL;
    b2_client.servercount = cl.servercount;
    b2_client.frame_number = cl.frame.number;
    b2_client.client_time = cl.time;
    b2_client.server_time = cl.servertime;
    b2_client.demo_file = cls.demo.playback;
    b2_client.demo_offset = cls.demo.file_offset;
    Q_strlcpy(b2_client.mapname, cl.mapname, sizeof(b2_client.mapname));
    b2_client.entities_hash = b2_scene_entities_hash(&b2_client.scene);
    b2_client.particles_hash = b2_scene_particles_hash(&b2_client.scene);
    b2_client.particle_allocations = CL_ParticleAllocations();
    b2_client.scene_serial++;
    if (initial) {
        B2_HistoryReset(&b2_client.history);
        B2XR_InvalidateHistory("client_scene_freeze");
    }
    b2_client.pending = false;
    b2_client.frozen = true;
    if (initial) { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2: scene %s map=%s frame=%d time=%d entities=%d lights=%d scale=%g.\n",
        b2_client.advancing ? "anchored-advancing" : "frozen",
        b2_client.mapname, b2_client.frame_number, b2_client.client_time,
        fd->num_entities, fd->num_dlights, b2_units_per_metre->value); }
    return true;
}

bool B2_HasFrozenScene(void)
{
    return b2_client.frozen || b2_client.stopped;
}

// Home changes only the application tracking reference. The queued request is
// consumed at a stereo boundary and never starts or resumes an XR session.
static void b2_recenter_down_f(void)
{
    if(b2_client.recenter_held)return;
    b2_client.recenter_held=true;
    if(UI_B2BindingCapture304() || !B2_InputFocused556() ||
       !B2XR_InteractionReady() || B2XR_StopRequested() || b2_client.stopped ||
       b2_client.history.frame_open)return;
    // Film/main-menu presentation has an independent reference. Never retire
    // the gameplay room-scale reference or standing height from this route.
    if(B2_UISurfaceActive()) { b2_client.recenter_pending=true;return; }
    if((b2_client.ui_interrupted && !b2_client.headset_waiting) ||
       b2_client.world_waiting || b2_client.world_rebinding)return;
    if (b2_client.frozen && b2_client.walk_ready && cls.state==ca_active &&
        (cls.key_dest==KEY_GAME || ((cls.key_dest&KEY_MENU) && !(cls.key_dest&KEY_CONSOLE))))
    {
        if (b2_controller.requested) { b2_clear_walk_input(); CL_B2ReleaseButtons(); b2_client.walk_release_required=true; }
        R_B2UIReset();
        b2_client.recenter_pending = true;
    }
}

static void b2_recenter_up_f(void) { b2_client.recenter_held = false; }

// Explicit Home resets current optical height in either input mode. Controller
// return without Home retains its saved reference; ordinary frames keep it.
// Initial spawn still establishes an origin-relative reference, not a floor.
// Preserve the controller optical reference once, at the first committed KBM
// reset. Subsequent KBM resets cannot adopt a seated height as standing height.
static void b2_reference_height_committed480(void)
{
    if (b2_controller.requested) {
        b2_client.kbm_height_reference=false;
    } else if (!b2_client.kbm_height_reference) {
        b2_client.controller_reference_height=b2_client.reference_origin[1];
        b2_client.kbm_height_reference=true;
    }
}

static void b2rs_keep_reference_height(float origin[3])
{
    if (b2_client.origin_latched && b2_controller.requested &&
        (b2_client.kbm_height_reference ||
         (b2_roomscale && b2_roomscale->integer==1) ||
         (b2_physical_crouch && b2_physical_crouch->integer==1)))
        origin[1]=b2_client.kbm_height_reference ?
            b2_client.controller_reference_height : b2_client.reference_origin[1];
}
static bool b2_recenter_pose304(const b2_frame_t *frame, float origin[3], float *yaw)
{
    float forward_x = 0, forward_z = 0;
    if (!frame->should_render || !frame->tracking_valid) return false;
    for (unsigned eye = 0; eye < B2_EYE_COUNT; ++eye) {
        const float *q = frame->orientations[eye];
        float norm = 0;
        for (unsigned i = 0; i < 4; ++i) {
            if (!isfinite(q[i])) return false;
            norm += q[i] * q[i];
        }
        if (!isfinite(norm) || fabsf(norm - 1) > .02f) return false;
        forward_x += -2 * (q[0]*q[2] + q[1]*q[3]) / norm;
        forward_z += -(1 - 2 * (q[0]*q[0] + q[1]*q[1]) / norm);
        for (unsigned i = 0; i < 3; ++i)
            if (!isfinite(frame->positions[eye][i])) return false;
    }
    // Looking almost straight up/down has no dependable horizontal heading.
    if (forward_x*forward_x + forward_z*forward_z < .0001f) return false;
    for (unsigned i = 0; i < 3; ++i)
        origin[i] = .5f * frame->positions[0][i] + .5f * frame->positions[1][i];
    *yaw = atan2f(-forward_x, -forward_z);
    return true;
}

static bool b2_recenter_reference(const b2_frame_t *frame,float origin[3],float *yaw)
{
 if(!b2_recenter_pose304(frame,origin,yaw))return false;
 return true; // Explicit Home (and KBM entry) uses the current optical height.
}

static void b2_relative_pose(const float position[3], const float q[4],
    const float origin[3], float yaw, float p[3], float out[4])
{
    float c = cosf(yaw), s = sinf(yaw), h = sinf(-yaw*.5f), w = cosf(-yaw*.5f);
    float x = position[0]-origin[0], z = position[2]-origin[2];
    p[0] = c*x-s*z; p[1] = position[1]-origin[1]; p[2] = s*x+c*z;
    // Left multiply by inverse yaw around tracking-space up. Pitch and roll,
    // eye cant and separation remain in the individual runtime eye poses.
    out[0] = w*q[0]+h*q[2]; out[1] = w*q[1]+h*q[3];
    out[2] = w*q[2]-h*q[0]; out[3] = w*q[3]-h*q[1];
}

static bool b2_head_view(const b2_frame_t *frame, float reference_yaw, b2_view_t *head)
{
    float q[4], p[3], relative[4], zero[3]={0}, origin[3], dot=0, norm=0;
    for(unsigned i=0;i<3;i++) origin[i]=.5f*(frame->views[0].origin[i]+frame->views[1].origin[i]);
    for(unsigned i=0;i<4;i++) dot+=frame->orientations[0][i]*frame->orientations[1][i];
    for(unsigned i=0;i<4;i++) {q[i]=frame->orientations[0][i]+(dot<0?-1.f:1.f)*frame->orientations[1][i];norm+=q[i]*q[i];}
    if(norm<.0001f || !isfinite(norm)) return false;
    for(unsigned i=0;i<4;i++) q[i]/=sqrtf(norm);
    b2_relative_pose(zero,q,zero,reference_yaw,p,relative);
    return B2_BuildView(zero,relative,frame->fov_lrud[0],origin,b2_client.scene.viewangles,b2_units_per_metre->value,head);
}

// Read-only sample once after a successful logical frame, before the next
// prediction/input/world update. Diagnoses incline cadence without smoothing changes.
static void b2_log_movement_sample(const b2_frame_t *frame)
{
	if (!b2_client.advancing || !b2_client.walking || B2_UIPaused())
		return;
	if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2MOVE logical=%u scene=%u views=%u realtime=%u clienttime=%d servertime=%d serverframe=%d cmd=%u last_msec=%u pending_msec=%u step_frame=%u step=%.6f step_age_ms=%u lerp=%.6f pmflags=%u server_z=%.6f predicted_z=%.6f prediction_error_z=%.6f viewoffset_z=%.6f old_viewoffset_z=%.6f scene_xyz=%.6f,%.6f,%.6f xr_mid_y=%.6f predicted_ns=%lld period_ns=%lld\n",
		frame->logical_index, b2_client.scene_serial, frame->view_count,
		cls.realtime, cl.time, cl.servertime, cl.frame.number, cl.cmdNumber,
		(unsigned)cl.cmds[cl.cmdNumber & CMD_MASK].msec, (unsigned)cl.cmd.msec,
		cl.predicted_step_frame, cl.predicted_step, cls.realtime - cl.predicted_step_time,
		cl.lerpfrac, (unsigned)cl.frame.ps.pmove.pm_flags,
		cl.frame.ps.pmove.origin[2] * .125f, cl.predicted_origin[2], cl.prediction_error[2],
		cl.frame.ps.viewoffset[2], cl.oldframe.ps.viewoffset[2],
		b2_client.scene.vieworg[0], b2_client.scene.vieworg[1], b2_client.scene.vieworg[2],
		.5f * (frame->positions[0][1] + frame->positions[1][1]),
		(long long)frame->predicted_time_ns, (long long)frame->duration_ns);
}

// Explicit controller handoff establishes only the position of the settled headset. All views still
// pass the ordinary pose/projection validation before this candidate commits.
static bool b2_handoff_reference(const b2_frame_t *frame, float origin[3])
{
    if (!frame->should_render || !frame->tracking_valid ||
        (frame->view_count!=2 && frame->view_count!=4)) return false;
    for (unsigned eye=0;eye<frame->view_count;eye++)
        for (unsigned axis=0;axis<3;axis++)
            if (!isfinite(frame->positions[eye][axis])) return false;
    for (unsigned axis=0;axis<3;axis++)
        origin[axis]=.5f*frame->positions[0][axis]+.5f*frame->positions[1][axis];
    b2rs_keep_reference_height(origin);
    return true;
}

static void b2_headset_bootstrap(void)
{
    if (!B2_HeadsetWaiting() || b2_client.world_epoch || b2_client.pending ||
        b2_client.frozen || b2_client.stopped || !b2_controller_enabled() ||
        !b2_lifecycle || b2_lifecycle->integer!=1 || !b2_same_map_ui ||
        b2_same_map_ui->integer!=1 || cls.state!=ca_active || !cl.frame.valid)
        return;
    // No controller, head pose, focus manipulation or synthetic key is needed
    // to establish the ordinary local scene hold and begin XR presentation.
    // A first accepted local world (including cold Load) uses the same
    // committed-pair/neutral gate as later normal world transitions.
    if (B2_MenuSession() && !b2_client.ui_interrupted &&
        B2_InputFocused556() && B2XR_InteractionReady()) {
        b2_menu_transition=true;
        b2_menu_interruption=B2XR_InterruptionEpoch();
    }
    b2_headset_bootstrapping=true;
    b2_freeze_f();
    b2_headset_bootstrapping=false;
}


// Read-only presentation query in the existing accepted world frame. The body
// eye-level scene anchor already includes viewoffset, crouch and stair smoothing.
static float b2rs_wall_return(const b2_frame_t *frame,const b2_view_t *head,float direction[3])
{
    if(!b2_wall_return || b2_wall_return->integer!=1 || !b2_roomscale ||
       b2_roomscale->integer!=1 || !b2_client.origin_latched || !frame->tracking_valid ||
       b2_client.world_waiting || b2_client.world_rebinding ||
       B2_UISimulationHeld() || !B2XR_InteractionReady() ||
       cl.frame.ps.pmove.pm_type!=PM_NORMAL || cl.frame.ps.stats[STAT_HEALTH]<=0)return 0;
    const float *anchor=b2_client.scene.vieworg;
    const vec3_t zero={0};
    if(!B2RC_Direction(anchor,head->origin,direction))return 0;
    trace_t valid;
    CL_Trace(&valid,anchor,zero,zero,anchor,MASK_PLAYERSOLID);
    if(!isfinite(valid.fraction)||valid.fraction<1.f||valid.startsolid||valid.allsolid)return 0;
    // Separation persists beyond finite solids and empty leaves. Movement
    // admission may retire on backlog/age; it does not clear this current
    // geometric query in an otherwise accepted tracked world frame.
    return CL_B2WallDepth(anchor,head->origin);
}

#include "../../../../baseline/b2-consolidated215/inputs/origin-039/endpoint/base1-wall-client.h"

// Current-eye-time held display only. Command/fire/reticle owners are unchanged.
// Reset before every early-return path as well as before an accepted pair.
static void b2wv_reset_frame(void)
{
    B2WP_DisplayBegin(true);
    if (b2_client.weapon_base_valid && b2_client.entities)
        for (int i=0;i<b2_client.entity_base;i++) {
            entity_t *e=&b2_client.entities[i];
            if (e->flags&RF_WEAPONMODEL) {
                VectorCopy(b2_client.weapon_base_muzzle,e->origin);
                VectorCopy(b2_client.weapon_base_muzzle,e->oldorigin);
            }
        }
    if (b2_client.reticle_tracked) b2_reticle_clear();
}

static void b2wv_prepare(const b2_frame_t *frame, const float reference[3],
    float yaw, const float anchor[3], bool reference_stable)
{
    // Keep all existing operational fallbacks; only the user selector is retired.
    B2WP_DisplayBegin(true);
    if (!reference_stable || !b2_client.weapon_base_valid) return;
    uint32_t now=Sys_Milliseconds();
    b2wp_record_t owner;
    if (!B2W_Presentation(&owner) || !B2WV_Held(&owner) ||
        b2_controller_dominant->integer!=(int)owner.source.aim.hand ||
        !b2c_sample_valid(&b2_controller_sample,now)) return;
    b2c_snapshot_t sample;
    if (!B2XR_ReadWeaponDisplay(frame,owner.source.aim.hand,&sample)) return;
    b2w_space_t space={0};space.committed=true;space.epochs=owner.source.aim.epochs;
    VectorCopy(reference,space.reference_origin);space.reference_yaw=yaw;
    VectorCopy(anchor,space.anchor_origin);VectorCopy(b2_client.scene.viewangles,space.anchor_angles);
    space.units=b2_units_per_metre->value;
    now=Sys_Milliseconds(); // Sample acquisition may cross a millisecond boundary.
    b2w_pose_t aim;
    if (!B2W_BuildAim(&sample,owner.source.aim.hand,&space,now,&aim)) return;
    b2w_game_api_t bridge;b2w_inventory_t inventory;
    if (!SV_B2WeaponAPI(&bridge) || !bridge.inventory || !bridge.inventory(&inventory) ||
        inventory.epoch!=owner.packet.game_epoch || inventory.current!=owner.packet.weapon ||
        !inventory.ready || inventory.pending>=0) return;
    // Existing calibrated read-only collision preview; never publish this packet.
    b2w_packet_t candidate=owner.packet;candidate.pose=aim;
    b2wp_source_t visual;
    model_t *model=MOD_ForHandle(owner.source.handle);
    if (!B2WP_TryPreview(&candidate,&visual,owner.source.handle,model,&bridge,now,NULL) ||
        !B2WP_DisplayApply(&owner,&aim,visual.preview,frame->predicted_time_ns,Sys_Milliseconds())) return;
    for (int i=0;i<b2_client.scene.num_entities;i++) {
        entity_t *e=&b2_client.entities[i];
        if (e->flags&RF_WEAPONMODEL) {
            VectorCopy(visual.preview,e->origin);VectorCopy(visual.preview,e->oldorigin);
        }
    }
}

#include "../../../bloom331/source/b2_startup276.h"
#include "../../../reference-realignment397/source/b2_menu_input397.inc"
#include "../../../rc-cleanup457/source/b2_menu151_client457.inc"

// Three distinct completed frames, each with fresh current head/actions and
// neutral controls. No age limit is raised and no old pose/input is replayed.
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

bool B2_RenderStereo(void)
{
    extern bool R_B2SettingsRejected580(void);
    b2ds_frame_t denoiser_snapshot = { 0 };
    b2wv_reset_frame();
    b2_frame_t frame = { 0 };
    b2_view_t head_view={0};
    float reference_origin[3], reference_yaw = b2_client.reference_yaw;
    bool recenter = false, handoff = false, history_committed;
    bool accepted_pair = true, transaction = false, ended, renderer_committed;
    bool initial_reference_ready, initial_heading_ready = false;
    cvar_t *projection;
    b2_headset_bootstrap();
    bool start_requested=b2_start.pending;
    b2_start.pending=false; // This frame only; every early return consumes it.
    bool start_commit=false;
    if (B2XR_Requested())
        B2XR_PollEvents();
    B2_ProcessInterruptions();
    b2aw_tick();
    if (b2_client.walking && !b2_client.stopped)
        B2_FilterInput();
    if (b2_client.advancing && B2XR_StopRequested() && !b2_client.stopped)
        b2_stop_at(__func__, __LINE__);
    if (b2_client.stopped) {
        if (!CL_B2ShuttingDown457() && !B2XR_QuitOnly()) B2XR_NotifyStopped555();
        // Terminal gameplay stays frozen. Refused continuity may keep only
        // fresh tracking/actions and zero-layer frames for deliberate Quit.
        if(B2XR_QuitOnly() && B2XR_BeginFrame(&frame)) {
            if(!B2XR_EndFrame(false))B2XR_RequestStop();
            R_B2PairFinish(false);
        }
        return true;
    }
    if (B2_UISurfaceActive()) return b2_render_surface();
    if (B2_RealignActive391()) {
        if (!b2_reset397.automatic) return b2_render_surface();
        // A fresh basis/head sample is needed before drawing either world eye.
        // This is a bounded ordinary frame pump, never an inferred timeout.
        if (!b2_reset397.staged) {
            if (B2XR_BeginFrame(&frame)) {
                if (B2XR_ReferenceApplied397() && (!frame.should_render || !frame.tracking_valid))
                    b2_reference_fallback397();
                if (!B2XR_EndFrame(false)) b2_stop_at(__func__,__LINE__);
                R_B2PairFinish(false);
            }
            return true;
        }
    }
    if (b2_client.world_waiting && !b2_bind_new_world()) {
        if (!b2_client.stopped && B2XR_BeginFrame(&frame)) {
            if (!B2XR_EndFrame(false)) b2_stop_at(__func__, __LINE__);
            R_B2PairFinish(false);
        }
        return true;
    }
    if (!b2_client.frozen)
        return false;
    if (b2_client.world_rebinding) {
        if (!V_BuildB2WorldScene()) { b2_stop_at(__func__, __LINE__); return true; }
        b2_client.world_rebinding=false;
        b2_ui_apply_pause();
        if (b2_client.stopped) return true;
    }
    projection = Cvar_FindVar("pt_projection");
    // A settings refusal is not scene/ownership corruption. Pace a zero-layer
    // XR frame and retain the session; do not enter either renderer eye.
    if (b2_same_map_ui->integer != (int)b2_client.ui_enabled || com_timedemo->integer ||
        b2_advance_world->integer != (int)b2_client.advancing ||
        b2_keyboard_locomotion->integer != (int)b2_client.walking ||
        b2_combat->integer != (int)b2_client.combat ||
        b2_turning_mouselook->integer != (int)b2_client.turning ||
         b2_blaster_fire->integer != (int)b2_client.firing ||
         b2_jump_crouch->integer != (int)b2_client.vertical ||
         b2_visual_target->integer != (int)b2_client.target || b2_target_fault->string[0] ||
        b2_units_per_metre->modified || !projection || projection->integer != 0) {
        static bool warned;
        if (!warned) Com_Printf("VR frame skipped: session settings changed. Restart with the VR launcher.\n");
        warned = true;
        if (B2XR_BeginFrame(&frame)) {
            if (!B2XR_EndFrame(false)) b2_stop_at(__func__, __LINE__);
            R_B2PairFinish(false);
        }
        b2_client.history.valid = false;
        b2_client.audio_pending = false;
        return true;
    }
    if (!B2XR_Requested() || !b2_exclusive() || cls.state != ca_active ||
        !cl.frame.valid || cl.servercount != b2_client.servercount ||
        (!b2_client.advancing && (cl.frame.number != b2_client.frame_number ||
            cl.time != b2_client.client_time || cl.servertime != b2_client.server_time)) ||
        (b2_client.advancing && (cl.frame.number < b2_client.frame_number ||
            cl.time < b2_client.client_time || cl.servertime < b2_client.server_time)) ||
        strcmp(cl.mapname, b2_client.mapname) ||
        cls.demo.playback != b2_client.demo_file ||
        (b2_client.demo_file && cls.demo.file_offset != b2_client.demo_offset) ||
        (b2_client.advancing ? (B2_UIEnabled() ?
            (cl_paused->integer != (b2_client.ui_manual_pause ? 2 : b2_client.ui_paused ? 1 : 0) || sv_paused->integer != (int)b2_client.ui_paused) :
            (cl_paused->integer || sv_paused->integer)) :
            (cl_paused->integer != 2 || !sv_paused->integer)) ||
        r_config.width != B2G_Width() || r_config.height != B2G_Height()) {
        Com_Printf("B2: frozen scene, settings or playback boundary changed; stopping.\n");
        b2_stop_at(__func__, __LINE__);
        return true;
    }
    if (B2_UIEnabled()) B2_FilterInput();
    if (b2_client.stopped) return true;
    if (!b2_ui_validate_hold()) return true;
    if (b2_client.advancing && !B2_UIPaused() && !V_BuildB2WorldScene()) {
        b2_stop_at(__func__, __LINE__);
        return true;
    }
    if (b2_client.world_rebinding) {
        b2_client.world_rebinding=false;
        b2_client.ui_interrupted=true; b2_ui_apply_pause();
        if (b2_client.stopped) return true;
    }
    b2_reticle_retire();
    if(b2_client.teleport_count && (!b2tp_enabled() || !b2tp_input.aiming ||
        (uint32_t)(Sys_Milliseconds()-b2tp_preview_ms)>100)){
        int first=b2_client.scene.num_entities-b2_client.reticle_count-b2_client.teleport_count;
        if(b2_client.reticle_count)memmove(b2_client.entities+first,
            b2_client.entities+first+b2_client.teleport_count,sizeof(entity_t)*(size_t)b2_client.reticle_count);
        b2_client.scene.num_entities-=b2_client.teleport_count;b2_client.teleport_count=0;
    }
    // The action reader deliberately refuses xr.frame_open. Capture before
    // BeginFrame, then validate freshness/generation after its event polling.
    b2c_snapshot_t recenter_sample488={0};
    bool recenter_sample_ready488=false;
    if (b2_client.recenter_pending && b2_controller.requested &&
        !b2_client.combat_state.dead && cl.frame.ps.pmove.pm_type==PM_NORMAL)
        recenter_sample_ready488=B2XR_ReadControllerInput(&recenter_sample488);
    if (!B2XR_BeginFrame(&frame)) {
        b2_headset_wait_unavailable();
        if (B2_KeepFramePump() && !B2XR_StopRequested()) {
            b2_interrupt_kind582("XR-session-waiting", b2_manual_interruption582(false)); return true;
        }
        if (b2_client.walking && (b2_client.walk_ready || B2XR_StopRequested())) {
            Com_Printf("B2WALK: XR frame unavailable after input activation; terminal stop.\n");
            b2_stop_at(__func__, __LINE__);
        }
        return true;
    }
    frame.logical_index = b2_client.history.logical_index;
    if (B2_KeepFramePump() && (!frame.should_render || !frame.tracking_valid ||
        B2XR_InterruptionEpoch() != b2_client.ui_xr_epoch ||
        (!b2_client.ui_interrupted && !B2XR_InteractionReady()))) {
        b2_headset_wait_unavailable();
        b2_reference_fallback397();
        // Begin succeeded, so retire its images and zero-layer frame before
        // pause preparation. Never feed invalid/stale poses to either eye.
        ended = B2XR_EndFrame(false);
        R_B2PairFinish(false); // asserts no incomplete desktop/renderer pair
        cl.refdef = b2_client.scene;
        b2_client.history.valid = false;
        b2_client.audio_pending = false;
        if (!ended || B2XR_StopRequested()) b2_stop_at(__func__, __LINE__);
        else {
            bool manual = b2_manual_interruption582(B2XR_InterruptionEpoch() != b2_client.ui_xr_epoch);
            b2_client.ui_xr_epoch = B2XR_InterruptionEpoch();
            b2_interrupt_kind582(manual ? "focus-session-or-reference" : "XR-no-render-or-tracking", manual);
        }
        return true;
    }
    initial_reference_ready = B2XR_InitialReferenceReady();
    if (!frame.should_render || !frame.tracking_valid)
        accepted_pair = false;
    for (unsigned axis = 0; axis < 3; ++axis)
        reference_origin[axis] = b2_client.origin_latched ? b2_client.reference_origin[axis] :
            0.5f * frame.positions[0][axis] + 0.5f * frame.positions[1][axis];
    // Initial spawn and lifecycle rebind use the same tracking-space heading
    // as Home. Keep provisional frames until a visible horizontal pose exists;
    // position and yaw become persistent only after the stereo pair commits.
    if (!b2_client.origin_latched && initial_reference_ready && !b2_client.headset_waiting)
        initial_heading_ready = b2_recenter_reference(&frame, reference_origin, &reference_yaw);
    if (b2_client.headset_waiting && start_requested && initial_reference_ready &&
        b2_headset_start_sample594(Sys_Milliseconds())) {
        start_commit=b2_headset_reference(&b2_start.sample,reference_origin,&reference_yaw);
    }
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
    vec3_t room_view_origin;VectorCopy(b2_client.scene.vieworg,room_view_origin);
    room_view_origin[2]+=reset_controller_height ?
        B2RS_DuckDrop(true,(cl.oldframe.ps.pmove.pm_flags&PMF_DUCKED)!=0,
            (cl.frame.ps.pmove.pm_flags&PMF_DUCKED)!=0,cl.lerpfrac)-(reset_held_crouch?24.f:0.f) : b2rs_eye();
    for (unsigned eye = 0; accepted_pair && eye < frame.view_count; ++eye) {
        float position[3], orientation[4];
        b2_relative_pose(frame.positions[eye], frame.orientations[eye],
            reference_origin, reference_yaw, position, orientation);
        accepted_pair = B2_BuildView(position, orientation, frame.fov_lrud[eye],
            room_view_origin, b2_client.scene.viewangles,
            b2_units_per_metre->value, &frame.views[eye]);
    }
    if (accepted_pair) accepted_pair=b2_head_view(&frame,reference_yaw,&head_view);
    if (accepted_pair) accepted_pair=b2aw_prepare(&frame,&head_view);
    if (accepted_pair) {
        b2wv_prepare(&frame,reference_origin,reference_yaw,room_view_origin,
            b2_client.origin_latched && !recenter && !handoff && initial_reference_ready);
        b2_reticle_prepare(&frame,&head_view);
    }
    b2_client.entities_hash=b2_scene_entities_hash(&b2_client.scene);
    if (accepted_pair) {
        // READY/SYNCHRONIZED frames can be required to start presentation, but
        // their LOCAL pose origin may not yet be the visible headset origin.
        // Submit with a provisional midpoint and do not reuse its eye history.
        if (!b2_client.origin_latched || recenter || handoff)
            b2_client.history.valid = false;
        transaction = B2_HistoryBegin(&b2_client.history, &frame);
        denoiser_snapshot.logical = frame.logical_index;
        denoiser_snapshot.generation = frame.history_generation;
        denoiser_snapshot.runtime_foveated = B2XR_DenoiserFoveated(&frame);
        denoiser_snapshot.history_valid = frame.history_valid;
        denoiser_snapshot.headset_waiting = b2_client.headset_waiting;
        denoiser_snapshot.origin_latched = b2_client.origin_latched;
        denoiser_snapshot.walk_ready = b2_client.walk_ready;
        denoiser_snapshot.interrupted = b2_client.ui_interrupted;
        accepted_pair = transaction && b2aw_history(&frame);
    }
    float return_direction[3]={0};
    float return_depthQU=accepted_pair?b2rs_wall_return(&frame,&head_view,return_direction):0;
    uint32_t comfort_now=Sys_Milliseconds();
    int comfort_mode=b2_comfort_vignette?b2_comfort_vignette->integer:0;
    float comfort_strength=b2_comfort_strength?b2_comfort_strength->value:0.f;
    if(!isfinite(comfort_strength))comfort_strength=0;
    comfort_strength=Q_clipf(comfort_strength,0,1);
    bool comfort_ready=accepted_pair && frame.should_render && frame.tracking_valid &&
        b2_controller_locomotion556(comfort_now) && !B2XR_StopRequested() &&
        b2_controller_sample.xr_generation==B2XR_InputGeneration();
    bool comfort_tp=comfort_mode==1 || comfort_mode==3;
    float teleport_comfort=B2TP_Comfort(comfort_now,b2tp_accepted_ms,b2tp_pulsing,
        comfort_ready && comfort_tp && b2tp_enabled(),comfort_strength);
    bool comfort_moving=(comfort_tp && b2move_active301(comfort_now)) ||
       ((comfort_mode==2 || comfort_mode==3) && !B2B_TeleportOnly() &&
        b2_smooth_published304 && (uint32_t)(comfort_now-b2_smooth_ms304)<=100) ||
       ((comfort_mode==2 || comfort_mode==3) && b2_turn_published556 &&
        (uint32_t)(comfort_now-b2_turn_ms556)<=50);
    const uint32_t comfort_owner[9]={(uint32_t)comfort_mode,B2B_Revision(),
        b2_client.world_epoch,b2_client.recenter_epoch,b2_controller_sample.generation,
        b2_controller_sample.xr_generation,B2XR_InterruptionEpoch(),B2B_Hand(),B2B_TeleportOnly()};
    float movement_comfort=B2Comfort_Release308(&b2_comfort_release308,
        frame.logical_index,comfort_now,comfort_owner,
        comfort_ready && comfort_mode>=1 && comfort_mode<=3,comfort_moving,comfort_strength);
    teleport_comfort=max(teleport_comfort,movement_comfort);
    for (unsigned eye = 0; accepted_pair && eye < frame.view_count; ++eye) {
        cl.refdef = b2_client.scene;
        cl.refdef.x = cl.refdef.y = 0;
        cl.refdef.width = B2G_Width(); cl.refdef.height = B2G_Height();
        VectorCopy(frame.views[eye].origin, cl.refdef.vieworg);
        VectorCopy(frame.views[eye].angles, cl.refdef.viewangles);
        cl.refdef.fov_x = frame.views[eye].fov_xy[0];
        cl.refdef.fov_y = frame.views[eye].fov_xy[1];
        cl.refdef.b2_active = true;
        cl.refdef.b2_render_accepted = false;
        cl.refdef.b2_eye = eye;
        cl.refdef.b2_frame = &frame;
        if (b2_client.advancing)
            if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2WORLD pair=%u scene=%u eye=%u serverframe=%d clienttime=%d scenetime=%.6f entities=%d entityhash=%08x particles=%d particlehash=%08x allocated=%llu psalpha=%.6f blend=%.6f,%.6f,%.6f,%.6f dlights=%d\n",
                frame.logical_index, b2_client.scene_serial, eye, b2_client.frame_number,
                b2_client.client_time, b2_client.scene.time, b2_client.scene.num_entities,
                b2_client.entities_hash, b2_client.scene.num_particles,
                b2_client.particles_hash, (unsigned long long)b2_client.particle_allocations,
                cl.frame.ps.blend[3], b2_client.scene.blend[0], b2_client.scene.blend[1],
                b2_client.scene.blend[2], b2_client.scene.blend[3], b2_client.scene.num_dlights);
        R_BeginFrame();
        R_RenderFrame(&cl.refdef);
        if (!eye) SCR_DrawB2HUD();
        R_B2HUD(eye,&head_view);
        // Stock body aim, projected separately through each actual eye.
        // This is an angular reticle; head turns do not steer the weapon.
        vec3_t aim; AngleVectors(b2_client.scene.viewangles,aim,NULL,NULL);
        const b2_view_t *v=&frame.views[eye];
        float ax=aim[0]*v->V[0]+aim[1]*v->V[4]+aim[2]*v->V[8];
        float ay=aim[0]*v->V[1]+aim[1]*v->V[5]+aim[2]*v->V[9];
        float az=aim[0]*v->V[2]+aim[1]*v->V[6]+aim[2]*v->V[10];
        if (az>0.001f && !B2W_HideBodyReticle() &&
            (!b2_world_reticle || b2_world_reticle->integer!=1)) {
            float x=.5f*(v->P[0]*ax/az+v->P[8]+1.f);
            float y=.5f*(v->P[5]*ay/az+v->P[9]+1.f);
            if(x>=0 && x<=1 && y>=0 && y<=1) SCR_DrawB2Aim(x,y);
        }
        R_B2WallReturn(eye,return_direction,return_depthQU);
        if(teleport_comfort>0)R_B2Comfort(eye,teleport_comfort);
        R_EndFrame();
        accepted_pair = b2_scene_particles_hash(&b2_client.scene) == b2_client.particles_hash &&
            frame.should_render && frame.tracking_valid &&
            cl.refdef.b2_render_accepted &&
            B2_HistoryAcceptEye(&b2_client.history, eye);
        if (accepted_pair && eye < 4 && R_B2DenoiserReadView(frame.logical_index,
            frame.history_generation, eye, &denoiser_snapshot.view[eye]))
            denoiser_snapshot.mask |= 1u << eye;
    }
    ended = B2XR_EndFrame(accepted_pair);
    renderer_committed = R_B2PairFinish(accepted_pair && ended);
    cl.refdef = b2_client.scene; // Never retain the stack-owned frame pointer.
    history_committed = transaction && B2_HistoryFinish(&b2_client.history, renderer_committed);
    denoiser_snapshot.completed_ms = Sys_Milliseconds();
    b2ds_record(&denoiser_snapshot, accepted_pair && ended && renderer_committed && history_committed);
    if (!b2aw_commit(&frame, accepted_pair && ended && renderer_committed && history_committed)) b2aw_fail();
    if (!history_committed) {
        b2wv_reset_frame();
        b2_client.entities_hash=b2_scene_entities_hash(&b2_client.scene);
        if (!transaction)
            b2_client.history.valid = false;
        B2XR_InvalidateHistory("client_pair_not_committed");
    } else {
        Com_DPrintf("B2 pair=%u generation=%u predicted=%lld duration=%lld history=%d committed\n",
            frame.logical_index, frame.history_generation,
            (long long)frame.predicted_time_ns, (long long)frame.duration_ns,
            frame.history_valid);
    }
    if (accepted_pair && ended && renderer_committed && history_committed &&
        !b2_client.origin_latched && initial_heading_ready && !b2_client.headset_waiting) {
        b2_client.kbm_height_reference=false;
        memcpy(b2_client.reference_origin, reference_origin, sizeof(reference_origin));
        b2_client.reference_yaw = reference_yaw;
        b2_client.origin_latched = true;
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2ORIGIN finalised pair=%u reference=%.9g,%.9g,%.9g yaw=%.9g visible_committed=1 heading_aligned=1\n",
            frame.logical_index, reference_origin[0], reference_origin[1], reference_origin[2], reference_yaw);
    }
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
    if (accepted_pair && ended && renderer_committed) {
        b2_log_movement_sample(&frame);
        VectorCopy(head_view.origin,b2_client.audio_origin);
        VectorCopy(head_view.angles,b2_client.audio_angles);
        b2_client.audio_pending=true;
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
    if (b2_client.headset_waiting) {
        b2_client.ui_resume_request=false;
        b2_client.ui_recovery_ready=accepted_pair && ended && renderer_committed && history_committed &&
            B2_InputFocused556() && B2XR_InteractionReady();
    }
    b2_reference_commit397(accepted_pair,ended,renderer_committed,history_committed);
    b2_finish_interruption256(accepted_pair,ended,renderer_committed,history_committed);
    return true;
}

// Called by the ordinary audio update. Never advances sound per eye.
bool B2_AudioListener(void)
{
    if (!B2XR_Requested() || B2_UISurfaceActive()) return true;
    if (b2_client.stopped || !b2_client.audio_pending) return false;
    b2_client.audio_pending=false;
    VectorCopy(b2_client.audio_origin, listener_origin);
    AngleVectors(b2_client.audio_angles,listener_forward,listener_right,listener_up);
    return true;
}


static bool b2_controller_enabled(void)
{
    return b2_controller_input && b2_controller_input->integer==1 &&
        b2_controller_dominant && (b2_controller_dominant->integer==0 || b2_controller_dominant->integer==1);
}

// Runs after the ordinary reducer consumes the one existing input snapshot.
// B2C_BLOCKED still suppresses every normal action throughout interruption.
static void b2_controller_resume_tick(uint32_t now)
{
    const b2c_snapshot_t *p=&b2_controller_sample;
    int hand=b2_controller_dominant ? b2_controller_dominant->integer : -1;
    int turn=b2_controller_turn_mode ? b2_controller_turn_mode->integer : -1;
    int tracked=b2_tracked_weapon ? b2_tracked_weapon->integer : 0;
    int teleport=b2_teleport ? b2_teleport->integer : 0;
    if (B2_RealignActive391() || !b2_controller_resume || b2_controller_resume->integer!=1 ||
        !b2_controller_enabled() || !b2_controller.requested || !B2_UIEnabled() ||
        !b2_client.ui_interrupted || !b2_client.ui_recovery_ready || b2_client.stopped || !b2_client.walk_ready ||
        b2_client.world_waiting || b2_client.world_rebinding || b2_client.handoff_position_pending ||
        !B2_InputFocused556() || !B2XR_InteractionReady() ||
        turn<0 || turn>B2C_TURN_SMOOTH || !b2w_owner_epoch || !b2_client.world_epoch ||
        !b2c_sample_valid(p,now) || !p->xr_generation || p->xr_generation!=B2XR_InputGeneration() ||
        p->serial!=b2_controller.serial || p->generation!=b2_controller.generation) {
        b2_resume_reset(); return;
    }
    bool pressed=B2B_Value(p,B2B_RESUME,hand,0)>.5f;
    if (!b2_resume.configured || b2_resume.generation!=p->generation ||
        b2_resume.xr_generation!=p->xr_generation || b2_resume.owner!=b2w_owner_epoch ||
        b2_resume.world!=b2_client.world_epoch || b2_resume.interruption!=b2_client.ui_xr_epoch ||
        b2_resume.dominant!=hand || b2_resume.turn!=turn ||
        b2_resume.tracked!=tracked || b2_resume.teleport!=teleport || p->serial<b2_resume.serial) {
        b2_resume_reset();
        b2_resume.configured=true;
        b2_resume.generation=p->generation; b2_resume.xr_generation=p->xr_generation;
        b2_resume.owner=b2w_owner_epoch; b2_resume.world=b2_client.world_epoch;
        b2_resume.interruption=b2_client.ui_xr_epoch;
        b2_resume.dominant=hand; b2_resume.turn=turn;
        b2_resume.tracked=tracked; b2_resume.teleport=teleport;
        b2_resume.serial=p->serial; b2_resume.previous=pressed;
        return; // A new boundary needs a later fresh neutral sample.
    }
    if (p->serial==b2_resume.serial) return;
    b2_resume.serial=p->serial;
    bool was_pressed=b2_resume.previous;
    bool edge=pressed && !was_pressed;
    b2_resume.previous=pressed;
    b2c_snapshot_t released=*p;
    B2B_Clear(&released,B2B_RESUME,hand);
    bool otherwise_neutral=b2c_neutral(&released);
    if (edge) {
        bool allowed=b2_resume.armed && otherwise_neutral;
        b2_resume.armed=false;
        // Existing press-to-request remains unchanged when headset startup is off.
        // The opted-in sleep/wake route requires release before simulation resumes.
        b2_resume.pressed=allowed && b2_headset_start && b2_headset_start->integer==1;
        if (allowed && !b2_resume.pressed) b2_request_interruption_resume(true);
        if (!allowed) Com_Printf("B2_CONTROLLER_RESUME refused; release all controls before a fresh off-hand top-button press.\n");
    } else if (!pressed) {
        if (was_pressed && b2_resume.pressed && b2c_neutral(p)) b2_request_interruption_resume(true);
        b2_resume.pressed=false;
        b2_resume.armed=b2c_neutral(p);
    } else if (!otherwise_neutral) {
        b2_resume.armed=false;b2_resume.pressed=false;
    }
}

// Headless CPU proof extracts these exact predicates and gesture consumer.
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

// Recheck the consumed request too: an XR frame wait can age it or change
// its generation. Neither keyboard nor controller requests bypass this gate.
static bool b2_headset_start_sample594(uint32_t now)
{
    return b2_headset_start_valid(now) && b2c_sample_valid(&b2_start.sample,now) &&
        b2_start.sample.generation==b2_controller_sample.generation &&
        b2_start.sample.generation==B2XR_ActionGeneration594() &&
        b2_start.sample.xr_generation==B2XR_InputGeneration() &&
        b2_start.sample_epoch==B2XR_InterruptionEpoch() &&
        b2c_neutral(&b2_start.sample) && b2c_neutral(&b2_controller_sample);
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

static void b2_controller_status_f(void)
{
    Com_Printf("B2_CONTROLLER enabled=%d owner=%s valid=%d neutral_required=%d dominant=%d\n",
        b2_controller_enabled(), b2_controller.requested?"controllers":"keyboard-mouse",
        b2_controller.valid,b2_controller.wait_neutral,b2_controller_dominant?b2_controller_dominant->integer:1);
}

static void b2_controller_handoff_f(void)
{
    B2H_Reset();
    if (!b2_controller_enabled() || !B2_UIEnabled() || !b2_client.walking || !b2_client.turning ||
        !b2_client.walk_ready || b2_client.stopped || b2_client.history.frame_open ||
        b2_client.handoff_position_pending) return;
    b2_controller.requested=!b2_controller.requested;
    Cvar_SetInteger(b2_input_mode484,b2_controller.requested?1:0,FROM_MENU);
    if (!b2_controller.requested) {
        // Retire only the physical stance contribution. Keyboard crouch still
        // follows the existing neutral-release and physical-key input path.
        if (b2rs_stance.crouched) {
            b2_realign_duck=false;b2_realign_duck_seen=false;
        }
        memset(&b2rs_client.eye,0,sizeof(b2rs_client.eye));
        b2_client.recenter_pending=true;
    }
    // Keep an explicit Home request; it takes heading precedence at commit.
    b2_client.handoff_position_pending=true;
    R_B2UIReset();
    b2_clear_walk_input(); CL_B2ReleaseButtons();
    b2_client.walk_release_required=true;
    if (Cvar_VariableInteger("developer") > 0) b2_controller_status_f(); // Does not alter an existing keyboard recovery interruption.
}

static void b2w_update_configuration(void)
{
    bool feature=b2_tracked_weapon && b2_tracked_weapon->integer!=0;
    if (b2w_config_hand==b2_controller_dominant->integer &&
        b2w_config_turn==b2_controller_turn_mode->integer && b2w_config_feature==feature) return;
    b2w_config_hand=b2_controller_dominant->integer;
    b2w_config_turn=b2_controller_turn_mode->integer;
    b2w_config_feature=feature;
    b2_clear_walk_input();CL_B2ReleaseButtons();
}

bool B2W_HideBodyReticle(void)
{
    return b2_tracked_weapon && b2_tracked_weapon->integer!=0 && b2_controller.requested;
}

bool B2W_HUDState(b2w_hud_t *out, uint32_t now)
{
    b2w_epochs_t epochs={b2_controller_sample.generation,b2_controller_sample.xr_generation,
        b2_client.world_epoch,b2_client.recenter_epoch,b2w_owner_epoch,1};
    if (!out || !B2W_HideBodyReticle() || !b2_controller.valid || b2_client.stopped ||
        B2_UISimulationHeld() || b2_client.world_waiting || b2_client.world_rebinding ||
        b2_client.recenter_pending || b2_client.handoff_position_pending || b2_client.combat_state.dead ||
        cls.key_dest!=KEY_GAME || !B2_InputFocused556() ||
        !B2W_EpochsEqual(&epochs,&b2w_hud.epochs) ||
        !b2c_sample_valid(&b2_controller_sample,now) || (uint32_t)(now-b2w_hud.capture_ms)>100) return false;
    float head_norm=0;
    for (unsigned i=0;i<3;i++) if (!isfinite(b2_controller_sample.head_position[i])) return false;
    for (unsigned i=0;i<4;i++) {
        float value=b2_controller_sample.head_orientation[i];
        if (!isfinite(value)) return false;
        head_norm+=value*value;
    }
    if (!isfinite(head_norm) || fabsf(head_norm-1.f)>.02f) return false;
    *out=b2w_hud;
    if (out->open && !out->anchored) return false;
    if (out->anchored) for (unsigned i=0;i<3;i++)
        out->plane.origin[i]+=b2_client.scene.vieworg[i]-out->body_origin[i];
    if (out->status>=B2W_UI_ACCEPTED && (uint32_t)(now-out->status_ms)>1000) out->status=B2W_UI_NONE;
    return out->open || out->rearm || out->status!=B2W_UI_NONE;
}

static void b2w_suppress_actions(void)
{
    b2w_blocked_preview.active=false;
    memset(&b2_controller.intent,0,sizeof(b2_controller.intent));
    b2_controller.attack=false;b2_controller.crouch=false;b2_controller.turn_clock_valid=false;b2_controller.turn_ms=0;
    b2_controller.snap_ready=false;b2w_move_latched=false;
}

// Selecting suppresses firing and turning, while admitted locomotion keeps
// its original reducer intent and the offhand crouch hysteresis.
static void b2w_suppress_selector_actions(bool hold_turn)
{
    b2_controller.intent.attack=false;b2_controller.intent.events=0;
    b2_controller.attack=false;
    // Selector ownership suppresses activation, while normal locomotion and
    // turning keep their existing gates. Only unqualified FIRE retains the hold.
    if (!hold_turn) return;
    b2_controller.intent.yaw=0;
    b2_controller.turn_clock_valid=false;b2_controller.turn_ms=0;
    b2_controller.snap_ready=false;
}

// Movement direction is separate from command aim.
// B2 latches the head heading only when a stick stroke starts: looking/firing
// sideways then leaves travel unchanged until the stick returns to neutral.
// Heading remains fixed until the stick returns to neutral.
static bool b2w_latch_movement(const b2w_space_t *space, uint32_t now)
{
    if (!b2_controller.intent.forward && !b2_controller.intent.side) {
        b2w_move_latched=false;return true;
    }
    if (b2w_move_latched) return true;
    if(!b2c_sample_valid(&b2_controller_sample,now))return false;
    const float *q=b2_controller_sample.head_orientation;
    float h=sinf(-space->reference_yaw*.5f),w=cosf(-space->reference_yaw*.5f);
    float relative[4]={w*q[0]+h*q[2],w*q[1]+h*q[3],w*q[2]-h*q[0],w*q[3]-h*q[1]};
    const float zero[3]={0},fov[4]={-.5f,.5f,.5f,-.5f};b2_view_t view;
    if(!B2_BuildView(zero,relative,fov,zero,space->anchor_angles,1,&view) ||
       view.invV[8]*view.invV[8]+view.invV[9]*view.invV[9]<=.01f)return false;
    float heading=atan2f(view.invV[9],view.invV[8]);
    float offset=heading-space->anchor_angles[YAW]*(M_PI/180.f);
    b2w_move_cos=cosf(offset);b2w_move_sin=sinf(offset);b2w_move_latched=true;
    return true;
}


// Cache the opening facing in world coordinates, while the origin follows the
// admitted head/player translation. Drawing and pointing use this same plane.
static bool b2w_selector_plane(const b2w_epochs_t *epochs, uint32_t now,
    b2w_pose_t *plane, b2w_pose_t *aim)
{
    b2w_space_t space={0};space.epochs=*epochs;space.committed=true;
    b2rs_reference(space.reference_origin);
    space.reference_yaw=b2_client.reference_yaw;space.units=b2_units_per_metre->value;
    VectorCopy(b2_client.scene.vieworg,space.anchor_origin);space.anchor_origin[2]+=b2rs_eye();
    for (unsigned i=0;i<3;i++) space.anchor_angles[i]=b2_client.input_angles[i]+
        SHORT2ANGLE(b2_client.anchor_player.delta_angles[i]);
    unsigned hand=b2_controller_dominant->integer;
    if (hand>1 || !B2W_BuildAim(&b2_controller_sample,hand,&space,now,aim)) return false;
    b2c_snapshot_t head=b2_controller_sample;
    VectorCopy(head.head_position,head.hands[hand].aim_position);
    memcpy(head.hands[hand].aim_orientation,head.head_orientation,sizeof(head.head_orientation));
    if (!B2W_BuildAim(&head,hand,&space,now,plane)) return false;
    if (b2w_selector.open && b2w_hud.anchored) {
        // The open grid already latches orientation. Latch its position in the
        // same body-relative space too, so native head micro-motion cannot
        // translate the panel between samples. Rendering uses this same delta.
        for(unsigned i=0;i<3;i++)
            plane->origin[i]=b2w_hud.plane.origin[i]+b2_client.scene.vieworg[i]-b2w_hud.body_origin[i];
        VectorCopy(b2w_hud.plane.forward,plane->forward);
        VectorCopy(b2w_hud.plane.right,plane->right);
        VectorCopy(b2w_hud.plane.up,plane->up);
    }
    return true;
}

static bool b2_grid_give_all_available(uint32_t now)
{
    return b2_give_all && b2_give_all->integer==1 && b2_controller.requested &&
        b2_controller.valid && !b2_controller.wait_neutral && b2_client.origin_latched &&
        b2_client.walk_ready && !b2_client.walk_release_required &&
        b2_client.lifecycle && b2_client.combat && !b2_client.combat_state.dead &&
        !b2_client.headset_waiting && !b2_client.stopped && !B2_UISimulationHeld() &&
        !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.handoff_position_pending && !b2_client.recenter_pending &&
        !b2_client.history.frame_open && cls.state==ca_active && cl.frame.valid &&
        cl.frame.ps.pmove.pm_type==PM_NORMAL && cl.frame.ps.stats[STAT_HEALTH]>0 &&
        B2_InputFocused556() && cls.key_dest==KEY_GAME &&
        cls.serverAddress.type==NA_LOOPBACK && sv_running->integer && cl.maxclients==1 &&
        B2XR_InteractionReady() && b2c_sample_valid(&b2_controller_sample,now) &&
        b2_controller_sample.xr_generation==B2XR_InputGeneration();
}

static void b2_grid_give_all(uint32_t now)
{
    if (!b2_grid_give_all_available(now) || !b2c_neutral(&b2_controller_sample)) return;
    b2wi_inventory_t shown;
    if (!SV_B2ItemState(&shown)) return;
    int client_time=cl.time,server_time=cl.servertime;
    // The grid owns this fresh neutral grip release. Dispatch synchronously
    // through the same stock GiveItems command used by the guarded console;
    // no console, pause transition, queued retry or synthetic input is needed.
    int result=SV_B2GiveAll(&shown,b2_controller_sample.serial,b2w_owner_epoch);
    if (result<0 || cl.time!=client_time || cl.servertime!=server_time) {
        Com_Printf("B2_GIVE_ALL refused unowned simulation change\n");
        b2_stop_at(__func__,__LINE__);return;
    }
    Com_Printf("B2_GIVE_ALL selected result=%d serial=%llu owner=%u\n",
        result,(unsigned long long)b2_controller_sample.serial,b2w_owner_epoch);
}

// Hold ownership stays with the grip that opened the grid, including after a page slide.
static bool b2w_items_grip316;
static void b2w_selector_grips316(b2c_snapshot_t *sample,unsigned dominant)
{
    float weapons=B2B_Value(sample,B2B_SELECT,dominant,0);
    float items=B2B_Value(sample,B2B_ITEMS,dominant,0);
    if(!b2w_selector.open){
        if(items>.65f && weapons>.65f)B2W_CancelSelector(&b2w_selector);
        b2w_items_grip316=items>.65f && weapons<.35f;
        b2w_selector.page=b2w_items_grip316 && !b2w_selector.rearm;
    }
    float other=B2B_Source(B2B_ITEMS,dominant)>=0 ? items : B2B_Value(sample,B2B_CROUCH,dominant,0);
    int source=B2B_Source(b2w_items_grip316?B2B_ITEMS:B2B_SELECT,dominant);
    sample->hands[dominant].squeeze_active=source>=0 && source<12 &&
        (b2_controller_sample.hands[source/6].ready556&B2C_GRIP556)!=0;
    sample->hands[dominant].squeeze=b2w_items_grip316?items:weapons;
    sample->hands[1-dominant].squeeze=b2w_items_grip316?weapons:other;
}

static bool b2w_update_selector(const b2w_inventory_t *inventory, const b2w_epochs_t *epochs, uint32_t now)
{
    int acknowledged_slot;bool accepted;
    if (b2w_pending_token && !B2W_EpochsEqual(&b2w_pending_epochs,epochs)) {
        b2w_hud.status=B2W_UI_REFUSED;b2w_hud.status_ms=now;b2w_pending_token=0;
    }
    if (b2w_pending_token) {
        if (b2w_pending_sent && inventory->epoch==b2w_pending_game_epoch &&
            B2W_SelectionAck(b2w_pending_token,&acknowledged_slot,&accepted,now)) {
            b2w_hud.slot=acknowledged_slot;b2w_hud.status_ms=now;
            if (accepted) {
                b2w_hud.status=B2W_UI_ACCEPTED;b2w_pending_token=0;
            } else if ((inventory->available&(1u<<b2w_pending_slot)) && b2w_token_counter!=UINT64_MAX) {
                // Stock may become busy between publication and consumption.
                // Retire the refused token; only a new fresh command may retry.
                b2w_pending_token=++b2w_token_counter;b2w_pending_sent=false;
                b2w_hud.status=B2W_UI_WAITING;
            } else {
                b2w_hud.status=B2W_UI_REFUSED;b2w_pending_token=0;
            }
        } else if (inventory->epoch!=b2w_pending_game_epoch ||
            b2w_pending_slot<0 || b2w_pending_slot>=12 || !(inventory->available&(1u<<b2w_pending_slot)) ||
            (b2w_pending_sent && (uint32_t)(now-b2w_pending_ms)>100)) {
            b2w_hud.status=B2W_UI_REFUSED;b2w_hud.status_ms=now;b2w_pending_token=0;
        }
    }
    float x=0,y=0;
    b2w_pose_t plane={0},aim={0};
    bool geometry=b2w_selector_plane(epochs,now,&plane,&aim);
    b2w_pose_t metric_plane=plane,metric_aim=aim;
    float units=b2_units_per_metre->value;
    if (!isfinite(units) || units<=0) geometry=false;
    if (geometry) for (unsigned i=0;i<3;i++) {
        metric_plane.origin[i]/=units;metric_aim.origin[i]/=units;
    }
    bool cursor=geometry && B2W_PlaneRay(&metric_plane,&metric_aim,
        (float)B2G_Height()/(float)B2G_Width(),&x,&y);
    b2wi_inventory_t items={0};
    bool item_state=SV_B2ItemState(&items) && items.epoch==inventory->epoch;
    if (!item_state) memset(&items,0,sizeof(items));
    const unsigned give_bit=1u<<B2WI_COUNT; // UI slot 7; stock item ordinals remain 0..6.
    bool give_available=b2_grid_give_all_available(now);
    // Availability changes retire hover/motion and old-page candidates.
    bool availability_ok=item_state && (!b2w_selector.open || !b2w_hud.open ||
        (inventory->available==b2w_hud.inventory.available &&
         inventory->owned==b2w_hud.inventory.owned &&
         items.available==(b2w_hud.items.available&~give_bit) && items.owned==(b2w_hud.items.owned&~give_bit) &&
         give_available==!!(b2w_hud.items.available&give_bit) &&
         items.active==b2w_hud.items.active));
    b2c_snapshot_t selector_sample=b2_controller_sample;
    unsigned sd=(unsigned)b2_controller_dominant->integer;
    b2w_selector_grips316(&selector_sample,sd);
    int pointed=cursor ? B2W_GridSlot(x,y,(float)B2G_Height()/B2G_Width()) : -1;
    if (cursor && B2W_PageHit(b2w_selector.page,x,y,(float)B2G_Height()/B2G_Width())) pointed=12;
    selector_sample.hands[sd].trigger=B2B_Value(&b2_controller_sample,B2B_FIRE,sd,0);
    selector_sample.hands[1-sd].trigger=B2B_Value(&b2_controller_sample,B2B_TELEPORT,sd,0);
    b2w_selection_t selection=B2W_Select(&b2w_selector,&selector_sample,epochs,now,
        // Opening is independent of stock animation readiness; only dispatch waits.
        b2_controller_dominant->integer,geometry && availability_ok,true,b2w_selector.page?(items.available|(give_available?give_bit:0)):inventory->available,pointed);
    if (selection.opened) {
        // Reset retires all old bound packets, including any unacknowledged
        // selection. Preserve the choice with a new token, never the old packet.
        if (b2w_pending_token && b2w_pending_sent) {
            b2w_pending_token=b2w_token_counter==UINT64_MAX ? 0 : ++b2w_token_counter;
            b2w_pending_sent=false;
        }
        B2W_Reset();b2w_published_serial=0;
        b2w_hud.status=b2w_pending_token ? B2W_UI_WAITING : B2W_UI_NONE;
        B2W_SetContext(true,epochs,false,now);
    }
    if (selection.selected>=0) {
        if (b2w_token_counter!=UINT64_MAX) {
            b2w_pending_token=++b2w_token_counter;b2w_pending_ms=now;b2w_pending_sent=false;
            b2w_pending_game_epoch=inventory->epoch;b2w_pending_slot=selection.selected;
            b2w_pending_epochs=*epochs;
            b2w_hud.slot=selection.selected;b2w_hud.status=B2W_UI_WAITING;b2w_hud.status_ms=now;
        }
    }
    if (selection.used==B2WI_COUNT) {
        b2_grid_give_all(now);
        return false; // No firing, movement publication or item dispatch leaks through.
    }
    if (selection.used>=0) {
        bool used=item_state && SV_B2UseItem(&items,(unsigned)selection.used,b2_controller_sample.serial,epochs->owner);
        b2w_hud.status=used?B2W_UI_ITEM_USED:B2W_UI_ITEM_REFUSED;b2w_hud.status_ms=now;
        // Refresh counts after the synchronous stock command. Item actions do
        // not replace, retry or acknowledge any pending weapon-selection token.
        if (!SV_B2ItemState(&items) || items.epoch!=inventory->epoch) memset(&items,0,sizeof(items));
    }
    // One pending deliberate choice, replaced only by a later release. A busy
    // stock weapon keeps the choice locally without publishing a stale token.
    if (b2w_pending_token && !b2w_pending_sent && inventory->ready && !b2w_selector.open &&
        b2_controller_sample.serial!=b2w_published_serial) {
        b2w_pending_sent=true;b2w_pending_ms=now;
    }
    b2w_actions_suppressed=selection.suppress || b2w_selector.open || b2w_selector.rearm || b2w_pending_token!=0;
    b2w_hud.epochs=*epochs;b2w_hud.inventory=*inventory;b2w_hud.capture_ms=now;
    b2w_hud.open=b2w_selector.open;b2w_hud.rearm=b2w_selector.rearm;
    b2w_hud.page=b2w_selector.page;b2w_hud.items=items;
    if (give_available) { b2w_hud.items.owned|=give_bit; b2w_hud.items.available|=give_bit; }
    b2w_hud.sliding=b2w_selector.sliding;b2w_hud.slide_from=b2w_selector.slide_from;
    b2w_hud.slide_ms=b2w_selector.slide_ms;
    b2w_hud.arrow_pressed=b2w_selector.hovering || b2w_selector.sliding;
    b2w_hud.anchored=b2w_selector.open && geometry;
    if (b2w_hud.anchored) {
        b2w_hud.plane=plane;VectorCopy(b2_client.scene.vieworg,b2w_hud.body_origin);
    } else {
        memset(&b2w_hud.plane,0,sizeof(b2w_hud.plane));
        VectorClear(b2w_hud.body_origin);
    }
    b2w_hud.candidate=b2w_selector.candidate;b2w_hud.stable=b2w_selector.selected;
    b2w_hud.cursor=cursor;b2w_hud.x=x;b2w_hud.y=y;
    return geometry;
}

// One immutable input publication, outside the renderer and before command
// finalisation. The optional FIRE capability remains absent until calibrated
// representation and selector integration also qualify; no fallback is used.
// Locomotion is independent of a safe weapon muzzle, but not of tracking,
// neutral, model identity, local player inventory, epochs or the 100ms bound.
static bool b2w_move_only_ready(uint32_t now)
{
    b2w_epochs_t epochs={b2_controller_sample.generation,b2_controller_sample.xr_generation,
        b2_client.world_epoch,b2_client.recenter_epoch,b2w_owner_epoch,1};
    b2wm_info_t info;b2w_game_api_t bridge;b2w_inventory_t inventory;
    if (!b2w_blocked_preview.active || !b2_tracked_weapon || b2_tracked_weapon->integer!=1 ||
        !b2_controller.requested || !b2_controller.valid || b2_controller.wait_neutral ||
        b2_controller.destination!=B2C_GAME || !b2_client.walk_ready || b2_client.walk_release_required ||
        !b2_client.origin_latched || b2_client.stopped || b2_client.world_waiting ||
        b2_client.world_rebinding || b2_client.recenter_pending || b2_client.handoff_position_pending ||
        b2_client.ui_interrupted || b2_client.combat_state.dead || B2_UISimulationHeld() ||
        cls.state!=ca_active || !cl.frame.valid || !B2_InputFocused556() || cls.key_dest!=KEY_GAME ||
        cl.frame.ps.pmove.pm_type!=PM_NORMAL || cl.frame.ps.stats[STAT_HEALTH]<=0 ||
        !B2XR_InteractionReady() || !b2c_sample_valid(&b2_controller_sample,now) ||
        b2_controller_sample.serial!=b2w_blocked_preview.pose.serial ||
        !B2W_PoseCurrent(&b2w_blocked_preview.pose,&epochs,now) ||
        cl.model_draw[cl.frame.ps.gunindex&GUNINDEX_MASK]!=(qhandle_t)b2w_blocked_preview.handle ||
        !B2WM_Query(MOD_ForHandle(b2w_blocked_preview.handle),B2WM_VERSION,sizeof(info),&info) ||
        memcmp(&info.key,&b2w_blocked_preview.model,sizeof(info.key)) ||
        !SV_B2WeaponAPI(&bridge) || !(bridge.capabilities&B2W_CAP_PREVIEW) ||
        !bridge.inventory || !bridge.inventory(&inventory) ||
        inventory.epoch!=b2w_blocked_preview.inventory.epoch ||
        inventory.current!=b2w_blocked_preview.inventory.current ||
        inventory.pending!=b2w_blocked_preview.inventory.pending ||
        !(inventory.owned&(1u<<inventory.current))) return false;
    return true;
}

static bool b2w_input_move_only556(uint32_t now)
{
    b2w_epochs_t epochs={b2_controller_sample.generation,b2_controller_sample.xr_generation,
        b2_client.world_epoch,b2_client.recenter_epoch,b2w_owner_epoch,1};
    return b2w_input_move_serial556 && b2w_input_move_serial556==b2_controller_sample.serial &&
        B2W_EpochsEqual(&epochs,&b2w_input_move_epochs556) &&
        b2_tracked_weapon && b2_tracked_weapon->integer==1 && b2_controller.requested &&
        b2_controller.valid && !b2_controller.wait_neutral && b2_controller.destination==B2C_GAME &&
        b2_controller.serial==b2_controller_sample.serial && b2_controller.generation==epochs.action &&
        b2_controller_sample.xr_generation==B2XR_InputGeneration() &&
        b2_client.walk_ready && !b2_client.walk_release_required && b2_client.origin_latched &&
        !b2_client.stopped && !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.recenter_pending && !b2_client.handoff_position_pending &&
        !b2_client.history.frame_open && !b2_client.ui_interrupted && !B2_UISimulationHeld() &&
        !b2_client.combat_state.dead && cls.state==ca_active && cl.frame.valid &&
        cl.frame.ps.pmove.pm_type==PM_NORMAL && cl.frame.ps.stats[STAT_HEALTH]>0 &&
        cls.key_dest==KEY_GAME && B2_InputFocused556() && B2XR_InteractionReady() &&
        b2c_sample_valid(&b2_controller_sample,now) &&
        !b2c_aim_usable556(&b2_controller_sample,b2_controller_dominant->integer,now);
}

static bool b2w_publish_controller(float yaw, uint32_t now)
{
    b2w_waiting_input=false;b2w_update_configuration();
    b2w_epochs_t epochs={b2_controller_sample.generation,b2_controller_sample.xr_generation,
        b2_client.world_epoch,b2_client.recenter_epoch,b2w_owner_epoch,1};
    B2W_SetContext(true,&epochs,b2_controller.intent.attack,now);
    if (!b2_client.origin_latched || b2_client.recenter_pending || b2_client.handoff_position_pending || b2_client.history.frame_open ||
        b2_client.world_waiting || b2_client.world_rebinding || b2_client.stopped ||
        !b2_controller.valid || b2_controller.wait_neutral || !B2XR_InteractionReady() ||
        B2_UISimulationHeld() || cls.key_dest!=KEY_GAME || !B2_InputFocused556() ||
        b2_client.combat_state.dead || !b2c_sample_valid(&b2_controller_sample,now)) { b2w_last_refusal=__LINE__; goto refuse; }

    // Independent locomotion uses the actual tracked head and current world.
    // No aim packet, selection, teleport or shot is published without an aim.
    if(!b2c_aim_usable556(&b2_controller_sample,b2_controller_dominant->integer,now)){
        b2w_input_move_serial556=b2_controller_sample.serial;b2w_input_move_epochs556=epochs;
        if(!b2w_input_move_only556(now)){b2w_input_move_serial556=0;goto refuse;}
        B2W_CancelSelector(&b2w_selector);b2w_actions_suppressed=true;b2w_pending_token=0;
        memset(&b2w_hud,0,sizeof(b2w_hud));b2w_blocked_preview.active=false;
        B2W_Reset();B2W_SetContext(true,&epochs,false,now);
        b2_controller.intent.attack=false;b2_controller.attack=false;b2_controller.fire_ready556=false;
        if(b2w_published_serial!=b2_controller_sample.serial){
            b2w_space_t space={0};space.epochs=epochs;space.committed=true;
            b2rs_reference(space.reference_origin);space.reference_yaw=b2_client.reference_yaw;
            space.units=b2_units_per_metre->value;
            VectorCopy(b2_client.scene.vieworg,space.anchor_origin);space.anchor_origin[2]+=b2rs_eye();
            vec3_t proposed;VectorCopy(b2_client.input_angles,proposed);
            if(!isfinite(yaw))goto refuse;
            for(unsigned i=0;i<3;i++)if(!isfinite(proposed[i]) || fabsf(proposed[i])>720.f)goto refuse;
            proposed[YAW]=remainderf(proposed[YAW]+yaw,360.f);
            for(unsigned i=0;i<3;i++)space.anchor_angles[i]=SHORT2ANGLE((short)(
                ANGLE2SHORT(proposed[i])+b2_client.anchor_player.delta_angles[i]));
            if(!b2rs_turn_space(yaw,&space) || !b2w_latch_movement(&space,now))goto refuse;
            VectorCopy(proposed,b2_client.input_angles);VectorCopy(proposed,cl.viewangles);
            b2rs_commit_turn(yaw,&space);
        if(yaw){b2_turn_published556=true;b2_turn_ms556=now;}b2w_published_serial=b2_controller_sample.serial;
        }
        b2w_waiting_input=true;return false;
    }
    b2w_input_move_serial556=0;
    b2w_game_api_t bridge;
    b2w_inventory_t inventory, checked;
    const unsigned required=B2W_CAP_INVENTORY|B2W_CAP_PREVIEW|B2W_CAP_SELECTION;
    if (!SV_B2WeaponAPI(&bridge) || (bridge.capabilities&required)!=required ||
        !bridge.inventory(&inventory) || !inventory.epoch || inventory.current<0 || inventory.current>=12 ||
        !(inventory.owned&(1u<<inventory.current))) { b2w_last_refusal=__LINE__; goto refuse; }
    if (b2w_blocked_preview.active &&
        b2w_blocked_preview.pose.serial==b2_controller_sample.serial) {
        if (!b2w_move_only_ready(now)) { b2w_last_refusal=__LINE__; goto refuse; }
        // The same blocked pose cannot regain FIRE by repeated preview calls.
        b2w_waiting_input=true;return false;
    }
    if (!b2w_update_selector(&inventory,&epochs,now)) { b2w_last_refusal=__LINE__; goto refuse; }
    if (!(bridge.capabilities&B2W_CAP_FIRE)) b2w_actions_suppressed=true;
    if (b2w_actions_suppressed) {
        bool hold_turn=!(bridge.capabilities&B2W_CAP_FIRE);
        b2w_suppress_selector_actions(hold_turn);if (hold_turn) yaw=0;
    }
    B2W_SetContext(true,&epochs,b2_controller.intent.attack,now);
    // Neutral command-owned presentation is useful while FIRE is unqualified.
    // FIRE remains blocked; a qualified open selector permits ordinary turning.
    // An unchanged native sample never refreshes its timestamps or integrates
    // another turn. A retired sample cannot bootstrap a new publication.
    if (b2_controller_sample.serial == 0) { b2w_last_refusal=__LINE__; goto refuse; }
    if (b2w_published_serial==b2_controller_sample.serial) {
        if (inventory.epoch!=b2w_published_inventory.epoch) { b2w_last_refusal=__LINE__; goto refuse; }
        if (inventory.current!=b2w_published_inventory.current ||
            inventory.pending!=b2w_published_inventory.pending) {
            // Stock can finish an animation before another native sample.
            // Retire the obsolete publication, retain the deliberate choice,
            // and wait for fresh input rather than recycling this pose.
            B2W_Reset();b2w_waiting_input=true;
            memset(&b2_controller.intent,0,sizeof(b2_controller.intent));
            b2_controller.attack=false;b2_controller.turn_clock_valid=false;
            b2_controller.snap_ready=false;
            return false;
        }
        return B2W_Ready(now);
    }

    b2w_space_t space={0};space.epochs=epochs;space.committed=true;
    b2rs_reference(space.reference_origin);
    space.reference_yaw=b2_client.reference_yaw;space.units=b2_units_per_metre->value;
    VectorCopy(b2_client.scene.vieworg,space.anchor_origin);space.anchor_origin[2]+=b2rs_eye();
    vec3_t proposed;VectorCopy(b2_client.input_angles,proposed);
    if (!isfinite(yaw)) { b2w_last_refusal=__LINE__; goto refuse; }
    for (unsigned i=0;i<3;i++) if (!isfinite(proposed[i]) || fabsf(proposed[i])>720.f) { b2w_last_refusal=__LINE__; goto refuse; }
    proposed[YAW]=remainderf(proposed[YAW]+yaw,360.f);
    if (!isfinite(proposed[YAW])) { b2w_last_refusal=__LINE__; goto refuse; }
    // Match the body basis that the finalised 16-bit command actually transmits.
    // Keep the float accumulator separately so smooth turning loses no fractions.
    for (unsigned i=0;i<3;i++) space.anchor_angles[i]=SHORT2ANGLE((short)(
        ANGLE2SHORT(proposed[i])+b2_client.anchor_player.delta_angles[i]));
    if (!b2rs_turn_space(yaw,&space)) { b2w_last_refusal=__LINE__; goto refuse; }
    if (!b2w_latch_movement(&space,now)) { b2w_last_refusal=__LINE__; goto refuse; }
    b2w_packet_t packet={0};packet.game_epoch=inventory.epoch;packet.weapon=inventory.current;
    packet.slot=-1;packet.attack=b2_controller.intent.attack;
    if (b2w_pending_token && b2w_pending_sent) {
        packet.selection=b2w_pending_token;packet.selection_ms=b2w_pending_ms;
        packet.slot=b2w_pending_slot;packet.attack=false;
    }
    vec3_t corrected;
    b2wp_source_t presentation={0};
    if (!B2W_BuildAim(&b2_controller_sample,b2_controller_dominant->integer,&space,now,&packet.pose)) { b2w_last_refusal=__LINE__; goto refuse; }
    qhandle_t handle=cl.model_draw[cl.frame.ps.gunindex & GUNINDEX_MASK];
    model_t *model=MOD_ForHandle(handle);
    bool blocked=false;
    bool calibrated=B2WP_TryPreview(&packet,&presentation,(uint32_t)handle,model,&bridge,now,&blocked);
    if (calibrated) {
        VectorCopy(space.anchor_origin,presentation.body_origin);
        VectorCopy(space.anchor_angles,presentation.body_angles);
    }
    if (!calibrated && blocked && (bridge.capabilities&B2W_CAP_FIRE)) {
        if (!bridge.inventory(&checked) || checked.epoch!=inventory.epoch ||
            checked.current!=inventory.current || checked.pending!=inventory.pending ||
            !(checked.owned&(1u<<inventory.current))) { b2w_last_refusal=__LINE__; goto refuse; }
        bool first=!b2w_blocked_preview.active;
        B2W_Reset();B2W_SetContext(true,&epochs,false,now);
        B2W_CancelSelector(&b2w_selector);b2w_pending_token=0;
        memset(&b2w_hud,0,sizeof(b2w_hud));
        b2w_actions_suppressed=true;b2_controller.intent.attack=false;b2_controller.attack=false;
        b2_controller.intent.events=0;b2_controller.intent.yaw=0;
        b2w_blocked_preview.active=true;b2w_blocked_preview.pose=presentation.aim;
        b2w_blocked_preview.model=presentation.model;b2w_blocked_preview.handle=presentation.handle;
        b2w_blocked_preview.inventory=checked;
        VectorCopy(proposed,b2_client.input_angles);VectorCopy(proposed,cl.viewangles);
        b2rs_commit_turn(yaw,&space);
        if(yaw){b2_turn_published556=true;b2_turn_ms556=now;}
        b2w_waiting_input=true;
        if (first) { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_WEAPON_PREVIEW rejected movement_only=1 fire=0 display=0 fresh_sample_required=1\n"); }
        return false;
    }
    if (!calibrated && (bridge.capabilities&B2W_CAP_FIRE)) { b2w_last_refusal=__LINE__; goto refuse; }
    // Unrepresented assets retain neutral selector
    // transport only. They never gain FIRE or a body-aim display fallback.
    if ((!calibrated && !bridge.preview(&packet,corrected)) || !bridge.inventory(&checked) ||
        checked.epoch!=inventory.epoch || checked.current!=inventory.current ||
        checked.pending!=inventory.pending || !(checked.owned&(1u<<inventory.current))) { b2w_last_refusal=__LINE__; goto refuse; }
    if (!calibrated) for (unsigned i=0;i<3;i++) if (!isfinite(corrected[i])) { b2w_last_refusal=__LINE__; goto refuse; }
    // packet.pose.origin is the calibrated desired muzzle. Its command-owned
    // preview is a separate engine record, never proof that a shot occurred.
    if (calibrated ? !B2WP_Publish(&packet,&presentation,now) : !B2W_Publish(&packet,now)) { b2w_last_refusal=__LINE__; goto refuse; }
    if (b2w_blocked_preview.active)
        { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_WEAPON_PREVIEW recovered fresh_sample=1\n"); }
    b2w_blocked_preview.active=false;
    b2w_last_refusal=0;
    b2w_published_serial=b2_controller_sample.serial;
    b2w_published_inventory=checked;
    VectorCopy(proposed,b2_client.input_angles);VectorCopy(proposed,cl.viewangles);
        b2rs_commit_turn(yaw,&space);
        if(yaw){b2_turn_published556=true;b2_turn_ms556=now;}
    return true;
refuse:
    b2w_input_move_serial556=0;
    B2W_CancelSelector(&b2w_selector);b2w_actions_suppressed=true;b2w_pending_token=0;
    memset(&b2w_hud,0,sizeof(b2w_hud));b2w_suppress_actions();
    B2W_Reset();
    return false;
}

#include "../../../../baseline/b2-menu-bindings316/source/b2_teleport_client287.inc"

#include "../../../psvr2-default471/source/b2_resume_client471.inc"

#include "../../../inventory342/source/b2_inventory342.inc"

static bool b2_stock_overlay316(unsigned destination,unsigned events)
{
    static bool held_overlay;
    if(destination!=B2C_GAME && destination!=B2C_INVENTORY342){held_overlay=false;b2_screen_phase338=0;return false;}
    bool visible=(cl.frame.ps.stats[STAT_LAYOUTS] & (LAYOUTS_LAYOUT|LAYOUTS_INVENTORY|LAYOUTS_HELP))!=0;
    if(held_overlay && !visible){
        held_overlay=false;b2_screen_phase338=0;
        b2c_clear(&b2_controller);
        b2tp_cancel();B2W_Reset();return true;
    }
    if(events&B2C_OPEN_MENU){
        b2_inventory_cancel342();
        b2_screen_phase338=0;
        if(!visible || (destination!=B2C_INVENTORY342 &&
           !(b2_controller_sample.hands[0].menu || b2_controller_sample.hands[1].menu)))return false;
        b2_menu_route320(&b2_controller_sample,"putaway");
        CL_ClientCommand("putaway");
        b2c_clear(&b2_controller);
        b2tp_cancel();B2W_Reset();return true;
    }
    if(!b2_client.walk_release_required && (events & (B2B_OPEN_INVENTORY316|B2B_OPEN_HELP316|B2B_CYCLE338))){
        b2_inventory_cancel342();
        if(!b2w_selector.open && !b2w_pending_token && !b2_client.recenter_pending &&
           !B2_UISimulationHeld() && B2XR_InteractionReady()){
            if(events&B2B_CYCLE338){
                b2_screen_phase338=(b2_screen_phase338+1)%3;
                char command[32];
                Q_snprintf(command,sizeof(command),"b2_screens338 %u",b2_screen_phase338);
                CL_ClientCommand(command);
            }else{
                b2_screen_phase338=0;
                CL_ClientCommand(events&B2B_OPEN_INVENTORY316?"inven":"help");
            }
        }
        b2tp_cancel();B2W_Reset();return true;
    }
    if(visible && (destination==B2C_INVENTORY342 || B2B_Cycle338() || B2B_Source(B2B_INVENTORY,B2B_Hand())>=0 || B2B_Source(B2B_HELP,B2B_Hand())>=0)){
        held_overlay=true;
        if(destination==B2C_INVENTORY342)b2_inventory_keys342(Sys_Milliseconds());
        memset(&b2_controller.intent,0,sizeof(b2_controller.intent));
        b2tp_cancel();B2W_Reset();return true;
    }
    return false;
}

static bool b2_controller_locomotion556(uint32_t now)
{
    return b2_controller.requested && b2_controller.valid && !b2_controller.wait_neutral &&
        b2_controller.destination==B2C_GAME && b2_client.walk_ready && !b2_client.walk_release_required &&
        !b2_client.stopped && !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.recenter_pending && !b2_client.handoff_position_pending &&
        !b2_client.ui_interrupted && !B2_UISimulationHeld() && !b2_client.combat_state.dead &&
        cl.frame.ps.pmove.pm_type==PM_NORMAL && B2_InputFocused556() && cls.key_dest==KEY_GAME &&
        B2XR_InteractionReady() && b2c_sample_valid(&b2_controller_sample,now) &&
        b2_controller_sample.xr_generation==B2XR_InputGeneration();
}

void B2_ControllerTick(int msec)
{
    b2_smooth_published304=false;
    b2b_poll();
    if(!b2_inventory_feedback342())b2_inventory_cancel342();
    if(!B2B_Cycle338() || cls.key_dest!=KEY_GAME || !B2_InputFocused556() ||
       b2_client.ui_interrupted || b2_client.world_waiting || b2_client.world_rebinding ||
       b2_client.headset_waiting || b2_client.stopped || b2_client.combat_state.dead)
        b2_screen_phase338=0;
    if(!b2_controller_locomotion556(Sys_Milliseconds()) || !b2_comfort_vignette ||
       !b2_comfort_vignette->integer || !b2_comfort_strength ||
       !isfinite(b2_comfort_strength->value) || b2_comfort_strength->value<=0)
        { B2Comfort_Reset308(&b2_comfort_release308);b2_turn_published556=false; }
    (void)msec;
    if (!B2_InputFocused556() || !(cls.key_dest&KEY_MENU) || (cls.key_dest&KEY_CONSOLE)) b2_menu_cancel253();
    static b2cq_state_t quit;
    if(!b2tp_enabled())b2tp_drop301();
    if(B2TP_Expired(Sys_Milliseconds()))b2tp_cancel();
    if ((b2_tracked_weapon && b2_tracked_weapon->integer) || b2w_config_feature) b2w_update_configuration();
    if (B2_UISurfaceActive()) { b2tp_cancel(); b2_surface_tick(Sys_Milliseconds());return; }
    if (!b2_controller_enabled() || (!b2_controller.requested && !b2_client.headset_waiting)) { b2_menu_cancel253(); b2_resume_reset(); b2tp_cancel(); return; }
    if (b2_client.history.frame_open) { b2_menu_cancel253(); b2_stop_at(__func__,__LINE__); return; }
    B2_ProcessInterruptions(); B2_FilterInput();
    if(B2_RealignActive391()){
        B2XR_ReadControllerInput(&b2_controller_sample);
        B2XR_UpdateResumeCue(b2_controller_sample.ready,b2_controller_sample.serial,
            b2_controller_sample.generation,b2_controller_sample.xr_generation);
        b2_realign_tick391(Sys_Milliseconds());return;
    }
    if(b2_client.stopped && B2XR_QuitOnly()){
        b2_menu_cancel253();
        b2c_snapshot_t sample={0};B2XR_PollEvents();
        // Terminal quit has its own focused/fresh XR reader. Gameplay readiness
        // necessarily refuses this state; keyboard mode still needs desktop focus.
        bool ready=(B2_ControllerFocusMode556() || cls.active==ACT_ACTIVATED) &&
            B2XR_ReadQuitInput(&sample);
        if(b2c_quit_step(&quit,&sample,Sys_Milliseconds(),
            (unsigned)b2_controller_dominant->integer,ready)){
            Com_Printf("B2_CONTROLLER_QUIT deliberate_terminal_chord=1 gameplay=0 resume=0\n");
            Cmd_ExecuteString(&cmd_buffer,"quit");
        }
        return;
    }
    memset(&quit,0,sizeof(quit));
    if (b2_client.stopped || (!b2_client.walk_ready && !b2_client.headset_waiting) || b2_client.world_waiting ||
        b2_client.world_rebinding || b2_client.handoff_position_pending) {
        b2_menu_cancel253();b2_resume_reset();b2c_clear(&b2_controller);b2tp_cancel(); return;
    }
    B2XR_ReadControllerInput(&b2_controller_sample);
    if(B2B_Barrier(&b2_controller_sample,b2_controller_dominant->integer)){b2c_clear(&b2_controller);b2_resume_reset();b2tp_cancel();return;}
    B2XR_UpdateResumeCue(b2_client.ui_interrupted && !b2_client.headset_waiting &&
        b2_controller_sample.ready,b2_controller_sample.serial,b2_controller_sample.generation,
        b2_controller_sample.xr_generation);
    if (b2_client.headset_waiting) { b2tp_cancel();b2_headset_start_tick(Sys_Milliseconds()); return; }
    unsigned destination=B2C_BLOCKED;
    if (B2_InputFocused556() && !b2_client.ui_interrupted) {
        if (cls.key_dest&KEY_CONSOLE) destination=B2C_CONSOLE;
        else if (cls.key_dest&KEY_MENU) destination=B2C_MENU;
        else if (b2_client.combat && b2_client.combat_state.dead && !B2_UIPaused()) destination=B2C_DEAD;
        else if (!B2_UIPaused()) {
            if (cl.frame.ps.pmove.pm_type!=PM_FREEZE) destination=B2C_GAME;
            else if (b2_client.lifecycle) destination=B2C_INTERMISSION;
        }
    }
    if(destination==B2C_GAME && cls.key_dest==KEY_GAME &&
       (cl.frame.ps.stats[STAT_LAYOUTS]&LAYOUTS_INVENTORY))destination=B2C_INVENTORY342;
    if(destination!=B2C_INVENTORY342)b2_inventory_cancel342();
    if(destination!=B2C_MENU)b2_menu_cancel253();
    if(destination!=B2C_GAME)b2tp_cancel();
    cvar_t *run=Cvar_FindVar("cl_run");
    float speed=((run && run->integer!=0) ^ (Key_IsDown(K_SHIFT)!=0)) ? 400.f : 200.f;
    uint32_t sample_now=Sys_Milliseconds();
    b2c_state_t previous=b2_controller;
    if (b2c_step_turn(&b2_controller,&b2_controller_sample,sample_now,destination,
        b2_controller_dominant->integer,speed,b2_controller_turn_mode->integer,
        b2_controller_smooth_speed->value,b2_controller_snap_angle->value)) {
        b2c_diagnostic_t diagnostic;
        B2XR_ControllerDiagnostic(&diagnostic);
        Com_Printf("B2_CONTROLLER_LOSS sample=%s read=%s hand=%d capture_age=%u head_age=%u read_head_age=%u generation=%u/%u serial=%llu/%llu dominant=%u/%d turn=%u/%d speed=%.9g destination=%u/%u\n",
            b2c_sample_reason(&b2_controller_sample,sample_now),
            diagnostic.reason?diagnostic.reason:"unrecorded",diagnostic.hand,
            sample_now-b2_controller_sample.capture_ms,sample_now-b2_controller_sample.head_ms,
            diagnostic.now_ms-diagnostic.head_ms,previous.generation,b2_controller_sample.generation,
            (unsigned long long)previous.serial,(unsigned long long)b2_controller_sample.serial,
            previous.dominant,b2_controller_dominant->integer,previous.turn_mode,
            b2_controller_turn_mode->integer,speed,previous.destination,destination);
        bool transient = diagnostic.reason &&
            (!strcmp(diagnostic.reason,"head-age") || !strcmp(diagnostic.reason,"head-generation") ||
             (!strcmp(diagnostic.reason,"interaction-not-ready") && B2XR_Focused582()) ||
             !strcmp(b2c_sample_reason(&b2_controller_sample,sample_now),"capture-age") ||
             !strcmp(b2c_sample_reason(&b2_controller_sample,sample_now),"head-age"));
        b2_menu_cancel253();
        b2_interrupt_kind582("controller-input-unavailable-or-profile-change",
            !transient || b2_manual_interruption582(false)); return;
    }
    if(B2B_TeleportOnly())b2_controller.intent.forward=b2_controller.intent.side=0;
    // The former temporary Y-pause edge now enters the normal menu.
    // It never clears interruption, tracking or lifecycle holds.
    bool pause_ready=b2_controller.requested && B2_UIEnabled() &&
        B2_InputFocused556() && cls.key_dest==KEY_GAME &&
        !b2_client.ui_interrupted && !b2_client.recenter_pending &&
        !b2aw_owned_hold() && B2XR_InteractionReady() &&
        b2_controller_sample.xr_generation==B2XR_InputGeneration() &&
        (destination==B2C_GAME || destination==B2C_INACTIVE || destination==B2C_DEAD ||
         destination==B2C_INTERMISSION || b2_client.ui_manual_pause);
    if (b2c_pause_step(&b2_pause,&b2_controller_sample,sample_now,pause_ready,b2_controller_dominant->integer)) {
        b2tp_cancel();UI_OpenMenu(UIMENU_GAME);return;
    }
    b2_controller_resume_tick(sample_now);
    b2c_intent_t intent=b2_controller.intent;
    if(b2_stock_overlay316(destination,intent.events))return;
    if (intent.events&B2C_OPEN_MENU) { b2_menu_route320(&b2_controller_sample,"game-menu");b2tp_cancel();UI_OpenMenu(UIMENU_GAME); return; }
    if (destination==B2C_CONSOLE) {
        if (intent.events&B2C_BACK) Con_Close(true);
        return;
    }
    if (destination==B2C_MENU) {
        b2_menu_keys253(&b2_controller,&b2_controller_sample,sample_now,3);
        return;
    }
    if (destination==B2C_INTERMISSION || destination==B2C_DEAD) return; // No movement/weapon publication.
    if (destination!=B2C_GAME || b2_client.walk_release_required || b2_controller.wait_neutral) {b2tp_cancel();return;}
    if (b2_tracked_weapon && b2_tracked_weapon->integer) {
        if ((intent.events&B2C_RECENTRE) && !b2w_selector.open && !b2w_selector.rearm && !b2w_pending_token &&
            B2B_Value(&b2_controller_sample,B2B_SELECT,b2_controller_dominant->integer,0)<.35f) {
            b2_recenter_down_f();b2_recenter_up_f();return;
        }
        // A blocked muzzle cancels the selector and rearms it every sample.
        // Remember the deliberate reset before publication clears its events;
        // only a newly validated blocked preview may release this specific gate.
        bool blocked_recenter=(intent.events&B2C_RECENTRE) && b2w_selector.rearm &&
            b2w_blocked_preview.active && !b2w_selector.open && !b2w_pending_token &&
            B2B_Value(&b2_controller_sample,B2B_SELECT,b2_controller_dominant->integer,0)<.35f;
        // Build before asking Ready, so a neutral fresh sample can establish
        // ownership without depending on a publication that does not yet exist.
        b2move_tick301(sample_now); // Supply the admitted nudge to the existing head-heading latch.
        if (!b2w_publish_controller(intent.yaw,Sys_Milliseconds())) {
            if (blocked_recenter && b2w_move_only_ready(sample_now)) {
                b2_recenter_down_f();b2_recenter_up_f();return;
            }
            if (!b2w_waiting_input) b2c_clear(&b2_controller);
            b2tp_cancel();
            // Modified 12 November 2026: a rejected muzzle may still carry
            // independently guarded locomotion. Recheck at this caller boundary;
            // waiting alone never grants movement, firing or teleport authority.
            if (!b2w_waiting_input || (!b2w_move_only_ready(sample_now) && !b2w_input_move_only556(sample_now)))
                b2_controller.intent.forward=b2_controller.intent.side=0;
            b2_smooth_published304=b2_controller.intent.forward!=0 || b2_controller.intent.side!=0;
            b2_smooth_ms304=sample_now;
            return;
        }
        b2_smooth_published304=fabsf(b2_controller.intent.forward)>0 || fabsf(b2_controller.intent.side)>0;
        b2_smooth_ms304=sample_now;
        b2move_commit_run301();
        b2tp_tick(sample_now);
        intent=b2_controller.intent;
        intent.yaw=0; // The successful packet and body turn were committed together.
    }
    if (!(b2_tracked_weapon && b2_tracked_weapon->integer)) {b2move_tick301(sample_now);b2move_commit_run301();intent=b2_controller.intent;}
    if (intent.events&B2C_RECENTRE) { b2_recenter_down_f(); b2_recenter_up_f(); return; }
    if (intent.yaw) {
        b2w_space_t space={0};space.committed=true;space.units=b2_units_per_metre->value;
        space.reference_yaw=b2_client.reference_yaw;b2rs_reference(space.reference_origin);
        VectorCopy(b2_client.scene.vieworg,space.anchor_origin);space.anchor_origin[2]+=b2rs_eye();
        float yaw=remainderf(b2_client.input_angles[YAW]+intent.yaw,360.f);
        for(unsigned i=0;i<3;i++)space.anchor_angles[i]=SHORT2ANGLE((short)(
            ANGLE2SHORT(i==YAW?yaw:b2_client.input_angles[i])+b2_client.anchor_player.delta_angles[i]));
        if(!b2rs_turn_space(intent.yaw,&space)){b2c_clear(&b2_controller);return;}
        b2_client.input_angles[YAW]=yaw;
        VectorCopy(b2_client.input_angles,cl.viewangles);
        b2rs_commit_turn(intent.yaw,&space);
        b2_turn_published556=true;b2_turn_ms556=sample_now;
    }
}

static const cmdreg_t b2_commands[] = {
    { "b2_newgame", b2_newgame_f },
    { "b2_controller_handoff", b2_controller_handoff_f },
    { "b2_controller_status", b2_controller_status_f },
    { "b2_freeze", b2_freeze_f },
    { "b2_wall_start", b2aw_start_f },
    { "b2_stop", b2_stop_f },
    { "+b2_recenter", b2_recenter_down_f },
    { "-b2_recenter", b2_recenter_up_f },
    { NULL }
};

void B2_Init(void)
{
    b2_wall_recipe=Cvar_Get("b2_wall_recipe","0",CVAR_NOSET|CVAR_NOARCHIVE);
    b2aw_init_ms=Sys_Milliseconds();
    if(b2aw_enabled()) Com_Printf("B2_WALL_INIT recipe=%s effective=\"%s\" startup_only=%u default_off=1\n",B2AW_ID,b2_wall_recipe->string,!!(b2_wall_recipe->flags&CVAR_NOSET));
    b2_wall_return=Cvar_Get("b2_wall_return","0",CVAR_NOARCHIVE);
    b2_teleport=Cvar_Get("b2_teleport","1",CVAR_ARCHIVE);
    b2_teleport_range=Cvar_Get("b2_teleport_max_distance","768",CVAR_NOARCHIVE);
    b2_comfort_vignette=Cvar_Get("b2_comfort_vignette","1",CVAR_ARCHIVE);
    b2_comfort_strength=Cvar_Get("b2_comfort_vignette_strength","0.9",CVAR_ARCHIVE);
    b2_roomscale=Cvar_Get("b2_roomscale","0",CVAR_NOSET|CVAR_NOARCHIVE);
    b2_physical_crouch=Cvar_Get("b2_physical_crouch","0",CVAR_NOSET|CVAR_NOARCHIVE);
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2ROOM_INIT roomscale=%d physical_crouch=%d startup_only=1\n",b2_roomscale->integer,b2_physical_crouch->integer); }
    b2_haptics_master=Cvar_Get("b2_haptics","1",CVAR_ARCHIVE);
    b2_haptics_damage=Cvar_Get("b2_haptics_damage","1",CVAR_NOARCHIVE);
    b2_haptics_armour=Cvar_Get("b2_haptics_armor","1",CVAR_NOARCHIVE);
    b2_haptics_bfg=Cvar_Get("b2_haptics_bfg","1",CVAR_NOARCHIVE);
    b2_haptics_weapons=Cvar_Get("b2_haptics_weapons","1",CVAR_NOARCHIVE);
    b2_haptics_master->changed=b2h_options_changed;
    b2_haptics_damage->changed=b2h_options_changed;
    b2_haptics_armour->changed=b2h_options_changed;
    b2_haptics_bfg->changed=b2h_options_changed;
    b2_haptics_weapons->changed=b2h_options_changed;
    b2_world_reticle=Cvar_Get("b2_world_reticle","0",CVAR_NOARCHIVE);
    b2_world_reticle_scale=Cvar_Get("b2_world_reticle_scale","1",CVAR_NOARCHIVE);
    b2_input_mode484=Cvar_Get("b2_input_mode","1",CVAR_ARCHIVE);
    b2_controller_input=Cvar_Get("b2_controller_input","0",CVAR_NOSET|CVAR_NOARCHIVE);
    b2_controller_dominant=Cvar_Get("b2_controller_dominant_hand","1",CVAR_NOSET|CVAR_NOARCHIVE);
    b2_controller_turn_mode=Cvar_Get("b2_controller_turn_mode","1",CVAR_ARCHIVE);
    b2_controller_smooth_speed=Cvar_Get("b2_controller_smooth_speed","360",CVAR_ARCHIVE);
    b2_controller_snap_angle=Cvar_Get("b2_controller_snap_angle","45",CVAR_ARCHIVE);
    Cvar_Get("b2_telefrag","0",CVAR_ARCHIVE);
    b2_controller_resume=Cvar_Get("b2_controller_resume","0",CVAR_NOSET|CVAR_NOARCHIVE);
    b2_headset_start=Cvar_Get("b2_headset_start","0",CVAR_NOSET|CVAR_NOARCHIVE);
    b2_give_all=Cvar_Get("b2_give_all","0",CVAR_NOSET|CVAR_NOARCHIVE);
    b2_tracked_weapon=Cvar_Get("b2_tracked_weapon","0",CVAR_NOARCHIVE);
    cvar_t *requested_lifecycle = Cvar_FindVar("b2_lifecycle");
    int lifecycle_request = requested_lifecycle ? requested_lifecycle->integer : 0;
    b2_lifecycle = Cvar_Get("b2_lifecycle", "0", CVAR_NOSET | CVAR_NOARCHIVE);
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2LIFE_INIT requested=%d effective=%d startup_only=%d\n",
        lifecycle_request, b2_lifecycle->integer, !!(b2_lifecycle->flags & CVAR_NOSET)); }
    b2_same_map_ui = Cvar_Get("b2_same_map_ui", "0", CVAR_NOARCHIVE);
    b2_combat = Cvar_Get("b2_combat", "0", CVAR_NOARCHIVE);
    b2_units_per_metre = Cvar_Get("b2_units_per_metre", "39.37007874", CVAR_PRIVATE | CVAR_NOARCHIVE);
    b2_advance_world = Cvar_Get("b2_advance_world", "0", CVAR_PRIVATE | CVAR_NOARCHIVE);
    b2_keyboard_locomotion = Cvar_Get("b2_keyboard_locomotion", "0", CVAR_PRIVATE | CVAR_NOARCHIVE);
    b2_jump_crouch = Cvar_Get("b2_jump_crouch", "0", CVAR_NOARCHIVE);
    b2_turning_mouselook = Cvar_Get("b2_turning_mouselook", "0", CVAR_NOARCHIVE);
    b2_blaster_fire = Cvar_Get("b2_blaster_fire", "0", CVAR_NOARCHIVE);
    b2_visual_target = Cvar_Get("b2_visual_target", "0", CVAR_NOARCHIVE);
    b2_target_fault = Cvar_Get("b2_target_fault", "", CVAR_PRIVATE | CVAR_NOARCHIVE | CVAR_ROM);
    b2_walk_fault = Cvar_Get("b2_walk_fault", "", CVAR_PRIVATE | CVAR_NOARCHIVE | CVAR_ROM);
    Cmd_Register(b2_commands);
}

void B2_BeforeRendererShutdown(void)
{
    B2H_Reset();
    R_B2UIReset();
    if (B2_KeyboardMode())
        b2_clear_walk_input();
    if (B2XR_Requested())
        B2XR_RequestStop();
    if (b2_client.pending || b2_client.frozen || b2_client.stopped)
        memset(&cl.refdef, 0, sizeof(cl.refdef));
    b2_release_scene();
}

void B2_Shutdown(void)
{
    B2_BeforeRendererShutdown();
    Cmd_Deregister(b2_commands);
}


// Narrow existing-event poll before local server execution; no input/yaw tick.
void B2W_PreServerFrame(void)
{
    bool requested=b2_tracked_weapon && b2_tracked_weapon->integer!=0 && b2_controller.requested;
    if ((b2_tracked_weapon && b2_tracked_weapon->integer) || b2w_config_feature) b2w_update_configuration();
    if (!requested && !B2W_Requested()) return;
    if (requested) b2w_update_configuration();
    if (requested && !b2_client.history.frame_open) B2_ProcessInterruptions();
    bool intermission_attack=b2_controller_intermission_attack(Sys_Milliseconds());
    bool death_attack=B2_ControllerDeadAttack(Sys_Milliseconds());
    // Preserve the release latch independently of the one-sample death edge.
    // Clearing it here would manufacture another edge while fire stays held.
    bool death_held=b2_controller.destination==B2C_DEAD && b2_controller.attack;
    b2w_epochs_t epochs={b2_controller_sample.generation,b2_controller_sample.xr_generation,
        b2_client.world_epoch,b2_client.recenter_epoch,b2w_owner_epoch,1};
    bool eligible=requested && b2_controller_enabled() && b2_controller.valid && !b2_controller.wait_neutral &&
        b2_client.walk_ready && !b2_client.walk_release_required && !b2_client.stopped &&
        b2_client.origin_latched && !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.combat_state.dead && cl.frame.ps.pmove.pm_type!=PM_FREEZE &&
        !b2_client.recenter_pending && !b2_client.handoff_position_pending && !b2_client.history.frame_open && !B2_UISimulationHeld() &&
        cls.key_dest==KEY_GAME && B2_InputFocused556() && b2c_sample_valid(&b2_controller_sample,Sys_Milliseconds());
    B2W_SetContext(requested,&epochs,eligible && b2_controller.intent.attack,Sys_Milliseconds());
    if (!eligible) {
        B2W_Reset();B2W_CancelSelector(&b2w_selector);b2w_actions_suppressed=true;b2w_pending_token=0;
        memset(&b2w_hud,0,sizeof(b2w_hud));b2w_suppress_actions();
        // Weapon context remains disabled. Preserve only currently admitted
        // lifecycle input for the server's fresh-command checks, never firing.
        b2_controller.intent.attack=intermission_attack || death_attack;
        b2_controller.attack=intermission_attack || death_held;
    }
}

bool B2WN_Enabled(void)
{
    return cls.ref_type==REF_TYPE_VKPT && B2XR_Requested() && B2_AdvancingWorld() &&
        b2_client.combat && b2_client.origin_latched && b2_client.walk_ready &&
        !b2_client.walk_release_required && !b2_client.stopped && !b2_client.world_waiting &&
        !b2_client.world_rebinding && !b2_client.recenter_pending && !b2_client.handoff_position_pending &&
        !B2_UISimulationHeld() && cls.state==ca_active && B2_InputFocused556() &&
        cls.key_dest==KEY_GAME && cl.frame.valid && cl.frame.ps.pmove.pm_type==PM_NORMAL &&
        !b2_client.combat_state.dead && B2XR_InteractionReady();
}
static void b2wn_offsets(const b2wn_state_t *s, float origin[3])
{
    vec3_t f,r,u;AngleVectors(s->body_angles,f,r,u);
    VectorCopy(s->origin,origin);
    VectorMA(origin,cl_gun_y->value,f,origin);
    VectorMA(origin,cl_gun_x->value,r,origin);
    VectorMA(origin,cl_gun_z->value,u,origin);
}
bool B2WN_Sample(const b2wn_state_t *s, const b2we_owner_t *owner, b2wn_pose_t *out)
{
    if (!out || !s || s->size!=sizeof(*s) || s->version!=B2WN_VERSION || !B2WN_Enabled() ||
        !s->epoch || s->slot<0 || s->slot>=12 || s->gunindex<=0 || s->gunindex>=cl.csr.max_models ||
        s->gunframe<0 || s->model_frame<0 || (uint32_t)(Sys_Milliseconds()-s->captured_ms)>100) return false;
    for (unsigned i=0;i<3;i++) if (!isfinite(s->body_origin[i]) || !isfinite(s->body_angles[i]) ||
        !isfinite(s->origin[i]) || !isfinite(s->angles[i]) || !isfinite(s->recoil[i])) return false;
    uint32_t handle=(uint32_t)cl.model_draw[s->gunindex];model_t *model=MOD_ForHandle(handle);
    b2wm_info_t info;b2wm_datum_t datum;float matrix[16]={0};int hand;
    if (!handle || !B2WM_Query(model,B2WM_VERSION,sizeof(info),&info) || info.key.slot!=(unsigned)s->slot ||
        s->model_frame>=info.frames) return false;
    int model_frame=owner && s->idle?info.ready:s->model_frame;
    if (!B2WM_Datum(model,&info.key,model_frame,model_frame,0,&datum)) return false;
    memset(out,0,sizeof(*out));
    if (owner) {
        b2we_owner_t current;b2w_pose_t aim;vec3_t recoil;
        if (!B2WE_OwnerAt(&current,Sys_Milliseconds()) || memcmp(owner,&current,sizeof(current)) ||
            owner->handle!=handle || memcmp(&owner->model,&info.key,sizeof(info.key)) ||
            owner->packet.game_epoch!=s->epoch) return false;
        VectorCopy(s->recoil,recoil);
        if (!B2WM_Recoil(&owner->aim,recoil,&aim)) return false;
        VectorCopy(owner->packet.pose.origin,out->anchor);
        if (info.requires_body_partition) {
            b2wm_part_t parts[3];
            if (!B2WM_GrenadeParts(model,&info.key,model_frame,model_frame,0,&aim,out->anchor,true,parts)) return false;
            memcpy(matrix,parts[0].matrix,sizeof(matrix));
        } else if (!B2WM_Matrix(model,&info.key,model_frame,model_frame,0,&aim,out->anchor,matrix)) return false;
        hand=owner->aim.hand==0?-1:1;
    } else {
        if (b2_controller.requested || s->hand<0 || s->hand>1) return false;
        vec3_t axes[3],origin;AnglesToAxis(s->angles,axes);b2wn_offsets(s,origin);
        hand=s->hand==1?-1:1;
        for (unsigned row=0;row<3;row++) {
            for (unsigned col=0;col<3;col++) matrix[col*4+row]=axes[col][row]*info.scale*(col==1?hand:1);
            matrix[12+row]=origin[row];out->anchor[row]=origin[row];
        }
        matrix[15]=1;
        if (info.key.slot==8 && !B2WM_HyperStabilise(model,&info.key,
                model_frame,model_frame,0,false,matrix)) return false;
    }
    if (!B2WM_Muzzle(&datum,matrix,hand,&out->pose)) return false;
    if (!owner && !b2_controller.requested && (s->slot==6 || s->slot==11))
        AngleVectors(s->body_angles,out->pose.forward,out->pose.right,out->pose.up);
    return true;
}
// Smooth only horizontal gun translation and yaw sway relative to the body.
// Z, pitch, roll, recoil timing, body motion and gameplay remain instantaneous.
// A bounded 40ms response discards stalls instead of integrating catch-up.
static void b2wn_horizontal(const b2wn_state_t *s,float offset[3],float *yaw,uint32_t now)
{
    uint32_t dt=now-b2wn_visual.ms;
    bool reset=!b2wn_visual.valid || dt>100 || b2wn_visual.game!=s->epoch ||
        b2wn_visual.world!=b2_client.world_epoch || b2wn_visual.recentre!=b2_client.recenter_epoch ||
        b2wn_visual.owner!=b2w_owner_epoch || b2wn_visual.slot!=s->slot || b2wn_visual.model!=s->gunindex;
    if (!isfinite(offset[0]) || !isfinite(offset[1]) || !isfinite(*yaw)) {
        b2wn_visual.valid=false;return;
    }
    float alpha=reset?1.f:min(1.f,(float)dt/40.f);
    for (unsigned i=0;i<2;i++) {
        if (reset) b2wn_visual.xy[i]=offset[i];
        else b2wn_visual.xy[i]+=alpha*(offset[i]-b2wn_visual.xy[i]);
        offset[i]=b2wn_visual.xy[i];
    }
    if (reset) b2wn_visual.yaw=*yaw;
    else b2wn_visual.yaw=LerpAngle(b2wn_visual.yaw,*yaw,alpha);
    *yaw=b2wn_visual.yaw;
    b2wn_visual.valid=true;b2wn_visual.ms=now;b2wn_visual.game=s->epoch;
    b2wn_visual.world=b2_client.world_epoch;b2wn_visual.recentre=b2_client.recenter_epoch;
    b2wn_visual.owner=b2w_owner_epoch;b2wn_visual.slot=s->slot;b2wn_visual.model=s->gunindex;
}

// Read-only game motion supplies the B2 KBM model's recoil. Prediction and
// interpolation remain presentation-only; neither is sent back to firing.
bool B2WN_View(entity_t *gun)
{
    player_state_t *ps = CL_KEYPS, *ops = CL_OLDKEYPS;
    b2w_game_api_t bridge;b2wn_state_t s;b2wm_info_t info;
    if (!gun || !B2WN_Enabled() || b2_controller.requested || !SV_B2WeaponAPI(&bridge) ||
        !bridge.motion || !bridge.motion(&s) || s.version!=B2WN_VERSION || s.size!=sizeof(s) ||
        s.gunindex<=0 || s.gunindex>=cl.csr.max_models || (uint32_t)gun->model!=(uint32_t)cl.model_draw[s.gunindex] ||
        !B2WM_Query(MOD_ForHandle(gun->model),B2WM_VERSION,sizeof(info),&info) || info.key.slot!=(unsigned)s.slot) { b2wn_visual.valid=false;return false; }
    // Render the ordinary interpolated animation, sway and kick. This read-only
    // pose may trail simulation emission by one keyframe; it never owns fire.
    vec3_t body,kick,forward,right,up,offset;
    float yaw=LerpAngle(ops->gunangles[YAW],ps->gunangles[YAW],CL_KEYLERPFRAC);
    for (unsigned i=0;i<3;i++)
        offset[i]=ops->gunoffset[i]+CL_KEYLERPFRAC*(ps->gunoffset[i]-ops->gunoffset[i]);
    b2wn_horizontal(&s,offset,&yaw,Sys_Milliseconds());
    LerpAngles(ops->kick_angles,ps->kick_angles,CL_KEYLERPFRAC,kick);
    for (unsigned i=0;i<3;i++) {
        body[i]=cl.refdef.viewangles[i]-(cl_kickangles->integer?kick[i]:0);
        gun->origin[i]=cl.refdef.vieworg[i]+offset[i];
        gun->angles[i]=body[i]+(i==YAW?yaw:LerpAngle(ops->gunangles[i],ps->gunangles[i],CL_KEYLERPFRAC))+kick[i];
    }
    AngleVectors(body,forward,right,up);
    VectorMA(gun->origin,cl_gun_y->value,forward,gun->origin);
    VectorMA(gun->origin,cl_gun_x->value,right,gun->origin);
    VectorMA(gun->origin,cl_gun_z->value,up,gun->origin);
    VectorCopy(gun->origin,gun->oldorigin);return true;
}

bool B2W_Current(b2w_packet_t *out) { return B2W_CurrentAt(out,Sys_Milliseconds()); }
bool B2WE_Owner(b2we_owner_t *out)
{
    b2wp_record_t presentation;
    b2wm_info_t info;
    if (!out || !B2WE_OwnerAt(out,Sys_Milliseconds()) || !B2W_Presentation(&presentation)) return false;
    model_t *model=MOD_ForHandle(out->handle);
    if (!B2WM_Query(model,B2WM_VERSION,sizeof(info),&info) ||
        info.key.slot!=out->model.slot || info.key.calibration!=out->model.calibration ||
        info.key.generation!=out->model.generation || memcmp(info.key.sha256,out->model.sha256,32)) return false;
    return true;
}
bool B2WE_Record(const b2we_event_t *event)
{
    b2we_owner_t owner;
    return event && B2WE_Owner(&owner) && !memcmp(&owner,&event->owner,sizeof(owner)) &&
        B2WE_RecordAt(event,Sys_Milliseconds());
}



bool B2W_Presentation(b2wp_record_t *out)
{
    b2w_game_api_t bridge;
    b2w_inventory_t inventory;
    if (!out || !B2W_HideBodyReticle() || b2_client.stopped || b2_client.recenter_pending || b2_client.handoff_position_pending ||
        b2_client.world_waiting || b2_client.world_rebinding || !b2_controller.valid ||
        b2_controller.wait_neutral || !B2XR_InteractionReady() || B2_UISimulationHeld() ||
        cls.key_dest!=KEY_GAME || !B2_InputFocused556() || b2_client.combat_state.dead ||
        !B2WP_CurrentAt(out,Sys_Milliseconds()) || !SV_B2WeaponAPI(&bridge) ||
        !bridge.inventory(&inventory) || inventory.epoch!=out->packet.game_epoch ||
        inventory.current!=out->packet.weapon || !(inventory.owned&(1u<<inventory.current))) return false;
    b2wn_state_t motion;
    if (!bridge.motion || !bridge.motion(&motion) || motion.epoch!=out->packet.game_epoch ||
        motion.slot!=out->packet.weapon || motion.size!=sizeof(motion) || motion.version!=B2WN_VERSION) return false;
    out->emission_pose=motion.emission;
    if (motion.emission && (!out->emission_serial || motion.frame!=out->emission_frame ||
        motion.gunframe!=out->emission_gunframe)) return false;
    VectorCopy(motion.recoil,out->recoil);
    out->idle_pose=motion.idle;out->model_frame=motion.model_frame;
    if (motion.idle) {
        b2wm_info_t info;
        if (!B2WM_Query(MOD_ForHandle(out->source.handle),B2WM_VERSION,sizeof(info),&info) ||
            info.key.slot!=(unsigned)motion.slot) return false;
        out->model_frame=info.ready;
    }
    if (out->packet.weapon==5) {
        b2we_owner_t owner;
        if (!bridge.grenade || !B2WE_OwnerAt(&owner,Sys_Milliseconds()) ||
            !bridge.grenade(&owner,&out->grenade)) return false;
    }
    return true;
}
