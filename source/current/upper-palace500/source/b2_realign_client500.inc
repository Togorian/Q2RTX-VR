// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Explicit new physical alignment, never a reconstruction of the lost space.
bool B2_RealignActive391(void) { return b2_realign.active && !b2_client.stopped; }
// SPDX-License-Identifier: GPL-2.0-or-later
// Reference ownership is independent of whether gameplay/alive actions may run.
static struct {
    bool captured,automatic,known,staged,manual,interrupted;
    int destination,pm_type,health;
    uint32_t world,epoch,owner,revision,generation;
} b2_reset397;
bool B2_ReferenceBasisAllowed397(void)
{
    return b2_headset_start && b2_headset_start->integer==1 &&
        b2_controller_resume && b2_controller_resume->integer==1 &&
        B2_MenuSession() && !b2_client.stopped;
}
bool B2_RealignAllowed391(void)
{
    return B2_ReferenceBasisAllowed397() && b2_controller.requested &&
        B2_UIEnabled() && b2_client.frozen && b2_client.walk_ready &&
        b2_client.origin_latched && !b2_client.headset_waiting &&
        !b2_client.world_waiting && !b2_client.world_rebinding &&
        cls.state==ca_active && cl.frame.valid;
}
void B2_ReferenceReset397(bool stable,bool known,uint32_t previous_epoch)
{
    // Called from event polling before any recovery hold. Never restore this
    // player's bytes: the live server/world remains the sole state authority.
    bool repeat=b2_reset397.captured;
    if(!repeat){
        b2_reset397.manual=b2_client.ui_manual_pause;
        b2_reset397.destination=cls.key_dest;
        b2_reset397.interrupted=b2_client.ui_interrupted;
        b2_reset397.pm_type=cl.frame.ps.pmove.pm_type;
        b2_reset397.health=cl.frame.ps.stats[STAT_HEALTH];
        b2_reset397.world=b2_client.world_epoch;
    }
    b2_reset397.automatic=!repeat && stable && B2_RealignAllowed391() &&
        B2_InputFocused556() && previous_epoch==b2_client.ui_xr_epoch &&
        !b2_reset397.interrupted && !b2aw_owned_hold() &&
        !b2_client.recenter_pending && !b2_client.handoff_position_pending;
    b2_reset397.captured=true;b2_reset397.known=known;b2_reset397.staged=false;
    b2_reset397.epoch=B2XR_InterruptionEpoch();
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REFERENCE snapshot automatic=%d known=%d manual=%d destination=%d interrupted=%d pm=%d health=%d world=%u cause=unattributed\n",
        b2_reset397.automatic,known,b2_reset397.manual,b2_reset397.destination,
        b2_reset397.interrupted,b2_reset397.pm_type,b2_reset397.health,b2_reset397.world);
}
static void b2_reference_fallback397(void)
{
    if(!b2_reset397.captured)return;
    b2_reset397.automatic=false;b2_reset397.staged=false;
    b2_realign.pending=b2_realign.armed=b2_realign.pressed=b2_realign.key_pressed=false;
    b2_realign.visible=false;
}
static void b2_reference_retire397(void)
{
    b2_reference_fallback397();
    memset(&b2_realign,0,sizeof(b2_realign));
}
static void b2_realign_begin391(void)
{
    if(!B2XR_RealignPending391())return;
    // An ordinary lifecycle transition retires the old hold; wait for its new
    // world/initial reference instead of restoring an old scene or refusing XR.
    if(!B2_RealignAllowed391()){
        b2_reference_fallback397();
        memset(&b2_realign,0,sizeof(b2_realign));return;
    }
    unsigned epoch=B2XR_InterruptionEpoch();
    if(b2_reset397.epoch!=epoch || b2_reset397.world!=b2_client.world_epoch ||
       !B2_InputFocused556() ||
       b2_reset397.destination!=cls.key_dest ||
       b2_reset397.manual!=b2_client.ui_manual_pause)b2_reference_fallback397();
    if(b2_reset397.known && !b2_reset397.automatic){
        // Known continuity already preserves the application coordinates.
        // Removal/focus recovery keeps its existing single deliberate resume.
        if(b2_realign.active || b2_realign.epoch!=epoch){
            memset(&b2_realign,0,sizeof(b2_realign));b2_realign.epoch=epoch;
            b2_interrupt("reference-continuity");
        }
        return;
    }
    if(b2_realign.active && b2_realign.epoch==epoch)return;
    memset(&b2_realign,0,sizeof(b2_realign));
    b2_realign.active=true;b2_realign.epoch=epoch;
    b2_interrupt("reference-realignment");
    // Packet completion at the pause boundary may have published death/stance.
    b2_realign.duck=(cl.frame.ps.pmove.pm_flags&PMF_DUCKED)!=0;
    b2_client.recenter_pending=b2_client.handoff_position_pending=false;
    b2_client.ui_resume_request=b2_client.ui_recovery_ready=false;
    b2_client.history.valid=false;b2_client.audio_pending=false;R_B2UIReset();
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REALIGN waiting automatic=%d body_unchanged=1 duck=%d\n",
        b2_reset397.automatic,b2_realign.duck);
}
static bool b2_reference_sample397(uint32_t now)
{
    const b2c_snapshot_t *p=&b2_controller_sample;
    return B2_RealignActive391() && B2_RealignAllowed391() &&
        B2XR_ReferenceApplied397() && b2_client.ui_interrupted && B2_UIPaused() &&
        b2_realign.epoch==B2XR_InterruptionEpoch() &&
        B2_InputFocused556() && B2XR_InteractionReady() && !B2XR_StopRequested() &&
        b2c_sample_valid(p,now) && p->xr_generation &&
        p->xr_generation==B2XR_InputGeneration() && b2w_owner_epoch && b2_ui_validate_hold();
}
static bool b2_reference_neutral397(void)
{
    for(int key=0;key<256;key++)if(Key_IsDown(key))return false;
    return b2c_neutral(&b2_controller_sample);
}
static void b2_reference_stage397(uint32_t now)
{
    if(!b2_reset397.automatic || b2_reset397.staged || !b2_reference_sample397(now))return;
    float origin[3],yaw;
    if(!b2_headset_reference(&b2_controller_sample,origin,&yaw) || !B2RS_HeightValid(origin[1]))return;
    bool calibrated=b2rs_stance.calibrated;
    float baseline=b2rs_stance.baseline;
    uint32_t generation=b2_controller_sample.xr_generation;
    b2_clear_walk_input();CL_B2ReleaseButtons();B2RM_Reset();
    memset(&b2rs_client,0,sizeof(b2rs_client));memset(&b2rs_stance,0,sizeof(b2rs_stance));
    b2rs_stance.calibrated=b2_reset397.known?calibrated:true;
    b2rs_stance.baseline=b2_reset397.known?baseline:origin[1];
    b2rs_stance.last_height=origin[1];
    if(!b2_reset397.known){
        b2_client.kbm_height_reference=false;
        VectorCopy(origin,b2_client.reference_origin);b2_client.reference_yaw=yaw;
    }
    b2_realign_duck=(cl.frame.ps.pmove.pm_flags&PMF_DUCKED)!=0;
    b2_realign_duck_seen=false;b2_realign_duck_world=b2_client.world_epoch;
    // Only a preserved calibrated basis can prove standing after this reset.
    b2_realign_duck_standing602=b2_reset397.known && calibrated && B2RS_HeightValid(baseline);
    b2_client.recenter_epoch++;b2_client.history.valid=false;b2_client.audio_pending=false;
    b2_client.walk_release_required=true;
    b2_reset397.owner=b2w_owner_epoch;b2_reset397.revision=B2B_Revision();
    b2_reset397.generation=generation;b2_reset397.staged=true;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REFERENCE staged automatic=1 known=%d gameplay_held=1 fresh_world_pair_required=1\n",b2_reset397.known);
}
static void b2_reference_commit397(bool pair,bool ended,bool renderer,bool history)
{
    if(!b2_reset397.captured || !pair || !ended || !renderer || !history ||
       b2_client.stopped || b2_client.world_waiting || b2_client.world_rebinding ||
       b2_client.headset_waiting || !b2_client.origin_latched || !b2_client.walk_ready)return;
    if(b2_reset397.world!=b2_client.world_epoch && !B2_RealignActive391()){
        // Initial reference in the NEW world is now committed. It owns its own
        // startup/deliberate-resume path, never the retired reset's destination.
        if(B2XR_CompleteRealign391(B2XR_InputGeneration())){
            memset(&b2_reset397,0,sizeof(b2_reset397));
            b2_client.walk_release_required=true;
            if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REFERENCE rebound new_world=1 old_snapshot_restored=0\n");
        }
        return;
    }
    if(b2_reset397.known && !b2_reset397.automatic && !B2_RealignActive391()){
        if(b2_ui_validate_hold() && B2XR_CompleteRealign391(B2XR_InputGeneration())){
            memset(&b2_reset397,0,sizeof(b2_reset397));
            b2_client.walk_release_required=true;
            if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REFERENCE continuity_committed deliberate_resume_unchanged=1\n");
        }
        return;
    }
    if(!b2_reset397.automatic || !b2_reset397.staged)return;
    if(b2_reset397.epoch!=B2XR_InterruptionEpoch() ||
       b2_reset397.owner!=b2w_owner_epoch || b2_reset397.revision!=B2B_Revision() ||
       b2_reset397.generation!=B2XR_InputGeneration() ||
       b2_reset397.destination!=cls.key_dest || b2_reset397.manual!=b2_client.ui_manual_pause ||
       b2aw_owned_hold() || !B2_InputFocused556() || !B2XR_InteractionReady()){
        b2_reference_fallback397();return;
    }
    // Read after EndFrame: fresh actions and head belong to this completed pair.
    if(!B2XR_ReadControllerInput(&b2_controller_sample) ||
       !b2_reference_sample397(Sys_Milliseconds()) || !b2_reference_neutral397() ||
       !B2XR_CompleteRealign391(b2_controller_sample.xr_generation))return;
    b2_client.ui_interrupted=false; // Only the hold we introduced is released.
    b2_client.ui_resume_request=b2_client.ui_recovery_ready=false;
    b2_clear_walk_input();CL_B2ReleaseButtons();b2_client.walk_release_required=true;
    memset(&b2_realign,0,sizeof(b2_realign));memset(&b2_reset397,0,sizeof(b2_reset397));
    R_B2UIReset();b2_ui_apply_pause();
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REFERENCE automatic_committed whole_pair=1 history=1 neutral=1 manual=%d destination=%d release_rearm=1 body_unchanged=1\n",
        b2_client.ui_manual_pause,cls.key_dest);
}
static bool b2_realign_ready391(uint32_t now)
{
    const b2c_snapshot_t *p=&b2_controller_sample;
    return B2_RealignActive391() && B2_RealignAllowed391() &&
        B2XR_RealignPending391() && b2_client.ui_interrupted && B2_UIPaused() &&
        b2_realign.visible && b2_realign.epoch==B2XR_InterruptionEpoch() &&
        B2_InputFocused556() && B2XR_InteractionReady() &&
        !B2XR_StopRequested() && b2c_sample_valid(p,now) &&
        p->xr_generation && p->xr_generation==B2XR_InputGeneration() &&
        b2w_owner_epoch && b2_client.world_epoch && b2_ui_validate_hold();
}
static void b2_realign_confirm391(uint32_t now)
{
    if(!b2_realign_ready391(now) || !b2c_neutral(&b2_controller_sample))return;
    for(int key=0;key<256;key++)if(key!=K_PAUSE && Key_IsDown(key))return;
    float origin[3],yaw;
    if(!b2_headset_reference(&b2_controller_sample,origin,&yaw) ||
        !B2RS_HeightValid(origin[1]))return;
    b2_realign.sample=b2_controller_sample;b2_realign.pending=true;
    b2_realign.owner=b2w_owner_epoch;b2_realign.world=b2_client.world_epoch;
    b2_realign.revision=B2B_Revision();
}
static void b2_realign_tick391(uint32_t now)
{
    if(b2_reset397.automatic){b2_reference_stage397(now);return;}
    const b2c_snapshot_t *p=&b2_controller_sample;
    if(!b2_realign_ready391(now)){
        b2_realign.pending=false;
        b2_realign.armed=b2_realign.pressed=b2_realign.key_pressed=false;
        b2_realign.serial=0;return;
    }
    unsigned hand=B2B_Hand(),revision=B2B_Revision();
    bool pressed=B2B_Value(p,B2B_RESUME,hand,0)>.5f;
    if(!b2_realign.serial || b2_realign.generation!=p->generation ||
        b2_realign.xr_generation!=p->xr_generation || b2_realign.hand!=hand ||
        b2_realign.revision!=revision || p->serial<b2_realign.serial){
        b2_realign.serial=p->serial;b2_realign.generation=p->generation;
        b2_realign.xr_generation=p->xr_generation;b2_realign.hand=hand;
        b2_realign.revision=revision;b2_realign.previous=pressed;
        b2_realign.pending=false;
        b2_realign.armed=b2_realign.pressed=b2_realign.key_pressed=false;return;
    }
    if(p->serial==b2_realign.serial)return;
    b2_realign.serial=p->serial;
    b2c_snapshot_t released=*p;B2B_Clear(&released,B2B_RESUME,hand);
    bool neutral=b2c_neutral(&released);
    if(pressed && !b2_realign.previous){
        b2_realign.pressed=b2_realign.armed && neutral;b2_realign.armed=false;
    }else if(!pressed){
        if(b2_realign.previous && b2_realign.pressed && neutral)b2_realign_confirm391(now);
        b2_realign.pressed=false;b2_realign.armed=neutral;
    }else if(!neutral)b2_realign.pressed=false;
    b2_realign.previous=pressed;
}
static void b2_realign_key391(bool down)
{
    uint32_t now=Sys_Milliseconds();
    if(down){
        if(Key_IsDown(K_PAUSE)==1)b2_realign.key_pressed=b2_realign.armed &&
            b2_realign_ready391(now) && b2c_neutral(&b2_controller_sample);
    }else{
        bool accepted=b2_realign.key_pressed;b2_realign.key_pressed=false;
        if(accepted)b2_realign_confirm391(now);
    }
}
static void b2_realign_finish391(bool committed)
{
    if(!B2_RealignActive391() || b2_reset397.automatic)return;
    b2_realign.visible=committed;
    bool pending=b2_realign.pending;b2_realign.pending=false;
    uint32_t now=Sys_Milliseconds();
    if(!committed || !pending || !b2_realign_ready391(now) || !b2c_neutral(&b2_controller_sample) ||
        b2_realign.owner!=b2w_owner_epoch || b2_realign.world!=b2_client.world_epoch ||
        b2_realign.revision!=B2B_Revision() ||
        !b2c_sample_valid(&b2_realign.sample,now) || !b2c_neutral(&b2_realign.sample) ||
        b2_realign.sample.xr_generation!=B2XR_InputGeneration())return;
    float origin[3],yaw;
    if(!b2_headset_reference(&b2_realign.sample,origin,&yaw) || !B2RS_HeightValid(origin[1]))return;
    if(!B2XR_CompleteRealign391(b2_realign.sample.xr_generation))return;
    // Retire old pending movement without publishing it; authoritative body is held.
    b2_clear_walk_input();B2RM_Reset();memset(&b2rs_client,0,sizeof(b2rs_client));
    memset(&b2rs_stance,0,sizeof(b2rs_stance));
    b2rs_stance.calibrated=true;b2rs_stance.baseline=origin[1];
    b2rs_stance.last_height=origin[1];
    b2_realign_duck=b2_realign.duck;b2_realign_duck_seen=false;
    b2_realign_duck_standing602=false; // Fresh alignment is not evidence of standing.
    b2_realign_duck_world=b2_client.world_epoch;
    b2_client.kbm_height_reference=false;
    VectorCopy(origin,b2_client.reference_origin);b2_client.reference_yaw=yaw;
    b2_client.origin_latched=true;b2_client.recenter_epoch++;
    b2_client.ui_resume_request=b2_client.ui_recovery_ready=false;
    b2_client.walk_release_required=true;b2_client.history.valid=false;
    b2_client.audio_pending=false;b2_resume_reset();
    memset(&b2_realign,0,sizeof(b2_realign));
    memset(&b2_reset397,0,sizeof(b2_reset397));R_B2UIReset();
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REALIGN committed body_unchanged=1 fresh_posture=1 duck_retained=%d separate_neutral_resume=required\n",b2_realign_duck);
}
void B2_RealignPrompt391(char *text,unsigned capacity)
{
    char label[B2_RESUME_LABEL261];unsigned hand=B2B_Hand();
    int source=B2B_Source(B2B_RESUME,hand);
    if(source>=0 && source<12 && B2B_Label((unsigned)source,label,sizeof(label)))
        snprintf(text,capacity,"Release controls; press/release %s %s to align",source<6?"left":"right",label);
    else Q_strlcpy(text,"Release controls; press and release Pause to align",capacity);
}
