// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Include only after the owning unit's canonical usercmd_t declaration.
#ifndef B2_WEAPON_ENGINE_H
#define B2_WEAPON_ENGINE_H
#include "b2_weapon_bridge.h"
static inline b2w_command_t B2W_Command(const usercmd_t *cmd, unsigned light)
{
    b2w_command_t key={0};
    for (unsigned i=0;i<3;i++) key.angles[i]=cmd->angles[i];
    key.forward=cmd->forwardmove; key.side=cmd->sidemove; key.up=cmd->upmove;
    key.buttons=cmd->buttons; key.msec=cmd->msec; key.impulse=cmd->impulse; key.light=(uint8_t)light;
    return key;
}
#endif
