/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/*
Copyright (C) 1997-2001 Id Software, Inc.

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
/* Modified by Togorian with AI assistance, 12 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */

// g_combat.c

#include "g_local.h"
#include "../../../hangar-push452/source/b2_trigger_push452.h"
#include "shared/b2_combat.h"
static void b2w_game_reset(void);
static bool b2tp_use_allowed(edict_t *ent);

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
bool CanDamage(edict_t *targ, edict_t *inflictor)
{
    vec3_t  dest;
    trace_t trace;

// bmodels need special checking because their origin is 0,0,0
    if (targ->movetype == MOVETYPE_PUSH) {
        VectorAdd(targ->absmin, targ->absmax, dest);
        VectorScale(dest, 0.5f, dest);
        trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
        if (trace.fraction == 1.0f)
            return true;
        if (trace.ent == targ)
            return true;
        return false;
    }

    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0f)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] += 15.0f;
    dest[1] += 15.0f;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0f)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] += 15.0f;
    dest[1] -= 15.0f;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0f)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] -= 15.0f;
    dest[1] += 15.0f;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0f)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] -= 15.0f;
    dest[1] -= 15.0f;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0f)
        return true;

    return false;
}

/*
============
Killed
============
*/
void Killed(edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    if (targ->health < -999)
        G_CombatHealth(targ, -999);

    targ->enemy = attacker;

    if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD)) {
//      targ->svflags |= SVF_DEADMONSTER;   // now treat as a different content type
        if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY)) {
            level.killed_monsters++;
            if (coop->value && attacker->client)
                attacker->client->resp.score++;
            // medics won't heal monsters that they kill themselves
            if (strcmp(attacker->classname, "monster_medic") == 0)
                targ->owner = attacker;
        }
    }

    if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE) {
        // doors, triggers, etc
        targ->die(targ, inflictor, attacker, damage, point);
        return;
    }

    if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD)) {
        targ->touch = NULL;
        monster_death_use(targ);
    }

    targ->die(targ, inflictor, attacker, damage, point);
}

/*
================
SpawnDamage
================
*/
void SpawnDamage(int type, const vec3_t origin, const vec3_t normal, int damage)
{
    if (damage > 255)
        damage = 255;
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(type);
//  gi.WriteByte (damage);
    gi.WritePosition(origin);
    gi.WriteDir(normal);
    gi.multicast(origin, MULTICAST_PVS);
}

/*
============
T_Damage

targ        entity that is being damaged
inflictor   entity that is causing the damage
attacker    entity that caused the inflictor to damage targ
    example: targ=monster, inflictor=rocket, attacker=player

dir         direction of the attack
point       point at which the damage is being inflicted
normal      normal vector from that point
damage      amount of damage being inflicted
knockback   force to be applied against targ as a result of the damage

dflags      these flags are used to control how T_Damage works
    DAMAGE_RADIUS           damage was indirect (from a nearby explosion)
    DAMAGE_NO_ARMOR         armor does not protect from this damage
    DAMAGE_ENERGY           damage is from an energy based weapon
    DAMAGE_NO_KNOCKBACK     do not affect velocity, just view angles
    DAMAGE_BULLET           damage is from a bullet (used for ricochets)
    DAMAGE_NO_PROTECTION    kills godmode, armor, everything
============
*/
static int CheckPowerArmor(edict_t *ent, const vec3_t point, const vec3_t normal, int damage, int dflags)
{
    gclient_t   *client;
    int         save;
    int         power_armor_type;
    int         index;
    int         damagePerCell;
    int         pa_te_type;
    int         power;
    int         power_used;

    if (!damage)
        return 0;

    client = ent->client;

    if (dflags & DAMAGE_NO_ARMOR)
        return 0;

    index = 0;  // shut up gcc

    if (client) {
        power_armor_type = PowerArmorType(ent);
        if (power_armor_type != POWER_ARMOR_NONE) {
            index = ITEM_INDEX(FindItem("Cells"));
            power = client->pers.inventory[index];
        }
    } else if (ent->svflags & SVF_MONSTER) {
        power_armor_type = ent->monsterinfo.power_armor_type;
        power = ent->monsterinfo.power_armor_power;
    } else
        return 0;

    if (power_armor_type == POWER_ARMOR_NONE)
        return 0;
    if (!power)
        return 0;

    if (power_armor_type == POWER_ARMOR_SCREEN) {
        vec3_t      vec;
        float       dot;
        vec3_t      forward;

        // only works if damage point is in front
        AngleVectors(ent->s.angles, forward, NULL, NULL);
        VectorSubtract(point, ent->s.origin, vec);
        VectorNormalize(vec);
        dot = DotProduct(vec, forward);
        if (dot <= 0.3f)
            return 0;

        damagePerCell = 1;
        pa_te_type = TE_SCREEN_SPARKS;
        damage = damage / 3;
    } else {
        damagePerCell = 2;
        pa_te_type = TE_SHIELD_SPARKS;
        damage = (2 * damage) / 3;
    }

    save = power * damagePerCell;
    if (!save)
        return 0;
    if (save > damage)
        save = damage;

    SpawnDamage(pa_te_type, point, normal, save);
    ent->powerarmor_framenum = level.framenum + 0.2f * BASE_FRAMERATE;

    power_used = save / damagePerCell;

    if (client)
        G_CombatInventory(ent, index, client->pers.inventory[index] - power_used, "powerarmor.cells");
    else
        ent->monsterinfo.power_armor_power -= power_used;
    return save;
}

static int CheckArmor(edict_t *ent, const vec3_t point, const vec3_t normal, int damage, int te_sparks, int dflags)
{
    gclient_t   *client;
    int         save;
    int         index;
    const gitem_t   *armor;

    if (!damage)
        return 0;

    client = ent->client;

    if (!client)
        return 0;

    if (dflags & DAMAGE_NO_ARMOR)
        return 0;

    index = ArmorIndex(ent);
    if (!index)
        return 0;

    armor = GetItemByIndex(index);

    if (dflags & DAMAGE_ENERGY)
        save = ceil(((const gitem_armor_t *)armor->info)->energy_protection * damage);
    else
        save = ceil(((const gitem_armor_t *)armor->info)->normal_protection * damage);
    if (save >= client->pers.inventory[index])
        save = client->pers.inventory[index];

    if (!save)
        return 0;

    G_CombatInventory(ent, index, client->pers.inventory[index] - save, "armor.absorption");
    SpawnDamage(te_sparks, point, normal, save);

    return save;
}

void M_ReactToDamage(edict_t *targ, edict_t *attacker)
{
    if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
        return;

    if (attacker == targ || attacker == targ->enemy)
        return;

    // dead monsters, like misc_deadsoldier, don't have AI functions, but 
    // M_ReactToDamage might still be called on them
    if (targ->svflags & SVF_DEADMONSTER)
        return;

    // if we are a good guy monster and our attacker is a player
    // or another good guy, do not get mad at them
    if (targ->monsterinfo.aiflags & AI_GOOD_GUY) {
        if (attacker->client || (attacker->monsterinfo.aiflags & AI_GOOD_GUY))
            return;
    }

    // we now know that we are not both good guys

    // if attacker is a client, get mad at them because he's good and we're not
    if (attacker->client) {
        targ->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

        // this can only happen in coop (both new and old enemies are clients)
        // only switch if can't see the current enemy
        if (targ->enemy && targ->enemy->client) {
            if (visible(targ, targ->enemy)) {
                targ->oldenemy = attacker;
                return;
            }
            targ->oldenemy = targ->enemy;
        }
        targ->enemy = attacker;
        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
        return;
    }

    // it's the same base (walk/swim/fly) type and a different classname and it's not a tank
    // (they spray too much), get mad at them
    if (((targ->flags & (FL_FLY | FL_SWIM)) == (attacker->flags & (FL_FLY | FL_SWIM))) &&
        (strcmp(targ->classname, attacker->classname) != 0) &&
        (strcmp(attacker->classname, "monster_tank") != 0) &&
        (strcmp(attacker->classname, "monster_supertank") != 0) &&
        (strcmp(attacker->classname, "monster_makron") != 0) &&
        (strcmp(attacker->classname, "monster_jorg") != 0)) {
        if (targ->enemy && targ->enemy->client)
            targ->oldenemy = targ->enemy;
        targ->enemy = attacker;
        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
    }
    // if they *meant* to shoot us, then shoot back
    else if (attacker->enemy == targ) {
        if (targ->enemy && targ->enemy->client)
            targ->oldenemy = targ->enemy;
        targ->enemy = attacker;
        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
    }
    // otherwise get mad at whoever they are mad at (help our buddy) unless it is us!
    else if (attacker->enemy && attacker->enemy != targ) {
        if (targ->enemy && targ->enemy->client)
            targ->oldenemy = targ->enemy;
        targ->enemy = attacker->enemy;
        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
    }
}

bool CheckTeamDamage(edict_t *targ, edict_t *attacker)
{
    //FIXME make the next line real and uncomment this block
    // if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
    return false;
}

static cvar_t *b2_observer_protect, *b2_observer_xr, *b2_observer_advance;
static cvar_t *b2_combat;
// Stock single-player HUD stats end before slot 30. Retain the complete
// publication identity across the two existing 16-bit network stat words.
enum { B2_COMBAT_SERIAL_HIGH_STAT = B2_COMBAT_STAT - 1 };
static struct {
    bool enabled, death_scope, returning, anchored, released, requested, input_hold;
    bool death_was_dead, death_was_gib, startup_equipped;
    bool restored_anchor_pending, restored_anchor_valid;
    bool client_begin_angles;
    bool level_exiting, restore_pending, restore_warming, restore_body;
    int restore_frame;
    unsigned restore_ticks;
    vec3_t restored_origin, restored_velocity;
    int restored_health;
    char mapname[MAX_QPATH];
    unsigned serial, epoch, returns, relocations;
    vec3_t origin, angles;
    struct { player_state_t ps; b2_combat_state_t state; int frame; } publications[64];
} combat;
static edict_t *b2_observer;
static bool b2_observer_configured, b2_observer_hit_logged;

// Temporary, observer-only attribution. No gameplay state is written here.
// One baseline and one command's collision context; no retained frame history.
#define OBS_TRACE_LIMIT 512
#define OBS_DAMAGE_LIMIT 32
#define OBS_CONTACT_LIMIT 4
typedef struct {
    vec3_t origin, velocity, mins, maxs;
    pmove_state_t pm;
    int health, stat_health, gun, ground, ground_link, link, solid, movetype;
    int flags, svflags, clipmask, dead, takedamage, waterlevel, watertype;
    int inventory[MAX_ITEMS];
    const gitem_t *weapon, *pending_weapon;
    int ammo_index;
} observer_state_t;
static struct {
    bool enabled, valid, command_active, pm_active;
    bool walk_guard, walk_faulted, heading_valid, turning, firing, vertical;
    bool ground_contact, stand_blocked, ladder_contact, first_motion_trace;
    unsigned pm_trace_calls;
    unsigned content_count, world_events;
    struct { vec3_t point; int contents; } content[8];
    short heading[3];
    unsigned events, damage_events, commands, contacts, item_events, pickup_events, rejected_pickups, weapon_events, use_events;
    bool weapon_changing;
    const gitem_t *changing_to;
    struct { unsigned added, spent, writes, selected, fire_ammo; } usage[MAX_ITEMS];
    int command_frame;
    float command_time;
    const char *phase;
    observer_state_t last;
    usercmd_t command;
    pmove_state_t pm_before;
    bool snapinitial;
    struct { trace_t trace; vec3_t start, end; } contact[OBS_CONTACT_LIMIT];
    // Last eight actual callbacks include the final stock snap attempts.
    // Retained for one command, printed only on its first sticky refusal.
    struct { trace_t trace; vec3_t start, end; } pm_tail[8];
    unsigned pm_tail_count;
} obs_trace;

