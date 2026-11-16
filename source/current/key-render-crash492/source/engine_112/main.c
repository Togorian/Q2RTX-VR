/* Modified 14 November 2026: enforce local single-player VR network and command boundaries. */
// Modified 14 November 2026: preserve diagnostic origin independently of product branding.
/* Modified 14 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
#include "../../../runtime-recovery555/source/b2_runtime555.h"
#include "../../../bloom331/source/b2_startup276.h"
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


#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/shared/shared.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/common/bsp.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/common/cmd.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/common/common.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/common/cvar.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/common/files.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/common/math.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/client/video.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/client/client.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/refresh/debug.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/refresh/refresh.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/refresh/images.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/refresh/models.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/system/hunk.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/refresh/vkpt/vkpt.h"
#include "../../../../baseline/b2-denoiser-state233/source/b2_denoiser_state.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-036/src/refresh/vkpt/b2_hack_peripheral_guide.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-037/include/refresh/b2_weapon_presentation.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/inc/refresh/b2_world.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/refresh/vkpt/material.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/refresh/vkpt/fog.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/refresh/vkpt/cameras.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/refresh/vkpt/physical_sky.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/refresh/vkpt/conversion.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/client/client.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/client/ui/ui.h"
#ifdef REFERENCE_B_B2
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/refresh/vkpt/reference_b_b2_xr.h"
#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/client/reference_b_b2.h"
#endif

#include "baseline/b2-consolidated215/inputs/origin-018/compat/engine-112/historical/quad-views/source/src/refresh/vkpt/shader/vertex_buffer.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


cvar_t *cvar_profiler = NULL;
cvar_t *cvar_profiler_samples = NULL;
cvar_t *cvar_profiler_scale = NULL;
cvar_t *cvar_hdr = NULL;
cvar_t *cvar_vsync = NULL;
cvar_t *cvar_pt_caustics = NULL;
cvar_t *cvar_pt_enable_nodraw = NULL;
cvar_t *cvar_pt_enable_surface_lights = NULL;
cvar_t *cvar_pt_enable_surface_lights_warp = NULL;
cvar_t* cvar_pt_surface_lights_fake_emissive_algo = NULL;
cvar_t* cvar_pt_surface_lights_threshold = NULL;
cvar_t* cvar_pt_bsp_radiance_scale = NULL;
cvar_t *cvar_pt_bsp_sky_lights = NULL;
cvar_t *cvar_pt_accumulation_rendering = NULL;
cvar_t *cvar_pt_accumulation_rendering_framenum = NULL;
cvar_t *cvar_pt_projection = NULL;
cvar_t *cvar_pt_dof = NULL;
cvar_t* cvar_pt_freecam = NULL;
cvar_t *cvar_pt_nearest = NULL;
cvar_t *cvar_pt_bilerp_chars = NULL;
cvar_t *cvar_pt_bilerp_pics = NULL;
cvar_t *cvar_pt_waterwarp = NULL;
cvar_t *cvar_drs_enable = NULL;
cvar_t *cvar_drs_target = NULL;
cvar_t *cvar_drs_minscale = NULL;
cvar_t *cvar_drs_maxscale = NULL;
cvar_t *cvar_drs_adjust_up = NULL;
cvar_t *cvar_drs_adjust_down = NULL;
cvar_t *cvar_drs_gain = NULL;
cvar_t *cvar_drs_last_scale = NULL;
cvar_t *cvar_tm_blend_enable = NULL;
cvar_t *cvar_ui_hdr_nits = NULL; /* HDR mode UI (stretch pic) brightness in nits */
extern cvar_t *scr_viewsize;
extern cvar_t *cvar_bloom_enable;
extern cvar_t* cvar_flt_taa;
static int drs_current_scale = 0;
static int drs_effective_scale = 0;
static bool drs_last_frame_world = false;

cvar_t* cvar_min_driver_version_nvidia = NULL;
cvar_t* cvar_min_driver_version_amd = NULL;
cvar_t *cvar_ray_tracing_api = NULL;
cvar_t *cvar_vk_validation = NULL;
#ifdef REFERENCE_B_B2
/* Validation callbacks may arrive from driver threads; only renderer init resets this latch. */
static SDL_atomic_t b2_validation_failed;
#endif
static bool vr_device_lost_terminal;
static bool vr_abandon_children;
/* Latched after any real queue submission; an unproved failure leaves children to the device. */
static bool vr_has_submitted;

#if USE_DEBUG
cvar_t *cvar_pt_test_shell = NULL;
#endif

extern uiStatic_t uis;

#ifdef VKPT_DEVICE_GROUPS
cvar_t *cvar_sli = NULL;
#endif

#ifdef VKPT_IMAGE_DUMPS
cvar_t *cvar_dump_image = NULL;
#endif

byte cluster_debug_mask[VIS_MAX_BYTES];
int cluster_debug_index;

#define UBO_CVAR_DO(name, default_value) cvar_t *cvar_##name;
UBO_CVAR_LIST
#undef UBO_CVAR_DO

#ifdef REFERENCE_B_B2
// Private hack-day selection; public controls and per-view histories are preserved.
static cvar_t *cvar_b2_hack_mode;
#endif

static bsp_t *bsp_world_model;

static bool temporal_frame_valid = false;
#ifdef REFERENCE_B_B2
#include "../../../../baseline/b2-denoiser-state233/source/b2_denoiser_renderer.inc"
#endif

static int world_anim_frame = 0;

static vec3_t avg_envmap_color = { 0.f };

static image_t *water_normal_texture = NULL;

int num_accumulated_frames = 0;

static bool frame_ready = false;

static float sky_rotation = 0.f;
static int sky_autorotate = 0;
static vec3_t sky_axis = { 0.f };

#define NUM_TAA_SAMPLES 128
static vec2_t taa_samples[NUM_TAA_SAMPLES];

typedef enum {
	VKPT_INIT_DEFAULT            = (0),
	VKPT_INIT_SWAPCHAIN_RECREATE = (1 << 1),
	VKPT_INIT_RELOAD_SHADER      = (1 << 2),
} VkptInitFlags_t;

typedef struct VkptInit_s {
	const char *name;
	VkResult (*initialize)(void);
	VkResult (*destroy)(void);
	VkptInitFlags_t flags;
	int is_initialized;
} VkptInit_t;
VkptInit_t vkpt_initialization[] = {
	{ "profiler", vkpt_profiler_initialize,            vkpt_profiler_destroy,                VKPT_INIT_DEFAULT,            0 },
	{ "vbo",      vkpt_vertex_buffer_create,           vkpt_vertex_buffer_destroy,           VKPT_INIT_DEFAULT,            0 },
	{ "ubo",      vkpt_uniform_buffer_create,          vkpt_uniform_buffer_destroy,          VKPT_INIT_DEFAULT,            0 },
	{ "textures", vkpt_textures_initialize,            vkpt_textures_destroy,                VKPT_INIT_DEFAULT,            0 },
	{ "shadowmap", 	vkpt_shadow_map_initialize,        vkpt_shadow_map_destroy,              VKPT_INIT_DEFAULT,            0 },
	{ "shadowmap|", vkpt_shadow_map_create_pipelines,  vkpt_shadow_map_destroy_pipelines,    VKPT_INIT_RELOAD_SHADER ,     0 },
	{ "images",   vkpt_create_images,                  vkpt_destroy_images,                  VKPT_INIT_SWAPCHAIN_RECREATE, 0 },
	{ "draw",     vkpt_draw_initialize,                vkpt_draw_destroy,                    VKPT_INIT_DEFAULT,            0 },
	{ "pt",       vkpt_pt_init,                        vkpt_pt_destroy,                      VKPT_INIT_DEFAULT,            0 },
	{ "pt|",      vkpt_pt_create_pipelines,            vkpt_pt_destroy_pipelines,            VKPT_INIT_RELOAD_SHADER,      0 },
	{ "draw|",    vkpt_draw_create_pipelines,          vkpt_draw_destroy_pipelines,          VKPT_INIT_SWAPCHAIN_RECREATE
																						   | VKPT_INIT_RELOAD_SHADER,      0 },
	{ "debug",    vkpt_debugdraw_create,               vkpt_debugdraw_destroy,               VKPT_INIT_DEFAULT,            0 },
	{ "debug|",   vkpt_debugdraw_create_pipelines,     vkpt_debugdraw_destroy_pipelines,     VKPT_INIT_SWAPCHAIN_RECREATE
																						   | VKPT_INIT_RELOAD_SHADER,      0 },
	{ "vbo|",     vkpt_vertex_buffer_create_pipelines, vkpt_vertex_buffer_destroy_pipelines, VKPT_INIT_RELOAD_SHADER,      0 },
	{ "asvgf",    vkpt_asvgf_initialize,               vkpt_asvgf_destroy,                   VKPT_INIT_DEFAULT,            0 },
	{ "asvgf|",   vkpt_asvgf_create_pipelines,         vkpt_asvgf_destroy_pipelines,         VKPT_INIT_RELOAD_SHADER,      0 },
	{ "bloom",    vkpt_bloom_initialize,               vkpt_bloom_destroy,                   VKPT_INIT_DEFAULT,            0 },
	{ "bloom|",   vkpt_bloom_create_pipelines,         vkpt_bloom_destroy_pipelines,         VKPT_INIT_RELOAD_SHADER | VKPT_INIT_SWAPCHAIN_RECREATE,      0 },
	{ "tonemap",  vkpt_tone_mapping_initialize,        vkpt_tone_mapping_destroy,            VKPT_INIT_DEFAULT,            0 },
	{ "tonemap|", vkpt_tone_mapping_create_pipelines,  vkpt_tone_mapping_destroy_pipelines,  VKPT_INIT_RELOAD_SHADER,      0 },
	{ "fsr",      vkpt_fsr_initialize,                 vkpt_fsr_destroy,                     VKPT_INIT_DEFAULT,            0 },
	{ "fsr|",     vkpt_fsr_create_pipelines,           vkpt_fsr_destroy_pipelines,           VKPT_INIT_RELOAD_SHADER,      0 },

	{ "physicalSky", vkpt_physical_sky_initialize,         vkpt_physical_sky_destroy,            VKPT_INIT_DEFAULT,        0 },
	{ "physicalSky|", vkpt_physical_sky_create_pipelines,  vkpt_physical_sky_destroy_pipelines,  VKPT_INIT_RELOAD_SHADER,  0 },
	{ "godrays",    vkpt_initialize_god_rays,           vkpt_destroy_god_rays,              VKPT_INIT_DEFAULT,             0 },
	{ "godrays|",   vkpt_god_rays_create_pipelines,     vkpt_god_rays_destroy_pipelines,    VKPT_INIT_RELOAD_SHADER,       0 },
	{ "godraysI",   vkpt_god_rays_update_images,        vkpt_god_rays_noop,                 VKPT_INIT_SWAPCHAIN_RECREATE,  0 },
};

// Values returned by pick_surface_format_*
typedef struct picked_surface_format_s {
	// Swapchain surface format
	VkSurfaceFormatKHR surface_fmt;
	// Swapchain image view format. This will always be an *_SRGB format, while the surface format may not.
	VkFormat swapchain_view_fmt;
} picked_surface_format_t;

void debug_output(const char* format, ...);
static void recreate_swapchain(void);

static void viewsize_changed(cvar_t *self)
{
	Cvar_ClampInteger(scr_viewsize, 25, 200);
	Com_Printf("Resolution scale: %d%%\n", scr_viewsize->integer);
}

// R6-B2: callbacks run serially between frames. Reject unsafe configuration
// before acquiring a desktop image or resetting a physical frame fence.
static struct {
    const char *name;
    float value;
    int integer;
} b2_callback_settings580[] = {
    {"pt_nearest"}, {"pt_bilerp_chars"}, {"pt_bilerp_pics"},
    {"flt_enable"}, {"flt_temporal_hf"}, {"flt_temporal_lf"}, {"flt_temporal_spec"}
};
static void b2_accept_callback_settings580(void)
{
    for (size_t i=0; i<LENGTH(b2_callback_settings580); i++) {
        cvar_t *var=Cvar_FindVar(b2_callback_settings580[i].name);
        if (!var) Com_Error(ERR_FATAL, "VR callback setting was not registered");
        b2_callback_settings580[i].value=var->value;
        b2_callback_settings580[i].integer=var->integer;
    }
}
static bool b2_callback_settings_compatible580(void)
{
    for (size_t i=0; i<LENGTH(b2_callback_settings580); i++) {
        cvar_t *var=Cvar_FindVar(b2_callback_settings580[i].name);
        if (!var || var->value!=b2_callback_settings580[i].value ||
            var->integer!=b2_callback_settings580[i].integer) return false;
    }
    return true;
}
static bool b2_settings_changed580;
static bool b2_settings_rejected580;
static bool b2_physical_started580;
static bool b2_settings_history_reset580;
static bool b2_frame_settings_valid580(void);
static void b2_discard_settings_frame580(void);
static void b2_settings_notice580(void)
{
    static bool warned;
    if (!warned) Com_Printf("VR frame skipped: unsupported graphics settings. Restart with the VR launcher.\n");
    warned = true;
}
static void pt_nearest_changed(cvar_t* self)
{
#ifdef REFERENCE_B_B2
    if (B2XR_Requested() && qvk.b2_temporal_activated) {
        b2_settings_changed580 = true;
        b2_settings_notice580();
        return;
    }
#endif
	vkpt_invalidate_texture_descriptors();
}

static void drs_target_changed(cvar_t *self)
{
	Cvar_ClampInteger(self, 30, 240);
}

static void drs_minscale_changed(cvar_t *self)
{
	Cvar_ClampInteger(self, 25, 100);
}

static void drs_maxscale_changed(cvar_t *self)
{
	Cvar_ClampInteger(self, 50, 200);
}

static void accumulation_cvar_changed(cvar_t* self)
{
	// Reset accumulation rendering on DoF parameter change
	num_accumulated_frames = 0;
}

static inline bool extents_equal(VkExtent2D a, VkExtent2D b)
{
	return a.width == b.width && a.height == b.height;
}

// Keep this presentation state outside QVK_t: existing resource-object layouts
// and every render/history extent retain their accepted ABI and meaning.
static VkExtent2D desktop_present_extent;
#ifdef REFERENCE_B_B2
static VkExtent2D b2_desktop_drawable;
static bool b2_desktop_dirty534;
static bool b2_desktop_failed582; // Keep desktop-mode extents; never retry this session.
extern void Cvar_B2GraphicsActive534(bool active);
extern void vkpt_draw_destroy_desktop534(void);
extern VkResult vkpt_draw_create_desktop534(void);
static bool b2_desktop_enabled(void)
{
	cvar_t *option = Cvar_FindVar("b2_desktop_mirror");
	if (!option || option->value == 0.f) return false;
	if (option->value != 1.f || !B2XR_Requested()
	)
		Com_Error(ERR_FATAL, "B2_FAIL: desktop mirror requires exclusive B2");
	return true;
}

static void b2_note_desktop_drawable(int width, int height)
{
	if (width < 0 || height < 0 || width > 8192 || height > 8192)
		Com_Error(ERR_FATAL, "B2_FAIL: desktop drawable outside checked bounds");
	if (b2_desktop_drawable.width != width || b2_desktop_drawable.height != height)
		b2_desktop_dirty534 = true;
	b2_desktop_drawable = (VkExtent2D){width, height};
	qvk.draw_width = width; qvk.draw_height = height;
	// r_config describes the logical view/UI canvas in this opt-in mode.
	// Physical window dimensions remain independently checked above and at acquire.
	r_config.width = B2G_Width(); r_config.height = B2G_Height();
}
#endif

VkExtent2D vkpt_get_present_extent(void)
{
#ifdef REFERENCE_B_B2
	if (b2_desktop_enabled()) return desktop_present_extent;
#endif
	return qvk.extent_unscaled;
}

static void vkpt_select_swapchain_extent(const VkSurfaceCapabilitiesKHR *caps)
{
	VkExtent2D extent = caps->currentExtent;
	if (extent.width == ~0u) {
		extent.width = max(caps->minImageExtent.width, min(caps->maxImageExtent.width, qvk.draw_width));
		extent.height = max(caps->minImageExtent.height, min(caps->maxImageExtent.height, qvk.draw_height));
	}
	desktop_present_extent = extent;
#ifdef REFERENCE_B_B2
	if (b2_desktop_enabled()) {
		// Surface capabilities may change between the SDL event and this query.
		if (extent.width < 2 || extent.height < 2 || extent.width > 8192 || extent.height > 8192)
			Com_Error(ERR_FATAL, "B2_FAIL: desktop swapchain outside checked bounds");
		qvk.extent_unscaled = (VkExtent2D){B2G_Width(), B2G_Height()};
		return;
	}
#endif
	qvk.extent_unscaled = extent;
}

static VkExtent2D get_render_extent(void)
{
#ifdef REFERENCE_B_B2
    // Runtime percentage is already applied once by B2G at session creation.
    // Preserve archived desktop viewsize, but never rescale owned VR eyes.
    if (B2XR_Requested()) return (VkExtent2D){B2G_Width(),B2G_Height()};
#endif
	int scale;
	if(drs_effective_scale)
	{
		scale = drs_effective_scale;
	}
	else
	{
		scale = scr_viewsize->integer;
		if(cvar_drs_enable->integer)
		{
			// Ensure render extent stays below get_screen_image_extent() result
			scale = min(cvar_drs_maxscale->integer, scale);
		}
	}

	VkExtent2D result;
	result.width = (uint32_t)(qvk.extent_unscaled.width * (float)scale / 100.f);
	result.height = (uint32_t)(qvk.extent_unscaled.height * (float)scale / 100.f);

	result.width = (result.width + 1) & ~1;

	return result;
}

static VkExtent2D get_screen_image_extent(void)
{
	VkExtent2D result;
	if (cvar_drs_enable->integer)
	{
		int image_scale = max(cvar_drs_minscale->integer, cvar_drs_maxscale->integer);

		// In case FSR enable we'll always upscale to 100% and thus need at least the unscaled extent
		if(vkpt_fsr_is_enabled())
			image_scale = max(image_scale, 100);

		result.width = (uint32_t)(qvk.extent_unscaled.width * (float)image_scale / 100.f);
		result.height = (uint32_t)(qvk.extent_unscaled.height * (float)image_scale / 100.f);
	}
	else
	{
		result.width = max(qvk.extent_render.width, qvk.extent_unscaled.width);
		result.height = max(qvk.extent_render.height, qvk.extent_unscaled.height);
	}

	result.width = (result.width + 1) & ~1;

	return result;
}

void vkpt_reset_accumulation()
{
	num_accumulated_frames = 0;
}

VkResult
vkpt_initialize_all(VkptInitFlags_t init_flags)
{
	VkResult initial_idle_result = vkDeviceWaitIdle(qvk.device);
	if (initial_idle_result != VK_SUCCESS)
	{
		if (initial_idle_result == VK_ERROR_DEVICE_LOST)
			vr_device_lost_terminal = true;
		vr_abandon_children = true;
		Com_Error(ERR_FATAL,
			"VR_FAIL: renderer initialisation idle boundary failed (%s)",
			qvk_result_to_string(initial_idle_result));
	}

	qvk.extent_render = get_render_extent();
	qvk.extent_screen_images = get_screen_image_extent();
    B2D_Record(0,"RENDER_INIT flags=%u internal=%ux%u render=%ux%u screen=%ux%u",(unsigned)init_flags,B2G_Width(),B2G_Height(),qvk.extent_render.width,qvk.extent_render.height,qvk.extent_screen_images.width,qvk.extent_screen_images.height);

	qvk.extent_taa_images.width = max(qvk.extent_screen_images.width, qvk.extent_unscaled.width);
	qvk.extent_taa_images.height = max(qvk.extent_screen_images.height, qvk.extent_unscaled.height);

	qvk.gpu_slice_width = (qvk.extent_render.width + qvk.device_count - 1) / qvk.device_count;

	for(int i = 0; i < LENGTH(vkpt_initialization); i++) {
		VkptInit_t *init = vkpt_initialization + i;
		if((init->flags & init_flags) != init_flags)
			continue;
		
		// some entries will respond to multiple events --- do not initialize twice
		if (init->is_initialized)
			continue;

		VkResult initialize_result = init->initialize ?
			init->initialize() : VK_SUCCESS;
		init->is_initialized = initialize_result == VK_SUCCESS;

		if (!init->is_initialized)
		{
			if (initialize_result == VK_ERROR_DEVICE_LOST)
			{
				vr_device_lost_terminal = true;
				vr_abandon_children = true;
			}
			else if (vr_has_submitted)
				vr_abandon_children = true;
			Com_Error(ERR_FATAL, "Couldn't initialize %s (%s).\n", init->name,
				qvk_result_to_string(initialize_result));
		}
		assert(init->is_initialized);
	}

	if ((VKPT_INIT_DEFAULT & init_flags) == init_flags)
	{
		if (!initialize_transparency())
			return VK_RESULT_MAX_ENUM;
	}

	vkpt_textures_prefetch();

	water_normal_texture = IMG_Find("textures/water_n.tga", IT_SKIN, IF_PERMANENT);

	return VK_SUCCESS;
}

VkResult
vkpt_destroy_all(VkptInitFlags_t destroy_flags)
{
	
	VkResult idle_result = vkDeviceWaitIdle(qvk.device);
	
	if (idle_result != VK_SUCCESS)
	{
		if (idle_result == VK_ERROR_DEVICE_LOST)
			vr_device_lost_terminal = true;
		vr_abandon_children = true;
		return idle_result;
	}

	
	for(int i = LENGTH(vkpt_initialization) - 1; i >= 0; i--) {
		VkptInit_t *init = vkpt_initialization + i;
		if((init->flags & destroy_flags) != destroy_flags)
			continue;
		
		// some entries will respond to multiple events --- do not destroy twice
		if (!init->is_initialized)
			continue;

		VkResult destroy_result = init->destroy ?
			init->destroy() : VK_SUCCESS;
		if (destroy_result != VK_SUCCESS)
		{
			if (destroy_result == VK_ERROR_DEVICE_LOST)
				vr_device_lost_terminal = true;
			vr_abandon_children = true;
			return destroy_result;
		}
		init->is_initialized = 0;
		assert(!init->is_initialized);
	}

	if ((VKPT_INIT_DEFAULT & destroy_flags) == destroy_flags)
	{
		destroy_transparency();
		vkpt_light_buffers_destroy();
	}

	
	return VK_SUCCESS;
}

void
vkpt_reload_shader(void)
{
#ifdef REFERENCE_B_B2
    Com_Printf("Live shader compilation is disabled in Q2RTX VR. Build shaders offline and restart the game.\n");
    return;
#endif

	char buf[1024];
#ifdef _WIN32
	FILE *f = _popen("compile_shaders.bat", "r");
#else
	FILE *f = popen("make -j compile_shaders", "r");
#endif
	if(f) {
		while(fgets(buf, sizeof buf, f)) {
			Com_Printf("%s", buf);
		}
#ifdef _WIN32
		_pclose(f);
#else
		pclose(f);
#endif
	}

	vkpt_destroy_shader_modules();
	vkpt_load_shader_modules();

	vkpt_destroy_all(VKPT_INIT_RELOAD_SHADER);
	vkpt_initialize_all(VKPT_INIT_RELOAD_SHADER);
}

static void vkpt_reload_textures(void)
{
	IMG_ReloadAll();
}

//
//
//

vkpt_refdef_t vkpt_refdef = {
	.z_near = 1.0f,
	.z_far  = 4096.0f,
};

QVK_t qvk = {
	.draw_width         = 1920,
	.draw_height        = 1080,
	.frame_counter      = 0,
};

#define VK_EXTENSION_DO(a) PFN_##a q##a = 0;
LIST_EXTENSIONS_ACCEL_STRUCT
LIST_EXTENSIONS_RAY_PIPELINE
LIST_EXTENSIONS_DEBUG
LIST_EXTENSIONS_INSTANCE
#undef VK_EXTENSION_DO

const char *vk_validation_layers[] = {
	"VK_LAYER_KHRONOS_validation"
};

const char *vk_requested_instance_extensions[] = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#ifdef VKPT_DEVICE_GROUPS
	VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
#endif
};

const char *vk_requested_device_extensions_common[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef VKPT_DEVICE_GROUPS
	VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
	VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
#endif
};

const char *vk_requested_device_extensions_ray_pipeline[] = {
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
};

const char* vk_requested_device_extensions_ray_query[] = {
	VK_KHR_RAY_QUERY_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
};

#define OPTIONAL_INSTANCE_EXTENSIONS		\
	VK_OPT_EXT_DO(VK_EXT_SWAPCHAIN_COLOR_SPACE)

enum optional_instance_extension_id
{
#define VK_OPT_EXT_DO(ext)	OPT_EXT_ ## ext,
	OPTIONAL_INSTANCE_EXTENSIONS
#undef VK_OPT_EXT_DO
	NUM_OPTIONAL_INSTANCE_EXTENSIONS
};

static const char *optional_instance_extension_name[NUM_OPTIONAL_INSTANCE_EXTENSIONS] = {
#define VK_OPT_EXT_DO(ext)	ext ## _EXTENSION_NAME,
	OPTIONAL_INSTANCE_EXTENSIONS
#undef VK_OPT_EXT_DO
};

#define OPTIONAL_DEVICE_EXTENSIONS					\
	VK_OPT_EXT_DO(VK_KHR_LINE_RASTERIZATION)		\
	VK_OPT_EXT_DO(VK_KHR_SHADER_NON_SEMANTIC_INFO)	\
	VK_OPT_EXT_DO(VK_EXT_DEBUG_MARKER)				\
	VK_OPT_EXT_DO(VK_EXT_MEMORY_BUDGET)


enum optional_device_extension_id
{
#define VK_OPT_EXT_DO(ext)	OPT_EXT_ ## ext,
	OPTIONAL_DEVICE_EXTENSIONS
#undef VK_OPT_EXT_DO
	NUM_OPTIONAL_DEVICE_EXTENSIONS
};

static const char *optional_device_extension_name[NUM_OPTIONAL_DEVICE_EXTENSIONS] = {
#define VK_OPT_EXT_DO(ext)	ext ## _EXTENSION_NAME,
	OPTIONAL_DEVICE_EXTENSIONS
#undef VK_OPT_EXT_DO
};

static const VkApplicationInfo vk_app_info = {
	.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	.pApplicationName   = "quake 2 pathtracing",
	.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
	.pEngineName        = "vkpt",
	.engineVersion      = VK_MAKE_VERSION(1, 0, 0),
	.apiVersion         = VK_API_VERSION_1_2,
};

/* use this to override file names */
static const char *shader_module_file_names[NUM_QVK_SHADER_MODULES];


void
get_vk_extension_list(
		const char *layer,
		uint32_t *num_extensions,
		VkExtensionProperties **ext)
{
	_VK(vkEnumerateInstanceExtensionProperties(layer, num_extensions, NULL));
	*ext = malloc(sizeof(**ext) * *num_extensions);
	_VK(vkEnumerateInstanceExtensionProperties(layer, num_extensions, *ext));
}

void
get_vk_layer_list(
		uint32_t *num_layers,
		VkLayerProperties **ext)
{
	_VK(vkEnumerateInstanceLayerProperties(num_layers, NULL));
	*ext = malloc(sizeof(**ext) * *num_layers);
	_VK(vkEnumerateInstanceLayerProperties(num_layers, *ext));
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		void *user_data)
{
	Com_EPrintf("validation layer %i %i: %s\n", (int32_t)type, (int32_t)severity,  callback_data->pMessage);
	debug_output("Vulkan error: %s\n", callback_data->pMessage);
#ifdef REFERENCE_B_B2
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		SDL_AtomicSet(&b2_validation_failed, 1);
#endif

	if (callback_data->cmdBufLabelCount)
	{
		Com_EPrintf("~~~ ");
		for (uint32_t i = 0; i < callback_data->cmdBufLabelCount; ++i)
		{
			const VkDebugUtilsLabelEXT* label = &callback_data->pCmdBufLabels[i];
			Com_EPrintf("%s ~ ", label->pLabelName);
		}
		Com_EPrintf("\n");
	}

	if (callback_data->objectCount)
	{
		for (uint32_t i = 0; i < callback_data->objectCount; ++i)
		{
			const VkDebugUtilsObjectNameInfoEXT* obj = &callback_data->pObjects[i];
			Com_EPrintf("--- %s %i\n", obj->pObjectName, (int32_t)obj->objectType);
		}
	}

	Com_EPrintf("\n");
	return VK_FALSE;
}

VkResult
qvkCreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pCallback)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if(func)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VkResult
qvkDestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT callback,
		const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if(func) {
		func(instance, callback, pAllocator);
		return VK_SUCCESS;
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static bool pick_surface_format_hdr(picked_surface_format_t* picked_fmt, const VkSurfaceFormatKHR avail_surface_formats[], size_t num_avail_surface_formats)
{
	VkSurfaceFormatKHR acceptable_formats[] = {
		{ VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT }
	};

	for(int i = 0; i < LENGTH(acceptable_formats); i++) {
		for(int j = 0; j < num_avail_surface_formats; j++) {
			if((acceptable_formats[i].format == avail_surface_formats[j].format)
				&& (acceptable_formats[i].colorSpace == avail_surface_formats[j].colorSpace)){
				picked_fmt->surface_fmt = avail_surface_formats[j];
				picked_fmt->swapchain_view_fmt = avail_surface_formats[j].format;
				return true;
			}
		}
	}
	return false;
}

static bool pick_surface_format_sdr(picked_surface_format_t* picked_fmt, const VkSurfaceFormatKHR avail_surface_formats[], size_t num_avail_surface_formats)
{
	struct {
		VkFormat format;
		VkFormat swapchain_view_fmt;
	} acceptable_formats[] = {
		{VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB},
		{VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB},
		{VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB},
		{VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SRGB},
	};

	for(int i = 0; i < LENGTH(acceptable_formats); i++) {
		for(int j = 0; j < num_avail_surface_formats; j++) {
			if(acceptable_formats[i].format == avail_surface_formats[j].format) {
				picked_fmt->surface_fmt = avail_surface_formats[j];
				picked_fmt->swapchain_view_fmt = acceptable_formats[i].swapchain_view_fmt;
				return true;
			}
		}
	}
	return false;
}

VkResult
create_swapchain(void)
{
	num_accumulated_frames = 0;

	/* create swapchain (query details and ignore them afterwards :-) )*/
	VkSurfaceCapabilitiesKHR surf_capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(qvk.physical_device, qvk.surface, &surf_capabilities);
    if (result != VK_SUCCESS) return result;

	if (surf_capabilities.currentExtent.width == 0 || surf_capabilities.currentExtent.height == 0)
		return VK_SUCCESS;

	uint32_t num_formats = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(qvk.physical_device, qvk.surface, &num_formats, NULL);
    if (result != VK_SUCCESS) return result;
    if (!num_formats || num_formats > 256) return VK_ERROR_INITIALIZATION_FAILED;
	VkSurfaceFormatKHR *avail_surface_formats = alloca(sizeof(VkSurfaceFormatKHR) * num_formats);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(qvk.physical_device, qvk.surface, &num_formats, avail_surface_formats);
    if (result != VK_SUCCESS) return result;
	/* Com_Printf("num surface formats: %d\n", num_formats);

	Com_Printf("available surface formats:\n");
	for(int i = 0; i < num_formats; i++)
		Com_Printf("  %s\n", vk_format_to_string(avail_surface_formats[i].format)); */ 


	picked_surface_format_t picked_format;
	bool surface_format_found = false;
	if(cvar_hdr->integer != 0) {
		surface_format_found = qvk.supports_colorspace && pick_surface_format_hdr(&picked_format, avail_surface_formats, num_formats);
		qvk.surf_is_hdr = surface_format_found;
		if(!surface_format_found) {
			Com_WPrintf("HDR was requested but no supported surface format was found.\n");
			Cvar_SetByVar(cvar_hdr, "0", FROM_CODE);
			}
	} else {
		qvk.surf_is_hdr = false;
	}
	if(!surface_format_found) {
		// HDR disabled, or fallback to SDR
		surface_format_found = pick_surface_format_sdr(&picked_format, avail_surface_formats, num_formats);
	}
	if(!surface_format_found) {
		Com_EPrintf("no acceptable surface format available!\n");
		return VK_ERROR_FORMAT_NOT_SUPPORTED;
	}
	qvk.surf_format = picked_format.surface_fmt;

	uint32_t num_present_modes = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(qvk.physical_device, qvk.surface, &num_present_modes, NULL);
    if (result != VK_SUCCESS) return result;
    if (!num_present_modes || num_present_modes > 64) return VK_ERROR_INITIALIZATION_FAILED;
	VkPresentModeKHR *avail_present_modes = alloca(sizeof(VkPresentModeKHR) * num_present_modes);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(qvk.physical_device, qvk.surface, &num_present_modes, avail_present_modes);
    if (result != VK_SUCCESS) return result;
	bool immediate_mode_available = false;

	for (int i = 0; i < num_present_modes; i++) {
		if (avail_present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			immediate_mode_available = true;
			break;
		}
	}

	qvk.surf_vsync = (cvar_vsync->integer != 0);

	if (qvk.surf_vsync) {
		qvk.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	} else if (immediate_mode_available) {
		qvk.present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	} else {
		qvk.present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
	}

	vkpt_select_swapchain_extent(&surf_capabilities);

	uint32_t num_images = max(surf_capabilities.minImageCount, 2);
	if(surf_capabilities.maxImageCount > 0)
		num_images = min(num_images, surf_capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR swpch_create_info = {
		.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface               = qvk.surface,
		.minImageCount         = num_images,
		.imageFormat           = qvk.surf_format.format,
		.imageColorSpace       = qvk.surf_format.colorSpace,
		.imageExtent           = vkpt_get_present_extent(),
		.imageArrayLayers      = 1, /* only needs to be changed for stereoscopic rendering */ 
		.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
							   | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
							   | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE, /* VK_SHARING_MODE_CONCURRENT if not using same queue */
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL,
		.preTransform          = surf_capabilities.currentTransform,
		.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, /* no alpha for window transparency */
		.presentMode           = qvk.present_mode,
		.clipped               = VK_FALSE, /* do not render pixels that are occluded by other windows */
		//.clipped               = VK_TRUE, /* do not render pixels that are occluded by other windows */
		.oldSwapchain          = VK_NULL_HANDLE, /* need to provide previous swapchain in case of window resize */
	};

	VkResult create_result534 = vkCreateSwapchainKHR(qvk.device, &swpch_create_info, NULL, &qvk.swap_chain);
	if (create_result534 != VK_SUCCESS) {
		qvk.swap_chain = VK_NULL_HANDLE;
		Com_EPrintf("error creating swapchain (%s)\n", qvk_result_to_string(create_result534));
		return create_result534;
	}

    uint32_t count = 0;
    result = vkGetSwapchainImagesKHR(qvk.device, qvk.swap_chain, &count, NULL);
    if (result != VK_SUCCESS) return result;
    if (!count || count > 64) return VK_ERROR_INITIALIZATION_FAILED;
    qvk.num_swap_chain_images = count;
    qvk.swap_chain_images = calloc(count, sizeof(*qvk.swap_chain_images));
    qvk.swap_chain_image_views = calloc(count, sizeof(*qvk.swap_chain_image_views));
    if (!qvk.swap_chain_images || !qvk.swap_chain_image_views) return VK_ERROR_OUT_OF_HOST_MEMORY;
    result = vkGetSwapchainImagesKHR(qvk.device, qvk.swap_chain, &qvk.num_swap_chain_images, qvk.swap_chain_images);
    if (result != VK_SUCCESS) return result;
	for(int i = 0; i < qvk.num_swap_chain_images; i++) {
		VkImageViewCreateInfo img_create_info = {
			.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image      = qvk.swap_chain_images[i],
			.viewType   = VK_IMAGE_VIEW_TYPE_2D,
			.format     = picked_format.swapchain_view_fmt,
#if 1
			.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			},
#endif
			.subresourceRange = {
				.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = 1
			}
		};

        VkImageView view = VK_NULL_HANDLE;
        result = vkCreateImageView(qvk.device, &img_create_info, NULL, &view);
        if (result != VK_SUCCESS) return result; // Retain only successfully created views.
        qvk.swap_chain_image_views[i] = view;

	}

	VkCommandBuffer cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);

	for (int image_index = 0; image_index < qvk.num_swap_chain_images; image_index++)
	{
		IMAGE_BARRIER(cmd_buf,
			.image = qvk.swap_chain_images[image_index],
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		);
	}

	vkpt_submit_command_buffer_simple(cmd_buf, qvk.queue_graphics, true);
	vkpt_wait_idle(qvk.queue_graphics, &qvk.cmd_buffers_graphics);

	return VK_SUCCESS;
}

