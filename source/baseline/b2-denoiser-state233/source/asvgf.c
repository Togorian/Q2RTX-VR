/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/*
Copyright (C) 2018 Christoph Schied
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
#include "b2_hack_peripheral_guide_host.h"
#include "b2_denoiser_state.h"

#ifdef REFERENCE_VR_HISTORY
#define VR_HISTORY_PARITY() vkpt_vr_history_parity()
#define VR_HISTORY_IMAGE(_index) vkpt_vr_history_image((_index))
#else
#define VR_HISTORY_PARITY() (qvk.frame_counter & 1)
#define VR_HISTORY_IMAGE(_index) qvk.images[(_index)]
#endif

enum {
	GRADIENT_IMAGE,
	GRADIENT_ATROUS,
	GRADIENT_REPROJECT,
	TEMPORAL,
	ATROUS_LF,
	ATROUS_ITER_0,
	ATROUS_ITER_1,
	ATROUS_ITER_2,
	ATROUS_ITER_3,
	TAAU,
	CHECKERBOARD_INTERLEAVE,
	COMPOSITING,
	ASVGF_NUM_PIPELINES
};

static VkPipeline       pipeline_asvgf[ASVGF_NUM_PIPELINES];
static VkPipelineLayout pipeline_layout_atrous;
static VkPipelineLayout pipeline_layout_general;
static VkPipelineLayout pipeline_layout_taa;
static bool b2ds_pipeline_attempted;
static VkResult b2ds_pipeline_result;

int R_B2DenoiserPipelineResult(bool *attempted)
{
	*attempted = b2ds_pipeline_attempted;
	return (int)b2ds_pipeline_result;
}


VkResult
vkpt_asvgf_initialize()
{
	VkDescriptorSetLayout desc_set_layouts[] = {
		qvk.desc_set_layout_ubo, qvk.desc_set_layout_textures,
		qvk.desc_set_layout_vertex_buffer
	};

	VkPushConstantRange push_constant_range_atrous = {
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset     = 0,
		.size       = sizeof(uint32_t)
	};

	CREATE_PIPELINE_LAYOUT(qvk.device, &pipeline_layout_atrous, 
		.setLayoutCount         = LENGTH(desc_set_layouts),
		.pSetLayouts            = desc_set_layouts,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges    = &push_constant_range_atrous
	);
	ATTACH_LABEL_VARIABLE(pipeline_layout_atrous, PIPELINE_LAYOUT);

	CREATE_PIPELINE_LAYOUT(qvk.device, &pipeline_layout_general, 
		.setLayoutCount         = LENGTH(desc_set_layouts),
		.pSetLayouts            = desc_set_layouts,
	);
	ATTACH_LABEL_VARIABLE(pipeline_layout_general, PIPELINE_LAYOUT);

	CREATE_PIPELINE_LAYOUT(qvk.device, &pipeline_layout_taa, 
		.setLayoutCount         = LENGTH(desc_set_layouts),
		.pSetLayouts            = desc_set_layouts,
	);
	ATTACH_LABEL_VARIABLE(pipeline_layout_taa, PIPELINE_LAYOUT);
	VkResult guide_result = b2_hack_guide_initialise(desc_set_layouts);
	if (guide_result != VK_SUCCESS) return guide_result;
	return VK_SUCCESS;
}

VkResult
vkpt_asvgf_destroy()
{
	vkDestroyPipelineLayout(qvk.device, pipeline_layout_atrous,     NULL);
	vkDestroyPipelineLayout(qvk.device, pipeline_layout_general,   NULL);
	vkDestroyPipelineLayout(qvk.device, pipeline_layout_taa,   NULL);
	b2_hack_guide_destroy();

	return VK_SUCCESS;
}

VkResult
vkpt_asvgf_create_pipelines()
{
	VkSpecializationMapEntry specEntries[] = {
		{ .constantID = 0, .offset = 0, .size = sizeof(uint32_t) }
	};

	uint32_t spec_data[] = { 
		0, 
		1, 
		2, 
		3
	};

	VkSpecializationInfo specInfo[] = {
		{ .mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32_t), .pData = &spec_data[0] },
		{ .mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32_t), .pData = &spec_data[1] },
		{ .mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32_t), .pData = &spec_data[2] },
		{ .mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32_t), .pData = &spec_data[3] },
	};

	VkComputePipelineCreateInfo pipeline_info[ASVGF_NUM_PIPELINES] = {
		[GRADIENT_IMAGE] = {
			.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage  = SHADER_STAGE(QVK_MOD_ASVGF_GRADIENT_IMG_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_general,
		},
		[GRADIENT_ATROUS] = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = SHADER_STAGE(QVK_MOD_ASVGF_GRADIENT_ATROUS_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_atrous,
		},
		[GRADIENT_REPROJECT] = {
			.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage  = SHADER_STAGE(QVK_MOD_ASVGF_GRADIENT_REPROJECT_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = b2_hack_guide_pipeline_layout,
		},
		[TEMPORAL] = {
			.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage  = SHADER_STAGE(QVK_MOD_ASVGF_TEMPORAL_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_general,
		},
		[ATROUS_LF] = {
			.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage  = SHADER_STAGE(QVK_MOD_ASVGF_LF_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_atrous,
		},
		[ATROUS_ITER_0] = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = SHADER_STAGE_SPEC(QVK_MOD_ASVGF_ATROUS_COMP, VK_SHADER_STAGE_COMPUTE_BIT, &specInfo[0]),
			.layout = pipeline_layout_general,
		},
		[ATROUS_ITER_1] = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = SHADER_STAGE_SPEC(QVK_MOD_ASVGF_ATROUS_COMP, VK_SHADER_STAGE_COMPUTE_BIT, &specInfo[1]),
			.layout = pipeline_layout_general,
		},
		[ATROUS_ITER_2] = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = SHADER_STAGE_SPEC(QVK_MOD_ASVGF_ATROUS_COMP, VK_SHADER_STAGE_COMPUTE_BIT, &specInfo[2]),
			.layout = pipeline_layout_general,
		},
		[ATROUS_ITER_3] = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = SHADER_STAGE_SPEC(QVK_MOD_ASVGF_ATROUS_COMP, VK_SHADER_STAGE_COMPUTE_BIT, &specInfo[3]),
			.layout = pipeline_layout_general,
		},
		[CHECKERBOARD_INTERLEAVE] = {
			.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage  = SHADER_STAGE(QVK_MOD_CHECKERBOARD_INTERLEAVE_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_general,
		},
		[TAAU] = {
			.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage  = SHADER_STAGE(QVK_MOD_ASVGF_TAAU_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_taa,
		},
		[COMPOSITING] = {
			.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage  = SHADER_STAGE(QVK_MOD_COMPOSITING_COMP, VK_SHADER_STAGE_COMPUTE_BIT),
			.layout = pipeline_layout_general,
		},
	};

	b2ds_pipeline_result = vkCreateComputePipelines(qvk.device, 0,
		LENGTH(pipeline_info), pipeline_info, 0, pipeline_asvgf);
	b2ds_pipeline_attempted = true;
	if (b2ds_pipeline_result != VK_SUCCESS) {
		// Failed creation can still return some handles. This init entry will
		// remain uninitialised, so release them here before the existing error path.
		for (unsigned i = 0; i < ASVGF_NUM_PIPELINES; ++i) {
			if (pipeline_asvgf[i] != VK_NULL_HANDLE)
				vkDestroyPipeline(qvk.device, pipeline_asvgf[i], NULL);
			pipeline_asvgf[i] = VK_NULL_HANDLE;
		}
	}
	return b2ds_pipeline_result;
}
	
VkResult
vkpt_asvgf_destroy_pipelines()
{
	for(int i = 0; i < ASVGF_NUM_PIPELINES; i++) {
		vkDestroyPipeline(qvk.device, pipeline_asvgf[i], NULL);
		pipeline_asvgf[i] = VK_NULL_HANDLE;
	}
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

#define BARRIER_ASVGF_COMPUTE_WRITE_TO_READ(cmd_buf, img) \
	do { \
		VkImageSubresourceRange subresource_range = { \
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT, \
			.baseMipLevel   = 0, \
			.levelCount     = 1, \
			.baseArrayLayer = 0, \
			.layerCount     = 1 \
		}; \
		IMAGE_BARRIER_STAGES(cmd_buf, \
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, \
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, \
				.image            = img, \
				.subresourceRange = subresource_range, \
				.srcAccessMask    = VK_ACCESS_SHADER_WRITE_BIT, \
				.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT, \
				.oldLayout        = VK_IMAGE_LAYOUT_GENERAL, \
				.newLayout        = VK_IMAGE_LAYOUT_GENERAL, \
		); \
	} while(0)

#define BARRIER_ASVGF_COMPUTE_WRITE_TO_READ_WRITE(cmd_buf, img) \
	do { \
		VkImageSubresourceRange subresource_range = { \
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT, \
			.baseMipLevel   = 0, \
			.levelCount     = 1, \
			.baseArrayLayer = 0, \
			.layerCount     = 1 \
		}; \
		IMAGE_BARRIER_STAGES(cmd_buf, \
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, \
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, \
				.image            = img, \
				.subresourceRange = subresource_range, \
				.srcAccessMask    = VK_ACCESS_SHADER_WRITE_BIT, \
				.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, \
				.oldLayout        = VK_IMAGE_LAYOUT_GENERAL, \
				.newLayout        = VK_IMAGE_LAYOUT_GENERAL, \
		); \
	} while(0)

#define BARRIER_ASVGF_COMPUTE_WRITE_TO_TRACE_READ(cmd_buf, img) \
	do { \
		VkImageSubresourceRange subresource_range = { \
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT, \
			.baseMipLevel   = 0, \
			.levelCount     = 1, \
			.baseArrayLayer = 0, \
			.layerCount     = 1 \
		}; \
		VkPipelineStageFlags consumer_stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | \
			(qvk.use_ray_query ? 0 : VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR); \
		IMAGE_BARRIER_STAGES(cmd_buf, \
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, \
				consumer_stages, \
				.image            = img, \
				.subresourceRange = subresource_range, \
				.srcAccessMask    = VK_ACCESS_SHADER_WRITE_BIT, \
				.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT, \
				.oldLayout        = VK_IMAGE_LAYOUT_GENERAL, \
				.newLayout        = VK_IMAGE_LAYOUT_GENERAL, \
		); \
	} while(0)

VkResult
vkpt_asvgf_gradient_reproject(VkCommandBuffer cmd_buf)
{
	VkDescriptorSet desc_sets[] = {
		qvk.desc_set_ubo,
		qvk_get_current_desc_set_textures(),
		qvk.desc_set_vertex_buffer,
		b2_hack_guide_bind(cmd_buf)
	};

	int current_sample_pos_image = VKPT_IMG_ASVGF_GRAD_SMPL_POS_A + VR_HISTORY_PARITY();


	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[GRADIENT_REPROJECT]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		b2_hack_guide_pipeline_layout, 0, LENGTH(desc_sets), desc_sets, 0, 0);

	uint32_t group_size_pixels = 24; // matches GROUP_SIZE_PIXELS in asvgf_gradient_reproject.comp
	vkCmdDispatch(cmd_buf,
		(qvk.gpu_slice_width + group_size_pixels - 1) / group_size_pixels,
		(qvk.extent_render.height + group_size_pixels - 1) / group_size_pixels,
		1);

	BARRIER_ASVGF_COMPUTE_WRITE_TO_READ_WRITE(cmd_buf,
		qvk.images[VKPT_IMG_ASVGF_GRAD_HF_SPEC_PING]);
	BARRIER_ASVGF_COMPUTE_WRITE_TO_TRACE_READ(cmd_buf,
		VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_RNG_SEED_A + VR_HISTORY_PARITY()));
	BARRIER_ASVGF_COMPUTE_WRITE_TO_TRACE_READ(cmd_buf,
		VR_HISTORY_IMAGE(current_sample_pos_image));
	BARRIER_ASVGF_COMPUTE_WRITE_TO_TRACE_READ(cmd_buf,
		VR_HISTORY_IMAGE(VKPT_IMG_PT_NORMAL_A + VR_HISTORY_PARITY()));
	BARRIER_ASVGF_COMPUTE_WRITE_TO_TRACE_READ(cmd_buf,
		VR_HISTORY_IMAGE(VKPT_IMG_PT_BASE_COLOR_A + VR_HISTORY_PARITY()));
	BARRIER_ASVGF_COMPUTE_WRITE_TO_TRACE_READ(cmd_buf,
		VR_HISTORY_IMAGE(VKPT_IMG_PT_METALLIC_A + VR_HISTORY_PARITY()));
	BARRIER_ASVGF_COMPUTE_WRITE_TO_TRACE_READ(cmd_buf,
		qvk.images[VKPT_IMG_PT_SHADING_POSITION]);
	BARRIER_ASVGF_COMPUTE_WRITE_TO_TRACE_READ(cmd_buf,
		qvk.images[VKPT_IMG_PT_VIEW_DIRECTION]);

	return VK_SUCCESS;
}

VkResult
vkpt_asvgf_filter(VkCommandBuffer cmd_buf, bool enable_lf)
{
	VkDescriptorSet desc_sets[] = {
		qvk.desc_set_ubo,
		qvk_get_current_desc_set_textures(),
		qvk.desc_set_vertex_buffer
	};
	const uint32_t grad_width =
		(qvk.gpu_slice_width + GRAD_DWN - 1) / GRAD_DWN;
	const uint32_t grad_height =
		(qvk.extent_render.height + GRAD_DWN - 1) / GRAD_DWN;
	const uint32_t grad_group_width = (grad_width + 15) / 16;
	const uint32_t grad_group_height = (grad_height + 15) / 16;
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_PT_COLOR_LF_SH]);
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_PT_COLOR_LF_COCG]);
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_PT_COLOR_HF));
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_PT_COLOR_SPEC));

	BEGIN_PERF_MARKER(cmd_buf, PROFILER_ASVGF_RECONSTRUCT_GRADIENT);

	/* create gradient image */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[GRADIENT_IMAGE]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_general, 0, LENGTH(desc_sets), desc_sets, 0, 0);
	vkCmdDispatch(cmd_buf,
			grad_group_width,
			grad_group_height,
			1);

	// XXX BARRIERS!!!
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_GRAD_LF_PING]);
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_GRAD_HF_SPEC_PING]);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[GRADIENT_ATROUS]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_atrous, 0, LENGTH(desc_sets), desc_sets, 0, 0);

	/* reconstruct gradient image */
	const int num_atrous_iterations_gradient = 7;
	for(int i = 0; i < num_atrous_iterations_gradient; i++) {
		uint32_t push_constants[1] = {
			i
		};

		vkCmdPushConstants(cmd_buf, pipeline_layout_atrous,
			VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push_constants), push_constants);

		vkCmdDispatch(cmd_buf,
				grad_group_width,
				grad_group_height,
				1);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_GRAD_LF_PING + !(i & 1)]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_GRAD_HF_SPEC_PING + !(i & 1)]);

	}

	END_PERF_MARKER(cmd_buf, PROFILER_ASVGF_RECONSTRUCT_GRADIENT);
	BEGIN_PERF_MARKER(cmd_buf, PROFILER_ASVGF_TEMPORAL);

	/* temporal accumulation / filtering */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[TEMPORAL]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_general, 0, LENGTH(desc_sets), desc_sets, 0, 0);
	vkCmdDispatch(cmd_buf,
			(qvk.gpu_slice_width + 14) / 15,
			(qvk.extent_render.height + 14) / 15,
			1);


	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_LF_SH]);
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_LF_COCG]);
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_HF]);
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_MOMENTS]);
	BARRIER_ASVGF_COMPUTE_WRITE_TO_READ(cmd_buf,
		qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_SPEC]);
	BARRIER_ASVGF_COMPUTE_WRITE_TO_READ(cmd_buf,
		VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_HIST_COLOR_LF_SH_A + VR_HISTORY_PARITY()));
	BARRIER_ASVGF_COMPUTE_WRITE_TO_READ(cmd_buf,
		VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_HIST_COLOR_LF_COCG_A + VR_HISTORY_PARITY()));
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_FILTERED_SPEC_A + VR_HISTORY_PARITY()));
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_HIST_MOMENTS_HF_A));
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_HIST_MOMENTS_HF_B));

	END_PERF_MARKER(cmd_buf, PROFILER_ASVGF_TEMPORAL);

	BEGIN_PERF_MARKER(cmd_buf, PROFILER_ASVGF_ATROUS);

	/* spatial reconstruction filtering */
	const int num_atrous_iterations = 4;
	for(int i = 0; i < num_atrous_iterations; i++) 
	{
		if (enable_lf)
		{
			uint32_t push_constants[1] = {
				i
			};

			vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[ATROUS_LF]);

			vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeline_layout_atrous, 0, LENGTH(desc_sets), desc_sets, 0, 0);

			vkCmdPushConstants(cmd_buf, pipeline_layout_atrous,
				VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push_constants), push_constants);

			vkCmdDispatch(cmd_buf,
				grad_group_width,
				grad_group_height,
				1);

			if (i == num_atrous_iterations - 1)
			{
				BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_LF_SH]);
				BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_LF_COCG]);
			}
		}

		int specialization = ATROUS_ITER_0 + i;

		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[specialization]);

		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline_layout_general, 0, LENGTH(desc_sets), desc_sets, 0, 0);

		vkCmdDispatch(cmd_buf,
				(qvk.gpu_slice_width + 15) / 16,
				(qvk.extent_render.height + 15) / 16,
				1);

		if (i < num_atrous_iterations - 1) {
			BARRIER_ASVGF_COMPUTE_WRITE_TO_READ(cmd_buf,
				qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_SPEC + !(i & 1)]);
		}

		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_LF_SH]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_LF_COCG]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_HF]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_MOMENTS]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PONG_LF_SH]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PONG_LF_COCG]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PONG_HF]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PONG_MOMENTS]);
		BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_HIST_COLOR_HF));
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_LF_SH + !(i & 1)]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_ATROUS_PING_LF_COCG + !(i & 1)]);
		BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_COLOR]);
	}


	END_PERF_MARKER(cmd_buf, PROFILER_ASVGF_ATROUS);

	return VK_SUCCESS;
}

