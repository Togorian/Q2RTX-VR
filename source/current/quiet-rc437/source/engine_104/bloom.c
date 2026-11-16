/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/*
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/* Modified by Togorian with AI assistance, 12 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */


#include "vkpt.h"
#include "system/system.h"
#ifdef REFERENCE_B_B2
#include "b2_bloom_scale.h"
static b2_bloom_state_t b2_bloom_state;
static cvar_t *cvar_b2_bloom_scale;
#endif

enum {
	BLUR,
	COMPOSITE,
	DOWNSCALE,

	BLOOM_NUM_PIPELINES
};

static VkPipeline pipelines[BLOOM_NUM_PIPELINES];
static VkPipelineLayout pipeline_layout_blur;
static VkPipelineLayout pipeline_layout_composite;

struct bloom_blur_push_constants {
	float pixstep_x;
	float pixstep_y;
	float argument_scale;
	float normalization_scale;
	int num_samples;
	int pass;
};

static struct bloom_blur_push_constants push_constants_hblur;
static struct bloom_blur_push_constants push_constants_vblur;

cvar_t *cvar_bloom_enable = NULL;
cvar_t *cvar_bloom_debug = NULL;
cvar_t *cvar_bloom_sigma = NULL;
cvar_t *cvar_bloom_intensity = NULL;
cvar_t *cvar_bloom_sigma_water = NULL;
cvar_t *cvar_bloom_intensity_water = NULL;

static float bloom_intensity;
static float bloom_sigma;
static float under_water_animation;
static uint32_t menu_start_ms;
static float menu_phase;

#include "../b2_bloom_margin.h"

static void compute_push_constants(void)
{
	float sigma_pixels = bloom_sigma * (float)(qvk.extent_taa_output.height);

	float effective_sigma = sigma_pixels * 0.25f;
	effective_sigma = min(effective_sigma, 100.f);
	effective_sigma = max(effective_sigma, 1.f);


#ifdef REFERENCE_B_B2
    const refdef_t *fd = vkpt_refdef.fd;
    const b2_frame_t *frame = fd && fd->b2_active ? fd->b2_frame : NULL;
    uint32_t view = frame ? fd->b2_eye : 0;
    b2_bloom_scale_t scaled = b2_bloom_scale(&b2_bloom_state,
        cvar_b2_bloom_scale && cvar_b2_bloom_scale->integer == 1,
        frame != NULL, frame ? frame->view_count : 0, view,
        frame ? frame->logical_index : 0, frame ? frame->history_generation : 0,
        qvk.frame_counter, qvk.extent_taa_output.width, qvk.extent_taa_output.height,
        frame && view < frame->view_count ? frame->views[view].P : NULL, effective_sigma);
    float sigma_y = scaled.sigma[1];
    effective_sigma = scaled.sigma[0];
    /* One line per changed state, plus a sampled line every 120 logical frames.
     * CPU recording only: it does not certify dispatch completion or pixels. */
    static int previous_mode[4] = {-99, -99, -99, -99};
    static int previous_status[4] = {-99, -99, -99, -99};
    if (frame && view < 4 && (previous_mode[view] != cvar_b2_bloom_scale->integer ||
        previous_status[view] != scaled.status || frame->logical_index % 120 == 0)) {
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_BLOOM_KERNEL logical=%u generation=%u view=%u internal=%ux%u mode=%d status=%d factor=%.6f,%.6f requested=%.6f,%.6f sigma=%.6f,%.6f recorded_only=1\n",
            frame->logical_index, frame->history_generation, view,
            qvk.extent_taa_output.width, qvk.extent_taa_output.height,
            cvar_b2_bloom_scale->integer, scaled.status, scaled.factor[0], scaled.factor[1],
            scaled.requested_sigma[0], scaled.requested_sigma[1], scaled.sigma[0], scaled.sigma[1]);
        previous_mode[view] = cvar_b2_bloom_scale->integer;
        previous_status[view] = scaled.status;
    }
