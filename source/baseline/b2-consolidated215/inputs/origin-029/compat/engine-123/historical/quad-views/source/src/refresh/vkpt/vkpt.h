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


#ifndef  __VKPT_H__
#define  __VKPT_H__

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#if !defined(HAVE_M_PI)
#define HAVE_M_PI
#endif // !defined(HAVE_M_PI)

#include "vk_util.h"

#include "shared/shared.h"
#include "common/bsp.h"
#include "common/cmd.h"
#include "common/common.h"
#include "common/cvar.h"
#include "common/files.h"
#include "common/math.h"
#include "client/video.h"
#include "client/client.h"
#include "refresh/refresh.h"
#include "refresh/images.h"
#include "refresh/models.h"
#include "system/hunk.h"

#include "shader/global_ubo.h"
#include "shader/global_textures.h"
#include "shader/vertex_buffer.h"

#define LENGTH(a) ((sizeof (a)) / (sizeof(*(a))))

#define LOG_FUNC_(f) do {} while(0)
//#define LOG_FUNC_(f) Com_Printf("%s\n", f)
#define LOG_FUNC() LOG_FUNC_(__func__)

#ifdef USE_DEBUG
#define _VK(...) \
	do { \
		VkResult _res = __VA_ARGS__; \
		if(_res != VK_SUCCESS) { \
			Com_EPrintf("error %d executing %s! %s:%d\n", _res, # __VA_ARGS__, __FILE__, __LINE__); \
		} \
	} while(0)
#else
#define _VK(...) do { __VA_ARGS__; } while(0)
#endif

/* see main.c to override default file path. By default it will strip away
 * QVK_MOD_, fix the file ending, and convert to lower case */
#define LIST_SHADER_MODULES \
	SHADER_MODULE_DO(QVK_MOD_STRETCH_PIC_VERT)                       \
	SHADER_MODULE_DO(QVK_MOD_STRETCH_PIC_FRAG)                       \
	SHADER_MODULE_DO(QVK_MOD_FINAL_BLIT_FRAG)                        \
	SHADER_MODULE_DO(QVK_MOD_FINAL_BLIT_VERT)                        \
	SHADER_MODULE_DO(QVK_MOD_INSTANCE_GEOMETRY_COMP)                 \
	SHADER_MODULE_DO(QVK_MOD_ANIMATE_MATERIALS_COMP)                 \
	SHADER_MODULE_DO(QVK_MOD_ASVGF_GRADIENT_IMG_COMP)                \
	SHADER_MODULE_DO(QVK_MOD_ASVGF_GRADIENT_ATROUS_COMP)             \
	SHADER_MODULE_DO(QVK_MOD_ASVGF_GRADIENT_REPROJECT_COMP)          \
	SHADER_MODULE_DO(QVK_MOD_ASVGF_ATROUS_COMP)                      \
	SHADER_MODULE_DO(QVK_MOD_ASVGF_LF_COMP)                          \
	SHADER_MODULE_DO(QVK_MOD_ASVGF_TEMPORAL_COMP)                    \
	SHADER_MODULE_DO(QVK_MOD_ASVGF_TAAU_COMP)                        \
	SHADER_MODULE_DO(QVK_MOD_BLOOM_BLUR_COMP)                        \
	SHADER_MODULE_DO(QVK_MOD_BLOOM_COMPOSITE_COMP)                   \
	SHADER_MODULE_DO(QVK_MOD_BLOOM_DOWNSCALE_COMP)                   \
	SHADER_MODULE_DO(QVK_MOD_TONE_MAPPING_HISTOGRAM_COMP)            \
	SHADER_MODULE_DO(QVK_MOD_TONE_MAPPING_CURVE_COMP)                \
	SHADER_MODULE_DO(QVK_MOD_TONE_MAPPING_APPLY_COMP)                \
	SHADER_MODULE_DO(QVK_MOD_PHYSICAL_SKY_COMP)                      \
	SHADER_MODULE_DO(QVK_MOD_PHYSICAL_SKY_SPACE_COMP)                \
	SHADER_MODULE_DO(QVK_MOD_SKY_BUFFER_RESOLVE_COMP)                \
	SHADER_MODULE_DO(QVK_MOD_CHECKERBOARD_INTERLEAVE_COMP)           \
	SHADER_MODULE_DO(QVK_MOD_GOD_RAYS_COMP)                          \
	SHADER_MODULE_DO(QVK_MOD_GOD_RAYS_FILTER_COMP)                   \
	SHADER_MODULE_DO(QVK_MOD_SHADOW_MAP_VERT)                        \
	SHADER_MODULE_DO(QVK_MOD_COMPOSITING_COMP)                       \
	SHADER_MODULE_DO(QVK_MOD_FSR_EASU_FP16_COMP)                     \
	SHADER_MODULE_DO(QVK_MOD_FSR_EASU_FP32_COMP)                     \
	SHADER_MODULE_DO(QVK_MOD_FSR_RCAS_FP16_COMP)                     \
	SHADER_MODULE_DO(QVK_MOD_FSR_RCAS_FP32_COMP)                     \
	SHADER_MODULE_DO(QVK_MOD_NORMALIZE_NORMAL_MAP_COMP)              \
	SHADER_MODULE_DO(QVK_MOD_DEBUG_LINE_FRAG)                        \
	SHADER_MODULE_DO(QVK_MOD_DEBUG_LINE_VERT)                        \

#define LIST_RT_RGEN_SHADER_MODULES \
	SHADER_MODULE_DO(QVK_MOD_PRIMARY_RAYS_RGEN)                      \
	SHADER_MODULE_DO(QVK_MOD_REFLECT_REFRACT_RGEN)                   \
	SHADER_MODULE_DO(QVK_MOD_DIRECT_LIGHTING_RGEN)                   \
	SHADER_MODULE_DO(QVK_MOD_INDIRECT_LIGHTING_RGEN)                 \

#define LIST_RT_PIPELINE_SHADER_MODULES \
	SHADER_MODULE_DO(QVK_MOD_PATH_TRACER_RCHIT)                      \
	SHADER_MODULE_DO(QVK_MOD_PATH_TRACER_MASKED_RAHIT)               \
	SHADER_MODULE_DO(QVK_MOD_PATH_TRACER_PARTICLE_RAHIT)             \
	SHADER_MODULE_DO(QVK_MOD_PATH_TRACER_BEAM_RAHIT)                 \
	SHADER_MODULE_DO(QVK_MOD_PATH_TRACER_BEAM_RINT)                  \
	SHADER_MODULE_DO(QVK_MOD_PATH_TRACER_RMISS)                      \
	SHADER_MODULE_DO(QVK_MOD_PATH_TRACER_EXPLOSION_RAHIT)            \
	SHADER_MODULE_DO(QVK_MOD_PATH_TRACER_SPRITE_RAHIT)               \


#define SHADER_STAGE(_module, _stage) \
	{ \
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, \
		.stage = _stage, \
		.module = qvk.shader_modules[_module], \
		.pName = "main" \
	}

#define SHADER_STAGE_SPEC(_module, _stage, _spec) \
	{ \
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, \
		.stage = _stage, \
		.module = qvk.shader_modules[_module], \
		.pName = "main", \
		.pSpecializationInfo = _spec, \
	}

enum QVK_SHADER_MODULES {
#define SHADER_MODULE_DO(a) a,
	LIST_SHADER_MODULES
	LIST_RT_RGEN_SHADER_MODULES
	LIST_RT_PIPELINE_SHADER_MODULES
#undef SHADER_MODULE_DO
	NUM_QVK_SHADER_MODULES
};

#define MAX_FRAMES_IN_FLIGHT 2

#ifdef REFERENCE_B_B1
#define REFERENCE_B_B1_DESCRIPTOR_ROUTE_SPV \
	"reference_b_private_shader/reference_b_b1_descriptor_route.comp.spv"

#define VKPT_B1_HOST_BUFFER_TOTAL_LIMIT_BYTES (UINT64_C(512) * 1024u * 1024u)
#define VKPT_B1_HOST_BUFFER_MIN_HEAP_RESERVE_BYTES (UINT64_C(1) << 30)

typedef enum vkpt_b1_host_buffer_failure_e {
	VKPT_B1_HOST_BUFFER_FAILURE_NONE = 0,
	VKPT_B1_HOST_BUFFER_FAILURE_INVALID_INPUT,
	VKPT_B1_HOST_BUFFER_FAILURE_CREATE,
	VKPT_B1_HOST_BUFFER_FAILURE_REQUIREMENTS,
	VKPT_B1_HOST_BUFFER_FAILURE_MEMORY_TYPE,
	VKPT_B1_HOST_BUFFER_FAILURE_INVENTORY,
	VKPT_B1_HOST_BUFFER_FAILURE_OVERFLOW,
	VKPT_B1_HOST_BUFFER_FAILURE_TOTAL_LIMIT,
	VKPT_B1_HOST_BUFFER_FAILURE_BUDGET_UNAVAILABLE,
	VKPT_B1_HOST_BUFFER_FAILURE_HEAP_RESERVE,
	VKPT_B1_HOST_BUFFER_FAILURE_ALLOCATION,
	VKPT_B1_HOST_BUFFER_FAILURE_BIND,
	VKPT_B1_HOST_BUFFER_FAILURE_MAP
} vkpt_b1_host_buffer_failure_t;

/*
 * A zero-initialised B1-private host buffer is prepared without allocating
 * memory, admitted as part of one complete batch, and only then allocated.
 * Raw handles and mapped pointers are deliberately excluded from the retained
 * semantic inventory identity.
 */
typedef struct vkpt_b1_host_buffer_s {
	BufferResource_t resource;
	VkDeviceSize logical_bytes;
	VkDeviceSize allocation_bytes;
	VkDeviceSize required_alignment;
	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags memory_property_flags;
	uint32_t purpose;
	uint32_t memory_type_bits;
	uint32_t memory_type_index;
	uint32_t heap_index;
	uint64_t preflight_generation;
	uint64_t allocation_generation;
	VkDeviceSize mapped_logical_bytes;
	VkResult create_result;
	VkResult allocation_result;
	VkResult bind_result;
	VkResult map_result;
	vkpt_b1_host_buffer_failure_t failure;
	bool prepared;
	bool allocated;
	bool mapped;
} vkpt_b1_host_buffer_t;

typedef struct vkpt_b1_host_buffer_batch_s {
	uint32_t buffer_count;
	uint32_t heap_count;
	VkDeviceSize total_logical_bytes;
	VkDeviceSize total_allocation_bytes;
	VkDeviceSize heap_budget[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize heap_usage[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize heap_proposed_bytes[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize heap_required_reserve[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize cumulative_fixed_bytes;
	VkDeviceSize cumulative_base_total_bytes;
	VkDeviceSize cumulative_total_bytes;
	VkDeviceSize cumulative_base_preallocation_usage[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize cumulative_known_base_bytes[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize cumulative_effective_usage[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize cumulative_projected_usage[VK_MAX_MEMORY_HEAPS];
	uint64_t inventory_hash64;
	uint64_t cumulative_inventory_hash64;
	uint64_t preflight_generation;
	vkpt_b1_host_buffer_failure_t failure;
	bool preflight_complete;
	bool cumulative_preflight_complete;
	bool allocated;
} vkpt_b1_host_buffer_batch_t;

enum {
	VKPT_B1_READBACK_DEVICE_ALLOCATION = 0,
	VKPT_B1_READBACK_STAGING_0_ALLOCATION = 1,
	VKPT_B1_READBACK_STAGING_1_ALLOCATION = 2,
	VKPT_B1_READBACK_ALLOCATION_COUNT = 3
};

typedef struct vkpt_b1_readback_allocation_identity_s {
	VkDeviceSize logical_bytes;
	VkDeviceSize allocation_bytes;
	VkDeviceSize required_alignment;
	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags memory_property_flags;
	uint32_t memory_type_bits;
	uint32_t memory_type_index;
	uint32_t heap_index;
	uint64_t preflight_generation;
	uint64_t allocation_generation;
} vkpt_b1_readback_allocation_identity_t;

typedef struct vkpt_b1_readback_base_admission_s {
	vkpt_b1_readback_allocation_identity_t
		allocations[VKPT_B1_READBACK_ALLOCATION_COUNT];
	VkDeviceSize total_logical_bytes;
	VkDeviceSize total_allocation_bytes;
	VkDeviceSize heap_admitted_bytes[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize heap_budget[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize heap_usage[VK_MAX_MEMORY_HEAPS];
	VkDeviceSize heap_required_reserve[VK_MAX_MEMORY_HEAPS];
	uint32_t heap_count;
	uint64_t inventory_hash64;
	uint64_t preflight_generation;
	bool preflight_complete;
	bool allocated;
} vkpt_b1_readback_base_admission_t;

typedef enum vkpt_b1_world_oracle_checkpoint_e {
	VKPT_B1_WORLD_ORACLE_PAIR_ENTRY = 0,
	VKPT_B1_WORLD_ORACLE_AFTER_LEFT = 1,
	VKPT_B1_WORLD_ORACLE_AFTER_RIGHT = 2,
	VKPT_B1_WORLD_ORACLE_CURRENT_STATE = 3
} vkpt_b1_world_oracle_checkpoint_t;

typedef enum vkpt_b1_world_oracle_reset_owner_e {
	VKPT_B1_WORLD_ORACLE_RESET_NONE = 0,
	VKPT_B1_WORLD_ORACLE_RESET_BSP_UPLOAD_BEGIN = 1,
	VKPT_B1_WORLD_ORACLE_RESET_SEED_BEGIN = 2,
	VKPT_B1_WORLD_ORACLE_RESET_BSP_BUFFER_FAILURE = 3,
	VKPT_B1_WORLD_ORACLE_RESET_BSP_MAP_FAILURE = 4,
	VKPT_B1_WORLD_ORACLE_RESET_BSP_PREFLIGHT_FAILURE = 5,
	VKPT_B1_WORLD_ORACLE_RESET_PROCESS_UNSELECTED = 6,
	VKPT_B1_WORLD_ORACLE_RESET_BSP_COMPLETION_FAILURE = 7,
	VKPT_B1_WORLD_ORACLE_RESET_BSP_CLEANUP = 8,
	VKPT_B1_WORLD_ORACLE_RESET_MATERIAL_STAGE_PRECONDITION = 9,
	VKPT_B1_WORLD_ORACLE_RESET_MATERIAL_STAGE_CONTENT = 10,
	VKPT_B1_WORLD_ORACLE_RESET_MATERIAL_UPLOAD_PRECONDITION = 11,
	VKPT_B1_WORLD_ORACLE_RESET_MATERIAL_UPLOAD_PENDING = 12,
	VKPT_B1_WORLD_ORACLE_RESET_MATERIAL_UPLOAD_CONTENT = 13,
	VKPT_B1_WORLD_ORACLE_RESET_VERTEX_BUFFER_DESTROY = 14,
	VKPT_B1_WORLD_ORACLE_RESET_ANIMATION_ADVANCE_FAILURE = 15,
	VKPT_B1_WORLD_ORACLE_RESET_OWNER_COUNT = 16
} vkpt_b1_world_oracle_reset_owner_t;

typedef struct vkpt_b1_world_oracle_seeded_allocation_v1_s {
	uint32_t schema_version;
	uint64_t map_generation;
	uint64_t world_allocation_generation;
	uint64_t oracle_generation;
	uint64_t advance_count;
	uint64_t seed_raw_hash64;
	uint64_t current_raw_hash64;
	uint64_t current_material_hash64;
	uint64_t reset_count;
	uint64_t reset_owner_mask;
	uint32_t last_reset_owner;
	uint32_t active_primitive_count;
	VkDeviceSize active_bytes;
	uint32_t num_cameras;
	uint64_t staged_material_table_generation;
	bool valid;
	bool material_table_staged;
	bool material_upload_recorded;
} vkpt_b1_world_oracle_seeded_allocation_v1_t;

typedef struct vkpt_b1_world_oracle_material_stage_diagnostic_v2_s {
	uint32_t schema_version;
	uint32_t staging_slot;
	uint64_t map_generation;
	uint64_t attempt_count;
	uint64_t success_count;
	uint64_t pending_table_generation;
	uint64_t aggregate_material_table_generation;
	bool observation_valid;
	bool state_witness_process_selected;
	bool world_oracle_lifecycle_process_selected;
	bool stage_attempted;
	bool stage_succeeded;
	bool pending_slot_valid;
} vkpt_b1_world_oracle_material_stage_diagnostic_v2_t;

typedef struct vkpt_b1_world_material_upload_s {
	uint64_t map_generation;
	uint64_t world_allocation_generation;
	uint64_t material_table_generation;
	uint64_t material_upload_generation;
	uint64_t material_table_hash64;
	VkDeviceSize copy_source_offset;
	VkDeviceSize copy_destination_offset;
	VkDeviceSize copy_bytes;
	uint32_t staging_slot;
	uint32_t table_word_count;
	uint32_t registered_material_count;
	bool command_recorded;
} vkpt_b1_world_material_upload_t;

typedef struct vkpt_b1_world_oracle_snapshot_s {
	vkpt_b1_world_oracle_checkpoint_t checkpoint;
	uint64_t map_generation;
	uint64_t world_allocation_generation;
	uint64_t material_table_generation;
	uint64_t material_upload_generation;
	uint64_t material_table_hash64;
	uint64_t oracle_generation;
	uint64_t advance_count;
	uint64_t seed_raw_hash64;
	uint64_t current_raw_hash64;
	uint64_t current_material_hash64;
	uint32_t active_primitive_count;
	VkDeviceSize active_bytes;
	uint32_t num_cameras;
	bool valid;
} vkpt_b1_world_oracle_snapshot_t;
#endif

typedef struct cmd_buf_group_s {
	uint32_t count_per_frame;
	uint32_t used_this_frame;
	VkCommandBuffer* buffers;
	VkCommandPool command_pool;
#ifdef USE_DEBUG
	void** buffer_begin_addrs;
#endif
} cmd_buf_group_t;

typedef struct semaphore_group_s {
	VkSemaphore image_available;
	VkSemaphore render_finished;
	VkSemaphore transfer_finished;
	VkSemaphore trace_finished;
	bool trace_signaled;
} semaphore_group_t;

#ifdef REFERENCE_B_B2
#include "current/renderer-services557/source/qvk_state582.h"
#else
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
#ifdef REFERENCE_B_B1
	bool                        supports_memory_budget;
#endif

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
#ifdef REFERENCE_B_B1
	VkDescriptorSet             b1_desc_set_textures_right[2];
	VkImage                     b1_right_images[NUM_VKPT_IMAGES];
	VkImageView                 b1_right_image_views[NUM_VKPT_IMAGES];
	VkShaderModule              b1_descriptor_route_shader_module;
	bool                        b1_temporal_activated;
#endif
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
#endif

extern QVK_t qvk;

// Desktop drawable/swapchain dimensions; eye resources retain extent_unscaled.
VkExtent2D vkpt_get_present_extent(void);

#define LIST_EXTENSIONS_ACCEL_STRUCT \
	VK_EXTENSION_DO(vkCreateAccelerationStructureKHR) \
	VK_EXTENSION_DO(vkDestroyAccelerationStructureKHR) \
	VK_EXTENSION_DO(vkCmdBuildAccelerationStructuresKHR) \
	VK_EXTENSION_DO(vkCmdCopyAccelerationStructureKHR) \
	VK_EXTENSION_DO(vkGetAccelerationStructureDeviceAddressKHR) \
	VK_EXTENSION_DO(vkCmdWriteAccelerationStructuresPropertiesKHR) \
	VK_EXTENSION_DO(vkGetAccelerationStructureBuildSizesKHR) \
	VK_EXTENSION_DO(vkGetBufferDeviceAddress) \

#define LIST_EXTENSIONS_RAY_PIPELINE \
	VK_EXTENSION_DO(vkCreateRayTracingPipelinesKHR) \
	VK_EXTENSION_DO(vkCmdTraceRaysKHR) \
	VK_EXTENSION_DO(vkGetRayTracingShaderGroupHandlesKHR) \

#define LIST_EXTENSIONS_DEBUG \
	VK_EXTENSION_DO(vkDebugMarkerSetObjectNameEXT) \

#define LIST_EXTENSIONS_INSTANCE \
	VK_EXTENSION_DO(vkCmdBeginDebugUtilsLabelEXT) \
	VK_EXTENSION_DO(vkCmdEndDebugUtilsLabelEXT)

#define VK_EXTENSION_DO(a) extern PFN_##a q##a;
LIST_EXTENSIONS_ACCEL_STRUCT
LIST_EXTENSIONS_RAY_PIPELINE
LIST_EXTENSIONS_DEBUG
LIST_EXTENSIONS_INSTANCE
#undef VK_EXTENSION_DO

#define MAX_SKY_CLUSTERS 1024

typedef mat3 prim_positions_t;

typedef struct
{
	uint8_t* geometry_storage;
	VkAccelerationStructureGeometryKHR* geometries;
	VkAccelerationStructureBuildRangeInfoKHR* build_ranges;
	uint32_t* prim_counts;
	uint32_t* prim_offsets;
	uint32_t num_geometries;
	uint32_t max_geometries;
	VkAccelerationStructureBuildSizesInfoKHR build_sizes;
	VkDeviceSize blas_data_offset;
	VkAccelerationStructureKHR accel;
	VkDeviceAddress blas_device_address;
	VkGeometryInstanceFlagsKHR instance_flags;
	uint32_t instance_mask;
	uint32_t sbt_offset;
} model_geometry_t;

typedef struct {
	BufferResource_t buffer;
	BufferResource_t staging_buffer;
	int registration_sequence;
	model_geometry_t geom_opaque;
	model_geometry_t geom_transparent;
	model_geometry_t geom_masked;
	size_t vertex_data_offset;
	uint32_t total_tris;
	bool is_static;
} model_vbo_t;

typedef struct
{
	model_geometry_t geometry;

	vec3_t center;
	vec3_t aabb_min;
	vec3_t aabb_max;

	int num_light_polys;
	int allocated_light_polys;
	light_poly_t *light_polys;

	bool transparent;
	bool masked;
} bsp_model_t;

typedef struct aabb_s {
	vec3_t mins;
	vec3_t maxs;
} aabb_t;

typedef struct bsp_mesh_s {
	bsp_model_t *models;
	int num_models;

	aabb_t world_aabb;

	VboPrimitive* primitives;
	uint32_t num_primitives_allocated;
	uint32_t num_primitives;
	size_t vertex_data_offset;

	model_geometry_t geom_opaque;
	model_geometry_t geom_transparent;
	model_geometry_t geom_masked;
	model_geometry_t geom_sky;
	model_geometry_t geom_custom_sky;

	int num_clusters;

	int num_cluster_lights;
	int *cluster_light_offsets;
	int *cluster_lights;

	int num_light_polys;
	int allocated_light_polys;
	light_poly_t *light_polys;

	uint32_t sky_clusters[MAX_SKY_CLUSTERS];
	int num_sky_clusters;
	bool all_lava_emissive;

	struct { vec3_t pos; vec3_t dir; } cameras[MAX_CAMERAS];
	int num_cameras;

	byte sky_visibility[VIS_MAX_BYTES];

	aabb_t* cluster_aabbs;
} bsp_mesh_t;

void bsp_mesh_create_from_bsp(bsp_mesh_t *wm, bsp_t *bsp, const char* map_name);
void bsp_mesh_destroy(bsp_mesh_t *wm);
void bsp_mesh_register_textures(bsp_t *bsp);
void bsp_mesh_animate_light_polys(bsp_mesh_t *wm);
uint32_t encode_normal(const vec3_t normal);

typedef struct vkpt_refdef_s {
	QVKUniformBuffer_t uniform_buffer;
	InstanceBuffer uniform_instance_buffer;
	refdef_t *fd;
	float view_matrix[16];
	float view_matrix_inv[16];

	float z_near, z_far;

	bsp_mesh_t bsp_mesh_world;
	int bsp_mesh_world_loaded;

	lightstyle_t prev_lightstyles[MAX_LIGHTSTYLES];
} vkpt_refdef_t;

extern vkpt_refdef_t vkpt_refdef;

extern BufferResource_t buf_accel_scratch;

typedef struct sun_light_s {
	vec3_t direction;
	vec3_t direction_envmap;
	vec3_t color;
	float angular_size_rad;
	bool use_physical_sky;
	bool visible;
} sun_light_t;

void mult_matrix_matrix(mat4_t p, const mat4_t a, const mat4_t b);
void mult_matrix_vector(vec4_t v, const mat4_t a, const vec4_t b);
void create_entity_matrix(mat4_t matrix, entity_t *e);
void create_viewweapon_matrix(mat4_t matrix, entity_t *e);
void create_projection_matrix(mat4_t matrix, float znear, float zfar, float fov_x, float fov_y);
void create_view_matrix(mat4_t matrix, refdef_t *fd);
void inverse(const mat4_t m, mat4_t inv);
void create_orthographic_matrix(mat4_t matrix, float xmin, float xmax,
		float ymin, float ymax, float znear, float zfar);

#define PROFILER_LIST \
	PROFILER_DO(FRAME_TIME,                 0) \
	PROFILER_DO(INSTANCE_GEOMETRY,          1) \
	PROFILER_DO(BVH_UPDATE,                 1) \
	PROFILER_DO(PRIMARY_RAYS,               1) \
	PROFILER_DO(REFLECT_REFRACT_1,          1) \
	PROFILER_DO(REFLECT_REFRACT_2,          1) \
	PROFILER_DO(ASVGF_GRADIENT_REPROJECT,   1) \
	PROFILER_DO(DIRECT_LIGHTING,            1) \
	PROFILER_DO(INDIRECT_LIGHTING,          1) \
	PROFILER_DO(INDIRECT_LIGHTING_0,        2) \
	PROFILER_DO(INDIRECT_LIGHTING_1,        2) \
	PROFILER_DO(ASVGF_FULL,                 1) \
	PROFILER_DO(ASVGF_RECONSTRUCT_GRADIENT, 2) \
	PROFILER_DO(ASVGF_TEMPORAL,             2) \
	PROFILER_DO(ASVGF_ATROUS,               2) \
	PROFILER_DO(MGPU_TRANSFERS,             1) \
	PROFILER_DO(INTERLEAVE,                 1) \
	PROFILER_DO(ASVGF_TAA,                  2) \
	PROFILER_DO(BLOOM,                      1) \
	PROFILER_DO(TONE_MAPPING,               1) \
	PROFILER_DO(FSR,                        1) \
	PROFILER_DO(FSR_EASU,                   2) \
	PROFILER_DO(FSR_RCAS,                   2) \
	PROFILER_DO(UPDATE_ENVIRONMENT,         1) \
	PROFILER_DO(GOD_RAYS,                   1) \
	PROFILER_DO(GOD_RAYS_REFLECT_REFRACT,   1) \
	PROFILER_DO(GOD_RAYS_FILTER,            1) \
	PROFILER_DO(SHADOW_MAP,                 1) \
	PROFILER_DO(COMPOSITING,                1) \

enum {
#define PROFILER_DO(a, ...) PROFILER_##a,
	PROFILER_LIST
#undef PROFILER_DO
	NUM_PROFILER_ENTRIES,
};

#define NUM_PROFILER_QUERIES_PER_FRAME (NUM_PROFILER_ENTRIES * 2)

typedef enum {
	PROFILER_START, 
	PROFILER_STOP,
} VKPTProfilerAction;

typedef struct EntityUploadInfo
{
	uint32_t num_instances;
	uint32_t num_prims;
	uint32_t opaque_prim_count;
	uint32_t transparent_prim_offset;
	uint32_t transparent_prim_count;
	uint32_t masked_prim_offset;
	uint32_t masked_prim_count;
	uint32_t viewer_model_prim_offset;
	uint32_t viewer_model_prim_count;
	uint32_t viewer_weapon_prim_offset;
	uint32_t viewer_weapon_prim_count;
	uint32_t explosions_prim_offset;
	uint32_t explosions_prim_count;
	bool weapon_left_handed;
} EntityUploadInfo;

VkDescriptorSet qvk_get_current_desc_set_textures(void);
#ifdef REFERENCE_B_B1
uint32_t vkpt_b1_current_history_parity(void);
VkImage vkpt_b1_resolve_history_image(int image_index);
#ifdef REFERENCE_B_B2
bool vkpt_b2_temporal_activate(void);
VkDescriptorSet vkpt_b2_view_descriptor(uint32_t view, uint32_t parity);
VkImage vkpt_b2_view_image(uint32_t view, int image_index);
void vkpt_pt_b2_save_instance_base(void);
void vkpt_pt_b2_restore_instance_base(void);
#endif
typedef struct vkpt_b1_tlas_semantic_identity_s {
	uint32_t active_instance_count;
	uint32_t geometry_instance_count;
	uint32_t effects_instance_count;
	uint64_t instance_inventory_hash64;
	uint64_t prim_offset_hash64;
	uint64_t model_index_hash64;
	uint64_t combined_hash64;
} vkpt_b1_tlas_semantic_identity_t;

typedef struct vkpt_b1_temporal_semantic_identity_s {
	uint64_t activation_generation;
	uint64_t allocation_inventory_hash64;
	uint64_t physical_image_set_hash64;
	uint64_t descriptor_route_hash64;
	uint64_t raw_bytes_per_role;
	uint64_t bound_bytes_per_role;
	uint32_t role_count;
	uint32_t images_per_role;
	uint32_t descriptor_set_count;
	uint32_t descriptor_canonical_route_entry_count;
	uint32_t descriptor_shader_route_observation_count;
	bool descriptor_shader_route_witness_complete;
} vkpt_b1_temporal_semantic_identity_t;

bool vkpt_pt_b1_tlas_semantic_identity(
	vkpt_b1_tlas_semantic_identity_t *identity);
bool vkpt_pt_b1_prepare_tlas_semantic_plan(int frame_index,
	const EntityUploadInfo *upload_info, bool weapon_left_handed,
	vkpt_b1_tlas_semantic_identity_t *identity);
bool vkpt_pt_b1_tlas_semantic_plan_consumed(
	vkpt_b1_tlas_semantic_identity_t *identity);
void vkpt_b1_log_path_history_provenance(void);
bool vkpt_b1_temporal_activate(b1_renderer_log_fn log_line, void *opaque,
	bool run_witness);
bool vkpt_b1_temporal_semantic_identity(
	vkpt_b1_temporal_semantic_identity_t *identity);
bool vkpt_b1_temporal_activation_witness_requested(void);
bool vkpt_b1_temporal_activation_attempted(void);
VkResult vkpt_b1_temporal_activation_preallocation_result(void);
bool vkpt_b1_temporal_activation_preallocation_device_lost(void);
bool vkpt_b1_temporal_activation_device_lost_terminal(void);
bool vkpt_b1_temporal_activation_abandons_resources(void);
VkResult vkpt_b1_temporal_activation_terminal_device_result(void);
uint32_t vkpt_b1_temporal_activation_setup_count(void);
VkResult vkpt_b1_temporal_activation_setup_pool_result(uint32_t ordinal);
VkResult vkpt_b1_temporal_activation_setup_allocate_result(uint32_t ordinal);
VkResult vkpt_b1_temporal_activation_setup_fence_result(uint32_t ordinal);
VkResult vkpt_b1_temporal_activation_setup_begin_result(uint32_t ordinal);
bool vkpt_b1_temporal_activation_setup_device_lost(uint32_t ordinal);
uint32_t vkpt_b1_temporal_activation_submission_count(void);
uint32_t vkpt_b1_temporal_activation_expected_submission_count(void);
const char *vkpt_b1_temporal_activation_submission_label(uint32_t ordinal);
uint32_t vkpt_b1_temporal_activation_submission_ordinal(uint32_t ordinal);
VkResult vkpt_b1_temporal_activation_submission_end_result(uint32_t ordinal);
VkResult vkpt_b1_temporal_activation_submission_queue_result(uint32_t ordinal);
VkResult vkpt_b1_temporal_activation_submission_fence_wait_result(
	uint32_t ordinal);
VkResult vkpt_b1_temporal_activation_submission_fence_status_result(
	uint32_t ordinal);
bool vkpt_b1_temporal_activation_submission_device_lost(uint32_t ordinal);
uint32_t vkpt_b1_temporal_activation_submit_call_count(void);
uint32_t vkpt_b1_temporal_activation_successful_submit_count(void);
uint32_t vkpt_b1_temporal_activation_completion_wait_call_count(void);
uint32_t vkpt_b1_temporal_activation_completion_count(void);
uint32_t vkpt_b1_temporal_activation_sentinel_decision_count(void);
bool vkpt_b1_temporal_activation_sentinel_decision(uint32_t ordinal);
uint32_t vkpt_b1_temporal_activation_initial_sentinel_round_count(void);
uint32_t vkpt_b1_temporal_activation_repeated_sentinel_round_count(void);
bool vkpt_b1_temporal_activation_initial_restore_complete(void);
bool vkpt_b1_temporal_activation_tagged_route_complete(void);
bool vkpt_b1_temporal_activation_zero_route_complete(void);
bool vkpt_b1_temporal_activation_final_restore_complete(void);
bool vkpt_b1_temporal_activation_semantic_images_initialised(void);
bool vkpt_b1_temporal_activation_descriptors_complete(void);
bool vkpt_b1_temporal_activation_ready_to_publish(void);
bool vkpt_b1_temporal_activation_published(void);
bool vkpt_b1_temporal_publish_ready(b1_renderer_log_fn log_line, void *opaque);
void vkpt_b1_require_maintenance_open(const char *operation);
#endif

VkResult vkpt_profiler_initialize(void);
VkResult vkpt_profiler_destroy(void);
VkResult vkpt_profiler_query(VkCommandBuffer cmd_buf, int idx, VKPTProfilerAction action);
VkResult vkpt_profiler_next_frame(VkCommandBuffer cmd_buf);
void draw_profiler(int enable_asvgf);
double vkpt_get_profiler_result(int idx);

VkResult vkpt_readback(ReadbackBuffer* dst);

VkResult vkpt_textures_initialize(void);
VkResult vkpt_textures_destroy(void);
VkResult vkpt_textures_end_registration(void);
VkResult vkpt_textures_upload_envmap(int w, int h, byte *data);
void vkpt_textures_destroy_unused(void);
void vkpt_textures_update_descriptor_set(void);
image_t *vkpt_fake_emissive_texture(image_t *image, int bright_threshold_int);
void vkpt_extract_emissive_texture_info(image_t *image);
void vkpt_textures_prefetch(void);
void vkpt_invalidate_texture_descriptors(void);
void vkpt_init_light_textures(void);

VkCommandBuffer vkpt_begin_command_buffer(cmd_buf_group_t* group);
void vkpt_free_command_buffers(cmd_buf_group_t* group);
void vkpt_reset_command_buffers(cmd_buf_group_t* group);
void vkpt_wait_idle(VkQueue queue, cmd_buf_group_t* group);

void vkpt_submit_command_buffer(
	VkCommandBuffer cmd_buf,
	VkQueue queue,
	uint32_t execute_device_mask,
	int wait_semaphore_count,
	VkSemaphore* wait_semaphores,
	VkPipelineStageFlags* wait_stages,
	uint32_t* wait_device_indices,
	int signal_semaphore_count,
	VkSemaphore* signal_semaphores,
	uint32_t* signal_device_indices,
	VkFence fence);

void vkpt_submit_command_buffer_simple(
	VkCommandBuffer cmd_buf,
	VkQueue queue,
	bool all_gpus);


#define ALL_GPUS (-1)
void set_current_gpu(VkCommandBuffer cmd_buf, int gpu_index);

VkResult allocate_gpu_memory(VkMemoryRequirements mem_req, VkDeviceMemory* pMemory);

#ifdef VKPT_DEVICE_GROUPS
void vkpt_mgpu_global_barrier(VkCommandBuffer cmd_buf);

void
vkpt_mgpu_image_copy(VkCommandBuffer cmd_buf,
					int src_image_index,
					int dst_image_index,
					int src_gpu_index,
					int dst_gpu_index,
					VkOffset2D src_offset,
					VkOffset2D dst_offset,
					VkExtent2D size);
#endif

void
vkpt_image_copy(VkCommandBuffer cmd_buf,
					int src_image_index,
					int dst_image_index,
					VkOffset2D src_offset,
					VkOffset2D dst_offset,
					VkExtent2D size);

VkResult vkpt_draw_initialize(void);
VkResult vkpt_draw_destroy(void);
VkResult vkpt_draw_destroy_pipelines(void);
VkResult vkpt_draw_create_pipelines(void);
VkResult vkpt_draw_submit_stretch_pics(VkCommandBuffer cmd_buf);
VkResult vkpt_final_blit(VkCommandBuffer cmd_buf, unsigned int image_index, VkExtent2D extent, bool filtered, bool warped);
VkResult vkpt_draw_clear_stretch_pics(void);

VkResult vkpt_uniform_buffer_create(void);
VkResult vkpt_uniform_buffer_destroy(void);
VkResult vkpt_uniform_buffer_upload_to_staging(void);
void vkpt_uniform_buffer_copy_from_staging(VkCommandBuffer command_buffer);

void vkpt_init_model_geometry(model_geometry_t* info, uint32_t max_geometries);
void vkpt_destroy_model_geometry(model_geometry_t* info);
void vkpt_append_model_geometry(model_geometry_t* info, uint32_t num_prims, uint32_t prim_offset, const char* model_name);
VkResult vkpt_vertex_buffer_create(void);
VkResult vkpt_vertex_buffer_destroy(void);
void vkpt_vertex_buffer_ensure_primbuf_size(uint32_t prim_count);
VkResult vkpt_vertex_buffer_upload_bsp_mesh(bsp_mesh_t* bsp_mesh);
void vkpt_vertex_buffer_cleanup_bsp_mesh(bsp_mesh_t *bsp_mesh);
#ifdef REFERENCE_B_B1
typedef enum vkpt_b1_raw_copy_owner_v1_e {
	VKPT_B1_RAW_COPY_OWNER_UNKNOWN = 0,
	VKPT_B1_RAW_COPY_OWNER_LIGHT = 1,
	VKPT_B1_RAW_COPY_OWNER_IQM = 2,
	VKPT_B1_RAW_COPY_OWNER_DEVICE_UNIFORM = 3,
	VKPT_B1_RAW_COPY_OWNER_STATE_STAGING = 4,
	VKPT_B1_RAW_COPY_OWNER_TRANSPARENCY_VERTEX = 5,
	VKPT_B1_RAW_COPY_OWNER_TRANSPARENCY_BEAM_AABB = 6,
	VKPT_B1_RAW_COPY_OWNER_TRANSPARENCY_PARTICLE_COLOR = 7,
	VKPT_B1_RAW_COPY_OWNER_TRANSPARENCY_BEAM_COLOR = 8,
	VKPT_B1_RAW_COPY_OWNER_TRANSPARENCY_SPRITE_INFO = 9,
	VKPT_B1_RAW_COPY_OWNER_TRANSPARENCY_BEAM_INTERSECT = 10,
	VKPT_B1_RAW_COPY_OWNER_TRANSPARENCY_INDEX = 11,
	VKPT_B1_RAW_COPY_OWNER_READBACK_STAGING_0 = 12,
	VKPT_B1_RAW_COPY_OWNER_READBACK_STAGING_1 = 13,
	VKPT_B1_RAW_COPY_OWNER_COUNT = 14
} vkpt_b1_raw_copy_owner_v1_t;

bool vkpt_b1_uniform_raw_owner_binding_v1(VkBuffer *buffer,
	VkDeviceSize *logical_bytes);
VkResult vkpt_b1_host_buffer_prepare(vkpt_b1_host_buffer_t *buffer,
	uint32_t purpose, VkDeviceSize logical_bytes, VkBufferUsageFlags usage);
VkResult vkpt_b1_host_buffer_batch_preflight(vkpt_b1_host_buffer_batch_t *batch,
	vkpt_b1_host_buffer_t *const *inventory, uint32_t buffer_count);
VkResult vkpt_b1_host_buffer_batch_cumulative_preflight(
	vkpt_b1_host_buffer_batch_t *batch, VkDeviceSize cumulative_fixed_bytes,
	VkDeviceSize cumulative_base_total_bytes,
	const VkDeviceSize cumulative_base_preallocation_usage[VK_MAX_MEMORY_HEAPS],
	const VkDeviceSize cumulative_known_base_bytes[VK_MAX_MEMORY_HEAPS]);
VkResult vkpt_b1_host_buffer_batch_allocate(vkpt_b1_host_buffer_batch_t *batch,
	vkpt_b1_host_buffer_t *const *inventory, uint32_t buffer_count);
void vkpt_b1_host_buffer_destroy(vkpt_b1_host_buffer_t *buffer);
void vkpt_b1_host_buffer_batch_destroy(vkpt_b1_host_buffer_batch_t *batch,
	vkpt_b1_host_buffer_t *const *inventory, uint32_t buffer_count);
VkResult vkpt_b1_host_buffer_map(vkpt_b1_host_buffer_t *buffer, void **mapped);
void vkpt_b1_host_buffer_unmap(vkpt_b1_host_buffer_t *buffer);
bool vkpt_b1_light_stats_active_bytes(const bsp_mesh_t *bsp_mesh,
	VkDeviceSize *active_bytes);
bool vkpt_b1_light_stats_source_identity(uint32_t slot,
	VkDeviceSize *active_bytes, uint64_t *allocation_generation);
bool vkpt_b1_readback_staging_allocation(uint32_t slot,
	VkDeviceSize *logical_bytes, VkDeviceSize *allocation_bytes,
	uint32_t *heap_index, uint64_t *allocation_generation);
bool vkpt_b1_readback_base_admission(
	vkpt_b1_readback_base_admission_t *admission);
VkResult vkpt_b1_readback_staging_snapshot(uint32_t slot,
	uint64_t expected_allocation_generation, ReadbackBuffer *destination);
bool vkpt_b1_readback_tail_initialised(uint64_t *initialisation_generation);
void vkpt_b1_tail_safety_initialisation_begin(void);
VkResult vkpt_b1_tail_safety_initialisation_complete(void);
bool vkpt_b1_technical_process_active(void);
bool vkpt_b1_state_witness_process_selected(void);
bool vkpt_b1_world_oracle_lifecycle_process_selected(void);
VkResult vkpt_b1_state_witness_preflight_map(const bsp_mesh_t *bsp_mesh,
	VkDeviceSize world_bytes, uint64_t world_allocation_generation);
void vkpt_b1_state_witness_reset_for_map(void);
void vkpt_b1_state_witness_latch_device_loss(void);
bool vkpt_b1_asvgf_coverage_identity(uint32_t role, uint32_t parity,
	uint64_t *header_hash64, uint64_t *inventory_hash64,
	uint64_t *physical_image_hash64, uint32_t physical_image_capacity,
	uint64_t *physical_image_set_hash64,
	uint64_t *descriptor_context_hash64);
bool vkpt_b1_world_oracle_get_material_upload(
	vkpt_b1_world_material_upload_t *upload);
bool vkpt_b1_runtime_record_material_upload_copy(
	VkCommandBuffer command_buffer, uint32_t staging_slot,
	VkDeviceSize source_offset, VkDeviceSize destination_offset,
	VkDeviceSize copy_bytes);
bool vkpt_b1_runtime_material_upload_ready_for_dispatch(
	const vkpt_b1_world_material_upload_t *upload);
bool vkpt_b1_world_oracle_get_seeded_allocation_v1(
	vkpt_b1_world_oracle_seeded_allocation_v1_t *state);
bool vkpt_b1_world_oracle_get_material_stage_diagnostic_v2(
	vkpt_b1_world_oracle_material_stage_diagnostic_v2_t *state);
bool vkpt_b1_world_oracle_get_material_upload_complete_v1(
	vkpt_b1_world_oracle_snapshot_t *state);
bool vkpt_b1_world_oracle_snapshot_upload_complete_v1(
	vkpt_b1_world_oracle_checkpoint_t checkpoint, void *destination,
	VkDeviceSize destination_bytes, vkpt_b1_world_oracle_snapshot_t *snapshot);
/* The actual animate-materials dispatch path owns this mutation. Callers use
 * get_state/snapshot to observe it and must not advance it independently. */
bool vkpt_b1_world_oracle_advance_for_dispatch(uint32_t active_primitives,
	uint32_t num_cameras, uint64_t expected_material_table_generation,
	uint64_t expected_material_upload_generation);
VkResult vkpt_b1_asvgf_coverage_create(vkpt_b1_host_buffer_t *left,
	vkpt_b1_host_buffer_t *right, uint32_t technical_process_kind,
	bool handshake_complete);
void vkpt_b1_asvgf_coverage_destroy(void);
bool vkpt_b1_asvgf_coverage_begin_role(uint32_t role, uint32_t parity,
	uint64_t allocation_generation, uint64_t reuse_epoch);
VkResult vkpt_b1_asvgf_coverage_poison_direct_lighting(
	VkCommandBuffer cmd_buf);
bool vkpt_b1_asvgf_coverage_mark_submitted(uint32_t role, uint32_t parity,
	uint64_t allocation_generation, uint64_t reuse_epoch,
	uint64_t submission_serial, uint64_t expected_completion_serial);
bool vkpt_b1_asvgf_coverage_validate(uint32_t role, uint32_t parity,
	uint64_t allocation_generation, uint64_t reuse_epoch,
	uint64_t submission_serial, uint64_t observed_completion_serial,
	bool exact_fence_complete, uint64_t *initial_hash64,
	uint64_t *final_hash64, uint32_t *failure_record,
	uint32_t *failure_word);
#endif
VkResult vkpt_vertex_buffer_create_pipelines(void);
VkResult vkpt_vertex_buffer_destroy_pipelines(void);
typedef struct vkpt_instance_geometry_audit_s {
	bool animation_dispatch_emitted;
	bool world_visibility_barrier_emitted;
	bool world_barrier_targets_world;
	uint32_t active_world_primitives;
	VkDeviceSize active_world_bytes;
	VkDeviceSize world_barrier_offset;
	VkDeviceSize world_barrier_size;
	VkPipelineStageFlags world_barrier_src_stages;
	VkPipelineStageFlags world_barrier_dst_stages;
	VkAccessFlags world_barrier_src_access;
	VkAccessFlags world_barrier_dst_access;
	uint32_t world_barrier_src_queue_family;
	uint32_t world_barrier_dst_queue_family;
} vkpt_instance_geometry_audit_t;
VkResult vkpt_instance_geometry(VkCommandBuffer cmd_buf, uint32_t num_instances,
	bool update_world_animations, vkpt_instance_geometry_audit_t *audit);
void vkpt_vertex_buffer_invalidate_static_model_vbos(int material_index);
VkResult vkpt_vertex_buffer_upload_models(void);
void vkpt_light_buffer_reset_counts(void);
VkResult vkpt_light_buffer_upload_to_staging(bool render_world, bsp_mesh_t *bsp_mesh, bsp_t* bsp, int num_model_lights, light_poly_t* transformed_model_lights, const float* sky_radiance, uint32_t history_identity);
VkResult vkpt_light_buffer_upload_staging(VkCommandBuffer cmd_buf);
VkResult vkpt_light_stats_clear(VkCommandBuffer cmd_buf, uint32_t logical_index);
#ifdef REFERENCE_B_B1
typedef struct vkpt_b1_light_semantic_state_s {
	uint64_t producer_serial;
	uint64_t hash64;
	uint32_t cluster_count;
	uint32_t active_node_count;
	uint32_t referenced_light_count;
	uint32_t registered_material_count;
	uint32_t active_mask_word_count;
	uint32_t light_style_count;
} vkpt_b1_light_semantic_state_t;
VkResult vkpt_light_stats_clear_all(VkCommandBuffer cmd_buf);
uint64_t vkpt_light_count_history_seed(uint32_t source_index);
void vkpt_b1_reset_light_semantic_state(void);
bool vkpt_b1_get_light_semantic_state(vkpt_b1_light_semantic_state_t *state);
bool vkpt_b1_close_light_staging_semantic_state(bool render_world,
	const bsp_mesh_t *bsp_mesh, int total_lights,
	vkpt_b1_light_semantic_state_t *state);
bool vkpt_b1_material_seed_identity(uint32_t *active_primitives, uint64_t *hash64);
void vkpt_b1_validate_instance_light_inputs(int count, const light_poly_t *lights,
	const char *source);
void vkpt_b1_validate_light_animation_inputs(const bsp_mesh_t *bsp_mesh);
uint64_t vkpt_b1_close_light_count_ledger(bsp_mesh_t *bsp_mesh, bsp_t *bsp,
	int num_model_lights, light_poly_t *model_lights, uint32_t source_index,
	uint64_t complete_active_hash, bool emit_details,
	b1_renderer_log_fn log_line, void *opaque);
#endif
VkResult vkpt_light_buffers_create(bsp_mesh_t *bsp_mesh);
VkResult vkpt_light_buffers_destroy(void);
bool vkpt_model_is_static(const model_t* model);
const model_vbo_t* vkpt_get_model_vbo(const model_t* model);

VkResult vkpt_iqm_matrix_buffer_upload_staging(VkCommandBuffer cmd_buf);

typedef struct {
	VkImage image;
	VkImageView image_view;
	VkDeviceMemory image_mem;

#ifdef VKPT_DEVICE_GROUPS
	// local per-GPU image bindings for SLI
	VkImage image_local[VKPT_MAX_GPUS];
	VkImageView image_view_local[VKPT_MAX_GPUS];
#endif
} vkpt_lazy_image_t;

VkResult vkpt_load_shader_modules(void);
VkResult vkpt_destroy_shader_modules(void);
VkResult vkpt_create_images(void);
// Fill a "lazy" image with actual Vulkan resources
VkResult vkpt_prepare_lazy_image(vkpt_lazy_image_t *lazy_image, int w, int h, VkFormat format, const char *descr);
VkResult vkpt_destroy_images(void);
// Destroy resources associated with a "lazy" image
VkResult vkpt_destroy_lazy_image(vkpt_lazy_image_t *lazy_image);

VkResult vkpt_pt_init(void);
VkResult vkpt_pt_destroy(void);
VkResult vkpt_pt_create_pipelines(void);
VkResult vkpt_pt_destroy_pipelines(void);

void vkpt_pt_reset_instances(void);
#ifdef REFERENCE_B_B1
void vkpt_pt_b1_save_instance_base(void);
void vkpt_pt_b1_restore_instance_base(void);
#endif
void vkpt_pt_instance_model_blas(const model_geometry_t* geom, const mat4 transform, uint32_t buffer_idx, int model_instance_index, uint32_t override_instance_mask);

VkResult vkpt_pt_create_toplevel(VkCommandBuffer cmd_buf, int idx, const EntityUploadInfo* upload_info, bool weapon_left_handed);
VkResult vkpt_pt_trace_primary_rays(VkCommandBuffer cmd_buf);
VkResult vkpt_pt_trace_reflections(VkCommandBuffer cmd_buf, int bounce);
VkResult vkpt_pt_trace_lighting(VkCommandBuffer cmd_buf, float num_bounce_rays);
VkResult vkpt_pt_update_descripter_set_bindings(int idx);
VkResult vkpt_pt_create_all_dynamic(VkCommandBuffer cmd_buf, int idx, const EntityUploadInfo* upload_info);

VkResult vkpt_asvgf_initialize(void);
VkResult vkpt_asvgf_destroy(void);
VkResult vkpt_asvgf_create_pipelines(void);
VkResult vkpt_asvgf_destroy_pipelines(void);
VkResult vkpt_asvgf_filter(VkCommandBuffer cmd_buf, bool enable_lf);
VkResult vkpt_compositing(VkCommandBuffer cmd_buf);
VkResult vkpt_interleave(VkCommandBuffer cmd_buf);
VkResult vkpt_taa(VkCommandBuffer cmd_buf);
VkResult vkpt_asvgf_gradient_reproject(VkCommandBuffer cmd_buf);

void vkpt_fsr_init_cvars(void);
VkResult vkpt_fsr_initialize(void);
VkResult vkpt_fsr_destroy(void);
VkResult vkpt_fsr_create_pipelines(void);
VkResult vkpt_fsr_destroy_pipelines(void);
bool vkpt_fsr_is_enabled(void);
bool vkpt_fsr_needs_upscale(void);
void vkpt_fsr_update_ubo(QVKUniformBuffer_t *ubo);
VkResult vkpt_fsr_do(VkCommandBuffer cmd_buf);
VkResult vkpt_fsr_final_blit(VkCommandBuffer cmd_buf, bool warp);

VkResult vkpt_bloom_initialize(void);
VkResult vkpt_bloom_destroy(void);
VkResult vkpt_bloom_create_pipelines(void);
VkResult vkpt_bloom_destroy_pipelines(void);
void vkpt_bloom_reset(void);
typedef struct vkpt_bloom_snapshot_s {
	float under_water_animation;
	float bloom_intensity;
	float bloom_sigma;
	float tonemap_hdr_clamp_strength;
	float menu_phase;
	uint32_t menu_start_ms;
} vkpt_bloom_snapshot_t;
void vkpt_bloom_advance(vkpt_bloom_snapshot_t *snapshot, float frame_time, bool under_water, bool menu_mode);
void vkpt_bloom_apply(QVKUniformBuffer_t *ubo, const vkpt_bloom_snapshot_t *snapshot);
void vkpt_bloom_update(QVKUniformBuffer_t * ubo, float frame_time, bool under_water, bool menu_mode);
VkResult vkpt_bloom_record_cmd_buffer(VkCommandBuffer cmd_buf);

VkResult vkpt_tone_mapping_initialize(void);
VkResult vkpt_tone_mapping_destroy(void);
VkResult vkpt_tone_mapping_create_pipelines(void);
VkResult vkpt_tone_mapping_reset(VkCommandBuffer cmd_buf);
VkResult vkpt_tone_mapping_destroy_pipelines(void);
VkResult vkpt_tone_mapping_record_update(VkCommandBuffer cmd_buf, float frame_time);
VkResult vkpt_tone_mapping_record_apply(VkCommandBuffer cmd_buf);
VkResult vkpt_tone_mapping_record_cmd_buffer(VkCommandBuffer cmd_buf, float frame_time);
void vkpt_tone_mapping_request_reset(void);
void vkpt_tone_mapping_draw_debug(void);

VkResult vkpt_shadow_map_initialize(void);
VkResult vkpt_shadow_map_destroy(void);
VkResult vkpt_shadow_map_create_pipelines(void);
VkResult vkpt_shadow_map_destroy_pipelines(void);
VkResult vkpt_shadow_map_render(VkCommandBuffer cmd_buf, float* view_projection_matrix,
	uint32_t static_offset, uint32_t num_static_verts,
	uint32_t dynamic_offset, uint32_t num_dynamic_verts,
	uint32_t transparent_offset, uint32_t num_transparent_verts);
VkImageView vkpt_shadow_map_get_view(void);
void vkpt_shadow_map_setup(const sun_light_t* light, const float* bbox_min, const float* bbox_max,
	float* VP, float* depth_scale, bool random_sampling);
void vkpt_shadow_map_reset_instances(void);
void vkpt_shadow_map_add_instance(const float* model_matrix, VkBuffer buffer, size_t vertex_offset, uint32_t prim_count);

int load_img(const char *name, image_t *image);
// Transparency module API

bool initialize_transparency(void);
void destroy_transparency(void);

void update_transparency(VkCommandBuffer command_buffer, const float* view_matrix,
	const particle_t* particles, int particle_num, const entity_t* entities, int entity_num);

typedef enum {
	VKPT_TRANSPARENCY_PARTICLES,
	VKPT_TRANSPARENCY_SPRITES,

	VKPT_TRANSPARENCY_COUNT
} vkpt_transparency_t;

void vkpt_get_transparency_buffers(
	vkpt_transparency_t ttype, 
	BufferResource_t** vertex_buffer,
	uint64_t* vertex_offset, 
	BufferResource_t** index_buffer,
	uint64_t* index_offset,
	uint32_t* num_vertices,
	uint32_t* num_indices);
void vkpt_get_beam_aabb_buffer(
	BufferResource_t** aabb_buffer,
	uint64_t* aabb_offset,
	uint32_t* num_aabbs);

VkBufferView get_transparency_particle_color_buffer_view(void);
VkBufferView get_transparency_beam_color_buffer_view(void);
VkBufferView get_transparency_sprite_info_buffer_view(void);
VkBufferView get_transparency_beam_intersect_buffer_view(void);
void get_transparency_counts(int* particle_num, int* beam_num, int* sprite_num);
void vkpt_build_beam_lights(light_poly_t* light_list, int* num_lights, int max_lights, bsp_t *bsp, entity_t* entities, int num_entites, float adapted_luminance);
bool vkpt_build_cylinder_light(light_poly_t* light_list, int* num_lights, int max_lights, bsp_t *bsp, vec3_t begin, vec3_t end, vec3_t color, float radius);
bool get_triangle_off_center(const float* positions, float* center, float* anti_center, float offset);

VkResult vkpt_initialize_god_rays(void);
VkResult vkpt_destroy_god_rays(void);
VkResult vkpt_god_rays_create_pipelines(void);
VkResult vkpt_god_rays_destroy_pipelines(void);
VkResult vkpt_god_rays_update_images(void);
VkResult vkpt_god_rays_noop(void);
bool vkpt_god_rays_enabled(const sun_light_t* sun_light);
void vkpt_record_god_rays_trace_command_buffer(VkCommandBuffer command_buffer, int pass);
void vkpt_record_god_rays_filter_command_buffer(VkCommandBuffer command_buffer);
void vkpt_god_rays_prepare_ubo(
	QVKUniformBuffer_t * ubo, 
	const aabb_t* world_aabb,
	const float* proj, 
	const float* view, 
	const float* shadowmap_viewproj, 
	float shadowmap_depth_scale);

void vkpt_freecam_reset(void);
void vkpt_freecam_update(float frame_time);
void vkpt_reset_accumulation(void);

typedef struct maliasframe_s {
    vec3_t  scale;
    vec3_t  translate;
    vec3_t  bounds[2];
    vec_t   radius;
} maliasframe_t;

typedef struct maliasmesh_s {
    int             numverts;
    int             numtris;
    int             numindices;
    int             numskins;
    int             tri_offset; /* offset in vertex buffer on device */
    int             *indices;
    vec3_t          *positions;
    vec3_t          *normals;
    vec2_t          *tex_coords;
	vec3_t          *tangents;
	uint32_t        *blend_indices; // iqm only
	uint32_t        *blend_weights; // iqm only
	struct pbr_material_s **materials;
	bool            handedness;
} maliasmesh_t;

// needed for model.c
#define TESS_MAX_VERTICES   16384
#define TESS_MAX_INDICES    (3 * TESS_MAX_VERTICES)

#define QGL_INDEX_TYPE  GLuint

typedef struct {
    color_t     colors[2]; // 0 - actual color, 1 - transparency (for text drawing)
    float scale;
	float alpha_scale;
} drawStatic_t;

// Performance marker debug labels
extern const char *perf_marker_labels[NUM_PROFILER_ENTRIES];

static inline void begin_perf_marker(VkCommandBuffer command_buffer, int index)
{
	_VK(vkpt_profiler_query(command_buffer, index, PROFILER_START));

	const VkDebugUtilsLabelEXT label = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = perf_marker_labels[index]
	};

	if (qvkCmdBeginDebugUtilsLabelEXT != NULL)
		qvkCmdBeginDebugUtilsLabelEXT(command_buffer, &label);
}

static inline void end_perf_marker(VkCommandBuffer command_buffer, int index)
{
	if (qvkCmdEndDebugUtilsLabelEXT != NULL)
		qvkCmdEndDebugUtilsLabelEXT(command_buffer);

	_VK(vkpt_profiler_query(command_buffer, index, PROFILER_STOP));
}

#define BEGIN_PERF_MARKER(command_buffer, name)  begin_perf_marker(command_buffer, name)
#define END_PERF_MARKER(command_buffer, name)    end_perf_marker(command_buffer, name)

void R_SetClipRect_RTX(const clipRect_t *clip);
void R_ClearColor_RTX(void);
void R_SetAlpha_RTX(float alpha);
void R_SetAlphaScale_RTX(float alpha);
void R_SetColor_RTX(uint32_t color);
void R_LightPoint_RTX(const vec3_t origin, vec3_t light);
void R_SetScale_RTX(float scale);
void R_DrawStretchPic_RTX(int x, int y, int w, int h, qhandle_t pic);
void R_DrawKeepAspectPic_RTX(int x, int y, int w, int h, qhandle_t pic);
void R_DrawPic_RTX(int x, int y, qhandle_t pic);
void R_DrawStretchRaw_RTX(int x, int y, int w, int h);
void R_UpdateRawPic_RTX(int pic_w, int pic_h, const uint32_t *pic);
void R_DiscardRawPic_RTX(void);
void R_TileClear_RTX(int x, int y, int w, int h, qhandle_t pic);
void R_DrawFill8_RTX(int x, int y, int w, int h, int c);
void R_DrawFill32_RTX(int x, int y, int w, int h, uint32_t color);
void R_DrawChar_RTX(int x, int y, int flags, int c, qhandle_t font);
int R_DrawString_RTX(int x, int y, int flags, size_t maxlen, const char *s, qhandle_t font);
bool R_InterceptKey_RTX(unsigned key, bool down);

void IMG_Load_RTX(image_t *image, byte *pic);
void IMG_Unload_RTX(image_t *image);
void IMG_ReadPixels_RTX(screenshot_t *s);
void IMG_ReadPixelsHDR_RTX(screenshot_t *s);

int MOD_LoadMD2_RTX(model_t *model, const void *rawdata, size_t length, const char* mod_name);
int MOD_LoadMD3_RTX(model_t* model, const void* rawdata, size_t length, const char* mod_name);
int MOD_LoadIQM_RTX(model_t *model, const void *rawdata, size_t length, const char* mod_name);
void MOD_Reference_RTX(model_t *model);

bool vkpt_debugdraw_supported(void);
void vkpt_debugdraw_addtext(const vec3_t origin, const vec3_t angles, const char *text, float size, uint32_t color, uint32_t time, bool depth_test);
bool vkpt_debugdraw_have(void);
void vkpt_debugdraw_draw(VkCommandBuffer cmd_buf);
VkResult vkpt_debugdraw_create(void);
VkResult vkpt_debugdraw_create_pipelines(void);
void vkpt_debugdraw_prepare(void);
VkImageView vpkt_debugdraw_imageview(void);
VkResult vkpt_debugdraw_destroy(void);
VkResult vkpt_debugdraw_destroy_pipelines(void);

#endif  /*__VKPT_H__*/