bool G_ObserverTracing(edict_t *ent)
{
    return (obs_trace.enabled || obs_trace.walk_guard) && b2_observer && b2_observer->inuse &&
        b2_observer->client == game.clients && (!ent || ent == b2_observer);
}

static int observer_number(edict_t *ent) { return ent ? (int)(ent - g_edicts) : -1; }
static const char *observer_class(edict_t *ent) { return ent && ent->classname ? ent->classname : "none"; }

static bool b2tp_moved; // Private authorised teleport timer, reset at game boundaries.
static void observer_walk_fault(const char *phase, edict_t *source)
{
    if (obs_trace.walk_faulted) return;
    // Restore admission can fail before a loopback observer is selected.
    static const vec3_t zero = {0,0,0};
    const edict_t *reported = b2_observer ? b2_observer : source;
    const float *origin = reported ? reported->s.origin : zero;
    const float *velocity = reported ? reported->velocity : zero;
    obs_trace.walk_faulted = true;
    // One sticky, process-local report to the existing client scene owner.
    // Do not repair position/velocity or alter normal physics here.
    gi.cvar_forceset("b2_walk_fault", phase);
    gi.dprintf("B2_WALK_FAULT frame=%d time=%.3f phase=%s source=%d class=%.80s "
        "before_origin=%.9g,%.9g,%.9g origin=%.9g,%.9g,%.9g before_velocity=%.9g,%.9g,%.9g velocity=%.9g,%.9g,%.9g "
        "client_terminal_stop_required=1\n", level.framenum, level.time, phase,
        observer_number(source), observer_class(source),
        obs_trace.last.origin[0], obs_trace.last.origin[1], obs_trace.last.origin[2],
        origin[0], origin[1], origin[2],
        obs_trace.last.velocity[0], obs_trace.last.velocity[1], obs_trace.last.velocity[2],
        velocity[0], velocity[1], velocity[2]);
}

static void observer_read(observer_state_t *s)
{
    edict_t *e = b2_observer;
    memset(s, 0, sizeof(*s));
    VectorCopy(e->s.origin, s->origin); VectorCopy(e->velocity, s->velocity);
    VectorCopy(e->mins, s->mins); VectorCopy(e->maxs, s->maxs);
    s->pm = e->client->ps.pmove;
    s->health = e->health; s->stat_health = e->client->ps.stats[STAT_HEALTH]; s->gun = e->client->ps.gunindex;
    s->ground = observer_number(e->groundentity); s->ground_link = e->groundentity_linkcount;
    s->link = e->linkcount; s->solid = e->solid; s->movetype = e->movetype;
    s->flags = e->flags; s->svflags = e->svflags; s->clipmask = e->clipmask;
    s->dead = e->deadflag; s->takedamage = e->takedamage;
    s->waterlevel = e->waterlevel; s->watertype = e->watertype;
    memcpy(s->inventory, e->client->pers.inventory, sizeof(s->inventory));
    s->weapon = e->client->pers.weapon; s->pending_weapon = e->client->newweapon;
    s->ammo_index = e->client->ammo_index;
}

static void observer_print_pm(const char *which, const pmove_state_t *p)
{
    gi.dprintf("B2_ATTR_PM event=%u which=%s origin=%d,%d,%d velocity=%d,%d,%d delta=%d,%d,%d type=%d flags=%d time=%d gravity=%d\n",
        obs_trace.events, which, p->origin[0], p->origin[1], p->origin[2],
        p->velocity[0], p->velocity[1], p->velocity[2], p->delta_angles[0], p->delta_angles[1], p->delta_angles[2],
        p->pm_type, p->pm_flags, p->pm_time, p->gravity);
}

static void observer_print_state(const char *which, const observer_state_t *s)
{
    gi.dprintf("B2_ATTR_STATE event=%u which=%s origin=%.9g,%.9g,%.9g velocity=%.9g,%.9g,%.9g "
        "mins=%.9g,%.9g,%.9g maxs=%.9g,%.9g,%.9g health=%d stat_health=%d gun=%d "
        "ground=%d ground_link=%d link=%d solid=%d movetype=%d flags=%d svflags=%d clipmask=%d dead=%d takedamage=%d water=%d,%d\n",
        obs_trace.events, which, s->origin[0], s->origin[1], s->origin[2], s->velocity[0], s->velocity[1], s->velocity[2],
        s->mins[0], s->mins[1], s->mins[2], s->maxs[0], s->maxs[1], s->maxs[2], s->health, s->stat_health, s->gun,
        s->ground, s->ground_link, s->link, s->solid, s->movetype, s->flags, s->svflags, s->clipmask, s->dead, s->takedamage,
        s->waterlevel, s->watertype);
    observer_print_pm(which, &s->pm);
}

static bool observer_sample(const char *phase, edict_t *source, bool force)
{
    observer_state_t now, comparison;
    bool changed;
    if (!G_ObserverTracing(NULL)) return false;
    observer_read(&now);
    if (combat.enabled && obs_trace.valid && !combat.death_scope && !combat.returning) {
        if (now.pm.pm_flags & (PMF_NO_PREDICTION | PMF_TELEPORT_BIT))
            observer_walk_fault("unsupported_combat_flags", source);
        if ((now.pm.pm_flags & PMF_TIME_TELEPORT) &&
            (!(obs_trace.last.pm.pm_flags & PMF_TIME_TELEPORT) || now.pm.pm_time > obs_trace.last.pm.pm_time))
            observer_walk_fault("unexplained_teleport_timer", source);
        if (now.health != obs_trace.last.health || now.dead != obs_trace.last.dead ||
            now.movetype != obs_trace.last.movetype ||
            memcmp(now.pm.delta_angles, obs_trace.last.pm.delta_angles, sizeof(now.pm.delta_angles)))
            observer_walk_fault("unexplained_combat_state", source);
        if (memcmp(now.inventory, obs_trace.last.inventory, sizeof(now.inventory)))
            observer_walk_fault("unexplained_inventory_writer", source);
        if (combat.anchored && (now.weapon != obs_trace.last.weapon ||
            now.pending_weapon != obs_trace.last.pending_weapon || now.ammo_index != obs_trace.last.ammo_index))
            observer_walk_fault("unexplained_weapon_writer", source);
        if (now.gun != obs_trace.last.gun && !(now.dead && !now.gun))
            observer_walk_fault("unexplained_combat_weapon", source);
        if (now.pm.pm_type != obs_trace.last.pm.pm_type &&
            !(now.dead && (now.pm.pm_type == PM_DEAD || now.pm.pm_type == PM_GIB)))
            observer_walk_fault("unexplained_combat_pm_type", source);
    }
    if (obs_trace.walk_guard && obs_trace.valid && strcmp(phase, "Pmove.applied")) {
        if (obs_trace.vertical && ((now.pm.pm_flags ^ obs_trace.last.pm.pm_flags) & (PMF_DUCKED | PMF_JUMP_HELD | PMF_TIME_WATERJUMP)))
            observer_walk_fault("external_vertical_flags", source);
        if (combat.enabled && (now.waterlevel != obs_trace.last.waterlevel || now.watertype != obs_trace.last.watertype))
            observer_walk_fault("external_water_state", source);
        for (unsigned axis = 0; axis < 3; ++axis) {
            if (now.origin[axis] != obs_trace.last.origin[axis] ||
                now.velocity[axis] != obs_trace.last.velocity[axis]) {
                observer_walk_fault(phase, source);
                break;
            }
        }
    }
    comparison = now;
    // Relinking alone is routine on every applied command. Retain counts in
    // output, but do not spend the event budget on an otherwise unchanged link.
    comparison.link = obs_trace.last.link;
    comparison.ground_link = obs_trace.last.ground_link;
    changed = !obs_trace.valid || memcmp(&comparison, &obs_trace.last, sizeof(now));
    if (obs_trace.enabled && (changed || force) && obs_trace.events < OBS_TRACE_LIMIT) {
        obs_trace.events++;
        gi.dprintf("B2_ATTR event=%u frame=%d time=%.3f phase=%s previous=%s baseline=%d changed=%d "
            "source=%d class=%.80s target=%.80s source_origin=%.9g,%.9g,%.9g source_velocity=%.9g,%.9g,%.9g\n",
            obs_trace.events, level.framenum, level.time, phase, obs_trace.phase ? obs_trace.phase : "none", obs_trace.valid, changed,
            observer_number(source), observer_class(source), source && source->target ? source->target : "none",
            source ? source->s.origin[0] : 0, source ? source->s.origin[1] : 0, source ? source->s.origin[2] : 0,
            source ? source->velocity[0] : 0, source ? source->velocity[1] : 0, source ? source->velocity[2] : 0);
        observer_print_state("before", &obs_trace.last);
        observer_print_state("after", &now);
        gi.dprintf("B2_ATTR_CMD event=%u ordinal=%u active=%d frame=%d time=%.3f msec=%u angles=%d,%d,%d "
            "move=%d,%d,%d buttons=%u impulse=%u light=%u network_sequence=unavailable_in_game_API\n",
            obs_trace.events, obs_trace.commands, obs_trace.command_active, obs_trace.command_frame, obs_trace.command_time,
            obs_trace.command.msec, obs_trace.command.angles[0], obs_trace.command.angles[1], obs_trace.command.angles[2],
            obs_trace.command.forwardmove, obs_trace.command.sidemove, obs_trace.command.upmove,
            obs_trace.command.buttons, obs_trace.command.impulse, obs_trace.command.lightlevel);
        if (obs_trace.events == OBS_TRACE_LIMIT)
            gi.dprintf("B2_ATTR_LIMIT events=%d further_events_omitted=1 attribution_after_this_point_incomplete=1\n", OBS_TRACE_LIMIT);
    } else {
        force = false; changed = false;
    }
    obs_trace.last = now; obs_trace.valid = true; obs_trace.phase = phase;
    return changed || force;
}

void G_ObserverTrace(const char *phase, edict_t *source) { observer_sample(phase, source, false); }

void G_ObserverMeleeSuppressed(edict_t *source, edict_t *recipient)
{
    if (G_ObserverTracing(recipient)) observer_sample("fire_hit.kick.suppressed", source, true);
}

#include "../../../square410/source/command410.inc"

static bool combat_stock_weapon(const gitem_t *item)
{
    // Authenticate the table entry before dereferencing a possibly unknown pointer.
    for (int i = 1; i < game.num_items; ++i)
        if (item == GetItemByIndex(i))
            return (item->flags & IT_WEAPON) && item->weaponthink && item->use == Use_Weapon;
    return false;
}

bool G_ObserverWeaponAllowed(edict_t *ent)
{
    if ((!G_IsProtectedObserver(ent) && !G_CombatObserver(ent)) || !obs_trace.firing) return true;
    if (combat.enabled) G_ObserverTrace("weapon.before", ent);
    if (combat.enabled && ent->deadflag) return !obs_trace.walk_faulted; // normal ChangeWeapon clears the dead weapon
    gclient_t *client = ent->client;
    if (!combat.enabled) {
        if (!obs_trace.walk_faulted && client->pers.weapon &&
            !strcmp(client->pers.weapon->classname, "weapon_blaster") &&
            (!client->newweapon || client->newweapon == client->pers.weapon)) return true;
    } else if (!obs_trace.walk_faulted && combat_stock_weapon(client->pers.weapon) &&
        (!client->newweapon || (combat_stock_weapon(client->newweapon) &&
         client->pers.inventory[ITEM_INDEX(client->newweapon)] > 0)) &&
        client->ammo_index == (client->pers.weapon->ammo ? ITEM_INDEX(FindItem(client->pers.weapon->ammo)) : 0) &&
        client->weaponstate >= WEAPON_READY && client->weaponstate <= WEAPON_FIRING)
        return true;
    observer_walk_fault("unsupported_weapon", ent);
    client->buttons &= ~BUTTON_ATTACK;
    client->latched_buttons &= ~BUTTON_ATTACK;
    return false; // do not execute another weapon even once
}

