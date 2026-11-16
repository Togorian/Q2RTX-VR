// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Design/direction by Togorian; implementation with Codex assistance.
// SPDX-License-Identifier: GPL-2.0-or-later
// Modified 12 November 2026: private VR services separated from diagnostics.
/* Private renderer services. Implementations stay in their owning renderer units. */
#ifndef VKPT_VR_SERVICES_H
#define VKPT_VR_SERVICES_H
const char *vkpt_b2_history_failure582(void);
uint32_t vkpt_vr_history_parity(void);
VkImage vkpt_vr_history_image(int image_index);
void vkpt_vr_require_maintenance_open(const char *operation);
void vkpt_vr_latch_device_loss(void);
void vkpt_vr_abandon_children(void);
VkResult vkpt_light_stats_clear_all(VkCommandBuffer command_buffer);
#endif
