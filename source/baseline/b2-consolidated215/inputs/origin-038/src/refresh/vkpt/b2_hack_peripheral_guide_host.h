// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Design/direction by Togorian; implementation with Codex assistance.
// Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
// Modified 11 November 2026: provenance/notice clarification only. See PROVENANCE.md.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* Private HF experiment; GPL-2.0-or-later, like asvgf.c.
 * Six sampled descriptors reference existing same-eye peripheral images.
 * No image, filtered history, final colour or replay payload is copied. */
#include "b2_hack_peripheral_guide.h"
#ifdef REFERENCE_B_B2
/* These existing textures.c accessors are absent from the frozen engine-103 header. */
extern VkDescriptorSet vkpt_b2_view_descriptor(uint32_t view, uint32_t parity);
extern VkImage vkpt_b2_view_image(uint32_t view, int image_index);
#endif

typedef struct {
    float vp[16];
    float origin[4];
    float extent_jitter[4];
    uint32_t flags[4];
} b2_hack_guide_push_t;
typedef char b2_hack_guide_push_size_check[sizeof(b2_hack_guide_push_t) == 112 ? 1 : -1];

typedef struct {
    b2_hack_guide_push_t push;
    uint32_t view, logical_index, history_generation;
    bool quad_valid, sampling_valid, submitted;
} b2_hack_guide_state_t;

static b2_hack_guide_state_t b2_hack_guide_current, b2_hack_guide_donor[2];
static VkDescriptorSetLayout b2_hack_guide_set_layout;
static VkDescriptorPool b2_hack_guide_pool;
static VkDescriptorSet b2_hack_guide_sets[MAX_FRAMES_IN_FLIGHT][4];
static VkPipelineLayout b2_hack_guide_pipeline_layout;
static void b2_hack_guide_destroy(void);
static const uint32_t b2_hack_guide_images[6] = {
    VKPT_IMG_PT_COLOR_HF, VKPT_IMG_PT_COLOR_SPEC, VKPT_IMG_PT_VISBUF_PRIM_A,
    VKPT_IMG_PT_VIEW_DEPTH_A, VKPT_IMG_PT_GEO_NORMAL_A, VKPT_IMG_ASVGF_GRAD_SMPL_POS_A
};

void b2_hack_guide_snapshot(const float *projection, const float *view_matrix,
    const float *origin, const float *jitter, uint32_t width, uint32_t height,
    bool quad_mode_valid, bool sampling_valid, uint32_t view,
    uint32_t logical_index, uint32_t history_generation)
{
    memset(&b2_hack_guide_current, 0, sizeof(b2_hack_guide_current));
    if (!quad_mode_valid || view >= 4)
        return;
    if (view == 0)
        memset(b2_hack_guide_donor, 0, sizeof(b2_hack_guide_donor));
    b2_hack_guide_state_t *state = &b2_hack_guide_current;
    state->quad_valid = true;
    state->view = view;
    state->logical_index = logical_index;
    state->history_generation = history_generation;
    state->sampling_valid = sampling_valid && width > 0 && height > 0 && (width & 1) == 0;
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row) {
            float value = 0.0f;
            for (int k = 0; k < 4; ++k)
                value += projection[k * 4 + row] * view_matrix[col * 4 + k];
            state->push.vp[col * 4 + row] = value;
        }
    memcpy(state->push.origin, origin, sizeof(state->push.origin));
    state->push.extent_jitter[0] = (float)width;
    state->push.extent_jitter[1] = (float)height;
    state->push.extent_jitter[2] = jitter[0];
    state->push.extent_jitter[3] = jitter[1];
    if (view < 2)
        b2_hack_guide_donor[view] = *state;
}

void b2_hack_guide_submitted(uint32_t view, uint32_t logical_index,
    uint32_t history_generation)
{
    if (view < 2) {
        b2_hack_guide_state_t *state = &b2_hack_guide_donor[view];
        if (state->quad_valid && state->logical_index == logical_index &&
            state->history_generation == history_generation)
            state->submitted = true;
    }
}

static VkResult b2_hack_guide_initialise(const VkDescriptorSetLayout *base_layouts)
{
    VkDescriptorSetLayoutBinding bindings[6];
    memset(bindings, 0, sizeof(bindings));
    for (uint32_t i = 0; i < LENGTH(bindings); ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = LENGTH(bindings), .pBindings = bindings
    };
    VkResult result = vkCreateDescriptorSetLayout(qvk.device, &layout_info, NULL,
        &b2_hack_guide_set_layout);
    if (result != VK_SUCCESS) { b2_hack_guide_destroy(); return result; }
    VkDescriptorPoolSize pool_size = {
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * 4 * 6
    };
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = MAX_FRAMES_IN_FLIGHT * 4,
        .poolSizeCount = 1, .pPoolSizes = &pool_size
    };
    result = vkCreateDescriptorPool(qvk.device, &pool_info, NULL, &b2_hack_guide_pool);
    if (result != VK_SUCCESS) { b2_hack_guide_destroy(); return result; }
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT * 4];
    for (uint32_t i = 0; i < LENGTH(layouts); ++i) layouts[i] = b2_hack_guide_set_layout;
    VkDescriptorSetAllocateInfo allocation = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = b2_hack_guide_pool,
        .descriptorSetCount = LENGTH(layouts), .pSetLayouts = layouts
    };
    result = vkAllocateDescriptorSets(qvk.device, &allocation, &b2_hack_guide_sets[0][0]);
    if (result != VK_SUCCESS) { b2_hack_guide_destroy(); return result; }
    VkDescriptorSetLayout pipeline_layouts[4] = {
        base_layouts[0], base_layouts[1], base_layouts[2], b2_hack_guide_set_layout
    };
    VkPushConstantRange push_range = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT, .offset = 0,
        .size = sizeof(b2_hack_guide_push_t)
    };
    VkPipelineLayoutCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = LENGTH(pipeline_layouts), .pSetLayouts = pipeline_layouts,
        .pushConstantRangeCount = 1, .pPushConstantRanges = &push_range
    };
    memset(&b2_hack_guide_current, 0, sizeof(b2_hack_guide_current));
    memset(b2_hack_guide_donor, 0, sizeof(b2_hack_guide_donor));
    result = vkCreatePipelineLayout(qvk.device, &pipeline_info, NULL,
        &b2_hack_guide_pipeline_layout);
    if (result != VK_SUCCESS) b2_hack_guide_destroy();
    return result;
}

