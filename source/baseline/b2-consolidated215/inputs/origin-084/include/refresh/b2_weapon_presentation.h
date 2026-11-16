// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Private engine-only presentation. No public entity, game, save or shader fields.
#ifndef B2_WEAPON_PRESENTATION_H
#define B2_WEAPON_PRESENTATION_H
#include "b2_weapon_bridge.h"
#include "b2_weapon_model.h"
#define B2WP_VERSION 0x42325007u
typedef struct {
    unsigned size, version;
    uint32_t handle;
    b2wm_key_t model;
    b2w_pose_t aim;                 // Original grip/aim, before calibrated muzzle placement.
    float desired[3], preview[3];   // A preview is never an emission event.
    uint32_t preview_ms;
    float body_origin[3], body_angles[3]; // Captured command body basis; presentation only.
} b2wp_source_t;
typedef struct {
    unsigned size, version;
    b2wp_source_t source;
    b2w_packet_t packet;
    b2w_command_t command;
    uint32_t number, sequence, ordinal;
    uint64_t emission_serial; // Zero means preview only, never an inferred shot.
    int emission_frame, emission_gunframe;
    b2wg_state_t grenade;
    float recoil[3]; // Read-only game view motion, never a firing input.
    bool emission_pose; // Matching completed game emission, not an inferred shot.
    bool idle_pose;
    int model_frame; // Independent of the completed event's gunframe.
} b2wp_record_t;
bool B2WP_Preview(b2w_packet_t *packet, b2wp_source_t *source,
    uint32_t handle, const void *model, const b2w_game_api_t *game, uint32_t now);
// A blocked result is game preview rejection after independent model/pose checks.
// It grants no firing/presentation; callers must recheck inventory and lifecycle.
bool B2WP_TryPreview(b2w_packet_t *packet, b2wp_source_t *source,
    uint32_t handle, const void *model, const b2w_game_api_t *game, uint32_t now, bool *blocked);
bool B2WP_Publish(const b2w_packet_t *packet, const b2wp_source_t *source, uint32_t now);
bool B2WP_CurrentAt(b2wp_record_t *out, uint32_t now);
bool B2W_Presentation(b2wp_record_t *out);
void B2WP_BeginView(bool requested);
void B2WP_RetireView(void);
bool B2WP_Stage(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, const b2wp_record_t *record, uint32_t now);
// Read-only natural-scene transform from the immutable staged matrix.
bool B2WP_ReadSceneMatrix(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, const float origin[3], const float angles[3], uint32_t now, float matrix[16]);
bool B2WP_RebaseView(const float origin[3], const float angles[3], uint32_t now, float muzzle[3]);
bool B2WP_ViewRequested(void);
bool B2WP_FrameReady(uint32_t now);
bool B2WP_ViewMatrix(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, float matrix[16]);
bool B2WP_ViewPart(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, unsigned id, b2wm_part_t *out);
bool B2WP_LeftHanded(void);
bool B2WP_HistoryMatches(void);
void B2WP_Commit(bool submitted);
#endif