#endif

	push_constants_hblur.pixstep_x = 1.f;
	push_constants_hblur.pixstep_y = 0.f;
	push_constants_hblur.argument_scale = -1.f / (2.0 * effective_sigma * effective_sigma);
	push_constants_hblur.normalization_scale = 1.f / (sqrtf(2 * M_PI) * effective_sigma);
	push_constants_hblur.num_samples = roundf(effective_sigma * 4.f);
	push_constants_hblur.pass = 0;

	push_constants_vblur = push_constants_hblur;
	push_constants_vblur.pixstep_x = 0.f;
	push_constants_vblur.pixstep_y = 1.f;
	push_constants_vblur.pass = 1;
#ifdef REFERENCE_B_B2
    if (scaled.status > 0) {
        push_constants_vblur.argument_scale = -1.f / (2.0 * sigma_y * sigma_y);
        push_constants_vblur.normalization_scale = 1.f / (sqrtf(2 * M_PI) * sigma_y);
        push_constants_vblur.num_samples = roundf(sigma_y * 4.f);
    }
#endif
}
static float bloom_radius320(void)
{
#ifdef REFERENCE_B_B2
    if(B2XR_Requested())
        return Q_clipf(cvar_bloom_sigma->value,.001f,.050f);
#endif
    return cvar_bloom_sigma->value;
}

void vkpt_bloom_reset()
{
	bloom_intensity = cvar_bloom_intensity->value;
	bloom_sigma = bloom_radius320();
	under_water_animation = 0.f;
	menu_start_ms = 0;
	menu_phase = 0.f;
}

static float mix(float a, float b, float s)
{
	return a * (1.f - s) + b * s;
}

void vkpt_bloom_advance(vkpt_bloom_snapshot_t *snapshot, float frame_time, bool under_water, bool menu_mode)
{
	// Radius is sigma/view height; intensity is the existing composite blend.
	// Scale special scenes relative to the original defaults so all defaults
	// remain identical, while zero intensity also clears menu/water bloom.
	const float normal_radius = bloom_radius320();
	const float radius_scale = normal_radius / 0.037f;
	const float intensity_scale = cvar_bloom_intensity->value / 0.002f;
	if (under_water)
	{
		under_water_animation = min(1.f, under_water_animation + frame_time * 3.f);
		bloom_intensity = cvar_bloom_intensity_water->value * intensity_scale;
		bloom_sigma = normal_radius;
	}
	else
	{
		under_water_animation = max(0.f, under_water_animation - frame_time * 3.f);
		bloom_intensity = mix(cvar_bloom_intensity->value, cvar_bloom_intensity_water->value * intensity_scale, under_water_animation);
		bloom_sigma = mix(normal_radius, cvar_bloom_sigma_water->value * radius_scale, under_water_animation);
	}

	if (menu_mode)
	{
		if (menu_start_ms == 0)
			menu_start_ms = Sys_Milliseconds();
		uint32_t current_ms = Sys_Milliseconds();

		menu_phase = max(0.f, min(1.f, (float)(current_ms - menu_start_ms) / 150.f));
		float phase = powf(menu_phase, 0.25f);

		bloom_sigma = phase * 0.03f * radius_scale;
		bloom_intensity = min(1.f, intensity_scale); // Full menu blur is the upper bound.
	}
	else
	{
		menu_start_ms = 0;
		menu_phase = 0.f;
	}

	snapshot->under_water_animation = under_water_animation;
	snapshot->bloom_intensity = bloom_intensity;
	snapshot->bloom_sigma = bloom_sigma;
	snapshot->tonemap_hdr_clamp_strength = menu_phase;
	snapshot->menu_phase = menu_phase;
	snapshot->menu_start_ms = menu_start_ms;
}

