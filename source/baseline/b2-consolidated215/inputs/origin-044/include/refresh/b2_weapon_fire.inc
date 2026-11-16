// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Included by the canonical combat owner after b2_weapon_game.inc.
#pragma push_macro("world")
#undef world
#include "b2_weapon_fire.h"
static struct {
    edict_t *player;
    gclient_t *client;
    const gitem_t *weapon;
    b2w_epochs_t epochs;
    b2w_epochs_t armed_epochs;
    uint32_t game_epoch;
    uint32_t armed_game_epoch;
    int slot;
    bool requested, rearm, armed, ordinary, owned, emitted, cancel, in_think, packet_valid, emission;
    bool recovering;
    int recovery_frame;
    bool death, death_ready, death_pending;
    b2w_packet_t packet;
    b2we_owner_t armed_owner, cycle_owner;
    b2wg_state_t grenade;
    vec3_t command_angles;
    b2w_pose_t nominal;
    vec3_t ray_forward, ray_right, ray_up, body_forward, body_right, body_up;
} b2w_fire;
static struct {
    gclient_t *client;
    vec3_t kick, origin;
    gclient_t *recoil_client;
    vec3_t recoil;
    bool aim_prepared;
    b2wn_state_t aim;
    bool prepared, display_emission;
    b2wn_state_t pending, emitted;
    b2w_packet_t pending_owner, emitted_owner;
    bool pending_tracked, emitted_tracked;
} b2wn_previous;
static bool b2w_fire_local(edict_t *ent)
{
    return ent && g_edicts && ent==g_edicts+1 && ent->client;
}
static unsigned b2w_fire_context(void) { return b2w_host.context ? b2w_host.context() : 0; }
static int b2w_fire_slot(edict_t *ent)
{
    for (unsigned i=0;i<12;i++) if (ent->client->pers.weapon==b2w_item(i)) return (int)i;
    return -1;
}
bool B2W_WeaponAligned(edict_t *ent)
{
    return b2w_live_player()==ent && b2w_fire_local(ent) && b2w_host.nominal_enabled &&
        b2w_host.nominal_enabled() && b2w_host.nominal && b2w_fire_slot(ent)>=0 &&
        ((b2w_fire_context()&B2W_CONTEXT_REQUESTED) || ent->client->pers.hand!=CENTER_HANDED);
}
void B2W_WeaponRecoil(edict_t *ent)
{
    if (!b2w_fire_local(ent)) return;
    b2wn_previous.recoil_client=ent->client;
    VectorCopy(ent->client->ps.kick_angles,b2wn_previous.recoil);
}
void B2W_WeaponView(edict_t *ent)
{
    // Keep raw local-player observation current across display and simulation holds.
    // It never authorises input or changes stock recoil generation.
    if (!b2w_fire_local(ent)) return;
    b2wn_previous.client=ent->client;
    VectorCopy(ent->client->kick_angles,b2wn_previous.kick);
    VectorCopy(ent->client->kick_origin,b2wn_previous.origin);
}
static bool b2wn_state(edict_t *ent, bool emission, b2wn_state_t *out)
{
    if (!out || !B2W_WeaponAligned(ent)) return false;
    memset(out,0,sizeof(*out));out->size=sizeof(*out);out->version=B2WN_VERSION;
    out->epoch=combat.epoch+1;out->captured_ms=b2w_host.milliseconds();
    out->slot=b2w_fire_slot(ent);out->frame=level.framenum;out->gunframe=ent->client->ps.gunframe;
    out->gunindex=ent->client->ps.gunindex&GUNINDEX_MASK;out->hand=ent->client->pers.hand;
    out->model_frame=out->gunframe;
    out->idle=ent->client->weaponstate==WEAPON_READY && !ent->client->newweapon;
    for (unsigned i=0;i<3;i++) {
        float oldkick=b2wn_previous.client==ent->client?b2wn_previous.kick[i]:0;
        float oldorigin=b2wn_previous.client==ent->client?b2wn_previous.origin[i]:0;
        out->body_origin[i]=ent->s.origin[i];out->body_angles[i]=ent->client->v_angle[i];
        out->origin[i]=ent->s.origin[i]+ent->client->ps.viewoffset[i]+ent->client->ps.gunoffset[i];
        out->angles[i]=ent->client->v_angle[i]+ent->client->ps.gunangles[i]+ent->client->ps.kick_angles[i];
        out->recoil[i]=b2wn_previous.recoil_client==ent->client?b2wn_previous.recoil[i]:0;
        if (emission) {
            out->angles[i]+=ent->client->kick_angles[i]-oldkick;
            out->recoil[i]+=ent->client->kick_angles[i]-oldkick;
            out->origin[i]+=ent->client->kick_origin[i]-oldorigin;
        }
    }
    return true;
}
static bool b2we_same_packet(const b2w_packet_t *,const b2w_packet_t *);
static bool b2wn_motion(b2wn_state_t *out)
{
    if (!b2wn_state(b2w_live_player(),false,out)) return false;
    const b2wn_state_t *shot=&b2wn_previous.emitted;
    b2w_packet_t current;
    bool owner_current=!b2wn_previous.emitted_tracked ||
        (b2w_host.current && b2w_host.current(&current) &&
         b2we_same_packet(&current,&b2wn_previous.emitted_owner));
    // Stock fire callbacks often advance gunframe before publication. Keep the
    // completed shot's pose visible for its tick; never feed display into fire.
    if (b2wn_previous.display_emission && owner_current && shot->epoch==out->epoch &&
        shot->slot==out->slot && shot->gunindex==out->gunindex && shot->hand==out->hand &&
        out->frame>=shot->frame && out->frame-shot->frame<=1 &&
        (uint32_t)(out->captured_ms-shot->captured_ms)<=100) {
        *out=*shot;out->emission=true;
    }
    return true;
}
static bool b2w_fire_packet(edict_t *ent, b2w_packet_t *p)
{
    return b2w_host.current && b2w_host.current(p) && b2w_live_player()==ent &&
        b2w_game_packet(p,ent) && p->weapon==b2w_fire_slot(ent);
}
static bool b2w_fire_same_cycle(const b2w_packet_t *p)
{
    return p->game_epoch==b2w_fire.game_epoch &&
        !memcmp(&p->pose.epochs,&b2w_fire.epochs,sizeof(b2w_fire.epochs));
}
static bool b2w_fire_armed(const b2w_packet_t *p)
{
    return b2w_fire.armed && p->game_epoch==b2w_fire.armed_game_epoch &&
        !memcmp(&p->pose.epochs,&b2w_fire.armed_epochs,sizeof(b2w_fire.armed_epochs));
}
static bool b2w_fire_same_pose(const b2w_pose_t *a, const b2w_pose_t *b)
{
    // Compare values, not tail padding in the private cross-unit structure.
    return !memcmp(&a->epochs,&b->epochs,sizeof(a->epochs)) && a->serial==b->serial &&
        a->pose_time==b->pose_time && a->capture_ms==b->capture_ms && a->head_ms==b->head_ms && a->hand==b->hand &&
        !memcmp(a->origin,b->origin,sizeof(a->origin)) && !memcmp(a->forward,b->forward,sizeof(a->forward)) &&
        !memcmp(a->right,b->right,sizeof(a->right)) && !memcmp(a->up,b->up,sizeof(a->up));
}

