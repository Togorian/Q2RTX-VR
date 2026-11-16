// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
/* CPU copy of final per-view values; validity follows existing render acceptance. */
static b2ds_view_t b2ds_render_view;
static bool b2ds_temporal_input;

static void b2ds_capture_view(const refdef_t *fd, const QVKUniformBuffer_t *ubo,
    int branch)
{
    const b2_frame_t *frame = fd->b2_frame;
    b2ds_render_view = (b2ds_view_t) {
        .logical = frame->logical_index, .generation = frame->history_generation,
        .eye = fd->b2_eye, .views = frame->view_count,
        .valid = fd->b2_render_accepted, .temporal_input = b2ds_temporal_input,
        .branch = branch, .history_permission = ubo->temporal_history_valid,
        .enabled = ubo->flt_enable, .temporal_hf = ubo->flt_temporal_hf,
        .atrous_hf = ubo->flt_atrous_hf, .min_hf = ubo->flt_min_alpha_color_hf,
        .moments_hf = ubo->flt_min_alpha_moments_hf, .taa = ubo->flt_taa
    };
}

bool R_B2DenoiserReadView(uint32_t logical, uint32_t generation,
    uint32_t eye, b2ds_view_t *out)
{
    if (!out || !b2ds_render_view.valid || b2ds_render_view.logical != logical ||
        b2ds_render_view.generation != generation || b2ds_render_view.eye != eye)
        return false;
    *out = b2ds_render_view;
    b2ds_render_view.valid = false;
    return true;
}
