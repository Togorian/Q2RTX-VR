/* Modified 14 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
#include "../../../../baseline/b2-controls-ai293/source/b2_graphics270.h"
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
/* Modified by Togorian with AI assistance, 14 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */


#include "shared/shared.h"
#include "refresh/refresh.h"
#include "client/client.h"
#include "refresh/images.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-003/include/refresh/b2_weapon_ui.h"
#include "b2_comfort304.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-003/include/refresh/b2_wall_return.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-003/include/b2/reference_b_b2_lifecycle.h"

#include <assert.h>

#include "vkpt.h"
#include "shader/global_textures.h"

enum {
	STRETCH_PIC_SDR,
	STRETCH_PIC_HDR,
	STRETCH_PIC_NUM_PIPELINES
};

enum
{
	// Note: those values are ORed together to get the pipeline number
	FINAL_BLIT_FILTERED = 0x1,
	FINAL_BLIT_WARPED = 0x2,
	// ... resulting in 4 different combinations
	FINAL_BLIT_NUM_PIPELINES = 4
};

#define TEXNUM_WHITE (~0)
#define MAX_STRETCH_PICS (1<<14)

static drawStatic_t draw = {
	.scale = 1.0f,
	.alpha_scale = 1.0f
};

static int num_stretch_pics = 0;
typedef struct {
	float x, y, w,   h;
	float s, t, w_s, h_t;
	uint32_t color, tex_handle;
    uint32_t projected, padding;
    float clip[4][4]; // std430: offset48, stride112; matches stretch_pic.vert
} StretchPic_t;
typedef char StretchPicLayoutCheck[(sizeof(StretchPic_t) == 112 && offsetof(StretchPic_t, clip) == 48) ? 1 : -1];

// Not using global UBO b/c it's only filled when a world is drawn, but here we need it all the time
typedef struct {
	float hdr_color_scale;
	float tm_hdr_saturation_scale;
} StretchPic_UBO_t;

typedef struct {
	vec2_t input_dimensions;
} FinalBlitPushConstants_t;

static clipRect_t clip_rect;
static bool clip_enable = false;

static StretchPic_t stretch_pic_queue[MAX_STRETCH_PICS];

static VkPipelineLayout        pipeline_layout_stretch_pic;
static VkPipelineLayout        pipeline_layout_final_blit;
static VkRenderPass            render_pass_stretch_pic;
#ifdef REFERENCE_B_B2
static VkRenderPass b2_hud_pass;
static VkPipeline b2_hud_pipeline;
static VkFramebuffer b2_hud_framebuffer;
static StretchPic_t b2_hud_queue[MAX_STRETCH_PICS];
static int b2_hud_count;
static unsigned b2_hud_pair;
static int b2_selector_first,b2_selector_end;
static bool b2_selector_valid;
static b2w_pose_t b2_selector_plane;
// Padding remains GPU-unused; CPU roles do not change the 112-byte shader ABI.
static unsigned b2_ui_role;
#define B2UI_CINEMATIC 6u
#define B2UI_CINEMATIC_OVERLAY 7u
static int b2_cinematic_width,b2_cinematic_height;
static bool b2_ui_capture, b2_ui_pair_valid;
typedef struct {
    b2_view_t head;
    float half_width, half_height;
    bool valid;
} b2_ui_anchor_t;
static b2_ui_anchor_t b2_console_anchor, b2_help_anchor, b2_menu_anchor;
static unsigned b2_ui_generation;
static float b2_ui_clip[B2_MAX_VIEWS][MAX_STRETCH_PICS][4][4];
#endif
static VkPipeline              pipeline_stretch_pic[STRETCH_PIC_NUM_PIPELINES];
static VkPipeline              pipeline_final_blit[FINAL_BLIT_NUM_PIPELINES];
static VkFramebuffer*          framebuffer_stretch_pic = NULL;
static BufferResource_t        buf_stretch_pic_queue[MAX_FRAMES_IN_FLIGHT];
static BufferResource_t        buf_ubo[MAX_FRAMES_IN_FLIGHT];
static VkDescriptorSetLayout   desc_set_layout_sbo;
static VkDescriptorSetLayout   desc_set_layout_ubo;
static VkDescriptorSetLayout   desc_set_layout_final_blit;
static VkDescriptorPool        desc_pool_sbo;
static VkDescriptorPool        desc_pool_ubo;
static VkDescriptorPool        desc_pool_final_blit;
static VkDescriptorSet         desc_set_sbo[MAX_FRAMES_IN_FLIGHT];
static VkDescriptorSet         desc_set_ubo[MAX_FRAMES_IN_FLIGHT];
static VkDescriptorSet         desc_set_final_blit[MAX_FRAMES_IN_FLIGHT];

#ifdef REFERENCE_B_B2
#include "../../../../baseline/b2-consolidated215/inputs/origin-003/include/refresh/b2_wall_return_spv.h"
typedef struct {
    float projection[4],rotation[12],direction[3],depthQU;
    uint32_t width,height;
} b2_cyan_constants_t;
typedef char B2CyanLayoutCheck[(sizeof(b2_cyan_constants_t)==88 &&
    offsetof(b2_cyan_constants_t,rotation)==16 &&
    offsetof(b2_cyan_constants_t,direction)==64 &&
    offsetof(b2_cyan_constants_t,depthQU)==76 &&
    offsetof(b2_cyan_constants_t,width)==80) ? 1 : -1];
static VkDescriptorSetLayout b2_cyan_set_layout;
static VkDescriptorPool b2_cyan_pool;
static VkDescriptorSet b2_cyan_set;
static VkPipelineLayout b2_cyan_layout;
static VkPipeline b2_cyan_pipeline;
static struct {
    bool valid;
    unsigned eye,pair,generation;
    b2_cyan_constants_t constants;
} b2_cyan;

static void b2_cyan_create(void)
{
    b2_cyan.valid=false;
    VkDescriptorSetLayoutBinding binding={.binding=0,
        .descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,.descriptorCount=1,
        .stageFlags=VK_SHADER_STAGE_COMPUTE_BIT};
    VkDescriptorSetLayoutCreateInfo set_info={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount=1,.pBindings=&binding};
    _VK(vkCreateDescriptorSetLayout(qvk.device,&set_info,NULL,&b2_cyan_set_layout));
    VkDescriptorPoolSize size={VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,1};
    VkDescriptorPoolCreateInfo pool={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets=1,.poolSizeCount=1,.pPoolSizes=&size};
    _VK(vkCreateDescriptorPool(qvk.device,&pool,NULL,&b2_cyan_pool));
    VkDescriptorSetAllocateInfo alloc={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool=b2_cyan_pool,.descriptorSetCount=1,.pSetLayouts=&b2_cyan_set_layout};
    _VK(vkAllocateDescriptorSets(qvk.device,&alloc,&b2_cyan_set));
    // The image view stays fixed for this pipeline lifetime. Never update a
    // descriptor in flight; recreation owns both this set and its image view.
    VkDescriptorImageInfo image={.imageView=qvk.images_views[VKPT_IMG_TAA_OUTPUT],
        .imageLayout=VK_IMAGE_LAYOUT_GENERAL};
    VkWriteDescriptorSet write={.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet=b2_cyan_set,.dstBinding=0,.descriptorCount=1,
        .descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,.pImageInfo=&image};
    vkUpdateDescriptorSets(qvk.device,1,&write,0,NULL);
    VkPushConstantRange range={VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(b2_cyan_constants_t)};
    VkPipelineLayoutCreateInfo layout={.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount=1,.pSetLayouts=&b2_cyan_set_layout,
        .pushConstantRangeCount=1,.pPushConstantRanges=&range};
    _VK(vkCreatePipelineLayout(qvk.device,&layout,NULL,&b2_cyan_layout));
    VkShaderModule module=VK_NULL_HANDLE;
    VkShaderModuleCreateInfo shader={.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize=sizeof(b2_wall_return_spv),.pCode=b2_wall_return_spv};
    _VK(vkCreateShaderModule(qvk.device,&shader,NULL,&module));
    VkComputePipelineCreateInfo pipeline={.sType=VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage={.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage=VK_SHADER_STAGE_COMPUTE_BIT,.module=module,.pName="main"},
        .layout=b2_cyan_layout};
    VkResult result=vkCreateComputePipelines(qvk.device,VK_NULL_HANDLE,1,&pipeline,NULL,&b2_cyan_pipeline);
    vkDestroyShaderModule(qvk.device,module,NULL);
    _VK(result);
}
static void b2_cyan_destroy(void)
{
    b2_cyan.valid=false;
    vkDestroyPipeline(qvk.device,b2_cyan_pipeline,NULL);b2_cyan_pipeline=VK_NULL_HANDLE;
    vkDestroyPipelineLayout(qvk.device,b2_cyan_layout,NULL);b2_cyan_layout=VK_NULL_HANDLE;
    vkDestroyDescriptorPool(qvk.device,b2_cyan_pool,NULL);b2_cyan_pool=VK_NULL_HANDLE;
    b2_cyan_set=VK_NULL_HANDLE;
    vkDestroyDescriptorSetLayout(qvk.device,b2_cyan_set_layout,NULL);b2_cyan_set_layout=VK_NULL_HANDLE;
}
#endif

