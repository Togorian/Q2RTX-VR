// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 14 November 2026: historical RC6 fixture regenerated from the selected
// Q2RTX VR/Quake II RTX source spans listed in R6.json; external effects are seams.
/*
Copyright (C) 1997-2001 Id Software, Inc.

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
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <setjmp.h>
#define REFERENCE_B_B2 1
#define VK_SUCCESS 0
#define VK_ERROR_OUT_OF_DATE_KHR 1
#define ERR_FATAL 1
typedef int VkResult;
typedef struct {unsigned width,height;} Extent;
typedef struct {Extent currentExtent;} VkSurfaceCapabilitiesKHR;
typedef struct {int format,colorSpace;} VkSurfaceFormatKHR;
static struct {int window,physical_device,surface,device,swap_chain; bool b2_temporal_activated; int surf_vsync; VkSurfaceFormatKHR surf_format;} qvk;
static struct {bool held;} b2_mirror;
static bool b2_desktop_dirty534,b2_desktop_failed582,vr_device_lost_terminal,vr_abandon_children;
static struct {int integer;} vsync={0},*cvar_vsync=&vsync;
#define VK_ERROR_DEVICE_LOST 2
#define VK_ERROR_FORMAT_NOT_SUPPORTED 3
#define Com_EPrintf(...) ((void)0)
static const char *qvk_result_to_string(int r){return "fixture";}
static void b2_disable_desktop582(const char *s,int r){b2_desktop_failed582=true;}
static int w=640,h=480,query_result,idle_result,destroy_result,create_result,fb_result,calls,format_change;
static jmp_buf trap;
#define Com_Error(...) longjmp(trap,1)
static bool b2_desktop_enabled(void){return !b2_desktop_failed582;}
static void SDL_Vulkan_GetDrawableSize(int win,int *x,int *y){*x=w;*y=h;}
static void b2_note_desktop_drawable(int x,int y){}
static int vkGetPhysicalDeviceSurfaceCapabilitiesKHR(int p,int s,VkSurfaceCapabilitiesKHR *c){c->currentExtent=(Extent){w,h};return query_result;}
static int vkDeviceWaitIdle(int d){calls|=1;return idle_result;}
static void vkpt_draw_destroy_desktop534(void){assert(calls&1);calls|=2;}
static int destroy_swapchain(void){calls|=4;return destroy_result;}
static int create_swapchain(void){calls|=8;qvk.swap_chain=1;qvk.surf_format.format+=format_change;return create_result;}
static int vkpt_draw_create_desktop534(void){calls|=16;return fb_result;}
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
int main(void){
qvk.b2_temporal_activated=true;qvk.swap_chain=1;
b2_mirror.held=true;recreate_swapchain();assert(calls==0 && b2_desktop_dirty534);
b2_mirror.held=false;w=h=0;recreate_swapchain();assert(calls==0 && b2_desktop_dirty534);
w=640;h=480;recreate_swapchain();assert(calls==31 && !b2_desktop_dirty534);
calls=0;query_result=2;recreate_swapchain();assert(b2_desktop_failed582&&calls==0);query_result=0;b2_desktop_failed582=false;
idle_result=2;if(!setjmp(trap)){recreate_swapchain();assert(0);}assert(calls==1);idle_result=0;
calls=0;create_result=1;recreate_swapchain();assert(calls==15 && b2_desktop_dirty534);create_result=0;
calls=0;recreate_swapchain();assert(calls==31 && !b2_desktop_dirty534);
format_change=1;recreate_swapchain();assert(b2_desktop_failed582);format_change=0;b2_desktop_failed582=false;
fb_result=2;recreate_swapchain();assert(b2_desktop_failed582);puts("desktop branch: held/minimised/restore/idle/errors/retry pass");return 0;}
