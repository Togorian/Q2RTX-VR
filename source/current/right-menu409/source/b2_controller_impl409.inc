// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian
// OpenXR controller input for Q2RTX VR.
#include <math.h>
#include "b2_snap256.h"
#include <string.h>
#include <stdio.h>

#ifdef B2C_REDUCER
// Private input destination; no public, game, network or save ABI change.
enum { B2C_INVENTORY342=8 };
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

// Application gesture only. The runtime still owns hold-to-recentre.
// Use admitted capture time; never infer hardware identity or generate a reset.
static bool b2cm_release409(b2cm_release409_t *s,const b2c_snapshot_t *p)
{
    if(!(p->hands[1].ready556&B2C_MENU556)){memset(s,0,sizeof(*s));return false;}
    bool down=p->hands[1].menu;
    uint32_t dt=p->capture_ms-s->sample_ms;
    if(!s->clock_valid || !p->xr_generation || s->xr!=p->xr_generation || dt>B2C_MAX_AGE_MS){
        memset(s,0,sizeof(*s));s->clock_valid=p->xr_generation!=0;
        s->xr=p->xr_generation;s->sample_ms=p->capture_ms;s->down=down;
        return false; // Never accept a release whose press crossed a boundary.
    }
    s->sample_ms=p->capture_ms;
    if(down && !s->down){s->press_ms=p->capture_ms;s->pending=true;}
    if(s->pending && (uint32_t)(p->capture_ms-s->press_ms)>350)s->pending=false;
    bool click=!down && s->down && s->pending;
    if(!down)s->pending=false;
    s->down=down;
    return click;
}

static void b2c_clear(b2c_state_t *s)
{
    memset(&s->menu_release409,0,sizeof(s->menu_release409));
    memset(&s->intent,0,sizeof(s->intent));
    s->wait_neutral=true; s->snap_ready=s->menu_ready=s->attack=s->crouch=false;
    s->previous_primary=s->previous_secondary=s->previous_menu=s->previous_click=false;
    s->valid=false;s->fire_ready556=false;
    s->turn_clock_valid=false; s->turn_ms=0;
}

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

