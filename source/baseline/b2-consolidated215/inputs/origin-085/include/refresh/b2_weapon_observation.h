// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_WEAPON_OBSERVATION_H
#define B2_WEAPON_OBSERVATION_H
#include <stdbool.h>
#include <stdint.h>
// Private, read-only observation. No engine callbacks or new admission decision.
typedef struct {
    bool requested, valid, record_present, emission, idle;
    unsigned matrix_reads, slot;
    uint32_t number, sequence, ordinal, capture_ms, head_ms;
    uint64_t serial;
    int64_t pose_time; // Always the original command-pose time.
    bool display_mode, display_used;
    int64_t display_time;
    uint32_t display_capture_ms;
    int frame, oldframe;
    float backlerp, matrix[16], recoil[3];
} b2wp_observation_t;
void B2WP_ObserveWindow(bool enabled);
void B2WP_ObserveSnapshot(b2wp_observation_t *out);
#endif
