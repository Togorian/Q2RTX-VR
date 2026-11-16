// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#define B2_MAX_VIEWS 4u
#define B2_MAX_RENDER_DIMENSION 4096u
typedef struct b2_view_extent_s { uint32_t width, height, max_width, max_height, samples; } b2_view_extent_t;
static inline uint32_t B2_PhysicalEye(uint32_t view) { return view & 1u; }
static inline bool B2_ValidViewCount(uint32_t count) { return count == 2u || count == 4u; }
static inline uint32_t B2_ViewMask(uint32_t count) { return B2_ValidViewCount(count) ? (1u << count) - 1u : 0u; }
// Keep shader/scratch extents uniform, but honour every runtime output extent.
// This oversamples smaller views; no inset holes or performance claim.
static inline bool B2_SelectViewExtents(bool quad, uint32_t count,
    const b2_view_extent_t *views, uint32_t system_width, uint32_t system_height,
    uint32_t *render_width, uint32_t *render_height)
{
    if (!views || !render_width || !render_height || count != (quad ? 4u : 2u)) return false;
    uint32_t width = 0, height = 0;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t w = quad ? views[i].width : 1948u, h = quad ? views[i].height : 1563u;
        if (!w || !h || w > views[i].max_width || h > views[i].max_height ||
            w > system_width || h > system_height || w > B2_MAX_RENDER_DIMENSION ||
            h > B2_MAX_RENDER_DIMENSION || !views[i].samples) return false;
        if (w > width) width = w;
        if (h > height) height = h;
    }
    *render_width = width; *render_height = height; return true;
}