VkResult
vkpt_compositing(VkCommandBuffer cmd_buf)
{
	VkDescriptorSet desc_sets[] = {
		qvk.desc_set_ubo,
		qvk_get_current_desc_set_textures(),
		qvk.desc_set_vertex_buffer
	};
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_PT_COLOR_LF_SH]);
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_PT_COLOR_LF_COCG]);
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_PT_COLOR_HF));
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_PT_COLOR_SPEC));

	BEGIN_PERF_MARKER(cmd_buf, PROFILER_COMPOSITING);

	/* create gradient image */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[COMPOSITING]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_general, 0, LENGTH(desc_sets), desc_sets, 0, 0);

	vkCmdDispatch(cmd_buf,
		(qvk.gpu_slice_width + 15) / 16,
		(qvk.extent_render.height + 15) / 16,
		1);

	END_PERF_MARKER(cmd_buf, PROFILER_COMPOSITING);

	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_ASVGF_COLOR]);

	return VK_SUCCESS;
}

VkResult 
vkpt_interleave(VkCommandBuffer cmd_buf)
{
	VkDescriptorSet desc_sets[] = {
		qvk.desc_set_ubo,
		qvk_get_current_desc_set_textures(),
		qvk.desc_set_vertex_buffer
	};

#ifdef VKPT_DEVICE_GROUPS
	if (qvk.device_count > 1) {
		BEGIN_PERF_MARKER(cmd_buf, PROFILER_MGPU_TRANSFERS);

		// create full interleaved motion and color buffers on GPU 0
		VkOffset2D offset_left = { 0, 0 };
		VkOffset2D offset_right = { qvk.extent_render.width / 2, 0 };
		VkExtent2D extent = { qvk.extent_render.width / 2, qvk.extent_render.height };

		vkpt_mgpu_image_copy(cmd_buf,
							VKPT_IMG_PT_MOTION,
							VKPT_IMG_PT_MOTION,
							1,
							0,
							offset_left,
							offset_right,
							extent);
		
		vkpt_mgpu_image_copy(cmd_buf,
							VKPT_IMG_ASVGF_COLOR,
							VKPT_IMG_ASVGF_COLOR,
							1,
							0,
							offset_left,
							offset_right,
							extent);

		vkpt_mgpu_global_barrier(cmd_buf);
		
		END_PERF_MARKER(cmd_buf, PROFILER_MGPU_TRANSFERS);
	}
#endif
	
	BEGIN_PERF_MARKER(cmd_buf, PROFILER_INTERLEAVE);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[CHECKERBOARD_INTERLEAVE]);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_general, 0, LENGTH(desc_sets), desc_sets, 0, 0);

	set_current_gpu(cmd_buf, 0);

	// dispatch using the image dimensions, not render dimensions - to clear the unused area with black color
	vkCmdDispatch(cmd_buf,
		(qvk.extent_screen_images.width + 15) / 16,
		(qvk.extent_screen_images.height + 15) / 16,
		1);

	END_PERF_MARKER(cmd_buf, PROFILER_INTERLEAVE);

	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_FLAT_COLOR]);
	BARRIER_COMPUTE(cmd_buf, qvk.images[VKPT_IMG_FLAT_MOTION]);

	return VK_SUCCESS;
}