VkResult
create_command_pool_and_fences(void)
{
	VkResult result;
	VkCommandPoolCreateInfo cmd_pool_create_info = {
		.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = qvk.queue_idx_graphics,
		.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};

	/* command pool and buffers */
	result = vkCreateCommandPool(qvk.device, &cmd_pool_create_info, NULL,
		&qvk.cmd_buffers_graphics.command_pool);
	if (result != VK_SUCCESS)
		return result;
	
	cmd_pool_create_info.queueFamilyIndex = qvk.queue_idx_transfer;
	result = vkCreateCommandPool(qvk.device, &cmd_pool_create_info, NULL,
		&qvk.cmd_buffers_transfer.command_pool);
	if (result != VK_SUCCESS)
		return result;

	/* fences and semaphores */
	for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
	{
		for (int gpu = 0; gpu < qvk.device_count; gpu++)
		{
			semaphore_group_t* group = &qvk.semaphores[frame][gpu];

			VkSemaphoreCreateInfo semaphore_info = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		
			result = vkCreateSemaphore(qvk.device, &semaphore_info, NULL,
				&group->image_available);
			if (result != VK_SUCCESS)
				return result;
			result = vkCreateSemaphore(qvk.device, &semaphore_info, NULL,
				&group->render_finished);
			if (result != VK_SUCCESS)
				return result;
			result = vkCreateSemaphore(qvk.device, &semaphore_info, NULL,
				&group->transfer_finished);
			if (result != VK_SUCCESS)
				return result;
			result = vkCreateSemaphore(qvk.device, &semaphore_info, NULL,
				&group->trace_finished);
			if (result != VK_SUCCESS)
				return result;

			ATTACH_LABEL_VARIABLE(group->image_available, SEMAPHORE);
			ATTACH_LABEL_VARIABLE(group->render_finished, SEMAPHORE);
			ATTACH_LABEL_VARIABLE(group->transfer_finished, SEMAPHORE);
			ATTACH_LABEL_VARIABLE(group->trace_finished, SEMAPHORE);

			group->trace_signaled = false;
		}
	}

	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT, /* fence's initial state set to be signaled
												  to make program not hang */
	};
	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		result = vkCreateFence(qvk.device, &fence_info, NULL,
			qvk.fences_frame_sync + i);
		if (result != VK_SUCCESS)
			return result;
		ATTACH_LABEL_VARIABLE(qvk.fences_frame_sync[i], FENCE);
	}

	return VK_SUCCESS;
}

static void
append_string_list(const char** dst, uint32_t* dst_count, uint32_t dst_capacity, const char** src, uint32_t src_count)
{
	assert(*dst_count + src_count <= dst_capacity);
	dst += *dst_count;
	memcpy((void*)dst, src, sizeof(char*) * src_count);
	*dst_count += src_count;
}

bool
init_vulkan(void)
{
	Com_Printf("----- init_vulkan -----\n");

	/* layers */
	get_vk_layer_list(&qvk.num_layers, &qvk.layers);
	Com_Printf("Available Vulkan layers: \n");
	for(int i = 0; i < qvk.num_layers; i++) {
		Com_Printf("  %s\n", qvk.layers[i].layerName);
	}
	
	/* instance extensions */

	if (!SDL_Vulkan_GetInstanceExtensions(qvk.window, &qvk.num_sdl2_extensions, NULL)) {
		Com_EPrintf("Couldn't get SDL2 Vulkan extension count\n");
		return false;
	}

	qvk.sdl2_extensions = malloc(sizeof(char*) * qvk.num_sdl2_extensions);
	if (!SDL_Vulkan_GetInstanceExtensions(qvk.window, &qvk.num_sdl2_extensions, qvk.sdl2_extensions)) {
		Com_EPrintf("Couldn't get SDL2 Vulkan extensions\n");
		return false;
	}

	Com_Printf("Vulkan instance extensions required by SDL2: \n");
	for (int i = 0; i < qvk.num_sdl2_extensions; i++) {
		Com_Printf("  %s\n", qvk.sdl2_extensions[i]);
	}

	int num_inst_ext_max = qvk.num_sdl2_extensions + LENGTH(vk_requested_instance_extensions) + NUM_OPTIONAL_INSTANCE_EXTENSIONS;
	const char **ext = alloca(sizeof(const char *) * num_inst_ext_max);
	int num_inst_ext_combined = 0;
	memcpy(ext + num_inst_ext_combined, qvk.sdl2_extensions, qvk.num_sdl2_extensions * sizeof(*qvk.sdl2_extensions));
	num_inst_ext_combined += qvk.num_sdl2_extensions;
	memcpy(ext + num_inst_ext_combined, vk_requested_instance_extensions, sizeof(vk_requested_instance_extensions));
	num_inst_ext_combined += LENGTH(vk_requested_instance_extensions);

	bool available_optional_instance_extensions[NUM_OPTIONAL_INSTANCE_EXTENSIONS] = { false };

	get_vk_extension_list(NULL, &qvk.num_extensions, &qvk.extensions); /* valid here? */
	int num_inst_ext_required = num_inst_ext_combined;
	Com_Printf("Supported Vulkan instance extensions: \n");
	for(int i = 0; i < qvk.num_extensions; i++) {
		int requested = 0;
		for(int j = 0; j < num_inst_ext_required; j++) {
			if(!strcmp(qvk.extensions[i].extensionName, ext[j])) {
				requested = 1;
				break;
			}
		}
		for(int j = 0; j < NUM_OPTIONAL_INSTANCE_EXTENSIONS; j++) {
			const char *ext_name = optional_instance_extension_name[j];
			if(!strcmp(qvk.extensions[i].extensionName, ext_name)) {
				requested = 1;
				ext[num_inst_ext_combined++] = ext_name;
				available_optional_instance_extensions[j] = true;
				break;
			}
		}
		Com_Printf("  %s%s\n", qvk.extensions[i].extensionName, requested ? " (requested)" : "");
	}

	/* create instance */
	VkInstanceCreateInfo inst_create_info = {
		.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo        = &vk_app_info,
		.enabledExtensionCount   = num_inst_ext_combined,
		.ppEnabledExtensionNames = (const char * const*)ext,
	};

	qvk.enable_validation = false;

	if (cvar_vk_validation->integer)
	{
		inst_create_info.ppEnabledLayerNames = vk_validation_layers;
		inst_create_info.enabledLayerCount = LENGTH(vk_validation_layers);
		qvk.enable_validation = true;
	}

#ifdef REFERENCE_B_B2
	SDL_AtomicSet(&b2_validation_failed, 0);
	if (!B2XR_Init(vk_app_info.apiVersion))
		return false;
	VkResult result = B2XR_CreateVulkanInstance(&inst_create_info, &qvk.instance);
#else
	VkResult result = vkCreateInstance(&inst_create_info, NULL, &qvk.instance);
#endif

	if (result == VK_ERROR_LAYER_NOT_PRESENT)
	{
#ifdef REFERENCE_B_B2
		if (B2XR_Requested())
			Com_Error(ERR_FATAL, "The requested Vulkan validation layer is unavailable. Restore the standard graphics configuration and relaunch.");
#endif
		Com_WPrintf("Vulkan validation layer is requested through cvar %s but is not available.\n", cvar_vk_validation->name);

		// Try again, this time without the validation layer

		inst_create_info.enabledLayerCount = 0;
		result = vkCreateInstance(&inst_create_info, NULL, &qvk.instance);
		qvk.enable_validation = false;
	}
	else if (cvar_vk_validation->integer)
	{
		Com_WPrintf("Vulkan validation layer is enabled, expect degraded game performance.\n");
	}

	if (result != VK_SUCCESS)
	{
        Com_EPrintf("Vulkan instance startup result=%s\n",qvk_result_to_string(result));
#ifdef REFERENCE_B_B2
        Com_Error(ERR_FATAL,"%s",B2XR_StartupMessage555("Vulkan could not create a graphics instance. Check your graphics driver, then relaunch."));
#else
        Com_Error(ERR_FATAL,"Vulkan could not create a graphics instance.");
#endif
		return false;
	}

#define VK_EXTENSION_DO(a) \
		q##a = (PFN_##a) vkGetInstanceProcAddr(qvk.instance, #a); \
		if (!q##a) { Com_EPrintf("warning: could not load instance function %s\n", #a); }
	LIST_EXTENSIONS_INSTANCE
#undef VK_EXTENSION_DO

	/* setup debug callback */
	VkDebugUtilsMessengerCreateInfoEXT dbg_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
			  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
		.pfnUserCallback = vk_debug_callback,
		.pUserData = NULL
	};

	_VK(qvkCreateDebugUtilsMessengerEXT(qvk.instance, &dbg_create_info, NULL, &qvk.dbg_messenger));

	/* create surface */
	if(!SDL_Vulkan_CreateSurface(qvk.window, qvk.instance, &qvk.surface)) {
		Com_EPrintf("SDL2 could not create a surface!\n");
		return false;
	}

	/* pick physical device (iterate over all but pick device 0 anyways) */
	uint32_t num_devices = 0;
	_VK(vkEnumeratePhysicalDevices(qvk.instance, &num_devices, NULL));
	if(num_devices == 0)
		return false;
	VkPhysicalDevice *devices = alloca(sizeof(VkPhysicalDevice) *num_devices);
	_VK(vkEnumeratePhysicalDevices(qvk.instance, &num_devices, devices));

#ifdef VKPT_DEVICE_GROUPS
	uint32_t num_device_groups = 0;

	if (cvar_sli->integer)
		_VK(vkEnumeratePhysicalDeviceGroups(qvk.instance, &num_device_groups, NULL));

	VkDeviceGroupDeviceCreateInfo device_group_create_info;
	VkPhysicalDeviceGroupProperties device_group_info;
	device_group_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
	device_group_info.pNext = NULL;

	if(num_device_groups > 0) {
		// we always use the first group
		num_device_groups = 1;
		_VK(vkEnumeratePhysicalDeviceGroups(qvk.instance, &num_device_groups, &device_group_info));

		if (device_group_info.physicalDeviceCount > VKPT_MAX_GPUS)
		{
			Com_EPrintf("SLI: device group 0 has %d devices, which is more than maximum supported count (%d).\n",
				device_group_info.physicalDeviceCount, VKPT_MAX_GPUS);
			return false;
		}

		device_group_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
		device_group_create_info.pNext = NULL;
		device_group_create_info.physicalDeviceCount = device_group_info.physicalDeviceCount;
		device_group_create_info.pPhysicalDevices = device_group_info.physicalDevices;

		qvk.device_count = device_group_create_info.physicalDeviceCount;
		for(int i = 0; i < qvk.device_count; i++) {
			qvk.device_group_physical_devices[i] = device_group_create_info.pPhysicalDevices[i];
		}
		Com_Printf("SLI: using device group 0 with %d device(s).\n", qvk.device_count);
	}
	else
	{
		qvk.device_count = 1;
		if (!cvar_sli->integer)
			Com_Printf("SLI: multi-GPU support disabled through the 'sli' console variable.\n");
		else
			Com_Printf("SLI: no device groups found, using a single device.\n");
	}
#else
	qvk.device_count = 1;
#endif

	int picked_device_with_ray_pipeline = -1;
	int picked_device_with_ray_query = -1;
	VkDriverId picked_driver_ray_query = VK_DRIVER_ID_MAX_ENUM;
	qvk.use_ray_query = false;
#ifdef REFERENCE_B_B2
	VkPhysicalDevice b2_device = B2XR_Requested() ?
		B2XR_PhysicalDevice(qvk.instance) : VK_NULL_HANDLE;
	if (B2XR_Requested() && (!b2_device || qvk.device_count != 1))
		Com_Error(ERR_FATAL, "%s", B2XR_StartupMessage555("The VR runtime could not select a compatible single graphics device. Check the headset connection and graphics driver, then relaunch."));
#endif

	for(int i = 0; i < num_devices; i++) 
	{
#ifdef REFERENCE_B_B2
		if (b2_device && devices[i] != b2_device)
			continue;
#endif
		VkPhysicalDeviceDriverProperties driver_properties = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
			.pNext = NULL
		};

		VkPhysicalDeviceProperties2 dev_properties2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			.pNext = &driver_properties
		};
		vkGetPhysicalDeviceProperties2(devices[i], &dev_properties2);

		VkPhysicalDeviceFeatures dev_features;
		vkGetPhysicalDeviceFeatures(devices[i], &dev_features);

		Com_Printf("Physical device %d: %s\n", i, dev_properties2.properties.deviceName);

		uint32_t num_ext;
		vkEnumerateDeviceExtensionProperties(devices[i], NULL, &num_ext, NULL);

		VkExtensionProperties *ext_properties = alloca(sizeof(VkExtensionProperties) * num_ext);
		vkEnumerateDeviceExtensionProperties(devices[i], NULL, &num_ext, ext_properties);

		Com_Printf("Supported Vulkan device extensions:\n");
		for(int j = 0; j < num_ext; j++) 
		{
			Com_Printf("  %s\n", ext_properties[j].extensionName);

			if(!strcmp(ext_properties[j].extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)) 
			{
				if (picked_device_with_ray_pipeline < 0)
				{
					picked_device_with_ray_pipeline = i;
				}
			}

			if (!strcmp(ext_properties[j].extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME))
			{
				if (picked_device_with_ray_query < 0)
				{
					picked_device_with_ray_query = i;
					picked_driver_ray_query = driver_properties.driverID;
				}
			}
		}
	}

	int picked_device = -1;

	if (!Q_strcasecmp(cvar_ray_tracing_api->string, "query") && picked_device_with_ray_query >= 0)
	{
		qvk.use_ray_query = true;
		picked_device = picked_device_with_ray_query;
	}
	else if (!Q_strcasecmp(cvar_ray_tracing_api->string, "pipeline") && picked_device_with_ray_pipeline >= 0)
	{
		qvk.use_ray_query = false;
		picked_device = picked_device_with_ray_pipeline;
	}
	
	if (picked_device < 0)
	{
		if (Q_strcasecmp(cvar_ray_tracing_api->string, "auto"))
		{
			Com_WPrintf("Requested Ray Tracing API (%s) is not available, switching to automatic selection.\n", cvar_ray_tracing_api->string);
		}

		if (picked_driver_ray_query == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
		{
			// Prefer KHR_ray_query on NVIDIA drivers, if available.
			qvk.use_ray_query = true;
			picked_device = picked_device_with_ray_query;
		}
		else if (picked_device_with_ray_pipeline >= 0)
		{
			// Prefer KHR_ray_tracing_pipeline otherwise
			qvk.use_ray_query = false;
			picked_device = picked_device_with_ray_pipeline;
		}
		else if (picked_device_with_ray_query >= 0)
		{
			qvk.use_ray_query = true;
			picked_device = picked_device_with_ray_query;
		}
	}

	if (picked_device < 0)
	{
		Com_Error(ERR_FATAL, "No ray tracing capable GPU found.");
	}

	qvk.physical_device = devices[picked_device];

	{
		VkPhysicalDeviceDriverProperties driver_properties = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
			.pNext = NULL
		};

		VkPhysicalDeviceProperties2 dev_properties2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			.pNext = &driver_properties
		};

		vkGetPhysicalDeviceProperties2(devices[picked_device], &dev_properties2);

		// Store the timestamp period to get correct profiler results
		qvk.timestampPeriod = dev_properties2.properties.limits.timestampPeriod;

		Com_Printf("Picked physical device %d: %s\n", picked_device, dev_properties2.properties.deviceName);
		Com_Printf("Using %s\n", (qvk.use_ray_query ? VK_KHR_RAY_QUERY_EXTENSION_NAME : VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME));

#ifdef _WIN32
		if (dev_properties2.properties.vendorID == 0x10de) // NVIDIA vendor ID
		{
			uint32_t driver_major = (dev_properties2.properties.driverVersion >> 22) & 0x3ff;
			uint32_t driver_minor = (dev_properties2.properties.driverVersion >> 14) & 0xff;

			Com_Printf("NVIDIA GPU detected. Driver version: %u.%02u\n", driver_major, driver_minor);
			
			uint32_t required_major = 0;
			uint32_t required_minor = 0;
			int nfields = sscanf(cvar_min_driver_version_nvidia->string, "%u.%u", &required_major, &required_minor);
			if (nfields == 2)
			{
				if (driver_major < required_major || driver_major == required_major && driver_minor < required_minor)
				{
					Com_Error(ERR_FATAL, "This game requires NVIDIA Graphics Driver version to be at least %u.%02u, "
						"while the installed version is %u.%02u.\nPlease update the NVIDIA Graphics Driver.",
						required_major, required_minor, driver_major, driver_minor);
				}
			}
		}
		else if (driver_properties.driverID == VK_DRIVER_ID_AMD_PROPRIETARY)
		{
			Com_Printf("AMD GPU detected. Driver version: %s\n", driver_properties.driverInfo);

			uint32_t present_major = 0;
			uint32_t present_minor = 0;
			uint32_t present_patch = 0;
			int nfields_present = sscanf(driver_properties.driverInfo, "%u.%u.%u", &present_major, &present_minor, &present_patch);

			uint32_t required_major = 0;
			uint32_t required_minor = 0;
			uint32_t required_patch = 0;
			int nfields_required = sscanf(cvar_min_driver_version_amd->string, "%u.%u.%u", &required_major, &required_minor, &required_patch);

			if (nfields_present == 3 && nfields_required == 3)
			{
				if (present_major < required_major || present_major == required_major && present_minor < required_minor || present_major == required_major && present_minor == required_minor && present_patch < required_patch)
				{
					Com_Error(ERR_FATAL, "This game requires AMD Radeon Software version to be at least %s, while the installed version is %s.\nPlease update the AMD Radeon Software.",
						cvar_min_driver_version_amd->string, driver_properties.driverInfo);
				}
			}
		}
#endif
	}

	// Collect available "optional" extensions
	bool available_optional_device_extensions[NUM_OPTIONAL_DEVICE_EXTENSIONS] = { false };
	{
		uint32_t num_ext;
		vkEnumerateDeviceExtensionProperties(qvk.physical_device, NULL, &num_ext, NULL);

		VkExtensionProperties *ext_properties = alloca(sizeof(VkExtensionProperties) * num_ext);
		vkEnumerateDeviceExtensionProperties(qvk.physical_device, NULL, &num_ext, ext_properties);

		for (int test_ext = 0; test_ext < NUM_OPTIONAL_DEVICE_EXTENSIONS; test_ext++) {
			for (uint32_t dev_ext = 0; dev_ext < num_ext; dev_ext++) {
				if(strcmp(ext_properties[dev_ext].extensionName, optional_device_extension_name[test_ext]) == 0) {
					available_optional_device_extensions[test_ext] = true;
					break;
				}
			}
		}
	}

	// Query device 16-bit float capabilities
	VkPhysicalDevice16BitStorageFeatures features_16bit_storage = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES,
	};
	{
		VkPhysicalDeviceVulkan12Features device_features_1_2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &features_16bit_storage
		};
		VkPhysicalDeviceFeatures2 device_features = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
			.pNext = &device_features_1_2
		};
		VkPhysicalDeviceLineRasterizationFeaturesKHR device_features_lines = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_KHR,
		};
		if (available_optional_device_extensions[OPT_EXT_VK_KHR_LINE_RASTERIZATION]) {
			device_features_lines.pNext = device_features.pNext;
			device_features.pNext = &device_features_lines;
		}
		vkGetPhysicalDeviceFeatures2(qvk.physical_device, &device_features);
		qvk.supports_fp16 = device_features_1_2.shaderFloat16 && features_16bit_storage.storageBuffer16BitAccess;
		qvk.supports_debug_lines = device_features.features.fillModeNonSolid && device_features.features.wideLines;
		qvk.supports_smooth_lines = qvk.supports_debug_lines && device_features_lines.smoothLines;
		qvk.supports_memory_budget = available_optional_device_extensions[OPT_EXT_VK_EXT_MEMORY_BUDGET];
	}
	Com_Printf("FP16 support: %s\n", qvk.supports_fp16 ? "yes" : "no");
	Com_Printf("Debug lines support: %s%s\n", qvk.supports_debug_lines ? "yes" : "no", qvk.supports_smooth_lines ? " (smooth)" : "");

	vkGetPhysicalDeviceMemoryProperties(qvk.physical_device, &qvk.mem_properties);

	/* queue family and create physical device */
	uint32_t num_queue_families = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(qvk.physical_device, &num_queue_families, NULL);
	VkQueueFamilyProperties *queue_families = alloca(sizeof(VkQueueFamilyProperties) * num_queue_families);
	vkGetPhysicalDeviceQueueFamilyProperties(qvk.physical_device, &num_queue_families, queue_families);

	// Com_Printf("num queue families: %d\n", num_queue_families);

	qvk.queue_idx_graphics = -1;
	qvk.queue_idx_transfer = -1;

	for(int i = 0; i < num_queue_families; i++) {
		if(!queue_families[i].queueCount)
			continue;
		VkBool32 present_support = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR(qvk.physical_device, i, qvk.surface, &present_support);

		const int supports_graphics = queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
		const int supports_compute = queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
		const int supports_transfer = queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT;

		if(supports_graphics && supports_compute && qvk.queue_idx_graphics < 0) {
			if(!present_support)
				continue;
			qvk.queue_idx_graphics = i;
		}
		if(supports_transfer && (qvk.queue_idx_transfer < 0 || qvk.queue_idx_graphics == qvk.queue_idx_transfer)) {
			qvk.queue_idx_transfer = i;
		}
	}

	if(qvk.queue_idx_graphics < 0 || qvk.queue_idx_transfer < 0) {
		Com_Error(ERR_FATAL, "Could not find a suitable Vulkan queue family!\n");
		return false;
	}
#ifdef REFERENCE_B_B2
	/* The first live prototype serializes transfer and trace on one queue. */
	if (B2XR_Requested())
		qvk.queue_idx_transfer = qvk.queue_idx_graphics;
#endif
	
	float queue_priorities = 1.0f;
	int num_create_queues = 0;
	VkDeviceQueueCreateInfo queue_create_info[3];

	{
		VkDeviceQueueCreateInfo q = {
			.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueCount       = 1,
			.pQueuePriorities = &queue_priorities,
			.queueFamilyIndex = qvk.queue_idx_graphics,
		};

		queue_create_info[num_create_queues++] = q;
	};
	if(qvk.queue_idx_transfer != qvk.queue_idx_graphics) {
		VkDeviceQueueCreateInfo q = {
			.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueCount       = 1,
			.pQueuePriorities = &queue_priorities,
			.queueFamilyIndex = qvk.queue_idx_transfer,
		};
		queue_create_info[num_create_queues++] = q;
	};

#ifdef VKPT_DEVICE_GROUPS
	if (qvk.device_count > 1) {
		features_16bit_storage.pNext = &device_group_create_info;
	}
#endif

	VkPhysicalDeviceAccelerationStructureFeaturesKHR physical_device_as_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
		.pNext = &features_16bit_storage,
		.accelerationStructure = VK_TRUE,
	};

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR physical_device_rt_pipeline_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
		.pNext = &physical_device_as_features,
		.rayTracingPipeline = VK_TRUE
	};

	VkPhysicalDeviceRayQueryFeaturesKHR physical_device_ray_query_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
		.pNext = &physical_device_as_features,
		.rayQuery = VK_TRUE
	};

	VkPhysicalDeviceVulkan12Features device_features_vk12 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.descriptorIndexing = VK_TRUE,
		.shaderFloat16 = qvk.supports_fp16,
		.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
		.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
		.runtimeDescriptorArray = VK_TRUE,
		.samplerFilterMinmax = VK_TRUE,
		.bufferDeviceAddress = VK_TRUE,
		.bufferDeviceAddressMultiDevice = qvk.device_count > 1 ? VK_TRUE : VK_FALSE,
	};
	VkPhysicalDeviceFeatures2 device_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
		.pNext = &device_features_vk12,
		.features = {
			.robustBufferAccess = VK_TRUE,
			.fullDrawIndexUint32 = VK_TRUE,
			.imageCubeArray = VK_TRUE,
			.independentBlend = VK_TRUE,
			.geometryShader = VK_FALSE,
			.tessellationShader = VK_FALSE,
			.sampleRateShading = VK_FALSE,
			.dualSrcBlend = VK_FALSE,
			.logicOp = VK_FALSE,
			.multiDrawIndirect = VK_FALSE,
			.drawIndirectFirstInstance = VK_FALSE,
			.depthClamp = VK_FALSE,
			.depthBiasClamp = VK_FALSE,
			.fillModeNonSolid = qvk.supports_debug_lines,
			.depthBounds = VK_FALSE,
			.wideLines = qvk.supports_debug_lines,
			.largePoints = VK_FALSE,
			.alphaToOne = VK_FALSE,
			.multiViewport = VK_FALSE,
			.samplerAnisotropy = VK_TRUE,
			.textureCompressionETC2 = VK_FALSE,
			.textureCompressionASTC_LDR = VK_FALSE,
			.textureCompressionBC = VK_FALSE,
			.occlusionQueryPrecise = VK_FALSE,
			.pipelineStatisticsQuery = VK_TRUE,
			.vertexPipelineStoresAndAtomics = VK_FALSE,
			.fragmentStoresAndAtomics = VK_FALSE,
			.shaderTessellationAndGeometryPointSize = VK_FALSE,
			.shaderImageGatherExtended = VK_FALSE,
			.shaderStorageImageExtendedFormats = VK_TRUE,
			.shaderStorageImageMultisample = VK_FALSE,
			.shaderStorageImageReadWithoutFormat = VK_FALSE,
			.shaderStorageImageWriteWithoutFormat = VK_FALSE,
			.shaderUniformBufferArrayDynamicIndexing = VK_TRUE,
			.shaderSampledImageArrayDynamicIndexing = VK_TRUE,
			.shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
			.shaderStorageImageArrayDynamicIndexing = VK_TRUE,
			.shaderClipDistance = VK_FALSE,
			.shaderCullDistance = VK_FALSE,
			.shaderFloat64 = VK_FALSE,
			.shaderInt64 = VK_FALSE,
			.shaderInt16 = qvk.supports_fp16,
			.shaderResourceResidency = VK_FALSE,
			.shaderResourceMinLod = VK_FALSE,
			.sparseBinding = VK_FALSE,
			.sparseResidencyBuffer = VK_FALSE,
			.sparseResidencyImage2D = VK_FALSE,
			.sparseResidencyImage3D = VK_FALSE,
			.sparseResidency2Samples = VK_FALSE,
			.sparseResidency4Samples = VK_FALSE,
			.sparseResidency8Samples = VK_FALSE,
			.sparseResidency16Samples = VK_FALSE,
			.sparseResidencyAliased = VK_FALSE,
			.variableMultisampleRate = VK_FALSE,
			.inheritedQueries = VK_FALSE,
		}
	};
	VkDeviceCreateInfo dev_create_info = {
		.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext                   = &device_features,
		.pQueueCreateInfos       = queue_create_info,
		.queueCreateInfoCount    = num_create_queues
	};

	uint32_t max_extension_count = LENGTH(vk_requested_device_extensions_common);
	max_extension_count += max(LENGTH(vk_requested_device_extensions_ray_pipeline), LENGTH(vk_requested_device_extensions_ray_query));
	max_extension_count += NUM_OPTIONAL_DEVICE_EXTENSIONS;

	const char** device_extensions = alloca(sizeof(char*) * max_extension_count);
	uint32_t device_extension_count = 0;

	append_string_list(device_extensions, &device_extension_count, max_extension_count, 
		vk_requested_device_extensions_common, LENGTH(vk_requested_device_extensions_common));

	if (qvk.use_ray_query)
	{
		append_string_list(device_extensions, &device_extension_count, max_extension_count,
			vk_requested_device_extensions_ray_query, LENGTH(vk_requested_device_extensions_ray_query));

		device_features_vk12.pNext = &physical_device_ray_query_features;
	}
	else
	{
		append_string_list(device_extensions, &device_extension_count, max_extension_count,
			vk_requested_device_extensions_ray_pipeline, LENGTH(vk_requested_device_extensions_ray_pipeline));

		device_features_vk12.pNext = &physical_device_rt_pipeline_features;
	}
	
	// Add detected optional device extensions
	for (int i = 0; i < NUM_OPTIONAL_DEVICE_EXTENSIONS; i++) {
		if (available_optional_device_extensions[i])
			append_string_list(device_extensions, &device_extension_count, max_extension_count,
							   optional_device_extension_name + i, 1);
	}

	dev_create_info.enabledExtensionCount = device_extension_count;
	dev_create_info.ppEnabledExtensionNames = device_extensions;

	VkPhysicalDeviceLineRasterizationFeaturesKHR line_rast_feat = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_KHR,
		.smoothLines = VK_TRUE,
	};
	if (qvk.supports_smooth_lines) {
		line_rast_feat.pNext = device_features.pNext;
		device_features.pNext = &line_rast_feat;
	}

	/* create device and queue */
#ifdef REFERENCE_B_B2
	result = B2XR_CreateVulkanDevice(&dev_create_info, qvk.physical_device, &qvk.device);
#else
	result = vkCreateDevice(qvk.physical_device, &dev_create_info, NULL, &qvk.device);
#endif
	if (result != VK_SUCCESS)
	{
        Com_EPrintf("Vulkan device startup result=%s\n",qvk_result_to_string(result));
#ifdef REFERENCE_B_B2
        Com_Error(ERR_FATAL,"%s",B2XR_StartupMessage555("Vulkan could not create a graphics device. Check your GPU and graphics driver, then relaunch."));
#else
        Com_Error(ERR_FATAL,"Vulkan could not create a graphics device.");
#endif
		return false;
	}

	vkGetDeviceQueue(qvk.device, qvk.queue_idx_graphics, 0, &qvk.queue_graphics);
	vkGetDeviceQueue(qvk.device, qvk.queue_idx_transfer, 0, &qvk.queue_transfer);
#ifdef REFERENCE_B_B2
	if (!B2XR_CreateSession(qvk.instance, qvk.physical_device, qvk.device,
		qvk.queue_idx_graphics, qvk.queue_graphics))
		return false;
#endif

#define VK_EXTENSION_DO(a) \
	q##a = (PFN_##a) vkGetDeviceProcAddr(qvk.device, #a); \
	if(!q##a) { Com_EPrintf("warning: could not load function %s\n", #a); }

	LIST_EXTENSIONS_ACCEL_STRUCT

	if (!qvk.use_ray_query)
	{
		LIST_EXTENSIONS_RAY_PIPELINE
	}

	if(available_optional_device_extensions[OPT_EXT_VK_EXT_DEBUG_MARKER])
	{
		LIST_EXTENSIONS_DEBUG
	}

#undef VK_EXTENSION_DO

	Com_Printf("-----------------------\n");

	qvk.supports_colorspace = available_optional_instance_extensions[OPT_EXT_VK_EXT_SWAPCHAIN_COLOR_SPACE];

	return true;
}