void G_CombatPendingWeapon(edict_t *ent, const gitem_t *item)
{
    bool tracked = G_CombatObserver(ent) && !combat.returning;
    if (tracked) {
        G_ObserverTrace("weapon.select.before", ent);
        if (item && (!combat_stock_weapon(item) || ent->client->pers.inventory[ITEM_INDEX(item)] <= 0)) {
            observer_walk_fault("invalid_stock_weapon_selection", ent); return;
        }
    }
    ent->client->newweapon = item;
    if (tracked) obs_trace.last.pending_weapon = item;
}

void G_CombatWeaponChange(edict_t *ent, bool before)
{
    if (!G_CombatObserver(ent) || combat.returning) return;
    if (before) {
        G_ObserverTrace("weapon.change.before", ent);
        if (obs_trace.weapon_changing) observer_walk_fault("nested_weapon_change", ent);
        obs_trace.weapon_changing = true; obs_trace.changing_to = ent->client->newweapon;
        return;
    }
    const gitem_t *item = ent->client->pers.weapon;
    bool valid = item ? combat_stock_weapon(item) : ent->health <= 0;
    if (!obs_trace.weapon_changing || item != obs_trace.changing_to || ent->client->newweapon || !valid ||
        ent->client->ammo_index != (item && item->ammo ? ITEM_INDEX(FindItem(item->ammo)) : 0) ||
        ent->client->ps.gunindex != (item ? gi.modelindex(item->view_model) : 0))
        observer_walk_fault("invalid_stock_weapon_change", ent);
    obs_trace.weapon_changing = false;
    // Admit only the checked stock ChangeWeapon outputs, not its callbacks.
    obs_trace.last.weapon = item; obs_trace.last.pending_weapon = ent->client->newweapon;
    obs_trace.last.ammo_index = ent->client->ammo_index; obs_trace.last.gun = ent->client->ps.gunindex;
    G_ObserverTrace("weapon.change.after", ent);
    if (item && valid) obs_trace.usage[ITEM_INDEX(item)].selected++;
    if (obs_trace.weapon_events < 128) {
        obs_trace.weapon_events++;
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_WEAPON frame=%d current=%.80s gun=%d ammo_index=%d stock_change=1\n",
            level.framenum, item && valid ? item->pickup_name : "none", ent->client->ps.gunindex, ent->client->ammo_index);
        if (obs_trace.weapon_events == 128) if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_WEAPON_LIMIT events=128 state_guard_continues=1\n");
    }
}

void G_CombatItemUse(edict_t *ent, const char *command)
{
    if (!G_CombatObserver(ent)) return;
    G_ObserverTrace("stock.command.after", ent);
    if (obs_trace.use_events < 128) {
        obs_trace.use_events++;
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_USE frame=%d command=%.80s selected=%d quad_until=%d invulnerable_until=%d breather_until=%d enviro_until=%d silencer=%d powerarmor=%d\n",
            level.framenum, command, ent->client->pers.selected_item, ent->client->quad_framenum,
            ent->client->invincible_framenum, ent->client->breather_framenum, ent->client->enviro_framenum,
            ent->client->silencer_shots, (ent->flags & FL_POWER_ARMOR) != 0);
        if (obs_trace.use_events == 128) if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_USE_LIMIT events=128 state_guard_continues=1\n");
    }
}

void G_CombatReport(void)
{
    if (!combat.enabled) return;
    for (int i = 1; i < game.num_items; ++i) {
        if (!obs_trace.usage[i].writes && !obs_trace.usage[i].selected && !obs_trace.usage[i].fire_ammo) continue;
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_USAGE item=%.80s index=%d added=%u spent=%u writes=%u selected=%u fire_ammo=%u\n",
            GetItemByIndex(i)->pickup_name, i, obs_trace.usage[i].added, obs_trace.usage[i].spent,
            obs_trace.usage[i].writes, obs_trace.usage[i].selected, obs_trace.usage[i].fire_ammo);
    }
    if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_USAGE_END frame=%d fault=%d event_caps_do_not_limit_totals=1\n", level.framenum, obs_trace.walk_faulted);
}

static bool observer_stock_snap_fallback(const pmove_t *pm, const vec3_t sampled_origin)
{
    // PM_SnapPosition restores previous_origin only after these eight exact
    // stationary jitter tests all fail. Water was categorized before them.
    if (obs_trace.pm_tail_count < 8) return false;
    if (obs_trace.snapinitial) {
        // PM_InitialSnapPosition can replace previous_origin by one eighth
        // per axis, or leave it unchanged when all 27 candidates fail.
        // Initial categorisation does not move that position: its first
        // actual content sample independently identifies the restore target.
        if (!obs_trace.content_count) return false;
        for (unsigned axis = 0; axis < 3; ++axis) {
            float initial = obs_trace.content[0].point[axis] -
                (axis == 2 ? (int)pm->mins[2] + 1 : 0);
            if (!isfinite(initial) || initial != pm->s.origin[axis] * .125f ||
                abs((int)pm->s.origin[axis] - obs_trace.pm_before.origin[axis]) > 1)
                return false;
        }
    } else if (memcmp(pm->s.origin, obs_trace.pm_before.origin, sizeof(pm->s.origin))) {
        return false;
    }
    static const byte jitterbits[8] = {0, 4, 1, 2, 3, 5, 6, 7};
    int base[3], sign[3];
    for (unsigned axis = 0; axis < 3; ++axis) {
        if (!isfinite(sampled_origin[axis]) || fabsf(sampled_origin[axis]) > 4095.f) return false;
        base[axis] = (int)(sampled_origin[axis] * 8);
        sign[axis] = base[axis] * .125f == sampled_origin[axis] ? 0 : sampled_origin[axis] >= 0 ? 1 : -1;
    }
    for (unsigned i = 0; i < 8; ++i) {
        unsigned n = (obs_trace.pm_tail_count - 8 + i) % 8;
        if (!obs_trace.pm_tail[n].trace.allsolid) return false;
        for (unsigned axis = 0; axis < 3; ++axis) {
            float expected = (base[axis] + ((jitterbits[i] & (1 << axis)) ? sign[axis] : 0)) * .125f;
            if (obs_trace.pm_tail[n].start[axis] != expected || obs_trace.pm_tail[n].end[axis] != expected) return false;
        }
    }
    return true;
}

