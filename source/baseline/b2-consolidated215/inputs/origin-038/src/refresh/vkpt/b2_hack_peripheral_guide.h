// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Design/direction by Togorian; implementation with Codex assistance.
// Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
// Modified 11 November 2026: provenance/notice clarification only. See PROVENANCE.md.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* Private HF experiment; GPL-2.0-or-later, like the surrounding Q2RTX code.
 * This header follows vkpt.h and does not replace its accepted declarations. */
#ifndef B2_HACK_PERIPHERAL_GUIDE_H
#define B2_HACK_PERIPHERAL_GUIDE_H
/* Pass explicit scalars/arrays: engine-103 and engine-112 keep different frozen UBO headers. */
void b2_hack_guide_snapshot(const float *projection, const float *view_matrix,
    const float *origin, const float *jitter, uint32_t width, uint32_t height,
    bool quad_mode_valid, bool sampling_valid, uint32_t view,
    uint32_t logical_index, uint32_t history_generation);
void b2_hack_guide_submitted(uint32_t view, uint32_t logical_index,
    uint32_t history_generation);
#endif
