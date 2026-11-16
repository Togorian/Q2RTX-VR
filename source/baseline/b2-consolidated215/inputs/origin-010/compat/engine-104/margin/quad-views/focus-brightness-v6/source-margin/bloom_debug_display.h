// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
#pragma once

/* Caller supplies this view's active TAA extent and the actual allocation.
 * The quarter-sized source is the same valid floor extent used by blur. */
static int bloom_debug_copy(VkCommandBuffer cmd, VkImage source, VkImage destination,
    VkExtent2D active, VkExtent2D allocation)
{
    if (!source || !destination || source == destination || active.width < 4 ||
        active.height < 4 || active.width > allocation.width ||
        active.height > allocation.height || active.width > 0x7fffffffu ||
        active.height > 0x7fffffffu) return 0;
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageMemoryBarrier barriers[2] = {
        {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
         .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
         .oldLayout = VK_IMAGE_LAYOUT_GENERAL, .newLayout = VK_IMAGE_LAYOUT_GENERAL,
         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .image = source, .subresourceRange = range},
        {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         .srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
         .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
         .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
         .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .image = destination, .subresourceRange = range}
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 2, barriers);
    VkImageBlit region = {
        .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .srcOffsets = {{0,0,0}, {(int32_t)(active.width/4), (int32_t)(active.height/4),1}},
        .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .dstOffsets = {{0,0,0}, {(int32_t)active.width, (int32_t)active.height,1}}
    };
    vkCmdBlitImage(cmd, source, VK_IMAGE_LAYOUT_GENERAL, destination,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
    barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barriers[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    barriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barriers[1].newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 2, barriers);
    return 1;
}

#ifndef BLOOM_DEBUG_CPU_TEST
static void bloom_debug_show_image(VkCommandBuffer cmd_buf, int vis_img)
{
    if (!bloom_debug_copy(cmd_buf, qvk.images[vis_img], qvk.images[VKPT_IMG_TAA_OUTPUT],
        qvk.extent_taa_output, qvk.extent_taa_images))
        Com_Error(ERR_FATAL, "Bloom debug display: invalid active extent or image");
}
#endif