static VkShaderModule
create_shader_module_from_file(const char *name, const char *enum_name, bool is_rt_shader)
{
	const char* suffix = "";
	if (is_rt_shader)
	{
		if (qvk.use_ray_query)
			suffix = ".query";
		else
			suffix = ".pipeline";
	}

	char path[1024];
	snprintf(path, sizeof path, "shader_vkpt/%s%s.spv", name ? name : (enum_name + 8), suffix);
	if(!name) {
		int len = 0;
		for(len = 0; path[len]; len++)
			path[len] = tolower(path[len]);
		while(--len >= 0) {
			if(path[len] == '_') {
				path[len] = '.';
				break;
			}
		}
	}

	char *data;
	size_t size;

	size = FS_LoadFile(path, (void**)&data);
	if(!data) {
		Com_EPrintf("Couldn't find shader module %s!\n", path);
		return VK_NULL_HANDLE;
	}

	VkShaderModule module = VK_NULL_HANDLE;

	VkShaderModuleCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = size,
		.pCode = (uint32_t *) data,
	};

	VkResult result = vkCreateShaderModule(qvk.device, &create_info, NULL,
		&module);
	if (result == VK_ERROR_DEVICE_LOST)
	{
		vr_device_lost_terminal = true;
		vr_abandon_children = true;
	}

	Z_Free(data);

	return module;
}

VkResult
vkpt_load_shader_modules()
{
	VkResult ret = VK_SUCCESS;
#define SHADER_MODULE_DO(a) do { \
	qvk.shader_modules[a] = create_shader_module_from_file(shader_module_file_names[a], #a, IS_RT_SHADER); \
	ret = (ret == VK_SUCCESS && qvk.shader_modules[a]) ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED; \
	if(qvk.shader_modules[a]) { \
		ATTACH_LABEL_VARIABLE_NAME((uint64_t)qvk.shader_modules[a], SHADER_MODULE, #a); \
	}\
	} while(0);

#define IS_RT_SHADER false
	LIST_SHADER_MODULES;
#undef IS_RT_SHADER
#define IS_RT_SHADER true
	LIST_RT_RGEN_SHADER_MODULES
	if(!qvk.use_ray_query)
	{
		LIST_RT_PIPELINE_SHADER_MODULES
	}
#undef IS_RT_SHADER

#undef SHADER_MODULE_DO
	return ret;
}

VkResult
vkpt_destroy_shader_modules()
{
	for (int i = 0; i < NUM_QVK_SHADER_MODULES; i++)
	{
		vkDestroyShaderModule(qvk.device, qvk.shader_modules[i], NULL);
		qvk.shader_modules[i] = VK_NULL_HANDLE;
	}

	return VK_SUCCESS;
}

VkResult
destroy_swapchain(void)
{
	for(int i = 0; qvk.swap_chain_image_views && i < qvk.num_swap_chain_images; i++) {
		vkDestroyImageView  (qvk.device, qvk.swap_chain_image_views[i], NULL);
		qvk.swap_chain_image_views[i] = VK_NULL_HANDLE;
	}
	free(qvk.swap_chain_image_views);
	qvk.swap_chain_image_views = NULL;

	free(qvk.swap_chain_images);
	qvk.swap_chain_images = NULL;

	qvk.num_swap_chain_images = 0;

	vkDestroySwapchainKHR(qvk.device, qvk.swap_chain, NULL);
	qvk.swap_chain = VK_NULL_HANDLE;

	return VK_SUCCESS;
}

int
destroy_vulkan(void)
{
#ifdef REFERENCE_B_B2
	B2XR_Shutdown();
#endif
	bool abandon_children = false;
	abandon_children = vr_abandon_children;
	if (!abandon_children)
	{
		
		VkResult idle_result = vkDeviceWaitIdle(qvk.device);
		
		if (idle_result != VK_SUCCESS)
		{
			if (idle_result == VK_ERROR_DEVICE_LOST)
				vr_device_lost_terminal = true;
			vr_abandon_children = true;
			abandon_children = true;
			Com_EPrintf("VR: terminal Vulkan teardown idle result %s; child resources abandoned\n",
				qvk_result_to_string(idle_result));
		}
	}
	if (!abandon_children)
	{
		
		destroy_swapchain();
		for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
		{
			for (int gpu = 0; gpu < qvk.device_count; gpu++)
			{
				semaphore_group_t* group = &qvk.semaphores[frame][gpu];

				vkDestroySemaphore(qvk.device, group->image_available, NULL);
				
				vkDestroySemaphore(qvk.device, group->render_finished, NULL);
				vkDestroySemaphore(qvk.device, group->transfer_finished, NULL);
				vkDestroySemaphore(qvk.device, group->trace_finished, NULL);
			}
		}

		for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyFence(qvk.device, qvk.fences_frame_sync[i], NULL);
		}

		vkpt_free_command_buffers(&qvk.cmd_buffers_graphics);
		vkpt_free_command_buffers(&qvk.cmd_buffers_transfer);

		vkDestroyCommandPool(qvk.device, qvk.cmd_buffers_graphics.command_pool, NULL);
		vkDestroyCommandPool(qvk.device, qvk.cmd_buffers_transfer.command_pool, NULL);
		
	}
	else
	{
		/* A terminal technical submission has no proved completion.  Child
		 * objects remain untouched and are reclaimed by vkDestroyDevice. */
		free(qvk.swap_chain_image_views);
		qvk.swap_chain_image_views = NULL;
		free(qvk.swap_chain_images);
		qvk.swap_chain_images = NULL;
		qvk.num_swap_chain_images = 0;
	}

	vkDestroyDevice(qvk.device,   NULL);
	qvk.device = VK_NULL_HANDLE;
	vr_has_submitted = false;
	
	vkDestroySurfaceKHR(qvk.instance, qvk.surface, NULL);
	qvk.surface = VK_NULL_HANDLE;
	_VK(qvkDestroyDebugUtilsMessengerEXT(qvk.instance, qvk.dbg_messenger, NULL));
	vkDestroyInstance(qvk.instance, NULL);
	qvk.instance = VK_NULL_HANDLE;

	free(qvk.extensions);
	qvk.extensions = NULL;
	qvk.num_extensions = 0;

	free(qvk.layers);
	qvk.layers = NULL;
	qvk.num_layers = 0;

	// Clear the extension function pointers to make sure they don't refer non-requested extensions after vid_restart
#define VK_EXTENSION_DO(a) q##a = NULL;
	LIST_EXTENSIONS_ACCEL_STRUCT
	LIST_EXTENSIONS_RAY_PIPELINE
	LIST_EXTENSIONS_DEBUG
	LIST_EXTENSIONS_INSTANCE
#undef VK_EXTENSION_DO

	return 0;
}

typedef struct entity_hash_s {
	unsigned int mesh : 8;
	unsigned int model : 9;
	unsigned int entity : 14;
	unsigned int bsp : 1;
} entity_hash_t;

static int entity_frame_num = 0;
static uint32_t model_entity_ids[2][MAX_MODEL_INSTANCES];
static int model_entity_id_count[2];
static int iqm_matrix_count[2];
static ModelInstance model_instances_prev[MAX_MODEL_INSTANCES];

#ifdef REFERENCE_B_B2
/* Live stereo state has no replay admission, capture, or B1 ledger ownership. */
typedef struct {
    uint32_t identity;
    float transform[16];
    uint32_t current_pose, previous_pose;
    float lerp;
    int iqm_offset;
} b2_weapon_prior_t;

typedef struct b2_cadence_state_s {
	bool pair_prepared;
	uint32_t logical_index, view_count;
	uint32_t history_generation;
	uint32_t rendered_mask;
	uint32_t finished_mask;
	uint32_t prepares, light_uploads, light_clears, bloom_updates, tone_updates;
	uint32_t tone_applies[B2_MAX_VIEWS];
	float adapted_luminance;
	bool sun_visible;
	bool update_world_animations;
	int old_world_anim_frame;
	int new_world_anim_frame;
	vec3_t sky_matrix[3];
	sun_light_t sun_light;
	ref_feedback_t feedback;
	vkpt_bloom_snapshot_t bloom;
	EntityUploadInfo upload_info;
	InstanceBuffer role_instances[B2_MAX_VIEWS];
	IqmMatrixBuffer iqm_snapshot;
	uint32_t pair_entity_ids[MAX_MODEL_INSTANCES];
	int pair_entity_count;
	b2_weapon_prior_t weapon_prior[B2_MAX_VIEWS][MAX_MODEL_INSTANCES];
	int weapon_prior_count[B2_MAX_VIEWS];
} b2_cadence_state_t;
static b2_cadence_state_t b2_cadence;
static int b2_particle_counts[B2_MAX_VIEWS];
// Diagnostic copy of each eye's actual UBO input; no effect/history state.
static vec4_t b2_screen_blends[B2_MAX_VIEWS];

static void b2_note_particle_upload(const refdef_t *fd)
{
    int particles, beams, sprites;
    get_transparency_counts(&particles, &beams, &sprites);
    if (!fd->b2_frame || fd->b2_eye >= fd->b2_frame->view_count ||
        fd->num_particles < 0 || fd->num_particles > MAX_PARTICLES ||
        particles != fd->num_particles ||
        (fd->b2_eye > 0 && particles != b2_particle_counts[0]))
        Com_Error(ERR_FATAL, "B2_FAIL: particle upload differs from sampled stereo scene");
    b2_particle_counts[fd->b2_eye] = particles;
}
// One desktop acquisition spans two physical eye frames. No extra image or
// semaphore: FinishEye waits for the graphics queue before releasing each XR eye.
static struct {
	bool held, wait_acquire, no_image534;
	VkSwapchainKHR swapchain;
	VkExtent2D extent;
	uint32_t image, logical_index;
	uint64_t left_frame;
} b2_mirror;
extern bool B2_UISurfaceActive(void);
extern bool B2_RealignActive391(void);
static struct {
    bool open;
    uint32_t views,logical,generation,rendered,finished;
} b2_surface_pair;

static bool b2_mirror_reuse(void)
{
	const refdef_t *fd = vkpt_refdef.fd;
	bool right = fd && fd->b2_active && fd->b2_eye > 0;
	if (b2_mirror.held) {
		if (!right || !fd->b2_frame || ((fd->rdflags & RDF_NOWORLDMODEL) ? b2_surface_pair.finished : b2_cadence.finished_mask) != ((1u << fd->b2_eye) - 1u) ||
			fd->b2_frame->logical_index != b2_mirror.logical_index ||
			qvk.swap_chain != b2_mirror.swapchain ||
			qvk.current_swap_chain_image_index != b2_mirror.image ||
			!extents_equal(vkpt_get_present_extent(), b2_mirror.extent) ||
			qvk.frame_counter != b2_mirror.left_frame + fd->b2_eye)
			Com_Error(ERR_FATAL, "B2_FAIL: incomplete desktop pair cannot be reused; restart required");
		return true;
	}
	if (right)
		Com_Error(ERR_FATAL, "B2_FAIL: right desktop eye lacks its acquired left image");
	return false;
}
#endif

// Called only before acquire/reset, or after the final submission/present.
// Once disabled, no_image534 keeps both acquire and render-finished semaphores
// unused. A failed present may leave its binary semaphore signalled; retain it
// until ordinary idle shutdown rather than guessing its state or reusing it.
static void b2_disable_desktop582(const char *operation, VkResult result)
{
    Com_EPrintf("B2_DESKTOP_FAILURE operation=%s result=%s\n", operation, qvk_result_to_string(result));
    if (result == VK_ERROR_DEVICE_LOST) {
        vr_device_lost_terminal = vr_abandon_children = true;
        Com_Error(ERR_FATAL, "The graphics device stopped responding. Restart Q2RTX VR after checking the graphics driver.");
    }
    if (result != VK_ERROR_SURFACE_LOST_KHR && result != VK_ERROR_NATIVE_WINDOW_IN_USE_KHR &&
        result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_ERROR_OUT_OF_DEVICE_MEMORY &&
        result != VK_ERROR_OUT_OF_HOST_MEMORY && result != VK_ERROR_FORMAT_NOT_SUPPORTED &&
        result != VK_ERROR_INITIALIZATION_FAILED && result != VK_INCOMPLETE) {
        vr_abandon_children = true;
        Com_Error(ERR_FATAL, "Desktop graphics failed unexpectedly. Restart Q2RTX VR; see the game log for details.");
    }
    if (b2_physical_started580 || b2_mirror.wait_acquire)
        Com_Error(ERR_FATAL, "Desktop graphics could not safely retire an unfinished frame. Restart Q2RTX VR.");
    VkResult idle = vkDeviceWaitIdle(qvk.device);
    if (idle != VK_SUCCESS) {
        vr_device_lost_terminal = idle == VK_ERROR_DEVICE_LOST;
        vr_abandon_children = true;
        Com_Error(ERR_FATAL, "Graphics work could not complete. Restart Q2RTX VR after checking the graphics driver.");
    }
    vkpt_draw_destroy_desktop534();
    if (destroy_swapchain() != VK_SUCCESS)
        Com_Error(ERR_FATAL, "Desktop graphics cleanup failed. Restart Q2RTX VR.");
    b2_desktop_failed582 = true;
    b2_desktop_dirty534 = false;
    b2_mirror.held = b2_mirror.wait_acquire = false;
    b2_mirror.no_image534 = true;
    Com_Printf("Desktop mirror disabled for this session; VR continues. Restart the game to restore the mirror.\n");
}

void vkpt_vr_latch_device_loss(void)
{
	vr_device_lost_terminal = true;
	vr_abandon_children = true;
}

void vkpt_vr_abandon_children(void)
{
	vr_abandon_children = true;
}

#define MAX_MODEL_LIGHTS 16384
static int num_model_lights = 0;
static light_poly_t model_lights[MAX_MODEL_LIGHTS];

static pbr_material_t const * get_mesh_material(const entity_t* entity, const maliasmesh_t* mesh)
{
	if (entity->skin)
	{
		return MAT_ForSkin(IMG_ForHandle(entity->skin));
	}

	int skinnum = 0;
	if (mesh->materials[entity->skinnum])
		skinnum = entity->skinnum;

	return mesh->materials[skinnum];
}

typedef struct {
	uint32_t material_id;
	uint32_t shell;
} material_and_shell_t;

static material_and_shell_t compute_mesh_material_flags(const entity_t* entity, const model_t* model,
	const maliasmesh_t* mesh, bool is_viewer_weapon, bool is_double_sided, float alpha)
{
	pbr_material_t const* material = get_mesh_material(entity, mesh);
	material_and_shell_t mat_shell = {.material_id = 0, .shell = 0};

	if (!material)
	{
		Com_EPrintf("Cannot find material for model '%s'\n", model->name);
		return mat_shell;
	}

	uint32_t material_id = material->flags;

	if (MAT_IsKind(material_id, MATERIAL_KIND_INVISIBLE))
		return mat_shell; // skip the mesh

	if (MAT_IsKind(material_id, MATERIAL_KIND_CHROME))
		material_id = MAT_SetKind(material_id, MATERIAL_KIND_CHROME_MODEL);

	if (MAT_IsKind(material_id, MATERIAL_KIND_TRANSPARENT) || (MAT_IsKind(material_id, MATERIAL_KIND_REGULAR) && (alpha < 1.0f)))
		material_id = MAT_SetKind(material_id, MATERIAL_KIND_TRANSP_MODEL);

	if (model->model_class == MCLASS_EXPLOSION)
	{
		material_id = MAT_SetKind(material_id, MATERIAL_KIND_EXPLOSION);
		material_id |= MATERIAL_FLAG_LIGHT;
	}

	if (is_viewer_weapon)
		material_id |= MATERIAL_FLAG_WEAPON;

	if (is_double_sided)
		material_id |= MATERIAL_FLAG_DOUBLE_SIDED;

	if (!MAT_IsKind(material_id, MATERIAL_KIND_GLASS))
	{
	#if USE_DEBUG
		if (cvar_pt_test_shell->integer != 0)
			mat_shell.shell = cvar_pt_test_shell->integer;
	#endif

		if ((entity->flags & RF_IR_VISIBLE) && (vkpt_refdef.fd->rdflags & RDF_IRGOGGLES)) {
			// IR googgles: force red shell
			mat_shell.shell |= SHELL_RED;
		} else {
			if (entity->flags & RF_SHELL_HALF_DAM)
				mat_shell.shell |= SHELL_HALF_DAM;
			if (entity->flags & RF_SHELL_DOUBLE)
				mat_shell.shell |= SHELL_DOUBLE;
			if (entity->flags & RF_SHELL_RED)
				mat_shell.shell |= SHELL_RED;
			if (entity->flags & RF_SHELL_GREEN)
				mat_shell.shell |= SHELL_GREEN;
			if (entity->flags & RF_SHELL_BLUE)
				mat_shell.shell |= SHELL_BLUE;
		}
	}

	if (mesh->handedness)
		material_id |= MATERIAL_FLAG_HANDEDNESS;

	mat_shell.material_id = material_id;

	return mat_shell;
}

static void fill_model_instance(ModelInstance* instance, const entity_t* entity, const model_t* model, const maliasmesh_t* mesh,
	const float* transform, material_and_shell_t mat_shell, int instance_index, int iqm_matrix_index)
{
	int cluster = -1;
	if (bsp_world_model)
		cluster = BSP_PointLeaf(bsp_world_model->nodes, entity->origin)->cluster;
	
	int frame = entity->frame;
	int oldframe = entity->oldframe;
	if (frame >= model->numframes) frame = 0;
	if (oldframe >= model->numframes) oldframe = 0;

	memcpy(instance->transform, transform, sizeof(float) * 16);
	memcpy(instance->transform_prev, transform, sizeof(float) * 16);
	instance->material = mat_shell.material_id;
	instance->shell = mat_shell.shell;
	instance->cluster = cluster;
	instance->source_buffer_idx = (int)(model - r_models) + VERTEX_BUFFER_FIRST_MODEL;
	instance->prim_count = mesh->numtris;
	instance->prim_offset_curr_pose_curr_frame = mesh->tri_offset + frame * mesh->numtris;
	instance->prim_offset_prev_pose_curr_frame = mesh->tri_offset + oldframe * mesh->numtris;
	instance->prim_offset_curr_pose_prev_frame = instance->prim_offset_curr_pose_curr_frame;
	instance->prim_offset_prev_pose_prev_frame = instance->prim_offset_prev_pose_curr_frame;
	instance->pose_lerp_curr_frame = entity->backlerp;
	instance->pose_lerp_prev_frame = instance->pose_lerp_curr_frame;
	instance->iqm_matrix_offset_curr_frame = iqm_matrix_index;
	instance->iqm_matrix_offset_prev_frame = instance->iqm_matrix_offset_curr_frame;
	instance->alpha_and_frame = floatToHalf((entity->flags & RF_TRANSLUCENT) ? entity->alpha : 1.0f);
	instance->render_buffer_idx = 0; // to be filled later
	instance->render_prim_offset = 0;

	// If this is a static wall light model, the renderer creates a custom set of light polys
	// for this model. Mark the material with the light flag to avoid double contribution and noise
	// from the GI rays. This (together with the custom lights) is a hack that should be replaced
	// by a better model that has a separate mesh for the light rod.
	if (model->model_class == MCLASS_STATIC_LIGHT)
		instance->material |= MATERIAL_FLAG_LIGHT;
}

static void add_dlight_spot(const dlight_t* light, DynLightData* dynlight_data)
{
	// Copy spot data
	VectorCopy(light->spot.direction, dynlight_data->spot_direction);
	switch(light->spot.emission_profile)
	{
	case DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF:
		dynlight_data->type |= DYNLIGHT_SPOT_EMISSION_PROFILE_FALLOFF << 16;
		dynlight_data->spot_data = floatToHalf(light->spot.cos_total_width) | (floatToHalf(light->spot.cos_falloff_start) << 16);
		break;
	case DLIGHT_SPOT_EMISSION_PROFILE_AXIS_ANGLE_TEXTURE:
		dynlight_data->type |= DYNLIGHT_SPOT_EMISSION_PROFILE_AXIS_ANGLE_TEXTURE << 16;
		dynlight_data->spot_data = floatToHalf(light->spot.total_width) | (light->spot.texture << 16);
		break;
	}
}

static void
add_dlights(const dlight_t* lights, int num_lights, QVKUniformBuffer_t* ubo)
{
	ubo->num_dyn_lights = 0;

	for (int i = 0; i < num_lights; i++)
	{
		const dlight_t* light = lights + i;

		DynLightData* dynlight_data = ubo->dyn_light_data + ubo->num_dyn_lights;
		VectorCopy(light->origin, dynlight_data->center);
		VectorScale(light->color, light->intensity / 25.f, dynlight_data->color);
		dynlight_data->radius = light->radius;
		switch(light->light_type) {
		case DLIGHT_SPHERE:
			dynlight_data->type = DYNLIGHT_SPHERE;
			break;
		case DLIGHT_SPOT:
			dynlight_data->type = DYNLIGHT_SPOT;
			add_dlight_spot(light, dynlight_data);
			break;
		}

		ubo->num_dyn_lights++;
	}
}

static inline void transform_point(const float* p, const float* matrix, float* result)
{
	vec4_t point = { p[0], p[1], p[2], 1.f };
	vec4_t transformed;
	mult_matrix_vector(transformed, matrix, point);
	VectorCopy(transformed, result); // vec4 -> vec3
}

static void instance_model_lights(int num_light_polys, const light_poly_t* light_polys, const float* transform)
{
	for (int nlight = 0; nlight < num_light_polys; nlight++)
	{
		if (num_model_lights >= MAX_MODEL_LIGHTS)
		{
			assert(!"Model light count overflow");
			break;
		}

		const light_poly_t* src_light = light_polys + nlight;
		light_poly_t* dst_light = model_lights + num_model_lights;

		// Transform the light's positions and center
		transform_point(src_light->positions + 0, transform, dst_light->positions + 0);
		transform_point(src_light->positions + 3, transform, dst_light->positions + 3);
		transform_point(src_light->positions + 6, transform, dst_light->positions + 6);
		transform_point(src_light->off_center, transform, dst_light->off_center);

		// Find the cluster based on the center. Maybe it's OK to use the model's cluster, need to test.
		dst_light->cluster = BSP_PointLeaf(bsp_world_model->nodes, dst_light->off_center)->cluster;

		// We really need to map these lights to a cluster
		if (dst_light->cluster < 0)
			continue;

		// Copy the other light properties
		VectorCopy(src_light->color, dst_light->color);
		dst_light->material = src_light->material;
		dst_light->style = src_light->style;

		num_model_lights++;
	}
}
static const mat4 g_identity_transform = {
	{ 1.f, 0.f, 0.f, 0.f },
	{ 0.f, 1.f, 0.f, 0.f },
	{ 0.f, 0.f, 1.f, 0.f },
	{ 0.f, 0.f, 0.f, 1.f }
};

static void process_bsp_entity(const entity_t* entity, int* instance_count)
{
	InstanceBuffer* uniform_instance_buffer = &vkpt_refdef.uniform_instance_buffer;

	const int current_instance_idx = *instance_count;
	if (current_instance_idx >= MAX_MODEL_INSTANCES)
	{
		assert(!"Entity count overflow");
		return;
	}
	
	float transform[16];
	create_entity_matrix(transform, (entity_t*)entity);

	bsp_model_t* model = vkpt_refdef.bsp_mesh_world.models + (~entity->model);

	vec3_t origin;
	transform_point(model->center, transform, origin);
	int cluster = BSP_PointLeaf(bsp_world_model->nodes, origin)->cluster;

	if (cluster < 0)
	{
		// In some cases, a model slides into a wall, like a push button, so that its center 
		// is no longer in any BSP node. We still need to assign a cluster to the model,
		// so try the corners of the model instead, see if any of them has a valid cluster.

		for (int corner = 0; corner < 8; corner++)
		{
			vec3_t corner_pt = {
				(corner & 1) ? model->aabb_max[0] : model->aabb_min[0],
				(corner & 2) ? model->aabb_max[1] : model->aabb_min[1],
				(corner & 4) ? model->aabb_max[2] : model->aabb_min[2]
			};

			vec3_t corner_pt_world;
			transform_point(corner_pt, transform, corner_pt_world);

			cluster = BSP_PointLeaf(bsp_world_model->nodes, corner_pt_world)->cluster;

			if (cluster >= 0)
				break;
		}
	}

	entity_hash_t hash;
	hash.entity = entity->id;
	hash.model = ~entity->model;
	hash.mesh = 0;
	hash.bsp = 1;

	memcpy(&model_entity_ids[entity_frame_num][current_instance_idx], &hash, sizeof(uint32_t));

	float model_alpha = (entity->flags & RF_TRANSLUCENT) ? entity->alpha : 1.f;
	ModelInstance* mi = uniform_instance_buffer->model_instances + current_instance_idx;
	memcpy(&mi->transform, transform, sizeof(transform));
	memcpy(&mi->transform_prev, transform, sizeof(transform));
	mi->material = 0;
	mi->cluster = cluster;
	mi->source_buffer_idx = VERTEX_BUFFER_WORLD;
	mi->prim_count = model->geometry.prim_counts[0];
	mi->prim_offset_curr_pose_curr_frame = 0; // bsp models are not processed by the instancing shader
	mi->prim_offset_prev_pose_curr_frame = 0;
	mi->prim_offset_curr_pose_prev_frame = 0;
	mi->prim_offset_prev_pose_prev_frame = 0;
	mi->pose_lerp_curr_frame = 0.f;
	mi->pose_lerp_prev_frame = 0.f;
	mi->iqm_matrix_offset_curr_frame = -1;
	mi->iqm_matrix_offset_prev_frame = -1;
	mi->alpha_and_frame = (entity->frame << 16) | floatToHalf(model_alpha);
	mi->render_buffer_idx = VERTEX_BUFFER_WORLD;
	mi->render_prim_offset = model->geometry.prim_offsets[0];
	
	instance_model_lights(model->num_light_polys, model->light_polys, transform);

	if (model->geometry.accel)
	{
		vkpt_pt_instance_model_blas(&model->geometry, mi->transform, VERTEX_BUFFER_WORLD, current_instance_idx, (model_alpha < 1.f) ? AS_FLAG_TRANSPARENT : 0);
	}

	if (!model->transparent)
	{
		vkpt_shadow_map_add_instance(transform, qvk.buf_world.buffer, vkpt_refdef.bsp_mesh_world.vertex_data_offset
			+ mi->render_prim_offset * sizeof(prim_positions_t), mi->prim_count);
	}

	(*instance_count)++;
}

#define MESH_FILTER_TRANSPARENT 1
#define MESH_FILTER_OPAQUE 2
#define MESH_FILTER_MASKED 4
#define MESH_FILTER_ALL 7

// Range selection changes only instances, never loaded geometry or bindings.
static void b2_apply_grenade_part(ModelInstance *instance, const maliasmesh_t *mesh,
    const b2wm_part_t *part)
{
    memcpy(instance->transform,part->matrix,sizeof(instance->transform));
    memcpy(instance->transform_prev,part->matrix,sizeof(instance->transform_prev));
    instance->prim_count=part->count;
    instance->prim_offset_curr_pose_curr_frame=mesh->tri_offset+part->frame*mesh->numtris+part->first;
    instance->prim_offset_prev_pose_curr_frame=mesh->tri_offset+part->oldframe*mesh->numtris+part->first;
    instance->prim_offset_curr_pose_prev_frame=instance->prim_offset_curr_pose_curr_frame;
    instance->prim_offset_prev_pose_prev_frame=instance->prim_offset_prev_pose_curr_frame;
    instance->pose_lerp_curr_frame=instance->pose_lerp_prev_frame=part->backlerp;
}
static void process_regular_entity(
	const entity_t* entity, 
	const model_t* model, 
	bool is_viewer_weapon, 
	bool is_double_sided, 
	int* instance_count, 
	int* animated_count, 
	int* num_instanced_prim, 
	int mesh_filter, 
	bool* contains_transparent,
	bool* contains_masked,
	int* iqm_matrix_offset,
	float* iqm_matrix_data)
{
	InstanceBuffer* uniform_instance_buffer = &vkpt_refdef.uniform_instance_buffer;

	float transform[16];
	if (is_viewer_weapon)
		create_viewweapon_matrix(transform, (entity_t *)entity);
	else
		create_entity_matrix(transform, (entity_t*)entity);
	
	int current_instance_index = *instance_count;
	int current_animated_index = *animated_count;
	int current_num_instanced_prim = *num_instanced_prim;

	if (contains_transparent)
		*contains_transparent = false;

    b2wm_part_t grenade_parts[3];
    bool grenade=is_viewer_weapon && B2WP_ViewRequested() &&
        B2WP_ViewPart(entity->model,model,entity->frame,entity->oldframe,entity->backlerp,0,grenade_parts);
    if (grenade) {
        for (unsigned p=1;p<3;p++)
            if (!B2WP_ViewPart(entity->model,model,entity->frame,entity->oldframe,entity->backlerp,p,grenade_parts+p)) return;
        unsigned count=2+(unsigned)grenade_parts[1].visible;
        if (model->nummeshes!=1 || model->iqmData || model->num_light_polys ||
            current_instance_index+count>MAX_MODEL_INSTANCES || current_animated_index+count>MAX_MODEL_INSTANCES) return;
    }
	int iqm_matrix_index = -1;
	if (model->iqmData && model->iqmData->num_poses)
	{
		iqm_matrix_index = *iqm_matrix_offset;
		
		if (iqm_matrix_index + model->iqmData->num_poses > MAX_IQM_MATRICES)
		{
			assert(!"IQM matrix buffer overflow");
			return;
		}
		
		R_ComputeIQMTransforms(model->iqmData, entity, iqm_matrix_data + (iqm_matrix_index * 12));
		
		*iqm_matrix_offset += (int)model->iqmData->num_poses;
	}

	float alpha = (entity->flags & RF_TRANSLUCENT) ? entity->alpha : 1.f;

	bool use_static_blas = !grenade && vkpt_model_is_static(model) && (mesh_filter != MESH_FILTER_ALL);

	const model_vbo_t* vbo = vkpt_get_model_vbo(model);

	if (use_static_blas)
	{
		const model_geometry_t* geom = NULL;

		if (mesh_filter & MESH_FILTER_MASKED)
			geom = &vbo->geom_masked;
		else if (mesh_filter & MESH_FILTER_TRANSPARENT)
			geom = &vbo->geom_transparent;
		else
			geom = &vbo->geom_opaque;
		
		if (geom->accel)
		{
			// ugly typecast
			mat4 transform_;
			memcpy(transform_, transform, sizeof(mat4));

			uint32_t model_index = (uint32_t)(model - r_models);

			vkpt_pt_instance_model_blas(geom, transform_, VERTEX_BUFFER_FIRST_MODEL + model_index, current_instance_index, (alpha < 1.f) ? AS_FLAG_TRANSPARENT : 0);
		}
	}

	for (int i = 0; i < model->nummeshes; i++)
	{
		const maliasmesh_t* mesh = model->meshes + i;

		if (current_instance_index >= MAX_MODEL_INSTANCES)
		{
			assert(!"Model instance count overflow");
			break;
		}

		if (!use_static_blas && current_animated_index >= MAX_MODEL_INSTANCES)
		{
			assert(!"Animated model count overflow");
			break;
		}

		if (mesh->tri_offset < 0)
		{
			// failed to upload the vertex data - don't instance this mesh
			continue;
		}

		material_and_shell_t mat_shell = compute_mesh_material_flags(entity, model, mesh, is_viewer_weapon, is_double_sided, alpha);

		if (!mat_shell.material_id)
			continue;

		if (MAT_IsMasked(mat_shell.material_id))
		{
			if (contains_masked)
				*contains_masked = true;

			if (!(mesh_filter & MESH_FILTER_MASKED))
				continue;
		}
		else if (MAT_IsTransparent(mat_shell.material_id) || (alpha < 1.0f))
		{
			if(contains_transparent)
				*contains_transparent = true;

			if(!(mesh_filter & MESH_FILTER_TRANSPARENT))
				continue;
		}
		else
		{
			if (!(mesh_filter & MESH_FILTER_OPAQUE))
				continue;
		}

        for (unsigned p=0;p<(grenade?3u:1u);p++) {
        if (grenade && !grenade_parts[p].visible) continue;
		entity_hash_t hash;
		hash.entity = entity->id;
		hash.model = entity->model;
		hash.mesh = grenade ? 128u+p : i;
		hash.bsp = 0;

		memcpy(&model_entity_ids[entity_frame_num][current_instance_index], &hash, sizeof(uint32_t));
		
		ModelInstance* mi = uniform_instance_buffer->model_instances + current_instance_index;

		fill_model_instance(mi, entity, model, mesh, transform, mat_shell,
			current_instance_index, iqm_matrix_index);
        if (grenade) b2_apply_grenade_part(mi,mesh,grenade_parts+p);

		if (use_static_blas)
		{
			mi->render_buffer_idx = mi->source_buffer_idx;
			mi->render_prim_offset = mi->prim_offset_curr_pose_curr_frame;

			if (!MAT_IsTransparent(mat_shell.material_id))
			{
				vkpt_shadow_map_add_instance(transform, vbo->buffer.buffer, vbo->vertex_data_offset
					+ mi->render_prim_offset * sizeof(prim_positions_t), mi->prim_count);
			}
		}
		else
		{
			uniform_instance_buffer->animated_model_indices[current_animated_index] = current_instance_index;

			mi->render_buffer_idx = VERTEX_BUFFER_INSTANCED;
			mi->render_prim_offset = current_num_instanced_prim;

			current_animated_index++;
			current_num_instanced_prim += mi->prim_count;
		}

		current_instance_index++;
        }
	}

	// add cylinder lights for wall lamps
	if (model->model_class == MCLASS_STATIC_LIGHT)
	{
		vec4_t begin, end, color;
		vec4_t offset1 = { 0.f, 0.5f, -10.f, 1.f };
		vec4_t offset2 = { 0.f, 0.5f,  10.f, 1.f };

		mult_matrix_vector(begin, transform, offset1);
		mult_matrix_vector(end, transform, offset2);
		VectorSet(color, 0.25f, 0.5f, 0.07f);

		vkpt_build_cylinder_light(model_lights, &num_model_lights, MAX_MODEL_LIGHTS, bsp_world_model, begin, end, color, 1.5f);
	}

	*instance_count = current_instance_index;
	*animated_count = current_animated_index;
	*num_instanced_prim = current_num_instanced_prim;
}

