// Modified 14 November 2026: selected Quake II RTX/Q2RTX VR production code
// adapted into a CPU fixture; copied paths and spans are listed in its manifest.
// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Retained CPU fixture: selected production spans are verified by rc7-regression.py.
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

#define VK_NO_PROTOTYPES
#define REFERENCE_B_B2
#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#define alloca _alloca
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#define ERR_FATAL 1
#define FROM_CODE 0
#define VKPT_MAX_GPUS 1
#define VKPT_IMG_TAA_OUTPUT 0
#define AA_MODE_UPSCALE 1
#define RDF_UNDERWATER 1
#define RDF_NOWORLDMODEL 2
#define LOG_FUNC()
#define IMAGE_BARRIER(...)
#define Com_Printf(...) ((void)0)
#define Com_EPrintf(...) ((void)0)
#define Com_WPrintf(...) ((void)0)
static jmp_buf trap;
#define Com_Error(...) longjmp(trap,1)
static struct {int integer;} cv[6];
#define cvar_hdr (&cv[0])
#define cvar_vsync (&cv[1])
#define cvar_profiler (&cv[2])
#define cvar_flt_enable (&cv[3])
#define cvar_tm_debug (&cv[4])
#define cvar_pt_waterwarp (&cv[5])
static struct {VkDevice device;VkPhysicalDevice physical_device;VkSurfaceKHR surface;VkQueue queue_graphics;VkSwapchainKHR swap_chain;VkImage *swap_chain_images;VkImageView *swap_chain_image_views;uint32_t num_swap_chain_images,current_swap_chain_image_index;VkSurfaceFormatKHR surf_format;VkPresentModeKHR present_mode;bool supports_colorspace,surf_is_hdr,surf_vsync,frame_menu_mode;VkExtent2D extent_taa_output,extent_unscaled,extent_render;VkImage images[1];int cmd_buffers_graphics,current_frame_index,device_count,effective_aa_mode;struct{VkSemaphore image_available,render_finished;}semaphores[1][1];VkFence fences_frame_sync[1];uint64_t frame_counter;void *window;} qvk;
typedef struct {VkSurfaceFormatKHR surface_fmt;VkFormat swapchain_view_fmt;}picked_surface_format_t;
static int num_accumulated_frames,drs_effective_scale;
typedef struct {uint32_t logical_index,view_count;} Frame;
typedef struct {bool b2_active,b2_render_accepted;uint32_t b2_eye,rdflags;Frame *b2_frame;} refdef_t;
static struct {refdef_t *fd;} vkpt_refdef;
static struct {bool held,wait_acquire,no_image534;VkSwapchainKHR swapchain;VkExtent2D extent;uint32_t image,logical_index;uint64_t left_frame;} b2_mirror;
static struct {bool pair_prepared;unsigned finished_mask;} b2_cadence;
static struct {bool open;unsigned finished;} b2_surface_pair;
static bool b2_physical_started580,b2_desktop_failed582,b2_desktop_dirty534,vr_device_lost_terminal,vr_abandon_children,b2_xr_pair_failed555,b2_settings_rejected580,b2_settings_history_reset580,temporal_frame_valid,frame_ready;
static VkExtent2D b2_desktop_drawable={100,100};
static int empty_submits,eye_submits,acquire_waits,acquire_signal,render_signal,fence_signal=1,finish_count,stop_count,present_count,view_live,chain_live,draw_live,idles,creates;
static int query_count;
static int fail_query,fail_formats,fail_modes,fail_images,fail_view,view_calls,fail_pick,drawable=100,copy_ok=1,draw_error;
static VkResult idle_result,submit_result,present_result;
static bool extents_equal(VkExtent2D a,VkExtent2D b){return a.width==b.width&&a.height==b.height;}
static bool b2_desktop_enabled(void){return true;}
static VkExtent2D vkpt_get_present_extent(void){return (VkExtent2D){100,100};}
static void vkpt_select_swapchain_extent(VkSurfaceCapabilitiesKHR *c){}
static bool pick_surface_format_sdr(picked_surface_format_t *p,VkSurfaceFormatKHR *f,unsigned n){p->surface_fmt=*f;p->swapchain_view_fmt=f->format;return !fail_pick;}
static bool pick_surface_format_hdr(picked_surface_format_t *p,VkSurfaceFormatKHR *f,unsigned n){return pick_surface_format_sdr(p,f,n);}
static void Cvar_SetByVar(void *c,const char *s,int source){}
static const char *qvk_result_to_string(VkResult r){return "fixture";}
static VkResult vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice d,VkSurfaceKHR s,VkSurfaceCapabilitiesKHR*c){query_count++;if(fail_query)return (VkResult)fail_query;*c=(VkSurfaceCapabilitiesKHR){.currentExtent={100,100},.minImageCount=2};return VK_SUCCESS;}
static VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice d,VkSurfaceKHR s,uint32_t*n,VkSurfaceFormatKHR*f){if(fail_formats)return VK_ERROR_SURFACE_LOST_KHR;*n=1;if(f)*f=(VkSurfaceFormatKHR){VK_FORMAT_R8G8B8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};return VK_SUCCESS;}
static VkResult vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice d,VkSurfaceKHR s,uint32_t*n,VkPresentModeKHR*p){if(fail_modes)return VK_ERROR_SURFACE_LOST_KHR;*n=1;if(p)*p=VK_PRESENT_MODE_FIFO_KHR;return VK_SUCCESS;}
static VkResult vkCreateSwapchainKHR(VkDevice d,const VkSwapchainCreateInfoKHR*i,const void*a,VkSwapchainKHR*s){assert(!chain_live);*s=(VkSwapchainKHR)(uintptr_t)1;chain_live=1;creates++;return VK_SUCCESS;}
static VkResult vkGetSwapchainImagesKHR(VkDevice d,VkSwapchainKHR s,uint32_t*n,VkImage*i){if(fail_images)return VK_ERROR_SURFACE_LOST_KHR;*n=3;if(i)for(int j=0;j<3;j++)i[j]=(VkImage)(uintptr_t)(j+1);return VK_SUCCESS;}
static VkResult vkCreateImageView(VkDevice d,const VkImageViewCreateInfo*i,const void*a,VkImageView*v){view_calls++;if(fail_view==view_calls){*v=(VkImageView)(uintptr_t)999;return VK_ERROR_OUT_OF_DEVICE_MEMORY;}*v=(VkImageView)(uintptr_t)view_calls;view_live++;return VK_SUCCESS;}
static void vkDestroyImageView(VkDevice d,VkImageView v,const void*a){if(v){assert((uintptr_t)v!=999);assert(view_live>0);view_live--;}}
static void vkDestroySwapchainKHR(VkDevice d,VkSwapchainKHR s,const void*a){if(s){assert(chain_live);chain_live=0;}}
static VkCommandBuffer vkpt_begin_command_buffer(void*p){return (VkCommandBuffer)(uintptr_t)1;}
static void vkpt_submit_command_buffer_simple(VkCommandBuffer b,VkQueue q,bool w){}
static void vkpt_wait_idle(VkQueue q,void*p){}
static VkResult vkDeviceWaitIdle(VkDevice d){idles++;if(idle_result)return idle_result;assert(fence_signal);return VK_SUCCESS;}
static void vkpt_draw_destroy_desktop534(void){assert(fence_signal&&!acquire_signal);draw_live=0;}
static VkResult vkpt_draw_create_desktop534(void){draw_live=1;return draw_error;}
static VkResult vkQueueSubmit(VkQueue q,unsigned n,const VkSubmitInfo*s,VkFence f){assert(n==1&&!s->commandBufferCount);if(submit_result)return submit_result;empty_submits++;if(s->waitSemaphoreCount){assert(acquire_signal);acquire_signal=0;acquire_waits++;}fence_signal=1;return VK_SUCCESS;}
static void B2WP_Commit(bool b){assert(!b);}
static void vkpt_draw_clear_stretch_pics(void){}
static void draw_profiler(bool b){}
static void vkpt_tone_mapping_draw_debug(void){}
static void vkpt_draw_submit_stretch_pics(VkCommandBuffer b){}
void vkpt_b2_wall_return(VkCommandBuffer b){}
static bool vkpt_fsr_is_enabled(void){return false;}
static void vkpt_fsr_final_blit(VkCommandBuffer b,bool w){}
static void vkpt_final_blit(VkCommandBuffer b,int i,VkExtent2D e,bool n,bool w){}
static unsigned B2G_Width(void){return 100;}
static unsigned B2G_Height(void){return 100;}
static bool B2XR_CopyEye(VkCommandBuffer b,VkImage i,unsigned e,unsigned w,unsigned h){return copy_ok;}
static void B2XR_RequestStop(void){stop_count++;}
static void b2_finish_eye(unsigned e){finish_count++;b2_cadence.finished_mask|=1u<<e;b2_surface_pair.finished|=1u<<e;}
static void vkpt_submit_command_buffer(VkCommandBuffer b,VkQueue q,unsigned mask,unsigned waits,VkSemaphore*w,VkPipelineStageFlags*s,unsigned*wi,unsigned signals,VkSemaphore*sig,unsigned*si,VkFence f){eye_submits++;assert(!fence_signal);if(waits){assert(acquire_signal);acquire_signal=0;acquire_waits++;}if(signals){assert(!render_signal);render_signal=1;}fence_signal=1;}
static VkResult vkQueuePresentKHR(VkQueue q,const VkPresentInfoKHR*p){present_count++;assert(render_signal);if(present_result==VK_SUCCESS)render_signal=0;return present_result;}
static void SDL_Vulkan_GetDrawableSize(void*w,int*x,int*y){*x=*y=drawable;}
static void b2_note_desktop_drawable(int w,int h){b2_desktop_drawable=(VkExtent2D){w,h};}
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
#endif
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
static void surface_boundary(void){bool mode_changed=b2_desktop_dirty534;
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

}

