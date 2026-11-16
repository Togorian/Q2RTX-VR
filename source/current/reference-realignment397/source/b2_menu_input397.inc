// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Bridge the existing three menu reducers to local comfort-slider repeat.
static struct {
    uint64_t serial;
    uint32_t generation,xr,interruption,last;
    unsigned route,hand;
    int dir;
    bool configured,armed;
} b2_menu_repeat253;

// At most six actual dispatches, including logical-left Menu and mapped input.
static void b2_menu_route320(const b2c_snapshot_t *p,const char *route)
{
    static unsigned records;
    if(records<6){
        B2D_Record(0,"MENU_ROUTE serial=%llu left=%u right=%u mapped_source=%d route=%s",
            (unsigned long long)p->serial,(unsigned)p->hands[0].menu,
            (unsigned)p->hands[1].menu,B2B_Source(B2B_MENU,B2B_Hand()),route);
        records++;
    }
}
static void b2_menu_cancel253(void)
{
    if(!b2_inventory_feedback342())b2_ui_cancel304();
    memset(&b2_menu_repeat253,0,sizeof(b2_menu_repeat253));
    UI_B2VRRepeatController(0,false,0);
}

static void b2_menu_keys253(const b2c_state_t *ui,const b2c_snapshot_t *p,
    uint32_t now,unsigned route)
{
    unsigned hand=ui->dominant, interruption=B2XR_InterruptionEpoch();
    if (UI_B2BindingCapture304() || !B2_InputFocused556() || !(cls.key_dest&KEY_MENU) || (cls.key_dest&KEY_CONSOLE) ||
        !B2XR_InteractionReady() || !b2c_sample_valid(p,now) || !ui->valid || ui->wait_neutral ||
        hand>1 || p->xr_generation!=B2XR_InputGeneration()) { b2_menu_cancel253(); return; }
    if(ui->intent.events&B2C_RECENTRE){
        b2_recenter_down_f();b2_recenter_up_f();b2_menu_cancel253();return;
    }
    if (!b2_menu_repeat253.configured || b2_menu_repeat253.route!=route ||
        b2_menu_repeat253.hand!=hand || b2_menu_repeat253.generation!=p->generation ||
        b2_menu_repeat253.xr!=p->xr_generation || b2_menu_repeat253.interruption!=interruption ||
        p->serial<b2_menu_repeat253.serial || now-b2_menu_repeat253.last>B2C_MAX_AGE_MS) {
        b2_menu_cancel253();
        b2_menu_repeat253.configured=true; b2_menu_repeat253.route=route;
        b2_menu_repeat253.hand=hand; b2_menu_repeat253.generation=p->generation;
        b2_menu_repeat253.xr=p->xr_generation; b2_menu_repeat253.interruption=interruption;
    }
    if (p->serial==b2_menu_repeat253.serial) return;
    b2_menu_repeat253.serial=p->serial; b2_menu_repeat253.last=now;
    const b2c_intent_t *intent=&ui->intent;
    unsigned key=(intent->events&B2C_BACK)?K_ESCAPE:(intent->events&B2C_CONFIRM)?K_ENTER:
        intent->navigation==B2C_UP?K_UPARROW:intent->navigation==B2C_DOWN?K_DOWNARROW:
        intent->navigation==B2C_LEFT?K_LEFTARROW:intent->navigation==B2C_RIGHT?K_RIGHTARROW:0;
    int owner=b2c_menu_stick316(p,hand);
    if(owner==-2)b2_menu_repeat253.armed=false;
    float x=owner>=0?p->hands[owner].stick[0]:0;
    float y=owner>=0?p->hands[owner].stick[1]:0;
    bool neutral=fabsf(x)<.35f && fabsf(y)<.35f;
    if (neutral && owner==-1) b2_menu_repeat253.armed=true;
    float threshold=b2_menu_repeat253.dir?.35f:.65f;
    int dir=0;
    if(b2_menu_repeat253.armed){
        if(fabsf(x)>threshold && fabsf(x)>fabsf(y))dir=x>0?1:-1;
        else if(fabsf(y)>threshold)dir=y>0?2:-2;
    }
    if(key==K_ENTER || key==K_ESCAPE){UI_B2VRRepeatCancel();b2_menu_repeat253.armed=false;dir=0;}
    b2_ui_sample_set304(now);
    unsigned feedback_hand=owner>=0?(unsigned)owner:hand;
    if(key==K_ENTER)feedback_hand=p->hands[hand].primary?hand:1-hand;
    if(key==K_ESCAPE){
        if(p->hands[0].menu)feedback_hand=0;
        else if(p->hands[1].menu)feedback_hand=1;
        else {int source=B2B_Source(B2B_MENU,hand);if(source>=0 && source<12)feedback_hand=(unsigned)source/6;}
    }
    UI_B2ControllerInput304((int)feedback_hand);
    bool slider=UI_B2VRRepeatController(dir,dir && dir!=b2_menu_repeat253.dir,now);
    b2_menu_repeat253.dir=dir;
    if (slider && key!=K_ENTER && key!=K_ESCAPE) key=0;
    if(key==K_ESCAPE)b2_menu_route320(p,"menu-back");
    if (key) { UI_KeyEvent(key,true); UI_KeyEvent(key,false); }
    UI_B2ControllerInput304(-1);
}