// Separate from cycle retirement: synchronous stock damage/death may retire
// the player during fire_*. The completed event still keeps its original
// identity and is rejected by the engine if that owner is no longer eligible.
static struct {
    edict_t *player;
    gclient_t *client;
    b2we_event_t event;
    bool prepared, begun;
    unsigned grenade_state;
} b2we_pending;
static uint64_t b2we_serial, b2wg_cycle;
static bool b2we_same_packet(const b2w_packet_t *a,const b2w_packet_t *b)
{
    return a->game_epoch==b->game_epoch && a->weapon==b->weapon &&
        a->selection==b->selection && a->selection_ms==b->selection_ms && a->slot==b->slot &&
        b2w_fire_same_pose(&a->pose,&b->pose);
}
static bool b2we_owner(edict_t *ent, const b2w_packet_t *packet, b2we_owner_t *owner)
{
    return b2w_host.emission_owner && b2w_host.emitted && b2w_host.emission_owner(owner) &&
        owner->size==sizeof(*owner) && owner->version==B2WE_VERSION &&
        owner->handle && owner->model.generation && owner->model.calibration==B2WM_CALIBRATION &&
        owner->model.slot==(unsigned)b2w_fire_slot(ent) && b2we_same_packet(packet,&owner->packet);
}

static bool b2we_binding(const b2we_owner_t *a,const b2we_owner_t *b)
{
    return a->handle==b->handle && a->aim.hand==b->aim.hand &&
        a->model.slot==b->model.slot && a->model.calibration==b->model.calibration &&
        a->model.generation==b->model.generation && !memcmp(a->model.sha256,b->model.sha256,32);
}
static bool b2w_grenade(const b2we_owner_t *owner, b2wg_state_t *out)
{
    if (!out) return false;
    memset(out,0,sizeof(*out));
    edict_t *ent=b2w_live_player();b2w_packet_t p;
    if (!owner || owner->size!=sizeof(*owner) || owner->version!=B2WE_VERSION ||
        !owner->handle || !owner->model.generation || owner->model.slot!=5 ||
        owner->model.calibration!=B2WM_CALIBRATION || !b2w_fire_local(ent) ||
        !b2w_fire_packet(ent,&p) || !b2we_same_packet(&p,&owner->packet) ||
        b2w_fire_slot(ent)!=5 || !b2w_fire.requested || b2w_fire.ordinary) return false;
    out->frame=level.framenum;out->gunframe=ent->client->ps.gunframe;
    out->state=B2WG_INTERRUPTED;
    if (b2w_fire.rearm || !b2w_fire_armed(&p) || !b2we_binding(owner,&b2w_fire.armed_owner)) return true;
    if (b2w_fire.owned) {
        if (!b2w_fire_same_cycle(&p) || !b2we_binding(owner,&b2w_fire.cycle_owner)) return true;
        out->cycle=b2w_fire.grenade.cycle;out->emission_serial=b2w_fire.grenade.emission_serial;
        out->state=b2we_pending.begun ? B2WG_INTERRUPTED : b2w_fire.grenade.state;return true;
    }
    if (ent->client->weaponstate==WEAPON_READY && !ent->client->newweapon &&
        ent->client->pers.inventory[ent->client->ammo_index]>0) out->state=B2WG_READY;
    return true;
}
static bool b2w_fire_delayed(void) { return b2w_fire.slot==5 || b2w_fire.slot==10; }
static const int b2w_fire_idle[12]={9,19,18,6,32,16,17,13,21,19,33,14};