static void
prepare_entities(EntityUploadInfo* upload_info)
{
	entity_frame_num = !entity_frame_num;

	InstanceBuffer* instance_buffer = &vkpt_refdef.uniform_instance_buffer;
	
	static int transparent_model_indices[MAX_ENTITIES];
	static int masked_model_indices[MAX_ENTITIES];
	static int viewer_model_indices[MAX_ENTITIES];
	static int viewer_weapon_indices[MAX_ENTITIES];
	static int explosion_indices[MAX_ENTITIES];
	int transparent_model_num = 0;
	int masked_model_num = 0;
	int viewer_model_num = 0;
	int viewer_weapon_num = 0;
	int explosion_num = 0;

	int model_instance_idx = 0;
	int num_instanced_prim = 0; /* need to track this here to find lights */
	int instance_idx = 0;
	int iqm_matrix_offset = 0;

	const bool first_person_model = (cl_player_model->integer == CL_PLAYER_MODEL_FIRST_PERSON) && cl.baseclientinfo.model;
    // Primary preparation runs once per logical frame. Retired/stale command
    // snapshots disappear here; every render role reuses the same admitted set.
    const bool weapon_eligible=B2WP_FrameReady(Sys_Milliseconds());

	for (int i = 0; i < vkpt_refdef.fd->num_entities; i++)
	{
		const entity_t* entity = vkpt_refdef.fd->entities + i;

		if (entity->model & 0x80000000)
		{
			process_bsp_entity(entity, &model_instance_idx); /* embedded in bsp */
		}
		else
		{
			const model_t* model = MOD_ForHandle(entity->model);
			if (model == NULL || model->meshes == NULL)
				continue;

			if (entity->flags & RF_VIEWERMODEL)
				viewer_model_indices[viewer_model_num++] = i;
			else if (entity->flags & RF_WEAPONMODEL) {
                if (!weapon_eligible) continue;
				viewer_weapon_indices[viewer_weapon_num++] = i;
            }
			else if (model->model_class == MCLASS_EXPLOSION || model->model_class == MCLASS_FLASH)
				explosion_indices[explosion_num++] = i;
			else
			{
				bool contains_transparent = false;
				bool contains_masked = false;
				process_regular_entity(entity, model, false, false, &model_instance_idx, &instance_idx, &num_instanced_prim,
					MESH_FILTER_OPAQUE, &contains_transparent, &contains_masked, &iqm_matrix_offset, qvk.iqm_matrices_shadow);

				if (contains_transparent)
					transparent_model_indices[transparent_model_num++] = i;
				if (contains_masked)
					masked_model_indices[masked_model_num++] = i;
			}

			if (model->num_light_polys > 0)
			{
				float transform[16];
				const bool is_viewer_weapon = (entity->flags & RF_WEAPONMODEL) != 0;
				if (is_viewer_weapon)
					create_viewweapon_matrix(transform, (entity_t*)entity);
				else
					create_entity_matrix(transform, (entity_t*)entity);

				instance_model_lights(model->num_light_polys, model->light_polys, transform);
			}
		}
	}

	upload_info->opaque_prim_count = num_instanced_prim;
	upload_info->transparent_prim_offset = num_instanced_prim;
	
	for (int i = 0; i < transparent_model_num; i++)
	{
		const entity_t* entity = vkpt_refdef.fd->entities + transparent_model_indices[i];

		const model_t* model = MOD_ForHandle(entity->model);
		process_regular_entity(entity, model, false, false, &model_instance_idx, &instance_idx, &num_instanced_prim,
			MESH_FILTER_TRANSPARENT, NULL, NULL, &iqm_matrix_offset, qvk.iqm_matrices_shadow);
	}

	upload_info->transparent_prim_count = num_instanced_prim - upload_info->transparent_prim_offset;
	upload_info->masked_prim_offset = num_instanced_prim;

	for (int i = 0; i < masked_model_num; i++)
	{
		const entity_t* entity = vkpt_refdef.fd->entities + masked_model_indices[i];
		
		const model_t* model = MOD_ForHandle(entity->model);
		process_regular_entity(entity, model, false, true, &model_instance_idx, &instance_idx, &num_instanced_prim,
			MESH_FILTER_MASKED, NULL, NULL, &iqm_matrix_offset, qvk.iqm_matrices_shadow);
	}

	upload_info->masked_prim_count = num_instanced_prim - upload_info->masked_prim_offset;
	upload_info->viewer_model_prim_offset = num_instanced_prim;
	
	if (first_person_model)
	{
		for (int i = 0; i < viewer_model_num; i++)
		{
			const entity_t* entity = vkpt_refdef.fd->entities + viewer_model_indices[i];
			const model_t* model = MOD_ForHandle(entity->model);
			process_regular_entity(entity, model, false, true, &model_instance_idx, &instance_idx, &num_instanced_prim,
				MESH_FILTER_ALL, NULL, NULL, &iqm_matrix_offset, qvk.iqm_matrices_shadow);
		}
	}

	upload_info->viewer_model_prim_count = num_instanced_prim - upload_info->viewer_model_prim_offset;
	upload_info->viewer_weapon_prim_offset = num_instanced_prim;

	upload_info->weapon_left_handed = false;
	
	for (int i = 0; i < viewer_weapon_num; i++)
	{
		const entity_t* entity = vkpt_refdef.fd->entities + viewer_weapon_indices[i];
		const model_t* model = MOD_ForHandle(entity->model);
		process_regular_entity(entity, model, true, false, &model_instance_idx, &instance_idx, &num_instanced_prim,
			MESH_FILTER_ALL, NULL, NULL, &iqm_matrix_offset, qvk.iqm_matrices_shadow);

		if (B2WP_ViewRequested() ? B2WP_LeftHanded() : info_hand->integer == 1)
			upload_info->weapon_left_handed = true;
	}

	upload_info->viewer_weapon_prim_count = num_instanced_prim - upload_info->viewer_weapon_prim_offset;
	upload_info->explosions_prim_offset = num_instanced_prim;
	
	for (int i = 0; i < explosion_num; i++)
	{
		const entity_t* entity = vkpt_refdef.fd->entities + explosion_indices[i];
		const model_t* model = MOD_ForHandle(entity->model);
		process_regular_entity(entity, model, false, false, &model_instance_idx, &instance_idx, &num_instanced_prim,
			MESH_FILTER_ALL, NULL, NULL, &iqm_matrix_offset, qvk.iqm_matrices_shadow);
	}

	upload_info->explosions_prim_count = num_instanced_prim - upload_info->explosions_prim_offset;

	upload_info->num_instances = instance_idx;
	upload_info->num_prims  = num_instanced_prim;
	
	memset(instance_buffer->model_current_to_prev, -1, sizeof(instance_buffer->model_current_to_prev));
	memset(instance_buffer->model_prev_to_current, -1, sizeof(instance_buffer->model_prev_to_current));
	
	model_entity_id_count[entity_frame_num] = model_instance_idx;
	for(int i = 0; i < model_entity_id_count[entity_frame_num]; i++) {
		for(int j = 0; j < model_entity_id_count[!entity_frame_num]; j++) {
			entity_hash_t hash;
			memcpy(&hash, &model_entity_ids[entity_frame_num][i], sizeof(entity_hash_t));

			if(model_entity_ids[entity_frame_num][i] == model_entity_ids[!entity_frame_num][j] && hash.entity != 0u) {
				instance_buffer->model_current_to_prev[i] = j;
				instance_buffer->model_prev_to_current[j] = i;

				// Copy the "prev" instance paramters from the previous frame's instance buffer
				ModelInstance* mi_curr = instance_buffer->model_instances + i;
				ModelInstance* mi_prev = model_instances_prev + j;

				memcpy(mi_curr->transform_prev, mi_prev->transform, sizeof(mi_curr->transform_prev));
				mi_curr->prim_offset_curr_pose_prev_frame = mi_prev->prim_offset_curr_pose_curr_frame;
				mi_curr->prim_offset_prev_pose_prev_frame = mi_prev->prim_offset_prev_pose_curr_frame;
				mi_curr->pose_lerp_prev_frame = mi_prev->pose_lerp_curr_frame;
				mi_curr->iqm_matrix_offset_prev_frame = mi_prev->iqm_matrix_offset_curr_frame;
			}
		}
	}

	// Store the number of IQM matrices for the next frame
	iqm_matrix_count[entity_frame_num] = iqm_matrix_offset;

	if (iqm_matrix_count[entity_frame_num] > 0)
	{
		// If we had some matrices previously...
		if (iqm_matrix_count[!entity_frame_num] > 0)
		{
			// Copy over the previous frame IQM matrices into an offset location in the current frame buffer
			memcpy(qvk.iqm_matrices_shadow + (iqm_matrix_count[entity_frame_num] * 12),
				qvk.iqm_matrices_prev, iqm_matrix_count[!entity_frame_num] * 12 * sizeof(float));

			// Patch the previous matrix offsets to point at the new locations
			for (int i = 0; i < model_entity_id_count[entity_frame_num]; i++)
			{
				ModelInstance* instance = &instance_buffer->model_instances[i];
				if (instance->iqm_matrix_offset_prev_frame >= 0) {
					// Offset = current matrix count
					instance->iqm_matrix_offset_prev_frame += iqm_matrix_count[entity_frame_num];
				}
			}
		}

		// Store the current matrices for the next frame
		memcpy(qvk.iqm_matrices_prev, qvk.iqm_matrices_shadow, iqm_matrix_count[entity_frame_num] * 12 * sizeof(float));

		// Upload the current matrices to the staging buffer
		IqmMatrixBuffer* iqm_matrix_staging = buffer_map(&qvk.buf_iqm_matrices_staging[qvk.current_frame_index]);

		int total_matrix_count = (iqm_matrix_count[entity_frame_num] + iqm_matrix_count[!entity_frame_num]);
		memcpy(iqm_matrix_staging, qvk.iqm_matrices_shadow, total_matrix_count * 12 * sizeof(float));

		buffer_unmap(&qvk.buf_iqm_matrices_staging[qvk.current_frame_index]);
	}

	// Save the current model instances for the next frame
	memcpy(model_instances_prev, instance_buffer->model_instances, sizeof(ModelInstance) * model_entity_id_count[entity_frame_num]);
}

static const entity_t *vr_find_weapon_entity(uint32_t identity)
{
	entity_hash_t hash;
	memcpy(&hash, &identity, sizeof(hash));
	for (int index = 0; index < vkpt_refdef.fd->num_entities; index++)
	{
		const entity_t *entity = vkpt_refdef.fd->entities + index;
		if ((entity->flags & RF_WEAPONMODEL) &&
			(uint32_t)entity->id == hash.entity &&
			((uint32_t)entity->model & 0x1ffu) == hash.model)
			return entity;
	}
	return NULL;
}
static void vr_seed_entity_history(void)
{
	InstanceBuffer *instances = &vkpt_refdef.uniform_instance_buffer;
	int current_count = model_entity_id_count[entity_frame_num];
	int current_iqm_count = iqm_matrix_count[entity_frame_num];
	int previous_slot = !entity_frame_num;

	model_entity_id_count[previous_slot] = current_count;
	memcpy(model_entity_ids[previous_slot], model_entity_ids[entity_frame_num],
		sizeof(uint32_t) * current_count);
	memset(instances->model_current_to_prev, -1,
		sizeof(instances->model_current_to_prev));
	memset(instances->model_prev_to_current, -1,
		sizeof(instances->model_prev_to_current));
	for (int index = 0; index < current_count; index++)
	{
		ModelInstance *instance = instances->model_instances + index;
		instances->model_current_to_prev[index] = index;
		instances->model_prev_to_current[index] = index;
		memcpy(instance->transform_prev, instance->transform,
			sizeof(instance->transform_prev));
		instance->prim_offset_curr_pose_prev_frame = instance->prim_offset_curr_pose_curr_frame;
		instance->prim_offset_prev_pose_prev_frame = instance->prim_offset_prev_pose_curr_frame;
		instance->pose_lerp_prev_frame = instance->pose_lerp_curr_frame;
		instance->iqm_matrix_offset_prev_frame = instance->iqm_matrix_offset_curr_frame;
	}
	memcpy(model_instances_prev, instances->model_instances,
		sizeof(ModelInstance) * current_count);
	if (current_iqm_count > 0)
	{
		memcpy(qvk.iqm_matrices_shadow + current_iqm_count * 12,
			qvk.iqm_matrices_shadow, current_iqm_count * 12 * sizeof(float));
		memcpy(qvk.iqm_matrices_prev, qvk.iqm_matrices_shadow,
			current_iqm_count * 12 * sizeof(float));
	}
	iqm_matrix_count[previous_slot] = current_iqm_count;
}

#ifdef REFERENCE_B_B2
static void b2_restage_instances(uint32_t eye)
{
	memcpy(&vkpt_refdef.uniform_instance_buffer, b2_cadence.role_instances + eye,
		sizeof(vkpt_refdef.uniform_instance_buffer));
	IqmMatrixBuffer *staging = buffer_map(&qvk.buf_iqm_matrices_staging[qvk.current_frame_index]);
	if (!staging)
		Com_Error(ERR_FATAL, "B2_FAIL: cannot stage eye IQM snapshot");
	memcpy(staging, &b2_cadence.iqm_snapshot, sizeof(*staging));
	buffer_unmap(&qvk.buf_iqm_matrices_staging[qvk.current_frame_index]);
}

static void b2_prepare_role_instances(refdef_t *fd)
{
	const b2_frame_t *frame = fd->b2_frame;
    const bool weapon_continuity=B2WP_HistoryMatches();
	const cvar_t *combat_mode = Cvar_FindVar("b2_combat");
	const bool live_weapons = B2_AdvancingWorld() && combat_mode && combat_mode->integer == 1;
	float saved_view[16], saved_inverse[16];
	float saved_fov_x = fd->fov_x, saved_fov_y = fd->fov_y;
	if (!frame->history_valid)
		vr_seed_entity_history();
	memcpy(saved_view, vkpt_refdef.view_matrix, sizeof(saved_view));
	memcpy(saved_inverse, vkpt_refdef.view_matrix_inv, sizeof(saved_inverse));
	b2_cadence.pair_entity_count = model_entity_id_count[entity_frame_num];
	memcpy(b2_cadence.pair_entity_ids, model_entity_ids[entity_frame_num],
		sizeof(uint32_t) * b2_cadence.pair_entity_count);
	for (uint32_t eye = 0; eye < frame->view_count; eye++)
	{
		InstanceBuffer *snapshot = b2_cadence.role_instances + eye;
		memcpy(snapshot, &vkpt_refdef.uniform_instance_buffer, sizeof(*snapshot));
		// Gun-FOV correction changes model geometry. An inset must crop the
		// same weapon as its peripheral eye, not resize it for the inset FOV.
		// Only this weapon-construction context uses the physical eye; each
		// role retains its own projection, snapshot and previous transform.
		const uint32_t weapon_eye = eye % B2_EYE_COUNT;
		memcpy(vkpt_refdef.view_matrix, frame->views[weapon_eye].V, sizeof(saved_view));
		memcpy(vkpt_refdef.view_matrix_inv, frame->views[weapon_eye].invV, sizeof(saved_inverse));
		fd->fov_x = frame->views[weapon_eye].fov_xy[0];
		fd->fov_y = frame->views[weapon_eye].fov_xy[1];
		for (int index = 0; index < b2_cadence.pair_entity_count; index++)
		{
			ModelInstance *instance = snapshot->model_instances + index;
			if (!(instance->material & MATERIAL_FLAG_WEAPON))
				continue;
			uint32_t identity = b2_cadence.pair_entity_ids[index];
			const entity_t *entity = vr_find_weapon_entity(identity);
			const model_t *model = entity ? MOD_ForHandle(entity->model) : NULL;
			if (!model || model->num_light_polys != 0)
				Com_Error(ERR_FATAL, "B2_FAIL: unsupported emissive or unidentified viewweapon");
			entity_hash_t hash;
			memcpy(&hash, &identity, sizeof(hash));
            b2wm_part_t grenade_part;
            bool grenade=hash.mesh>=128 && hash.mesh<131 &&
                B2WP_ViewPart(entity->model,model,entity->frame,entity->oldframe,entity->backlerp,
                    hash.mesh-128,&grenade_part) && grenade_part.visible;
			if (hash.bsp || !hash.entity || (uint32_t)entity->model != hash.model ||
                (!grenade && hash.mesh>=model->nummeshes))
				Com_Error(ERR_FATAL, "B2_FAIL: viewweapon model/mesh identity differs from its instance");
            if (grenade && (model->nummeshes!=1 || instance->prim_count!=grenade_part.count ||
                instance->prim_offset_curr_pose_curr_frame!=model->meshes[0].tri_offset+
                    grenade_part.frame*model->meshes[0].numtris+grenade_part.first ||
                instance->prim_offset_prev_pose_curr_frame!=model->meshes[0].tri_offset+
                    grenade_part.oldframe*model->meshes[0].numtris+grenade_part.first ||
                instance->pose_lerp_curr_frame!=grenade_part.backlerp))
                Com_Error(ERR_FATAL,"B2_FAIL: grenade partition differs from its bound instance");
			for (int other = 0; other < b2_cadence.pair_entity_count; other++)
				if (other != index && b2_cadence.pair_entity_ids[other] == identity)
					Com_Error(ERR_FATAL, "B2_FAIL: duplicate viewweapon instance identity");
			float transform[16];
			create_viewweapon_matrix(transform, (entity_t *)entity);
            if (grenade) memcpy(transform,grenade_part.matrix,sizeof(transform));
			memcpy(instance->transform, transform, sizeof(instance->transform));
			const b2_weapon_prior_t *prior = NULL;
			if (frame->history_valid && weapon_continuity)
				for (int p = 0; p < b2_cadence.weapon_prior_count[eye]; p++)
					if (b2_cadence.weapon_prior[eye][p].identity == identity)
						prior = b2_cadence.weapon_prior[eye] + p;
			if (frame->history_valid && weapon_continuity)
			{
				uint32_t previous = snapshot->model_current_to_prev[index];
				if (prior)
				{
					if (previous >= (uint32_t)model_entity_id_count[!entity_frame_num] ||
						model_entity_ids[!entity_frame_num][previous] != identity ||
						snapshot->model_prev_to_current[previous] != (uint32_t)index)
						Com_Error(ERR_FATAL, "B2_FAIL: continuing viewweapon prior disagrees with instance mapping");
				}
				else
				{
					if (!live_weapons || previous != UINT32_MAX)
						Com_Error(ERR_FATAL, "B2_FAIL: continuing or frozen viewweapon lacks its same-eye prior");
					for (int p = 0; p < model_entity_id_count[!entity_frame_num]; p++)
						if (model_entity_ids[!entity_frame_num][p] == identity)
							Com_Error(ERR_FATAL, "B2_FAIL: unmatched viewweapon existed in the previous instance set");
					for (int p = 0; p < b2_cadence.weapon_prior_count[eye ^ 1u]; p++)
						if (b2_cadence.weapon_prior[eye ^ 1u][p].identity == identity)
							Com_Error(ERR_FATAL, "B2_FAIL: viewweapon prior exists for only one eye");
					// A changed model or a reappearing weapon is a new instance.
					// Keep the stock mapper's -1 correspondence; seed only this
					// eye's new geometry/transform, never another weapon or eye.
					instance->prim_offset_curr_pose_prev_frame = instance->prim_offset_curr_pose_curr_frame;
					instance->prim_offset_prev_pose_prev_frame = instance->prim_offset_prev_pose_curr_frame;
					instance->pose_lerp_prev_frame = instance->pose_lerp_curr_frame;
					instance->iqm_matrix_offset_prev_frame = instance->iqm_matrix_offset_curr_frame;
				}
			}
            if (!weapon_continuity) {
                uint32_t previous=snapshot->model_current_to_prev[index];
                if (previous<(uint32_t)model_entity_id_count[!entity_frame_num])
                    snapshot->model_prev_to_current[previous]=UINT32_MAX;
                snapshot->model_current_to_prev[index]=UINT32_MAX;
            }
            // Only completed submitted weapon animation is a temporal prior.
            instance->prim_offset_curr_pose_prev_frame=prior ? prior->current_pose : instance->prim_offset_curr_pose_curr_frame;
            instance->prim_offset_prev_pose_prev_frame=prior ? prior->previous_pose : instance->prim_offset_prev_pose_curr_frame;
            instance->pose_lerp_prev_frame=prior ? prior->lerp : instance->pose_lerp_curr_frame;
            instance->iqm_matrix_offset_prev_frame=prior ? (prior->iqm_offset<0 ? -1 :
                prior->iqm_offset+iqm_matrix_count[entity_frame_num]) : instance->iqm_matrix_offset_curr_frame;
			memcpy(instance->transform_prev, prior ? prior->transform : transform,
				sizeof(instance->transform_prev));
		}
	}
	fd->fov_x = saved_fov_x;
	fd->fov_y = saved_fov_y;
	memcpy(vkpt_refdef.view_matrix, saved_view, sizeof(saved_view));
	memcpy(vkpt_refdef.view_matrix_inv, saved_inverse, sizeof(saved_inverse));
	memcpy(model_instances_prev, b2_cadence.role_instances[0].model_instances,
		sizeof(ModelInstance) * b2_cadence.pair_entity_count);
	memcpy(&b2_cadence.iqm_snapshot, qvk.iqm_matrices_shadow, sizeof(b2_cadence.iqm_snapshot));
	b2_restage_instances(0);
	b2_cadence.pair_prepared = true;
	b2_cadence.prepares++;
}

static bool b2_xr_pair_failed555;
static void b2_finish_eye(uint32_t eye)
{
    if (b2_surface_pair.open) {
        if (eye>=b2_surface_pair.views || b2_surface_pair.rendered!=((1u<<(eye+1u))-1u) ||
            b2_surface_pair.finished!=((1u<<eye)-1u))
            Com_Error(ERR_FATAL,"B2_FAIL: UI eye completion order changed");
        if (!B2XR_FinishEye(eye)) {
            b2_xr_pair_failed555=true;vkpt_refdef.fd->b2_render_accepted=false;return;
        }
        b2_surface_pair.finished|=1u<<eye;return;
    }
	if (eye >= b2_cadence.view_count || !b2_cadence.pair_prepared || b2_cadence.rendered_mask != ((1u << (eye + 1u)) - 1u) ||
		b2_cadence.finished_mask != ((1u << eye) - 1u))
		Com_Error(ERR_FATAL, "B2_FAIL: eye completion did not close in pair order");
    if (!B2XR_FinishEye(eye)) {
        b2_xr_pair_failed555=true;vkpt_refdef.fd->b2_render_accepted=false;return;
    }

	b2_cadence.finished_mask |= 1u << eye;
}

static void b2_discard_settings_frame580(void);
static void b2_retire_xr_failure555(void)
{
    // The shared discard path discharges an unsubmitted physical fence/acquire
    // obligation, or destroys the retained desktop image after completed eyes.
    // Never present a partial pair. GPU completion uncertainty remains terminal.
    if (!b2_physical_started580) {
        VkResult result = vkDeviceWaitIdle(qvk.device);
        if (result != VK_SUCCESS) {
            vr_device_lost_terminal = result == VK_ERROR_DEVICE_LOST;
            vr_abandon_children = true;
            Com_EPrintf("B2_XR_RETIRE result=%s\n", qvk_result_to_string(result));
            Com_Error(ERR_FATAL,"VR graphics could not complete. Restart Q2RTX VR after checking the graphics driver.");
        }
    }
    b2_discard_settings_frame580();
    b2_xr_pair_failed555 = false;
}

// Rejecting an eye after BeginFrame still owes the physical fence and any
// acquire semaphore. Submit no eye work, then retire only desktop attachments.
static void b2_discard_settings_frame580(void)
{
    bool desktop_owned = !b2_mirror.no_image534 &&
        (b2_mirror.held || (b2_physical_started580 && b2_mirror.wait_acquire));
    if (b2_physical_started580) {
        VkSemaphore acquired = qvk.semaphores[qvk.current_frame_index][0].image_available;
        VkPipelineStageFlags stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkSubmitInfo submit = {.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount=b2_mirror.wait_acquire ? 1u : 0u,
            .pWaitSemaphores=&acquired, .pWaitDstStageMask=&stage};
        if (vkQueueSubmit(qvk.queue_graphics, 1, &submit,
            qvk.fences_frame_sync[qvk.current_frame_index]) != VK_SUCCESS)
            Com_Error(ERR_FATAL, "VR rejected-frame submission failed; restart required.");
    }
    if (b2_physical_started580 || b2_mirror.held) {
        if (vkDeviceWaitIdle(qvk.device) != VK_SUCCESS)
            Com_Error(ERR_FATAL, "VR rejected-frame retirement failed; restart required.");
    }
    if (desktop_owned) {
        // An acquired but unrendered image may have no present layout. Do not
        // present it or a partial mirror; recreate desktop attachments next time.
        vkpt_draw_destroy_desktop534();
        if (destroy_swapchain() != VK_SUCCESS)
            Com_Error(ERR_FATAL, "VR rejected desktop retirement failed; restart required.");
        b2_desktop_dirty534 = true;
    }
    if (b2_physical_started580) qvk.frame_counter++;
    b2_physical_started580 = false;
    b2_mirror.held = b2_mirror.wait_acquire = false;
    b2_cadence.pair_prepared = b2_surface_pair.open = false;
    temporal_frame_valid = frame_ready = false;
    b2_settings_history_reset580 = true;
    B2WP_Commit(false);
}

bool R_B2SettingsRejected580(void) { return b2_settings_rejected580; }

bool R_B2PairFinish(bool submitted_pair)
{
	if (SDL_AtomicGet(&b2_validation_failed))
	{
		B2XR_RequestStop();
		Com_EPrintf("B2 Vulkan validation error latched; stereo pair rejected, restart required\n");
		return false;
	}
    if (b2_settings_rejected580) {
        if (submitted_pair || (b2_mirror.held &&
            ((b2_surface_pair.open && b2_surface_pair.rendered != b2_surface_pair.finished) ||
             (b2_cadence.pair_prepared && b2_cadence.rendered_mask != b2_cadence.finished_mask))))
            Com_Error(ERR_FATAL, "VR settings rejection encountered unfinished eye ownership");
        // Admission failed before this eye acquired/reset/submitted anything.
        // Earlier completed eyes, if any, have consumed their acquire wait.
        b2_discard_settings_frame580();
        return false;
    }
    if (!submitted_pair && B2XR_StopRequested() &&
        (b2_xr_pair_failed555 ||
         (b2_surface_pair.open && b2_surface_pair.rendered==B2_ViewMask(b2_surface_pair.views) &&
          b2_surface_pair.finished==b2_surface_pair.rendered) ||
         (b2_cadence.pair_prepared && b2_cadence.rendered_mask==B2_ViewMask(b2_cadence.view_count) &&
          b2_cadence.finished_mask==b2_cadence.rendered_mask))) {
        b2_retire_xr_failure555();return false;
    }
    if (b2_surface_pair.open) {
        if (!submitted_pair || b2_mirror.held ||
            b2_surface_pair.rendered!=B2_ViewMask(b2_surface_pair.views) ||
            b2_surface_pair.finished!=b2_surface_pair.rendered) {
            B2XR_RequestStop();return false;
        }
        b2_surface_pair.open=false;return true;
    }
	if (!submitted_pair)
	{
		if (b2_mirror.held)
			Com_Error(ERR_FATAL, "B2_FAIL: acquired desktop pair incomplete; restart required");
		if (b2_cadence.pair_prepared)
		{
			B2XR_RequestStop();
			Com_EPrintf("B2 incomplete stereo pair discarded; restart required\n");
		}
		return false;
	}
	if (!b2_cadence.pair_prepared || b2_cadence.rendered_mask != B2_ViewMask(b2_cadence.view_count) || b2_cadence.finished_mask != B2_ViewMask(b2_cadence.view_count))
		Com_Error(ERR_FATAL, "B2_FAIL: submitted pair lacks both completed eye histories");
	if (b2_cadence.prepares != 1 || b2_cadence.light_uploads != 1 ||
		b2_cadence.light_clears != 1 || b2_cadence.bloom_updates != 1 ||
		b2_cadence.tone_updates != 1)
		Com_Error(ERR_FATAL, "B2_FAIL: shared logical-frame cadence differs");
	for (uint32_t view = 0; view < b2_cadence.view_count; ++view)
		if (b2_cadence.tone_applies[view] != 1) Com_Error(ERR_FATAL, "B2_FAIL: per-view tone application count differs");
	for (uint32_t role = 0; role < b2_cadence.view_count; role++)
	{
		b2_cadence.weapon_prior_count[role] = 0;
		for (int index = 0; index < b2_cadence.pair_entity_count; index++)
		{
			const ModelInstance *instance = b2_cadence.role_instances[role].model_instances + index;
			if (!(instance->material & MATERIAL_FLAG_WEAPON))
				continue;
			b2_weapon_prior_t *prior = b2_cadence.weapon_prior[role] + b2_cadence.weapon_prior_count[role]++;
			prior->identity = b2_cadence.pair_entity_ids[index];
			memcpy(prior->transform, instance->transform, sizeof(prior->transform));
            prior->current_pose=instance->prim_offset_curr_pose_curr_frame;
            prior->previous_pose=instance->prim_offset_prev_pose_curr_frame;
            prior->lerp=instance->pose_lerp_curr_frame;
            prior->iqm_offset=instance->iqm_matrix_offset_curr_frame;
		}
	}
    B2WP_Commit(true);
	b2_cadence.pair_prepared = false;
    if (b2_cadence.view_count == 2) if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_RENDER_PAIR logical=%u generation=%u parity=%u shared_prepare_light_clear_bloom_tone=1 eye_applies=1,1 completed=3 particles=%d,%d blend0=%.6f,%.6f,%.6f,%.6f blend1=%.6f,%.6f,%.6f,%.6f\n",
		b2_cadence.logical_index, b2_cadence.history_generation, b2_cadence.logical_index & 1u,
        b2_particle_counts[0], b2_particle_counts[1],
        b2_screen_blends[0][0], b2_screen_blends[0][1], b2_screen_blends[0][2], b2_screen_blends[0][3],
        b2_screen_blends[1][0], b2_screen_blends[1][1], b2_screen_blends[1][2], b2_screen_blends[1][3]);
	if (b2_cadence.view_count == 4) if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_RENDER_QUAD logical=%u generation=%u views=4 completed=%u once_shared_update=1 mirror=2x2_peripheral_top_inset_bottom\n", b2_cadence.logical_index, b2_cadence.history_generation, b2_cadence.finished_mask);
	return true;
}
#endif

