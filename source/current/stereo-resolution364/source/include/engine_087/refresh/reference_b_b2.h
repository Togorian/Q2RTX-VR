// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "reference_b_quad.h"

#define B2_EYE_COUNT 2u
#define B2_WIDTH 1948u
#define B2_HEIGHT 1563u

typedef struct b2_view_s {
    float origin[3], angles[3], fov_xy[2];
    float V[16], invV[16], P[16], invP[16];
} b2_view_t;

typedef struct b2_frame_s {
    b2_view_t views[B2_MAX_VIEWS], previous[B2_MAX_VIEWS];
    float positions[B2_MAX_VIEWS][3];
    float orientations[B2_MAX_VIEWS][4]; // x, y, z, w
    float fov_lrud[B2_MAX_VIEWS][4];     // left, right, up, down radians
    uint32_t view_count, output_width[B2_MAX_VIEWS], output_height[B2_MAX_VIEWS];
    uint32_t logical_index, history_generation;
    bool history_valid, should_render, tracking_valid;
    bool runtime_foveated; // Explicit dynamic focus projection; uses existing alignment padding.
    int64_t predicted_time_ns, duration_ns;
} b2_frame_t;

// Pure CPU camera and ordered, atomic two/four-view history transaction.
typedef struct b2_history_s {
    b2_view_t committed[B2_MAX_VIEWS], pending[B2_MAX_VIEWS];
    uint32_t view_count, output_width[B2_MAX_VIEWS], output_height[B2_MAX_VIEWS];
    uint32_t logical_index, generation, accepted_mask;
    bool valid, frame_open, failed;
} b2_history_t;

bool B2_BuildView(const float position[3], const float orientation_xyzw[4],
    const float fov_lrud[4], const float anchor_origin[3],
    const float anchor_angles[3], float units_per_metre, b2_view_t *view);
void B2_HistoryReset(b2_history_t *history);
bool B2_HistoryBegin(b2_history_t *history, b2_frame_t *frame);
bool B2_HistoryAcceptEye(b2_history_t *history, uint32_t eye);
bool B2_HistoryFinish(b2_history_t *history, bool submitted_pair);

// A true BeginFrame result owes exactly one EndFrame, including zero-layer
// frames. Runtime supplies poses, FOVs, timing, generation and render/tracking
// flags; the client builds views and supplies the logical/history identity.
bool B2XR_Requested(void);
uint32_t B2XR_ViewCount(void);
void B2XR_PollEvents(void);
bool B2XR_BeginFrame(b2_frame_t *frame);
// Startup frames may be submitted before the runtime presents the session.
// Finalise the initial relative head origin only once presentation is visible.
bool B2XR_InitialReferenceReady(void);
// Input/session prerequisites; return still requires a fresh committed frame.
bool B2XR_InteractionReady(void);
// Latches focus loss even if focus returns in the same event-poll batch.
uint32_t B2XR_InterruptionEpoch(void);
bool B2XR_EndFrame(bool rendered_pair);
void B2XR_InvalidateHistory(const char *reason);
void B2XR_RequestStop(void);
bool B2XR_StopRequested(void);