static void acquisition_result(VkResult res_swapchain){
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
	}
}

static void reset_frame(int eye,bool no_image){static Frame frame={7,2};static refdef_t fd;fd=(refdef_t){true,true,eye,0,&frame};vkpt_refdef.fd=&fd;b2_mirror.held=eye!=0;b2_mirror.no_image534=no_image;b2_mirror.wait_acquire=eye==0&&!no_image;acquire_signal=b2_mirror.wait_acquire;b2_physical_started580=true;fence_signal=0;frame_ready=true;}
int main(void){qvk.device_count=1;
// Actual construction and teardown retain exactly the successful handles.
for(int n=1;n<=3;n++){fail_view=n;view_calls=0;assert(create_swapchain()==VK_ERROR_OUT_OF_DEVICE_MEMORY);assert(view_live==n-1);assert(destroy_swapchain()==VK_SUCCESS);assert(!view_live&&!chain_live);}
fail_view=0;fail_images=1;assert(create_swapchain()==VK_ERROR_SURFACE_LOST_KHR);destroy_swapchain();assert(!chain_live);fail_images=0;
fail_formats=1;assert(create_swapchain()==VK_ERROR_SURFACE_LOST_KHR);assert(!chain_live);fail_formats=0;
fail_modes=1;assert(create_swapchain()==VK_ERROR_SURFACE_LOST_KHR);fail_modes=0;fail_pick=1;assert(create_swapchain()==VK_ERROR_FORMAT_NOT_SUPPORTED);fail_pick=0;
assert(create_swapchain()==VK_SUCCESS);assert(view_live==3);destroy_swapchain();assert(!view_live&&!chain_live);
// Real EndFrame refusal reaches real retire/discard: world/UI, acquired/retained/no mirror.
for(int ui=0;ui<2;ui++)for(int eye=0;eye<2;eye++)for(int absent=0;absent<2;absent++){
 int e=empty_submits,p=present_count,f=finish_count,s=eye_submits,w=acquire_waits;
 if(!absent)assert(create_swapchain()==VK_SUCCESS);
 reset_frame(eye,absent);b2_cadence.pair_prepared=!ui;b2_surface_pair.open=ui;copy_ok=0;
 R_EndFrame_RTX();assert(empty_submits==e+1&&eye_submits==s&&present_count==p&&finish_count==f);
 assert(acquire_waits==w+(eye==0&&!absent));assert(fence_signal&&!acquire_signal&&!b2_physical_started580&&!b2_mirror.held&&!b2_mirror.wait_acquire);
 assert(!b2_cadence.pair_prepared&&!b2_surface_pair.open&&!vkpt_refdef.fd->b2_render_accepted&&!view_live&&!chain_live);
}
// Successfully submitted first eye, failed desktop present on final eye, then no reuse.
copy_ok=1;assert(create_swapchain()==VK_SUCCESS);reset_frame(0,false);R_EndFrame_RTX();assert(b2_mirror.held&&!b2_physical_started580&&!b2_mirror.wait_acquire&&!acquire_signal&&!render_signal);
reset_frame(1,false);present_result=VK_ERROR_SURFACE_LOST_KHR;R_EndFrame_RTX();assert(b2_desktop_failed582&&b2_mirror.no_image534&&render_signal&&!chain_live&&!view_live&&!b2_mirror.held);
int p=present_count,w=acquire_waits,s=eye_submits;reset_frame(0,true);R_EndFrame_RTX();reset_frame(1,true);R_EndFrame_RTX();assert(present_count==p&&acquire_waits==w&&eye_submits==s+2&&render_signal);
int c=creates;cv[1].integer=1;recreate_swapchain();assert(creates==c&&qvk.surf_vsync);
// Recreation defers a retained pair/minimised window; partial framebuffer failure is cleaned.
b2_desktop_failed582=false;b2_mirror.held=true;recreate_swapchain();assert(creates==c);b2_mirror.held=false;drawable=1;recreate_swapchain();assert(creates==c);drawable=100;
qvk.surf_format=(VkSurfaceFormatKHR){VK_FORMAT_R8G8B8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};draw_error=VK_ERROR_OUT_OF_DEVICE_MEMORY;recreate_swapchain();assert(b2_desktop_failed582&&!chain_live&&!view_live&&!draw_live);draw_error=0;

// Actual later-eye surface boundary, reuse and EndFrame: dirty SUBOPTIMAL
// cannot query/tear down the held image. Recovery runs on the next boundary.
for(int views=2;views<=4;views+=2)for(int ui=0;ui<2;ui++){
 static Frame pair;static refdef_t fd;pair=(Frame){19,views};fd=(refdef_t){true,true,0,ui?RDF_NOWORLDMODEL:0,&pair};vkpt_refdef.fd=&fd;
 memset(&b2_mirror,0,sizeof(b2_mirror));
 b2_desktop_failed582=b2_desktop_dirty534=false;b2_physical_started580=false;
 b2_cadence.finished_mask=b2_surface_pair.finished=0;render_signal=acquire_signal=0;fence_signal=1;
 qvk.frame_counter=30;present_result=VK_SUCCESS;copy_ok=1;fail_query=0;
 assert(create_swapchain()==VK_SUCCESS);int queries=query_count,waits=acquire_waits,submits=eye_submits,presents=present_count;
 for(int eye=0;eye<views;eye++){
  fd.b2_eye=eye;surface_boundary();assert(query_count==queries);
  b2_mirror.wait_acquire=!b2_mirror_reuse();
  if(!eye){acquire_signal=1;acquisition_result(VK_SUBOPTIMAL_KHR);assert(b2_desktop_dirty534);}
  else assert(b2_mirror.held&&!b2_mirror.wait_acquire&&chain_live&&!b2_desktop_failed582);
  fail_query=VK_ERROR_SURFACE_LOST_KHR;
  b2_physical_started580=true;fence_signal=0;frame_ready=true;R_EndFrame_RTX();
  assert(fence_signal&&!acquire_signal&&!b2_mirror.wait_acquire&&!b2_physical_started580);
 }
 assert(!b2_mirror.held&&eye_submits==submits+views&&acquire_waits==waits+1&&present_count==presents+1&&!render_signal);
 fd.b2_eye=0;surface_boundary();assert(query_count==queries+1&&b2_desktop_failed582&&!chain_live&&!view_live&&b2_mirror.no_image534);
 assert(!acquire_signal&&!render_signal&&fence_signal);fail_query=0;
}

// Unknown/device/idle failure never becomes a successful mirror-only retirement.
b2_desktop_failed582=false;if(!setjmp(trap)){b2_disable_desktop582("test",VK_ERROR_DEVICE_LOST);assert(0);}assert(vr_device_lost_terminal&&vr_abandon_children&&!b2_desktop_failed582);
vr_device_lost_terminal=vr_abandon_children=false;if(!setjmp(trap)){b2_disable_desktop582("test",VK_ERROR_UNKNOWN);assert(0);}assert(vr_abandon_children&&!b2_desktop_failed582);
idle_result=VK_ERROR_DEVICE_LOST;if(!setjmp(trap)){b2_disable_desktop582("test",VK_ERROR_SURFACE_LOST_KHR);assert(0);}assert(vr_device_lost_terminal&&!b2_desktop_failed582);idle_result=0;
reset_frame(0,true);copy_ok=0;submit_result=VK_ERROR_DEVICE_LOST;if(!setjmp(trap)){R_EndFrame_RTX();assert(0);}assert(!fence_signal);
puts("actual EndFrame/retire/discard/create/destroy/recreate: refusal ownership, partial views/framebuffers, mirror semaphore non-reuse, terminal failures PASS");return 0;}