#ifdef VKPT_IMAGE_DUMPS
static void 
copy_to_dump_texture(VkCommandBuffer cmd_buf, int src_image_index)
{
	VkImage src_image = qvk.images[src_image_index];
	VkImage dst_image = qvk.dump_image;

	VkImageCopy image_copy_region = {
		.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.srcSubresource.mipLevel = 0,
		.srcSubresource.baseArrayLayer = 0,
		.srcSubresource.layerCount = 1,

		.srcOffset.x = 0,
		.srcOffset.y = 0,
		.srcOffset.z = 0,

		.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.dstSubresource.mipLevel = 0,
		.dstSubresource.baseArrayLayer = 0,
		.dstSubresource.layerCount = 1,

		.dstOffset.x = 0,
		.dstOffset.y = 0,
		.dstOffset.z = 0,

		.extent.width = IMG_WIDTH,
		.extent.height = IMG_HEIGHT,
		.extent.depth = 1
	};

	VkImageSubresourceRange subresource_range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	IMAGE_BARRIER(cmd_buf,
		.image = src_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	);

	IMAGE_BARRIER(cmd_buf,
		.image = dst_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_HOST_READ_BIT,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	vkCmdCopyImage(cmd_buf,
		src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &image_copy_region);

	IMAGE_BARRIER(cmd_buf,
		.image = src_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_GENERAL
	);

	IMAGE_BARRIER(cmd_buf,
		.image = dst_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_HOST_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_GENERAL
	);
}
#endif

VkDescriptorSet qvk_get_current_desc_set_textures()
{
#ifdef REFERENCE_B_B2
	if (vkpt_refdef.fd && vkpt_refdef.fd->b2_active)
	{
		const refdef_t *fd = vkpt_refdef.fd;
        if ((fd->rdflags & RDF_NOWORLDMODEL) && (B2_UISurfaceActive() || B2_RealignActive391()))
            return (qvk.frame_counter & 1u)?qvk.desc_set_textures_odd:qvk.desc_set_textures_even;
		if (!qvk.b2_temporal_activated || !fd->b2_frame || fd->b2_eye >= fd->b2_frame->view_count)
			Com_Error(ERR_FATAL, "B2_FAIL: invalid live temporal descriptor selection");
		uint32_t parity = fd->b2_frame->logical_index & 1u;
		return vkpt_b2_view_descriptor(fd->b2_eye, parity);
	}
#endif
	return (qvk.frame_counter & 1) ? qvk.desc_set_textures_odd : qvk.desc_set_textures_even;
}

uint32_t vkpt_vr_history_parity(void)
{
#ifdef REFERENCE_B_B2
	if (vkpt_refdef.fd && vkpt_refdef.fd->b2_active)
		return vkpt_refdef.fd->b2_frame->logical_index & 1u;
#endif
	
	return qvk.frame_counter & 1u;
}
VkImage vkpt_vr_history_image(int image_index)
{
	if (image_index < 0 || image_index >= NUM_VKPT_IMAGES)
		Com_Error(ERR_FATAL, "VR_FAIL: temporal image resolver index outside ABI");
#ifdef REFERENCE_B_B2
	if (vkpt_refdef.fd && vkpt_refdef.fd->b2_active && vkpt_refdef.fd->b2_eye > 0 &&
		(image_index == VKPT_IMG_ASVGF_HIST_COLOR_HF ||
		 image_index == VKPT_IMG_PT_COLOR_HF || image_index == VKPT_IMG_PT_COLOR_SPEC ||
		 (image_index >= VKPT_IMG_PT_VISBUF_PRIM_A && image_index <= VKPT_IMG_ASVGF_GRAD_SMPL_POS_B)))
	{
		if (!qvk.b2_temporal_activated || !vkpt_b2_view_image(vkpt_refdef.fd->b2_eye, image_index))
			Com_Error(ERR_FATAL, "B2_FAIL: right temporal image is unavailable");
		return vkpt_b2_view_image(vkpt_refdef.fd->b2_eye, image_index);
	}
#endif
	
	return qvk.images[image_index];
}
#ifdef REFERENCE_B_B2
/* A retired client generation owns exactly one synchronous registration.
 * Temporal allocations and the logical stereo index stay alive throughout. */
static struct {
    bool open, failed, first_frame_pending, registration_finished;
    unsigned epoch, completed_epoch;
    uint32_t prior_history_generation;
} b2_world_resources;

static bool b2_player_resources492;

static void b2_world_fail(const char *operation)
{
    b2_world_resources.failed = true;
    vr_abandon_children = true;
    B2XR_RequestStop();
    Com_EPrintf("B2_FAIL: world resource boundary rejected (%s)\n", operation);
    Com_Error(ERR_FATAL, "VR graphics could not safely continue. Restart Q2RTX VR and load a saved game. See the game log for details.");
}

static bool b2_world_runtime_valid(void)
{
    return B2XR_Requested() && !B2XR_StopRequested() &&
        !vr_device_lost_terminal && !b2_world_resources.failed &&
        !SDL_AtomicGet(&b2_validation_failed);
}

bool R_B2DropRetire555(unsigned *next_epoch)
{
    // Ordinary map/drop recovery is allowed only outside graphics ownership.
    // No command-buffer, descriptor or memory corruption is silently repaired.
    if (!b2_world_runtime_valid() || b2_mirror.held || b2_cadence.pair_prepared ||
        b2_surface_pair.open || b2_player_resources492) return false;
    if (qvk.device && vkDeviceWaitIdle(qvk.device)!=VK_SUCCESS) return false;
    if (!next_epoch || (qvk.b2_temporal_activated && !*next_epoch)) return false;
    // No world has reached temporal activation yet. A failed first load must
    // re-enter the ordinary initial scene/epoch-one gate, not replacement-world
    // registration with an invented completed generation.
    if (!qvk.b2_temporal_activated) *next_epoch=0;
    // A failed registration publishes no world. The next registration must
    // replace it at the already-retired client epoch, not skip a generation.
    b2_world_resources.open=false;b2_world_resources.registration_finished=false;
    b2_world_resources.first_frame_pending=false;
    b2_world_resources.completed_epoch=*next_epoch ? *next_epoch-1u : 0;
    b2_world_resources.epoch=0;
    memset(b2_cadence.weapon_prior_count,0,sizeof(b2_cadence.weapon_prior_count));
    temporal_frame_valid=false;
    return true;
}

bool R_B2WorldMaintenance(void)
{
    return b2_world_resources.open && b2_world_runtime_valid() &&
        B2_WorldMaintenanceEpoch() == b2_world_resources.epoch;
}

void R_B2ResourceResult(int result, const char *operation)
{
    if ((b2_world_resources.open || b2_player_resources492) && result != VK_SUCCESS) {
        if (result == VK_ERROR_DEVICE_LOST) vr_device_lost_terminal = true;
        b2_world_fail(operation);
    }
}

// The player menu registers only its selected icon/banner in VR. Publish at
// this explicit callback boundary, never while an eye/world transaction is open.
static bool b2_player_boundary492(void)
{
    return b2_world_runtime_valid() && !b2_world_resources.open &&
        !b2_world_resources.first_frame_pending && !b2_cadence.pair_prepared &&
        !b2_mirror.held && !b2_surface_pair.open &&
        B2_ValidViewCount(b2_cadence.view_count) &&
        b2_cadence.rendered_mask==B2_ViewMask(b2_cadence.view_count) &&
        b2_cadence.finished_mask==B2_ViewMask(b2_cadence.view_count) &&
        b2_surface_pair.rendered==b2_surface_pair.finished;
}

void R_B2PlayerMenuResources492(void)
{
    if (!qvk.b2_temporal_activated) return; // ordinary preactivation upload
    if (b2_player_resources492 || !b2_player_boundary492())
        b2_world_fail("player menu registration ownership");
    b2_player_resources492=true;
    R_B2ResourceResult(vkDeviceWaitIdle(qvk.device),"player menu prior GPU completion");
    vkpt_b2_world_textures_finish(); // all eye/parity sets, existing texture limits
    R_B2ResourceResult(vkDeviceWaitIdle(qvk.device),"player menu upload completion");
    if (!b2_player_boundary492()) b2_world_fail("player menu publication");
    b2_player_resources492=false;
}

static void b2_world_registration_commit(void);
bool R_B2WorldReady(unsigned epoch)
{
    // Client requests readiness only after its late configstrings and UI/HUD
    // media preparation, while the retired world still owns maintenance.
    if (epoch && qvk.b2_temporal_activated && b2_world_runtime_valid() &&
        b2_world_resources.open && b2_world_resources.registration_finished &&
        epoch == b2_world_resources.epoch)
        b2_world_registration_commit();
    return epoch && qvk.b2_temporal_activated && b2_world_runtime_valid() &&
        !b2_world_resources.open && epoch == b2_world_resources.completed_epoch;
}

static void b2_world_registration_begin(void)
{
    if (!qvk.b2_temporal_activated) return;
    unsigned epoch = B2_WorldMaintenanceEpoch();
    if (!b2_world_runtime_valid() || !epoch || b2_world_resources.open ||
        b2_world_resources.first_frame_pending ||
        b2_world_resources.completed_epoch == UINT_MAX ||
        epoch != b2_world_resources.completed_epoch + 1u ||
        b2_cadence.pair_prepared || b2_mirror.held ||
        b2_cadence.rendered_mask != B2_ViewMask(b2_cadence.view_count) || b2_cadence.finished_mask != B2_ViewMask(b2_cadence.view_count))
        b2_world_fail("begin registration ownership");
    /* Permission is not open until every prior queue has completed. */
    VkResult idle = vkDeviceWaitIdle(qvk.device);
    if (idle != VK_SUCCESS) {
        if (idle == VK_ERROR_DEVICE_LOST) vr_device_lost_terminal = true;
        b2_world_fail("begin registration GPU completion");
    }
    b2_world_resources.epoch = epoch;
    b2_world_resources.open = true;
    b2_world_resources.registration_finished = false;
    b2_world_resources.prior_history_generation = b2_cadence.history_generation;
    memset(b2_cadence.weapon_prior_count, 0, sizeof(b2_cadence.weapon_prior_count));
    b2_cadence.pair_entity_count = 0;
    memset(&b2_cadence.feedback, 0, sizeof(b2_cadence.feedback));
    b2_cadence.sun_visible = false;
    temporal_frame_valid = false;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_RESOURCE begin epoch=%u prior_gpu_complete=1 temporal_allocations_retained=1\n", epoch);
}

static void b2_world_registration_end(void)
{
    if (!qvk.b2_temporal_activated) return;
    if (!R_B2WorldMaintenance() || b2_world_resources.registration_finished)
        b2_world_fail("end registration ownership");
    // R_EndRegistration precedes subsequent server configstrings and client
    // UI media. Keep the same owned, GPU-idle boundary until client readiness.
    b2_world_resources.registration_finished = true;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_RESOURCE registration_staged epoch=%u awaiting_client_media=1\n",b2_world_resources.epoch);
}

static void b2_world_registration_commit(void)
{
    if (!qvk.b2_temporal_activated) return;
    if (!R_B2WorldMaintenance() || !b2_world_resources.registration_finished ||
        b2_cadence.pair_prepared || b2_mirror.held ||
        b2_cadence.rendered_mask != B2_ViewMask(b2_cadence.view_count) || b2_cadence.finished_mask != B2_ViewMask(b2_cadence.view_count))
        b2_world_fail("registration commit ownership");
    /* Finish deferred work here, before client bind or a new eye can run.
     * Both canonical and both right-eye sets get the replacement map. */
    vkpt_b2_world_textures_finish();
    R_B2ResourceResult(vkpt_vertex_buffer_upload_models(), "model upload");
    R_B2ResourceResult(vkDeviceWaitIdle(qvk.device), "registration producers completion");
    if (!R_B2WorldMaintenance()) b2_world_fail("registration publication");
    b2_world_resources.completed_epoch = b2_world_resources.epoch;
    b2_world_resources.first_frame_pending = true;
    b2_world_resources.open = false;
    b2_world_resources.registration_finished = false;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_RESOURCE ready epoch=%u uploads_complete=1 all_eye_parity_descriptors=1\n",
        b2_world_resources.completed_epoch);
}

static void b2_world_first_frame(const refdef_t *fd)
{
    // The initial client world is already bound before temporal activation.
    // Latch it from that validated first frame, so the first retirement advances
    // from epoch one to two just like every subsequent registration.
    if (!qvk.b2_temporal_activated && B2_LifecycleEnabled()) {
        const b2_frame_t *initial=fd ? fd->b2_frame : NULL;
        if (!fd || !initial || fd->b2_eye || initial->logical_index || initial->history_valid ||
            !B2_WorldBound(1) || b2_world_resources.completed_epoch || b2_world_resources.open ||
            b2_world_resources.failed || b2_world_resources.first_frame_pending)
            b2_world_fail("initial bound world ownership");
        b2_world_resources.completed_epoch=1;
    }
    if (!b2_world_resources.first_frame_pending) return;
    const b2_frame_t *frame = fd->b2_frame;
    if (!R_B2WorldReady(b2_world_resources.completed_epoch) ||
        !B2_WorldBound(b2_world_resources.completed_epoch) || !frame || fd->b2_eye ||
        frame->history_valid || frame->history_generation <= b2_world_resources.prior_history_generation ||
        b2_cadence.logical_index == UINT32_MAX ||
        frame->logical_index != b2_cadence.logical_index + 1u)
        b2_world_fail("first replacement-world frame");
    b2_world_resources.first_frame_pending = false;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_RESOURCE first_frame epoch=%u logical=%u generation=%u history_valid=0\n",
        b2_world_resources.completed_epoch, frame->logical_index, frame->history_generation);
}
#endif

void vkpt_vr_require_maintenance_open(const char *operation)
{
#ifdef REFERENCE_B_B2
	if (qvk.b2_temporal_activated) {
        bool world_operation = !strcmp(operation, "begin registration") ||
            !strcmp(operation, "end registration") || !strcmp(operation, "sky descriptor regeneration") ||
            !strcmp(operation, "texture descriptor invalidation") || !strcmp(operation, "environment descriptor regeneration") ||
            !strcmp(operation, "texture registration finalization") || !strcmp(operation, "texture descriptor write");
        bool ui_texture=!strcmp(operation,"texture descriptor invalidation") ||
            !strcmp(operation,"texture registration finalization") || !strcmp(operation,"texture descriptor write");
        if (ui_texture && b2_player_resources492 && b2_player_boundary492()) return;
        if (ui_texture && (B2_UISurfaceActive() || B2_RealignActive391()) && b2_world_runtime_valid() &&
            !b2_cadence.pair_prepared && b2_surface_pair.rendered==b2_surface_pair.finished) {
            // Ordinary raw-picture uploads occur between completed eyes. Keep
            // world resource ownership closed; only canonical UI descriptors change.
            if (vkDeviceWaitIdle(qvk.device)!=VK_SUCCESS) b2_world_fail("UI texture GPU completion");
            return;
        }
        if (!world_operation || !R_B2WorldMaintenance())
            b2_world_fail(operation);
    }
#endif
	
}

static bool
process_render_feedback(ref_feedback_t *feedback, mleaf_t* viewleaf, bool* sun_visible, float* adapted_luminance)
{
	ReadbackBuffer readback;
	if (vkpt_readback(&readback) != VK_SUCCESS)
		return false;

	if (viewleaf)
		feedback->viewcluster = viewleaf->cluster;
	else
		feedback->viewcluster = -1;

	{
		static char const * unknown = "<unknown>";
		char const * view_material = unknown;
		char const * view_material_override = unknown;
		if (readback.material != ~0u)
		{
			int material_id = readback.material & MATERIAL_INDEX_MASK;
			feedback->view_material_index = material_id;
			pbr_material_t const* material = MAT_ForIndex(material_id);
			if (material)
			{
				image_t const* image = material->image_base;
				if (image)
				{
					view_material = image->name;
					view_material_override = image->filepath;
				}
			}
		}
		else
			feedback->view_material_index = -1;
		strcpy(feedback->view_material, view_material);
		strcpy(feedback->view_material_override, view_material_override);

		feedback->lookatcluster = readback.cluster;
		feedback->num_light_polys = 0;

		if (vkpt_refdef.bsp_mesh_world_loaded && feedback->lookatcluster >= 0 && feedback->lookatcluster < vkpt_refdef.bsp_mesh_world.num_clusters)
		{
			int* light_offsets = vkpt_refdef.bsp_mesh_world.cluster_light_offsets + feedback->lookatcluster;
			feedback->num_light_polys = light_offsets[1] - light_offsets[0];
		}

		VectorCopy(readback.hdr_color, feedback->hdr_color);
		feedback->adapted_luminance = readback.adapted_luminance;

		*sun_visible = readback.sun_luminance > 0.f;
		*adapted_luminance = readback.adapted_luminance;
	}
	return true;
}

typedef struct reference_mode_s 
{
	bool enable_accumulation;
	bool enable_denoiser;
	float num_bounce_rays;
	float temporal_blend_factor;
	int reflect_refract;
} reference_mode_t;

static int
get_accumulation_rendering_framenum(void)
{
	return max(128, cvar_pt_accumulation_rendering_framenum->integer);
}

static bool is_accumulation_rendering_active(void)
{
	return cl_paused->integer == 2 && sv_paused->integer && cvar_pt_accumulation_rendering->integer > 0;
}

static void draw_shadowed_string(int x, int y, int flags, size_t maxlen, const char* s)
{
	R_SetColor(0xff000000u);
	SCR_DrawStringEx(x + 1, y + 1, flags, maxlen, s, SCR_GetFont());
	R_SetColor(~0u);
	SCR_DrawStringEx(x, y, flags, maxlen, s, SCR_GetFont());
}

static void
evaluate_reference_mode(reference_mode_t* ref_mode)
{
	if (is_accumulation_rendering_active())
	{
		num_accumulated_frames++;

		const int num_warmup_frames = 5;
		const int num_frames_to_accumulate = get_accumulation_rendering_framenum();

		ref_mode->enable_accumulation = true;
		ref_mode->enable_denoiser = false;
		ref_mode->num_bounce_rays = 2;
		ref_mode->temporal_blend_factor = 1.f / min(max(1, num_accumulated_frames - num_warmup_frames), num_frames_to_accumulate);
		ref_mode->reflect_refract = max(4, cvar_pt_reflect_refract->integer);

		switch (cvar_pt_accumulation_rendering->integer)
		{
		case 1: {
			char text[MAX_QPATH];
			float percentage = powf(max(0.f, (num_accumulated_frames - num_warmup_frames) / (float)num_frames_to_accumulate), 0.5f);
			Q_snprintf(text, sizeof(text), "Photo mode: accumulating samples... %d%%", (int)(min(1.f, percentage) * 100.f));

			int frames_after_accumulation_finished = num_accumulated_frames - num_warmup_frames - num_frames_to_accumulate;
			float hud_alpha = max(0.f, min(1.f, (50 - frames_after_accumulation_finished) * 0.02f)); // fade out for 50 frames after accumulation finishes

			int x = r_config.width / 4;
			int y = 30;
			R_SetScale(0.5f);
			R_SetAlphaScale(hud_alpha);
			draw_shadowed_string(x, y, UI_CENTER, MAX_QPATH, text);

			if (cvar_pt_dof->integer)
			{
				x = 5;
				y = r_config.height / 2 - 55;
				Q_snprintf(text, sizeof(text), "Focal Distance: %.1f", cvar_pt_focus->value);
				draw_shadowed_string(x, y, UI_LEFT, MAX_QPATH, text);

				y += 10;
				Q_snprintf(text, sizeof(text), "Aperture: %.2f", cvar_pt_aperture->value);
				draw_shadowed_string(x, y, UI_LEFT, MAX_QPATH, text);

				y += 10;
				draw_shadowed_string(x, y, UI_LEFT, MAX_QPATH, "Use Mouse Wheel, Shift, Ctrl to adjust");
			}

			R_SetAlphaScale(1.f);

			SCR_SetHudAlpha(hud_alpha);
			break;
		}
		case 2:
			SCR_SetHudAlpha(0.f);
			break;
		}
	}
	else
	{
		num_accumulated_frames = 0;

		ref_mode->enable_accumulation = false;
		ref_mode->enable_denoiser = !!cvar_flt_enable->integer;
		if (cvar_pt_num_bounce_rays->value == 0.5f)
			ref_mode->num_bounce_rays = 0.5f;
		else
			ref_mode->num_bounce_rays = max(0, min(2, round(cvar_pt_num_bounce_rays->value)));
		ref_mode->temporal_blend_factor = 0.f;
		ref_mode->reflect_refract = max(0, cvar_pt_reflect_refract->integer);
	}

	ref_mode->reflect_refract = min(10, ref_mode->reflect_refract);
}

static void
evaluate_taa_settings(const reference_mode_t* ref_mode)
{
	qvk.effective_aa_mode = AA_MODE_OFF;
	qvk.extent_taa_output = qvk.extent_render;

	if (!ref_mode->enable_denoiser)
		return;

	int flt_taa = cvar_flt_taa->integer;
	// FSR RCAS needs upscaled input; if EASU was disabled, force to TAAU
	bool force_upscaling = vkpt_fsr_is_enabled() && vkpt_fsr_needs_upscale();
	if(force_upscaling)
	{
		flt_taa = AA_MODE_UPSCALE;
	}

	if (flt_taa == AA_MODE_TAA)
	{
		qvk.effective_aa_mode = AA_MODE_TAA;
	}
	else if (flt_taa == AA_MODE_UPSCALE) // TAAU or TAA+FSR
	{
		if (qvk.extent_render.width > qvk.extent_unscaled.width || qvk.extent_render.height > qvk.extent_unscaled.height)
		{
			qvk.effective_aa_mode = AA_MODE_TAA;
		}
		else
		{
			qvk.effective_aa_mode = AA_MODE_UPSCALE;
			if (!vkpt_fsr_is_enabled() || force_upscaling)
				qvk.extent_taa_output = qvk.extent_unscaled;
		}
	}
}

static void
prepare_sky_matrix(float time, vec3_t sky_matrix[3])
{
	if (sky_rotation != 0.f)
	{
		SetupRotationMatrix(sky_matrix, sky_axis, (sky_autorotate ? time : 1.f) * sky_rotation);
	}
	else
	{
		VectorSet(sky_matrix[0], 1.f, 0.f, 0.f);
		VectorSet(sky_matrix[1], 0.f, 1.f, 0.f);
		VectorSet(sky_matrix[2], 0.f, 0.f, 1.f);
	}
}

static void
prepare_camera(const vec3_t position, const vec3_t direction, mat4_t data)
{
	vec3_t forward, right, up;
	VectorCopy(direction, forward);
	VectorNormalize(forward);

	if (fabsf(forward[2]) < 0.99f)
		VectorSet(up, 0.f, 0.f, 1.f);
	else
		VectorSet(up, 0.f, 1.f, 0.f);

	CrossProduct(forward, up, right);
	CrossProduct(right, forward, up);
	VectorNormalize(up);
	VectorNormalize(right);

	float aspect = 1.75f;
	float tan_half_fov_x = 1.f;
	float tan_half_fov_y = tan_half_fov_x / aspect;

	VectorCopy(position, data + 0);
	VectorCopy(forward, data + 4);
	VectorMA(data + 4, -tan_half_fov_x, right, data + 4);
	VectorMA(data + 4, tan_half_fov_y, up, data + 4);
	VectorScale(right, 2.f * tan_half_fov_x, data + 8);
	VectorScale(up, -2.f * tan_half_fov_y, data + 12);
}

static void
prepare_viewmatrix(refdef_t *fd)
{
#ifdef REFERENCE_B_B2
	if (fd->b2_active)
	{
		const b2_view_t *view = fd->b2_frame->views + fd->b2_eye;
		memcpy(vkpt_refdef.view_matrix, view->V, sizeof(view->V));
		memcpy(vkpt_refdef.view_matrix_inv, view->invV, sizeof(view->invV));
		return;
	}
#endif
	create_view_matrix(vkpt_refdef.view_matrix, fd);
	inverse(vkpt_refdef.view_matrix, vkpt_refdef.view_matrix_inv);
}

static void
prepare_ubo(refdef_t *fd, mleaf_t* viewleaf, const reference_mode_t* ref_mode, const vec3_t sky_matrix[3], bool render_world)
{
	const bsp_mesh_t* wm = &vkpt_refdef.bsp_mesh_world;

	float P[16];

	QVKUniformBuffer_t *ubo = &vkpt_refdef.uniform_buffer;
	memcpy(ubo->V_prev, ubo->V, sizeof(float) * 16);
	memcpy(ubo->P_prev, ubo->P, sizeof(float) * 16);
	memcpy(ubo->invP_prev, ubo->invP, sizeof(float) * 16);
	ubo->cylindrical_hfov_prev = ubo->cylindrical_hfov;
	ubo->prev_taa_output_width = ubo->taa_output_width;
	ubo->prev_taa_output_height = ubo->taa_output_height;

	{
#ifdef REFERENCE_B_B2
		if (fd->b2_active)
			memcpy(P, fd->b2_frame->views[fd->b2_eye].P, sizeof(P));
		else
#endif
		{
		float raw_proj[16];
		create_projection_matrix(raw_proj, vkpt_refdef.z_near, vkpt_refdef.z_far, fd->fov_x, fd->fov_y);

		// In some cases (ex.: player setup), 'fd' will describe a viewport that is not full screen.
		// Simulate that with a projection matrix adjustment to avoid modifying the rendering code.

		float viewport_proj[16] = {
			[0] = (float)fd->width / (float)qvk.extent_unscaled.width,
			[12] = (float)(fd->x * 2 + fd->width - (int)qvk.extent_unscaled.width) / (float)qvk.extent_unscaled.width,
			[5] = (float)fd->height / (float)qvk.extent_unscaled.height,
			[13] = -(float)(fd->y * 2 + fd->height - (int)qvk.extent_unscaled.height) / (float)qvk.extent_unscaled.height,
			[10] = 1.f,
			[15] = 1.f
		};

		mult_matrix_matrix(P, viewport_proj, raw_proj);
		}
	}
	memcpy(ubo->V, vkpt_refdef.view_matrix, sizeof(float) * 16);
	memcpy(ubo->P, P, sizeof(float) * 16);
	memcpy(ubo->invV, vkpt_refdef.view_matrix_inv, sizeof(float) * 16);
		inverse(P, *ubo->invP);

	float vfov = fd->fov_y * (float)M_PI / 180.f;
	float unscaled_aspect = (float)qvk.extent_unscaled.width / (float)qvk.extent_unscaled.height;
	float rad_per_pixel;
	float fov_scale[2] = { 0.f, 0.f };

	switch (cvar_pt_projection->integer)
	{
	case PROJECTION_PANINI:
		fov_scale[1] = tanf(vfov / 2.f);
		fov_scale[0] = fov_scale[1] * unscaled_aspect;
		break;
	case PROJECTION_STEREOGRAPHIC:
		fov_scale[1] = tanf(vfov / 2.f * STEREOGRAPHIC_ANGLE);
		fov_scale[0] = fov_scale[1] * unscaled_aspect;
		break;
	case PROJECTION_CYLINDRICAL:
		rad_per_pixel = atanf(tanf(fd->fov_y * (float)M_PI / 360.f) / ((float)qvk.extent_unscaled.height * 0.5f));
		ubo->cylindrical_hfov = rad_per_pixel * (float)qvk.extent_unscaled.width;
		break;
	case PROJECTION_EQUIRECTANGULAR:
		fov_scale[1] = vfov / 2.f;
		fov_scale[0] = fov_scale[1] * unscaled_aspect;
		break;
	case PROJECTION_MERCATOR:
		fov_scale[1] = logf(tanf((float)M_PI * 0.25f + (vfov / 2.f) * 0.5f));
		fov_scale[0] = fov_scale[1] * unscaled_aspect;
		break;
	}

	ubo->projection_fov_scale_prev[0] = ubo->projection_fov_scale[0];
	ubo->projection_fov_scale_prev[1] = ubo->projection_fov_scale[1];
	ubo->projection_fov_scale[0] = fov_scale[0];
	ubo->projection_fov_scale[1] = fov_scale[1];
	ubo->pt_projection = render_world ? cvar_pt_projection->integer : 0; // always use rectilinear projection when rendering the player setup view
#ifdef REFERENCE_VR_HISTORY
	ubo->temporal_history_valid = temporal_frame_valid ? 1 : 0;
#endif
	ubo->current_frame_idx = qvk.frame_counter;
	ubo->width = qvk.extent_render.width;
	ubo->height = qvk.extent_render.height;
	ubo->prev_width = qvk.extent_render_prev.width;
	ubo->prev_height = qvk.extent_render_prev.height;
	ubo->inv_width = 1.0f / (float)qvk.extent_render.width;
	ubo->inv_height = 1.0f / (float)qvk.extent_render.height;
	ubo->unscaled_width = qvk.extent_unscaled.width;
	ubo->unscaled_height = qvk.extent_unscaled.height;
	ubo->taa_image_width = qvk.extent_taa_images.width;
	ubo->taa_image_height = qvk.extent_taa_images.height;
	ubo->taa_output_width = qvk.extent_taa_output.width;
	ubo->taa_output_height = qvk.extent_taa_output.height;
	ubo->current_gpu_slice_width = qvk.gpu_slice_width;
	ubo->prev_gpu_slice_width = qvk.gpu_slice_width_prev;
	ubo->screen_image_width = qvk.extent_screen_images.width;
	ubo->screen_image_height = qvk.extent_screen_images.height;
	ubo->water_normal_texture = water_normal_texture - r_images;
	ubo->pt_swap_checkerboard = 0;
	ubo->ui_color_scale =
		qvk.surf_is_hdr ? (cvar_ui_hdr_nits->value * 0.0125) // an scRGB luminance of 1.0 is defined as 80 nits
						: 1.f /* no change */;

	qvk.extent_render_prev = qvk.extent_render;
	qvk.gpu_slice_width_prev = qvk.gpu_slice_width;

	int camera_cluster_contents = viewleaf ? viewleaf->contents : 0;

	if (camera_cluster_contents & CONTENTS_WATER)
		ubo->medium = MEDIUM_WATER;
	else if (camera_cluster_contents & CONTENTS_SLIME)
		ubo->medium = MEDIUM_SLIME;
	else if (camera_cluster_contents & CONTENTS_LAVA)
		ubo->medium = MEDIUM_LAVA;
	else
		ubo->medium = MEDIUM_NONE;

	ubo->time = fd->time;
	ubo->num_static_primitives = 0;
	if (wm->geom_opaque.prim_counts)      ubo->num_static_primitives += wm->geom_opaque.prim_counts[0];
	if (wm->geom_transparent.prim_counts) ubo->num_static_primitives += wm->geom_transparent.prim_counts[0];
	if (wm->geom_masked.prim_counts)      ubo->num_static_primitives += wm->geom_masked.prim_counts[0];
	ubo->num_static_lights = vkpt_refdef.bsp_mesh_world.num_light_polys;

	vkpt_fog_upload(ubo->fog_volumes);

#define UBO_CVAR_DO(name, default_value) ubo->name = cvar_##name->value;
	UBO_CVAR_LIST
#undef UBO_CVAR_DO

	// Clear the private transport for every freshly populated per-view UBO.
	ubo->padding[0] = 0.0f;
#ifdef REFERENCE_B_B2
	if (fd && fd->b2_active && fd->b2_frame && fd->b2_frame->view_count == 4 &&
		fd->b2_frame->tracking_valid && fd->b2_frame->should_render && (unsigned)fd->b2_eye < 4)
	{
		const float mode = Cvar_Get("b2_quad_style293","1",CVAR_ARCHIVE)->integer==0?0.f:11.f;
		// Exact comparisons reject fractional, non-finite and out-of-range values.
		if (mode == 1.0f || mode == 2.0f || mode == 3.0f || mode == 4.0f ||
			mode == 5.0f || mode == 6.0f || mode == 7.0f || mode == 8.0f || mode == 9.0f || mode == 10.0f || mode == 11.0f || mode == 12.0f)
		{
			// J reuses baseline selection in peripheral views and E in focus views.
			ubo->padding[0] = mode == 10.0f ? (fd->b2_eye >= 2 ? 5.0f : 0.0f) : mode;
			if (mode == 2.0f || mode == 3.0f)
			{
				const bool selected_role = mode == 2.0f ? fd->b2_eye < 2 : fd->b2_eye >= 2;
				if (!selected_role)
					ubo->flt_min_alpha_color_hf = 0.02f;
				ubo->flt_min_alpha_moments_hf = 0.01f;
			}
			// Sentinel 2 retains shared PDF reads and suppresses only focus writes.
			if (mode == 4.0f && fd->b2_eye >= 2 && ubo->pt_light_stats == 1.0f)
				ubo->pt_light_stats = 2.0f;
			// UBO-only override; the public value is restored on the next other-mode view.
			if (mode == 8.0f)
				ubo->pt_bump_scale = 0.0f;
		}
	}
#endif

	bool fsr_enabled = vkpt_fsr_is_enabled();

	if (!ref_mode->enable_denoiser)
	{
		// disable fake specular because it is not supported without denoiser, and the result
		// looks too dark with it missing
		ubo->pt_fake_roughness_threshold = 1.f;

		// swap the checkerboard fields every frame in reference or noisy mode to accumulate 
		// both reflection and refraction in every pixel
		ubo->pt_swap_checkerboard = (qvk.frame_counter & 1);

		if (ref_mode->enable_accumulation)
		{
			ubo->pt_texture_lod_bias = -log2f(sqrtf(get_accumulation_rendering_framenum()));

			// disable the other stabilization hacks
			ubo->pt_specular_anti_flicker = 0.f;
			ubo->pt_sun_bounce_range = 10000.f;
			ubo->pt_ndf_trim = 1.f;
		}
	}
	else if(fsr_enabled || (qvk.effective_aa_mode == AA_MODE_UPSCALE))
	{
		// adjust texture LOD bias to the resolution scale, i.e. use negative bias if scale is < 100
		float resolution_scale = (drs_effective_scale != 0) ? (float)drs_effective_scale : (float)scr_viewsize->integer;
		#ifdef REFERENCE_B_B2
        if (B2XR_Requested()) resolution_scale=100.f;
#endif
        resolution_scale *= 0.01f;
		resolution_scale = Q_clipf(resolution_scale, 0.1f, 1.f);
		ubo->pt_texture_lod_bias = cvar_pt_texture_lod_bias->value + log2f(resolution_scale);
	}

	{
		// figure out if DoF should be enabled in the current rendering mode

		bool enable_dof = true;

		switch (cvar_pt_dof->integer)
		{
		case 0: enable_dof = false; break;
		case 1: enable_dof = ref_mode->enable_accumulation; break;
		case 2: enable_dof = !ref_mode->enable_denoiser; break;
		default: enable_dof = true; break;
		}

		if (cvar_pt_projection->integer != 0)
		{
			// DoF does not make physical sense with the cylindrical projection
			enable_dof = false;
		}

		if (!enable_dof)
		{
			// if DoF should not be enabled, make the aperture size zero
			ubo->pt_aperture = 0.f;
		}
	}

	// number of polygon vertices must be an integer
	ubo->pt_aperture_type = roundf(ubo->pt_aperture_type);

	ubo->temporal_blend_factor = ref_mode->temporal_blend_factor;
	ubo->flt_enable = ref_mode->enable_denoiser;
	ubo->flt_taa = qvk.effective_aa_mode;
	ubo->pt_num_bounce_rays = ref_mode->num_bounce_rays;
	ubo->pt_reflect_refract = ref_mode->reflect_refract;

	if (ref_mode->num_bounce_rays < 1.f)
		ubo->pt_specular_mis = 0; // disable MIS if there are no specular rays

	ubo->pt_min_log_sky_luminance = exp2f(ubo->pt_min_log_sky_luminance);
	ubo->pt_max_log_sky_luminance = exp2f(ubo->pt_max_log_sky_luminance);

	memcpy(ubo->cam_pos, fd->vieworg, sizeof(float) * 3);
	ubo->cluster_debug_index = cluster_debug_index;

#ifdef REFERENCE_B_B2
	if (fd->b2_active) b2ds_temporal_input = temporal_frame_valid;
#endif
	if (!temporal_frame_valid)
	{
		ubo->flt_temporal_lf = 0;
		ubo->flt_temporal_hf = 0;
		ubo->flt_temporal_spec = 0;
		ubo->flt_taa = 0;
	}

	if (qvk.effective_aa_mode == AA_MODE_UPSCALE)
	{
		int taa_index = (int)(qvk.frame_counter % NUM_TAA_SAMPLES);
#ifdef REFERENCE_B_B2
		if (fd->b2_active) taa_index = fd->b2_frame->logical_index % NUM_TAA_SAMPLES;
#endif
		ubo->sub_pixel_jitter[0] = taa_samples[taa_index][0];
		ubo->sub_pixel_jitter[1] = taa_samples[taa_index][1];
	}
	else
	{
		ubo->sub_pixel_jitter[0] = 0.f;
		ubo->sub_pixel_jitter[1] = 0.f;
	}

#ifdef REFERENCE_B_B2
	if (fd->b2_active)
	{
		const b2_frame_t *frame = fd->b2_frame;
		const b2_view_t *view = frame->views + fd->b2_eye;
		const b2_view_t *previous = frame->previous + fd->b2_eye;
		memcpy(ubo->invP, view->invP, sizeof(view->invP));
		memcpy(ubo->V_prev, previous->V, sizeof(previous->V));
		memcpy(ubo->P_prev, previous->P, sizeof(previous->P));
		memcpy(ubo->invP_prev, previous->invP, sizeof(previous->invP));
		ubo->current_frame_idx = frame->logical_index;
		ubo->temporal_history_valid = frame->history_valid ? 1 : 0;
		ubo->prev_width = B2G_Width();
		ubo->prev_height = B2G_Height();
		ubo->prev_gpu_slice_width = B2G_Width();
		ubo->prev_taa_output_width = B2G_Width();
		ubo->prev_taa_output_height = B2G_Height();
		ubo->cylindrical_hfov = ubo->cylindrical_hfov_prev = 0.f;
		memset(ubo->projection_fov_scale, 0, sizeof(ubo->projection_fov_scale));
		memset(ubo->projection_fov_scale_prev, 0, sizeof(ubo->projection_fov_scale_prev));
		if (!ref_mode->enable_denoiser)
			ubo->pt_swap_checkerboard = frame->logical_index & 1u;
	}
#endif

	// Set up constants for FSR
	if (fsr_enabled)
	{
		vkpt_fsr_update_ubo(ubo);
	}

	ubo->first_person_model = cl_player_model->integer == CL_PLAYER_MODEL_FIRST_PERSON;

	memset(ubo->environment_rotation_matrix, 0, sizeof(ubo->environment_rotation_matrix));
	VectorCopy(sky_matrix[0], ubo->environment_rotation_matrix[0]);
	VectorCopy(sky_matrix[1], ubo->environment_rotation_matrix[1]);
	VectorCopy(sky_matrix[2], ubo->environment_rotation_matrix[2]);
	
	add_dlights(vkpt_refdef.fd->dlights, vkpt_refdef.fd->num_dlights, ubo);

	if (wm->num_cameras > 0)
	{
		for (int n = 0; n < wm->num_cameras; n++)
		{
			prepare_camera(wm->cameras[n].pos, wm->cameras[n].dir, *ubo->security_camera_data[n]);
		}
	}
	else
	{
		ubo->pt_cameras = 0;
	}

	ubo->num_cameras = wm->num_cameras;
	/* Snapshot the actual per-view projection and jitter after the UBO is complete. */
#ifdef REFERENCE_B_B2
	if (fd && fd->b2_active && fd->b2_frame && fd->b2_frame->view_count == 4 &&
		fd->b2_frame->tracking_valid && fd->b2_frame->should_render && (unsigned)fd->b2_eye < 4)
		b2_hack_guide_snapshot(&ubo->P[0][0], &ubo->V[0][0], ubo->cam_pos,
			ubo->sub_pixel_jitter, ubo->width, ubo->height, ubo->padding[0] == 9.0f,
			ubo->pt_projection == 0 && ubo->pt_aperture == 0.0f &&
			ubo->flt_taa != 1.0f && ubo->temporal_blend_factor == 0.0f,
			fd->b2_eye, fd->b2_frame->logical_index, fd->b2_frame->history_generation);
	else
#endif
		b2_hack_guide_snapshot(NULL, NULL, NULL, NULL, 0, 0, false, false, 0, 0, 0);
}


