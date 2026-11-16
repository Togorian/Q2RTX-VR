/* Modified 14 November 2026: added provenance notice for the Q2RTX VR
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
/* Modified by Togorian with AI assistance, 14 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */


// Modified 14 November 2026: one active B2 renderer state definition,
// shared by all selected vkpt.h routes. Legacy non-B2 declarations stay in their headers.
#pragma once

typedef struct QVK_s {
	VkInstance                  instance;
	VkPhysicalDevice            physical_device;
	VkPhysicalDeviceMemoryProperties mem_properties;

	// number of GPUs we're rendering to --- if DG is disabled, this is 1
	int							device_count;
#ifdef VKPT_DEVICE_GROUPS
	VkPhysicalDevice			device_group_physical_devices[VKPT_MAX_GPUS];
#endif

	VkDevice                    device;
	VkQueue                     queue_graphics;
	VkQueue                     queue_transfer;
	int32_t                     queue_idx_graphics;
	int32_t                     queue_idx_transfer;
	VkSurfaceKHR                surface;
	VkSwapchainKHR              swap_chain;
	VkSurfaceFormatKHR          surf_format;
	bool                        surf_is_hdr;
	bool                        surf_vsync;
	VkPresentModeKHR            present_mode;
	VkExtent2D                  extent_screen_images;
	VkExtent2D                  extent_render;
	VkExtent2D                  extent_render_prev;
	VkExtent2D                  extent_unscaled;
	VkExtent2D                  extent_taa_images;
	VkExtent2D                  extent_taa_output;
	uint32_t                    gpu_slice_width;
	uint32_t                    gpu_slice_width_prev;
	uint32_t                    num_swap_chain_images;
	VkImage*                    swap_chain_images;
	VkImageView*                swap_chain_image_views;

	bool                        use_ray_query;
	bool                        enable_validation;
	bool                        supports_fp16;
	bool                        supports_colorspace;
	bool                        supports_debug_lines;
	bool                        supports_smooth_lines;
	bool                        supports_memory_budget;

	cmd_buf_group_t             cmd_buffers_graphics;
	cmd_buf_group_t             cmd_buffers_transfer;
	semaphore_group_t           semaphores[MAX_FRAMES_IN_FLIGHT][VKPT_MAX_GPUS];

	uint32_t                    num_extensions;
	VkExtensionProperties       *extensions;

	uint32_t                    num_layers;
	VkLayerProperties           *layers;

	VkDebugUtilsMessengerEXT    dbg_messenger;

	VkFence                     fences_frame_sync[MAX_FRAMES_IN_FLIGHT];


	int                         draw_width;
	int                         draw_height;
	uint64_t                    frame_counter;

	int                         effective_aa_mode;

	SDL_Window                  *window;
	uint32_t                    num_sdl2_extensions;
	const char                  **sdl2_extensions;

	uint32_t                    current_swap_chain_image_index;
	uint32_t                    current_frame_index;
	// when set, we'll do a WFI before acquire for this many frames
	uint32_t                    wait_for_idle_frames;
	float                       timestampPeriod;
	bool                        frame_menu_mode;

	VkShaderModule              shader_modules[NUM_QVK_SHADER_MODULES];

	VkDescriptorSetLayout       desc_set_layout_ubo;
	VkDescriptorSet             desc_set_ubo;

	VkDescriptorSetLayout       desc_set_layout_textures;
	VkDescriptorSet             desc_set_textures_even;
	VkDescriptorSet             desc_set_textures_odd;
	VkImage                     images      [NUM_VKPT_IMAGES]; // todo: rename to make consistent
	VkImageView                 images_views[NUM_VKPT_IMAGES]; // todo: rename to make consistent
#ifdef REFERENCE_B_B2
	VkDescriptorSet             b2_desc_set_textures_right[2];
	VkImage                     b2_right_images[NUM_VKPT_IMAGES];
	VkImageView                 b2_right_image_views[NUM_VKPT_IMAGES];
	bool                        b2_temporal_activated;
#endif

#ifdef VKPT_DEVICE_GROUPS
	// local per-GPU image bindings for SLI
	VkImage						images_local[NUM_VKPT_IMAGES][VKPT_MAX_GPUS];
	VkImageView					images_views_local[NUM_VKPT_IMAGES][VKPT_MAX_GPUS];
#endif

	VkDescriptorSetLayout       desc_set_layout_vertex_buffer;
	VkDescriptorSet             desc_set_vertex_buffer;

	BufferResource_t            buf_world;
	BufferResource_t            buf_primitive_instanced;
	BufferResource_t            buf_positions_instanced;

	BufferResource_t            buf_light;
	BufferResource_t            buf_light_staging[MAX_FRAMES_IN_FLIGHT];
	BufferResource_t            buf_light_stats[NUM_LIGHT_STATS_BUFFERS];
	BufferResource_t            buf_light_counts_history[LIGHT_COUNT_HISTORY];

	BufferResource_t            buf_iqm_matrices;
	BufferResource_t            buf_iqm_matrices_staging[MAX_FRAMES_IN_FLIGHT];
	float*                      iqm_matrices_shadow;
	float*                      iqm_matrices_prev;

	BufferResource_t            buf_readback;
	BufferResource_t            buf_readback_staging[MAX_FRAMES_IN_FLIGHT];

	BufferResource_t            buf_tonemap;
	BufferResource_t            buf_sun_color;

    VkSampler                   tex_sampler,
                                tex_sampler_nearest,
                                tex_sampler_nearest_mipmap_aniso,
                                tex_sampler_linear_clamp;

	float                       sintab[256];

	VkImage screenshot_image;
	VkDeviceMemory screenshot_image_memory;
	VkDeviceSize screenshot_image_memory_size;

	image_t *raw_image; // "raw" image, for cinematics

#ifdef VKPT_IMAGE_DUMPS
	// host-visible image for dumping FB data through
	VkImage dump_image;
	VkDeviceMemory dump_image_memory;
	VkDeviceSize dump_image_memory_size;
#endif
} QVK_t;
