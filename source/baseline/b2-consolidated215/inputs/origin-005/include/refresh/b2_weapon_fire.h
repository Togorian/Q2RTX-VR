// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Private game translation-unit contract; include after canonical g_local.h.
#ifndef B2_WEAPON_FIRE_H
#define B2_WEAPON_FIRE_H
void B2W_WeaponCommand(edict_t *ent);
void B2W_RoomscaleMove(edict_t *ent,pmove_t *pm);
void B2W_TeleportMove(edict_t *ent,pmove_t *pm);
bool B2W_WeaponBegin(edict_t *ent);
void B2W_WeaponEnd(edict_t *ent);
bool B2W_WeaponStart(edict_t *ent);
bool B2W_WeaponTracked(edict_t *ent);
bool B2W_WeaponAligned(edict_t *ent);
void B2W_WeaponView(edict_t *ent);
void B2W_WeaponRecoil(edict_t *ent);
bool B2W_WeaponMuzzle(edict_t *ent, float start[3], float forward[3]);
void B2W_WeaponGrenadeEmission(edict_t *ent, bool detonated);
void B2W_WeaponEmitted(edict_t *ent);
void B2W_WeaponEmissionEnd(edict_t *ent);
void B2W_WeaponBasis(edict_t *ent, float forward[3], float right[3], float up[3]);
void B2W_WeaponWaterBasis(edict_t *ent, const float direction[3], float forward[3], float right[3], float up[3]);
void B2W_WeaponBeforeChange(edict_t *ent);
void B2W_WeaponChanged(edict_t *ent);
#endif
