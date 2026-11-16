// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// UI-only eyes use the existing display image, HUD pipeline, mirror and XR copy.
// They neither activate temporal storage nor advance the world cadence.
static void b2_render_surface_eye(refdef_t *fd)
{
    const b2_frame_t *frame=fd->b2_frame;
    fd->b2_render_accepted=false;
    const uint32_t width=B2G_Width(),height=B2G_Height();
    if ((!B2_UISurfaceActive() && !B2_RealignActive391()) || !B2XR_Requested() || B2XR_StopRequested() ||
        !B2XR_InteractionReady() || !frame || !B2_ValidViewCount(frame->view_count) ||
        frame->view_count!=B2XR_ViewCount() || fd->b2_eye>=frame->view_count ||
        !frame->tracking_valid || !frame->should_render || frame->duration_ns<=0 ||
        frame->history_valid ||
        fd->num_entities || fd->num_particles || fd->num_dlights ||
        qvk.device_count!=1 || qvk.queue_idx_graphics!=qvk.queue_idx_transfer ||
        qvk.surf_is_hdr ||
        !B2G_Internal(width,height) ||
        qvk.extent_taa_images.width!=width || qvk.extent_taa_images.height!=height ||
        qvk.extent_render.width!=width || qvk.extent_render.height!=height ||
        SDL_AtomicGet(&b2_validation_failed) || b2_cadence.pair_prepared)
        Com_Error(ERR_FATAL,"B2_FAIL: invalid UI-only eye ownership");
    if (!fd->b2_eye) {
        if (b2_surface_pair.open || b2_mirror.held)
            Com_Error(ERR_FATAL,"B2_FAIL: UI pair replaced an incomplete pair");
        b2_surface_pair.open=true;b2_surface_pair.views=frame->view_count;
        b2_surface_pair.logical=frame->logical_index;
        b2_surface_pair.generation=frame->history_generation;
        b2_surface_pair.rendered=b2_surface_pair.finished=0;
    }
    if (!b2_surface_pair.open || b2_surface_pair.views!=frame->view_count ||
        b2_surface_pair.logical!=frame->logical_index ||
        b2_surface_pair.generation!=frame->history_generation ||
        b2_surface_pair.rendered!=((1u<<fd->b2_eye)-1u) ||
        b2_surface_pair.finished!=b2_surface_pair.rendered)
        Com_Error(ERR_FATAL,"B2_FAIL: UI eye order changed");
    vkpt_refdef.fd=fd;
    qvk.extent_taa_output=qvk.extent_render;
    VkCommandBuffer cmd=vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);
    VkImageMemoryBarrier barrier={.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask=VK_ACCESS_MEMORY_READ_BIT|VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout=VK_IMAGE_LAYOUT_GENERAL,.newLayout=VK_IMAGE_LAYOUT_GENERAL,
        .srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,
        .image=qvk.images[VKPT_IMG_TAA_OUTPUT],.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
    vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,0,NULL,0,NULL,1,&barrier);
    const VkClearColorValue black={{0.f,0.f,0.f,1.f}};
    vkCmdClearColorImage(cmd,barrier.image,VK_IMAGE_LAYOUT_GENERAL,&black,1,&barrier.subresourceRange);
    barrier.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,0,NULL,0,NULL,1,&barrier);
    vkpt_submit_command_buffer_simple(cmd,qvk.queue_graphics,true);
    b2_surface_pair.rendered|=1u<<fd->b2_eye;
    frame_ready=true;fd->b2_render_accepted=true;
}
