// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Stock inventory remains KEY_GAME. Only its controller ownership is distinct.
static struct {
    uint64_t serial;
    uint32_t generation,xr,interruption,world,revision,last,started,stepped,pending_ms;
    unsigned hand,pending_hand;
    int dir,selected;
    bool configured,armed,pending;
} b2_inventory342;

static bool b2_inventory_feedback342(void)
{
    uint32_t now=Sys_Milliseconds();
    return b2_inventory342.configured && cl.frame.valid && B2_InputFocused556() && cls.key_dest==KEY_GAME &&
        (cl.frame.ps.stats[STAT_LAYOUTS]&LAYOUTS_INVENTORY) &&
        b2_controller_input && b2_controller_input->integer && b2_controller.requested &&
        b2_controller.valid && !b2_controller.wait_neutral && b2_controller.destination==B2C_INVENTORY342 &&
        b2_client.walk_ready && !b2_client.walk_release_required && !b2_client.ui_interrupted &&
        !b2_client.stopped && !b2_client.world_waiting && !b2_client.world_rebinding &&
        !b2_client.headset_waiting && !b2_client.handoff_position_pending && !b2_client.recenter_pending &&
        !b2_client.combat_state.dead && !B2_UIPaused() && !B2_UISimulationHeld() &&
        B2XR_InteractionReady() && !B2XR_StopRequested() &&
        b2c_sample_valid(&b2_controller_sample,now) &&
        b2_controller_sample.xr_generation==B2XR_InputGeneration() &&
        b2_inventory342.generation==b2_controller_sample.generation &&
        b2_inventory342.xr==b2_controller_sample.xr_generation &&
        b2_inventory342.interruption==B2XR_InterruptionEpoch() &&
        b2_inventory342.world==b2_client.world_epoch && b2_inventory342.revision==B2B_Revision() &&
        b2_inventory342.hand==B2B_Hand() && now-b2_inventory342.last<=B2C_MAX_AGE_MS;
}

static void b2_inventory_cancel342(void)
{
    if(b2_inventory342.configured)b2_ui_cancel304();
    memset(&b2_inventory342,0,sizeof(b2_inventory342));
}

static void b2_inventory_keys342(uint32_t now)
{
    const b2c_snapshot_t *p=&b2_controller_sample;
    unsigned hand=B2B_Hand();
    if(!b2_controller.valid || b2_controller.wait_neutral || hand>1 ||
       !b2c_sample_valid(p,now)){b2_inventory_cancel342();return;}
    if(!b2_inventory342.configured){
        b2_inventory342.configured=true;b2_inventory342.hand=hand;
        b2_inventory342.generation=p->generation;b2_inventory342.xr=p->xr_generation;
        b2_inventory342.interruption=B2XR_InterruptionEpoch();b2_inventory342.world=b2_client.world_epoch;
        b2_inventory342.revision=B2B_Revision();b2_inventory342.last=now;
        b2_inventory342.selected=cl.frame.ps.stats[STAT_SELECTED_ITEM];
        UI_B2VRRepeatCancel();
        // First ownership, identity loss and a long sample gap all require
        // neutral again; no held navigation/confirmation is replayed.
        if(!b2c_neutral(p)){b2c_clear(&b2_controller);b2_inventory_cancel342();return;}
        b2_inventory342.armed=true;
    }
    if(!b2_inventory_feedback342() || p->serial<b2_inventory342.serial){
        b2c_clear(&b2_controller);b2_inventory_cancel342();return;
    }
    if(p->serial==b2_inventory342.serial)return;
    b2_inventory342.serial=p->serial;b2_inventory342.last=now;
    b2_ui_sample_set304(now);
    // The real server selects the item. A sent command is not an acknowledgement.
    // Feedback follows a changed STAT_SELECTED_ITEM within the current request
    // window; there is no invented command ID or claim of exclusive causality.
    int selected=cl.frame.ps.stats[STAT_SELECTED_ITEM];
    if(b2_inventory342.pending && now-b2_inventory342.pending_ms>1000)b2_inventory342.pending=false;
    if(selected!=b2_inventory342.selected){
        if(b2_inventory342.pending && selected>=0)B2H_UI304(0,b2_inventory342.pending_hand);
        b2_inventory342.pending=false;b2_inventory342.selected=selected;
    }
    unsigned events=b2_controller.intent.events;
    if(events&B2C_RECENTRE){
        b2_inventory_cancel342();b2_recenter_down_f();b2_recenter_up_f();return;
    }
    if(events&B2C_CONFIRM){
        b2_inventory342.armed=false;b2_inventory342.dir=0;b2_inventory342.pending=false;
        // This pulse acknowledges the fresh input only. Stock invuse decides
        // whether the selected item is usable and owns all item effects.
        B2H_UI304(1,p->hands[hand].primary?hand:1-hand);
        CL_ClientCommand("invuse");return;
    }
    int owner=b2c_menu_stick316(p,hand);
    if(owner==-2){b2_inventory342.armed=false;b2_inventory342.dir=0;return;}
    if(owner==-1){b2_inventory342.armed=true;b2_inventory342.dir=0;return;}
    float x=p->hands[owner].stick[0],y=p->hands[owner].stick[1];
    float threshold=b2_inventory342.dir?.35f:.65f;
    int dir=0;
    if(b2_inventory342.armed){
        if(fabsf(x)>threshold && fabsf(x)>fabsf(y))dir=x>0?1:-1;
        else if(fabsf(y)>threshold)dir=y>0?2:-2;
    }
    if(!dir){b2_inventory342.dir=0;return;}
    if(dir!=b2_inventory342.dir){b2_inventory342.started=b2_inventory342.stepped=now;}
    else {
        // Exact accepted menu timing from b2_repeat304.inc (GPL-2.0-or-later).
        // Kept local because the menu scheduler requires a real menu/item owner.
        uint32_t held=now-b2_inventory342.started;unsigned interval=held>=2000?45:held>=1000?80:150;
        if(held<400 || now-b2_inventory342.stepped<interval)return;
        b2_inventory342.stepped=now;
    }
    b2_inventory342.dir=dir;b2_inventory342.pending=true;
    b2_inventory342.pending_ms=now;b2_inventory342.pending_hand=(unsigned)owner;
    CL_ClientCommand(dir==1 || dir==-2?"invnext":"invprev");
}
