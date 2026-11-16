// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Included by g_combat.c after its existing observer/publication definitions.
// The stock game header defines `world` as an edict expression. Keep that
// legacy macro out of this private contract and restore it afterwards.
#pragma push_macro("world")
#undef world
#include "b2_weapon_ui.h"
static b2w_host_t b2w_host;
static uint64_t b2w_selection_seen;
static uint32_t b2w_selection_owner, b2w_selection_world;
static const char *const b2w_items[12]={"Blaster","Shotgun","Super Shotgun","Machinegun",
    "Chaingun","Grenades","Grenade Launcher","Rocket Launcher","HyperBlaster","Railgun","BFG10K","Flare Gun"};
static void b2w_fire_reset(void);
static void b2w_game_reset(void)
{
    b2w_fire_reset();b2tp_moved=false;
    b2w_selection_seen=0; b2w_selection_owner=b2w_selection_world=0;
}
static edict_t *b2w_live_player(void)
{
    edict_t *ent=b2_observer;
    return G_CombatObserver(ent) && combat.anchored && !combat.input_hold && !obs_trace.walk_faulted &&
        ent==g_edicts+1 && ent->health>0 && !ent->deadflag && ent->movetype==MOVETYPE_WALK &&
        ent->client->ps.pmove.pm_type==PM_NORMAL && combat.epoch!=UINT32_MAX ? ent : NULL;
}
static const gitem_t *b2w_item(unsigned slot)
{
    const gitem_t *item=slot<12 ? FindItem(b2w_items[slot]) : NULL;
    return item && combat_stock_weapon(item) ? item : NULL;
}
static bool b2w_available(edict_t *ent, const gitem_t *item)
{
    if (!item || ent->client->pers.inventory[ITEM_INDEX(item)]<=0) return false;
    if (!item->ammo || g_select_empty->value || (item->flags&IT_AMMO)) return true;
    const gitem_t *ammo=FindItem(item->ammo);
    return ammo && ent->client->pers.inventory[ITEM_INDEX(ammo)]>=item->quantity;
}
static bool b2w_inventory(b2w_inventory_t *out)
{
    if (!out) return false;
    memset(out,0,sizeof(*out)); out->current=out->pending=-1;
    edict_t *ent=b2w_live_player(); if (!ent) return false;
    out->epoch=combat.epoch+1;
    out->ready=ent->client->weaponstate==WEAPON_READY && !ent->client->newweapon &&
        !ent->client->grenade_framenum && !ent->client->grenade_blew_up &&
        !((ent->client->buttons|ent->client->latched_buttons)&BUTTON_ATTACK);
    for (unsigned i=0;i<12;i++) {
        const gitem_t *item=b2w_item(i); if (!item) continue;
        if (ent->client->pers.inventory[ITEM_INDEX(item)]>0) out->owned|=(uint16_t)(1u<<i);
        if (b2w_available(ent,item)) out->available|=(uint16_t)(1u<<i);
        if (ent->client->pers.weapon==item) out->current=(int)i;
        if (ent->client->newweapon==item) out->pending=(int)i;
    }
    return true;
}
void Use_Quad(edict_t *,const gitem_t *);
void Use_Invulnerability(edict_t *,const gitem_t *);
void Use_Silencer(edict_t *,const gitem_t *);
void Use_Breather(edict_t *,const gitem_t *);
void Use_Envirosuit(edict_t *,const gitem_t *);
void Use_PowerArmor(edict_t *,const gitem_t *);
// Read actual stock item identities/counts; keys, ammunition and passive
// armour never enter this usable-item page. No gameplay state is changed here.
q_exported bool GetB2ItemState(unsigned version, unsigned size, b2wi_inventory_t *out)
{
    if (!out || version!=B2WI_API || size!=sizeof(*out)) return false;
    memset(out,0,sizeof(*out));
    edict_t *ent=b2w_live_player();if (!ent || level.intermission_framenum) return false;
    void (*const uses[B2WI_COUNT])(edict_t *,const gitem_t *)={Use_Quad,Use_Invulnerability,
        Use_Silencer,Use_Breather,Use_Envirosuit,Use_PowerArmor,Use_PowerArmor};
    out->epoch=combat.epoch+1;
    const gitem_t *cells=FindItem("Cells");
    for (unsigned i=0;i<B2WI_COUNT;i++) {
        const gitem_t *item=FindItem(b2wi_names[i]);
        if (!item || item->use!=uses[i] || !item->icon || strcmp(item->icon,b2wi_icons[i])) continue;
        int index=ITEM_INDEX(item);if (index<=0 || index>=MAX_ITEMS) continue;
        out->index[i]=index;out->count[i]=ent->client->pers.inventory[index];
        if (out->count[i]>0) {
            out->owned|=(uint16_t)(1u<<i);
            if (i<5 || (ent->flags&FL_POWER_ARMOR) || (cells && ent->client->pers.inventory[ITEM_INDEX(cells)]>0))
                out->available|=(uint16_t)(1u<<i);
        }
    }
    if (ent->client->quad_framenum>level.framenum) out->active|=1u<<0;
    if (ent->client->invincible_framenum>level.framenum) out->active|=1u<<1;
    if (ent->client->silencer_shots>0) out->active|=1u<<2;
    if (ent->client->breather_framenum>level.framenum) out->active|=1u<<3;
    if (ent->client->enviro_framenum>level.framenum) out->active|=1u<<4;
    int armour=PowerArmorType(ent);
    if (armour==POWER_ARMOR_SCREEN) out->active|=1u<<5;
    if (armour==POWER_ARMOR_SHIELD) out->active|=1u<<6;
    return true;
}

