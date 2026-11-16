// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SHARED_B2_COMBAT_H
#define SHARED_B2_COMBAT_H
// Private, same-process B2 contract. No standard game API or wire ABI change.
// Slot 31 is unused by this game's HUD. It names an immutable publication,
// never an authority bit: the loaded game must independently validate it.
#define B2_COMBAT_STAT 31
#define B2_COMBAT_API 2
typedef struct { unsigned serial, epoch, returns; int dead; unsigned relocations; } b2_combat_state_t;
typedef bool (*b2_combat_query_t)(unsigned, const player_state_t *, b2_combat_state_t *);
bool SV_B2CombatState(const player_state_t *, b2_combat_state_t *);
#endif