extern cvar_t* cvar_ui_hdr_nits;
extern cvar_t* cvar_tm_hdr_saturation_scale;

VkExtent2D
vkpt_draw_get_extent(void)
{
	return vkpt_get_present_extent();
}

static inline void enqueue_stretch_pic(
		float x, float y, float w, float h,
		float s1, float t1, float s2, float t2,
		uint32_t color, int tex_handle)
{
	if (draw.alpha_scale == 0.f)
		return;

	if(num_stretch_pics == MAX_STRETCH_PICS) {
		Com_EPrintf("Error: stretch pic queue full!\n");
		assert(0);
		return;
	}
	assert(tex_handle);
	StretchPic_t *sp = stretch_pic_queue + num_stretch_pics++;
    sp->projected=sp->padding=0; memset(sp->clip,0,sizeof(sp->clip));
#ifdef REFERENCE_B_B2
    if (b2_ui_capture) {
        // A fully clipped UI primitive may return below; never replay stale data.
        memset(sp,0,sizeof(*sp)); sp->padding=b2_ui_role;
    }
#endif

	if (clip_enable)
	{
		if (x >= clip_rect.right || x + w <= clip_rect.left || y >= clip_rect.bottom || y + h <= clip_rect.top)
			return;

		if (x < clip_rect.left)
		{
			float dw = clip_rect.left - x;
			s1 += dw / w * (s2 - s1);
			w -= dw;
			x = clip_rect.left;

			if (w <= 0) return;
		}

		if (x + w > clip_rect.right)
		{
			float dw = (x + w) - clip_rect.right;
			s2 -= dw / w * (s2 - s1);
			w -= dw;

			if (w <= 0) return;
		}

		if (y < clip_rect.top)
		{
			float dh = clip_rect.top - y;
			t1 += dh / h * (t2 - t1);
			h -= dh;
			y = clip_rect.top;

			if (h <= 0) return;
		}

		if (y + h > clip_rect.bottom)
		{
			float dh = (y + h) - clip_rect.bottom;
			t2 -= dh / h * (t2 - t1);
			h -= dh;

			if (h <= 0) return;
		}
	}

	float width = r_config.width * draw.scale;
	float height = r_config.height * draw.scale;

	x = 2.0f * x / width - 1.0f;
	y = 2.0f * y / height - 1.0f;

	w = 2.0f * w / width;
	h = 2.0f * h / height;

	sp->x = x;
	sp->y = y;
	sp->w = w;
	sp->h = h;

	sp->s   = s1;
	sp->t   = t1;
	sp->w_s = s2 - s1;
	sp->h_t = t2 - t1;

	if (draw.alpha_scale < 1.f)
	{
		float alpha = (color >> 24) & 0xff;
		alpha *= draw.alpha_scale;
		alpha = max(0.f, min(255.f, alpha));
		color = (color & 0xffffff) | ((int)(alpha) << 24);
	}

	sp->color = color;
	sp->tex_handle = tex_handle;
	if(tex_handle >= 0 && tex_handle < MAX_RIMAGES
	&& !r_images[tex_handle].registration_sequence) {
		sp->tex_handle = TEXNUM_WHITE;
	}
}

