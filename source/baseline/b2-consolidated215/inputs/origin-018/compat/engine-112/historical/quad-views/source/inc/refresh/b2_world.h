// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#ifdef REFERENCE_B_B2
unsigned B2_WorldMaintenanceEpoch(void);
bool B2_WorldBound(unsigned epoch);
bool R_B2WorldReady(unsigned epoch);
bool R_B2WorldMaintenance(void);
void R_B2ResourceResult(int result, const char *operation);
void vkpt_b2_world_textures_finish(void);
#define B2_WORLD_VK(call) do { int b2_result_ = (call); \
    R_B2ResourceResult(b2_result_, #call); _VK(b2_result_); } while (0)
#else
#define B2_WORLD_VK(call) _VK(call)
#endif
