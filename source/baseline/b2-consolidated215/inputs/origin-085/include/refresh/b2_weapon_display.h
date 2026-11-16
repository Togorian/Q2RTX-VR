// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Engine-only display experiment. No game/command/controller ABI changes.
#ifndef B2_WEAPON_DISPLAY_H
#define B2_WEAPON_DISPLAY_H
#include "b2_weapon_presentation.h"
struct b2_frame_s;
// Pure policy: completed-shot and non-idle ownership stay command-bound.
static inline bool B2WV_Held(const b2wp_record_t *r)
{
    return r && r->idle_pose && !r->emission_pose && !r->emission_serial &&
        !r->packet.attack && !(r->command.buttons & 1u) && !r->packet.selection &&
        r->packet.weapon != 5;
}
static inline bool B2WV_Continuity(bool mode, bool used, bool old_mode, bool old_used)
{
    return mode == old_mode && used == old_used;
}
// Reset only the visual overlay; original staged record/base/rebase stay intact.
void B2WP_DisplayBegin(bool requested);
bool B2WP_DisplayApply(const b2wp_record_t *owner, const b2w_pose_t *aim,
    const float preview[3], int64_t eye_time, uint32_t now);
// One cached display-only action-space locate in the open current XR frame.
// No action sync, input serial increment or controller publication.
bool B2XR_ReadWeaponDisplay(const struct b2_frame_s *frame, unsigned hand,
    b2c_snapshot_t *out);
#endif