static void
create_render_pass(void)
{
	LOG_FUNC();
	VkAttachmentDescription color_attachment = {
		.format         = qvk.surf_format.format,
		.samples        = VK_SAMPLE_COUNT_1_BIT,
		.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
		//.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		//.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
		.initialLayout  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference color_attachment_ref = {
		.attachment = 0, /* index in fragment shader */
		.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments    = &color_attachment_ref,
	};

	VkSubpassDependency dependencies[] = {
		{
			.srcSubpass    = VK_SUBPASS_EXTERNAL,
			.dstSubpass    = 0, /* index for own subpass */
			.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			// Make prior color writes visible to LOAD and its layout transition.
			// This also covers left -> right SBS and the subsequent UI pass.
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
			               | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},
	};

	VkRenderPassCreateInfo render_pass_info = {
		.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments    = &color_attachment,
		.subpassCount    = 1,
		.pSubpasses      = &subpass,
		.dependencyCount = LENGTH(dependencies),
		.pDependencies   = dependencies,
	};

	_VK(vkCreateRenderPass(qvk.device, &render_pass_info, NULL, &render_pass_stretch_pic));
	ATTACH_LABEL_VARIABLE(render_pass_stretch_pic, RENDER_PASS);
#ifdef REFERENCE_B_B2
    if (B2XR_Requested()) {
        color_attachment.format=VK_FORMAT_R16G16B16A16_SFLOAT;
        color_attachment.initialLayout=color_attachment.finalLayout=VK_IMAGE_LAYOUT_GENERAL;
        dependencies[0].srcStageMask=VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        dependencies[0].srcAccessMask=VK_ACCESS_MEMORY_WRITE_BIT;
        _VK(vkCreateRenderPass(qvk.device,&render_pass_info,NULL,&b2_hud_pass));
    }
#endif
}

VkResult
vkpt_draw_initialize()
{
	num_stretch_pics = 0;
	LOG_FUNC();
	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		_VK(buffer_create(buf_stretch_pic_queue + i, sizeof(StretchPic_t) * MAX_STRETCH_PICS, 
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		buffer_attach_name(buf_stretch_pic_queue + i, va("stretch pic queue %d", i));

		_VK(buffer_create(buf_ubo + i, sizeof(StretchPic_UBO_t),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		buffer_attach_name(buf_stretch_pic_queue + i, va("stretch pic ubo %d", i));
	}

	VkDescriptorSetLayoutBinding layout_bindings[] = {
		{
			.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.binding         = 0,
			.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
		},
	};

	VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = LENGTH(layout_bindings),
		.pBindings    = layout_bindings,
	};

	_VK(vkCreateDescriptorSetLayout(qvk.device, &layout_info, NULL, &desc_set_layout_sbo));
	ATTACH_LABEL_VARIABLE(desc_set_layout_sbo, DESCRIPTOR_SET_LAYOUT);

	VkDescriptorPoolSize pool_size = {
		.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = MAX_FRAMES_IN_FLIGHT,
	};

	VkDescriptorPoolCreateInfo pool_info = {
		.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = 1,
		.pPoolSizes    = &pool_size,
		.maxSets       = MAX_FRAMES_IN_FLIGHT,
	};

	_VK(vkCreateDescriptorPool(qvk.device, &pool_info, NULL, &desc_pool_sbo));
	ATTACH_LABEL_VARIABLE(desc_pool_sbo, DESCRIPTOR_POOL);

	VkDescriptorSetLayoutBinding layout_bindings_ubo[] = {
		{
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.binding         = 2,
			.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
	};

	VkDescriptorSetLayoutCreateInfo layout_info_ubo = {
		.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = LENGTH(layout_bindings_ubo),
		.pBindings    = layout_bindings_ubo,
	};

	_VK(vkCreateDescriptorSetLayout(qvk.device, &layout_info_ubo, NULL, &desc_set_layout_ubo));
	ATTACH_LABEL_VARIABLE(desc_set_layout_ubo, DESCRIPTOR_SET_LAYOUT);

	VkDescriptorPoolSize pool_size_ubo = {
		.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = MAX_FRAMES_IN_FLIGHT,
	};

	VkDescriptorPoolCreateInfo pool_info_ubo = {
		.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = 1,
		.pPoolSizes    = &pool_size_ubo,
		.maxSets       = MAX_FRAMES_IN_FLIGHT,
	};

	_VK(vkCreateDescriptorPool(qvk.device, &pool_info_ubo, NULL, &desc_pool_ubo));
	ATTACH_LABEL_VARIABLE(desc_pool_ubo, DESCRIPTOR_POOL);

	VkDescriptorSetLayoutBinding layout_binding_final_blit[] = {
		{
			.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.binding         = 0,
			.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
		{
			.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.binding         = 1,
			.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
	};

	VkDescriptorSetLayoutCreateInfo layout_info_final_blit = {
		.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = LENGTH(layout_binding_final_blit),
		.pBindings    = layout_binding_final_blit,
	};

	_VK(vkCreateDescriptorSetLayout(qvk.device, &layout_info_final_blit, NULL, &desc_set_layout_final_blit));
	ATTACH_LABEL_VARIABLE(desc_set_layout_final_blit, DESCRIPTOR_SET_LAYOUT);

	VkDescriptorPoolSize pool_size_final_blit[] = {
		{
			.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_FRAMES_IN_FLIGHT,
		},
		{
			.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_FRAMES_IN_FLIGHT,
		},
	};

	VkDescriptorPoolCreateInfo pool_info_final_blit = {
		.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = LENGTH(pool_size_final_blit),
		.pPoolSizes    = pool_size_final_blit,
		.maxSets       = MAX_FRAMES_IN_FLIGHT,
	};

	_VK(vkCreateDescriptorPool(qvk.device, &pool_info_final_blit, NULL, &desc_pool_final_blit));
	ATTACH_LABEL_VARIABLE(desc_pool_final_blit, DESCRIPTOR_POOL);

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = desc_pool_sbo,
		.descriptorSetCount = 1,
		.pSetLayouts        = &desc_set_layout_sbo,
	};

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info_ubo = {
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = desc_pool_ubo,
		.descriptorSetCount = 1,
		.pSetLayouts        = &desc_set_layout_ubo,
	};

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info_final_blit = {
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = desc_pool_final_blit,
		.descriptorSetCount = 1,
		.pSetLayouts        = &desc_set_layout_final_blit,
	};

	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		_VK(vkAllocateDescriptorSets(qvk.device, &descriptor_set_alloc_info, desc_set_sbo + i));
		BufferResource_t *sbo = buf_stretch_pic_queue + i;

		VkDescriptorBufferInfo buf_info = {
			.buffer = sbo->buffer,
			.offset = 0,
			.range  = sizeof(stretch_pic_queue),
		};

		VkWriteDescriptorSet output_buf_write = {
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = desc_set_sbo[i],
			.dstBinding      = 0,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo     = &buf_info,
		};

		vkUpdateDescriptorSets(qvk.device, 1, &output_buf_write, 0, NULL);

		_VK(vkAllocateDescriptorSets(qvk.device, &descriptor_set_alloc_info_ubo, desc_set_ubo + i));
		BufferResource_t *ubo = buf_ubo + i;

		VkDescriptorBufferInfo buf_info_ubo = {
			.buffer = ubo->buffer,
			.offset = 0,
			.range  = sizeof(StretchPic_UBO_t),
		};

		VkWriteDescriptorSet output_buf_write_ubo = {
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = desc_set_ubo[i],
			.dstBinding      = 2,
			.dstArrayElement = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo     = &buf_info_ubo,
		};

		vkUpdateDescriptorSets(qvk.device, 1, &output_buf_write_ubo, 0, NULL);

		_VK(vkAllocateDescriptorSets(qvk.device, &descriptor_set_alloc_info_final_blit, desc_set_final_blit + i));
	}
	return VK_SUCCESS;
}

VkResult
vkpt_draw_destroy()
{
	LOG_FUNC();
	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		buffer_destroy(buf_stretch_pic_queue + i);
		buffer_destroy(buf_ubo + i);
	}
	vkDestroyDescriptorPool(qvk.device, desc_pool_sbo, NULL);
	vkDestroyDescriptorSetLayout(qvk.device, desc_set_layout_sbo, NULL);
	vkDestroyDescriptorPool(qvk.device, desc_pool_ubo, NULL);
	vkDestroyDescriptorSetLayout(qvk.device, desc_set_layout_ubo, NULL);
	vkDestroyDescriptorPool(qvk.device, desc_pool_final_blit, NULL);
	vkDestroyDescriptorSetLayout(qvk.device, desc_set_layout_final_blit, NULL);

	return VK_SUCCESS;
}

void vkpt_draw_destroy_desktop534(void)
{
	if (!framebuffer_stretch_pic) return;
	for(int i = 0; i < qvk.num_swap_chain_images; i++) {
		vkDestroyFramebuffer(qvk.device, framebuffer_stretch_pic[i], NULL);
	}
	free(framebuffer_stretch_pic);
	framebuffer_stretch_pic = NULL;
}

VkResult vkpt_draw_create_desktop534(void)
{
	framebuffer_stretch_pic = calloc(qvk.num_swap_chain_images, sizeof(*framebuffer_stretch_pic));
	if (qvk.num_swap_chain_images && !framebuffer_stretch_pic) return VK_ERROR_OUT_OF_HOST_MEMORY;
	for(int i = 0; i < qvk.num_swap_chain_images; i++) {
		VkImageView attachments[] = {
			qvk.swap_chain_image_views[i]
		};

		VkFramebufferCreateInfo fb_create_info = {
			.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass      = render_pass_stretch_pic,
			.attachmentCount = 1,
			.pAttachments    = attachments,
			.width           = vkpt_draw_get_extent().width,
			.height          = vkpt_draw_get_extent().height,
			.layers          = 1,
		};

        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkResult result = vkCreateFramebuffer(qvk.device, &fb_create_info, NULL, &framebuffer);
        if (result != VK_SUCCESS) { vkpt_draw_destroy_desktop534(); return result; }
        framebuffer_stretch_pic[i] = framebuffer; // Own only a successfully created handle.
		ATTACH_LABEL_VARIABLE(framebuffer_stretch_pic[i], FRAMEBUFFER);
	}

	return VK_SUCCESS;
}

VkResult
vkpt_draw_destroy_pipelines()
{
	LOG_FUNC();
	for(int i = 0; i < STRETCH_PIC_NUM_PIPELINES; i++) {
		vkDestroyPipeline(qvk.device, pipeline_stretch_pic[i], NULL);
	}
	for(int i = 0; i < FINAL_BLIT_NUM_PIPELINES; i++) {
		vkDestroyPipeline(qvk.device, pipeline_final_blit[i], NULL);
	}
#ifdef REFERENCE_B_B2
    b2_cyan_destroy();
    vkDestroyFramebuffer(qvk.device,b2_hud_framebuffer,NULL); b2_hud_framebuffer=VK_NULL_HANDLE;
    vkDestroyPipeline(qvk.device,b2_hud_pipeline,NULL); b2_hud_pipeline=VK_NULL_HANDLE;
    vkDestroyRenderPass(qvk.device,b2_hud_pass,NULL); b2_hud_pass=VK_NULL_HANDLE;
#endif
	vkDestroyPipelineLayout(qvk.device, pipeline_layout_stretch_pic, NULL);
	vkDestroyPipelineLayout(qvk.device, pipeline_layout_final_blit, NULL);
	vkpt_draw_destroy_desktop534();

	vkDestroyRenderPass(qvk.device, render_pass_stretch_pic, NULL);
	
	return VK_SUCCESS;
}

VkResult
vkpt_draw_create_pipelines()
{
	LOG_FUNC();

	create_render_pass();

	assert(desc_set_layout_sbo);
	VkDescriptorSetLayout desc_set_layouts[] = {
		desc_set_layout_sbo, qvk.desc_set_layout_textures, desc_set_layout_ubo
	};
	CREATE_PIPELINE_LAYOUT(qvk.device, &pipeline_layout_stretch_pic, 
		.setLayoutCount = LENGTH(desc_set_layouts),
		.pSetLayouts    = desc_set_layouts
	);

	VkPushConstantRange push_constant_range_final_blit = {
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset     = 0,
		.size       = sizeof(FinalBlitPushConstants_t)
	};

	VkDescriptorSetLayout desc_set_layouts_final_blit[] = {
		qvk.desc_set_layout_ubo, desc_set_layout_final_blit
	};
	CREATE_PIPELINE_LAYOUT(qvk.device, &pipeline_layout_final_blit,
		.setLayoutCount = LENGTH(desc_set_layouts_final_blit),
		.pSetLayouts = desc_set_layouts_final_blit,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range_final_blit
	);

	VkSpecializationMapEntry specEntries[] = {
		{ .constantID = 0, .offset = 0, .size = sizeof(uint32_t) }
	};

	// "HDR display" flag
	uint32_t spec_data[] = {
		0,
		1,
	};

	VkSpecializationInfo specInfo_SDR = {.mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32_t), .pData = &spec_data[0]};
	VkSpecializationInfo specInfo_HDR = {.mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32_t), .pData = &spec_data[1]};

	VkPipelineShaderStageCreateInfo shader_info_SDR[] = {
		SHADER_STAGE(QVK_MOD_STRETCH_PIC_VERT, VK_SHADER_STAGE_VERTEX_BIT),
		SHADER_STAGE_SPEC(QVK_MOD_STRETCH_PIC_FRAG, VK_SHADER_STAGE_FRAGMENT_BIT, &specInfo_SDR)
	};

	VkPipelineShaderStageCreateInfo shader_info_HDR[] = {
		SHADER_STAGE(QVK_MOD_STRETCH_PIC_VERT, VK_SHADER_STAGE_VERTEX_BIT),
		SHADER_STAGE_SPEC(QVK_MOD_STRETCH_PIC_FRAG, VK_SHADER_STAGE_FRAGMENT_BIT, &specInfo_HDR)
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount   = 0,
		.pVertexBindingDescriptions      = NULL,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions    = NULL,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkViewport viewport = {
		.x        = 0.0f,
		.y        = 0.0f,
		.width    = (float) vkpt_draw_get_extent().width,
		.height   = (float) vkpt_draw_get_extent().height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = vkpt_draw_get_extent(),
	};

	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports    = &viewport,
		.scissorCount  = 1,
		.pScissors     = &scissor,
	};

	VkPipelineRasterizationStateCreateInfo rasterizer_state = {
		.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable        = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE, /* skip rasterizer */
		.polygonMode             = VK_POLYGON_MODE_FILL,
		.lineWidth               = 1.0f,
		.cullMode                = VK_CULL_MODE_BACK_BIT,
		.frontFace               = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable         = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp          = 0.0f,
		.depthBiasSlopeFactor    = 0.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisample_state = {
		.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable   = VK_FALSE,
		.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading      = 1.0f,
		.pSampleMask           = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable      = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment = {
		.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT
			                 | VK_COLOR_COMPONENT_G_BIT
			                 | VK_COLOR_COMPONENT_B_BIT
			                 | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable         = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp        = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.alphaBlendOp        = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state = {
		.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable   = VK_FALSE,
		.logicOp         = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments    = &color_blend_attachment,
		.blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
	};

	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount          = LENGTH(shader_info_SDR),

		.pVertexInputState   = &vertex_input_info,
		.pInputAssemblyState = &input_assembly_info,
		.pViewportState      = &viewport_state,
		.pRasterizationState = &rasterizer_state,
		.pMultisampleState   = &multisample_state,
		.pDepthStencilState  = NULL,
		.pColorBlendState    = &color_blend_state,
		.pDynamicState       = NULL,
		
		.layout              = pipeline_layout_stretch_pic,
		.renderPass          = render_pass_stretch_pic,
		.subpass             = 0,

		.basePipelineHandle  = VK_NULL_HANDLE,
		.basePipelineIndex   = -1,
	};

	VkDynamicState desktop_dynamic534[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo desktop_dynamic_info534 = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = LENGTH(desktop_dynamic534), .pDynamicStates = desktop_dynamic534
	};
	pipeline_info.pDynamicState = &desktop_dynamic_info534;
	pipeline_info.pStages = shader_info_SDR;
	_VK(vkCreateGraphicsPipelines(qvk.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline_stretch_pic[STRETCH_PIC_SDR]));
	ATTACH_LABEL_VARIABLE(pipeline_stretch_pic[STRETCH_PIC_SDR], PIPELINE);

#ifdef REFERENCE_B_B2
    if (b2_hud_pass) {
        b2_cyan_create();
        VkViewport saved_viewport=viewport; VkRect2D saved_scissor=scissor;
        viewport.width=B2G_Width(); viewport.height=B2G_Height();
        scissor.extent=(VkExtent2D){B2G_Width(),B2G_Height()};
        pipeline_info.renderPass=b2_hud_pass;
        pipeline_info.pDynamicState=NULL; // Fixed headset canvas is unchanged.
        _VK(vkCreateGraphicsPipelines(qvk.device,VK_NULL_HANDLE,1,&pipeline_info,NULL,&b2_hud_pipeline));
        VkFramebufferCreateInfo fb={.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass=b2_hud_pass,.attachmentCount=1,.pAttachments=&qvk.images_views[VKPT_IMG_TAA_OUTPUT],
            .width=B2G_Width(),.height=B2G_Height(),.layers=1};
        _VK(vkCreateFramebuffer(qvk.device,&fb,NULL,&b2_hud_framebuffer));
        pipeline_info.renderPass=render_pass_stretch_pic;
        pipeline_info.pDynamicState=&desktop_dynamic_info534;
        viewport=saved_viewport; scissor=saved_scissor;
    }
#endif
	pipeline_info.pStages = shader_info_HDR;
	_VK(vkCreateGraphicsPipelines(qvk.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline_stretch_pic[STRETCH_PIC_HDR]));
	ATTACH_LABEL_VARIABLE(pipeline_stretch_pic[STRETCH_PIC_HDR], PIPELINE);


	VkSpecializationMapEntry final_blit_spec_entries[] = {
		{ .constantID = 0, .offset = 0, .size = sizeof(uint32_t) },
		{ .constantID = 1, .offset = 4, .size = sizeof(uint32_t) }
	};

	pipeline_info.layout = pipeline_layout_final_blit;
	VkDynamicState final_blit_dynamic[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo final_blit_dynamic_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = LENGTH(final_blit_dynamic),
		.pDynamicStates = final_blit_dynamic,
	};
	pipeline_info.pDynamicState = &final_blit_dynamic_info;
	for (int i = 0; i < FINAL_BLIT_NUM_PIPELINES; i++) {
		uint32_t final_blit_spec_data[2] = {(i & FINAL_BLIT_FILTERED) ? 1 : 0, (i & FINAL_BLIT_WARPED) ? 1 : 0};
		VkSpecializationInfo spec_info_final_blit = {.mapEntryCount = LENGTH(final_blit_spec_entries), .pMapEntries = final_blit_spec_entries, .dataSize = sizeof(uint32_t) * 2, .pData = final_blit_spec_data};

		VkPipelineShaderStageCreateInfo shader_info_final_blit[] = {
			SHADER_STAGE(QVK_MOD_FINAL_BLIT_VERT, VK_SHADER_STAGE_VERTEX_BIT),
			SHADER_STAGE_SPEC(QVK_MOD_FINAL_BLIT_FRAG, VK_SHADER_STAGE_FRAGMENT_BIT, &spec_info_final_blit)
		};

		pipeline_info.pStages = shader_info_final_blit;
		_VK(vkCreateGraphicsPipelines(qvk.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline_final_blit[i]));
		ATTACH_LABEL_VARIABLE(pipeline_final_blit[i], PIPELINE);
	}

	return vkpt_draw_create_desktop534();
}

VkResult
vkpt_draw_clear_stretch_pics()
{
#ifdef REFERENCE_B_B2
    b2_cyan.valid=false;
#endif
	num_stretch_pics = 0;
	return VK_SUCCESS;
}

#ifdef REFERENCE_B_B2

// Material is clipped to the existing integer triangle scanlines.
// Shared source bounds for both original1254-square PNGs; left mirrors only.
void R_B2PageArrow(int x, int y, int w, int h, bool left, int pic)
{
    if (w<=0 || h<=0 || w>4096 || h>4096 || pic<=0 || pic>=MAX_RIMAGES) return;
    image_t *image=IMG_ForHandle(pic);
    if (image->width!=1254 || image->height!=1254) return;
    for (int row=0;row<h;row++) {
        int span=(int)(w*(1.f-fabsf(2.f*(row+.5f)/h-1.f)));
        if (span<=0) continue;
        int px=left?x+w-span:x;
        float extent=(float)span/w;
        float s0=476.f/1254.f,s1=(476.f+375.f*extent)/1254.f;
        float t0=(65.f+1092.f*row/h)/1254.f;
        float t1=(65.f+1092.f*(row+1)/h)/1254.f;
        enqueue_stretch_pic(px,y+row,span,1,0,0,1,1,0xff343434u,TEXNUM_WHITE);
        enqueue_stretch_pic(px,y+row,span,1,left?s1:s0,t0,left?s0:s1,t1,0xffffffffu,pic);
    }
}

static clipRect_t b2_page_saved_clip;
static bool b2_page_saved_enable;
void R_B2PageClip(bool begin, int x, int y, int w, int h)
{
    if (!begin) {
        clip_rect=b2_page_saved_clip;clip_enable=b2_page_saved_enable;return;
    }
    b2_page_saved_clip=clip_rect;b2_page_saved_enable=clip_enable;
    clipRect_t bounds={0};
    bounds.left=x;bounds.top=y;bounds.right=x+w;bounds.bottom=y+h;
    if (clip_enable) {
        bounds.left=max(bounds.left,clip_rect.left);bounds.top=max(bounds.top,clip_rect.top);
        bounds.right=min(bounds.right,clip_rect.right);bounds.bottom=min(bounds.bottom,clip_rect.bottom);
    }
    clip_rect=bounds;clip_enable=true;
}
void R_B2SelectorBegin(const b2w_pose_t *plane)
{
    b2_selector_first=num_stretch_pics;b2_selector_end=num_stretch_pics;
    b2_selector_valid=plane!=NULL;
    if (plane) b2_selector_plane=*plane;
}
void R_B2SelectorEnd(void) { b2_selector_end=num_stretch_pics; }

void R_B2UIBegin(void) { b2_ui_capture=true; b2_ui_role=B2UI_FLAT; }
void R_B2UIEnd(void) { b2_ui_capture=false; b2_ui_role=B2UI_FLAT; }
void R_B2UIRole(unsigned role)
{
    if (b2_ui_capture) b2_ui_role=(role<=B2UI_HELP || role==B2UI_CINEMATIC || role==B2UI_CINEMATIC_OVERLAY)?role:B2UI_FLAT;
}
void R_B2CinematicPicture(int width,int height)
{
    b2_cinematic_width=width;b2_cinematic_height=height;
    R_B2UIRole(B2UI_CINEMATIC);
}

// Normalised rectangle at the console centre plane: width first, bottom fixed.
static bool b2_cinematic_rect(int width,int height,float hw,float hh,float rect[4])
{
    if(width<=0 || height<=0 || !isfinite(hw) || !isfinite(hh) || hw<=0 || hh<=0) return false;
    float h=2.f*(hw/hh)*((float)height/(float)width),w=2.f;
    if(!isfinite(h) || h<=0) return false;
    if(h>2.f) { w*=2.f/h;h=2.f; } // Tall images fit safely, preserving aspect.
    rect[0]=-.5f*w;rect[1]=1.f-h;rect[2]=w;rect[3]=h;return true;
}

void R_B2UIReset(void)
{
    b2_cyan.valid=false;
    b2_ui_capture=false;b2_ui_role=B2UI_FLAT;
    b2_console_anchor.valid=b2_help_anchor.valid=b2_menu_anchor.valid=false;
    b2_ui_pair_valid=false;
}

static void b2_cinematic_overlay_rect(float src[4],const float rect[4],
    int screen_width,int screen_height,int film_width,int film_height)
{
    src[0]=rect[0]+(src[0]+1.f)*.5f*screen_width/film_width*rect[2];
    src[1]=rect[1]+(src[1]+1.f)*.5f*screen_height/film_height*rect[3];
    src[2]*=.5f*screen_width/film_width*rect[2];
    src[3]*=.5f*screen_height/film_height*rect[3];
}

// Earlier project capture UI attributes/conventions, GPL-2.0-or-later.
// Adapted to B2's +forward world basis and existing projection matrices.
// No Earlier project runtime/session/renderer code or world frustum is imported.
static bool b2_ui_anchor(b2_ui_anchor_t *out, const b2_view_t *head,
    const b2_frame_t *frame)
{
    out->valid=false;
    if (!head || !frame || !frame->tracking_valid || !frame->should_render ||
        frame->view_count<2 || frame->view_count>B2_MAX_VIEWS) return false;
    for (unsigned k=0;k<3;k++) {
        if (!isfinite(head->origin[k]) || !isfinite(head->invV[k]) ||
            !isfinite(head->invV[4+k]) || !isfinite(head->invV[8+k])) return false;
    }
    // Invalid pose or eye matrices retire UI anchors before any replay.
    for(unsigned axis=0;axis<3;axis++) {
        float length=0;
        for(unsigned k=0;k<3;k++)length+=head->invV[axis*4+k]*head->invV[axis*4+k];
        if(fabsf(length-1.f)>.01f)return false;
        for(unsigned other=0;other<axis;other++) {
            float dot=0;
            for(unsigned k=0;k<3;k++)dot+=head->invV[axis*4+k]*head->invV[other*4+k];
            if(fabsf(dot)>.01f)return false;
        }
    }
    for(unsigned eye=0;eye<frame->view_count;eye++) {
        const b2_view_t *view=&frame->views[eye];
        for(unsigned k=0;k<16;k++)if(!isfinite(view->V[k])||!isfinite(view->P[k]))return false;
        for(unsigned axis=0;axis<3;axis++) {
            float length=0;
            for(unsigned k=0;k<3;k++)length+=view->V[k*4+axis]*view->V[k*4+axis];
            if(fabsf(length-1.f)>.01f)return false;
        }
    }
    float t[4];
    for (unsigned k=0;k<4;k++) {
        float a=frame->fov_lrud[0][k],b=frame->fov_lrud[1][k];
        if (!isfinite(a)||!isfinite(b)||fabsf(a)>=1.56f||fabsf(b)>=1.56f) return false;
        t[k]=.5f*(tanf(a)+tanf(b));
    }
    if (t[1]<=t[0] || t[2]<=t[3]) return false;
    out->head=*head;
    out->half_width=.5f*(t[1]-t[0]);
    // Fixed B2 canvas; independent of desktop/mirror and world projection.
    out->half_height=.5f*(t[2]-t[3])*(float)B2G_Height()/(float)B2G_Width();
    out->valid=true;return true;
}

// Upright console: freeze horizontal opening heading, not opening pitch/roll.
// Near-vertical heading has no stable yaw: omit console until it is defined.
static bool b2_ui_console_anchor(b2_ui_anchor_t *out, const b2_ui_anchor_t *current)
{
    *out=*current;out->valid=false;
    float x=current->head.invV[8],y=current->head.invV[9];
    float length=sqrtf(x*x+y*y);
    if (!current->valid || !isfinite(length) || length<.01f) return false;
    x/=length;y/=length;
    const float right[3]={y,-x,0},up[3]={0,0,1},forward[3]={x,y,0};
    for(unsigned k=0;k<3;k++) {
        out->head.invV[k]=right[k];out->head.invV[4+k]=up[k];
        out->head.invV[8+k]=forward[k];
    }
    out->valid=true;return true;
}

static bool b2_ui_point(const b2_ui_anchor_t *anchor, unsigned role,
    float x, float y, float units, float world[4])
{
    if (!anchor->valid || !isfinite(x)||!isfinite(y)||!isfinite(units)||units<=0) return false;
    float horizontal,vertical,depth,offset=0;
    if (role==B2UI_SELECTOR || role==B2UI_FLAT || role==B2UI_MENU) {
        depth=1.5f; horizontal=depth*.70020754f*x;
        vertical=-depth*.70020754f*(float)B2G_Height()/(float)B2G_Width()*y;
    } else {
        float distance=role==B2UI_STATUS?.9f:2.5f;
        float scale=role==B2UI_STATUS?1.6f:(role==B2UI_CONSOLE?.5f:.8f);
        horizontal=x*scale*distance*anchor->half_width;
        vertical=-y*scale*distance*anchor->half_height;
        if (role==B2UI_STATUS) {
            // Console-equivalent horizontal cylinder, scaled to centre32/radius64 QU.
            // Keep the accepted centre canvas size and head-up tilt.
            float rx=.65f*x*fminf(anchor->half_width,1.5f);
            float ry=-.65f*y*fminf(anchor->half_height,1.2f);
            const float tilt=.08f;
            float length=sqrtf(1.f+rx*rx+ry*ry);
            if (!isfinite(length) || length<=0) return false;
            float tilted_z=(cosf(tilt)-ry*sinf(tilt))/length;
            // Retain the existing front-facing content guard.
            if (!isfinite(tilted_z) || tilted_z<.34202014f) return false;
            float centre=32.f/units,angle=atan2f(rx,1.f);
            float sn=sinf(angle),cs=cosf(angle);
            float travel=centre*(-cs+sqrtf(4.f-sn*sn));
            if (!isfinite(travel) || travel<=0) return false;
            horizontal=travel*sn;
            vertical=travel*(ry*cosf(tilt)+cs*sinf(tilt));
            depth=travel*(cs*cosf(tilt)-ry*sinf(tilt));
        } else {
            // Cylinder radius5m, centre distance-radius=-2.5m.
            float angle=atan2f(horizontal,distance),sn=sinf(angle),cs=cosf(angle);
            float travel=-2.5f*cs+sqrtf(25.f-6.25f*sn*sn);
            horizontal=travel*sn;vertical*=travel/distance;depth=travel*cs;
            // Lower 10% of the full physical console canvas height at its centre.
            if(role==B2UI_CONSOLE)offset=-.1f*(2.f*scale*distance*anchor->half_height);
        }
    }
    for(unsigned k=0;k<3;k++) {
        world[k]=anchor->head.origin[k]+units*(horizontal*anchor->head.invV[k]+
            (vertical+offset)*anchor->head.invV[4+k]+depth*anchor->head.invV[8+k]);
        if(!isfinite(world[k]))return false;
    }
    world[3]=1;return true;
}

static bool b2_ui_project(const b2_view_t *view, const float world[4], float units, float clip[4])
{
    float camera[4];
    for(unsigned r=0;r<4;r++) {
        camera[r]=0;
        for(unsigned c=0;c<4;c++)camera[r]+=view->V[c*4+r]*world[c];
        if(!isfinite(camera[r]))return false;
    }
    if(camera[2]<=.01f*units)return false;
    for(unsigned r=0;r<4;r++) {
        clip[r]=0;
        for(unsigned c=0;c<4;c++)clip[r]+=view->P[c*4+r]*camera[c];
        if(!isfinite(clip[r]))return false;
    }
    if(clip[3]<=.01f*units)return false;
    clip[2]=0;return true; // Existing overlay depth; keep homogeneous XY/W.
}

static void b2_ui_segments(const StretchPic_t *sp, unsigned role, int available, int *nx, int *ny)
{
    *nx=*ny=1;
    if(role!=B2UI_STATUS && role!=B2UI_CONSOLE && role!=B2UI_HELP)return;
    // At most16 per axis, .125 normalised units BEFORE canvas scale.
    *nx=(int)ceilf(fminf(16.f,fabsf(sp->w)*8.f)); if(*nx<1)*nx=1;
    if(role==B2UI_STATUS){*ny=(int)ceilf(fminf(16.f,fabsf(sp->h)*8.f));if(*ny<1)*ny=1;}
    // Reserve one slot for every remaining input before expanding this one.
    while((*nx)*(*ny)>available){if(*nx>=*ny && *nx>1)(*nx)--;else if(*ny>1)(*ny)--;else break;}
}

static bool b2_ui_build(const b2_frame_t *frame, const b2_view_t *head, float units)
{
    b2_ui_anchor_t current;
    b2_hud_count=0;b2_ui_pair_valid=false;
    if(num_stretch_pics<0 || num_stretch_pics>MAX_STRETCH_PICS ||
        !isfinite(units)||units<=0 || !b2_ui_anchor(&current,head,frame)) {R_B2UIReset();return false;}
    if(frame->history_generation!=b2_ui_generation){R_B2UIReset();b2_ui_generation=frame->history_generation;}
    bool console=false,help=false,menu=false;
    for(int i=0;i<num_stretch_pics;i++){
        console|=(stretch_pic_queue[i].padding==B2UI_CONSOLE || stretch_pic_queue[i].padding==B2UI_CINEMATIC) && stretch_pic_queue[i].w!=0 && stretch_pic_queue[i].h!=0;
        menu|=stretch_pic_queue[i].padding==B2UI_MENU && stretch_pic_queue[i].w!=0 && stretch_pic_queue[i].h!=0;
        help|=stretch_pic_queue[i].padding==B2UI_HELP && stretch_pic_queue[i].w!=0 && stretch_pic_queue[i].h!=0;
    }
    if(!console)b2_console_anchor.valid=false;
    else if(!b2_console_anchor.valid)b2_ui_console_anchor(&b2_console_anchor,&current);
    if(!help)b2_help_anchor.valid=false;else if(!b2_help_anchor.valid)b2_help_anchor=current;
    if(!menu)b2_menu_anchor.valid=false;
    else if(!b2_menu_anchor.valid)b2_ui_console_anchor(&b2_menu_anchor,&current);
    const float corner[4][2]={{0,1},{0,0},{1,1},{1,0}};
    for(int i=0;i<num_stretch_pics;i++){
        StretchPic_t src=stretch_pic_queue[i];unsigned role=src.padding;
        if(!isfinite(src.x)||!isfinite(src.y)||!isfinite(src.w)||!isfinite(src.h)||
            !isfinite(src.s)||!isfinite(src.t)||!isfinite(src.w_s)||!isfinite(src.h_t)||src.w==0||src.h==0)continue;
        if(role==B2UI_CINEMATIC || role==B2UI_CINEMATIC_OVERLAY) {
            float rect[4];
            if(!b2_cinematic_rect(b2_cinematic_width,b2_cinematic_height,
                b2_console_anchor.half_width,b2_console_anchor.half_height,rect)) continue;
            if(role==B2UI_CINEMATIC) {
                src.x=rect[0];src.y=rect[1];src.w=rect[2];src.h=rect[3];
            } else {
                float overlay[4]={src.x,src.y,src.w,src.h};
                b2_cinematic_overlay_rect(overlay,rect,r_config.width,r_config.height,
                    b2_cinematic_width,b2_cinematic_height);
                src.x=overlay[0];src.y=overlay[1];src.w=overlay[2];src.h=overlay[3];
            }
            role=B2UI_CONSOLE;
        }
        b2_ui_anchor_t anchor=current;
        if(b2_selector_valid && i>=b2_selector_first && i<b2_selector_end){
            role=B2UI_SELECTOR;
            for(unsigned k=0;k<3;k++){
                anchor.head.origin[k]=b2_selector_plane.origin[k];
                anchor.head.invV[k]=b2_selector_plane.right[k];
                anchor.head.invV[4+k]=b2_selector_plane.up[k];
                anchor.head.invV[8+k]=b2_selector_plane.forward[k];
            }
        }else if(role==B2UI_MENU) {
            // Actual stock draw list, suspended at the opening pose. Reuse
            // console curvature/distance and tessellation, not a head HUD.
            anchor=b2_menu_anchor;role=B2UI_CONSOLE;
        }else if(role==B2UI_CONSOLE)anchor=b2_console_anchor;
        else if(role==B2UI_HELP)anchor=b2_help_anchor;
        int nx,ny,start=b2_hud_count;
        b2_ui_segments(&src,role,MAX_STRETCH_PICS-start-(num_stretch_pics-i-1),&nx,&ny);
        bool valid=true;
        for(int row=0;row<ny && valid;row++)for(int col=0;col<nx && valid;col++){
            int n=start+row*nx+col;StretchPic_t *dst=&b2_hud_queue[n];*dst=src;dst->padding=role;dst->projected=1;
            float x0=(float)col/nx,x1=(float)(col+1)/nx,y0=(float)row/ny,y1=(float)(row+1)/ny;
            dst->x=src.x+src.w*x0;dst->y=src.y+src.h*y0;
            dst->w=src.w*(x1-x0);dst->h=src.h*(y1-y0);
            dst->s=src.s+src.w_s*x0;dst->t=src.t+src.h_t*y0;
            dst->w_s=src.w_s*(x1-x0);dst->h_t=src.h_t*(y1-y0);
            for(unsigned c=0;c<4 && valid;c++){
                float world[4];valid=b2_ui_point(&anchor,role,dst->x+corner[c][0]*dst->w,dst->y+corner[c][1]*dst->h,units,world);
                for(unsigned e=0;e<frame->view_count && valid;e++)valid=b2_ui_project(&frame->views[e],world,units,b2_ui_clip[e][n][c]);
            }
        }
        // Whole source primitive rejected for ALL views if any corner is invalid.
        if(valid)b2_hud_count+=nx*ny;
    }
    b2_hud_pair=frame->logical_index;b2_ui_pair_valid=true;return true;
}

void R_B2Comfort(unsigned eye,float strength)
{
    const refdef_t *fd=vkpt_refdef.fd;
    if(!fd || !fd->b2_active || !fd->b2_frame || eye!=fd->b2_eye ||
       eye>=fd->b2_frame->view_count || strength<=0)return;
    const b2_frame_t *f=fd->b2_frame;unsigned physical=B2_PhysicalEye(eye);
    // Reserve the complete grid before adding anything; preserve existing UI.
    if(num_stretch_pics>MAX_STRETCH_PICS-B2TP_MASK_GRID*B2TP_MASK_GRID)return;
    const float radius=Cvar_Get("b2_comfort_radius304","0.25",CVAR_ARCHIVE)->value;
    for(unsigned row=0;row<B2TP_MASK_GRID;row++)for(unsigned col=0;col<B2TP_MASK_GRID;col++){
        float clip[4][4],alpha;
        if(!B2TP_ComfortQuad(f->fov_lrud[physical],f->views[physical].invV,
            f->views[eye].V,f->views[eye].P,row,col,strength,radius,physical,clip,&alpha))continue;
        StretchPic_t *p=&stretch_pic_queue[num_stretch_pics++];memset(p,0,sizeof(*p));
        p->projected=1;p->tex_handle=TEXNUM_WHITE;
        p->color=(uint32_t)lroundf(alpha*255.f)<<24;
        memcpy(p->clip,clip,sizeof(clip));
    }
}


void R_B2WallReturn(unsigned eye,const float direction[3],float depthQU)
{
    b2_cyan.valid=false;
    const refdef_t *fd=vkpt_refdef.fd;
    if(!fd || !fd->b2_active || !fd->b2_render_accepted || !fd->b2_frame ||
       eye!=fd->b2_eye || eye>=fd->b2_frame->view_count || eye>=B2_MAX_VIEWS ||
       !fd->b2_frame->tracking_valid || !isfinite(depthQU) || depthQU<=0.f)return;
    const b2_frame_t *f=fd->b2_frame;
    const b2_view_t *v=&f->views[eye];
    b2_cyan_constants_t c={0};
    c.projection[0]=v->P[0];c.projection[1]=v->P[5];
    c.projection[2]=v->P[8];c.projection[3]=v->P[9];
    for(unsigned i=0;i<4;i++)if(!isfinite(c.projection[i]))return;
    if(c.projection[0]==0.f || c.projection[1]==0.f)return;
    float n=0;
    for(unsigned i=0;i<3;i++){
        if(!isfinite(direction[i]))return;
        n+=direction[i]*direction[i];c.direction[i]=direction[i];
        for(unsigned j=0;j<3;j++){
            if(!isfinite(v->invV[4*j+i]))return;
            c.rotation[4*j+i]=v->invV[4*j+i];
        }
    }
    if(!isfinite(n)||n<1e-12f)return;
    c.depthQU=depthQU;c.width=qvk.extent_taa_output.width;c.height=qvk.extent_taa_output.height;
    if(c.width!=B2G_Width()||c.height!=B2G_Height())return;
    b2_cyan.constants=c;b2_cyan.eye=eye;b2_cyan.pair=f->logical_index;
    b2_cyan.generation=f->history_generation;b2_cyan.valid=true;
}

void vkpt_b2_wall_return(VkCommandBuffer cmd_buf)
{
    bool valid=b2_cyan.valid;b2_cyan.valid=false;
    const refdef_t *fd=vkpt_refdef.fd;
    if(!valid || !fd || !fd->b2_active || !fd->b2_render_accepted || !fd->b2_frame ||
       !fd->b2_frame->tracking_valid || b2_cyan.eye!=fd->b2_eye ||
       b2_cyan.pair!=fd->b2_frame->logical_index ||
       b2_cyan.generation!=fd->b2_frame->history_generation ||
       b2_cyan.constants.width!=qvk.extent_taa_output.width ||
       b2_cyan.constants.height!=qvk.extent_taa_output.height)return;
    if(!b2_cyan_pipeline || !b2_cyan_set)Com_Error(ERR_FATAL,"B2_FAIL: Cyan resources unavailable");
    VkImageMemoryBarrier barrier={.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask=VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT,
        .oldLayout=VK_IMAGE_LAYOUT_GENERAL,.newLayout=VK_IMAGE_LAYOUT_GENERAL,
        .srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,
        .image=qvk.images[VKPT_IMG_TAA_OUTPUT],.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
    vkCmdPipelineBarrier(cmd_buf,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,0,NULL,0,NULL,1,&barrier);
    vkCmdBindPipeline(cmd_buf,VK_PIPELINE_BIND_POINT_COMPUTE,b2_cyan_pipeline);
    vkCmdBindDescriptorSets(cmd_buf,VK_PIPELINE_BIND_POINT_COMPUTE,b2_cyan_layout,0,1,&b2_cyan_set,0,NULL);
    vkCmdPushConstants(cmd_buf,b2_cyan_layout,VK_SHADER_STAGE_COMPUTE_BIT,0,
        sizeof(b2_cyan.constants),&b2_cyan.constants);
    vkCmdDispatch(cmd_buf,(b2_cyan.constants.width+7)/8,(b2_cyan.constants.height+7)/8,1);
    barrier.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(cmd_buf,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,0,NULL,0,NULL,1,&barrier);
}

void R_B2HUD(unsigned eye,const b2_view_t *head)
{
    const refdef_t *fd=vkpt_refdef.fd;
    if(!fd || !fd->b2_active || !fd->b2_frame || eye>=fd->b2_frame->view_count || eye!=fd->b2_eye)
        Com_Error(ERR_FATAL,"B2 HUD outside active eye");
    if(!eye)b2_ui_build(fd->b2_frame,head,Cvar_Get("b2_units_per_metre","39.37007874",CVAR_NOARCHIVE)->value);
    if(!b2_ui_pair_valid){num_stretch_pics=0;return;}
    if(b2_hud_pair!=fd->b2_frame->logical_index)Com_Error(ERR_FATAL,"B2 HUD capture identity mismatch");
    num_stretch_pics=b2_hud_count;
    memcpy(stretch_pic_queue,b2_hud_queue,sizeof(StretchPic_t)*num_stretch_pics);
    for(int i=0;i<num_stretch_pics;i++)memcpy(stretch_pic_queue[i].clip,b2_ui_clip[eye][i],sizeof(stretch_pic_queue[i].clip));
    uint32_t hash=2166136261u;const unsigned char *p=(const unsigned char*)b2_hud_queue;
    for(size_t i=0;i<sizeof(StretchPic_t)*num_stretch_pics;i++)hash=(hash^p[i])*16777619u;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2HUD pair=%u eye=%u count=%d hash=%08x capture_once=1\n",b2_hud_pair,eye,num_stretch_pics,hash);
}
#endif

VkResult
vkpt_draw_submit_stretch_pics(VkCommandBuffer cmd_buf)
{
	if (num_stretch_pics == 0)
		return VK_SUCCESS;

	BufferResource_t *buf_spq = buf_stretch_pic_queue + qvk.current_frame_index;
	StretchPic_t *spq_dev = (StretchPic_t *) buffer_map(buf_spq);
	memcpy(spq_dev, stretch_pic_queue, sizeof(StretchPic_t) * num_stretch_pics);
	buffer_unmap(buf_spq);
	spq_dev = NULL;

	BufferResource_t *ubo_res = buf_ubo + qvk.current_frame_index;
	StretchPic_UBO_t *ubo = (StretchPic_UBO_t *) buffer_map(ubo_res);
	ubo->hdr_color_scale = cvar_ui_hdr_nits->value * 0.0125;
	ubo->tm_hdr_saturation_scale = cvar_tm_hdr_saturation_scale->value;
	buffer_unmap(ubo_res);
	ubo = NULL;

	VkRenderPassBeginInfo render_pass_info = {
		.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass        = render_pass_stretch_pic,
		.framebuffer       =
#ifdef REFERENCE_B_B2
            (vkpt_refdef.fd && vkpt_refdef.fd->b2_active) ? b2_hud_framebuffer :
#endif
            framebuffer_stretch_pic[qvk.current_swap_chain_image_index],
		.renderArea.offset = { 0, 0 },
		.renderArea.extent = vkpt_draw_get_extent()
	};

	VkDescriptorSet desc_sets[] = {
		desc_set_sbo[qvk.current_frame_index],
		qvk_get_current_desc_set_textures(),
		desc_set_ubo[qvk.current_frame_index],
	};

#ifdef REFERENCE_B_B2
    bool b2_eye=vkpt_refdef.fd && vkpt_refdef.fd->b2_active;
    if (b2_eye) {
        if(!b2_hud_pass || !b2_hud_framebuffer || !b2_hud_pipeline) Com_Error(ERR_FATAL,"B2_FAIL: HUD resources unavailable");
        render_pass_info.renderPass=b2_hud_pass;
        render_pass_info.framebuffer=b2_hud_framebuffer;
        render_pass_info.renderArea.extent=(VkExtent2D){B2G_Width(),B2G_Height()};
    }
#endif
	vkCmdBeginRenderPass(cmd_buf, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_layout_stretch_pic, 0, LENGTH(desc_sets), desc_sets, 0, 0);
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
#ifdef REFERENCE_B_B2
        b2_eye ? b2_hud_pipeline :
#endif
        pipeline_stretch_pic[qvk.surf_is_hdr ? STRETCH_PIC_HDR : STRETCH_PIC_SDR]);
#ifdef REFERENCE_B_B2
    if (!b2_eye)
#endif
    {
        VkExtent2D size = vkpt_draw_get_extent();
        VkViewport viewport = {.width = (float)size.width, .height = (float)size.height, .maxDepth = 1.f};
        VkRect2D scissor = {.offset = {0,0}, .extent = size};
        vkCmdSetViewport(cmd_buf, 0, 1, &viewport);
        vkCmdSetScissor(cmd_buf, 0, 1, &scissor);
    }
	vkCmdDraw(cmd_buf, 4, num_stretch_pics, 0, 0);
	vkCmdEndRenderPass(cmd_buf);

#ifdef REFERENCE_B_B2
    if (b2_eye) {
        VkImageMemoryBarrier barrier={.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_TRANSFER_READ_BIT,
            .oldLayout=VK_IMAGE_LAYOUT_GENERAL,.newLayout=VK_IMAGE_LAYOUT_GENERAL,
            .srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,
            .image=qvk.images[VKPT_IMG_TAA_OUTPUT],.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
        vkCmdPipelineBarrier(cmd_buf,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,0,0,NULL,0,NULL,1,&barrier);
    }
#endif
	num_stretch_pics = 0;
	return VK_SUCCESS;
}

VkResult
vkpt_final_blit(VkCommandBuffer cmd_buf, unsigned int image_index, VkExtent2D extent, bool filtered, bool warped)
{
	VkDescriptorImageInfo img_info_input = {
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
		.imageView   = qvk.images_views[image_index],
		.sampler     = qvk.tex_sampler,
	};
	VkImageView debug_lines_view = vpkt_debugdraw_imageview();
	if (!debug_lines_view)
		debug_lines_view = qvk.images_views[VKPT_IMG_CLEAR];
	VkDescriptorImageInfo img_info_debug_lines = {
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
		.imageView   = debug_lines_view,
		.sampler     = qvk.tex_sampler_nearest,
	};
	VkWriteDescriptorSet elem_images[] = {
		{
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = desc_set_final_blit[qvk.current_frame_index],
			.dstBinding      = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo      = &img_info_input,
		},
		{
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = desc_set_final_blit[qvk.current_frame_index],
			.dstBinding      = 1,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo      = &img_info_debug_lines,
		},
	};

	vkUpdateDescriptorSets(qvk.device, LENGTH(elem_images), elem_images, 0, NULL);

	VkRenderPassBeginInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass_stretch_pic,
		.framebuffer = framebuffer_stretch_pic[qvk.current_swap_chain_image_index],
		.renderArea.offset = { 0, 0 },
		.renderArea.extent = vkpt_draw_get_extent()
	};

	VkDescriptorSet desc_sets[] = {
		qvk.desc_set_ubo,
		desc_set_final_blit[qvk.current_frame_index]
	};

	FinalBlitPushConstants_t push_constants = {.input_dimensions = {extent.width, extent.height}};

	VkExtent2D desktop = vkpt_draw_get_extent();
	VkRect2D scissor = { .offset = {0, 0}, .extent = desktop };
	VkViewport viewport = { .width = (float)desktop.width, .height = (float)desktop.height, .maxDepth = 1.f };
#ifdef REFERENCE_B_B2
	bool sbs = vkpt_refdef.fd && vkpt_refdef.fd->b2_active;
	if (sbs) {
		unsigned eye = vkpt_refdef.fd->b2_eye;
        const b2_frame_t *frame = vkpt_refdef.fd->b2_frame;
        if (!frame || !B2_ValidViewCount(frame->view_count) || eye >= frame->view_count ||
            desktop.width < 2 || desktop.height < (frame->view_count == 4 ? 2u : 1u) || !extent.width || !extent.height)
            Com_Error(ERR_FATAL, "B2_FAIL: invalid desktop mirror rectangle");
        unsigned column = eye & 1u, row = eye >> 1u;
        unsigned rows = frame->view_count / 2u;
        scissor.offset.x = column ? desktop.width / 2 : 0;
        scissor.extent.width = column ? desktop.width - desktop.width / 2 : desktop.width / 2;
        scissor.offset.y = row ? desktop.height / rows : 0;
        scissor.extent.height = row ? desktop.height - desktop.height / rows : desktop.height / rows;
        float scale = min((float)scissor.extent.width / extent.width, (float)scissor.extent.height / extent.height);
        viewport.width = min((float)scissor.extent.width, extent.width * scale);
        viewport.height = min((float)scissor.extent.height, extent.height * scale);
        viewport.x = scissor.offset.x + (scissor.extent.width - viewport.width) * .5f;
        viewport.y = scissor.offset.y + (scissor.extent.height - viewport.height) * .5f;
	}
#endif
	vkCmdBeginRenderPass(cmd_buf, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
#ifdef REFERENCE_B_B2
	if (sbs && vkpt_refdef.fd->b2_eye == 0) {
		VkClearAttachment clear = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.colorAttachment = 0, .clearValue.color.float32 = {0.f, 0.f, 0.f, 1.f} };
		VkClearRect rect = { .rect = {{0, 0}, desktop}, .layerCount = 1 };
		vkCmdClearAttachments(cmd_buf, 1, &clear, 1, &rect);
	}
#endif
	vkCmdSetViewport(cmd_buf, 0, 1, &viewport);
	vkCmdSetScissor(cmd_buf, 0, 1, &scissor);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout_final_blit, 0, LENGTH(desc_sets), desc_sets, 0, 0);
	int pipeline_idx = (filtered ? FINAL_BLIT_FILTERED : 0) | (warped ? FINAL_BLIT_WARPED : 0);
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_final_blit[pipeline_idx]);
	vkCmdPushConstants(cmd_buf, pipeline_layout_final_blit,
		VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_constants), &push_constants);
	vkCmdDraw(cmd_buf, 4, 1, 0, 0);
	vkCmdEndRenderPass(cmd_buf);

	return VK_SUCCESS;
}

