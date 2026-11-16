// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Source151: UI owns no world, weapon packet, prediction or temporal history.
bool B2_MenuSession(void)
{
    return B2XR_Requested() && !B2XR_StopRequested() && !b2_client.stopped &&
        b2_controller_enabled() && b2_lifecycle && b2_lifecycle->integer==1 &&
        b2_same_map_ui && b2_same_map_ui->integer==1 && b2_headset_start &&
        b2_headset_start->integer==1 && cls.ref_type==REF_TYPE_VKPT &&
        !cls.demo.playback && b2_exclusive();
}

static bool b2_cinematic_preparing;
static b2cs_state_t b2_cinematic_skip;
static unsigned b2_cinematic_boundary;
void B2_CinematicBoundary(bool preparing)
{
    b2_cinematic_preparing=preparing;
    b2_cinematic_boundary++;
    b2cs_reset(&b2_cinematic_skip);
}
float B2_CinematicSkipProgress(void) { return b2_cinematic_skip.progress; }

bool B2_UISurfaceActive(void)
{
    return B2_MenuSession() && (b2_cinematic_preparing || cls.state==ca_cinematic || cls.state==ca_disconnected ||
        (!b2_client.world_epoch && cls.state<ca_active));
}

bool B2_MenuCommand(const char *command)
{
    if (!B2_MenuSession() || !B2_InputFocused556() ||
        !B2XR_InteractionReady() || b2_client.history.frame_open ||
        b2_client.world_rebinding) return false;
    // An opened campaign film or victory picture has retired the old world;
    // it will never publish a gameplay frame to clear its waiting flags.
    // Permit only the deliberate New Game command from that local surface.
    // Keep the old world retired and let the stock parser/save/spawn route
    // validate and establish the replacement, as it does at first startup.
    bool cinematic_restart = !strcmp(command,"map *ntro.cin+base1 force") &&
        cls.state==ca_cinematic && !b2_cinematic_preparing &&
        b2_client.world_waiting && b2_client.world_epoch &&
        b2_client.world_epoch!=UINT_MAX && cls.serverAddress.type==NA_LOOPBACK &&
        sv_running->integer && b2_world_wait_valid();
    if (((b2_client.ui_interrupted && !b2_client.headset_waiting) &&
         !B2_DropMenu555() && !cinematic_restart) ||
        (b2_client.world_waiting &&
         !(B2_DropMenu555() && B2_UISurfaceActive()) && !cinematic_restart)) return false;
    // Called synchronously only by deliberate new-game/load menu actions.
    // The stock command validates the target before retiring a running world.
    unsigned before=b2_client.world_epoch;
    unsigned cinematic_before=b2_cinematic_boundary;
    bool server_before=sv_running->integer!=0;
    b2_menu_transition=true;
    b2_menu_interruption=B2XR_InterruptionEpoch();
    b2_clear_walk_input(); CL_B2ReleaseButtons();
    // Clear the difficulty menu before dispatch, including the missing-cinematic
    // fallback. A cold client keeps its connection console until stock cinematic
    // or map activation closes it; an active client can close it immediately.
    UI_OpenMenu(UIMENU_NONE); Con_Close(true);
    Cmd_ExecuteString(&cmd_buffer,command);
    if (cinematic_restart) {
        // A valid forced restart synchronously stops the old cinematic through
        // SV_InitGame/CL_Disconnect. A parser failure leaves that boundary intact.
        // The already retired world need not acquire another epoch here.
        if (b2_cinematic_boundary==cinematic_before) b2_menu_transition=false;
    } else if (b2_client.world_epoch==before && !(!before && !server_before && sv_running->integer) &&
        cls.state!=ca_loading &&
        cls.state!=ca_connected && cls.state!=ca_connecting && cls.state!=ca_cinematic)
        b2_menu_transition=false; // Failed/missing save must not arm a later load.
    return true;
}

