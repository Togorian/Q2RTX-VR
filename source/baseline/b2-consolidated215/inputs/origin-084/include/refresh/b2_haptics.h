// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_HAPTICS_H
#define B2_HAPTICS_H
#include <stdbool.h>
#include <stdint.h>
// Private client/OpenXR seam, never part of the public game/save ABI.
void B2H_Reset(void);
void B2H_ServerData(bool attract);
void B2H_Snapshot(void);
void B2H_Muzzle(int entity, int weapon);
void B2H_Amplitudes(uint32_t now, float out[2]);
void B2XR_HapticStop(void);
#endif