// A retired shot keeps its remaining stock recovery, but no firing authority.
// The no-trigger path advances one model frame per simulation frame; chaingun
// spin-down skips from 14 to 32. Do not invoke its fire callback here: even with
// attack released that callback can emit bullets. CPU cases compare every
// affected slot with the selected stock weapon callbacks and Weapon_Generic.
static bool b2w_fire_recovery(edict_t *ent)
{
    if (!b2w_fire.recovering) return false;
    if (ent!=b2w_fire.player || ent->client!=b2w_fire.client || ent->health<1 ||
        ent->client->pers.weapon!=b2w_fire.weapon ||
        ent->client->weaponstate!=WEAPON_FIRING) {
        b2w_fire.recovering=false; return false;
    }
    ent->client->buttons&=~BUTTON_ATTACK;
    ent->client->latched_buttons&=~BUTTON_ATTACK;
    if (level.framenum>b2w_fire.recovery_frame) {
        b2w_fire.recovery_frame=level.framenum;
        if (b2w_fire.slot==4 && ent->client->ps.gunframe==14)
            ent->client->ps.gunframe=32;
        else
            ent->client->ps.gunframe++;
        if (ent->client->ps.gunframe>=b2w_fire_idle[b2w_fire.slot]+1) {
            ent->client->ps.gunframe=b2w_fire_idle[b2w_fire.slot]+1;
            ent->client->weaponstate=WEAPON_READY;
            b2w_fire.recovering=false;
        }
    }
    return true;
}