static void b2_surface_tick(uint32_t now)
{
    static b2c_state_t ui;
    static b2cp_state_t entry;
    static int previous_state=-1;
    static unsigned previous_boundary;
    static unsigned binding_revision;
    b2c_snapshot_t sample={0};
    bool ready=B2_InputFocused556() && B2XR_InteractionReady() &&
        B2XR_ReadControllerInput(&sample) && sample.xr_generation==B2XR_InputGeneration();
    if(binding_revision!=B2B_Revision()){binding_revision=B2B_Revision();b2c_clear(&ui);memset(&entry,0,sizeof(entry));b2cs_reset(&b2_cinematic_skip);}
    if(B2B_Barrier(&sample,b2_controller_dominant->integer)){b2c_clear(&ui);memset(&entry,0,sizeof(entry));b2cs_reset(&b2_cinematic_skip);return;}
    // Keyboard Home can request the same reset before this controller tick.
    // Cancel partial skip/menu input and require neutral after either route.
    if(b2_client.recenter_pending){
        b2c_clear(&ui);memset(&entry,0,sizeof(entry));
        b2cs_reset(&b2_cinematic_skip);b2_menu_cancel253();return;
    }
    if (previous_state!=cls.state || previous_boundary!=b2_cinematic_boundary || !ready) {
        b2c_clear(&ui);memset(&entry,0,sizeof(entry));b2cs_reset(&b2_cinematic_skip);
    }
    previous_state=cls.state;
    previous_boundary=b2_cinematic_boundary;
    ui.requested=true;
    unsigned dest=!ready?B2C_BLOCKED:(cls.key_dest&KEY_CONSOLE)?B2C_CONSOLE:
        (cls.key_dest&KEY_MENU)?B2C_MENU:cls.state==ca_cinematic?B2C_CINEMATIC:B2C_INACTIVE;
    bool fresh304=sample.serial!=ui.serial;
    b2c_step_mode(&ui,&sample,now,dest,b2_controller_dominant->integer,0,
        b2_controller_turn_mode->integer);
    if (dest!=B2C_MENU) b2_menu_cancel253();
    b2c_intent_t intent=ui.intent;
    if(fresh304 && ready && (dest==B2C_MENU || dest==B2C_CINEMATIC) && !ui.wait_neutral && !UI_B2BindingCapture304() &&
       (intent.events&B2C_RECENTRE)){
        b2_recenter_down_f();b2_recenter_up_f();
        b2c_clear(&ui);memset(&entry,0,sizeof(entry));b2cs_reset(&b2_cinematic_skip);b2_menu_cancel253();return;
    }
    bool held_skip=b2cs_step(&b2_cinematic_skip,&sample,now,
        ready && dest==B2C_CINEMATIC && !ui.wait_neutral,
        b2_controller_dominant->integer,B2XR_InterruptionEpoch());
    if (b2c_pause_mode316(&entry,&sample,now,ready && cls.key_dest==KEY_GAME,b2_controller_dominant->integer,dest==B2C_CINEMATIC)) {
        b2cs_reset(&b2_cinematic_skip);UI_OpenMenu(UIMENU_MAIN);return;
    }
    if (intent.events&B2C_OPEN_MENU) { b2_menu_route320(&sample,"main-menu");b2cs_reset(&b2_cinematic_skip);UI_OpenMenu(UIMENU_MAIN); }
    else if (dest==B2C_CINEMATIC && ((intent.events&B2C_CONFIRM) || held_skip)) {
        b2cs_reset(&b2_cinematic_skip);
        b2c_clear(&ui);
        SCR_FinishCinematic(); // Same completion/nextserver route as stock input.
    } else if (dest==B2C_CONSOLE && (intent.events&B2C_BACK)) {
        // With no world, stock Con_Close refuses until a menu exists.
        // Reuse the deliberate reducer edge; never infer a button press.
        b2_menu_route320(&sample,"startup-console-menu");
        UI_OpenMenu(UIMENU_MAIN); Con_Close(true);
    }
    else if (dest==B2C_MENU) {
        b2_menu_keys253(&ui,&sample,now,1);
    }
}