void vkpt_bloom_apply(QVKUniformBuffer_t *ubo, const vkpt_bloom_snapshot_t *snapshot)
{
	under_water_animation = snapshot->under_water_animation;
	bloom_intensity = snapshot->bloom_intensity;
	bloom_sigma = snapshot->bloom_sigma;
	menu_phase = snapshot->menu_phase;
	menu_start_ms = snapshot->menu_start_ms;
	ubo->bloom_intensity = snapshot->bloom_intensity;
	ubo->tonemap_hdr_clamp_strength = snapshot->tonemap_hdr_clamp_strength;
}

void vkpt_bloom_update(QVKUniformBuffer_t *ubo, float frame_time, bool under_water, bool menu_mode)
{
	vkpt_bloom_snapshot_t snapshot;
	vkpt_bloom_advance(&snapshot, frame_time, under_water, menu_mode);
	vkpt_bloom_apply(ubo, &snapshot);
}

VkResult
vkpt_bloom_initialize()
{
	cvar_bloom_enable = Cvar_Get("bloom_enable", "1", 0);
    bm_control = Cvar_Get("b2_bloom_margin", "0", 0);
#ifdef REFERENCE_B_B2
    cvar_b2_bloom_scale = Cvar_Get("b2_bloom_scale", "1", 0);
#endif
	cvar_bloom_debug = Cvar_Get("bloom_debug", "0", 0);
	cvar_bloom_sigma = Cvar_Get("bloom_sigma", "0.037", CVAR_ARCHIVE); // relative to screen height
	cvar_bloom_intensity = Cvar_Get("bloom_intensity", "0.002", CVAR_ARCHIVE);
	cvar_bloom_sigma_water = Cvar_Get("bloom_sigma_water", "0.037", 0);
	cvar_bloom_intensity_water = Cvar_Get("bloom_intensity_water", "0.2", 0);

	VkDescriptorSetLayout desc_set_layouts[] = {
		qvk.desc_set_layout_ubo, qvk.desc_set_layout_textures,
		qvk.desc_set_layout_vertex_buffer
	};

	VkPushConstantRange push_constant_range = {
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset     = 0,
		.size       = sizeof(push_constants_hblur)
	};

	CREATE_PIPELINE_LAYOUT(qvk.device, &pipeline_layout_blur,
		.setLayoutCount         = LENGTH(desc_set_layouts),
		.pSetLayouts            = desc_set_layouts,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges    = &push_constant_range
	);
	ATTACH_LABEL_VARIABLE(pipeline_layout_blur, PIPELINE_LAYOUT);

	CREATE_PIPELINE_LAYOUT(qvk.device, &pipeline_layout_composite,
		.setLayoutCount         = LENGTH(desc_set_layouts),
		.pSetLayouts            = desc_set_layouts,
	);
	ATTACH_LABEL_VARIABLE(pipeline_layout_composite, PIPELINE_LAYOUT);

	return VK_SUCCESS;
}

VkResult
vkpt_bloom_destroy()
{
	vkDestroyPipelineLayout(qvk.device, pipeline_layout_blur, NULL);
	pipeline_layout_blur = NULL;
	vkDestroyPipelineLayout(qvk.device, pipeline_layout_composite, NULL);
	pipeline_layout_composite = NULL;

	return VK_SUCCESS;
}

VkResult
vkpt_bloom_create_pipelines()
{
	VkComputePipelineCreateInfo pipeline_info[BLOOM_NUM_PIPELINES] = {
		[BLUR] = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = SHADER_STAGE(QVK_MOD_BLOOM_BLUR_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_blur,
		},
		[COMPOSITE] = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = SHADER_STAGE(QVK_MOD_BLOOM_COMPOSITE_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_composite,
		},
		[DOWNSCALE] = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = SHADER_STAGE(QVK_MOD_BLOOM_DOWNSCALE_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_composite,
		}
	};

	_VK(vkCreateComputePipelines(qvk.device, 0, LENGTH(pipeline_info), pipeline_info, 0, pipelines));
    bm_create();
	return VK_SUCCESS;
}