void R_SetClipRect_RTX(const clipRect_t *clip) 
{ 
	if (clip)
	{
		clip_enable = true;
		clip_rect = *clip;
	}
	else
	{
		clip_enable = false;
	}
}

void
R_ClearColor_RTX(void)
{
	draw.colors[0].u32 = U32_WHITE;
	draw.colors[1].u32 = U32_WHITE;
}

void
R_SetAlpha_RTX(float alpha)
{
    alpha = powf(fabsf(alpha), 0.4545f); // un-sRGB the alpha
	draw.colors[0].u8[3] = draw.colors[1].u8[3] = alpha * 255;
}

void
R_SetAlphaScale_RTX(float alpha)
{
	draw.alpha_scale = alpha;
}

void
R_SetColor_RTX(uint32_t color)
{
	draw.colors[0].u32   = color;
	draw.colors[1].u8[3] = draw.colors[0].u8[3];
}

void
R_LightPoint_RTX(const vec3_t origin, vec3_t light)
{
	VectorSet(light, 1, 1, 1);
}

void
R_SetScale_RTX(float scale)
{
	draw.scale = scale;
}

void
R_DrawStretchPic_RTX(int x, int y, int w, int h, qhandle_t pic)
{
	enqueue_stretch_pic(
		x,    y,    w,    h,
		0.0f, 0.0f, 1.0f, 1.0f,
		draw.colors[0].u32, pic);
}