#ifdef REFERENCE_B_B2
#include "../../../reference-realignment391/correction392/source/b2_menu151_renderer392.inc"
#endif

/* renders the map ingame */
void
R_RenderFrame_RTX(refdef_t *fd)
{
    if (fd->b2_active && !b2_frame_settings_valid580()) b2_settings_rejected580 = true;
    if (b2_settings_rejected580) {
        b2_settings_notice580();
        fd->b2_render_accepted = false;
        vkpt_refdef.fd = fd;
        b2ds_render_view.valid = false;
        return;
    }
	int b2_denoiser_branch = 0;
	b2ds_render_view.valid = false;
	bool b2_active = false;
	bool b2_primary = false;
	float b2_duration = 0.f;
	uint32_t b2_logical_index = 0;
	if (!qvk.swap_chain
		&& !fd->b2_active
	) return;

    if (fd->b2_active && (fd->rdflags & RDF_NOWORLDMODEL)) {
        b2_render_surface_eye(fd);return;
    }
	if (fd->b2_active)
	{
		const b2_frame_t *frame = fd->b2_frame;
		bool first_pair = !qvk.b2_temporal_activated;
		fd->b2_render_accepted = false;
		if (SDL_AtomicGet(&b2_validation_failed))
		{
			B2XR_RequestStop();
			Com_Error(ERR_FATAL, "B2_FAIL: Vulkan validation error latched before eye rendering");
		}
		if (!B2XR_Requested() || !frame || !B2_ValidViewCount(frame->view_count) || frame->view_count != B2XR_ViewCount() || fd->b2_eye >= fd->b2_frame->view_count ||
			qvk.device_count != 1 || qvk.queue_idx_graphics != qvk.queue_idx_transfer ||
			frame->duration_ns <= 0 || !frame->tracking_valid || !frame->should_render ||
			qvk.surf_is_hdr ||
			qvk.extent_unscaled.width != B2G_Width() || qvk.extent_unscaled.height != B2G_Height() ||
			qvk.extent_render.width != B2G_Width() || qvk.extent_render.height != B2G_Height() ||
			qvk.extent_taa_images.width != B2G_Width() || qvk.extent_taa_images.height != B2G_Height() ||
			(fd->rdflags & RDF_NOWORLDMODEL) || !vkpt_refdef.bsp_mesh_world_loaded)
			Com_Error(ERR_FATAL, "B2_FAIL: invalid frame identity or owned render resources");
		b2_world_first_frame(fd);
		b2_active = true;
		b2_primary = fd->b2_eye == 0;
		b2_duration = (float)((double)frame->duration_ns * 1e-9);
		b2_logical_index = frame->logical_index;
		if (first_pair)
		{
            if (!b2_primary || frame->logical_index != 0 || frame->history_valid)
                Com_Error(ERR_FATAL, "VR graphics could not start: invalid initial frame. See startup-status.txt.");
            if (!vkpt_b2_temporal_activate())
                Com_Error(ERR_FATAL, "%s", vkpt_b2_history_failure582());
			memset(&b2_cadence, 0, sizeof(b2_cadence));
			b2_cadence.view_count = frame->view_count;
		}
		if (b2_primary)
		{
			if (b2_cadence.pair_prepared || (!first_pair && !b2_settings_history_reset580 &&
				(b2_cadence.finished_mask != B2_ViewMask(b2_cadence.view_count) || b2_cadence.logical_index == UINT32_MAX ||
				 frame->logical_index != b2_cadence.logical_index + 1u)))
				Com_Error(ERR_FATAL, "B2_FAIL: live pair advanced without both completed eyes");
			b2_settings_history_reset580 = false;
			b2_cadence.logical_index = frame->logical_index;
			b2_cadence.history_generation = frame->history_generation;
			b2_cadence.rendered_mask = b2_cadence.finished_mask = 0;
			b2_cadence.prepares = b2_cadence.light_uploads = b2_cadence.light_clears = 0;
			b2_cadence.bloom_updates = b2_cadence.tone_updates = 0;
			memset(b2_cadence.tone_applies, 0, sizeof(b2_cadence.tone_applies));
		}
		else if (!b2_cadence.pair_prepared || b2_cadence.finished_mask != ((1u << fd->b2_eye) - 1u) ||
			b2_cadence.logical_index != frame->logical_index ||
			b2_cadence.history_generation != frame->history_generation)
			Com_Error(ERR_FATAL, "B2_FAIL: right eye does not own the current left-eye pair");
		temporal_frame_valid = frame->history_valid;
	}



	vkpt_refdef.fd = fd;
	const bool primary_update = (!b2_active || b2_primary);
	bool render_world = (fd->rdflags & RDF_NOWORLDMODEL) == 0;

	static float previous_time = -1.f;
	float frame_time = b2_active ? b2_duration :
		min(1.f, max(0.f, fd->time - previous_time));
	if (primary_update)
		previous_time = fd->time;

	if (primary_update)
		vkpt_freecam_update(cls.frametime);

	static unsigned previous_wallclock_time = 0;
	unsigned current_wallclock_time = previous_wallclock_time;
	float frame_wallclock_time = 0.f;
	if (primary_update)
	{
		current_wallclock_time = Sys_Milliseconds();
		frame_wallclock_time = (previous_wallclock_time != 0) ?
			(float)(current_wallclock_time - previous_wallclock_time) * 1e-3f : 0.f;
		previous_wallclock_time = current_wallclock_time;
	}

	if (!temporal_frame_valid && primary_update)
	{
		if (vkpt_refdef.fd && vkpt_refdef.fd->lightstyles)
			memcpy(vkpt_refdef.prev_lightstyles, vkpt_refdef.fd->lightstyles, sizeof(vkpt_refdef.prev_lightstyles));
		else
			memset(vkpt_refdef.prev_lightstyles, 0, sizeof(vkpt_refdef.prev_lightstyles));
	}

	mleaf_t* viewleaf = bsp_world_model ? BSP_PointLeaf(bsp_world_model->nodes, fd->vieworg) : NULL;
	
	bool sun_visible_prev = false;
	if (b2_active)
		sun_visible_prev = b2_cadence.sun_visible;
	static float prev_adapted_luminance = 0.f;
	float adapted_luminance = 0.f;
	if (b2_active)
	{
		if (b2_primary)
		{
			if (b2_logical_index == 0)
			{
				b2_cadence.feedback.viewcluster = viewleaf ? viewleaf->cluster : -1;
				b2_cadence.adapted_luminance = 0.005f;
			}
			else if (!process_render_feedback(&b2_cadence.feedback, viewleaf,
				&b2_cadence.sun_visible, &b2_cadence.adapted_luminance))
				Com_Error(ERR_FATAL, "B2_FAIL: shared pair feedback readback failed");
			if (!isfinite(b2_cadence.adapted_luminance) || b2_cadence.adapted_luminance <= 0.f)
				Com_Error(ERR_FATAL, "B2_FAIL: shared trace luminance is invalid");
		}
		fd->feedback = b2_cadence.feedback;
		prev_adapted_luminance = adapted_luminance = b2_cadence.adapted_luminance;
		sun_visible_prev = b2_cadence.sun_visible;
	}
	else
		(void)process_render_feedback(&fd->feedback, viewleaf,
			&sun_visible_prev, &adapted_luminance);

	// Sometimes, the readback returns 1.0 luminance instead of the real value.
	// Ignore these mysterious spikes.
	if (!b2_active && adapted_luminance != 1.0f)
		prev_adapted_luminance = adapted_luminance;
	
	if (prev_adapted_luminance <= 0.f)
		prev_adapted_luminance = 0.005f;

	LOG_FUNC();
	if (!vkpt_refdef.bsp_mesh_world_loaded && render_world)
	{
		drs_last_frame_world = false;
		return;
	}

	vec3_t sky_matrix[3];
	if (primary_update)
	{
		prepare_sky_matrix(fd->time, sky_matrix);
		
		if (b2_active)
			memcpy(b2_cadence.sky_matrix, sky_matrix, sizeof(sky_matrix));
	}
	else
		memcpy(sky_matrix, b2_cadence.sky_matrix, sizeof(sky_matrix));

	sun_light_t sun_light = { 0 };
	if (render_world && primary_update)
	{
		vkpt_evaluate_sun_light(&sun_light, sky_matrix, fd->time);

		if (!vkpt_physical_sky_needs_update())
			sun_light.visible = sun_light.visible && sun_visible_prev;
		
		if (b2_active)
			b2_cadence.sun_light = sun_light;
	}
	else if (b2_active)
		sun_light = b2_cadence.sun_light;

	reference_mode_t ref_mode;
	evaluate_reference_mode(&ref_mode);
	evaluate_taa_settings(&ref_mode);
	if (b2_active && (
		qvk.extent_taa_output.width != B2G_Width() || qvk.extent_taa_output.height != B2G_Height()))
		Com_Error(ERR_FATAL, "B2_FAIL: owned output extent changed after settings admission");
	
	if (primary_update)
	{
		qvk.frame_menu_mode = cl_paused->integer == 1 && uis.menuDepth > 0 && render_world;
		
	}
	

	bool expected_world_transition;
	bool actual_gpu_material_advance;
	int old_world_anim_frame;
	int new_world_anim_frame;
	if (primary_update)
	{
		old_world_anim_frame = world_anim_frame;
		new_world_anim_frame = (int)(fd->time * 2);
		expected_world_transition = (new_world_anim_frame != old_world_anim_frame);
		world_anim_frame = new_world_anim_frame;
		
		if (b2_active)
		{
			b2_cadence.update_world_animations = expected_world_transition;
			b2_cadence.old_world_anim_frame = old_world_anim_frame;
			b2_cadence.new_world_anim_frame = new_world_anim_frame;
		}
	}
	else
	{
		if (b2_active)
		{
			expected_world_transition = b2_cadence.update_world_animations;
			old_world_anim_frame = b2_cadence.old_world_anim_frame;
			new_world_anim_frame = b2_cadence.new_world_anim_frame;
		}
		
	}
	actual_gpu_material_advance = primary_update && expected_world_transition;

	EntityUploadInfo upload_info = { 0 };
	prepare_viewmatrix(fd);
	if (primary_update)
	{
		num_model_lights = 0;
		vkpt_pt_reset_instances();
		vkpt_shadow_map_reset_instances();
		prepare_entities(&upload_info);
	}
	else
	{
		if (b2_active)
		{
			upload_info = b2_cadence.upload_info;
			b2_restage_instances(fd->b2_eye);
			vkpt_pt_b2_restore_instance_base();
		}
		
	}
	if (bsp_world_model && render_world && primary_update)
	{
		vkpt_pt_instance_model_blas(&vkpt_refdef.bsp_mesh_world.geom_opaque,      g_identity_transform, VERTEX_BUFFER_WORLD, -1, 0);
		vkpt_pt_instance_model_blas(&vkpt_refdef.bsp_mesh_world.geom_transparent, g_identity_transform, VERTEX_BUFFER_WORLD, -1, 0);
		vkpt_pt_instance_model_blas(&vkpt_refdef.bsp_mesh_world.geom_masked,      g_identity_transform, VERTEX_BUFFER_WORLD, -1, 0);
		vkpt_pt_instance_model_blas(&vkpt_refdef.bsp_mesh_world.geom_sky,         g_identity_transform, VERTEX_BUFFER_WORLD, -1, 0);
		vkpt_pt_instance_model_blas(&vkpt_refdef.bsp_mesh_world.geom_custom_sky,  g_identity_transform, VERTEX_BUFFER_WORLD, -1, 0);

		vkpt_build_beam_lights(model_lights, &num_model_lights, MAX_MODEL_LIGHTS, bsp_world_model, fd->entities, fd->num_entities, prev_adapted_luminance);
	}
	
	if (b2_primary)
	{
		b2_prepare_role_instances(fd);
		b2_cadence.upload_info = upload_info;
		vkpt_pt_b2_save_instance_base();
	}

	vkpt_vertex_buffer_ensure_primbuf_size(upload_info.num_prims);

	QVKUniformBuffer_t *ubo = &vkpt_refdef.uniform_buffer;
	prepare_ubo(fd, viewleaf, &ref_mode, sky_matrix, render_world);
	ubo->prev_adapted_luminance = prev_adapted_luminance;

	if (cvar_tm_blend_enable->integer)
		Vector4Copy(fd->blend, ubo->fs_blend_color);
	else
		Vector4Set(ubo->fs_blend_color, 0.f, 0.f, 0.f, 0.f);
    if (fd->b2_active)
        Vector4Copy(ubo->fs_blend_color, b2_screen_blends[fd->b2_eye]);

	ubo->weapon_left_handed = upload_info.weapon_left_handed;

	if (vkpt_refdef.fd->rdflags & RDF_IRGOGGLES)
		Vector4Set(ubo->fs_colorize, 1.f, 0.f, 0.f, 0.8f);
	else
		Vector4Set(ubo->fs_colorize, 0.f, 0.f, 0.f, 0.f);

	vkpt_physical_sky_update_ubo(ubo, &sun_light, render_world);
	if (b2_active)
	{
		if (b2_primary)
		{
			vkpt_bloom_advance(&b2_cadence.bloom, frame_time,
				ubo->medium != MEDIUM_NONE, qvk.frame_menu_mode);
			b2_cadence.bloom_updates++;
		}
		vkpt_bloom_apply(ubo, &b2_cadence.bloom);
	}
	else
		vkpt_bloom_update(ubo, frame_time, ubo->medium != MEDIUM_NONE, qvk.frame_menu_mode);

	if (expected_world_transition && primary_update)
	{
		bsp_mesh_animate_light_polys(&vkpt_refdef.bsp_mesh_world);
		
	}
	vec3_t sky_radiance;
	VectorScale(avg_envmap_color, ubo->pt_env_scale, sky_radiance);
	if (primary_update)
	{
		uint32_t history_identity = b2_active ? b2_logical_index : qvk.frame_counter;
		VkResult light_upload_result = vkpt_light_buffer_upload_to_staging(render_world,
			&vkpt_refdef.bsp_mesh_world,
			bsp_world_model, num_model_lights, model_lights, sky_radiance, history_identity);
		if (b2_active && light_upload_result != VK_SUCCESS)
			Com_Error(ERR_FATAL, "B2_FAIL: shared light staging failed");
		if (b2_active)
			b2_cadence.light_uploads++;
		
		
	}
	
	
	
	float shadowmap_view_proj[16];
	float shadowmap_depth_scale;
	vkpt_shadow_map_setup(
		&sun_light,
		vkpt_refdef.bsp_mesh_world.world_aabb.mins,
		vkpt_refdef.bsp_mesh_world.world_aabb.maxs,
		shadowmap_view_proj,
		&shadowmap_depth_scale,
		ref_mode.enable_accumulation && num_accumulated_frames > 1);

	vkpt_god_rays_prepare_ubo(
		ubo,
		&vkpt_refdef.bsp_mesh_world.world_aabb,
		*ubo->P,
		*ubo->V,
		shadowmap_view_proj,
		shadowmap_depth_scale);

	if (vkpt_debugdraw_have())
		vkpt_debugdraw_prepare();


	bool god_rays_enabled = vkpt_god_rays_enabled(&sun_light) && render_world;

	VkSemaphore transfer_semaphores[VKPT_MAX_GPUS];
	VkSemaphore trace_semaphores[VKPT_MAX_GPUS];
	VkSemaphore transfer_wait_semaphores[VKPT_MAX_GPUS + 1];
	VkPipelineStageFlags wait_stages[VKPT_MAX_GPUS];
	VkPipelineStageFlags transfer_wait_stages[VKPT_MAX_GPUS + 1];
	uint32_t device_indices[VKPT_MAX_GPUS];
	uint32_t transfer_wait_device_indices[VKPT_MAX_GPUS + 1];
	uint32_t transfer_wait_count = 0;
	uint32_t all_device_mask = (1 << qvk.device_count) - 1;
	bool* prev_trace_signaled = &qvk.semaphores[(qvk.current_frame_index - 1) % MAX_FRAMES_IN_FLIGHT][0].trace_signaled;
	bool* curr_trace_signaled = &qvk.semaphores[qvk.current_frame_index][0].trace_signaled;

	{
		// Transfer the light buffer from staging into device memory.
		// Previous frame's tracing still uses device memory, so only do the copy after that is finished.

		VkCommandBuffer transfer_cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_transfer);
		if (b2_active)
		{
			/* Serial same-family route: retain all preceding eye reads/writes,
			 * including shared scratch, IQM and pooled light statistics. */
			VkMemoryBarrier barrier = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
				.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT
			};
			vkCmdPipelineBarrier(transfer_cmd_buf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);
		}

		if (primary_update)
		{
			vkpt_light_buffer_upload_staging(transfer_cmd_buf);
			if (b2_active)
			{
				VkResult clear_result = b2_logical_index == 0 ?
					vkpt_light_stats_clear_all(transfer_cmd_buf) :
					vkpt_light_stats_clear(transfer_cmd_buf, b2_logical_index);
				if (clear_result != VK_SUCCESS)
					Com_Error(ERR_FATAL, "B2_FAIL: shared logical light-stat clear failed");
				b2_cadence.light_clears++;
			}
			else
				vkpt_light_stats_clear(transfer_cmd_buf, qvk.frame_counter);
		}
		vkpt_iqm_matrix_buffer_upload_staging(transfer_cmd_buf);

		for (int gpu = 0; gpu < qvk.device_count; gpu++)
		{
			device_indices[gpu] = gpu;
			transfer_semaphores[gpu] = qvk.semaphores[qvk.current_frame_index][gpu].transfer_finished;
			trace_semaphores[gpu] = qvk.semaphores[qvk.current_frame_index][gpu].trace_finished;
			wait_stages[gpu] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			if (*prev_trace_signaled)
			{
				transfer_wait_semaphores[transfer_wait_count] =
					qvk.semaphores[(qvk.current_frame_index - 1) %
						MAX_FRAMES_IN_FLIGHT][gpu].trace_finished;
				transfer_wait_stages[transfer_wait_count] =
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				transfer_wait_device_indices[transfer_wait_count++] = gpu;
			}
		}

		vkpt_submit_command_buffer(
			transfer_cmd_buf, 
			qvk.queue_transfer, 
			all_device_mask, 
			transfer_wait_count, transfer_wait_semaphores,
			transfer_wait_stages, transfer_wait_device_indices,
			qvk.device_count, transfer_semaphores, device_indices, 
			VK_NULL_HANDLE);

		*prev_trace_signaled = false;
	}

	{
		VkCommandBuffer trace_cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);


		update_transparency(trace_cmd_buf, *ubo->V, fd->particles, fd->num_particles, fd->entities, fd->num_entities);
        if (fd->b2_active) b2_note_particle_upload(fd);

		// Copy the UBO contents from the staging buffer.
		// Actual contents are uploaded to the staging UBO below, right before executing the command buffer.
		vkpt_uniform_buffer_copy_from_staging(trace_cmd_buf);

		// put a profiler query without a marker for the frame begin/end - because markers do not 
		// work well across different command lists
		_VK(vkpt_profiler_query(trace_cmd_buf, PROFILER_FRAME_TIME, PROFILER_START));

		BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_UPDATE_ENVIRONMENT);
		if (render_world && primary_update)
		{
			vkpt_physical_sky_record_cmd_buffer(trace_cmd_buf);
		}
		END_PERF_MARKER(trace_cmd_buf, PROFILER_UPDATE_ENVIRONMENT);

		BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_INSTANCE_GEOMETRY);
		vkpt_instance_geometry_audit_t *material_audit = NULL;
		vkpt_instance_geometry(trace_cmd_buf, upload_info.num_instances,
			actual_gpu_material_advance, material_audit);
		END_PERF_MARKER(trace_cmd_buf, PROFILER_INSTANCE_GEOMETRY);

		BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_BVH_UPDATE);
		vkpt_pt_create_all_dynamic(trace_cmd_buf, qvk.current_frame_index, &upload_info);
		vkpt_pt_create_toplevel(trace_cmd_buf, qvk.current_frame_index, &upload_info, upload_info.weapon_left_handed);
		vkpt_pt_update_descripter_set_bindings(qvk.current_frame_index);
		END_PERF_MARKER(trace_cmd_buf, PROFILER_BVH_UPDATE);

		BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_SHADOW_MAP);
		if (god_rays_enabled)
		{
			vkpt_shadow_map_render(trace_cmd_buf, shadowmap_view_proj,
				vkpt_refdef.bsp_mesh_world.geom_opaque.prim_offsets[0] * 3,
				vkpt_refdef.bsp_mesh_world.geom_opaque.prim_counts[0] * 3,
				0,
				upload_info.opaque_prim_count * 3,
				vkpt_refdef.bsp_mesh_world.geom_transparent.prim_offsets[0] * 3,
				vkpt_refdef.bsp_mesh_world.geom_transparent.prim_counts[0] * 3);
		}
		END_PERF_MARKER(trace_cmd_buf, PROFILER_SHADOW_MAP);

		vkpt_pt_trace_primary_rays(trace_cmd_buf);

		// The host-side image of the uniform buffer is only ready after the `vkpt_pt_create_toplevel` call above
		_VK(vkpt_uniform_buffer_upload_to_staging());

		vkpt_submit_command_buffer(
			trace_cmd_buf,
			qvk.queue_graphics,
			all_device_mask,
			qvk.device_count, transfer_semaphores, wait_stages, device_indices,
			0, 0, 0,
			VK_NULL_HANDLE);
	}

	if (vkpt_debugdraw_have())
	{
		VkCommandBuffer lines_cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);

		if (qvkCmdBeginDebugUtilsLabelEXT != NULL)
		{
			const VkDebugUtilsLabelEXT label = {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
				.pLabelName = "debug lines"
			};
			qvkCmdBeginDebugUtilsLabelEXT(lines_cmd_buf, &label);
		}
		vkpt_debugdraw_draw(lines_cmd_buf);
		if (qvkCmdEndDebugUtilsLabelEXT != NULL)
			qvkCmdEndDebugUtilsLabelEXT(lines_cmd_buf);

		vkpt_submit_command_buffer_simple(lines_cmd_buf, qvk.queue_graphics, false);
	}

	{
		VkCommandBuffer trace_cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);

		if (god_rays_enabled)
		{
			BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_GOD_RAYS);
			vkpt_record_god_rays_trace_command_buffer(trace_cmd_buf, 0);
			END_PERF_MARKER(trace_cmd_buf, PROFILER_GOD_RAYS);
		}

		if (ref_mode.reflect_refract > 0)
		{
			BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_REFLECT_REFRACT_1);
			vkpt_pt_trace_reflections(trace_cmd_buf, 0);
			END_PERF_MARKER(trace_cmd_buf, PROFILER_REFLECT_REFRACT_1);
		}

		if (god_rays_enabled)
		{
			if (ref_mode.reflect_refract > 0)
			{
				BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_GOD_RAYS_REFLECT_REFRACT);
				vkpt_record_god_rays_trace_command_buffer(trace_cmd_buf, 1);
				END_PERF_MARKER(trace_cmd_buf, PROFILER_GOD_RAYS_REFLECT_REFRACT);
			}

			BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_GOD_RAYS_FILTER);
			vkpt_record_god_rays_filter_command_buffer(trace_cmd_buf);
			END_PERF_MARKER(trace_cmd_buf, PROFILER_GOD_RAYS_FILTER);
		}

		if (ref_mode.reflect_refract > 1)
		{
			BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_REFLECT_REFRACT_2);
			for (int pass = 0; pass < ref_mode.reflect_refract - 1; pass++)
			{
				vkpt_pt_trace_reflections(trace_cmd_buf, pass + 1);
			}
			END_PERF_MARKER(trace_cmd_buf, PROFILER_REFLECT_REFRACT_2);
		}

		if (ref_mode.enable_denoiser)
		{
			BEGIN_PERF_MARKER(trace_cmd_buf, PROFILER_ASVGF_GRADIENT_REPROJECT);
			vkpt_asvgf_gradient_reproject(trace_cmd_buf);
			END_PERF_MARKER(trace_cmd_buf, PROFILER_ASVGF_GRADIENT_REPROJECT);
		}

		vkpt_pt_trace_lighting(trace_cmd_buf, ref_mode.num_bounce_rays);
		
		vkpt_submit_command_buffer(
			trace_cmd_buf,
			qvk.queue_graphics,
			all_device_mask,
			0, 0, 0, 0,
			qvk.device_count, trace_semaphores, device_indices,
			VK_NULL_HANDLE);

		*curr_trace_signaled = true;
	}

	{
		VkCommandBuffer post_cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);

		BEGIN_PERF_MARKER(post_cmd_buf, PROFILER_ASVGF_FULL);
		if (ref_mode.enable_denoiser)
		{
			vkpt_asvgf_filter(post_cmd_buf, cvar_pt_num_bounce_rays->value >= 0.5f);
			b2_denoiser_branch = 1;
		}
		else
		{
			vkpt_compositing(post_cmd_buf);
			b2_denoiser_branch = 2;
		}
		END_PERF_MARKER(post_cmd_buf, PROFILER_ASVGF_FULL);

		vkpt_interleave(post_cmd_buf);

		vkpt_taa(post_cmd_buf);

		BEGIN_PERF_MARKER(post_cmd_buf, PROFILER_BLOOM);
		if (cvar_bloom_enable->integer != 0 || qvk.frame_menu_mode)
		{

			vkpt_bloom_record_cmd_buffer(post_cmd_buf);
		}

		END_PERF_MARKER(post_cmd_buf, PROFILER_BLOOM);

#ifdef VKPT_IMAGE_DUMPS
		if (cvar_dump_image->integer)
		{
			copy_to_dump_texture(post_cmd_buf, VKPT_IMG_TAA_OUTPUT);
		}
#endif

		BEGIN_PERF_MARKER(post_cmd_buf, PROFILER_TONE_MAPPING);
		if (cvar_tm_enable->integer != 0)
		{
			float tone_duration = frame_time <= 0.f ? frame_wallclock_time : frame_time;
			if (b2_active)
			{
				if (b2_primary && vkpt_tone_mapping_record_update(post_cmd_buf, tone_duration) != VK_SUCCESS)
					Com_Error(ERR_FATAL, "B2_FAIL: display tone update failed");
				if (b2_primary)
					b2_cadence.tone_updates++;

				if (vkpt_tone_mapping_record_apply(post_cmd_buf) != VK_SUCCESS)
					Com_Error(ERR_FATAL, "B2_FAIL: eye display tone application failed");
				b2_cadence.tone_applies[fd->b2_eye]++;
			}
			else
				vkpt_tone_mapping_record_cmd_buffer(post_cmd_buf, tone_duration);
		}
		END_PERF_MARKER(post_cmd_buf, PROFILER_TONE_MAPPING);

		// Skip FSR (upscaling) if image is going to be heavily blurred anyway (menu mode)
		if(vkpt_fsr_is_enabled() && !qvk.frame_menu_mode)
		{
			vkpt_fsr_do(post_cmd_buf);
		}

		{
			VkBufferCopy copyRegion = { 0, 0, sizeof(ReadbackBuffer) };

			vkCmdCopyBuffer(post_cmd_buf, qvk.buf_readback.buffer, qvk.buf_readback_staging[qvk.current_frame_index].buffer, 1, &copyRegion);
		}

		_VK(vkpt_profiler_query(post_cmd_buf, PROFILER_FRAME_TIME, PROFILER_STOP));

		vkpt_submit_command_buffer_simple(post_cmd_buf, qvk.queue_graphics, true);
	}

	temporal_frame_valid = ref_mode.enable_denoiser;
	
	frame_ready = true;
	drs_last_frame_world = true;
	if (b2_active)
	{
		if (b2_cadence.rendered_mask != ((1u << fd->b2_eye) - 1u))
			Com_Error(ERR_FATAL, "B2_FAIL: duplicate or out-of-order eye render");
		b2_cadence.rendered_mask |= 1u << fd->b2_eye;
		fd->b2_render_accepted = true;
		b2ds_capture_view(fd, ubo, b2_denoiser_branch);
		b2_hack_guide_submitted(fd->b2_eye, fd->b2_frame->logical_index,
			fd->b2_frame->history_generation);
	}

	if (vkpt_refdef.fd && vkpt_refdef.fd->lightstyles && primary_update) {
		memcpy(vkpt_refdef.prev_lightstyles, vkpt_refdef.fd->lightstyles, sizeof(vkpt_refdef.prev_lightstyles));
	}


	R_ExpireDebugLines();
}

static void temporal_cvar_changed(cvar_t *self)
{
#ifdef REFERENCE_B_B2
	if (qvk.b2_temporal_activated) {
        b2_settings_changed580 = true;
        b2_settings_notice580();
    }
#endif
	temporal_frame_valid = false;
}

