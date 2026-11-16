// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Included after the existing controller publication, before ControllerTick.
void B2XR_InvalidateRenderHistory(const char *reason);
// Engine-local read of the same live edict bounds used by the private game preview.
bool SV_B2TeleportHull(b2tp_hull_t *out);
static bool b2tp_enabled(void)
{
    return b2_teleport && b2_teleport->integer==1 && b2_controller.requested &&
        b2_tracked_weapon && b2_tracked_weapon->integer==1;
}
static void b2tp_drop301(void)
{
    B2TP_CancelInput(&b2tp_input);memset(&b2tp_preview,0,sizeof(b2tp_preview));
    memset(&b2tp_preview_hull,0,sizeof(b2tp_preview_hull));
    b2tp_use=false;b2tp_pulsing=false;B2TP_Reset();
}
static struct {
    bool configured,armed,down,consumed,block,nudge,combo,toggle;
    uint64_t serial;uint32_t press_ms,combo_ms,sample_ms,revision;
    unsigned hand;bool teleport_only;b2w_epochs_t epochs;
} b2move301;
static void b2move_cancel301(void){memset(&b2move301,0,sizeof(b2move301));}
static bool b2move_eligible301(uint32_t now)
{
    return b2_controller.requested && b2c_sample_valid(&b2_controller_sample,now) &&
        b2_controller_sample.xr_generation && b2_controller.valid && !b2_controller.wait_neutral &&
        b2_controller.destination==B2C_GAME && !b2w_actions_suppressed &&
        !b2w_selector.open && !b2w_selector.rearm && !b2w_pending_token &&
        b2_client.walk_ready && !b2_client.walk_release_required && !b2_client.stopped &&
        !b2_client.world_waiting && !b2_client.world_rebinding && !b2_client.recenter_pending &&
        !b2_client.handoff_position_pending && !b2_client.ui_interrupted && !B2_UISimulationHeld() &&
        !b2_client.combat_state.dead && cl.frame.ps.pmove.pm_type==PM_NORMAL &&
        B2_InputFocused556() && cls.key_dest==KEY_GAME && B2XR_InteractionReady();
}
static bool b2move_active301(uint32_t now)
{
    return b2move301.nudge && (uint32_t)(now-b2move301.sample_ms)<=100 &&
        b2move301.revision==B2B_Revision() && b2move_eligible301(now);
}
static void b2move_tick301(uint32_t now)
{
    if(!b2move_eligible301(now)){b2move_cancel301();return;}
    b2w_epochs_t epochs={b2_controller_sample.generation,b2_controller_sample.xr_generation,
        b2_client.world_epoch,b2_client.recenter_epoch,b2w_owner_epoch,1};
    unsigned hand=(unsigned)b2_controller_dominant->integer;
    bool mode=B2B_TeleportOnly();
    if(!b2move301.configured || b2move301.hand!=hand || b2move301.teleport_only!=mode ||
       b2move301.revision!=B2B_Revision() || memcmp(&epochs,&b2move301.epochs,sizeof(epochs)) ||
       b2_controller_sample.serial<b2move301.serial){
        b2move_cancel301();b2move301.configured=true;b2move301.hand=hand;
        b2move301.teleport_only=mode;b2move301.revision=B2B_Revision();b2move301.epochs=epochs;
    }
    if(b2_controller_sample.serial==b2move301.serial){
        // Controller polls may reuse a still-fresh sample; keep its admitted intent.
        if(mode && b2move301.nudge){b2_controller.intent.forward=100;b2_controller.intent.side=0;}
        return;
    }
    b2move301.serial=b2_controller_sample.serial;b2move301.sample_ms=now;b2move301.nudge=false;
    bool down=B2B_Value(&b2_controller_sample,B2B_RUN,hand,0)>.5f;
    float x=B2B_Value(&b2_controller_sample,B2B_MOVE,hand,0),y=B2B_Value(&b2_controller_sample,B2B_MOVE,hand,1);
    bool neutral=fabsf(x)<.25f && fabsf(y)<.25f;
    if(!b2move301.armed){if(!down && (!mode || neutral))b2move301.armed=true;return;}
    if(down && !b2move301.down){b2move301.press_ms=now;b2move301.consumed=false;b2move301.combo=false;}
    if(down && mode){
        b2move301.block=true; // Cancel pending aim immediately, before release can commit.
        if(!neutral)b2move301.consumed=true;
        bool forward=b2tp_enabled() && y>.55f && fabsf(x)<.25f;
        if(forward){
            if(!b2move301.combo){b2move301.combo=true;b2move301.combo_ms=now;}
            if((uint32_t)(now-b2move301.combo_ms)>=250){
                b2move301.nudge=true;b2move301.consumed=true;
                b2_controller.intent.forward=100;b2_controller.intent.side=0;
            }
        }else b2move301.combo=false;
    }
    if(down && (uint32_t)(now-b2move301.press_ms)>250)b2move301.consumed=true;
    if(!down && b2move301.down && !b2move301.consumed &&
       (uint32_t)(now-b2move301.press_ms)<=250 && (!mode || neutral)){
        b2move301.toggle=true;
    }
    b2move301.down=down;
    if(!down && (!mode || neutral)){b2move301.block=false;b2move301.combo=false;}
}
static void b2move_commit_run301(void)
{
    bool toggle=b2move301.toggle;b2move301.toggle=false;
    if(toggle){cvar_t *run=Cvar_Get("cl_run","0",CVAR_ARCHIVE);Cvar_SetInteger(run,!run->integer,FROM_CODE);}
}
static void b2tp_cancel(void){b2tp_drop301();b2move_cancel301();}
void B2_TeleportResult(const b2tp_request_t *r)
{
    B2TP_CancelInput(&b2tp_input);b2tp_preview.count=0;b2tp_use=false;
    if(!r || !r->accepted){b2tp_pulsing=false;return;}
    // The accepted physical displacement phase has already finished. Drop its
    // old records and wall residual, preserving vertical reference and posture.
    B2RM_Reset();
    b2_client.reference_origin[0]=r->head[0];b2_client.reference_origin[2]=r->head[2];
    b2rs_client.anchor[0]=r->head[0];b2rs_client.anchor[2]=r->head[2];
    // Head input uses the predicted display time, not the controller poll counter.
    b2rs_client.serial=(uint64_t)r->aim.pose.pose_time;b2rs_client.sample_ms=r->aim.pose.head_ms;
    b2rs_client.activation_ms=r->aim.pose.head_ms;b2rs_client.reproject=false;
    VectorCopy(r->head,b2rs_client.last_head);
    VectorClear(cl.prediction_error);cl.predicted_step=0;
    VectorCopy(r->accepted_origin,cl.predicted_origin);VectorClear(cl.predicted_velocity);
    for(unsigned i=0;i<3;i++)cl.predicted_origins[r->number&CMD_MASK][i]=(short)lroundf(r->accepted_origin[i]*8.f);
    b2_client.history.valid=false;B2XR_InvalidateRenderHistory("comfort_teleport");
    b2tp_accepted_ms=Sys_Milliseconds();b2tp_pulsing=true;
}
static void b2tp_tick(uint32_t now)
{
    if(!b2move_eligible301(now) || B2TP_Expired(now)){b2tp_cancel();return;}
    if(!b2tp_enabled() || b2move301.block){b2tp_drop301();return;}
    b2w_epochs_t epochs={b2_controller_sample.generation,b2_controller_sample.xr_generation,
        b2_client.world_epoch,b2_client.recenter_epoch,b2w_owner_epoch,1};
    b2w_space_t space={0};space.epochs=epochs;space.committed=true;
    b2rs_reference(space.reference_origin);space.reference_yaw=b2_client.reference_yaw;
    space.units=b2_units_per_metre->value;VectorCopy(b2_client.scene.vieworg,space.anchor_origin);
    space.anchor_origin[2]+=b2rs_eye();
    for(unsigned i=0;i<3;i++)space.anchor_angles[i]=SHORT2ANGLE((short)(
        ANGLE2SHORT(b2_client.input_angles[i])+b2_client.anchor_player.delta_angles[i]));
    b2tp_request_t request={0};request.aim.slot=-1;request.aim.weapon=b2w_published_inventory.current;
    request.aim.game_epoch=b2w_published_inventory.epoch;
    unsigned weapon_hand=(unsigned)b2_controller_dominant->integer;
    bool stick=B2B_TeleportOnly();
    unsigned aim_hand=(unsigned)B2B_Source(stick?B2B_MOVE:B2B_TELEPORT,weapon_hand)/6;
    bool pose=B2W_BuildAim(&b2_controller_sample,aim_hand,&space,now,&request.aim.pose);
    if(!pose){b2tp_cancel();return;}
    B2TP_SetOwner(true,&epochs);
    b2c_snapshot_t mapped=b2_controller_sample;
    float activation=B2B_Value(&b2_controller_sample,B2B_TELEPORT,weapon_hand,0);
    if(stick){
        float x=B2B_Value(&b2_controller_sample,B2B_MOVE,weapon_hand,0);
        float y=B2B_Value(&b2_controller_sample,B2B_MOVE,weapon_hand,1);
        // Deliberate .55 aim threshold, .25 neutral release hysteresis. The
        // existing reducer still owns serial/epoch/release-to-commit admission.
        float magnitude=sqrtf(x*x+y*y);
        float stick_activation=magnitude>.55f || (magnitude>=.25f && (!b2tp_input.armed || b2tp_input.aiming))?1.f:0.f;
        activation=B2B_Quest316()?fmaxf(activation,stick_activation):stick_activation;
    }
    mapped.hands[weapon_hand^1u].trigger=activation;
    bool release=B2TP_Input(&b2tp_input,&mapped,&epochs,now,weapon_hand,true,true);
    b2tp_use=b2tp_input.armed && (B2B_Value(&b2_controller_sample,B2B_USE,weapon_hand,0)>.5f);
    b2tp_preview.count=0;
    if(!b2tp_input.aiming && !release)return;
    b2w_game_api_t api;
    if(!SV_B2WeaponAPI(&api) || !api.teleport_preview || !SV_B2TeleportHull(&b2tp_preview_hull) ||
       !api.teleport_preview(&request.aim,B2TP_Range(b2_teleport_range->value),&b2tp_preview)){
        b2tp_cancel();return;
    }
    b2tp_preview_ms=now;
    if(release){
        if(b2tp_preview.status==B2TP_EMPTY || b2tp_preview.status==B2TP_OCCUPIED){
            VectorCopy(b2tp_preview.landing,request.target);
            VectorCopy(b2_controller_sample.head_position,request.head);
            request.range=B2TP_Range(b2_teleport_range->value);
            B2TP_Publish(&request,now);
        }
        b2tp_preview.count=0;b2tp_input.armed=false;
    }
}
// Horizontal collision footprint, centred beneath the proposed player origin.
// The circle matches the square hull's width; it does not cover its corners.
// Its centre line uses world game units, independent of metres-to-world scale.
static int b2tp_circle(entity_t *beams,unsigned status,const unsigned char colour[3])
{
    const b2tp_hull_t *h=&b2tp_preview_hull;
    if(!b2tp_hull_valid(h))return 0;
    float width=h->maxs[0]-h->mins[0],depth=h->maxs[1]-h->mins[1];
    if(!isfinite(width) || !isfinite(depth) || width<=0 || fabsf(width-depth)>.001f)return 0;
    float centre[3];
    if(status==B2TP_EMPTY || status==B2TP_OCCUPIED){
        VectorCopy(b2tp_preview.landing,centre);
        centre[0]+=.5f*(h->mins[0]+h->maxs[0]);
        centre[1]+=.5f*(h->mins[1]+h->maxs[1]);
        centre[2]+=h->mins[2];
    }else{
        VectorCopy(b2tp_preview.points[b2tp_preview.count-1],centre);
    }
    if(!b2tp_finite3(centre))return 0;
    float radius=.5f*width;
    for(unsigned i=0;i<B2TP_CIRCLE_SEGMENTS;i++){
        entity_t *b=&beams[i];memset(b,0,sizeof(*b));
        b->flags=RF_TRANSLUCENT|RF_BEAM|B2_RF_WORLD_RETICLE|B2_RF_TELEPORT_GUIDE;b->skinnum=-1;
        memcpy(b->rgba.u8,colour,3);b->rgba.u8[3]=255;b->alpha=.78f;b->frame=1;
        float a=(2.f*(float)M_PI*i)/B2TP_CIRCLE_SEGMENTS;
        float z=(2.f*(float)M_PI*((i+1)%B2TP_CIRCLE_SEGMENTS))/B2TP_CIRCLE_SEGMENTS;
        VectorCopy(centre,b->origin);VectorCopy(centre,b->oldorigin);
        b->origin[0]+=radius*cosf(a);b->origin[1]+=radius*sinf(a);
        b->oldorigin[0]+=radius*cosf(z);b->oldorigin[1]+=radius*sinf(z);
    }
    return B2TP_CIRCLE_SEGMENTS;
}
// Display-only midpoint refinement; authoritative preview stays byte-identical.
// Clamp each coordinate to its span and cap deviation to one game unit.
static bool b2tp_display_midpoint(const float previous[3], const float a[3],
    const float b[3], const float next[3], float midpoint[3])
{
    float deviation2=0,span2=0;
    for (unsigned axis=0;axis<3;axis++) {
        float centre=.5f*(a[axis]+b[axis]);
        float curve=centre+(a[axis]-previous[axis]+b[axis]-next[axis])*.0625f;
        midpoint[axis]=Q_clipf(curve,min(a[axis],b[axis]),max(a[axis],b[axis]));
        float d=midpoint[axis]-centre,s=b[axis]-a[axis];
        deviation2+=d*d;span2+=s*s;
    }
    if (!isfinite(deviation2) || !isfinite(span2) || span2<.0001f || deviation2<.000001f) return false;
    float bound=min(1.f,.25f*sqrtf(span2));
    if (deviation2>bound*bound) {
        float scale=bound/sqrtf(deviation2);
        for (unsigned axis=0;axis<3;axis++) {
            float centre=.5f*(a[axis]+b[axis]);
            midpoint[axis]=centre+(midpoint[axis]-centre)*scale;
        }
    }
    return true;
}
static bool b2tp_display_clear(const float a[3], const float b[3])
{
    // Cover the actual beam radius. Unavailable/extreme settings keep the
    // original spans; this query never changes any authoritative decision.
    cvar_t *width=Cvar_FindVar("pt_beam_width");
    if (!width || !isfinite(width->value) || width->value<=0.f || width->value>16.f) return false;
    float radius=.5f*width->value;
    vec3_t mins={-radius,-radius,-radius},maxs={radius,radius,radius};
    trace_t t;
    CL_Trace(&t,a,mins,maxs,b,MASK_PLAYERSOLID);
    return !t.startsolid && !t.allsolid && isfinite(t.fraction) && t.fraction==1.f;
}
static void b2tp_display_beam(entity_t *b,const float a[3],const float z[3],const unsigned char colour[3])
{
    memset(b,0,sizeof(*b));
    b->flags=RF_TRANSLUCENT|RF_BEAM|B2_RF_WORLD_RETICLE|B2_RF_TELEPORT_GUIDE;b->skinnum=-1;
    memcpy(b->rgba.u8,colour,3);b->rgba.u8[3]=255;b->alpha=.78f;b->frame=1;
    VectorCopy(a,b->origin);VectorCopy(z,b->oldorigin);
}
static int b2tp_geometry(entity_t beams[B2TP_GUIDE_ENTITIES])
{
    if(!b2tp_enabled() || !b2tp_input.aiming || b2tp_preview.count<2 ||
        b2tp_preview.count>B2TP_POINTS || (uint32_t)(Sys_Milliseconds()-b2tp_preview_ms)>100)return 0;
    const unsigned char colours[3][3]={{255,26,26},{26,255,26},{255,204,26}};
    unsigned status=b2tp_preview.status<=B2TP_OCCUPIED?b2tp_preview.status:B2TP_INVALID;
    int count=0;
    for(unsigned i=1;i<b2tp_preview.count;i++){
        const float *a=b2tp_preview.points[i-1],*b=b2tp_preview.points[i];
        float midpoint[3];
        // Exact start/end and terminal contact span; no extrapolation at either end.
        bool round=i>1 && i+1<b2tp_preview.count &&
            b2tp_display_midpoint(b2tp_preview.points[i-2],a,b,b2tp_preview.points[i+1],midpoint) &&
            b2tp_display_clear(a,midpoint) && b2tp_display_clear(midpoint,b);
        if (round) {
            b2tp_display_beam(&beams[count++],a,midpoint,colours[status]);
            b2tp_display_beam(&beams[count++],midpoint,b,colours[status]);
        } else b2tp_display_beam(&beams[count++],a,b,colours[status]);
    }
    count+=b2tp_circle(beams+count,status,colours[status]);
    return count;
}
