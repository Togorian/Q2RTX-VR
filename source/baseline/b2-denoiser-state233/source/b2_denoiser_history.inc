// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Included by the client. Publish only after the existing whole-frame commit. */
static b2ds_frame_t b2ds_frames[2];
static unsigned b2ds_count;

static void b2ds_record(const b2ds_frame_t *frame, bool committed)
{
    if (!committed || !frame || frame->mask != 15 || !frame->generation) {
        b2ds_count = 0;
        return;
    }
    for (unsigned eye = 0; eye < 4; ++eye) {
        const b2ds_view_t *v = &frame->view[eye];
        if (!v->valid || v->views != 4 || v->eye != eye ||
            v->logical != frame->logical || v->generation != frame->generation ||
            (v->branch != 1 && v->branch != 2)) {
            b2ds_count = 0;
            return;
        }
    }
    if (b2ds_count) {
        const b2ds_frame_t *last = &b2ds_frames[b2ds_count - 1];
        if (last->generation == frame->generation && frame->logical <= last->logical) {
            b2ds_count = 0;
            return;
        }
        if (last->generation == frame->generation && last->logical != UINT32_MAX &&
            last->logical + 1 == frame->logical) {
            b2ds_frames[0] = *last;
            b2ds_frames[1] = *frame;
            b2ds_count = 2;
            return;
        }
    }
    b2ds_frames[0] = *frame;
    b2ds_count = 1;
}

static void b2ds_print(uint32_t next_logical, uint32_t generation, uint32_t now)
{
    bool attempted = false;
    int result = R_B2DenoiserPipelineResult(&attempted);
    if (attempted)
        Com_Printf("B2DENOISE233 last_pipeline_create_result=%d\n", result);
    else
        Com_Printf("B2DENOISE233 pipeline_create=not_attempted\n");
    Com_Printf("B2DENOISE233 source=CPU_completed_frames shader_execution=unverified\n");
    if (!b2ds_count || !next_logical ||
        b2ds_frames[b2ds_count - 1].logical != next_logical - 1 ||
        b2ds_frames[b2ds_count - 1].generation != generation) {
        Com_Printf("B2DENOISE233 snapshot=unavailable_or_stale next_logical=%u generation=%u\n",
            next_logical, generation);
        return;
    }
    for (unsigned index = 0; index < b2ds_count; ++index) {
        const b2ds_frame_t *f = &b2ds_frames[index];
        Com_Printf("B2DENOISE233 frame logical=%u generation=%u completed_mask=%u age_ms=%u\n",
            f->logical, f->generation, f->mask, now - f->completed_ms);
        Com_Printf("B2DENOISE233 client_at_begin runtime_foveated=%d history_valid=%d waiting=%d origin_latched=%d\n",
            f->runtime_foveated, f->history_valid, f->headset_waiting, f->origin_latched);
        Com_Printf("B2DENOISE233 client_at_begin walk_ready=%d interrupted=%d\n",
            f->walk_ready, f->interrupted);
        for (unsigned eye = 0; eye < 4; ++eye) {
            const b2ds_view_t *v = &f->view[eye];
            Com_Printf("B2DENOISE233 view eye=%u branch=%s temporal_input=%d history_permission=%d\n",
                v->eye, v->branch == 1 ? "asvgf" : "compositing", v->temporal_input, v->history_permission);
            Com_Printf("B2DENOISE233 effective flt_enable=%.9g temporal_hf=%.9g atrous_hf=%.9g\n",
                v->enabled, v->temporal_hf, v->atrous_hf);
            Com_Printf("B2DENOISE233 effective min_hf=%.9g moments_hf=%.9g taa=%.9g\n",
                v->min_hf, v->moments_hf, v->taa);
        }
    }
}