static void
recreate_swapchain(void)
{
#ifdef REFERENCE_B_B2
    if (b2_desktop_failed582) {
        qvk.surf_vsync = cvar_vsync->integer != 0;
        return;
    }
	if (b2_desktop_enabled()) {
		b2_desktop_dirty534 = true;
		// Complete all views against the same acquisition before replacing it.
		if (b2_mirror.held) return;
		int width, height;
		SDL_Vulkan_GetDrawableSize(qvk.window, &width, &height);
		b2_note_desktop_drawable(width, height);
		if (width < 2 || height < 2) return;
		VkSurfaceCapabilitiesKHR caps;
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(qvk.physical_device, qvk.surface, &caps);
		if (result != VK_SUCCESS)
			{ b2_disable_desktop582("surface-query", result); return; }
		if (caps.currentExtent.width < 2 || caps.currentExtent.height < 2) return;
		
		// R6-B1: only desktop attachments change; held-pair and idle checks
		// below protect them without opening world/history maintenance.
		result = vkDeviceWaitIdle(qvk.device);
        if (result != VK_SUCCESS) {
            Com_EPrintf("B2_DESKTOP_FAILURE operation=retirement result=%s\n", qvk_result_to_string(result));
            vr_device_lost_terminal = result == VK_ERROR_DEVICE_LOST;
            vr_abandon_children = true;
            Com_Error(ERR_FATAL, "Graphics work could not complete. Restart Q2RTX VR after checking the graphics driver.");
        }
		VkSurfaceFormatKHR previous_format = qvk.surf_format;
		vkpt_draw_destroy_desktop534();
		result = destroy_swapchain();
		if (result == VK_SUCCESS) result = create_swapchain();
		if (result == VK_ERROR_OUT_OF_DATE_KHR || (result == VK_SUCCESS && !qvk.swap_chain)) return;
		if (result != VK_SUCCESS)
			{ b2_disable_desktop582("recreation", result); return; }
		// Existing SDR render pass/pipelines remain compatible across size changes.
		if (previous_format.format != qvk.surf_format.format || previous_format.colorSpace != qvk.surf_format.colorSpace)
			{ b2_disable_desktop582("format-change", VK_ERROR_FORMAT_NOT_SUPPORTED); return; }
		result = vkpt_draw_create_desktop534();
		if (result != VK_SUCCESS)
			{ b2_disable_desktop582("framebuffers", result); return; }
		b2_desktop_dirty534 = false;
		return; // No image/history/HUD destruction or extent changes.
	}
	// Retire a partial acquisition only through fatal device/swapchain teardown.
	// Never acquire a replacement image and mistake it for the retained left half.
	if (b2_mirror.held)
		Com_Error(ERR_FATAL, "B2_FAIL: desktop resize during incomplete pair; restart required");
#endif
	VkResult result;
	
	vkpt_vr_require_maintenance_open("swapchain/resource recreation");
	result = vkDeviceWaitIdle(qvk.device);
	if (result == VK_SUCCESS)
		result = vkpt_destroy_all(VKPT_INIT_SWAPCHAIN_RECREATE);
	if (result == VK_SUCCESS)
		result = destroy_swapchain();
	if (result != VK_SUCCESS)
	{
		if (result == VK_ERROR_DEVICE_LOST)
			vr_device_lost_terminal = true;
		vr_abandon_children = true;
		Com_Error(ERR_FATAL,
			"VR_FAIL: swapchain pre-recreation boundary failed (%s)",
			qvk_result_to_string(result));
	}
#ifdef REFERENCE_VR_HISTORY
	temporal_frame_valid = false;
#endif
	SDL_Vulkan_GetDrawableSize(qvk.window, &qvk.draw_width, &qvk.draw_height);
#ifdef REFERENCE_B_B2
	if (b2_desktop_enabled())
		b2_note_desktop_drawable(qvk.draw_width, qvk.draw_height);
	else
#endif
	{
	r_config.width = qvk.draw_width;
	r_config.height = qvk.draw_height;
	}
	result = create_swapchain();
	if (result == VK_SUCCESS)
		result = vkpt_initialize_all(VKPT_INIT_SWAPCHAIN_RECREATE);
	if (result != VK_SUCCESS)
	{
		if (result == VK_ERROR_DEVICE_LOST)
			vr_device_lost_terminal = true;
		vr_abandon_children = true;
		Com_Error(ERR_FATAL, "VR_FAIL: swapchain recreation failed (%s)",
			qvk_result_to_string(result));
	}

	qvk.wait_for_idle_frames = MAX_FRAMES_IN_FLIGHT * 2;
}

static int compare_doubles(const void* pa, const void* pb)
{
	double a = *(double*)pa;
	double b = *(double*)pb;

	if (a < b) return -1; 
	if (a > b) return 1;
	return 0;
}

// DRS (Dynamic Resolution Scaling) functions

static void drs_init(void)
{
	cvar_drs_enable = Cvar_Get("drs_enable", "0", CVAR_ARCHIVE);
	// Target FPS value
	cvar_drs_target = Cvar_Get("drs_target", "60", CVAR_ARCHIVE);
	cvar_drs_target->changed = drs_target_changed;
	// Minimum resolution scale in percents
	cvar_drs_minscale = Cvar_Get("drs_minscale", "50", 0);
	cvar_drs_minscale->changed = drs_minscale_changed;
	// Maximum resolution scale in percents
	cvar_drs_maxscale = Cvar_Get("drs_maxscale", "100", 0);
	cvar_drs_maxscale->changed = drs_maxscale_changed;
	// Resolution regulator parameters, see the `dynamic_resolution_scaling()` function
	cvar_drs_gain = Cvar_Get("drs_gain", "20", 0);
	cvar_drs_adjust_up = Cvar_Get("drs_adjust_up", "0.92", 0);
	cvar_drs_adjust_down = Cvar_Get("drs_adjust_down", "0.98", 0);

	// Value of drs_current_scale from last run. To start with a close-to-desired scale
	cvar_drs_last_scale = Cvar_Get("drs_last_scale", "0", CVAR_ARCHIVE);
}

static void drs_process(void)
{
#define SCALING_FRAMES 5
	static int num_valid_frames = 0;
	static double valid_frame_times[SCALING_FRAMES];

	if (cvar_drs_enable->integer == 0)
	{
		num_valid_frames = 0;

		if (is_accumulation_rendering_active())
			drs_effective_scale = max(100, scr_viewsize->integer);
		else
			drs_effective_scale = 0;

		return;
	}

	if (is_accumulation_rendering_active())
	{
		num_valid_frames = 0;
		drs_effective_scale = max(cvar_drs_minscale->integer, cvar_drs_maxscale->integer);
		return;
	}

	int last_scale = drs_current_scale;
	if(!last_scale)
		last_scale = cvar_drs_last_scale->integer;
	if(!last_scale)
		last_scale = scr_viewsize->integer;

	if (!drs_last_frame_world)
	{
		/* Last frame world was not rendered:
		* Frame times wouldn't be representative, so don't adjust DRS.
		* If DRS wasn't adjusted previously return a constrained default value */
		drs_effective_scale = max(cvar_drs_minscale->integer, min(cvar_drs_maxscale->integer, last_scale));
		return;
	}

	drs_effective_scale = last_scale;

	double ms = vkpt_get_profiler_result(PROFILER_FRAME_TIME);

	// 0ms frame may happen if we just played a cinematic
	if (ms <= 0 || ms > 1000)
		return;

	valid_frame_times[num_valid_frames] = ms;
	num_valid_frames++;

	if (num_valid_frames < SCALING_FRAMES)
		return;

	num_valid_frames = 0;

	qsort(valid_frame_times, SCALING_FRAMES, sizeof(double), compare_doubles);

	double representative_time = 0;
	for(int i = 1; i < SCALING_FRAMES - 1; i++)
		representative_time += valid_frame_times[i];
	representative_time /= (SCALING_FRAMES - 2);

	double target_time = 1000.0 / cvar_drs_target->value;
	double f = cvar_drs_gain->value * (1.0 - representative_time / target_time) - 1.0;

	int scale = drs_effective_scale;
	if (representative_time < target_time * cvar_drs_adjust_up->value)
	{
		f += 0.5;
		f = Q_clipf(f, 1, 10);
		scale += (int)f;
	}
	else if (representative_time > target_time * cvar_drs_adjust_down->value)
	{
		f -= 0.5;
		f = Q_clipf(f, -1, -10);
		scale += f;
	}

	drs_current_scale = max(cvar_drs_minscale->integer, min(cvar_drs_maxscale->integer, scale));
	drs_effective_scale = drs_current_scale;
}

static bool b2_frame_settings_valid580(void)
{
    return (!b2_settings_changed580 || b2_callback_settings_compatible580()) && !cvar_hdr->integer &&
        !cvar_pt_projection->integer && !cvar_drs_enable->integer &&
        !cvar_pt_freecam->integer && cvar_tm_enable->integer &&
        !cvar_pt_accumulation_rendering->integer && !vkpt_fsr_is_enabled() &&
        !Cvar_VariableInteger("bloom_debug");
}

void
R_BeginFrame_RTX(void)
{
	LOG_FUNC();
    b2_physical_started580 = false;
    b2_settings_rejected580 = B2XR_Requested() && !b2_frame_settings_valid580();
    if (b2_settings_rejected580) {
        frame_ready = false;
        b2_settings_notice580();
        return;
    }

	qvk.current_frame_index = qvk.frame_counter % MAX_FRAMES_IN_FLIGHT;

	VkResult res_fence = vkWaitForFences(qvk.device, 1, qvk.fences_frame_sync + qvk.current_frame_index, VK_TRUE, ~((uint64_t) 0));
	
	if (res_fence == VK_ERROR_DEVICE_LOST)
	{
		vr_device_lost_terminal = true;
		vr_abandon_children = true;
		
        Com_EPrintf("B2_DEVICE_LOST operation=frame-fence\n");
        Com_Error(ERR_FATAL, "The graphics device stopped responding. Restart Q2RTX VR after checking the graphics driver.");
	}
	if (res_fence != VK_SUCCESS)
	{
		vr_abandon_children = true;
		Com_Error(ERR_FATAL,
			"VR_FAIL: physical-slot pre-acquire fence wait failed (%s)",
			qvk_result_to_string(res_fence));
	}

	bool mode_changed = (qvk.draw_width != r_config.width) || (qvk.draw_height != r_config.height);
#ifdef REFERENCE_B_B2
	if (b2_desktop_enabled()) {
		int width, height;
		SDL_Vulkan_GetDrawableSize(qvk.window, &width, &height);
		b2_note_desktop_drawable(width, height);
		mode_changed = b2_desktop_dirty534;
	}
#endif
	if ((!qvk.swap_chain || mode_changed)
#ifdef REFERENCE_B_B2
        // Surface queries can trigger teardown. Defer them until the entire
        // acquired eye pair has retired, as recreate_swapchain already does.
        && !b2_desktop_failed582 && !b2_mirror.held
#endif
    )
	{
		VkSurfaceCapabilitiesKHR surf_capabilities;
		VkResult surface_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(qvk.physical_device, qvk.surface, &surf_capabilities);
        if (surface_result != VK_SUCCESS) {
#ifdef REFERENCE_B_B2
            if (b2_desktop_enabled()) b2_disable_desktop582("surface-query",surface_result);
            else
#endif
                Com_Error(ERR_FATAL, "VR desktop surface query failed (%s)", qvk_result_to_string(surface_result));
        }

		// see if we're un-minimized again
		if (surface_result == VK_SUCCESS && surf_capabilities.currentExtent.width != 0 && surf_capabilities.currentExtent.height != 0)
		{
			recreate_swapchain();
		}
	}

	drs_process();
	if (vkpt_refdef.fd)
	{
		vkpt_refdef.fd->feedback.resolution_scale = (drs_effective_scale != 0) ? drs_effective_scale : scr_viewsize->integer;
#ifdef REFERENCE_B_B2
        if (B2XR_Requested()) vkpt_refdef.fd->feedback.resolution_scale=100;
#endif
	}

#ifdef REFERENCE_VR_HISTORY
	VkExtent2D previous_render_extent = qvk.extent_render;
#endif
	qvk.extent_render = get_render_extent();
#ifdef REFERENCE_VR_HISTORY
	if (!extents_equal(previous_render_extent, qvk.extent_render))
		temporal_frame_valid = false;
#endif
	qvk.gpu_slice_width = (qvk.extent_render.width + qvk.device_count - 1) / qvk.device_count;
	
	VkExtent2D extent_screen_images = get_screen_image_extent();

	if(!extents_equal(extent_screen_images, qvk.extent_screen_images) || (!!cvar_hdr->integer != qvk.surf_is_hdr) || (!!cvar_vsync->integer != qvk.surf_vsync))
	{
#ifdef REFERENCE_B_B2
		if (b2_desktop_enabled() && !extents_equal(extent_screen_images, qvk.extent_screen_images))
			Com_Error(ERR_FATAL, "B2_FAIL: internal image extent changed during desktop-only session");
#endif
		qvk.extent_screen_images = extent_screen_images;
		recreate_swapchain();
	}

retry:;

	if (!qvk.swap_chain
#ifdef REFERENCE_B_B2
		&& !b2_desktop_enabled()
#endif
	) return;

#ifdef REFERENCE_B_B2
	b2_mirror.wait_acquire = !b2_mirror_reuse();
	if (b2_mirror.wait_acquire) {
		b2_mirror.no_image534 = b2_desktop_enabled() && (b2_desktop_failed582 || !qvk.swap_chain ||
			b2_desktop_drawable.width < 2 || b2_desktop_drawable.height < 2);
		if (b2_mirror.no_image534) b2_mirror.wait_acquire = false;
	}
	if (b2_mirror.wait_acquire)
#endif
	{
#ifdef VKPT_DEVICE_GROUPS
	VkAcquireNextImageInfoKHR acquire_info = {
		.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
		.swapchain = qvk.swap_chain,
		.timeout =
#ifdef REFERENCE_B_B2
			b2_desktop_enabled() ? 0 :
#endif
			(~((uint64_t) 0)),
		.semaphore = qvk.semaphores[qvk.current_frame_index][0].image_available,
		.fence = VK_NULL_HANDLE,
		.deviceMask = (1 << qvk.device_count) - 1,
	};

	VkResult res_swapchain = vkAcquireNextImage2KHR(qvk.device, &acquire_info, &qvk.current_swap_chain_image_index);
#else
	VkResult res_swapchain = vkAcquireNextImageKHR(qvk.device, qvk.swap_chain,
#ifdef REFERENCE_B_B2
		b2_desktop_enabled() ? 0 :
#endif
		~((uint64_t) 0),
		qvk.semaphores[qvk.current_frame_index][0].image_available, VK_NULL_HANDLE, &qvk.current_swap_chain_image_index);
#endif
#ifdef REFERENCE_B_B2
	if (b2_desktop_enabled()) {
		if (res_swapchain == VK_SUBOPTIMAL_KHR) b2_desktop_dirty534 = true;
		else if (res_swapchain == VK_ERROR_OUT_OF_DATE_KHR || res_swapchain == VK_NOT_READY || res_swapchain == VK_TIMEOUT) {
			b2_mirror.no_image534 = true;
			b2_mirror.wait_acquire = false; // Nothing acquired; semaphore was not signalled.
			if (res_swapchain == VK_ERROR_OUT_OF_DATE_KHR) b2_desktop_dirty534 = true;
        } else if (res_swapchain != VK_SUCCESS) {
            b2_mirror.wait_acquire = false; // Failed acquire returned no image/semaphore signal.
            b2_disable_desktop582("acquire", res_swapchain);
        }
	} else
#endif
	if(res_swapchain == VK_ERROR_OUT_OF_DATE_KHR || res_swapchain == VK_SUBOPTIMAL_KHR) {
		recreate_swapchain();
		goto retry;
	}
	else if(res_swapchain != VK_SUCCESS) {
#ifdef REFERENCE_B_B2
		if (B2XR_Requested())
			Com_Error(ERR_FATAL, "B2_FAIL: desktop swapchain acquire failed (%s)", qvk_result_to_string(res_swapchain));
#endif
		Com_EPrintf("Error %d in vkAcquireNextImageKHR\n", res_swapchain);
	}
	}

	if (qvk.wait_for_idle_frames) {
		VkResult idle_result = vkDeviceWaitIdle(qvk.device);
		if (idle_result != VK_SUCCESS)
		{
			if (idle_result == VK_ERROR_DEVICE_LOST)
				vr_device_lost_terminal = true;
			vr_abandon_children = true;
			Com_Error(ERR_FATAL,
				"VR_FAIL: technical post-recreation idle wait failed (%s)",
				qvk_result_to_string(idle_result));
		}
		qvk.wait_for_idle_frames--;
	}

	{
		
		VkResult reset_result = vkResetFences(qvk.device, 1,
			qvk.fences_frame_sync + qvk.current_frame_index);
		if (reset_result != VK_SUCCESS)
		{
			vr_device_lost_terminal =
				reset_result == VK_ERROR_DEVICE_LOST;
			vr_abandon_children = true;
			Com_Error(ERR_FATAL,
				"VR_FAIL: physical frame-fence reset failed or overflowed");
		}
		
	}

    b2_physical_started580 = B2XR_Requested();
	vkpt_reset_command_buffers(&qvk.cmd_buffers_graphics);
	vkpt_reset_command_buffers(&qvk.cmd_buffers_transfer);

	// Process the profiler queries - always enabled to support DRS
	{
		VkCommandBuffer reset_cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);

		_VK(vkpt_profiler_next_frame(reset_cmd_buf));

		vkpt_submit_command_buffer_simple(reset_cmd_buf, qvk.queue_graphics, true);
	}

	vkpt_textures_destroy_unused();
	vkpt_textures_end_registration();
	vkpt_textures_update_descriptor_set();

	vkpt_vertex_buffer_upload_models();
	vkpt_draw_clear_stretch_pics();

	SCR_SetHudAlpha(1.f);
}

void
R_EndFrame_RTX(void)
{
	LOG_FUNC();
    if (b2_settings_rejected580) {
        b2_discard_settings_frame580();
        vkpt_draw_clear_stretch_pics();
        return;
    }

	if (!qvk.swap_chain
#ifdef REFERENCE_B_B2
		&& !b2_desktop_enabled()
#endif
	) {
		vkpt_draw_clear_stretch_pics();
		return;
	}

	if(cvar_profiler->integer)
		draw_profiler(cvar_flt_enable->integer != 0);
	if(cvar_tm_debug->integer)
		vkpt_tone_mapping_draw_debug();

	VkCommandBuffer cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);

#ifdef REFERENCE_B_B2
	bool b2_eye = vkpt_refdef.fd && vkpt_refdef.fd->b2_active;
	bool b2_left = b2_eye && vkpt_refdef.fd->b2_eye == 0;
	bool b2_last = b2_eye && vkpt_refdef.fd->b2_frame && vkpt_refdef.fd->b2_eye + 1 == vkpt_refdef.fd->b2_frame->view_count;
	if (b2_eye && (!frame_ready || !vkpt_refdef.fd->b2_render_accepted ||
		!vkpt_refdef.fd->b2_frame || vkpt_refdef.fd->b2_eye >= vkpt_refdef.fd->b2_frame->view_count ||
		b2_mirror.held != !b2_left || b2_mirror.wait_acquire != (b2_left && !b2_mirror.no_image534)))
		Com_Error(ERR_FATAL, "B2_FAIL: desktop eye submission lacks complete ordered render");
#endif
	if (frame_ready)
	{
#ifdef REFERENCE_B_B2
        // Final tone-mapped output is not temporal history (ASVGF_TAA_A/B is).
        // Every eye rewrites TAA_OUTPUT before this one flat stock UI pass.
        if (b2_eye) {
            extern void vkpt_b2_wall_return(VkCommandBuffer cmd_buf);
            vkpt_draw_submit_stretch_pics(cmd_buf);
            // Final display composite: after all eye overlays, before either copy.
            vkpt_b2_wall_return(cmd_buf);
        }
#endif
#ifdef REFERENCE_B_B2
		if (!b2_mirror.no_image534)
#endif
		{
		bool waterwarp = (vkpt_refdef.fd->rdflags & RDF_UNDERWATER) && cvar_pt_waterwarp->integer;
		if (vkpt_fsr_is_enabled() && !qvk.frame_menu_mode)
		{
			vkpt_fsr_final_blit(cmd_buf, waterwarp);
		}
		else if (qvk.effective_aa_mode == AA_MODE_UPSCALE)
		{
			vkpt_final_blit(cmd_buf, VKPT_IMG_TAA_OUTPUT, qvk.extent_taa_output, false, waterwarp);
		}
		else
		{
			VkExtent2D extent_unscaled_half;
			extent_unscaled_half.width = qvk.extent_unscaled.width / 2;
			extent_unscaled_half.height = qvk.extent_unscaled.height / 2;

			if (extents_equal(qvk.extent_render, qvk.extent_unscaled) ||
				(extents_equal(qvk.extent_render, extent_unscaled_half) && drs_effective_scale == 0)) // don't do nearest filter 2x upscale with DRS enabled
				vkpt_final_blit(cmd_buf, VKPT_IMG_TAA_OUTPUT, qvk.extent_taa_output, false, waterwarp);
			else
				vkpt_final_blit(cmd_buf, VKPT_IMG_TAA_OUTPUT, qvk.extent_taa_output, true, waterwarp);
		}

		} // Desktop blit; XR copy below is independent of presentation.
	#ifdef REFERENCE_B_B2
		if (vkpt_refdef.fd->b2_active)
		{
			if (!vkpt_refdef.fd->b2_render_accepted ||
				!B2XR_CopyEye(cmd_buf, qvk.images[VKPT_IMG_TAA_OUTPUT],
					vkpt_refdef.fd->b2_eye, B2G_Width(), B2G_Height()))
            {
                Com_EPrintf("B2_XR_COPY_REFUSED eye=%u; discarding unsubmitted final work\n", vkpt_refdef.fd->b2_eye);
                vkpt_refdef.fd->b2_render_accepted = false;
                vkpt_draw_clear_stretch_pics();
                b2_retire_xr_failure555();
                B2XR_RequestStop();
                return;
            }
		}
	#endif
		frame_ready = false;
	}

#ifdef REFERENCE_B_B2
	// Overlay the completed desktop pair once; input/focus remains on this window.
	if (b2_mirror.no_image534 || (b2_eye && !b2_last)) vkpt_draw_clear_stretch_pics();
	else
#endif
		vkpt_draw_submit_stretch_pics(cmd_buf);

	VkSemaphore wait_semaphores[] = { qvk.semaphores[qvk.current_frame_index][0].image_available };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	uint32_t wait_device_indices[] = { 0 };

	VkSemaphore signal_semaphores[VKPT_MAX_GPUS];
	uint32_t signal_device_indices[VKPT_MAX_GPUS];
	for (int gpu = 0; gpu < qvk.device_count; gpu++)
	{
		signal_semaphores[gpu] = qvk.semaphores[qvk.current_frame_index][gpu].render_finished;
		signal_device_indices[gpu] = gpu;
	}

	vkpt_submit_command_buffer(
		cmd_buf,
		qvk.queue_graphics,
		(1 << qvk.device_count) - 1,
#ifdef REFERENCE_B_B2
		b2_mirror.wait_acquire ? LENGTH(wait_semaphores) : 0, wait_semaphores, wait_stages, wait_device_indices,
		(b2_mirror.no_image534 || (b2_eye && !b2_last)) ? 0 : qvk.device_count, signal_semaphores, signal_device_indices,
#else
		LENGTH(wait_semaphores), wait_semaphores, wait_stages, wait_device_indices,
		qvk.device_count, signal_semaphores, signal_device_indices,
#endif
		qvk.fences_frame_sync[qvk.current_frame_index]);
#ifdef REFERENCE_B_B2
    b2_physical_started580 = false; // The submission now owns/signals the fence.
    b2_mirror.wait_acquire = false; // The submitted wait consumes any acquisition.
#endif

#ifdef VKPT_IMAGE_DUMPS
	if (cvar_dump_image->integer) {
		_VK(vkQueueWaitIdle(qvk.queue_graphics));

		VkImageSubresource subresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.arrayLayer = 0,
			.mipLevel = 0
		};

		VkSubresourceLayout subresource_layout;
		vkGetImageSubresourceLayout(qvk.device, qvk.dump_image, &subresource, &subresource_layout);

		void *data;
		_VK(vkMapMemory(qvk.device, qvk.dump_image_memory, 0, qvk.dump_image_memory_size, 0, &data));
		save_to_pfm_file("color_buffer", qvk.frame_counter, IMG_WIDTH, IMG_HEIGHT, (char *)data, subresource_layout.rowPitch, 0);
		vkUnmapMemory(qvk.device, qvk.dump_image_memory);

		Cvar_SetInteger(cvar_dump_image, 0, FROM_CODE);
	}
#endif

	VkPresentInfoKHR present_info = {
		.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = qvk.device_count,
		.pWaitSemaphores    = signal_semaphores,
		.swapchainCount     = 1,
		.pSwapchains        = &qvk.swap_chain,
		.pImageIndices      = &qvk.current_swap_chain_image_index,
		.pResults           = NULL,
	};

#ifdef VKPT_DEVICE_GROUPS
	uint32_t present_device_mask = 1;
	VkDeviceGroupPresentInfoKHR group_present_info = {
		.sType				= VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR,
		.swapchainCount		= 1,
		.pDeviceMasks		= &present_device_mask,
		.mode				= VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR,
	};

	if (qvk.device_count > 1) {
		present_info.pNext = &group_present_info;
	}
#endif

#ifdef REFERENCE_B_B2
	if (b2_eye && !b2_last) {
		// The acquire wait was consumed above. Do not signal a render_finished
		// semaphore with no present to consume it. Right uses its own physical slot.
		if (b2_left) {
		b2_mirror.held = true;
		b2_mirror.swapchain = qvk.swap_chain;
		b2_mirror.extent = vkpt_get_present_extent();
		b2_mirror.image = qvk.current_swap_chain_image_index;
		b2_mirror.logical_index = vkpt_refdef.fd->b2_frame->logical_index;
		b2_mirror.left_frame = qvk.frame_counter;
		}
		b2_finish_eye(vkpt_refdef.fd->b2_eye);
		qvk.frame_counter++;
		return;
	}
	if (b2_mirror.no_image534) {
		if (b2_eye) b2_finish_eye(vkpt_refdef.fd->b2_eye);
		b2_mirror.held = false;
		qvk.frame_counter++;
		return; // No desktop image/wait/signal/present; completed XR work remains valid.
	}
#endif
	VkResult res_present = vkQueuePresentKHR(qvk.queue_graphics, &present_info);
#ifdef REFERENCE_B_B2
	if (vkpt_refdef.fd && vkpt_refdef.fd->b2_active)
	{
		if (res_present != VK_SUCCESS && res_present != VK_SUBOPTIMAL_KHR && res_present != VK_ERROR_OUT_OF_DATE_KHR)
            b2_disable_desktop582("present", res_present);
		b2_finish_eye(vkpt_refdef.fd->b2_eye);
		b2_mirror.held = false;
	}
#endif
	if(res_present == VK_ERROR_OUT_OF_DATE_KHR || res_present == VK_SUBOPTIMAL_KHR) {
		recreate_swapchain();
	}
	qvk.frame_counter++;
}

void
R_ModeChanged_RTX(int width, int height, int flags)
{
	Com_DPrintf("mode changed %d %d\n", width, height);

#ifdef REFERENCE_B_B2
	if (b2_desktop_enabled())
		b2_note_desktop_drawable(width, height);
	else
#endif
	{
	r_config.width  = width;
	r_config.height = height;
	}
	r_config.flags  = flags;

	qvk.wait_for_idle_frames = MAX_FRAMES_IN_FLIGHT * 2;
}

static void
vkpt_show_pvs(void)
{
	if (!vkpt_refdef.fd)
		return;

	if (vkpt_refdef.fd->feedback.lookatcluster < 0)
	{
		memset(cluster_debug_mask, 0, sizeof(cluster_debug_mask));
		cluster_debug_index = -1;
		return;
	}

	BSP_ClusterVis(bsp_world_model, cluster_debug_mask, vkpt_refdef.fd->feedback.lookatcluster, DVIS_PVS);
	cluster_debug_index = vkpt_refdef.fd->feedback.lookatcluster;
}

static float halton(int base, int index) {
	float f = 1.f;
	float r = 0.f;
	int i = index;

	while (i > 0)
	{
		f = f / base;
		r = r + f * (i % base);
		i = i / base;
	}
	return r;
};

// Autocompletion support for ray_tracing_api cvar
static void ray_tracing_api_g(genctx_t *ctx)
{
	Prompt_AddMatch(ctx, "auto");
	Prompt_AddMatch(ctx, "query");
	Prompt_AddMatch(ctx, "pipeline");
}
static void vr_require_initialisation_result(const char *stage,
	VkResult result)
{
	if (result == VK_SUCCESS)
		return;
	if (result == VK_ERROR_DEVICE_LOST)
	{
		vr_device_lost_terminal = true;
		vr_abandon_children = true;
	}
	else if (vr_has_submitted)
		vr_abandon_children = true;
	Com_Error(ERR_FATAL, "VR_FAIL: renderer initialisation stage %s failed (%s)",
		stage, qvk_result_to_string(result));
}

/* called when the library is loaded */
ref_type_t
R_Init_RTX(bool total)
{
	registration_sequence = 1;

#ifdef REFERENCE_VR_HISTORY
	temporal_frame_valid = false;
#endif

	if (!vid.init(GAPI_VULKAN)) {
		Com_Error(ERR_FATAL, "VID_Init failed\n");
		return REF_TYPE_NONE;
	}

    extern SDL_Window *get_sdl_window(void);
    qvk.window = get_sdl_window();

	cvar_profiler = Cvar_Get("profiler", "0", 0);
	cvar_profiler_samples = Cvar_Get("profiler_samples", "60", CVAR_ARCHIVE);
	cvar_profiler_scale = Cvar_Get("profiler_scale", "1", CVAR_ARCHIVE);
	cvar_vsync = Cvar_Get("vid_vsync", "0", CVAR_ARCHIVE);
	cvar_vsync->changed = NULL; // in case the GL renderer has set it
	cvar_hdr = Cvar_Get("vid_hdr", "0", CVAR_ARCHIVE);
	cvar_pt_caustics = Cvar_Get("pt_caustics", "1", CVAR_ARCHIVE);
	cvar_pt_enable_nodraw = Cvar_Get("pt_enable_nodraw", "0", 0);
	/* Synthesize materials for surfaces with LIGHT flag.
	 * 0: disabled
	 * 1: enabled for "custom" materials (not in materials.csv)
	 * 2: enabled for all materials w/o an emissive texture */
	cvar_pt_enable_surface_lights = Cvar_Get("pt_enable_surface_lights", "1", CVAR_FILES);
	/* LIGHT flag synthesis for "warp" surfaces (water, slime),
	 * separately controlled for aesthetic reasons
	 * 0: disabled
	 * 1: hack up a material that emits light but doesn't render with an emissive texture
	 * 2: "full" synthesis (incl emissive texture) */
	cvar_pt_enable_surface_lights_warp = Cvar_Get("pt_enable_surface_lights_warp", "0", CVAR_FILES);
	/* How to choose emissive texture for LIGHT flag synthesis:
	 * 0: Just use diffuse texture
	 * 1: Use (diffuse) pixels above a certain relative brightness for emissive texture */
	cvar_pt_surface_lights_fake_emissive_algo = Cvar_Get("pt_surface_lights_fake_emissive_algo", "1", CVAR_FILES);

	// Threshold for pixel values used when constructing a fake emissive image.
	cvar_pt_surface_lights_threshold = Cvar_Get("pt_surface_lights_threshold", "215", CVAR_FILES);

	// Multiplier for texinfo radiance field to convert radiance to emissive factors
	cvar_pt_bsp_radiance_scale = Cvar_Get("pt_bsp_radiance_scale", "0.001", CVAR_FILES);

	// Controls which sky surfaces become poly-lights.
	// 0 -> only the SKY surfaces in clusters listed in sky_clusters.txt
	// 1 -> also surfaces with both SKY and LIGHT flags set
	// 2 -> also surfaces with SKY, LIGHT, and NODRAW flags set become invisible portal lights
	// Nonzero settings should only be used for custom maps where sky surfaces are marked properly for Q2RTX.
	cvar_pt_bsp_sky_lights = Cvar_Get("pt_bsp_sky_lights", "0", 0);

	// 0 -> disabled, regular pause; 1 -> enabled; 2 -> enabled, hide GUI
	cvar_pt_accumulation_rendering = Cvar_Get("pt_accumulation_rendering", "1", CVAR_ARCHIVE);

	// number of frames to accumulate with linear weights in accumulation rendering modes
	cvar_pt_accumulation_rendering_framenum = Cvar_Get("pt_accumulation_rendering_framenum", "500", 0);

	// 0 -> perspective, 1 -> cylindrical
	cvar_pt_projection = Cvar_Get("pt_projection", "0", CVAR_ARCHIVE);

	// depth of field control:
	// 0 -> disabled
	// 1 -> enabled only in the reference mode
	// 2 -> enabled in the reference and no-denoiser modes
	// 3 -> always enabled (where are my glasses?)
	cvar_pt_dof = Cvar_Get("pt_dof", "1", CVAR_ARCHIVE);

	// freecam mode toggle
	cvar_pt_freecam = Cvar_Get("pt_freecam", "1", CVAR_ARCHIVE);

	// texture filtering mode for non-UI elements:
	// 0 -> linear magnification, anisotropic minification
	// 1 -> nearest magnification, anisotropic minification
	// 2 -> nearest magnification and minification, no mipmaps (noisy)
	cvar_pt_nearest = Cvar_Get("pt_nearest", "0", CVAR_ARCHIVE);
	cvar_pt_nearest->changed = pt_nearest_changed;

	// texture filtering mode for UI elements, follows
	// the gl_bilerp_ cvars, except for `cvar_pt_bilerp_pics` which
	// is only on/off, since vk has no scrap
	cvar_pt_bilerp_chars = Cvar_Get("pt_bilerp_chars", "0", CVAR_ARCHIVE);
	cvar_pt_bilerp_pics = Cvar_Get("pt_bilerp_pics", "0", CVAR_ARCHIVE);
	cvar_pt_bilerp_chars->changed = cvar_pt_bilerp_pics->changed = pt_nearest_changed;

	// waterwarp effect
	cvar_pt_waterwarp = Cvar_Get("pt_waterwarp", "0", CVAR_ARCHIVE);

#ifdef VKPT_DEVICE_GROUPS
	cvar_sli = Cvar_Get("sli", "1", CVAR_REFRESH | CVAR_ARCHIVE);
#endif

#ifdef VKPT_IMAGE_DUMPS
	cvar_dump_image = Cvar_Get("dump_image", "0", 0);
#endif

	scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);
	scr_viewsize->changed = viewsize_changed;

	// enables or disables full screen blending effects
	cvar_tm_blend_enable = Cvar_Get("tm_blend_enable", "1", CVAR_ARCHIVE);

	drs_init();
	vkpt_fsr_init_cvars();

	// Minimum NVIDIA driver version - this is a cvar in case something changes in the future,
	// and the current test no longer works.
	cvar_min_driver_version_nvidia = Cvar_Get("min_driver_version_nvidia", "460.82", 0);

	// Minimum AMD driver version
	cvar_min_driver_version_amd = Cvar_Get("min_driver_version_amd", "21.1.1", 0);

	// Selects which RT API to use:
	//  auto     - automatic selection based on the GPU
	//  query    - prefer KHR_ray_query
	//  pipeline - prefer KHR_ray_tracing_pipeline
	cvar_ray_tracing_api = Cvar_Get("ray_tracing_api", "auto", CVAR_REFRESH | CVAR_ARCHIVE);
	cvar_ray_tracing_api->generator = &ray_tracing_api_g;

	// When nonzero, the Vulkan validation layer is requested
	cvar_vk_validation = Cvar_Get("vk_validation", "0", CVAR_REFRESH | CVAR_ARCHIVE);

