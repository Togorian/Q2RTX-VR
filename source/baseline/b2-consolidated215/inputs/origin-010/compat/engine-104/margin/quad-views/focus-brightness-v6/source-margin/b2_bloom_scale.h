// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <math.h>
#include <stdint.h>
#include <string.h>

/* CPU-only policy, shared by production and its focused test. Dimensions are
 * actual internal TAA dimensions, never runtime output sizes or UI settings. */
typedef struct {
    uint32_t logical, generation, render_frame, width, height;
    float projection[16];
    int valid;
} b2_bloom_binding_t;
typedef struct { b2_bloom_binding_t peripheral[2]; } b2_bloom_state_t;
typedef struct {
    float factor[2], requested_sigma[2], sigma[2];
    int status; /* 0 unchanged; 1 applied; 2 capped; -1 invalid/stale */
} b2_bloom_scale_t;

static int b2_bloom_projection_valid(const float *p)
{
    if (!p) return 0;
    for (int i = 0; i < 16; ++i) if (!isfinite(p[i])) return 0;
    return fabsf(p[0]) > 0.f && fabsf(p[5]) > 0.f && p[1] == 0.f &&
        p[4] == 0.f && p[3] == 0.f && p[7] == 0.f && p[15] == 0.f;
}

static b2_bloom_scale_t b2_bloom_scale(b2_bloom_state_t *state,
    int enabled, int active, uint32_t count, uint32_t view,
    uint32_t logical, uint32_t generation, uint32_t render_frame,
    uint32_t width, uint32_t height, const float *projection, float base_sigma)
{
    b2_bloom_scale_t out = {{1.f, 1.f}, {base_sigma, base_sigma},
        {base_sigma, base_sigma}, 0};
    if (!active || count != 4 || view >= 4) {
        memset(state, 0, sizeof(*state));
        return out;
    }
    if (view == 0) memset(state, 0, sizeof(*state));
    if (view < 2) {
        b2_bloom_binding_t *b = &state->peripheral[view];
        b->valid = width >= 4 && height >= 4 && b2_bloom_projection_valid(projection);
        b->logical = logical; b->generation = generation;
        b->render_frame = render_frame; b->width = width; b->height = height;
        if (b->valid) memcpy(b->projection, projection, sizeof(b->projection));
        return out;
    }
    if (!enabled) return out;
    const b2_bloom_binding_t *b = &state->peripheral[view - 2];
    if (!b->valid || b->logical != logical || b->generation != generation ||
        (uint32_t)(render_frame - b->render_frame) != 2 || width < 4 || height < 4 ||
        !b2_bloom_projection_valid(projection) || !isfinite(base_sigma) ||
        base_sigma < 1.f || base_sigma > 100.f) {
        out.status = -1; return out;
    }
    /* A projection tangent span is 2/abs(P[0]) or 2/abs(P[5]), including asymmetric
     * frusta. Account for the real quarter-grid size and sigma's height basis.
     * Existing B2 guards keep all internal dimensions equal. */
    uint32_t dims[2] = {width / 4, height / 4};
    uint32_t other[2] = {b->width / 4, b->height / 4};
    out.status = 1;
    for (int axis = 0; axis < 2; ++axis) {
        int index = axis ? 5 : 0;
        double factor = ((double)b->height / height) *
            ((double)dims[axis] / other[axis]) *
            fabs((double)projection[index] / b->projection[index]);
        double requested = base_sigma * factor;
        if (!isfinite(factor) || !isfinite(requested) || factor <= 0. ||
            factor > 1.e6 || requested > 1.e8) {
            out.status = -1;
            out.factor[0] = out.factor[1] = 1.f;
            out.sigma[0] = out.sigma[1] = base_sigma;
            out.requested_sigma[0] = out.requested_sigma[1] = base_sigma;
            return out;
        }
        out.factor[axis] = (float)factor;
        out.requested_sigma[axis] = (float)requested;
        out.sigma[axis] = (float)fmax(1., fmin(100., requested));
        if (requested < 1. || requested > 100.) out.status = 2;
    }
    return out;
}