VkResult
vkpt_bloom_destroy_pipelines()
{
    bm_destroy();
	for(int i = 0; i < BLOOM_NUM_PIPELINES; i++)
		vkDestroyPipeline(qvk.device, pipelines[i], NULL);

	return VK_SUCCESS;
}


#define BARRIER_COMPUTE(cmd_buf, img) \
	do { \
		VkImageSubresourceRange subresource_range = { \
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT, \
			.baseMipLevel   = 0, \
			.levelCount     = 1, \
			.baseArrayLayer = 0, \
			.layerCount     = 1 \
		}; \
		IMAGE_BARRIER(cmd_buf, \
				.image            = img, \
				.subresourceRange = subresource_range, \
				.srcAccessMask    = VK_ACCESS_SHADER_WRITE_BIT, \
				.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT, \
				.oldLayout        = VK_IMAGE_LAYOUT_GENERAL, \
				.newLayout        = VK_IMAGE_LAYOUT_GENERAL, \
		); \
	} while(0)

#include "bloom_debug_display.h"

VkResult
vkpt_bloom_record_cmd_buffer(VkCommandBuffer cmd_buf)
{
	VkExtent2D extent = qvk.extent_taa_output;

	compute_push_constants();

	VkDescriptorSet desc_sets[] = {
		qvk.desc_set_ubo,
		qvk_get_current_desc_set_textures(),
		qvk.desc_set_vertex_buffer
	};

	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_TAA_OUTPUT]);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines[DOWNSCALE]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_composite, 0, LENGTH(desc_sets), desc_sets, 0, 0);

	vkCmdDispatch(cmd_buf,
		(extent.width / 4 + 15) / 16,
		(extent.height / 4 + 15) / 16,
		1);

	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_BLOOM_VBLUR]);

    bm_capture(cmd_buf, extent);

	if(cvar_bloom_debug->integer == 1)
	{
		bloom_debug_show_image(cmd_buf, VKPT_IMG_BLOOM_VBLUR);
		return VK_SUCCESS;
	}

	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_BLOOM_HBLUR]);

	// apply horizontal blur from BLOOM_VBLUR -> BLOOM_HBLUR
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines[BLUR]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_blur, 0, LENGTH(desc_sets), desc_sets, 0, 0);

	vkCmdPushConstants(cmd_buf, pipeline_layout_blur, VK_SHADER_STAGE_COMPUTE_BIT,
		0, sizeof(push_constants_hblur), &push_constants_hblur);
	vkCmdDispatch(cmd_buf,
		(extent.width / 4 + 15) / 16,
		(extent.height / 4 + 15) / 16,
		1);

	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_BLOOM_HBLUR]);

	if(cvar_bloom_debug->integer == 2)
	{
		bloom_debug_show_image(cmd_buf, VKPT_IMG_BLOOM_HBLUR);
		return VK_SUCCESS;
	}

	// vertical blur from BLOOM_HBLUR -> BLOOM_VBLUR
	vkCmdPushConstants(cmd_buf, pipeline_layout_blur, VK_SHADER_STAGE_COMPUTE_BIT,
		0, sizeof(push_constants_vblur), &push_constants_vblur);
	vkCmdDispatch(cmd_buf,
		(extent.width / 4 + 15) / 16,
		(extent.height / 4 + 15) / 16,
		1);

	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_BLOOM_VBLUR]);

	if(cvar_bloom_debug->integer == 3)
	{
		bloom_debug_show_image(cmd_buf, VKPT_IMG_BLOOM_VBLUR);
		return VK_SUCCESS;
	}

    bm_apply(cmd_buf, extent);

	// composite bloom into TAA_OUTPUT
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines[COMPOSITE]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_composite, 0, LENGTH(desc_sets), desc_sets, 0, 0);
	vkCmdDispatch(cmd_buf,
		(extent.width + 15) / 16,
		(extent.height + 15) / 16,
		1);

	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_BLOOM_VBLUR]);
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_TAA_OUTPUT]);


	return VK_SUCCESS;
}
