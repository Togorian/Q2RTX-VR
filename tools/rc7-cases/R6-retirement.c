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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#define VK_SUCCESS 0
#define VK_STRUCTURE_TYPE_SUBMIT_INFO 1
#define VK_PIPELINE_STAGE_ALL_COMMANDS_BIT 1
#define ERR_FATAL 1
typedef int VkSemaphore;typedef unsigned VkPipelineStageFlags;
typedef struct {int sType;unsigned waitSemaphoreCount;int *pWaitSemaphores;unsigned *pWaitDstStageMask;} VkSubmitInfo;
static struct {bool held,no_image534,wait_acquire;} b2_mirror;
static struct {struct {int image_available;} semaphores[1][1];int queue_graphics,device,fences_frame_sync[1],current_frame_index;unsigned frame_counter;} qvk;
static struct {bool pair_prepared;unsigned logical_index,history_generation,rendered_mask,finished_mask,view_count,prepares,light_uploads,light_clears,bloom_updates,tone_updates,tone_applies[4];} b2_cadence;
static struct {bool open;unsigned views,logical,generation,rendered,finished;} b2_surface_pair;
static bool temporal_frame_valid,frame_ready,b2_settings_history_reset580,b2_settings_rejected580,b2_physical_started580,b2_desktop_dirty534,b2_settings_changed580;
static int submit_count,wait_count,idle_count,destroy_count,commits,sem_signalled,fence_signalled=1,submit_error,idle_error;
static jmp_buf trap;
#define Com_Error(...) longjmp(trap,1)
static int vkQueueSubmit(int q,int n,const VkSubmitInfo *s,int fence){assert(n==1);if(submit_error)return submit_error;submit_count++;if(s->waitSemaphoreCount){assert(sem_signalled);sem_signalled=0;wait_count++;}fence_signalled=1;return 0;}
static int vkDeviceWaitIdle(int d){idle_count++;assert(fence_signalled);return idle_error;}
static void vkpt_draw_destroy_desktop534(void){assert(fence_signalled && !sem_signalled);}
static int destroy_swapchain(void){destroy_count++;return 0;}
static void B2WP_Commit(bool valid){assert(!valid);commits++;}
static unsigned B2_ViewMask(unsigned n){return (1u<<n)-1;}
typedef struct {int integer;} cvar_t;
static cvar_t vars[7];
static cvar_t *cvar_hdr=&vars[0],*cvar_pt_projection=&vars[1],*cvar_drs_enable=&vars[2],*cvar_pt_freecam=&vars[3],*cvar_tm_enable=&vars[4],*cvar_pt_accumulation_rendering=&vars[5];
static int fsr;
static bool vkpt_fsr_is_enabled(void){return fsr;}
static int Cvar_VariableInteger(const char *s){return vars[6].integer;}
typedef struct {unsigned logical_index,history_generation,view_count;bool history_valid;} Frame;
static bool B2XR_EndFrame(bool accepted){assert(!accepted);return true;}
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
// Callback numeric compatibility is separately exercised by R6-recovery.
static bool callback_compatible;
static bool b2_callback_settings_compatible580(void){return callback_compatible;}
static bool b2_frame_settings_valid580(void)
{
    return (!b2_settings_changed580 || b2_callback_settings_compatible580()) && !cvar_hdr->integer &&
        !cvar_pt_projection->integer && !cvar_drs_enable->integer &&
        !cvar_pt_freecam->integer && cvar_tm_enable->integer &&
        !cvar_pt_accumulation_rendering->integer && !vkpt_fsr_is_enabled() &&
        !Cvar_VariableInteger("bloom_debug");
}
static void retry_primary(Frame *frame){bool b2_primary=true,first_pair=false;		if (b2_primary)
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
		}}
int main(void){
vars[4].integer=1;assert(b2_frame_settings_valid580());
for(int i=0;i<7;i++){vars[i].integer=i==4?0:1;assert(!b2_frame_settings_valid580());vars[i].integer=i==4?1:0;}
fsr=1;assert(!b2_frame_settings_valid580());fsr=0;b2_settings_changed580=true;assert(!b2_frame_settings_valid580());callback_compatible=true;assert(b2_frame_settings_valid580());callback_compatible=false;b2_settings_changed580=false;
// Before acquisition: no queue/fence/semaphore operation is owed.
b2_discard_settings_frame580();assert(submit_count==0 && idle_count==0 && destroy_count==0 && fence_signalled);
for(int ui=0;ui<2;ui++)for(int partial=0;partial<2;partial++){
int old_submits=submit_count,old_waits=wait_count,old_destroys=destroy_count;
b2_physical_started580=true;fence_signalled=0;
b2_mirror.held=partial;b2_mirror.wait_acquire=!partial;b2_mirror.no_image534=false;sem_signalled=!partial;
b2_cadence.pair_prepared=!ui;b2_surface_pair.open=ui;
b2_cadence.logical_index=7;b2_cadence.view_count=2;b2_cadence.finished_mask=b2_cadence.rendered_mask=partial?1:0;
b2_discard_settings_frame580();assert(B2XR_EndFrame(false));
assert(submit_count==old_submits+1 && wait_count==old_waits+!partial && destroy_count==old_destroys+1);
assert(fence_signalled && !sem_signalled && !b2_mirror.held && !b2_mirror.wait_acquire && !b2_cadence.pair_prepared && !b2_surface_pair.open && !temporal_frame_valid);
Frame frame={7,2,2,false};retry_primary(&frame);assert(b2_cadence.logical_index==7 && b2_cadence.finished_mask==0 && !b2_settings_history_reset580);
// Complete the retry's eyes at the harness boundary, then use the actual next-frame admission gate.
b2_cadence.finished_mask=b2_cadence.rendered_mask=3;frame.logical_index=8;retry_primary(&frame);assert(b2_cadence.logical_index==8);
}
// Rejection before beginning a later eye: prior acquire already consumed.
int n=submit_count;b2_mirror.held=true;b2_mirror.wait_acquire=false;b2_discard_settings_frame580();assert(submit_count==n && !b2_mirror.held);
// Lost submission/idle remain fatal, not converted to settings rejection.
b2_physical_started580=true;submit_error=1;if(!setjmp(trap)){b2_discard_settings_frame580();assert(0);}submit_error=0;
idle_error=1;if(!setjmp(trap)){b2_discard_settings_frame580();assert(0);}puts("settings predicate and actual discard/cadence: early, acquired, partial world/UI, same-index retry, fatal device errors PASS");return 0;}