void
R_DrawPic_RTX(int x, int y, qhandle_t pic)
{
	image_t *image = IMG_ForHandle(pic);
	R_DrawStretchPic(x, y, image->width, image->height, pic);
}

void
R_DrawStretchRaw_RTX(int x, int y, int w, int h)
{
	if(!qvk.raw_image)
		return;
	R_DrawStretchPic(x, y, w, h, qvk.raw_image - r_images);
}

void
R_UpdateRawPic_RTX(int pic_w, int pic_h, const uint32_t *pic)
{
	if(qvk.raw_image)
		R_UnregisterImage(qvk.raw_image - r_images);

	size_t raw_size = pic_w * pic_h * 4;
	byte *raw_data = Z_Malloc(raw_size);
	memcpy(raw_data, pic, raw_size);
	static int raw_id;
	qvk.raw_image = r_images + R_RegisterRawImage(va("**raw[%d]**", raw_id++), pic_w, pic_h, raw_data, IT_SPRITE, IF_SRGB);
}

void
R_DiscardRawPic_RTX(void)
{
	if(qvk.raw_image) {
		R_UnregisterImage(qvk.raw_image - r_images);
		qvk.raw_image = NULL;
	}
}

void R_DrawKeepAspectPic_RTX(int x, int y, int w, int h, qhandle_t pic)
{
    image_t *image = IMG_ForHandle(pic);

    if (image->flags & IF_SCRAP) {
        R_DrawStretchPic_RTX(x, y, w, h, pic);
        return;
    }

    float scale_w = w;
    float scale_h = h * image->aspect;
    float scale = max(scale_w, scale_h);

    float s = (1.0f - scale_w / scale) * 0.5f;
    float t = (1.0f - scale_h / scale) * 0.5f;

    enqueue_stretch_pic(x, y, w, h, s, t, 1.0f - s, 1.0f - t, draw.colors[0].u32, pic);
}