static void observer_check_walk_pmove(edict_t *ent, const pmove_t *pm)
{
    if (!obs_trace.walk_guard || obs_trace.walk_faulted) return;
    const pmove_state_t *old = &obs_trace.pm_before, *now = &pm->s;
    vec3_t velocity, next_velocity, delta;
    for (unsigned i = 0; i < 3; ++i) {
        velocity[i] = old->velocity[i] * .125f;
        next_velocity[i] = now->velocity[i] * .125f;
        delta[i] = (now->origin[i] - old->origin[i]) * .125f;
    }
    float dt = obs_trace.command.msec * .001f;
    // The stock Pmove invokes at most three initial water samples, two
    // climb-out ledge samples and three final water samples. Retain those
    // actual callback results; do not infer water from a published flag.
    int initial_water = 0;
    for (unsigned i = 0; i < min(obs_trace.content_count, 3u); i++) {
        if (!(obs_trace.content[i].contents & MASK_WATER)) break;
        initial_water++;
    }
    bool water_samples = pm->waterlevel >= 0 && pm->waterlevel <= 3 && obs_trace.content_count <= 8;
    unsigned samples = pm->waterlevel == 3 ? 3u : (unsigned)pm->waterlevel + 1u;
    if (obs_trace.content_count < samples) water_samples = false;
    bool snap_fallback = false;
    vec3_t sampled_origin = {0};
    if (water_samples) {
        unsigned first = obs_trace.content_count - samples;
        int heights[3] = { (int)pm->mins[2] + 1,
            (int)pm->mins[2] + ((int)(pm->viewheight - pm->mins[2])) / 2, (int)pm->viewheight };
        VectorCopy(obs_trace.content[first].point, sampled_origin); sampled_origin[2] -= heights[0];
        snap_fallback = observer_stock_snap_fallback(pm, sampled_origin);
        for (unsigned i = 0; i < samples; i++) {
            const int contents = obs_trace.content[first+i].contents;
            if (((contents & MASK_WATER) != 0) != (i < (unsigned)pm->waterlevel)) water_samples = false;
            for (unsigned axis = 0; axis < 3; axis++)
                if (!isfinite(obs_trace.content[first+i].point[axis]) ||
                    fabsf(obs_trace.content[first+i].point[axis] - ((snap_fallback ? sampled_origin[axis] : now->origin[axis] * .125f) + (axis == 2 ? heights[i] : 0))) > .5f)
                    water_samples = false;
        }
        if (pm->watertype != (pm->waterlevel ? obs_trace.content[first].contents : 0)) water_samples = false;
    }
    bool ledge = false;
    if (combat.enabled && obs_trace.vertical && initial_water == 2 &&
        (!old->pm_time || ((old->pm_flags & PMF_TIME_WATERJUMP) && obs_trace.ground_contact)))
        for (unsigned i = 1; i < min(obs_trace.content_count, 8u); i++) {
            const float *a = obs_trace.content[i-1].point, *b = obs_trace.content[i].point;
            float x = a[0] - old->origin[0] * .125f, y = a[1] - old->origin[1] * .125f;
            if ((obs_trace.content[i-1].contents & CONTENTS_SOLID) && !obs_trace.content[i].contents &&
                a[0] == b[0] && a[1] == b[1] && b[2] - a[2] == 16.f &&
                fabsf(sqrtf(x*x+y*y)-30.f) < .5f && fabsf(a[2]-old->origin[2]*.125f-4.f) < .5f)
                ledge = true;
        }
    bool water_time = combat.enabled && (old->pm_flags & PMF_TIME_WATERJUMP);
    bool invalid_waterjump = (now->pm_flags & PMF_TIME_WATERJUMP) && (!combat.enabled || (!water_time && !ledge));
    // Bound one actual PM_NORMAL integration, not a published snapshot interval.
    // PmoveInit / PmoveEnableQW cap wishspeed at 300 / 320; the largest
    // acceleration is 10. Friction and normalized-plane slide clipping do not
    // increase speed. Include full incoming speed (direction-change momentum
    // and slopes), acceleration and gravity, rather than treating wishspeed as
    // a cap on resultant speed. PM_NORMAL does not use spectator speedmult.
    unsigned ticks = max(1, obs_trace.command.msec >> 3);
    if (now->pm_flags & PMF_TIME_WATERJUMP) {
        unsigned duration = ledge ? 255u : old->pm_time;
        if (duration <= ticks || now->pm_time != duration - ticks) invalid_waterjump = true;
    }
    bool jump_rise = !(old->pm_flags & PMF_JUMP_HELD) && (now->pm_flags & PMF_JUMP_HELD);
    bool authorised_jump = obs_trace.vertical && jump_rise && obs_trace.command.upmove == 200 &&
        obs_trace.ground_contact && initial_water < 2 && velocity[2] <= 180.f &&
        !((old->pm_flags & PMF_TIME_LAND) && old->pm_time > ticks);
    // PM_CheckJump adds/clamps Z to 270 only on a released, grounded jump.
    // Account for that known impulse before the existing integration bound;
    // do not grant extra speed to held, airborne or uncommanded input.
    vec3_t initial_velocity; VectorCopy(velocity, initial_velocity);
    if (authorised_jump) initial_velocity[2] = max(velocity[2] + 270.f, 270.f);
    // A stock swim impulse can set Z to100/80/50 without JUMP_HELD,
    // but waterhack or landing delay can skip it. This is a speed norm:
    // preserve incoming downward momentum as well as the possible impulse.
    if (combat.enabled && initial_water >= 2 && obs_trace.command.upmove == 200 &&
        !(old->pm_flags & PMF_JUMP_HELD) && velocity[2] > -300.f)
        initial_velocity[2] = max(fabsf(initial_velocity[2]), 100.f);
    float initial_speed = VectorLength(initial_velocity);
    if (ledge) initial_speed = max(initial_speed, sqrtf(50.f*50.f + 350.f*350.f));
    float travel_speed = initial_speed + (10.f * 320.f + fabsf(old->gravity)) * dt;
    float travel = travel_speed * dt;
    if (snap_fallback) {
        // The attempted position must satisfy the same integration bounds as
        // a successful result; restoring old origin grants no extra travel.
        vec3_t attempted;
        for (unsigned axis = 0; axis < 3; ++axis) attempted[axis] = sampled_origin[axis] - old->origin[axis] * .125f;
        if (sqrtf(attempted[0]*attempted[0] + attempted[1]*attempted[1]) > travel + 1.f ||
            fabsf(attempted[2]) > travel + 19.f) water_samples = false;
    }
    // Initial snap moves at most 1/8 per axis; final truncation+jitter <1/4.
    // One unit covers their combined vector error; step choice adds <=18 Z.
    bool teleport_time = combat.enabled && (combat.relocations || b2tp_moved) && (old->pm_flags & PMF_TIME_TELEPORT);
    bool teleport_after = teleport_time && old->pm_time > ticks;
    if (!!(now->pm_flags & PMF_TIME_TELEPORT) != teleport_after ||
        (teleport_after && now->pm_time != old->pm_time - ticks))
        observer_walk_fault("unexplained_teleport_pmove_timer", ent);
    unsigned unsupported = (teleport_time ? 0 : PMF_TIME_TELEPORT) | (combat.enabled ? 0 : PMF_TIME_WATERJUMP) |
        (obs_trace.vertical ? 0 : PMF_DUCKED | PMF_JUMP_HELD);
    bool invalid_vertical = false;
    if (obs_trace.vertical && old->pm_type == PM_NORMAL) {
        bool was_ducked = old->pm_flags & PMF_DUCKED, ducked = now->pm_flags & PMF_DUCKED;
        bool duck_command = obs_trace.command.upmove == -200 && (old->pm_flags & PMF_ON_GROUND);
        bool expected_duck = duck_command || (was_ducked && obs_trace.stand_blocked);
        bool held = now->pm_flags & PMF_JUMP_HELD, was_held = old->pm_flags & PMF_JUMP_HELD;
        bool landing_delay = ((old->pm_flags & PMF_TIME_LAND) && old->pm_time > ticks) ||
            (!(old->pm_flags & PMF_ON_GROUND) && velocity[2] < -200.f && obs_trace.ground_contact);
        invalid_vertical = (jump_rise && !authorised_jump) || ducked != expected_duck ||
            (was_held && !held && obs_trace.command.upmove >= 10) ||
            (was_held && held && obs_trace.command.upmove < 10 && !landing_delay && !water_time && !ledge && !teleport_after) ||
            (!combat.enabled && (pm->waterlevel || pm->watertype || obs_trace.ladder_contact)) ||
            pm->viewheight != (ducked ? -2.f : 22.f) || pm->mins[2] != -24.f || pm->maxs[2] != (ducked ? 4.f : 32.f);
    }
    usercmd_t expected = obs_trace.command;
    bool dead_move = combat.enabled && ent->deadflag &&
        old->pm_type == (ent->s.modelindex == MODELINDEX_PLAYER ? PM_DEAD : PM_GIB);
    if (dead_move) {
        expected.forwardmove = expected.sidemove = expected.upmove = 0; // actual Pmove clears these
        invalid_vertical =
            pm->viewheight != (old->pm_type == PM_GIB ? 8.f : -2.f) ||
            pm->mins[2] != (old->pm_type == PM_GIB ? 0.f : -24.f) ||
            pm->maxs[2] != (old->pm_type == PM_GIB ? 16.f : 4.f);
    }
    // Keep each refusal predicate and bound unchanged. Report exact reasons
    // even after the ordinary attribution event budget has been exhausted.
    unsigned reasons = (!obs_trace.command_active ? BIT(0) : 0) |
        (invalid_vertical ? BIT(1) : 0) | (invalid_waterjump ? BIT(2) : 0) |
        (!water_samples ? BIT(3) : 0) | (memcmp(&pm->cmd, &expected, sizeof(pm->cmd)) ? BIT(4) : 0) |
        ((!dead_move && old->pm_type != PM_NORMAL) ? BIT(5) : 0) | (now->pm_type != old->pm_type ? BIT(6) : 0) |
        (((old->pm_flags | now->pm_flags) & unsupported) ? BIT(7) : 0) |
        (old->gravity != now->gravity ? BIT(8) : 0) |
        (memcmp(old->delta_angles, now->delta_angles, sizeof(old->delta_angles)) ? BIT(9) : 0) |
        (sqrtf(delta[0]*delta[0] + delta[1]*delta[1]) > travel + 1.f ? BIT(10) : 0) |
        (fabsf(delta[2]) > travel + 19.f ? BIT(11) : 0) |
        (VectorLength(next_velocity) > travel_speed + 1.f ? BIT(12) : 0);
    if (reasons) {
        gi.dprintf("B2_WALK_PM_FAULT msec=%u delta=%.6f,%.6f,%.6f before_speed=%.6f result_speed=%.6f "
            "horizontal_limit=%.6f vertical_limit=%.6f speed_limit=%.6f\n",
            obs_trace.command.msec, delta[0], delta[1], delta[2], VectorLength(velocity),
            VectorLength(next_velocity), travel+1.f, travel+19.f, travel_speed+1.f);
        gi.dprintf("B2_WALK_PM_REASON frame=%d mask=0x%x command_active=%d vertical=%d waterjump=%d water_samples=%d "
            "snapinitial=%d ground_contact=%d stand_blocked=%d ladder=%d ledge=%d water=%d,%d samples=%u "
            "move=%d,%d,%d viewheight=%.9g mins=%.9g,%.9g,%.9g maxs=%.9g,%.9g,%.9g\n",
            level.framenum, reasons, obs_trace.command_active, invalid_vertical, invalid_waterjump, water_samples,
            obs_trace.snapinitial, obs_trace.ground_contact, obs_trace.stand_blocked, obs_trace.ladder_contact,
            ledge, pm->waterlevel, pm->watertype, obs_trace.content_count,
            obs_trace.command.forwardmove, obs_trace.command.sidemove, obs_trace.command.upmove,
            pm->viewheight, pm->mins[0], pm->mins[1], pm->mins[2], pm->maxs[0], pm->maxs[1], pm->maxs[2]);
        observer_print_pm("fault_pm_input", old); observer_print_pm("fault_pm_result", now);
        for (unsigned i = 0; i < min(obs_trace.content_count, 8u); ++i)
            gi.dprintf("B2_WALK_PM_CONTENT index=%u point=%.9g,%.9g,%.9g contents=%d\n", i,
                obs_trace.content[i].point[0], obs_trace.content[i].point[1], obs_trace.content[i].point[2], obs_trace.content[i].contents);
        for (unsigned i = obs_trace.pm_tail_count > 8 ? obs_trace.pm_tail_count - 8 : 0; i < obs_trace.pm_tail_count; ++i) {
            unsigned n = i % 8; const trace_t *t = &obs_trace.pm_tail[n].trace;
            gi.dprintf("B2_WALK_PM_TRACE ordinal=%u fraction=%.9g startsolid=%d allsolid=%d start=%.9g,%.9g,%.9g end=%.9g,%.9g,%.9g result=%.9g,%.9g,%.9g contents=%d\n",
                i, t->fraction, t->startsolid, t->allsolid,
                obs_trace.pm_tail[n].start[0], obs_trace.pm_tail[n].start[1], obs_trace.pm_tail[n].start[2],
                obs_trace.pm_tail[n].end[0], obs_trace.pm_tail[n].end[1], obs_trace.pm_tail[n].end[2],
                t->endpos[0], t->endpos[1], t->endpos[2], t->contents);
        }
        observer_walk_fault("unsupported_pmove_result", ent);
    }
    if (combat.enabled && !obs_trace.walk_faulted && obs_trace.world_events < 128 &&
        (pm->waterlevel != ent->waterlevel || pm->watertype != ent->watertype || ledge || obs_trace.ladder_contact || snap_fallback)) {
        obs_trace.world_events++;
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_WORLD_MOVE frame=%d water=%d,%d ladder=%d climb_out=%d flags=%d time=%d contents_verified=1 snap_fallback=%d\n",
            level.framenum, pm->waterlevel, pm->watertype, obs_trace.ladder_contact, ledge, now->pm_flags, now->pm_time, snap_fallback);
    }
}

void G_ObserverPmove(edict_t *ent, const pmove_t *pm, bool before)
{
    if (!G_ObserverTracing(ent)) return;
    if (!before) observer_check_walk_pmove(ent, pm);
    obs_trace.pm_active = before;
    if (before) {
        obs_trace.pm_before = pm->s; obs_trace.snapinitial = pm->snapinitial; obs_trace.contacts = 0;
        obs_trace.content_count = 0; obs_trace.pm_tail_count = 0;
        obs_trace.ground_contact = obs_trace.stand_blocked = obs_trace.ladder_contact = obs_trace.first_motion_trace = false; obs_trace.pm_trace_calls = 0;
        G_ObserverTrace("Pmove.before", ent);
    } else if (memcmp(&obs_trace.pm_before, &pm->s, sizeof(pm->s)) && observer_sample("Pmove.result", ent, true)) {
        observer_print_pm("pm_input", &obs_trace.pm_before); observer_print_pm("pm_result", &pm->s);
        gi.dprintf("B2_ATTR_PM_RESULT event=%u snapinitial=%d ground=%d touches=%d contacts=%u retained=%u omitted=%u water=%d,%d\n",
            obs_trace.events, obs_trace.snapinitial, observer_number(pm->groundentity), pm->numtouch,
            obs_trace.contacts, min(obs_trace.contacts, OBS_CONTACT_LIMIT),
            obs_trace.contacts > OBS_CONTACT_LIMIT ? obs_trace.contacts - OBS_CONTACT_LIMIT : 0, pm->waterlevel, pm->watertype);
        for (unsigned i = 0; i < obs_trace.contacts && i < OBS_CONTACT_LIMIT; i++) {
            const trace_t *t = &obs_trace.contact[i].trace;
            gi.dprintf("B2_ATTR_CONTACT event=%u index=%u entity=%d class=%.80s fraction=%.9g startsolid=%d allsolid=%d "
                "start=%.9g,%.9g,%.9g end=%.9g,%.9g,%.9g result=%.9g,%.9g,%.9g plane=%.9g,%.9g,%.9g dist=%.9g contents=%d\n",
                obs_trace.events, i, observer_number(t->ent), observer_class(t->ent), t->fraction, t->startsolid, t->allsolid,
                obs_trace.contact[i].start[0], obs_trace.contact[i].start[1], obs_trace.contact[i].start[2],
                obs_trace.contact[i].end[0], obs_trace.contact[i].end[1], obs_trace.contact[i].end[2],
                t->endpos[0], t->endpos[1], t->endpos[2], t->plane.normal[0], t->plane.normal[1], t->plane.normal[2], t->plane.dist, t->contents);
        }
    }
}