VkResult vkpt_taa(VkCommandBuffer cmd_buf)
{
	VkDescriptorSet desc_sets[] = {
		qvk.desc_set_ubo,
		qvk_get_current_desc_set_textures(),
		qvk.desc_set_vertex_buffer
	};

	BEGIN_PERF_MARKER(cmd_buf, PROFILER_ASVGF_TAA);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_asvgf[TAAU]);
	
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout_taa, 0, LENGTH(desc_sets), desc_sets, 0, 0);

	VkExtent2D dispatch_size = qvk.extent_taa_output;

	if (dispatch_size.width < qvk.extent_taa_images.width)
		dispatch_size.width += 8;

	if (dispatch_size.height < qvk.extent_taa_images.height)
		dispatch_size.height += 8;

	vkCmdDispatch(cmd_buf,
			(dispatch_size.width + 15) / 16,
			(dispatch_size.height + 15) / 16,
			1);
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_TAA_A));
	BARRIER_COMPUTE(cmd_buf, VR_HISTORY_IMAGE(VKPT_IMG_ASVGF_TAA_B));
	BARRIER_ASVGF_COMPUTE_WRITE_TO_READ(cmd_buf, qvk.images[VKPT_IMG_TAA_OUTPUT]);

	END_PERF_MARKER(cmd_buf, PROFILER_ASVGF_TAA);

	return VK_SUCCESS;
}