static bool b2w_game_packet(const b2w_packet_t *p, edict_t *ent)
{
    if (!p || !ent || !b2w_host.milliseconds || !p->pose.serial || !p->pose.epochs.action ||
        !p->pose.epochs.xr || !p->pose.epochs.world || !p->pose.epochs.owner || !p->pose.epochs.calibration ||
        p->pose.hand>1 || p->pose.pose_time<=0 || p->game_epoch!=combat.epoch+1) return false;
    uint32_t now=b2w_host.milliseconds();
    if ((uint32_t)(now-p->pose.capture_ms)>100 || (uint32_t)(now-p->pose.head_ms)>100) return false;
    const float *basis[3]={p->pose.forward,p->pose.right,p->pose.up};
    for (unsigned i=0;i<3;i++) {
        if (!isfinite(p->pose.origin[i])) return false;
        float n=0;
        for (unsigned j=0;j<3;j++) { if (!isfinite(basis[i][j])) return false; n+=basis[i][j]*basis[i][j]; }
        if (fabsf(n-1)>.02f) return false;
    }
    if (fabsf(DotProduct(basis[0],basis[1]))>.02f || fabsf(DotProduct(basis[0],basis[2]))>.02f ||
        fabsf(DotProduct(basis[1],basis[2]))>.02f) return false;
    vec3_t cross; CrossProduct(basis[0],basis[2],cross);
    return DotProduct(cross,basis[1])>.98f;
}
static bool b2wn_clip(edict_t *ent, const float desired[3], float corrected[3])
{
    if (!ent || !desired || !corrected) return false;
    VectorClear(corrected);
    vec3_t eye,delta,end;
    VectorCopy(ent->s.origin,eye); eye[2]+=(float)ent->viewheight;
    VectorSubtract(desired,eye,delta);
    float distance=VectorLength(delta);
    if (!isfinite(distance) || distance>96) return false;
    VectorCopy(desired,end);
    trace_t tr=gi.trace(eye,NULL,NULL,end,ent,MASK_SHOT);
    if (tr.startsolid || tr.allsolid || !isfinite(tr.fraction) || tr.fraction<0 || tr.fraction>1) return false;
    VectorCopy(desired,end);
    if (tr.fraction<1) {
        if (distance<=1 || distance*tr.fraction<=1) return false;
        VectorMA(eye,(distance*tr.fraction-1)/distance,delta,end);
    }
    tr=gi.trace(eye,NULL,NULL,end,ent,MASK_SHOT);
    if (tr.startsolid || tr.allsolid || tr.fraction!=1) return false;
    tr=gi.trace(end,NULL,NULL,end,ent,MASK_SHOT);
    if (tr.startsolid || tr.allsolid || tr.fraction!=1) return false;
    VectorCopy(end,corrected); return true;
}
static bool b2w_preview(const b2w_packet_t *packet, float corrected[3])
{
    edict_t *ent=b2w_live_player();
    return b2w_game_packet(packet,ent) && b2wn_clip(ent,packet->pose.origin,corrected);
}
static bool b2w_select(uint64_t token, int slot)
{
    b2w_packet_t p; b2w_inventory_t inventory;
    if (!token || slot<0 || slot>=12 || !b2w_host.current || !b2w_host.current(&p) ||
        p.selection!=token || p.slot!=slot || p.attack || !b2w_inventory(&inventory)) return false;
    edict_t *ent=b2w_live_player();
    if (!b2w_game_packet(&p,ent) || (uint32_t)(b2w_host.milliseconds()-p.selection_ms)>100) return false;
    if (b2w_selection_owner!=p.pose.epochs.owner || b2w_selection_world!=p.pose.epochs.world) {
        b2w_selection_seen=0; b2w_selection_owner=p.pose.epochs.owner; b2w_selection_world=p.pose.epochs.world;
    }
    if (token<=b2w_selection_seen) return false;
    b2w_selection_seen=token; // Consume even if inventory/ammo changed: no late replay.
    if (!inventory.ready) return false;
    const gitem_t *item=b2w_item((unsigned)slot);
    if (!b2w_available(ent,item)) return false;
    Use_Weapon(ent,item); // Existing stock ammo policy and observer-approved pending writer.
    return ent->client->pers.weapon==item || ent->client->newweapon==item;
}
#include "b2_teleport_game264.inc"
static bool b2w_grenade(const b2we_owner_t *owner, b2wg_state_t *out);
static bool b2wn_motion(b2wn_state_t *out);
q_exported bool GetB2WeaponAPI(unsigned version, unsigned size,
    const b2w_host_t *host, b2w_game_api_t *out)
{
    if (version!=B2W_API || size!=sizeof(b2w_game_api_t) || !out || !host ||
        host->version!=B2W_API || host->size!=sizeof(*host) || !host->milliseconds || !host->current || !host->context ||
        !host->emission_owner || !host->emitted || !host->nominal_enabled || !host->nominal || !host->room_current || !host->room_accepted || !host->teleport_enabled || !host->teleport_current || !host->teleport_result) return false;
    b2w_host=*host;
    *out=(b2w_game_api_t){sizeof(*out),B2W_API,B2W_CAP_INVENTORY|B2W_CAP_PREVIEW|B2W_CAP_SELECTION|B2W_CAP_FIRE,
        b2w_inventory,b2w_preview,b2w_select,b2w_game_reset,b2w_grenade,b2wn_motion,b2tp_game_preview};
    return true; // Tracked mode remains opt-in; the host and game retain per-command firing guards.
}
#pragma pop_macro("world")