int G_ObserverPointContents(const vec3_t point)
{
    int contents = gi.pointcontents(point);
    if (G_ObserverTracing(NULL) && obs_trace.pm_active) {
        unsigned index = obs_trace.content_count++;
        if (index < q_countof(obs_trace.content)) {
            VectorCopy(point, obs_trace.content[index].point);
            obs_trace.content[index].contents = contents;
        }
    }
    return contents;
}

void G_ObserverCollision(const vec3_t start, const vec3_t end, const trace_t *trace)
{
    if (!G_ObserverTracing(NULL) || !obs_trace.pm_active) return;
    unsigned tail = obs_trace.pm_tail_count++ % 8;
    obs_trace.pm_tail[tail].trace = *trace;
    VectorCopy(start, obs_trace.pm_tail[tail].start); VectorCopy(end, obs_trace.pm_tail[tail].end);
    if (obs_trace.vertical) {
        // PM_CheckDuck is the first trace when an already ducked player tries
        // to stand. Retain its actual allsolid result, including clear traces.
        if (!obs_trace.pm_trace_calls && (obs_trace.pm_before.pm_flags & PMF_DUCKED) &&
            !(obs_trace.command.upmove < 0 && (obs_trace.pm_before.pm_flags & PMF_ON_GROUND)) &&
            !memcmp(start, end, sizeof(vec3_t))) obs_trace.stand_blocked = trace->allsolid;
        if (!obs_trace.first_motion_trace && memcmp(start, end, sizeof(vec3_t))) {
            // Initial duck/snap tests are stationary. The first moving trace
            // is initial ground categorization, before jump/ladder/movement;
            // a later landing contact must not authorise an earlier impulse.
            obs_trace.ground_contact = start[0] == end[0] && start[1] == end[1] &&
                fabsf(start[2] - end[2] - .25f) < .0001f && trace->ent &&
                (trace->plane.normal[2] >= .7f || trace->startsolid);
            obs_trace.first_motion_trace = true;
        }
        if (trace->fraction < 1.f && (trace->contents & CONTENTS_LADDER)) obs_trace.ladder_contact = true;
        obs_trace.pm_trace_calls++;
    }
    if (trace->fraction == 1 && !trace->startsolid && !trace->allsolid) return;
    if (obs_trace.contacts < OBS_CONTACT_LIMIT) {
        obs_trace.contact[obs_trace.contacts].trace = *trace;
        VectorCopy(start, obs_trace.contact[obs_trace.contacts].start); VectorCopy(end, obs_trace.contact[obs_trace.contacts].end);
    }
    obs_trace.contacts++;
}

static void observer_damage(edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, int kick, int flags, int mod, const char *decision)
{
    if (!G_ObserverTracing(targ) || obs_trace.damage_events >= OBS_DAMAGE_LIMIT) return;
    if (!observer_sample("T_Damage.decision", attacker, true)) return;
    obs_trace.damage_events++;
    gi.dprintf("B2_ATTR_DAMAGE event=%u decision=%s attacker=%d class=%.80s inflictor=%d inflictor_class=%.80s damage=%d kick=%d flags=%d mod=%d\n",
        obs_trace.events, decision, observer_number(attacker), observer_class(attacker), observer_number(inflictor), observer_class(inflictor), damage, kick, flags, mod);
    if (obs_trace.damage_events == OBS_DAMAGE_LIMIT)
        gi.dprintf("B2_ATTR_DAMAGE_LIMIT events=%d further_damage_decisions_omitted=1 state_change_tracking_continues=1\n", OBS_DAMAGE_LIMIT);
}

static bool protected_observer_mode(void)
{
    bool mode = (b2_observer_protect->value == 1 && b2_combat->value == 0) ||
        (b2_observer_protect->value == 0 && b2_combat->value == 1 &&
         gi.cvar("b2_keyboard_locomotion", "0", 0)->value == 1 &&
         gi.cvar("b2_turning_mouselook", "0", 0)->value == 1 &&
         gi.cvar("b2_blaster_fire", "0", 0)->value == 1 &&
         gi.cvar("b2_jump_crouch", "0", 0)->value == 1);
    return mode && b2_observer_xr->value == 1 &&
        b2_observer_advance->value == 1 && !dedicated->value &&
        !deathmatch->value && !coop->value && maxclients->value == 1 &&
        game.maxclients == 1;
}

void G_InitProtectedObserver(void)
{
    b2w_game_reset();
    b2_observer_protect = gi.cvar("b2_observer_protect", "0", CVAR_LATCH);
    b2_combat = gi.cvar("b2_combat", "0", CVAR_LATCH);
    // Stock load can precede ClientBegin: register saved settings at game init.
    gi.cvar("b2_startup_give_all", "0", CVAR_LATCH);
    memset(&combat, 0, sizeof(combat));
    b2_observer_xr = gi.cvar("b2_openxr", "0", 0);
    b2_observer_advance = gi.cvar("b2_advance_world", "0", 0);
    b2_observer = NULL;
    b2_observer_hit_logged = false;
    b2_observer_configured = protected_observer_mode();
    combat.enabled = b2_observer_configured && b2_combat->value == 1;
    Q_strlcpy(combat.mapname, gi.cvar("b2_combat_map", "base1", CVAR_LATCH)->string, sizeof(combat.mapname));
    memset(&obs_trace, 0, sizeof(obs_trace));
    obs_trace.enabled = gi.cvar("b2_observer_trace", "0", CVAR_LATCH)->value == 1 && b2_observer_configured;
    obs_trace.walk_guard = gi.cvar("b2_keyboard_locomotion", "0", 0)->value == 1 && b2_observer_configured;
    obs_trace.turning = obs_trace.walk_guard && gi.cvar("b2_turning_mouselook", "0", 0)->value == 1;
    obs_trace.vertical = obs_trace.walk_guard && gi.cvar("b2_jump_crouch", "0", 0)->value == 1;
    obs_trace.firing = obs_trace.turning && gi.cvar("b2_blaster_fire", "0", 0)->value == 1;
    gi.cvar("b2_walk_fault", "", CVAR_NOSET);
    gi.cvar_forceset("b2_walk_fault", "");
    if (obs_trace.walk_guard)
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_WALK_GUARD enabled=1 allowed_writer=Pmove.applied keyboard_walk=200 keyboard_run=400 stock_pmove_cap=1 sticky_fault=1 independent_of_trace_budget=1\n");
    if (obs_trace.enabled)
        gi.dprintf("B2_ATTR_CONFIG enabled=1 event_limit=%d damage_limit=%d contacts_per_command=%d observer_only=1 behavior_unchanged=1\n",
            OBS_TRACE_LIMIT, OBS_DAMAGE_LIMIT, OBS_CONTACT_LIMIT);
    if (b2_observer_protect->value)
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_OBSERVER configured=%d before_world_ticks=1 option=%g; "
            "requires local single-player B2 advancing and living loopback player 1\n",
            b2_observer_configured, b2_observer_protect->value);
}

// Transient publications/guard baselines are not save data. These hooks are
// called only by the actual spawn/read/intermission writers, outside a frame.
void G_CombatWorldBoundary(const char *map, bool restoring)
{
    b2w_game_reset();
    if (!combat.enabled || gi.cvar("b2_lifecycle","0",0)->value!=1) return;
    if (combat.restore_warming) observer_walk_fault("unfinished_restore_warming", g_edicts + 1);
    bool equipped=combat.startup_equipped || restoring;
    memset(&combat.publications,0,sizeof(combat.publications));
    combat.serial=combat.epoch=combat.returns=combat.relocations=0;
    combat.anchored=combat.returning=combat.death_scope=false;
    combat.client_begin_angles=false;
    combat.released=combat.requested=false; combat.input_hold=true;
    combat.startup_equipped=equipped;
    // ReadLevel has decoded the real saved edict; ClientConnect/ClientBegin
    // must still authenticate the local player before any publication.
    const edict_t *restored = g_edicts + 1;
    combat.level_exiting = false;
    combat.restore_pending = restoring;
    combat.restore_warming = false;
    combat.restore_body = restoring && restored->inuse;
    combat.restore_frame = level.framenum;
    combat.restore_ticks = 0;
    combat.restored_anchor_pending = combat.restore_body;
    combat.restored_anchor_valid = restoring && restored->inuse &&
        restored->health > 0 && !restored->deadflag && restored->movetype == MOVETYPE_WALK;
    if (restoring) {
        VectorCopy(restored->s.origin, combat.restored_origin);
        VectorCopy(restored->velocity, combat.restored_velocity);
        combat.restored_health = restored->health;
    }
    Q_strlcpy(combat.mapname,map,sizeof(combat.mapname));
    obs_trace.valid=obs_trace.heading_valid=obs_trace.command_active=obs_trace.pm_active=false;
    b2_observer=NULL;
    if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_LIFECYCLE game_boundary map=%s restore=%d transient_publications_discarded=1 inventory_untouched=1\n",map,restoring);
}

void G_CombatIntermission(edict_t *ent, bool before)
{
    if (!G_CombatObserver(ent) || gi.cvar("b2_lifecycle","0",0)->value!=1) return;
    if (before) { G_ObserverTrace("intermission.before",ent); return; }
    if (!level.intermission_framenum || ent->client->ps.pmove.pm_type!=PM_FREEZE || (ent->solid!=SOLID_NOT && !level.exitintermission)) {
        observer_walk_fault("invalid_intermission_writer",ent); return;
    }
    observer_read(&obs_trace.last); obs_trace.valid=true;
    obs_trace.heading_valid=false;
    combat.epoch++; combat.relocations++; combat.input_hold=true;
    G_CombatPublish(ent);
}


bool G_CombatExitPending(const edict_t *ent)
{
    return combat.level_exiting && G_CombatObserver(ent);
}

q_exported bool B2CombatLevelExit(unsigned version, const char *command)
{
    char expected[256];
    if (version != B2_COMBAT_API || !command || !G_CombatObserver(b2_observer) ||
        gi.cvar("b2_lifecycle","0",0)->value != 1 || obs_trace.walk_faulted ||
        !combat.anchored || combat.level_exiting || !level.exitintermission || !level.changemap)
        return false;
    int n = Q_snprintf(expected, sizeof(expected), "gamemap \"%s\"\n", level.changemap);
    if (n < 0 || n >= sizeof(expected) || strcmp(command, expected)) return false;
    combat.level_exiting = true;
    if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_LIFECYCLE game_exit frame=%d authenticated=1 before_command_queue=1\n", level.framenum);
    return true;
}

static bool combat_restore_body_valid(const edict_t *ent)
{
    if (!ent->inuse || ent->client != game.clients || ent->health <= 0 || ent->deadflag ||
        ent->movetype != MOVETYPE_WALK || ent->client->ps.pmove.pm_type != PM_NORMAL ||
        ent->client->pers.connected || ent->client->pers.spectator || ent->client->resp.spectator)
        return false;
    for (unsigned i=0; i<3; i++)
        if (!isfinite(ent->s.origin[i]) || !isfinite(ent->velocity[i]) ||
            ent->s.origin[i] < -4096.f || ent->s.origin[i] > 4095.875f ||
            ent->velocity[i] < -4096.f || ent->velocity[i] > 4095.875f)
            return false;
    return true;
}