#if USE_DEBUG
	// Testing: force a colored shell on all entities
	cvar_pt_test_shell = Cvar_Get("pt_test_shell", "0", CVAR_CHEAT);
#endif

	InitialiseSkyCVars();

	MAT_Init();

#define UBO_CVAR_DO(name, default_value) cvar_##name = Cvar_Get(#name, #default_value, 0);
	UBO_CVAR_LIST
#undef UBO_CVAR_LIST

#ifdef REFERENCE_B_B2
	cvar_b2_hack_mode = Cvar_Get("b2_hack_mode", "0", 0);
#endif

	cvar_flt_temporal_hf->changed = temporal_cvar_changed;
	cvar_flt_temporal_lf->changed = temporal_cvar_changed;
	cvar_flt_temporal_spec->changed = temporal_cvar_changed;
	cvar_flt_enable->changed = temporal_cvar_changed;

	cvar_pt_dof->changed = accumulation_cvar_changed;
	cvar_pt_aperture->changed = accumulation_cvar_changed;
	cvar_pt_aperture_type->changed = accumulation_cvar_changed;
	cvar_pt_aperture_angle->changed = accumulation_cvar_changed;
	cvar_pt_focus->changed = accumulation_cvar_changed;
	cvar_pt_freecam->changed = accumulation_cvar_changed;
	cvar_pt_projection->changed = accumulation_cvar_changed;

	cvar_pt_num_bounce_rays->flags |= CVAR_ARCHIVE;

	cvar_ui_hdr_nits = Cvar_Get("ui_hdr_nits", "300", 0);

#ifdef REFERENCE_B_B2
	if (b2_desktop_enabled()) {
		int width, height;
		SDL_Vulkan_GetDrawableSize(qvk.window, &width, &height);
		b2_note_desktop_drawable(width, height);
		{ if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2DESKTOP drawable=%dx%d logical_eye=%ux%u fixed_resources=1\n", width, height, B2G_Width(), B2G_Height()); }
	} else
#endif
	{
		qvk.draw_width = r_config.width;
		qvk.draw_height = r_config.height;
	}

	IMG_Init();
	IMG_GetPalette();
	MOD_Init();
	
	if(!init_vulkan()) {
#ifdef REFERENCE_B_B2
        Com_Error(ERR_FATAL,"%s",B2XR_StartupMessage555("Vulkan graphics could not initialise. Check your GPU and graphics driver, then relaunch."));
#else
        Com_Error(ERR_FATAL,"Vulkan graphics could not initialise.");
#endif
		return REF_TYPE_NONE;
	}

#ifdef REFERENCE_B_B2
	if (b2_desktop_enabled()) {
		VkPhysicalDeviceProperties limits;
		vkGetPhysicalDeviceProperties(qvk.physical_device, &limits);
		if (B2G_Width() > limits.limits.maxComputeWorkGroupCount[0] ||
			B2G_Height() > limits.limits.maxComputeWorkGroupCount[1])
			Com_Error(ERR_FATAL, "B2_FAIL: selected render size exceeds dispatch bounds");
		if (!qvk.use_ray_query) {
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
			VkPhysicalDeviceProperties2 properties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &ray };
			vkGetPhysicalDeviceProperties2(qvk.physical_device, &properties);
			if ((uint64_t)B2G_Width()*B2G_Height() > ray.maxRayDispatchInvocationCount)
				Com_Error(ERR_FATAL, "B2_FAIL: selected render size exceeds ray dispatch bounds");
		}
		r_config.width=B2G_Width(); r_config.height=B2G_Height();
	}
#endif
	vr_require_initialisation_result("command-pool-and-fences",
		create_command_pool_and_fences());
	vr_require_initialisation_result("swapchain",
		create_swapchain());
	vr_require_initialisation_result("shader-modules",
		vkpt_load_shader_modules());
	vr_require_initialisation_result("default-resources",
		vkpt_initialize_all(VKPT_INIT_DEFAULT));
	vr_require_initialisation_result("reload-shader-resources",
		vkpt_initialize_all(VKPT_INIT_RELOAD_SHADER));
	vr_require_initialisation_result("swapchain-resources",
		vkpt_initialize_all(VKPT_INIT_SWAPCHAIN_RECREATE));

	Cmd_AddCommand("reload_shader", (xcommand_t)&vkpt_reload_shader);
	Cmd_AddCommand("reload_textures", (xcommand_t)&vkpt_reload_textures);
	Cmd_AddCommand("show_pvs", (xcommand_t)&vkpt_show_pvs);
	Cmd_AddCommand("next_sun", (xcommand_t)&vkpt_next_sun_preset);

	vkpt_fog_init();
	vkpt_cameras_init();

	for (int i = 0; i < 256; i++) {
		qvk.sintab[i] = sinf(i * (2 * M_PI / 255));
	}

	for (int i = 0; i < NUM_TAA_SAMPLES; i++)
	{
		taa_samples[i][0] = halton(2, i + 1) - 0.5f;
		taa_samples[i][1] = halton(3, i + 1) - 0.5f;
	}

    B2D_Record(0,"RENDERER_READY internal=%ux%u; world history allocates on first world frame",B2G_Width(),B2G_Height());
#ifdef REFERENCE_B_B2
    if (B2XR_Requested()) b2_accept_callback_settings580();
	Cvar_B2GraphicsActive534(B2XR_Requested());
	b2_desktop_dirty534 = false;
#endif
	return REF_TYPE_VKPT;
}

/* called before the library is unloaded */
void
R_Shutdown_RTX(bool total)
{
#ifdef REFERENCE_B_B2
	Cvar_B2GraphicsActive534(false);
    b2_settings_changed580 = b2_settings_rejected580 = false;
    b2_physical_started580 = b2_settings_history_reset580 = false;
    b2_desktop_failed582 = false;
#endif

	bool abandon_device_children = false;
	vkpt_freecam_reset();
	

	abandon_device_children = vr_abandon_children;
	if (!abandon_device_children)
	{
		
		VkResult idle_result = vkDeviceWaitIdle(qvk.device);
		
		if (idle_result != VK_SUCCESS)
		{
			if (idle_result == VK_ERROR_DEVICE_LOST)
				vr_device_lost_terminal = true;
			vr_abandon_children = true;
			abandon_device_children = true;
			Com_EPrintf("VR: terminal renderer-shutdown idle result %s; child resources abandoned\n",
				qvk_result_to_string(idle_result));
		}
	}

#ifdef REFERENCE_B_B2
	/* Renderer execution is over. The idle/abandon decision above owns cleanup;
	 * retire XR before permitting IMG_FreeAll's destruction-only invalidations. */
	B2XR_Shutdown();
	qvk.b2_temporal_activated = false;
    memset(&b2_world_resources, 0, sizeof(b2_world_resources));
    b2_player_resources492=false;
#endif

	// Persist current DRS scale
	if (drs_current_scale != 0)
		Cvar_SetInteger(cvar_drs_last_scale, drs_current_scale, FROM_CODE);

	Cmd_RemoveCommand("reload_shader");
	Cmd_RemoveCommand("reload_textures");
	Cmd_RemoveCommand("show_pvs");
	Cmd_RemoveCommand("next_sun");

	if (!abandon_device_children && vkpt_refdef.bsp_mesh_world_loaded)
	{
		vkpt_vertex_buffer_cleanup_bsp_mesh(&vkpt_refdef.bsp_mesh_world);
		bsp_mesh_destroy(&vkpt_refdef.bsp_mesh_world);
	}

	if (!abandon_device_children)
	{
		VkResult destroy_result;
		vkpt_fog_shutdown();
		vkpt_cameras_shutdown();
		MAT_Shutdown();
		
		IMG_FreeAll();
		
		vkpt_textures_destroy_unused();

		destroy_result = vkpt_destroy_all(VKPT_INIT_DEFAULT);
		if (destroy_result != VK_SUCCESS)
		{
			if (destroy_result == VK_ERROR_DEVICE_LOST)
				vr_device_lost_terminal = true;
			vr_abandon_children = true;
			abandon_device_children = true;
			Com_EPrintf("VR: terminal renderer-destroy result %s; remaining child resources abandoned\n",
				qvk_result_to_string(destroy_result));
		}
		else
			vkpt_destroy_shader_modules();
	}
	else
	{
		Com_EPrintf("VR: terminal device state; child Vulkan resources left to device teardown\n");
	}

	
	if(destroy_vulkan()) {
		Com_EPrintf("destroy_vulkan failed\n");
	}
	

	
	IMG_Shutdown();
	
	MOD_Shutdown(); // todo: currently leaks memory, need to clear submeshes
	vid.shutdown();
	
}

// for screenshots
void
IMG_ReadPixels_RTX(screenshot_t *s)
{
#ifdef REFERENCE_B_B2
    if (b2_desktop_enabled() && (b2_mirror.no_image534 || !qvk.swap_chain)) {
        Com_EPrintf("Desktop screenshot unavailable while mirror presentation is suspended.\n");
        return;
    }
#endif
	VkExtent2D desktop_extent = vkpt_get_present_extent();
	if (qvk.surf_format.format != VK_FORMAT_B8G8R8A8_SRGB &&
		qvk.surf_format.format != VK_FORMAT_R8G8B8A8_SRGB)
	{
		Com_EPrintf("IMG_ReadPixels: unsupported swap chain format (%d)!\n", qvk.surf_format.format);
		return;
	}

	VkCommandBuffer cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);

	VkImage swap_chain_image = qvk.swap_chain_images[qvk.current_swap_chain_image_index];

	VkImageSubresourceRange subresource_range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};
		
	IMAGE_BARRIER(cmd_buf,
		.image = swap_chain_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	);

	IMAGE_BARRIER_STAGES(cmd_buf,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		.image = qvk.screenshot_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_HOST_READ_BIT,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	VkImageCopy img_copy_region = {
		.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		.extent = { desktop_extent.width, desktop_extent.height, 1 }
	};

	vkCmdCopyImage(cmd_buf,
		swap_chain_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		qvk.screenshot_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &img_copy_region);

	IMAGE_BARRIER(cmd_buf,
		.image = swap_chain_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	);

	IMAGE_BARRIER_STAGES(cmd_buf,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_HOST_BIT,
		.image = qvk.screenshot_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_HOST_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_GENERAL
	);

	vkpt_submit_command_buffer_simple(cmd_buf, qvk.queue_graphics, false);
	vkpt_wait_idle(qvk.queue_graphics, &qvk.cmd_buffers_graphics);

	VkImageSubresource subresource = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.arrayLayer = 0,
		.mipLevel = 0
	};

	VkSubresourceLayout subresource_layout;
	vkGetImageSubresourceLayout(qvk.device, qvk.screenshot_image, &subresource, &subresource_layout);

	void *device_data;
	_VK(vkMapMemory(qvk.device, qvk.screenshot_image_memory, 0, qvk.screenshot_image_memory_size, 0, &device_data));
	
	int pitch = desktop_extent.width * 3;
	s->pixels = FS_AllocTempMem(pitch * desktop_extent.height);

	for (int row = 0; row < desktop_extent.height; row++)
	{
		byte* src_row = (byte*)device_data + subresource_layout.rowPitch * row;
		byte* dst_row = s->pixels + pitch * (desktop_extent.height - row - 1);

		if (qvk.surf_format.format == VK_FORMAT_B8G8R8A8_SRGB)
		{
			for (int col = 0; col < desktop_extent.width; col++)
			{
				dst_row[0] = src_row[2];
				dst_row[1] = src_row[1];
				dst_row[2] = src_row[0];

				src_row += 4;
				dst_row += 3;
			}
		}
		else // must be VK_FORMAT_R8G8B8A8_SRGB then
		{
			for (int col = 0; col < desktop_extent.width; col++)
			{
				dst_row[0] = src_row[0];
				dst_row[1] = src_row[1];
				dst_row[2] = src_row[2];

				src_row += 4;
				dst_row += 3;
			}
		}
	}

	vkUnmapMemory(qvk.device, qvk.screenshot_image_memory);

	s->width = desktop_extent.width;
	s->height = desktop_extent.height;
	s->rowbytes = pitch;
}

void
IMG_ReadPixelsHDR_RTX(screenshot_t *s)
{
#ifdef REFERENCE_B_B2
    if (b2_desktop_enabled() && (b2_mirror.no_image534 || !qvk.swap_chain)) {
        Com_EPrintf("Desktop screenshot unavailable while mirror presentation is suspended.\n");
        return;
    }
#endif
	VkExtent2D desktop_extent = vkpt_get_present_extent();
	if (qvk.surf_format.format != VK_FORMAT_R16G16B16A16_SFLOAT)
	{
		Com_EPrintf("IMG_ReadPixelsHDR: unsupported swap chain format (%d)!\n", qvk.surf_format.format);
		return;
	}

	VkCommandBuffer cmd_buf = vkpt_begin_command_buffer(&qvk.cmd_buffers_graphics);

	VkImage swap_chain_image = qvk.swap_chain_images[qvk.current_swap_chain_image_index];

	VkImageSubresourceRange subresource_range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};
		
	IMAGE_BARRIER(cmd_buf,
		.image = swap_chain_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	);

	IMAGE_BARRIER(cmd_buf,
		.image = qvk.screenshot_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_HOST_READ_BIT,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	VkImageCopy img_copy_region = {
		.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		.extent = { desktop_extent.width, desktop_extent.height, 1 }
	};

	vkCmdCopyImage(cmd_buf,
		swap_chain_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		qvk.screenshot_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &img_copy_region);

	IMAGE_BARRIER(cmd_buf,
		.image = swap_chain_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	);

	IMAGE_BARRIER(cmd_buf,
		.image = qvk.screenshot_image,
		.subresourceRange = subresource_range,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_HOST_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_GENERAL
	);

	vkpt_submit_command_buffer_simple(cmd_buf, qvk.queue_graphics, false);
	vkpt_wait_idle(qvk.queue_graphics, &qvk.cmd_buffers_graphics);

	VkImageSubresource subresource = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.arrayLayer = 0,
		.mipLevel = 0
	};

	VkSubresourceLayout subresource_layout;
	vkGetImageSubresourceLayout(qvk.device, qvk.screenshot_image, &subresource, &subresource_layout);

	void *device_data;
	_VK(vkMapMemory(qvk.device, qvk.screenshot_image_memory, 0, qvk.screenshot_image_memory_size, 0, &device_data));
	
	int pitch = desktop_extent.width * 3;
	s->pixels = FS_AllocTempMem(pitch * desktop_extent.height * sizeof(float));

	for (int row = 0; row < desktop_extent.height; row++)
	{
		uint16_t* src_row = (uint16_t*)((byte*)device_data + subresource_layout.rowPitch * row);
		float* dst_row = (float*)s->pixels + pitch * (desktop_extent.height - row - 1);

		for (int col = 0; col < desktop_extent.width; col++)
		{
			dst_row[0] = halfToFloat(src_row[0]);
			dst_row[1] = halfToFloat(src_row[1]);
			dst_row[2] = halfToFloat(src_row[2]);

			src_row += 4;
			dst_row += 3;
		}
	}

	vkUnmapMemory(qvk.device, qvk.screenshot_image_memory);

	s->width = desktop_extent.width;
	s->height = desktop_extent.height;
	s->rowbytes = pitch * sizeof(float);
}

void
R_SetSky_RTX(const char *name, float rotate, int autorotate, const vec3_t axis)
{
	vkpt_vr_require_maintenance_open("sky descriptor regeneration");
	int     i;
	char    pathname[MAX_QPATH];
	// 3dstudio environment map names
	const char *suf[6] = { "ft", "bk", "up", "dn", "rt", "lf" };

	byte *data = NULL;

	sky_rotation = rotate;
	sky_autorotate = autorotate;
	VectorNormalize2(axis, sky_axis);

	int avg_color[3] = { 0 };
	int w_prev, h_prev;
	for (i = 0; i < 6; i++) {
		Q_concat(pathname, sizeof(pathname), "env/", name, suf[i], ".tga");
		FS_NormalizePath(pathname);
		image_t *img = IMG_Find(pathname, IT_SKY, IF_NONE);

		if(img == R_NOTEXTURE) {
			if(data) {
				Z_Free(data);
			}
			data = Z_Malloc(6 * sizeof(uint32_t));
			for(int j = 0; j < 6; j++)
				((uint32_t *)data)[j] = 0xff00ffffu;
			w_prev = h_prev = 1;
			break;
		}

		size_t s = img->upload_width * img->upload_height * 4;
		if(!data) {
			data = Z_Malloc(s * 6);
			w_prev = img->upload_width;
			h_prev = img->upload_height;
		}

		memcpy(data + s * i, img->pix_data, s);

		for (int p = 0; p < img->upload_width * img->upload_height; p++)
		{
			uint32_t pix = *((uint32_t*)img->pix_data + p);
			avg_color[0] += pix & 0xff;
			avg_color[1] += (pix >> 8) & 0xff;
			avg_color[2] += (pix >> 16) & 0xff;
		}

		assert(w_prev == img->upload_width);
		assert(h_prev == img->upload_height);

		List_Remove(&img->entry);

		IMG_Unload(img);

		memset(img, 0, sizeof(*img));
	}

	float inv_num_pixels = 1.0f / (w_prev * h_prev * 6);

	VectorSet(avg_envmap_color,
		(float)avg_color[0] * inv_num_pixels / 255.f,
		(float)avg_color[1] * inv_num_pixels / 255.f,
		(float)avg_color[2] * inv_num_pixels / 255.f
	);

	B2_WORLD_VK(vkpt_textures_upload_envmap(w_prev, h_prev, data));
	Z_Free(data);
}

void R_AddDecal_RTX(decal_t *d)
{ }

void
R_BeginRegistration_RTX(const char *name)
{
#ifdef REFERENCE_B_B2
    b2_world_registration_begin();
#endif
	vkpt_vr_require_maintenance_open("begin registration");
#ifdef REFERENCE_VR_HISTORY
	temporal_frame_valid = false;
#endif
	registration_sequence++;
	LOG_FUNC();
	Com_Printf("loading %s\n", name);
	{
		VkResult idle_result = vkDeviceWaitIdle(qvk.device);
		if (idle_result != VK_SUCCESS)
		{
			if (idle_result == VK_ERROR_DEVICE_LOST)
				vr_device_lost_terminal = true;
			vr_abandon_children = true;
			Com_Error(ERR_FATAL,
				"VR_FAIL: map-registration idle boundary failed (%s)",
				qvk_result_to_string(idle_result));
		}
	}

	vkpt_fog_reset();

	Com_AddConfigFile("maps/default.cfg", 0);
	Com_AddConfigFile(va("maps/%s.cfg", name), 0);

	if(vkpt_refdef.bsp_mesh_world_loaded) {
		vkpt_vertex_buffer_cleanup_bsp_mesh(&vkpt_refdef.bsp_mesh_world);
		bsp_mesh_destroy(&vkpt_refdef.bsp_mesh_world);
		vkpt_refdef.bsp_mesh_world_loaded = 0;
	}

	if(bsp_world_model) {
		BSP_Free(bsp_world_model);
		bsp_world_model = NULL;
	}

	char bsp_path[MAX_QPATH];
	Q_concat(bsp_path, sizeof(bsp_path), "maps/", name, ".bsp");
	bsp_t *bsp;
	int ret = BSP_Load(bsp_path, &bsp);
	if(!bsp) {
#ifdef REFERENCE_B_B2
        if (qvk.b2_temporal_activated) b2_world_fail("map load");
#endif
		Com_Error(ERR_DROP, "%s: couldn't load %s: %s", __func__, bsp_path, Q_ErrorString(ret));
	}
	if (!bsp->vis) {
#ifdef REFERENCE_B_B2
        if (qvk.b2_temporal_activated) b2_world_fail("map visibility");
#endif
		Hunk_Free(&bsp->hunk);
		Z_Free(bsp);
		Com_Error(ERR_DROP, "BSP not vis'd; this is required for Q2RTX.");
	}
	bsp_world_model = bsp;
	bsp_mesh_register_textures(bsp);
	bsp_mesh_create_from_bsp(&vkpt_refdef.bsp_mesh_world, bsp, name);
	{
		VkResult create_result = vkpt_light_buffers_create(
			&vkpt_refdef.bsp_mesh_world);
		VkResult upload_result = create_result == VK_SUCCESS ?
			vkpt_vertex_buffer_upload_bsp_mesh(
				&vkpt_refdef.bsp_mesh_world) : VK_NOT_READY;
		if (create_result != VK_SUCCESS || upload_result != VK_SUCCESS)
		{
			if (create_result == VK_ERROR_DEVICE_LOST ||
				upload_result == VK_ERROR_DEVICE_LOST)
			{
				vr_device_lost_terminal = true;
				vr_abandon_children = true;
			}
			Com_Error(ERR_FATAL,
				"VR_FAIL: map producers failed before publication (light=%s world=%s)",
				qvk_result_to_string(create_result),
				qvk_result_to_string(upload_result));
		}
	}
	vkpt_refdef.bsp_mesh_world_loaded = 1;
	bsp = NULL;
	world_anim_frame = 0;

	Cvar_Set("sv_novis", vkpt_refdef.bsp_mesh_world.num_cameras > 0 ? "1" : "0");

	// register physical sky attributes based on map name lookup
	vkpt_physical_sky_beginRegistration();
	UpdatePhysicalSkyCVars();

	vkpt_physical_sky_latch_local_time();
	vkpt_bloom_reset();
	vkpt_tone_mapping_request_reset();
	vkpt_light_buffer_reset_counts();

	memset(cluster_debug_mask, 0, sizeof(cluster_debug_mask));
	cluster_debug_index = -1;

	drs_last_frame_world = false;
}

void
R_EndRegistration_RTX(void)
{
	vkpt_vr_require_maintenance_open("end registration");
	LOG_FUNC();
	
	B2_WORLD_VK(vkpt_physical_sky_endRegistration());

	IMG_FreeUnused();
	MOD_FreeUnused();
	MAT_FreeUnused();
#ifdef REFERENCE_B_B2
    b2_world_registration_end();
#endif
}

VkCommandBuffer vkpt_begin_command_buffer(cmd_buf_group_t* group)
{
	
	if (group->used_this_frame == group->count_per_frame)
	{
		uint32_t new_count = max(4, group->count_per_frame * 2);
		VkCommandBuffer* new_buffers = Z_Mallocz(new_count * MAX_FRAMES_IN_FLIGHT * sizeof(VkCommandBuffer));
		uint32_t allocated_frames = 0;

		for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
		{
			if (group->count_per_frame > 0)
			{
				memcpy(new_buffers + new_count * frame, group->buffers + group->count_per_frame * frame, group->count_per_frame * sizeof(VkCommandBuffer));
			}

			VkCommandBufferAllocateInfo cmd_buf_alloc_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = group->command_pool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = new_count - group->count_per_frame
			};

			VkResult allocation_result = vkAllocateCommandBuffers(qvk.device,
				&cmd_buf_alloc_info,
				new_buffers + new_count * frame + group->count_per_frame);
			if (allocation_result != VK_SUCCESS)
			{
				
				if (allocation_result == VK_ERROR_DEVICE_LOST)
				{
					vr_device_lost_terminal = true;
					vr_abandon_children = true;
				}
				else
				{
					for (uint32_t allocated = 0;
						allocated < allocated_frames; allocated++)
						vkFreeCommandBuffers(qvk.device, group->command_pool,
							new_count - group->count_per_frame,
							new_buffers + new_count * allocated +
								group->count_per_frame);
					Z_Free(new_buffers);
				}
				Com_Error(ERR_FATAL,
					"VR_FAIL: technical command-buffer allocation failed (%s)",
					qvk_result_to_string(allocation_result));
			}
			if (allocation_result == VK_SUCCESS)
				allocated_frames++;
			_VK(allocation_result);
		}

#ifdef USE_DEBUG
		void** new_addrs = Z_Mallocz(new_count * MAX_FRAMES_IN_FLIGHT * sizeof(void*));

		if (group->count_per_frame > 0)
		{
			for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
			{
				memcpy(new_addrs + new_count * frame, group->buffer_begin_addrs + group->count_per_frame * frame, group->count_per_frame * sizeof(void*));
			}
		}

		Z_Free(group->buffer_begin_addrs);
		group->buffer_begin_addrs = new_addrs;
#endif

		Z_Free(group->buffers);
		group->buffers = new_buffers;
		group->count_per_frame = new_count;
	}

	VkCommandBuffer cmd_buf = group->buffers[group->count_per_frame * qvk.current_frame_index + group->used_this_frame];

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL,
	};
	VkResult reset_result = vkResetCommandBuffer(cmd_buf, 0);
	VkResult begin_result = reset_result == VK_SUCCESS ?
		vkBeginCommandBuffer(cmd_buf, &begin_info) : VK_NOT_READY;
	
	if (reset_result != VK_SUCCESS || begin_result != VK_SUCCESS)
	{
		
		if (reset_result == VK_ERROR_DEVICE_LOST ||
			begin_result == VK_ERROR_DEVICE_LOST)
		{
			vr_device_lost_terminal = true;
			vr_abandon_children = true;
		}
		else if (vr_has_submitted)
			vr_abandon_children = true;
#ifdef REFERENCE_B_B2
		if (B2XR_Requested())
		{
			B2XR_RequestStop();
			Com_Error(ERR_FATAL,
				"B2_FAIL: Vulkan command reset/begin failed (reset=%s begin=%s)",
				qvk_result_to_string(reset_result),
				qvk_result_to_string(begin_result));
		}
#endif
		Com_Error(ERR_FATAL,
			"VR_FAIL: technical command reset/begin failed (reset=%s begin=%s)",
			qvk_result_to_string(reset_result),
			qvk_result_to_string(begin_result));
	}
	
	
	


#ifdef USE_DEBUG
	void** begin_addr = group->buffer_begin_addrs + group->count_per_frame * qvk.current_frame_index + group->used_this_frame;

#if (defined __GNUC__)
	*begin_addr = __builtin_return_address(0);
#elif (defined _MSC_VER)
	*begin_addr = _ReturnAddress();
#else
	*begin_addr = NULL;
#endif
#endif

	group->used_this_frame += 1;

	return cmd_buf;
}

void vkpt_free_command_buffers(cmd_buf_group_t* group)
{
	if (group->count_per_frame == 0)
		return;

	vkFreeCommandBuffers(qvk.device, group->command_pool, group->count_per_frame * MAX_FRAMES_IN_FLIGHT, group->buffers);

	Z_Freep((void**)&group->buffers);

#ifdef USE_DEBUG
	Z_Freep((void**)&group->buffer_begin_addrs);
#endif

	group->count_per_frame = 0;
	group->used_this_frame = 0;
}

void vkpt_reset_command_buffers(cmd_buf_group_t* group)
{
	group->used_this_frame = 0;

#if 0 // defined(USE_DEBUG)
	for (int i = 0; i < group->count_per_frame; i++)
	{
		void* addr = group->buffer_begin_addrs[group->count_per_frame * qvk.current_frame_index + i];
		//seth: this seems unrelated to the raytracing changes, but skip it until raytracing is working
		//assert(addr == 0);
	}
#endif
}

void vkpt_wait_idle(VkQueue queue, cmd_buf_group_t* group)
{
	VkResult wait_result = vkQueueWaitIdle(queue);
	if (wait_result != VK_SUCCESS)
	{
		if (wait_result == VK_ERROR_DEVICE_LOST)
			vr_device_lost_terminal = true;
		vr_abandon_children = true;
		Com_Error(ERR_FATAL, "VR_FAIL: technical queue-idle wait failed (%s)",
			qvk_result_to_string(wait_result));
	}
	vkpt_reset_command_buffers(group);
}

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
	VkFence fence)
{
	
	VkResult end_result = vkEndCommandBuffer(cmd_buf);
	VkResult submit_result = VK_NOT_READY;

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = wait_semaphore_count,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.signalSemaphoreCount = signal_semaphore_count,
		.pSignalSemaphores = signal_semaphores,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd_buf,
	};

#ifdef VKPT_DEVICE_GROUPS
	VkDeviceGroupSubmitInfo device_group_submit_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = wait_semaphore_count,
		.pWaitSemaphoreDeviceIndices = wait_device_indices,
		.commandBufferCount = 1,
		.pCommandBufferDeviceMasks = &execute_device_mask,
		.signalSemaphoreCount = signal_semaphore_count,
		.pSignalSemaphoreDeviceIndices = signal_device_indices,
	};

	if (qvk.device_count > 1) {
		submit_info.pNext = &device_group_submit_info;
	}
#endif

	if (end_result == VK_SUCCESS)
	{
		
		submit_result = vkQueueSubmit(queue, 1, &submit_info, fence);
		if (submit_result == VK_SUCCESS) vr_has_submitted = true;
	}
	{
		
		
		
		if (end_result != VK_SUCCESS || submit_result != VK_SUCCESS)
		{
			
			if (end_result == VK_ERROR_DEVICE_LOST ||
				submit_result == VK_ERROR_DEVICE_LOST)
			{
				vr_device_lost_terminal = true;
				vr_abandon_children = true;
			}
			else if (vr_has_submitted)
				vr_abandon_children = true;
#ifdef REFERENCE_B_B2
			if (B2XR_Requested())
			{
				B2XR_RequestStop();
				Com_Error(ERR_FATAL,
					"B2_FAIL: Vulkan command end/submit failed (end=%s submit=%s)",
					qvk_result_to_string(end_result),
					qvk_result_to_string(submit_result));
			}
#endif
			Com_Error(ERR_FATAL,
				"VR_FAIL: unscoped technical command submission failed (end=%s submit=%s)",
				qvk_result_to_string(end_result),
				qvk_result_to_string(submit_result));
		}
		
		_VK(end_result);
		if (end_result == VK_SUCCESS)
			_VK(submit_result);
	}
	

#ifdef USE_DEBUG
	cmd_buf_group_t* groups[] = { &qvk.cmd_buffers_graphics, &qvk.cmd_buffers_transfer };
	for (int ngroup = 0; ngroup < LENGTH(groups); ngroup++)
	{
		cmd_buf_group_t* group = groups[ngroup];
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT * group->count_per_frame; i++)
		{
			if (group->buffers[i] == cmd_buf)
			{
				group->buffer_begin_addrs[i] = NULL;
				return;
			}
		}
	}
#endif
}

void vkpt_submit_command_buffer_simple(
	VkCommandBuffer cmd_buf,
	VkQueue queue,
	bool all_gpus)
{
	vkpt_submit_command_buffer(cmd_buf, queue, all_gpus ? (1 << qvk.device_count) - 1 : 1, 0, NULL, NULL, NULL, 0, NULL, NULL, 0);
}

#if _WIN32
	#include <windows.h>
#else
	#include <stdio.h>
#endif

void debug_output(const char* format, ...)
{
	char buffer[2048];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

#if _WIN32
	OutputDebugStringA(buffer);
#else
	fprintf(stderr, "%s", buffer);
#endif
}

static bool R_IsHDR_RTX(void)
{
	return qvk.surf_is_hdr;
}

void R_RegisterFunctionsRTX()
{
	R_Init = R_Init_RTX;
	R_Shutdown = R_Shutdown_RTX;
	R_BeginRegistration = R_BeginRegistration_RTX;
	R_EndRegistration = R_EndRegistration_RTX;
	R_SetSky = R_SetSky_RTX;
	R_RenderFrame = R_RenderFrame_RTX;
	R_LightPoint = R_LightPoint_RTX;
	R_ClearColor = R_ClearColor_RTX;
	R_SetAlpha = R_SetAlpha_RTX;
	R_SetAlphaScale = R_SetAlphaScale_RTX;
	R_SetColor = R_SetColor_RTX;
	R_SetClipRect = R_SetClipRect_RTX;
	R_SetScale = R_SetScale_RTX;
	R_DrawChar = R_DrawChar_RTX;
	R_DrawString = R_DrawString_RTX;
	R_DrawPic = R_DrawPic_RTX;
	R_DrawStretchPic = R_DrawStretchPic_RTX;
	R_DrawStretchRaw = R_DrawStretchRaw_RTX;
	R_UpdateRawPic = R_UpdateRawPic_RTX;
	R_DiscardRawPic = R_DiscardRawPic_RTX;
	R_SupportsDebugLines = vkpt_debugdraw_supported;
	R_AddDebugText_ = vkpt_debugdraw_addtext;
	R_DrawKeepAspectPic = R_DrawKeepAspectPic_RTX;
	R_TileClear = R_TileClear_RTX;
	R_DrawFill8 = R_DrawFill8_RTX;
	R_DrawFill32 = R_DrawFill32_RTX;
	R_BeginFrame = R_BeginFrame_RTX;
	R_EndFrame = R_EndFrame_RTX;
	R_ModeChanged = R_ModeChanged_RTX;
	R_AddDecal = R_AddDecal_RTX;
	R_InterceptKey = R_InterceptKey_RTX;
	R_IsHDR = R_IsHDR_RTX;
	IMG_Load = IMG_Load_RTX;
	IMG_Unload = IMG_Unload_RTX;
	IMG_ReadPixels = IMG_ReadPixels_RTX;
	IMG_ReadPixelsHDR = IMG_ReadPixelsHDR_RTX;
	MOD_LoadMD2 = MOD_LoadMD2_RTX;
	MOD_LoadMD3 = MOD_LoadMD3_RTX;
	MOD_LoadIQM = MOD_LoadIQM_RTX;
	MOD_Reference = MOD_Reference_RTX;
}

// vim: shiftwidth=4 noexpandtab tabstop=4 cindent
