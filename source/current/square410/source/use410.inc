// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Adapted selected g_combat.c; original Id Software GPL notice remains in owner.
// Only an ordinary known local tracked command may retire its Use bit.
// Dead/return/intermission, disabled teleport, foreign owners, unknown buttons
// and impulses retain their original disposition. Movement is never edited.
static bool b2tp_retire_use410(edict_t *ent,const usercmd_t *cmd)
{
    if(!obs_trace.walk_guard || cmd->impulse || (cmd->buttons&~(BUTTON_ATTACK|BUTTON_USE)) ||
       !ent || ent!=b2w_live_player() || level.intermission_framenum ||
       !b2w_host.context || !b2w_host.teleport_enabled || !b2w_host.teleport_enabled())
        return false;
    unsigned context=b2w_host.context();
    return (context&(B2W_CONTEXT_REQUESTED|B2W_CONTEXT_COMMAND))==
        (B2W_CONTEXT_REQUESTED|B2W_CONTEXT_COMMAND);
}