q_exported bool B2CombatWarming(unsigned version, bool begin, unsigned ticks)
{
    edict_t *ent = g_edicts + 1;
    bool valid = version == B2_COMBAT_API && combat.enabled && protected_observer_mode() &&
        gi.cvar("b2_lifecycle","0",0)->value == 1 && !obs_trace.walk_faulted &&
        combat.restore_pending && !combat.anchored && !b2_observer &&
        ent->client == game.clients && !ent->client->pers.connected &&
        (ticks == 2 || ticks == 100) && !!ent->inuse == combat.restore_body;
    if (combat.restore_body) valid = valid && combat_restore_body_valid(ent);
    if (begin) {
        valid = valid && !combat.restore_warming && level.framenum == combat.restore_frame;
        if (combat.restore_body)
            valid = valid && combat.restored_anchor_valid && ent->health == combat.restored_health &&
                !memcmp(ent->s.origin,combat.restored_origin,sizeof(vec3_t)) &&
                !memcmp(ent->velocity,combat.restored_velocity,sizeof(vec3_t));
    } else valid = valid && combat.restore_warming && ticks == combat.restore_ticks &&
        level.framenum == combat.restore_frame + ticks;
    if (!valid) {
        combat.restore_warming = combat.restore_pending = false;
        observer_walk_fault("invalid_restore_warming",ent);
        return false;
    }
    if (begin) { combat.restore_warming = true; combat.restore_ticks = ticks; }
    else {
        if (combat.restore_body) {
            VectorCopy(ent->s.origin,combat.restored_origin);
            VectorCopy(ent->velocity,combat.restored_velocity);
            combat.restored_health = ent->health;
        }
        combat.restore_warming = combat.restore_pending = false;
    }
    if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_LIFECYCLE restore_warming begin=%d ticks=%u frame=%d body=%d connected=0 bounded_server_phase=1\n",
        begin,ticks,level.framenum,combat.restore_body);
    return true;
}

void G_SelectProtectedObserver(edict_t *ent, const char *userinfo)
{
    if (ent != g_edicts + 1)
        return;
    b2_observer = NULL; // A reconnect must authenticate the local slot again.
    if (combat.restore_pending || combat.restore_warming) {
        combat.restore_pending = combat.restore_warming = false;
        observer_walk_fault("connect_before_restore_warming_closed",ent);
        return;
    }
    if (!b2_observer_configured || !protected_observer_mode() ||
        ent->client != game.clients ||
        strcmp(Info_ValueForKey(userinfo, "ip"), "loopback"))
        return;
    b2_observer = ent;
    if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_OBSERVER selected=1 loopback=1 before_client_begin=1 frame=%d "
        "damage_knockback_feedback_protected=%d combat=%d targetability_collision_unchanged=1\n",
        level.framenum, !combat.enabled, combat.enabled);
}

// ClientBegin is reached from the live server's primed-client transition.
// ReadLevel clears connected even for an existing connection; map reconnects
// do not repeat ClientConnect. Restore that lifecycle bit only after the
// ordinary observer authentication and restore-warming boundary have passed.
void G_CombatClientBegin(edict_t *ent)
{
    combat.client_begin_angles = false;
    if (gi.cvar("b2_lifecycle", "0", 0)->value != 1 || !combat.enabled ||
        !ent || ent != b2_observer || ent != g_edicts + 1 ||
        ent->client != game.clients || !protected_observer_mode() ||
        obs_trace.walk_faulted || combat.restore_pending || combat.restore_warming ||
        ent->client->pers.spectator || ent->client->resp.spectator)
        return;
    ent->client->pers.connected = true;
    combat.client_begin_angles = ent->inuse;
}

// Own precisely the stock saved-body angle compensation, once per admitted
// ClientBegin. A publication may already exist while the client is connecting.
// Check prior writers before assigning; only delta-angle attribution advances.
void G_CombatClientBeginAngles(edict_t *ent)
{
    bool observer = gi.cvar("b2_lifecycle", "0", 0)->value == 1 && G_CombatObserver(ent);
    bool admitted = combat.client_begin_angles && observer &&
        !obs_trace.walk_faulted && !combat.restore_pending && !combat.restore_warming;
    combat.client_begin_angles = false;
    if (observer) G_ObserverTrace("client_begin_angles.before", ent);
    for (int i = 0; i < 3; i++)
        ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
    if (admitted && !obs_trace.walk_faulted) {
        memcpy(obs_trace.last.pm.delta_angles, ent->client->ps.pmove.delta_angles,
            sizeof(obs_trace.last.pm.delta_angles));
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_LIFECYCLE client_begin_angles frame=%d exact_stock_assignment=1 one_use=1\n", level.framenum);
    }
    if (observer) G_ObserverTrace("client_begin_angles.after", ent);
}

bool G_IsProtectedObserver(const edict_t *targ)
{
    return b2_observer_protect->value == 1 && targ && b2_observer_configured && targ == b2_observer &&
        protected_observer_mode() && targ->inuse && targ->client == game.clients &&
        targ->client->pers.connected && !targ->client->pers.spectator &&
        !targ->client->resp.spectator && targ->health > 0 && !targ->deadflag &&
        targ->movetype == MOVETYPE_WALK && targ->client->ps.pmove.pm_type == PM_NORMAL;
}

bool G_CombatObserver(const edict_t *ent)
{
    return combat.enabled && ent && ent == b2_observer && ent->inuse &&
        ent->client == game.clients && ent->client->pers.connected &&
        !ent->client->pers.spectator && !ent->client->resp.spectator && protected_observer_mode();
}

bool G_CombatTriggerPush452(edict_t *ent,edict_t *source)
{
    bool tracked=G_CombatObserver(ent);
    if (tracked) {
        bool owned=false;
        for (int i=1;i<globals.num_edicts;i++)
            if (source==g_edicts+i) { owned=true;break; }
        if (!owned || !source->inuse || source->solid!=SOLID_TRIGGER ||
            !source->classname || strcmp(source->classname,"trigger_push") ||
            source->touch!=trigger_push_touch || !isfinite(source->speed)) {
            observer_walk_fault("unsupported_trigger_push_writer",ent);return false;
        }
        G_ObserverTrace("trigger_push.velocity.before",source);
        if (obs_trace.walk_faulted) return false;
        vec3_t next;
        VectorScale(source->movedir,source->speed*10,next);
        for (unsigned i=0;i<3;i++)
            if (!isfinite(source->movedir[i]) || !isfinite(next[i]) || fabsf(next[i])>4095.f) {
                observer_walk_fault("trigger_push_velocity_not_representable",source);return false;
            }
    }
    // The literal stock assignment only. Do not acknowledge other callback state.
    VectorScale(source->movedir,source->speed*10,ent->velocity);
    if (tracked) VectorCopy(ent->velocity,obs_trace.last.velocity);
    return true;
}

void G_CombatImpulse(edict_t *ent, const vec3_t impulse, const char *reason)
{
    bool tracked = G_CombatObserver(ent);
    if (tracked) G_ObserverTrace("combat.impulse.before", ent);
    // Stock radius damage can exceed signed 12.3 player velocity (Big Gun).
    // Bound only the two stock impulse writers at this arithmetic boundary;
    // do not admit a corrupt incoming state, non-finite value or unknown writer.
    vec3_t previous;
    VectorCopy(ent->velocity, previous);
    VectorAdd(ent->velocity, impulse, ent->velocity);
    if (tracked && reason &&
        (!strcmp(reason, "T_Damage.impulse") || !strcmp(reason, "player_gib"))) {
        for (int i = 0; i < 3; i++)
            if (isfinite(previous[i]) && fabsf(previous[i]) <= 4095.f &&
                isfinite(impulse[i]) && isfinite(ent->velocity[i]))
                ent->velocity[i] = Q_clipf(ent->velocity[i], -4095.f, 4095.f);
    }
    if (tracked) {
        for (int i = 0; i < 3; i++)
            if (!isfinite(ent->velocity[i]) || fabsf(ent->velocity[i]) > 4095.f)
                observer_walk_fault("combat_impulse_not_representable", ent);
        // Only this literal velocity addition is admitted, not surrounding callbacks.
        VectorCopy(ent->velocity, obs_trace.last.velocity);
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_COMBAT impulse=%s frame=%d add=%.6f,%.6f,%.6f velocity=%.6f,%.6f,%.6f\n",
            reason, level.framenum, impulse[0], impulse[1], impulse[2], ent->velocity[0], ent->velocity[1], ent->velocity[2]);
    }
}

extern void target_earthquake_think(edict_t *self);

void G_CombatEarthquake(edict_t *ent, edict_t *source, float random_x, float random_y)
{
    bool tracked = G_CombatObserver(ent);
    if (tracked) {
        bool owned_source = false;
        for (int i = 1; i < globals.num_edicts; ++i)
            if (source == g_edicts + i) { owned_source = true; break; }
        if (!owned_source || !source->inuse || !source->classname ||
            strcmp(source->classname, "target_earthquake") ||
            source->think != target_earthquake_think || !ent->groundentity ||
            !isfinite(source->speed) || ent->mass <= 0 ||
            !isfinite(random_x) || !isfinite(random_y) ||
            fabsf(random_x) > 1.f || fabsf(random_y) > 1.f) {
            observer_walk_fault("unsupported_earthquake_writer", ent);
            return;
        }
        G_ObserverTrace("earthquake.velocity.before", source);
        vec3_t next = { ent->velocity[0] + random_x * 150,
            ent->velocity[1] + random_y * 150, source->speed * (100.0f / ent->mass) };
        for (unsigned i = 0; i < 3; ++i)
            if (!isfinite(next[i]) || fabsf(next[i]) > 4095.f) {
                observer_walk_fault("earthquake_velocity_not_representable", source);
                return;
            }
        if (obs_trace.walk_faulted) return;
    }
    ent->groundentity = NULL;
    ent->velocity[0] += random_x * 150;
    ent->velocity[1] += random_y * 150;
    ent->velocity[2] = source->speed * (100.0f / ent->mass);
    if (tracked) {
        // Only the stock velocity formula is admitted. Origin, inventory,
        // health and surrounding entity callbacks retain their usual guards.
        VectorCopy(ent->velocity, obs_trace.last.velocity);
        G_ObserverTrace("earthquake.velocity.after", source);
        if (obs_trace.world_events < 128) {
            obs_trace.world_events++;
            if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_WORLD_EARTHQUAKE frame=%d source=%d speed=%.9g mass=%d random=%.9g,%.9g velocity=%.9g,%.9g,%.9g\n",
                level.framenum, observer_number(source), source->speed, ent->mass,
                random_x, random_y, ent->velocity[0], ent->velocity[1], ent->velocity[2]);
        }
    }
}

void G_CombatPusherOrigin(edict_t *ent, const vec3_t origin, edict_t *pusher)
{
    bool tracked = G_CombatObserver(ent);
    if (tracked) {
        G_ObserverTrace("pusher.origin.before", pusher);
        const char *name = observer_class(pusher);
        bool stock = !strcmp(name, "func_door") || !strcmp(name, "func_door_secret") || !strcmp(name, "func_door_rotating") ||
            !strcmp(name, "func_plat") || !strcmp(name, "func_button") ||
            !strcmp(name, "func_train") || !strcmp(name, "func_rotating") || !strcmp(name, "func_water");
        if (!pusher || !pusher->inuse || pusher->solid != SOLID_BSP ||
            (pusher->movetype != MOVETYPE_PUSH && pusher->movetype != MOVETYPE_STOP) || !stock)
            observer_walk_fault("unsupported_pusher_writer", pusher);
        for (unsigned i = 0; i < 3; i++)
            if (!isfinite(origin[i]) || fabsf(origin[i]) > 4095.f)
                observer_walk_fault("pusher_origin_not_representable", pusher);
    }
    // Only the stock SV_Push calculated move/retry/saved rollback origin is
    // admitted here. Touch/blocked callbacks remain outside this writer.
    VectorCopy(origin, ent->s.origin);
    if (tracked) {
        VectorCopy(ent->s.origin, obs_trace.last.origin);
        if (obs_trace.world_events < 128) {
            obs_trace.world_events++;
            if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_WORLD_PUSH frame=%d source=%d class=%.80s origin=%.6f,%.6f,%.6f\n",
                level.framenum, observer_number(pusher), observer_class(pusher), origin[0], origin[1], origin[2]);
        }
    }
}