// Called only on the existing valid-owner loss edge, after its decision.
static const char *b2c_sample_reason(const b2c_snapshot_t *p,uint32_t now)
{
    if(!p->ready)return "not-ready";
    if(!p->serial || !p->generation)return "identity";
    if((uint32_t)(now-p->capture_ms)>B2C_MAX_AGE_MS)return "capture-age";
    if((uint32_t)(now-p->head_ms)>B2C_MAX_AGE_MS)return "head-age";
    return b2c_sample_valid(p,now)?"valid":"head-pose";
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

// Temporary physical left-secondary (Touch Y) manual-pause edge.
// Separate from gameplay destination so a deliberate release/press can unpause.
// Invalid ownership, focus, tracking, profile or serial requires fresh neutral.
typedef struct {
    uint64_t serial;
    uint32_t generation,xr;
    bool configured,armed;
} b2cp_state_t;
static bool b2c_pause_mode316(b2cp_state_t *s,const b2c_snapshot_t *p,
    uint32_t now,bool eligible,unsigned dominant,bool cinematic)
{
    if (!eligible || !b2c_sample_valid(p,now) || !p->xr_generation) {
        memset(s,0,sizeof(*s));return false;
    }
    if (!s->configured || s->generation!=p->generation || s->xr!=p->xr_generation ||
        p->serial<s->serial) {
        memset(s,0,sizeof(*s));s->configured=true;s->generation=p->generation;
        s->xr=p->xr_generation;s->serial=p->serial;return false;
    }
    if (p->serial==s->serial) return false;
    s->serial=p->serial;
    if (b2c_neutral(p)) { s->armed=true;return false; }
    bool armed=s->armed;s->armed=false;
    b2c_snapshot_t other=*p;
    if(cinematic && B2B_Quest316()){
        other.hands[1-dominant].secondary=false;
        return armed && p->hands[1-dominant].secondary && b2c_neutral(&other);
    }
    if(!cinematic && B2B_Cycle338() &&
       B2B_Source(B2B_EXTRA,dominant)==B2B_Source(B2B_CYCLE347,dominant))return false;
    B2B_Clear(&other,B2B_EXTRA,dominant);
    return armed && B2B_Value(p,B2B_EXTRA,dominant,0)>.5f && b2c_neutral(&other);
}

static bool b2c_pause_step(b2cp_state_t *s,const b2c_snapshot_t *p,uint32_t now,bool eligible,unsigned dominant)
{
    return b2c_pause_mode316(s,p,now,eligible,dominant,false);
}

// Returns loss of a previously valid controller owner; caller owns interruption.

// Terminal continuity refusal has a separate one-shot Quit gesture. It never
// produces gameplay/menu/resume intents: fresh neutral, then hold both face
// buttons for 1.2 seconds on the configured hand, or the independently
// eligible other hand when either required action is unavailable.
typedef struct {
    uint64_t serial;
    uint32_t generation,xr,last_ms,held_ms;
    unsigned hand;
    bool configured,armed,holding;
} b2cq_state_t;
static bool b2c_quit_step(b2cq_state_t *s,const b2c_snapshot_t *p,
    uint32_t now,unsigned hand,bool eligible)
{
    if(p && hand<2 && (p->hands[hand].ready556&(B2C_PRIMARY556|B2C_SECONDARY556))!=(B2C_PRIMARY556|B2C_SECONDARY556) &&
       (p->hands[1-hand].ready556&(B2C_PRIMARY556|B2C_SECONDARY556))==(B2C_PRIMARY556|B2C_SECONDARY556))hand=1-hand;
    if(!eligible || hand>1 || !b2c_sample_valid(p,now) || !p->xr_generation ||
       (p->hands[hand].ready556&(B2C_PRIMARY556|B2C_SECONDARY556))!=(B2C_PRIMARY556|B2C_SECONDARY556)){
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

// Returns the physical hand supplying menu navigation, -1 for neutral,
 // -2 for competing directions. Both hands must release after a conflict.
static int b2c_menu_stick316(const b2c_snapshot_t *p,unsigned dominant)
{
    int direction[2]={0,0};
    for(unsigned h=0;h<2;h++){
        float x=p->hands[h].stick[0],y=p->hands[h].stick[1];
        if(fmaxf(fabsf(x),fabsf(y))<.35f)continue;
        direction[h]=fabsf(x)>fabsf(y)?(x>0?1:-1):(y>0?2:-2);
    }
    if(direction[0] && direction[1] && direction[0]!=direction[1])return -2;
    if(direction[dominant])return dominant;
    return direction[1-dominant]?(int)(1-dominant):-1;
}

static bool b2c_step_turn(b2c_state_t *s, const b2c_snapshot_t *p, uint32_t now,
    unsigned destination, unsigned dominant, float speed, unsigned turn_mode, float smooth_speed, float snap_angle)
{
    // Console/config values are bounded too; keep the menu and actual turn in agreement.
    smooth_speed=isfinite(smooth_speed)?fminf(540.f,fmaxf(45.f,smooth_speed)):360.f;
    snap_angle=b2_snap_value256(snap_angle);
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
        if (b2c_neutral(p)) { s->wait_neutral=false; s->snap_ready=s->menu_ready=true;
            (void)b2cm_release409(&s->menu_release409,p); }
        return false;
    }
    int fire_source=B2B_Source(B2B_FIRE,dominant);
    bool fire_available=fire_source>=0 && fire_source<12 &&
        (p->hands[fire_source/6].ready556&B2C_TRIGGER556) &&
        (destination!=B2C_GAME || b2c_aim_usable556(p,dominant,now));
    float fire=fire_available?B2B_Value(p,B2B_FIRE,dominant,0):0;
    if(!fire_available){s->fire_ready556=false;s->attack=false;}
    else if(fire<.45f)s->fire_ready556=true;
    if(!s->fire_ready556)fire=0;
    bool inventory_ui=destination==B2C_INVENTORY342;
    const b2c_hand_t *d=&p->hands[dominant], *o=&p->hands[1-dominant];
    bool menu=p->hands[0].menu;
    bool right_menu_release=b2cm_release409(&s->menu_release409,p);
    bool mapped_back=B2B_Value(p,B2B_MENU,dominant,0)>.5f;
    int menu_source=B2B_Source(B2B_MENU,dominant);
    int cycle_source=B2B_Source(B2B_CYCLE347,dominant);
    // Shared cycle/back inputs never open the game menu. They cycle only in
    // live play/stock overlays; hardware Menu still works in every context.
    // Ordinary menu/console Back and the existing film shortcut remain intact.
    if(cycle_source>=0 && menu_source==cycle_source &&
       destination!=B2C_MENU && destination!=B2C_CONSOLE && destination!=B2C_CINEMATIC)mapped_back=false;
    if((destination==B2C_MENU || inventory_ui) && menu_source>=0 && menu_source%6==B2B_PRIMARY)mapped_back=false;
    if(destination==B2C_CINEMATIC && B2B_Quest316())mapped_back=d->secondary;
    bool back=(mapped_back && !s->previous_secondary &&
        (B2B_Source(B2B_MENU,dominant)!=B2B_Source(B2B_EXTRA,dominant) || destination!=B2C_GAME)) || (menu && !s->previous_menu) || right_menu_release;
    bool primary_down=d->primary || ((destination==B2C_MENU || inventory_ui) && o->primary);
    bool primary=primary_down && !s->previous_primary;
    bool mapped_click=B2B_Value(p,B2B_RECENTER,dominant,0)>.5f;
    if(destination==B2C_MENU || inventory_ui){
        int recentre_source=B2B_Source(B2B_RECENTER,dominant);
        if(recentre_source>=0 && recentre_source%6==B2B_PRIMARY)mapped_click=false;
    }
    bool click=mapped_click && !s->previous_click;
    s->previous_primary=primary_down; s->previous_secondary=mapped_back;
    s->previous_menu=menu; s->previous_click=mapped_click;
    if((destination==B2C_GAME || inventory_ui) && !back){
        // Defensive confirmation priority for a malformed primary assignment;
        // valid cycle bindings offer only secondary buttons and stick clicks.
        if(cycle_source>=0 && !(inventory_ui && cycle_source%6==B2B_PRIMARY) &&
           B2B_Value(p,B2B_CYCLE347,dominant,0)>.5f){
            s->wait_neutral=true;
            s->intent.events=B2B_CYCLE338;
            return false;
        }
        bool inventory=B2B_Value(p,B2B_INVENTORY,dominant,0)>.5f;
        bool help=B2B_Value(p,B2B_HELP,dominant,0)>.5f;
        // Primary buttons retain menu confirmation even if Custom binds a
        // gameplay screen toggle to one of those buttons.
        if(inventory_ui){
            int a=B2B_Source(B2B_INVENTORY,dominant),b=B2B_Source(B2B_HELP,dominant);
            if(a>=0 && a%6==B2B_PRIMARY)inventory=false;
            if(b>=0 && b%6==B2B_PRIMARY)help=false;
        }
        if(inventory || help){
            s->wait_neutral=true;
            if(inventory!=help)s->intent.events=inventory?B2B_OPEN_INVENTORY316:B2B_OPEN_HELP316;
            return false;
        }
    }
    if (destination==B2C_MENU || destination==B2C_CONSOLE || inventory_ui) {
        if (back) {s->intent.events=inventory_ui?B2C_OPEN_MENU:B2C_BACK;return false;}
        if (destination==B2C_CONSOLE) return false;
        if (primary) {s->intent.events=B2C_CONFIRM;return false;}
        if(click){s->intent.events=B2C_RECENTRE;return false;}
        int owner=b2c_menu_stick316(p,dominant);
        if(owner<0){s->menu_ready=owner==-1;return false;}
        float x=p->hands[owner].stick[0], y=p->hands[owner].stick[1];
        if (s->menu_ready && (fabsf(x)>.65f || fabsf(y)>.65f)) {
            s->intent.navigation=fabsf(x)>fabsf(y) ? (x>0?B2C_RIGHT:B2C_LEFT) : (y>0?B2C_UP:B2C_DOWN);
            s->menu_ready=false;
        } else if (fabsf(x)<.35f && fabsf(y)<.35f) s->menu_ready=true;
        return false;
    }
    if (back) { s->intent.events=B2C_OPEN_MENU; return false; }
    if (destination==B2C_INACTIVE) return false; // Menu access without gameplay.
    if (destination==B2C_DEAD) {
        // Destination changes require neutral first, so the killing/held fire
        // cannot restart. One fresh trigger edge; stock game owns the delay.
        bool pressed=fire>.55f && !s->attack;
        if (fire>.55f) s->attack=true;
        else if (fire<.45f) s->attack=false;
        s->intent.attack=pressed;
        return false;
    }
    if (destination==B2C_CINEMATIC) {
        if (primary) s->intent.events=B2C_CONFIRM;
        else if(click)s->intent.events=B2C_RECENTRE;
        return false;
    }
    // A separate destination reuses the normal release/freshness/hand gates,
    // but intermission permits only deliberate fire (and the menu above).
    if (destination==B2C_INTERMISSION) {
        if (fire>.55f) s->attack=true;
        else if (fire<.45f) s->attack=false;
        s->intent.attack=s->attack;
        return false;
    }
    if (click) { s->intent.events=B2C_RECENTRE; return false; }
    float x=b2c_axis(B2B_Value(p,B2B_MOVE,dominant,0),.25f), y=b2c_axis(B2B_Value(p,B2B_MOVE,dominant,1),.25f);
    float length=sqrtf(x*x+y*y);
    if (length>1) { x/=length; y/=length; }
    if (speed<0) speed=0;
    if (speed>400) speed=400;
    s->intent.forward=(short)(y*speed); s->intent.side=(short)(x*speed);
    if (B2B_Value(p,B2B_CROUCH,dominant,0)>.65f) s->crouch=true;
    else if (B2B_Value(p,B2B_CROUCH,dominant,0)<.35f) s->crouch=false;
    s->intent.up=200*((int)(B2B_Value(p,B2B_JUMP,dominant,0)>.5f)-(int)s->crouch);
    if (fire>.55f) s->attack=true;
    else if (fire<.45f) s->attack=false;
    s->intent.attack=s->attack;
    if (turn_mode==B2C_TURN_SMOOTH) {
        if (!s->turn_clock_valid) {
            s->turn_clock_valid=true; s->turn_ms=now;
        } else {
            uint32_t elapsed=now-s->turn_ms;
            s->turn_ms=now;
            // Never convert an inactive interval or stall into a catch-up turn.
            if (elapsed>50) return false; // Fresh movement/fire remain valid; discard only overdue yaw.
            s->intent.yaw=-smooth_speed*b2c_axis(B2B_Value(p,B2B_TURN,dominant,0),.25f)*(elapsed*.001f);
        }
    } else if (s->snap_ready && fabsf(B2B_Value(p,B2B_TURN,dominant,0))>.65f) {
        s->intent.yaw=B2B_Value(p,B2B_TURN,dominant,0)>0 ? -snap_angle : snap_angle; s->snap_ready=false;
    } else if (fabsf(B2B_Value(p,B2B_TURN,dominant,0))<.25f) s->snap_ready=true;
    return false;
}

// Existing callers keep the accepted defaults and call contract.
static bool b2c_step_mode(b2c_state_t *s, const b2c_snapshot_t *p, uint32_t now,
    unsigned destination, unsigned dominant, float speed, unsigned turn_mode)
{
    return b2c_step_turn(s,p,now,destination,dominant,speed,turn_mode,360.f,45.f);
}

// Preserve the accepted snap-only fixture/call contract for regression checks.
static bool b2c_step(b2c_state_t *s, const b2c_snapshot_t *p, uint32_t now,
    unsigned destination, unsigned dominant, float speed)
{
    return b2c_step_mode(s,p,now,destination,dominant,speed,B2C_TURN_SNAP);
}
#endif