#define DIV64 (1.0f / 64.0f)

void
R_TileClear_RTX(int x, int y, int w, int h, qhandle_t pic)
{
	enqueue_stretch_pic(x, y, w, h,
		x * DIV64, y * DIV64, (x + w) * DIV64, (y + h) * DIV64,
		U32_WHITE, pic);
}

void
R_DrawFill8_RTX(int x, int y, int w, int h, int c)
{
	if(!w || !h)
		return;
	enqueue_stretch_pic(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f,
		d_8to24table[c & 0xff], TEXNUM_WHITE);
}

void
R_DrawFill32_RTX(int x, int y, int w, int h, uint32_t color)
{
	if(!w || !h)
		return;
	enqueue_stretch_pic(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f,
		color, TEXNUM_WHITE);
}

static inline void
draw_char(int x, int y, int flags, int c, qhandle_t font)
{
	if ((c & 127) == 32) {
		return;
	}

	if (flags & UI_ALTCOLOR) {
		c |= 0x80;
	}
	if (flags & UI_XORCOLOR) {
		c ^= 0x80;
	}

	float s = (c & 15) * 0.0625f;
	float t = (c >> 4) * 0.0625f;

	float eps = 1e-5f; /* fixes some ugly artifacts */

	enqueue_stretch_pic(x, y, CHAR_WIDTH, CHAR_HEIGHT,
		s + eps, t + eps, s + 0.0625f - eps, t + 0.0625f - eps,
		draw.colors[c >> 7].u32, font);
}

void
R_DrawChar_RTX(int x, int y, int flags, int c, qhandle_t font)
{
	draw_char(x, y, flags, c & 255, font);
}

int
R_DrawString_RTX(int x, int y, int flags, size_t maxlen, const char *s, qhandle_t font)
{
	while(maxlen-- && *s) {
		byte c = *s++;
		draw_char(x, y, flags, c, font);
		x += CHAR_WIDTH;
	}

	return x;
}

// vim: shiftwidth=4 noexpandtab tabstop=4 cindent