bool G_CombatTeleport(edict_t *ent, edict_t *trigger, edict_t *dest)
{
    bool tracked = G_CombatObserver(ent);
    if (tracked) {
        G_ObserverTrace("teleporter.writer.before", trigger);
        if (!trigger || !trigger->inuse || trigger->solid != SOLID_TRIGGER ||
            !trigger->owner || !trigger->owner->inuse ||
            strcmp(observer_class(trigger->owner), "misc_teleporter") ||
            !dest || !dest->inuse || strcmp(observer_class(dest), "misc_teleporter_dest") || ent->deadflag) {
            observer_walk_fault("unsupported_teleporter_writer", trigger);
            return false;
        }
        for (int i=0;i<3;i++)
            if (!isfinite(dest->s.origin[i]) || fabsf(dest->s.origin[i]+(i==2?10.f:0.f))>4095.f ||
                !isfinite(dest->s.angles[i])) observer_walk_fault("teleporter_not_representable", dest);
        if (obs_trace.walk_faulted) return false;
    }
    // These are the original teleporter_touch assignments, kept together.
    VectorCopy(dest->s.origin, ent->s.origin);
    VectorCopy(dest->s.origin, ent->s.old_origin);
    ent->s.origin[2] += 10;
    VectorClear(ent->velocity);
    ent->client->ps.pmove.pm_time = 160 >> 3;
    ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
    for (int i=0;i<3;i++)
        ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i]-ent->client->resp.cmd_angles[i]);
    VectorClear(ent->s.angles);
    VectorClear(ent->client->ps.viewangles);
    VectorClear(ent->client->v_angle);
    if (tracked) {
        VectorCopy(ent->s.origin, obs_trace.last.origin);
        VectorCopy(ent->velocity, obs_trace.last.velocity);
        obs_trace.last.pm.pm_flags = ent->client->ps.pmove.pm_flags;
        obs_trace.last.pm.pm_time = ent->client->ps.pmove.pm_time;
        memcpy(obs_trace.last.pm.delta_angles, ent->client->ps.pmove.delta_angles, sizeof(obs_trace.last.pm.delta_angles));
        combat.epoch++; combat.relocations++; combat.input_hold=true;
        gi.dprintf("B2_TELEPORT frame=%d epoch=%u relocations=%u origin=%g,%g,%g stock_writer=1 release_required=1\n",
              level.framenum,combat.epoch,combat.relocations,ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
    }
    return true;
}

void G_CombatHealth(edict_t *ent, int health)
{
    bool tracked = G_CombatObserver(ent);
    if (tracked) G_ObserverTrace("combat.health.before", ent);
    int old = ent->health;
    ent->health = health;
    if (tracked) {
        obs_trace.last.health = health;
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_COMBAT health frame=%d before=%d after=%d\n", level.framenum, old, health);
    }
}

void G_CombatInventory(edict_t *ent, int index, int count, const char *reason)
{
    bool tracked = G_CombatObserver(ent);
    if (tracked) G_ObserverTrace("inventory.before", ent);
    if (index <= 0 || index >= MAX_ITEMS || count < 0) {
        if (tracked) observer_walk_fault("invalid_inventory_write", ent);
        return;
    }
    int old = ent->client->pers.inventory[index];
    ent->client->pers.inventory[index] = count;
    if (tracked) {
        // Admit this stock writer's one slot, never a surrounding callback.
        obs_trace.last.inventory[index] = count;
        if (old != count) {
            obs_trace.usage[index].writes++;
            if (count > old) obs_trace.usage[index].added += count - old;
            else obs_trace.usage[index].spent += old - count;
            if (count < old && !strcmp(reason, "stock.p_weapon") && index == ent->client->ammo_index &&
                combat_stock_weapon(ent->client->pers.weapon))
                obs_trace.usage[ITEM_INDEX(ent->client->pers.weapon)].fire_ammo += old - count;
        }
        if (old != count && obs_trace.item_events < 128) {
            obs_trace.item_events++;
            if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_ITEM writer=%s frame=%d index=%d before=%d after=%d\n", reason, level.framenum, index, old, count);
            if (obs_trace.item_events == 128)
                if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_ITEM_LIMIT writer_events=128 further_writer_logs_omitted=1 state_guard_continues=1\n");
        }
    }
}

#include "b2_feedback304.h"
static uint64_t b2_pickup_sequence304;
void G_B2Pickup304(edict_t *item,edict_t *player,bool taken,bool world_touch)
{
 if(taken && world_touch && item && item->item && G_CombatObserver(player) &&
    player==g_edicts+1 && player->health>0 && !player->deadflag && combat.anchored &&
    !combat.input_hold && !obs_trace.walk_faulted && !level.intermission_framenum &&
    b2_pickup_sequence304!=UINT64_MAX)b2_pickup_sequence304++;
}
void G_CombatPickupResult(edict_t *item, edict_t *player, bool taken)
{
    if (!G_CombatObserver(player)) return;
    G_ObserverTrace("pickup.after_stock_writer", item);
    unsigned *events = taken ? &obs_trace.pickup_events : &obs_trace.rejected_pickups;
    unsigned limit = taken ? 128 : 8;
    if (*events < limit) {
        ++*events;
        if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_PICKUP frame=%d class=%.80s taken=%d health=%d armor_index=%d item_index=%d count=%d bonus_alpha=%.6f\n",
            level.framenum, observer_class(item), taken, player->health, ArmorIndex(player),
            ITEM_INDEX(item->item), player->client->pers.inventory[ITEM_INDEX(item->item)], player->client->bonus_alpha);
        if (*events == limit)
            if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_PICKUP_LIMIT taken=%d events=%u further_same_result_logs_omitted=1 state_guard_continues=1\n", taken, limit);
    }
}

void G_CombatGibOrigin(edict_t *ent)
{
    bool tracked = G_CombatObserver(ent);
    if (tracked) {
        G_ObserverTrace("combat.gib.before", ent);
        if (!combat.death_scope) observer_walk_fault("gib_outside_death", ent);
    }
    ent->s.origin[2] += 32;
    if (tracked) obs_trace.last.origin[2] = ent->s.origin[2];
}

void G_CombatDeath(edict_t *ent, bool before)
{
    if (!G_CombatObserver(ent)) return;
    if (before) {
        G_ObserverTrace("combat.death.before", ent);
        if (combat.death_scope || ent->health > 0) observer_walk_fault("invalid_death_entry", ent);
        combat.death_scope = true;
        combat.death_was_dead = ent->deadflag != 0;
        combat.death_was_gib = ent->client->ps.pmove.pm_type == PM_GIB;
        if (!combat.death_was_dead)
            combat.released = combat.requested = false;
    } else {
        G_ObserverTrace("combat.death.after", ent); // origin/velocity still checked inside the scope
        bool gib = ent->health < -40;
        if (!combat.death_scope || ent->deadflag != DEAD_DEAD || ent->health > 0 ||
            ent->movetype != (gib ? MOVETYPE_BOUNCE : MOVETYPE_TOSS) ||
            ent->solid != (gib ? SOLID_NOT : SOLID_BBOX) ||
            ent->client->ps.pmove.pm_type != PM_DEAD)
            observer_walk_fault("invalid_death_result", ent);
        combat.death_scope = false;
        if (gib) {
            // Publish the actual gib movement model before the next command;
            // prediction must not integrate one PM_DEAD command against PM_GIB.
            ent->client->ps.pmove.pm_type = PM_GIB;
            obs_trace.last.pm.pm_type = PM_GIB;
        }
        // Ordinary corpse damage re-enters player_die. It is not a fresh
        // death/release boundary. A first gib conversion still changes the
        // movement model and therefore invalidates the lifecycle history.
        if (!combat.death_was_dead || (gib && !combat.death_was_gib)) {
            combat.epoch++;
            if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_COMBAT death frame=%d gib=%d epoch=%u release_required=%d\n",
                level.framenum, gib, combat.epoch, !combat.death_was_dead);
        }
    }
}

// Called once from ClientBegin after normal spawn and before its first
// publication. R/PutClientInServer do not call this hook.
bool G_CombatStartupEquipment(edict_t *ent)
{
    if (gi.cvar("b2_startup_give_all", "0", CVAR_LATCH)->value != 1) return false;
    if (!G_CombatObserver(ent) || combat.startup_equipped || combat.anchored ||
        combat.serial || ent->deadflag || level.intermission_framenum ||
        strcmp(level.mapname, combat.mapname) ||
        (strcmp(combat.mapname, "base1") && strcmp(combat.mapname, "command") && strcmp(combat.mapname, "mintro") && strcmp(combat.mapname, "fact2"))) return false;
    combat.startup_equipped = true;
    gi.dprintf("B2_EQUIPMENT startup stock_give_all=1 map=%s once=1 before_initial_publication=1\n", combat.mapname);
    return true;
}

bool G_CombatSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles)
{
    if (!G_CombatObserver(ent) || !combat.returning) return false;
    VectorCopy(combat.origin, origin); VectorCopy(combat.angles, angles);
    return true;
}

void G_CombatFilterCommand(edict_t *ent, usercmd_t *cmd)
{
    if (!G_CombatObserver(ent) || !combat.input_hold) return;
    // Commands already in flight at the qualified return may still contain R
    // or held gameplay input. Drain known controls until a neutral command.
    // Unknown buttons/impulses remain visible to the normal command guard.
    if (cmd->impulse || (cmd->buttons & ~(BUTTON_ATTACK | BUTTON_USE))) return;
    if (!cmd->buttons && !cmd->forwardmove && !cmd->sidemove && !cmd->upmove)
        combat.input_hold = false;
    cmd->buttons = 0;
    cmd->forwardmove = cmd->sidemove = cmd->upmove = 0;
}

bool G_CombatRespawn(edict_t *ent)
{
    if (!combat.enabled || ent != b2_observer) return false;
    if (gi.cvar("b2_lifecycle","0",0)->value==1 && !combat.requested &&
        (ent->client->latched_buttons & BUTTON_ATTACK)) return false;
    if (!G_CombatObserver(ent) || obs_trace.walk_faulted || !ent->deadflag || !combat.requested) return true;
    combat.requested = combat.released = false;
    if (!combat.anchored || level.intermission_framenum || strcmp(level.mapname, combat.mapname) ||
        level.framenum <= ent->client->respawn_framenum) {
        observer_walk_fault("invalid_respawn_request", ent); return true;
    }
    vec3_t mins = {-16,-16,-24}, maxs = {16,16,32};
    trace_t clear = gi.trace(combat.origin, mins, maxs, combat.origin, ent, MASK_PLAYERSOLID);
    if (clear.startsolid || clear.allsolid || clear.fraction != 1) {
        gi.dprintf("B2_COMBAT respawn_blocked frame=%d release_and_retry=1\n", level.framenum);
        return true; // no KillBox, telefrag or alternative spawn
    }
    G_ObserverTrace("combat.respawn.before", ent);
    int gravity = ent->client->ps.pmove.gravity;
    combat.returning = true;
    PutClientInServer(ent);
    combat.returning = false;
    ent->client->ps.pmove.gravity = gravity;
    ent->client->old_pmove = ent->client->ps.pmove;
    if (memcmp(ent->s.origin, combat.origin, sizeof(vec3_t)) || !VectorEmpty(ent->velocity) ||
        ent->health != 100 || ent->deadflag || ent->movetype != MOVETYPE_WALK ||
        ent->client->ps.pmove.pm_type != PM_NORMAL || !ent->client->pers.weapon ||
        strcmp(ent->client->pers.weapon->classname, "weapon_blaster"))
        observer_walk_fault("invalid_respawn_result", ent);
    observer_read(&obs_trace.last); // precisely the checked PutClientInServer return
    combat.epoch++; combat.returns++;
    combat.input_hold = true;
    if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_COMBAT respawn frame=%d epoch=%u returns=%u origin=%.6f,%.6f,%.6f release_required=1\n",
        level.framenum, combat.epoch, combat.returns, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
    return true;
}

