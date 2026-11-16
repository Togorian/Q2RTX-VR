// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Private CPU observations only; no renderer ABI or shader layout changes. */
#ifndef B2_DENOISER_STATE233_H
#define B2_DENOISER_STATE233_H
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t logical, generation, eye, views;
    bool valid, temporal_input;
    int branch, history_permission; /* branch: 1 ASVGF, 2 compositing */
    float enabled, temporal_hf, atrous_hf, min_hf, moments_hf, taa;
} b2ds_view_t;

typedef struct {
    uint32_t logical, generation, mask, completed_ms;
    bool runtime_foveated, history_valid;
    bool headset_waiting, origin_latched, walk_ready, interrupted;
    b2ds_view_t view[4];
} b2ds_frame_t;

bool R_B2DenoiserReadView(uint32_t logical, uint32_t generation,
    uint32_t eye, b2ds_view_t *out);
int R_B2DenoiserPipelineResult(bool *attempted);
struct b2_frame_s;
bool B2XR_DenoiserFoveated(const struct b2_frame_s *frame);
#endif