static void b2_hack_guide_destroy(void)
{
    vkDestroyPipelineLayout(qvk.device, b2_hack_guide_pipeline_layout, NULL);
    vkDestroyDescriptorPool(qvk.device, b2_hack_guide_pool, NULL);
    vkDestroyDescriptorSetLayout(qvk.device, b2_hack_guide_set_layout, NULL);
    b2_hack_guide_pipeline_layout = VK_NULL_HANDLE;
    b2_hack_guide_pool = VK_NULL_HANDLE;
    b2_hack_guide_set_layout = VK_NULL_HANDLE;
    memset(b2_hack_guide_sets, 0, sizeof(b2_hack_guide_sets));
    memset(&b2_hack_guide_current, 0, sizeof(b2_hack_guide_current));
    memset(b2_hack_guide_donor, 0, sizeof(b2_hack_guide_donor));
}

static VkDescriptorSet b2_hack_guide_bind(VkCommandBuffer command_buffer)
{
    const b2_hack_guide_state_t *current = &b2_hack_guide_current;
    uint32_t view = 0;
#ifdef REFERENCE_B_B2
    if (vkpt_refdef.fd && vkpt_refdef.fd->b2_active)
        view = vkpt_refdef.fd->b2_eye;
#endif
    if (view >= 4 || qvk.current_frame_index >= MAX_FRAMES_IN_FLIGHT)
        Com_Error(ERR_FATAL, "private peripheral guide: invalid descriptor slot");
    /* R_BeginFrame_RTX has waited this physical slot's fence. Each view has its
     * own destination set, so later views never update an in-flight view set. */
    VkDescriptorSet destination = b2_hack_guide_sets[qvk.current_frame_index][view];
    VkDescriptorSet source = qvk_get_current_desc_set_textures();
    b2_hack_guide_push_t push = { 0 };
    bool donor_valid = false;
    uint32_t donor_view = 0, parity = 0;
#ifdef REFERENCE_B_B2
    if (current->quad_valid && current->view >= 2) {
        push.flags[0] = 1; /* Mode 9 focus falls back to E if the donor is absent. */
        donor_view = current->view - 2;
        const b2_hack_guide_state_t *donor = &b2_hack_guide_donor[donor_view];
        donor_valid = current->sampling_valid && donor->quad_valid && donor->sampling_valid &&
            donor->submitted && donor->logical_index == current->logical_index &&
            donor->history_generation == current->history_generation && qvk.b2_temporal_activated;
        if (donor_valid) {
            parity = current->logical_index & 1u;
            source = vkpt_b2_view_descriptor(donor_view, parity);
            donor_valid = source != VK_NULL_HANDLE;
            for (uint32_t i = 0; donor_valid && i < LENGTH(b2_hack_guide_images); ++i) {
                uint32_t index = b2_hack_guide_images[i];
                if (index >= VKPT_IMG_PT_VISBUF_PRIM_A) index ^= parity;
                if (vkpt_b2_view_image(donor_view, index) == VK_NULL_HANDLE) donor_valid = false;
            }
            if (donor_valid) {
                push = donor->push;
                push.flags[0] = push.flags[1] = 1;
            }
        }
    }
    if (!donor_valid) source = qvk_get_current_desc_set_textures();
#endif
    VkCopyDescriptorSet copies[6];
    for (uint32_t i = 0; i < LENGTH(copies); ++i) {
        copies[i] = (VkCopyDescriptorSet) {
            .sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
            .srcSet = source, .srcBinding = BINDING_OFFSET_TEXTURES + b2_hack_guide_images[i],
            .dstSet = destination, .dstBinding = i, .descriptorCount = 1
        };
    }
    vkUpdateDescriptorSets(qvk.device, 0, NULL, LENGTH(copies), copies);
#ifdef REFERENCE_B_B2
    if (donor_valid) {
        VkImageMemoryBarrier barriers[6];
        memset(barriers, 0, sizeof(barriers));
        for (uint32_t i = 0; i < LENGTH(barriers); ++i) {
            uint32_t index = b2_hack_guide_images[i];
            if (index >= VKPT_IMG_PT_VISBUF_PRIM_A) index ^= parity;
            barriers[i] = (VkImageMemoryBarrier) {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_GENERAL, .newLayout = VK_IMAGE_LAYOUT_GENERAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = vkpt_b2_view_image(donor_view, index),
                .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
            };
        }
        VkPipelineStageFlags producer_stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
            (qvk.use_ray_query ? 0 : VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
        vkCmdPipelineBarrier(command_buffer, producer_stages, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 0, NULL, 0, NULL, LENGTH(barriers), barriers);
    }
#endif
    vkCmdPushConstants(command_buffer, b2_hack_guide_pipeline_layout,
        VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
    return destination;
}
