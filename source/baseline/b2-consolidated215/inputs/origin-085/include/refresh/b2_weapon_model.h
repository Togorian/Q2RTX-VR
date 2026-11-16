// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Private engine contract. No entity, game, save or shader layout changes.
#ifndef B2_WEAPON_MODEL_H
#define B2_WEAPON_MODEL_H
#include <stddef.h>
#include "b2_weapon.h"
#define B2WM_VERSION 0x42324d01u
#define B2WM_CALIBRATION 7u
typedef struct {
    uint32_t slot, calibration;
    uint64_t generation;
    unsigned char sha256[32];
} b2wm_key_t;
typedef struct {
    unsigned size, version;
    b2wm_key_t key;
    int frames, ready;
    float scale;
    double grip_from_muzzle[3];
    bool requires_body_partition;
} b2wm_info_t;
typedef struct {
    double muzzle[3], grip[3], basis[3][3]; // forward, left, up
    int frame, oldframe;
} b2wm_datum_t;
// Exact calibrated primitive partition in the immutable loaded MD2.
typedef struct {
    unsigned id, first, count;
    int frame, oldframe;
    float backlerp, matrix[16];
    bool visible;
} b2wm_part_t;
bool B2WM_GrenadeParts(const void *model, const b2wm_key_t *key,
    int frame, int oldframe, float backlerp, const b2w_pose_t *aim,
    const float corrected[3], bool held, b2wm_part_t parts[3]);
void B2WM_Forget(const void *model);
bool B2WM_Query(const void *model, unsigned version, unsigned size, b2wm_info_t *out);
bool B2WM_Datum(const void *model, const b2wm_key_t *key,
    int frame, int oldframe, float backlerp, b2wm_datum_t *out);
bool B2WM_Desired(const void *model, const b2wm_key_t *key,
    const b2w_pose_t *aim, float desired[3]);
bool B2WM_Matrix(const void *model, const b2wm_key_t *key,
    int frame, int oldframe, float backlerp, const b2w_pose_t *aim,
    const float corrected_muzzle[3], float column_major[16]);
// Private shared model correction; does not consume input or authorise fire.
bool B2WM_HyperStabilise(const void *model, const b2wm_key_t *key,
    int frame, int oldframe, float backlerp, bool tracked, float matrix[16]);
bool B2WM_Recoil(const b2w_pose_t *aim, const float angles[3], b2w_pose_t *out);
bool B2WM_Muzzle(const b2wm_datum_t *datum, const float matrix[16], int hand, b2w_pose_t *out);
#endif
