// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Adapted selected g_combat.c; original Id Software GPL notice remains in owner.
static bool b2tp_retire_use410(edict_t *ent,const usercmd_t *cmd);
static void observer_command410(edict_t *ent, const usercmd_t *cmd, bool use_allowed)
{
    if (!G_ObserverTracing(ent)) return;
    obs_trace.command_active = cmd != NULL;
    if (!cmd) return;
    obs_trace.command = *cmd; obs_trace.commands++;
    obs_trace.command_frame = level.framenum; obs_trace.command_time = level.time;
    if (obs_trace.walk_guard) {
        if (!obs_trace.heading_valid) {
            memcpy(obs_trace.heading, cmd->angles, sizeof(obs_trace.heading));
            obs_trace.heading_valid = true;
        }
        float forward = cmd->forwardmove, side = cmd->sidemove;
        // Quantized yaw is cyclic. The client owns intentional yaw updates;
        // the game independently constrains body pitch/roll and delta state.
        float pitch = SHORT2ANGLE((short)(cmd->angles[PITCH] + ent->client->ps.pmove.delta_angles[PITCH]));
        bool bad_angles = obs_trace.turning ?
            (fabsf(pitch) > 89.01f || cmd->angles[ROLL] != obs_trace.heading[ROLL]) :
            memcmp(obs_trace.heading, cmd->angles, sizeof(obs_trace.heading)) != 0;
        if ((obs_trace.vertical ? (cmd->upmove != 0 && cmd->upmove != 200 && cmd->upmove != -200) : cmd->upmove != 0) ||
            gi.cvar("b2_jump_crouch", "0", 0)->value != (int)obs_trace.vertical || (cmd->buttons & ~((obs_trace.firing ? BUTTON_ATTACK : 0) | ((combat.enabled && ent->deadflag) || use_allowed ? BUTTON_USE : 0))) || cmd->impulse || forward*forward + side*side > 160001.f ||
            bad_angles)
            observer_walk_fault("unsupported_applied_command", ent);
    }
    observer_sample("ClientThink.enter", ent, cmd->forwardmove || cmd->sidemove || cmd->upmove || cmd->buttons || cmd->impulse);
    if (combat.enabled && ent->deadflag) {
        bool neutral = !cmd->forwardmove && !cmd->sidemove && !cmd->upmove && !cmd->impulse;
        if (neutral && !cmd->buttons) combat.released = true;
        if (cmd->buttons || !neutral) {
            // Several ClientThink calls can precede one server frame. Keep the
            // eligible edge until G_CombatRespawn consumes it; held commands
            // must not erase a request already accepted in this frame.
            if (neutral && cmd->buttons == BUTTON_USE && combat.released &&
                level.framenum > ent->client->respawn_framenum)
                combat.requested = true;
            combat.released = false;
        }
    }
}

// Preserve the existing read-only observation/reset contract.
void G_ObserverCommand(edict_t *ent,const usercmd_t *cmd)
{
    bool allowed=G_ObserverTracing(ent) && cmd && b2tp_use_allowed(ent);
    observer_command410(ent,cmd,allowed);
}

// ClientThink owns a mutable local command copy. Classify Use once here,
// immediately before the unchanged movement/unknown-command guard consumes it.
// No pose is renewed, and later teleport/fire commits retain their own checks.
void G_ObserverApplyCommand410(edict_t *ent,usercmd_t *cmd)
{
    bool allowed=G_ObserverTracing(ent) && cmd && b2tp_use_allowed(ent);
    if(!allowed && G_ObserverTracing(ent) && cmd &&
       (cmd->buttons&BUTTON_USE) && b2tp_retire_use410(ent,cmd))
        cmd->buttons&=~BUTTON_USE;
    observer_command410(ent,cmd,allowed);
}
