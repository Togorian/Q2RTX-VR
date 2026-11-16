// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Private CPU contract. Does not extend game, player, usercmd or save structures.
#ifndef B2_WEAPON_H
#define B2_WEAPON_H
#include "b2_controller.h"

typedef struct {
    uint32_t action, xr, world, recentre, owner, calibration;
} b2w_epochs_t;

typedef struct {
    b2w_epochs_t epochs;
    uint64_t serial;
    int64_t pose_time;
    uint32_t capture_ms, head_ms;
    unsigned hand;
    float origin[3], forward[3], right[3], up[3];
} b2w_pose_t;

typedef struct {
    b2w_epochs_t epochs;
    float reference_origin[3], reference_yaw;
    float anchor_origin[3], anchor_angles[3], units;
    bool committed;
} b2w_space_t;

typedef struct {
    bool open, rearm, configured, hovering, nav_latched, sliding;
    unsigned page; // 0 weapons, 1 usable items; every closure resets to weapons.
    unsigned hand;
    b2w_epochs_t epochs;
    uint64_t serial;
    uint32_t candidate_ms, sample_ms, hover_ms, slide_ms;
    unsigned candidate_samples, hover_samples, slide_from;
    int candidate, selected;
} b2w_selector_t;

typedef struct {
    bool suppress, opened, closed, cancelled;
    int selected; // One weapon release edge; the game validates/acknowledges it.
    int used; // One item grip-release edge; never retained or retried.
    bool page_changed;
} b2w_selection_t;

bool B2W_EpochsEqual(const b2w_epochs_t *a, const b2w_epochs_t *b);
bool B2W_PoseCurrent(const b2w_pose_t *p, const b2w_epochs_t *epochs, uint32_t now);
bool B2W_TurnSpace(const b2w_space_t *old, const float head[3],
    const float angles[3], const float retire_raw[3], b2w_space_t *out);
bool B2W_BuildAim(const b2c_snapshot_t *sample, unsigned hand,
    const b2w_space_t *space, uint32_t now, b2w_pose_t *out);
bool B2W_HUDRay(const b2c_snapshot_t *sample, unsigned hand,
    float aspect_height_over_width, float *x, float *y);
bool B2W_PlaneRay(const b2w_pose_t *plane, const b2w_pose_t *aim,
    float aspect, float *x, float *y);
bool B2W_GridCell(unsigned slot, float aspect_height_over_width, float rect[4]);
int B2W_GridSlot(float x, float y, float aspect_height_over_width);
bool B2W_PageArrow(unsigned page, float aspect, float rect[4]);
bool B2W_PageHit(unsigned page, float x, float y, float aspect);
float B2W_PageProgress(uint32_t start, uint32_t now);
void B2W_CancelSelector(b2w_selector_t *state);
b2w_selection_t B2W_Select(b2w_selector_t *state, const b2c_snapshot_t *sample,
    const b2w_epochs_t *epochs, uint32_t now, unsigned hand, bool eligible,
    bool weapon_ready, uint16_t available, int pointed_slot);
#endif