static bool b2_render_surface(void)
{
    static uint32_t pair;
    static bool reference_valid;
    static uint32_t reference_generation;
    static float reference[3],yaw;
    b2_frame_t frame={0}; b2_view_t head={0};
    const float zero[3]={0};
    refdef_t saved=cl.refdef;
    if (!B2XR_BeginFrame(&frame)) { b2_realign_finish391(false);return true; }
    bool valid=frame.should_render && frame.tracking_valid &&
        B2_InputFocused556() && B2XR_InteractionReady() && B2XR_InitialReferenceReady();
    if (!valid) {
        b2_realign_finish391(false);
        b2_client.recenter_pending=false;
        R_B2UIReset(); reference_valid=false;
        if (!B2XR_EndFrame(false)) b2_stop_at(__func__,__LINE__);
        R_B2PairFinish(false); return true;
    }
    float proposed_reference[3],proposed_yaw=yaw;
    memcpy(proposed_reference,reference,sizeof(reference));
    bool reset_reference=!reference_valid || reference_generation!=B2XR_InputGeneration() || b2_client.recenter_pending || B2_RealignActive391();
    if (reset_reference) {
        valid=b2_recenter_pose304(&frame,proposed_reference,&proposed_yaw);
        if (valid) R_B2UIReset();
    }
    frame.logical_index=pair;
    frame.history_valid=false;
    for (unsigned eye=0;valid && eye<frame.view_count;eye++) {
        float p[3],q[4];
        b2_relative_pose(frame.positions[eye],frame.orientations[eye],proposed_reference,proposed_yaw,p,q);
        valid=B2_BuildView(p,q,frame.fov_lrud[eye],zero,zero,b2_units_per_metre->value,&frame.views[eye]);
    }
    // The head helper uses the scene's body angles; UI has an independent zero body.
    vec3_t saved_angles; VectorCopy(b2_client.scene.viewangles,saved_angles);
    VectorClear(b2_client.scene.viewangles);
    if (valid) valid=b2_head_view(&frame,proposed_yaw,&head);
    VectorCopy(saved_angles,b2_client.scene.viewangles);
    for (unsigned eye=0;valid && eye<frame.view_count;eye++) {
        memset(&cl.refdef,0,sizeof(cl.refdef));
        cl.refdef.width=B2G_Width();cl.refdef.height=B2G_Height();
        cl.refdef.rdflags=RDF_NOWORLDMODEL;
        cl.refdef.b2_active=true;cl.refdef.b2_frame=&frame;cl.refdef.b2_eye=eye;
        R_BeginFrame();R_RenderFrame(&cl.refdef);
        if (!eye) {
            if(B2_RealignActive391())SCR_DrawB2Realignment391();else SCR_DrawB2Surface();
        }
        R_B2HUD(eye,&head);R_EndFrame();
        valid=cl.refdef.b2_render_accepted;
    }
    bool ended=B2XR_EndFrame(valid);
    bool committed=R_B2PairFinish(valid && ended);
    cl.refdef=saved;
    extern bool R_B2SettingsRejected580(void);
    if (!valid && ended && !B2XR_StopRequested() && R_B2SettingsRejected580()) {
        reference_valid=false;
        b2_realign_finish391(false);
        return true; // A paced zero-layer frame, not a terminal UI failure.
    }
    if (!valid || !ended || !committed || pair==UINT32_MAX) {
        R_B2UIReset();reference_valid=false;b2_client.recenter_pending=false;
        b2_stop_at(__func__,__LINE__);
    } else {
        if(reset_reference){
            memcpy(reference,proposed_reference,sizeof(reference));yaw=proposed_yaw;
            reference_valid=true;reference_generation=B2XR_InputGeneration();b2_client.recenter_pending=false;
        }
        pair++;
    }
    b2_realign_finish391(valid && ended && committed);
    // No world history, player state or per-eye audio update is published.
    return true;
}