static void b2w_fire_cancel(edict_t *ent)
{
    b2wn_previous.display_emission=false;
    b2w_fire.packet_valid=b2w_fire.emission=false; b2w_fire.rearm=true;
    if (!b2w_fire.owned || ent!=b2w_fire.player || ent->client!=b2w_fire.client ||
        ent->client->pers.weapon!=b2w_fire.weapon) { b2w_fire.owned=false; return; }
    ent->client->buttons&=~BUTTON_ATTACK; ent->client->latched_buttons&=~BUTTON_ATTACK;
    // Already emitted grenade/BFG entities and their stock cooldown are untouched.
    if (b2w_fire_delayed() && b2w_fire.emitted) return;
    if (b2w_fire.slot==5) b2w_fire.grenade.state=B2WG_INTERRUPTED;
    if (G_CombatObserver(ent)) G_ObserverTrace("weapon.tracked.cancel.before",ent);
    if (ent->client->weaponstate==WEAPON_FIRING) {
        if (b2w_fire.emitted && ent->health>0 &&
            ent->client->ps.gunframe>=0 && ent->client->ps.gunframe<b2w_fire_idle[b2w_fire.slot]+1) {
            b2w_fire.recovering=true;
            b2w_fire.recovery_frame=level.framenum;
        } else {
            ent->client->weaponstate=WEAPON_READY;
            ent->client->ps.gunframe=b2w_fire_idle[b2w_fire.slot];
        }
    }
    if (b2w_fire.slot==5) { ent->client->grenade_framenum=0; ent->client->grenade_blew_up=false; }
    ent->client->weapon_sound=0; ent->client->machinegun_shots=0;
    b2w_fire.owned=false;
    if (G_CombatObserver(ent)) G_ObserverTrace("weapon.tracked.cancel.after",ent);
}
static void b2w_fire_reset(void)
{
    if (b2w_fire_local(b2w_fire.player) && b2w_fire.player->inuse &&
        b2w_fire.player->client==b2w_fire.client) b2w_fire_cancel(b2w_fire.player);
    memset(&b2w_fire,0,sizeof(b2w_fire));
    memset(&b2wn_previous,0,sizeof(b2wn_previous));
    if (!b2we_pending.begun) memset(&b2we_pending,0,sizeof(b2we_pending));
}
static void b2w_fire_transition(edict_t *ent)
{
    bool requested=(b2w_fire_context()&B2W_CONTEXT_REQUESTED)!=0;
    if (!b2w_fire.owned && !b2w_fire.recovering && ent->client->weaponstate==WEAPON_FIRING && requested && !b2w_fire.requested)
        b2w_fire.ordinary=true; // Finish an existing ordinary cycle without stealing it.
    if (b2w_fire.ordinary && ent->client->weaponstate!=WEAPON_FIRING) b2w_fire.ordinary=false;
    if (requested!=b2w_fire.requested) { b2w_fire.rearm=true; b2w_fire.armed=false; b2w_fire.requested=requested; }
}
bool B2W_WeaponTracked(edict_t *ent)
{
    return b2w_fire_local(ent) && !b2w_fire.ordinary &&
        ((b2w_fire_context()&B2W_CONTEXT_REQUESTED) || b2w_fire.owned || b2w_fire.rearm || b2w_fire.recovering);
}
// Keep stock death input separate from tracked firing. Only real new commands
// can release/press; entering death consumes the killing press. The engine has
// already required current controller ownership, focus, tracking and neutral.
// This state is reset by the existing game/lifecycle weapon reset, not saved.
static bool b2w_fire_death(edict_t *ent, bool command)
{
    unsigned context=b2w_fire_context();
    if (!(context&B2W_CONTEXT_REQUESTED) || !ent->deadflag || ent->health>0) {
        b2w_fire.death=b2w_fire.death_ready=b2w_fire.death_pending=false;
        return false;
    }
    int buttons=ent->client->buttons;
    int pending=b2w_fire.death_pending && (ent->client->latched_buttons&BUTTON_ATTACK) ? BUTTON_ATTACK : 0;
    if (!b2w_fire.death) {
        b2w_fire.death=true;
        b2w_fire.death_ready=false;
    }
    b2w_fire_cancel(ent);
    b2w_fire.owned=b2w_fire.ordinary=b2w_fire.armed=b2w_fire.recovering=false;
    b2w_fire.cancel=b2w_fire.in_think=false;
    if (command && (context&B2W_CONTEXT_NEW_COMMAND)) {
        if (!(buttons&BUTTON_ATTACK)) b2w_fire.death_ready=true;
        else if (b2w_fire.death_ready) {
            pending|=BUTTON_ATTACK;
            b2w_fire.death_ready=false;
        }
    }
    b2w_fire.death_pending=pending!=0;
    // Retain the observed held bit for stock oldbuttons edge detection.
    // All tracked firing ownership is retired above; this is death input.
    ent->client->buttons=(ent->client->buttons&~BUTTON_ATTACK)|(buttons&BUTTON_ATTACK);
    ent->client->latched_buttons=(ent->client->latched_buttons&~BUTTON_ATTACK)|pending;
    return true;
}
void B2W_WeaponCommand(edict_t *ent)
{
    if (!b2w_fire_local(ent)) return;
    if (b2w_fire_death(ent,true)) return;
    b2w_fire_transition(ent);
    unsigned context=b2w_fire_context();
    if ((context&B2W_CONTEXT_COMMAND) && !(context&B2W_CONTEXT_NEW_COMMAND)) {
        // Recovered movement cannot retire or rearm a live firing cycle.
        // Keep the last new command's held state without manufacturing a
        // fresh edge on the next command or invoking Think_Weapon here.
        if (b2w_fire.requested || b2w_fire.owned || b2w_fire.rearm) {
            ent->client->buttons=(ent->client->buttons&~BUTTON_ATTACK)|(ent->client->oldbuttons&BUTTON_ATTACK);
            ent->client->latched_buttons&=~BUTTON_ATTACK;
        }
        return;
    }
    if (b2w_fire.ordinary) return;
    bool held=((ent->client->buttons|ent->client->latched_buttons)&BUTTON_ATTACK)!=0;
    b2w_packet_t p; b2we_owner_t owner;
    bool valid=b2w_fire.requested && b2w_fire_packet(ent,&p) && b2we_owner(ent,&p,&owner);
    if ((valid && (!b2w_fire_armed(&p) || !b2we_binding(&owner,&b2w_fire.armed_owner))) || (b2w_fire.owned && (!valid || !b2w_fire_same_cycle(&p)))) b2w_fire_cancel(ent);
    // Only a real new command can release retirement/rearm, never a cleared cache.
    if ((b2w_fire_context()&B2W_CONTEXT_NEW_COMMAND) && !held && !b2w_fire.owned &&
        (!b2w_fire.requested || (valid && !p.attack))) {
        b2w_fire.rearm=false; b2w_fire.armed=valid;
        if (valid) { b2w_fire.armed_epochs=p.pose.epochs; b2w_fire.armed_game_epoch=p.game_epoch; b2w_fire.armed_owner=owner; }
    }
    if (b2w_fire.rearm || (b2w_fire.requested && (!valid || !p.attack))) {
        ent->client->buttons&=~BUTTON_ATTACK; ent->client->latched_buttons&=~BUTTON_ATTACK;
    }
}
bool B2W_WeaponBegin(edict_t *ent)
{
    if (!b2w_fire_local(ent)) return true;
    if (b2w_fire_death(ent,false)) return true;
    // Observe simulation aim before Weapon_Generic changes the model frame
    // and before this callback generates recoil. This grants no firing authority.
    b2wn_previous.aim_prepared=b2wn_state(ent,false,&b2wn_previous.aim);
    b2wn_previous.display_emission=false;
    unsigned context=b2w_fire_context();
    if ((context&B2W_CONTEXT_COMMAND) && !(context&B2W_CONTEXT_NEW_COMMAND) && B2W_WeaponTracked(ent)) return false;
    // Recovery also applies when switching to keyboard/mouse. A mode change
    // cannot turn a retired tracked shot into an immediately ready weapon.
    if (b2w_fire_recovery(ent)) return false;
    b2w_fire_transition(ent); b2w_fire.cancel=false; b2w_fire.emission=false;
    b2w_fire.in_think=true; b2w_fire.packet_valid=false;
    if (b2w_fire.ordinary || !B2W_WeaponTracked(ent)) return true;
    b2w_packet_t p; b2we_owner_t owner;
    bool valid=b2w_fire.requested && b2w_fire_packet(ent,&p) && b2we_owner(ent,&p,&owner);
    if (!valid || b2w_fire.rearm || !b2w_fire_armed(&p) ||
        !b2we_binding(&owner,&b2w_fire.armed_owner) ||
        (b2w_fire.owned && (!b2w_fire_same_cycle(&p) || !b2we_binding(&owner,&b2w_fire.cycle_owner)))) {
        b2w_fire_cancel(ent);
        ent->client->buttons&=~BUTTON_ATTACK; ent->client->latched_buttons&=~BUTTON_ATTACK;
        if (ent->health<1 || (b2w_fire.owned && b2w_fire.emitted && b2w_fire_delayed())) return true;
        b2w_fire.in_think=false; return false;
    }
    b2w_fire.packet=p; b2w_fire.packet_valid=true;
    VectorCopy(ent->client->v_angle,b2w_fire.command_angles);
    if (!p.attack) { ent->client->buttons&=~BUTTON_ATTACK; ent->client->latched_buttons&=~BUTTON_ATTACK; }
    return true;
}
void B2W_WeaponEnd(edict_t *ent)
{
    if (!b2w_fire_local(ent)) return;
    if (b2w_fire.cancel) b2w_fire_cancel(ent);
    if (b2w_fire.owned && (ent->client->weaponstate!=WEAPON_FIRING || ent->client->pers.weapon!=b2w_fire.weapon)) b2w_fire.owned=false;
    b2w_fire.in_think=b2w_fire.packet_valid=b2w_fire.emission=false;
}
bool B2W_WeaponStart(edict_t *ent)
{
    if (!B2W_WeaponTracked(ent)) return true;
    b2w_packet_t p; b2we_owner_t owner;
    if (!b2w_fire.requested || b2w_fire.rearm || !b2w_fire_packet(ent,&p) || !b2w_fire_armed(&p) || !p.attack ||
        !b2we_owner(ent,&p,&owner) || !b2we_binding(&owner,&b2w_fire.armed_owner)) return false;
    if (p.weapon==5 && b2wg_cycle==UINT64_MAX) return false;
    b2w_fire.cycle_owner=owner;
    b2w_fire.player=ent; b2w_fire.client=ent->client; b2w_fire.weapon=ent->client->pers.weapon;
    b2w_fire.slot=p.weapon; b2w_fire.epochs=p.pose.epochs; b2w_fire.game_epoch=p.game_epoch;
    b2w_fire.packet=p; b2w_fire.packet_valid=b2w_fire.owned=true; b2w_fire.emitted=false;
    if (p.weapon==5) b2w_fire.grenade=(b2wg_state_t){.cycle=++b2wg_cycle,.state=B2WG_HELD};
    return true;
}
static void b2w_fire_rotate(const vec3_t value, const vec3_t f, const vec3_t r, const vec3_t u,
    const b2w_pose_t *pose, vec3_t out)
{
    float x=DotProduct(value,f),y=DotProduct(value,r),z=DotProduct(value,u);
    for (unsigned i=0;i<3;i++) out[i]=x*pose->forward[i]+y*pose->right[i]+z*pose->up[i];
}
static bool b2wn_prepare(edict_t *ent, const b2we_owner_t *owner,
    float start[3], float forward[3], float anchor[3])
{
    b2wn_state_t state;b2wn_pose_t sample={0};
    if (!b2wn_state(ent,true,&state)) return false;
    if (state.slot==0 || state.slot==6 || state.slot==7 || state.slot==8 || state.slot==9) {
        const b2wn_state_t *aim=&b2wn_previous.aim;
        if (!b2wn_previous.aim_prepared || !b2w_fire.in_think ||
            aim->epoch!=state.epoch || aim->frame!=state.frame || aim->slot!=state.slot ||
            aim->gunindex!=state.gunindex || aim->hand!=state.hand) return false;
        state.model_frame=aim->model_frame;state.idle=aim->idle;
        VectorCopy(aim->origin,state.origin);VectorCopy(aim->angles,state.angles);
        VectorCopy(aim->recoil,state.recoil);
    }
    if (!b2w_host.nominal(&state,owner,&sample)) return false;
    vec3_t ref,body_f,body_r,body_u,angles,stock_f,stock_r,stock_u,out,delta={0},base,offset;
    VectorCopy(ent->client->v_angle,ref);
    // Machinegun already put its new kick into stock forward. The target
    // model includes that kick once; remove it from the mapping's source frame.
    if (state.slot==3) VectorAdd(ref,ent->client->kick_angles,ref);
    AngleVectors(ref,body_f,body_r,body_u);
    if (state.slot==4 || state.slot==8) {
        VectorSet(offset,state.slot==8?24:0,state.slot==8?8:7,ent->viewheight-8);
        if (ent->client->pers.hand==LEFT_HANDED) offset[1]*=-1;
        else if (ent->client->pers.hand==CENTER_HANDED) offset[1]=0;
        G_ProjectSource(ent->s.origin,offset,body_f,body_r,base);
        VectorSubtract(start,base,delta);
        b2w_fire_rotate(delta,body_f,body_r,body_u,&sample.pose,out);
        VectorAdd(sample.pose.origin,out,sample.pose.origin);
    }
    vec3_t corrected;
    if (!b2wn_clip(ent,sample.pose.origin,corrected)) return false;
    for (unsigned i=0;i<3;i++) anchor[i]=sample.anchor[i]+corrected[i]-sample.pose.origin[i];
    VectorCopy(corrected,start);
    VectorCopy(body_f,b2w_fire.body_forward);VectorCopy(body_r,b2w_fire.body_right);VectorCopy(body_u,b2w_fire.body_up);
    b2w_fire.nominal=sample.pose;
    // Preserve the stock integer-degree spread frame before the rigid mapping.
    vectoangles(forward,angles);AngleVectors(angles,stock_f,stock_r,stock_u);
    b2w_fire_rotate(stock_f,body_f,body_r,body_u,&sample.pose,b2w_fire.ray_forward);
    b2w_fire_rotate(stock_r,body_f,body_r,body_u,&sample.pose,b2w_fire.ray_right);
    b2w_fire_rotate(stock_u,body_f,body_r,body_u,&sample.pose,b2w_fire.ray_up);
    b2w_fire_rotate(forward,body_f,body_r,body_u,&sample.pose,out);VectorCopy(out,forward);
    b2wn_previous.pending=state;b2wn_previous.pending_tracked=owner!=NULL;
    if (owner) b2wn_previous.pending_owner=owner->packet;
    b2wn_previous.prepared=true;return true;
}
bool B2W_WeaponMuzzle(edict_t *ent, float start[3], float forward[3])
{
    b2wn_previous.prepared=false;
    if (!B2W_WeaponTracked(ent)) {
        if (!B2W_WeaponAligned(ent)) return true;
        vec3_t anchor;return b2wn_prepare(ent,NULL,start,forward,anchor);
    }
    b2w_packet_t p; b2we_owner_t owner,checked;vec3_t preview;
    if (!b2we_pending.begun) memset(&b2we_pending,0,sizeof(b2we_pending));
    bool valid=b2w_fire.requested && b2w_fire.owned && b2w_fire_packet(ent,&p) && b2w_fire_same_cycle(&p);
    if (valid && !b2w_fire.in_think) { b2w_fire.packet=p; b2w_fire.packet_valid=true;
        VectorCopy(ent->client->v_angle,b2w_fire.command_angles); }
    if (!valid || b2we_pending.begun || b2we_serial==UINT64_MAX ||
        !b2we_owner(ent,&p,&owner) || !b2we_binding(&owner,&b2w_fire.cycle_owner) || !b2w_fire.packet_valid || !b2w_fire_same_pose(&p.pose,&b2w_fire.packet.pose) ||
        (b2w_fire.slot==5 && b2w_fire.emitted) || !b2w_preview(&b2w_fire.packet,preview) ||
        !b2w_fire_packet(ent,&p) || !b2w_fire_same_cycle(&p) ||
        !b2w_fire_same_pose(&p.pose,&b2w_fire.packet.pose) ||
        !b2we_owner(ent,&p,&checked) || memcmp(&owner,&checked,sizeof(owner))) {
        b2w_fire.cancel=true;
        if (!b2w_fire.in_think) b2w_fire_cancel(ent);
        return false;
    }
    vec3_t anchor;
    if (!b2wn_prepare(ent,&owner,start,forward,anchor) || !b2w_fire_packet(ent,&p) ||
        !b2w_fire_same_cycle(&p) || !b2we_owner(ent,&p,&checked) || memcmp(&owner,&checked,sizeof(owner))) {
        b2wn_previous.prepared=false;b2w_fire.cancel=true;
        if (!b2w_fire.in_think) b2w_fire_cancel(ent);
        return false;
    }
    b2we_pending.player=ent;b2we_pending.client=ent->client;b2we_pending.prepared=true;
    b2we_event_t *e=&b2we_pending.event;
    e->size=sizeof(*e);e->version=B2WE_VERSION;e->owner=owner;
    e->frame=level.framenum;e->gunframe=ent->client->ps.gunframe;
    VectorCopy(start,e->corrected);VectorCopy(forward,e->direction);VectorCopy(anchor,e->anchor);
    VectorCopy(b2w_fire.command_angles,e->body_angles);
    return true;
}
void B2W_WeaponGrenadeEmission(edict_t *ent, bool detonated)
{
    if (B2W_WeaponTracked(ent) && b2w_fire.owned && b2w_fire.slot==5 &&
        b2we_pending.prepared && !b2we_pending.begun &&
        ent==b2we_pending.player && ent->client==b2we_pending.client)
        b2we_pending.grenade_state=detonated ? B2WG_DETONATED : B2WG_RELEASED;
}
void B2W_WeaponEmitted(edict_t *ent)
{
    if (b2w_fire_local(ent) && b2wn_previous.prepared) b2w_fire.emission=true;
    if (B2W_WeaponTracked(ent) && b2w_fire.owned && b2we_pending.prepared &&
        !b2we_pending.begun && ent==b2we_pending.player && ent->client==b2we_pending.client) {
        b2w_fire.emitted=b2w_fire.emission=true;
        b2we_pending.event.serial=++b2we_serial;b2we_pending.begun=true;

    }
}
void B2W_WeaponEmissionEnd(edict_t *ent)
{
    bool display=b2w_fire_local(ent) && b2w_fire.emission && b2wn_previous.prepared;
    b2wn_state_t display_state=b2wn_previous.pending;
    b2w_packet_t display_owner=b2wn_previous.pending_owner;
    bool display_tracked=b2wn_previous.pending_tracked;
    if (b2w_fire_local(ent)) b2w_fire.emission=false;
    if (!b2we_pending.begun || ent!=b2we_pending.player || ent->client!=b2we_pending.client) {
        if (display) { b2wn_previous.emitted=display_state;b2wn_previous.display_emission=true;
            b2wn_previous.emitted_owner=display_owner;b2wn_previous.emitted_tracked=display_tracked; }
        return;
    }
    b2we_event_t event=b2we_pending.event;
    unsigned grenade_state=b2we_pending.grenade_state;
    memset(&b2we_pending,0,sizeof(b2we_pending));
    // A synchronous spawn/damage callback has no completed release yet.
    // Hide that in-flight representation; publish terminal state only after
    // the real stock call has returned and only if its cycle still exists.
    if (grenade_state && b2w_fire.owned && b2w_fire.slot==5 &&
        ent==b2w_fire.player && ent->client==b2w_fire.client &&
        b2we_binding(&event.owner,&b2w_fire.cycle_owner)) {
        b2w_fire.grenade.state=grenade_state;
        b2w_fire.grenade.emission_serial=event.serial;
    }
    event.completed_ms=b2w_host.milliseconds();
    // Only the completed stock call publishes. BFG9 preview/noise and Flare17
    // never reach this boundary; one SSG/chaingun call produces one event.
    // Owner validation must see the prior completed presentation until this
    // event is accepted; publishing its pose earlier would reject its own owner.
    if (b2w_host.emitted(&event) && display) {
        b2wn_previous.emitted=display_state;b2wn_previous.display_emission=true;
        b2wn_previous.emitted_owner=display_owner;b2wn_previous.emitted_tracked=display_tracked;
    }
}
void B2W_WeaponBasis(edict_t *ent, float forward[3], float right[3], float up[3])
{
    if (!b2w_fire_local(ent) || !b2w_fire.emission) return;
    VectorCopy(b2w_fire.ray_forward,forward);
    VectorCopy(b2w_fire.ray_right,right);
    VectorCopy(b2w_fire.ray_up,up);
}
void B2W_WeaponWaterBasis(edict_t *ent, const float direction[3], float forward[3], float right[3], float up[3])
{
    if (!b2w_fire_local(ent) || !b2w_fire.emission) return;
    const b2w_pose_t *p=&b2w_fire.nominal;
    float x=DotProduct(direction,p->forward),y=DotProduct(direction,p->right),z=DotProduct(direction,p->up);
    vec3_t stock,angles,f,r,u;
    for (unsigned i=0;i<3;i++) stock[i]=x*b2w_fire.body_forward[i]+y*b2w_fire.body_right[i]+z*b2w_fire.body_up[i];
    vectoangles(stock,angles); AngleVectors(angles,f,r,u);
    b2w_fire_rotate(f,b2w_fire.body_forward,b2w_fire.body_right,b2w_fire.body_up,p,forward);
    b2w_fire_rotate(r,b2w_fire.body_forward,b2w_fire.body_right,b2w_fire.body_up,p,right);
    b2w_fire_rotate(u,b2w_fire.body_forward,b2w_fire.body_right,b2w_fire.body_up,p,up);
}
void B2W_WeaponBeforeChange(edict_t *ent)
{
    if (!b2w_fire_local(ent) || !b2w_fire.owned) return;
    b2w_packet_t p;
    if (!(b2w_fire_context()&B2W_CONTEXT_REQUESTED) || !b2w_fire_packet(ent,&p) || !b2w_fire_same_cycle(&p)) b2w_fire_cancel(ent);
}
void B2W_WeaponChanged(edict_t *ent)
{
    if (!b2w_fire_local(ent)) return;
    b2wn_previous.display_emission=false;
    if (b2w_fire.owned) b2w_fire.rearm=true;
    b2w_fire.owned=b2w_fire.ordinary=b2w_fire.packet_valid=b2w_fire.recovering=false;
}
#pragma pop_macro("world")
