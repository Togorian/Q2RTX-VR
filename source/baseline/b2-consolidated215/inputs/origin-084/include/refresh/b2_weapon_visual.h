// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Private pre-pair visual copy. No game, command, entity or save ABI changes.
#ifndef B2_WEAPON_VISUAL_H
#define B2_WEAPON_VISUAL_H
#include "b2_weapon_presentation.h"
typedef struct {
    b2wp_record_t owner;
    uint32_t handle;
    const void *model;
    int frame,oldframe;
    float backlerp,matrix[16];
    int64_t eye_time;
    bool display_used;
} b2wv_visual_t;
bool B2WP_CopyVisual(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, int64_t eye_time, uint32_t now, b2wv_visual_t *out);
#endif