void G_CombatPublish(edict_t *ent)
{
    if (!G_CombatObserver(ent)) return;
    if (combat.level_exiting) return;
    if (combat.restore_pending || combat.restore_warming) {
        combat.restore_pending = combat.restore_warming = false;
        observer_walk_fault("publication_during_restore_warming",ent); return;
    }
    G_ObserverTrace("combat.publication", ent);
    if(strcmp(level.mapname,combat.mapname)) observer_walk_fault("unannounced_map_change",ent);
    if (gi.cvar("b2_lifecycle","0",0)->value!=1 && (level.intermission_framenum || strcmp(level.mapname, combat.mapname) ||
        (strcmp(combat.mapname, "base1") && strcmp(combat.mapname, "command") && strcmp(combat.mapname, "mintro") && strcmp(combat.mapname, "fact2")))) observer_walk_fault("combat_map_boundary", ent);
    if (obs_trace.walk_faulted) return;
    if (!combat.anchored) {
        if (ent->deadflag) return;
        if (combat.restored_anchor_pending) {
            bool exact = combat.restored_anchor_valid && ent == g_edicts + 1 &&
                ent->health == combat.restored_health && ent->client->ps.pmove.pm_type == PM_NORMAL &&
                !memcmp(ent->s.origin, combat.restored_origin, sizeof(vec3_t)) &&
                !memcmp(ent->velocity, combat.restored_velocity, sizeof(vec3_t));
            for (unsigned i = 0; i < 3; i++)
                exact = exact && isfinite(ent->s.origin[i]) && isfinite(ent->velocity[i]) &&
                    ent->s.origin[i] >= -4096.f && ent->s.origin[i] <= 4095.875f &&
                    ent->velocity[i] >= -4096.f && ent->velocity[i] <= 4095.875f &&
                    ent->client->ps.pmove.origin[i] == (short)(ent->s.origin[i] * 8.f) &&
                    ent->client->ps.pmove.velocity[i] == (short)(ent->velocity[i] * 8.f);
            if (!exact) { observer_walk_fault("restored_initial_publication_mismatch", ent); return; }
            combat.restored_anchor_pending = false;
            if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_LIFECYCLE restored_anchor frame=%d saved_velocity=%.6f,%.6f,%.6f authenticated=1 exact_admitted_restore_state=1\n",
                level.framenum, ent->velocity[0], ent->velocity[1], ent->velocity[2]);
        } else if (!VectorEmpty(ent->velocity)) return;
        VectorCopy(ent->s.origin, combat.origin);
        VectorCopy(ent->client->ps.viewangles, combat.angles);
        combat.angles[PITCH] = combat.angles[ROLL] = 0;
        combat.anchored = true;
    }
    if (combat.serial == UINT_MAX) { observer_walk_fault("combat_serial_exhausted", ent); return; }
    ++combat.serial;
    // Encode without implementation-defined unsigned-to-signed narrowing.
    unsigned low = combat.serial & 0xffffu, high = combat.serial >> 16;
    ent->client->ps.stats[B2_COMBAT_STAT] = low <= 32767u ? (int)low : (int)low - 65536;
    ent->client->ps.stats[B2_COMBAT_SERIAL_HIGH_STAT] = high <= 32767u ? (int)high : (int)high - 65536;
    unsigned slot = combat.serial % q_countof(combat.publications);
    combat.publications[slot].ps = ent->client->ps;
    combat.publications[slot].state = (b2_combat_state_t){combat.serial, combat.epoch, combat.returns, ent->deadflag != 0, combat.relocations};
    combat.publications[slot].frame = level.framenum;
}

q_exported bool GetB2CombatState(unsigned version, const player_state_t *ps, b2_combat_state_t *out)
{
    if (version != B2_COMBAT_API || !ps || !out || !G_CombatObserver(b2_observer) ||
        obs_trace.walk_faulted || !combat.anchored) return false;
    unsigned id = (unsigned)(unsigned short)ps->stats[B2_COMBAT_STAT] |
        ((unsigned)(unsigned short)ps->stats[B2_COMBAT_SERIAL_HIGH_STAT] << 16);
    if (!id || id > combat.serial) return false;
    unsigned slot = id % q_countof(combat.publications);
    const player_state_t *saved = &combat.publications[slot].ps;
    if (combat.publications[slot].state.serial != id || level.framenum - combat.publications[slot].frame > 10 ||
        ps->pmove.pm_type != saved->pmove.pm_type || ps->pmove.pm_flags != saved->pmove.pm_flags ||
        ps->pmove.pm_time != saved->pmove.pm_time || ps->pmove.gravity != saved->pmove.gravity ||
        memcmp(ps->pmove.origin, saved->pmove.origin, sizeof(ps->pmove.origin)) ||
        memcmp(ps->pmove.velocity, saved->pmove.velocity, sizeof(ps->pmove.velocity)) ||
        memcmp(ps->pmove.delta_angles, saved->pmove.delta_angles, sizeof(ps->pmove.delta_angles)) ||
        ps->stats[STAT_HEALTH] != saved->stats[STAT_HEALTH] || ps->gunindex != saved->gunindex)
        return false;
    *out = combat.publications[slot].state;
    return true;
}

void T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker, const vec3_t dir, vec3_t point, const vec3_t normal, int damage, int knockback, int dflags, int mod)
{
    gclient_t   *client;
    int         take;
    int         save;
    int         asave;
    int         psave;
    int         te_sparks;

    if (!targ->takedamage) {
        observer_damage(targ, inflictor, attacker, damage, knockback, dflags, mod, "no_takedamage");
        return;
    }

    // Protect the selected observer before momentum, health, pain/death and
    // view-feedback effects. Keep takedamage/flags/collision/AI untouched.
    if (G_IsProtectedObserver(targ)) {
        observer_damage(targ, inflictor, attacker, damage, knockback, dflags, mod, "suppressed");
        if (!b2_observer_hit_logged) {
            if (gi.cvar("developer","0",0)->value >= 1) gi.dprintf("B2_OBSERVER first_suppressed_hit frame=%d time=%.3f observer=1 "
                "attacker=%d attacker_class=%s inflictor=%d mod=%d damage=%d knockback=%d flags=%d\n",
                level.framenum, level.time, attacker ? attacker->s.number : -1,
                attacker && attacker->classname ? attacker->classname : "none",
                inflictor ? inflictor->s.number : -1, mod, damage, knockback, dflags);
            b2_observer_hit_logged = true;
        }
        return;
    }

    observer_damage(targ, inflictor, attacker, damage, knockback, dflags, mod, "ordinary_damage_path");

    // easy mode takes half damage
    if (skill->value == 0 && deathmatch->value == 0 && targ->client) {
        damage *= 0.5f;
        if (!damage)
            damage = 1;
    }

    // friendly fire avoidance
    // if enabled you can't hurt teammates (but you can hurt yourself)
    // knockback still occurs
    if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || (coop->value && targ->client))) {
        if (OnSameTeam(targ, attacker)) {
            if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
                damage = 0;
            else
                mod |= MOD_FRIENDLY_FIRE;
        }
    }
    meansOfDeath = mod;

    client = targ->client;

    if (dflags & DAMAGE_BULLET)
        te_sparks = TE_BULLET_SPARKS;
    else
        te_sparks = TE_SPARKS;

// bonus damage for suprising a monster
    if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) && (attacker->client) && (!targ->enemy) && (targ->health > 0))
        damage *= 2;

    if (targ->flags & FL_NO_KNOCKBACK)
        knockback = 0;

// figure momentum add
    if (!(dflags & DAMAGE_NO_KNOCKBACK)) {
        if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP)) {
            vec3_t  kvel;
            float   mass;

            if (targ->mass < 50)
                mass = 50;
            else
                mass = targ->mass;

            VectorNormalize2(dir, kvel);

            if (targ->client && attacker == targ)
                VectorScale(kvel, 1600.0f * (float)knockback / mass, kvel);  // the rocket jump hack...
            else
                VectorScale(kvel, 500.0f * (float)knockback / mass, kvel);

            G_CombatImpulse(targ, kvel, "T_Damage.impulse");
        }
    }

    take = damage;
    save = 0;

    // check for godmode
    if ((targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION)) {
        take = 0;
        save = damage;
        SpawnDamage(te_sparks, point, normal, save);
    }

    // check for invincibility
    if ((client && client->invincible_framenum > level.framenum) && !(dflags & DAMAGE_NO_PROTECTION)) {
        if (targ->pain_debounce_framenum < level.framenum) {
            gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
            targ->pain_debounce_framenum = level.framenum + 2 * BASE_FRAMERATE;
        }
        take = 0;
        save = damage;
    }

    psave = CheckPowerArmor(targ, point, normal, take, dflags);
    take -= psave;

    asave = CheckArmor(targ, point, normal, take, te_sparks, dflags);
    take -= asave;

    //treat cheat/powerup savings the same as armor
    asave += save;

    // team damage avoidance
    if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(targ, attacker))
        return;

// do the damage
    if (take) {
        if ((targ->svflags & SVF_MONSTER) || (client))
        {
            // SpawnDamage(TE_BLOOD, point, normal, take);
            SpawnDamage(TE_BLOOD, point, dir, take);
        }
        else
            SpawnDamage(te_sparks, point, normal, take);

        G_CombatHealth(targ, targ->health - take);

        if (targ->health <= 0) {
            if ((targ->svflags & SVF_MONSTER) || (client))
                targ->flags |= FL_NO_KNOCKBACK;
            Killed(targ, inflictor, attacker, take, point);
            return;
        }
    }

    if (targ->svflags & SVF_MONSTER) {
        M_ReactToDamage(targ, attacker);
        if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take)) {
            targ->pain(targ, attacker, knockback, take);
            // nightmare mode monsters don't go into pain frames often
            if (skill->value == 3)
                targ->pain_debounce_framenum = level.framenum + 5 * BASE_FRAMERATE;
        }
    } else if (client) {
        if (!(targ->flags & FL_GODMODE) && (take))
            targ->pain(targ, attacker, knockback, take);
    } else if (take) {
        if (targ->pain)
            targ->pain(targ, attacker, knockback, take);
    }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    if (client) {
        client->damage_parmor += psave;
        client->damage_armor += asave;
        client->damage_blood += take;
        client->damage_knockback += knockback;
        VectorCopy(point, client->damage_from);
    }
}

/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage(edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
    float   points;
    edict_t *ent = NULL;
    vec3_t  v;
    vec3_t  dir;

    while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL) {
        if (ent == ignore)
            continue;
        if (!ent->takedamage)
            continue;

        VectorAdd(ent->mins, ent->maxs, v);
        VectorMA(ent->s.origin, 0.5f, v, v);
        VectorSubtract(inflictor->s.origin, v, v);
        points = damage - 0.5f * VectorLength(v);
        if (ent == attacker)
            points = points * 0.5f;
        if (points > 0) {
            if (CanDamage(ent, inflictor)) {
                VectorSubtract(ent->s.origin, inflictor->s.origin, dir);
                T_Damage(ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
            }
        }
    }
}


#include "b2_weapon_game264.inc"
#include "../../../square410/source/use410.inc"

#include "../../../../baseline/b2-consolidated215/inputs/origin-044/include/refresh/b2_weapon_fire.inc"

#include "../../../../baseline/b2-consolidated215/inputs/origin-002/include/refresh/b2_roomscale_game.inc"

q_exported bool GetB2Pickup304(unsigned size,b2_pickup304_t *out)
{
 if(!out || size!=sizeof(*out))return false;
 memset(out,0,sizeof(*out));if(!b2w_live_player() || level.intermission_framenum)return false;
 out->epoch=combat.epoch+1;out->sequence=b2_pickup_sequence304;return true;
}
