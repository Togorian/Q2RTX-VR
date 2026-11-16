// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <vulkan/vulkan.h>
#include "refresh/reference_b_b2.h"

bool B2XR_Init(uint32_t vk_api_version);
VkResult B2XR_CreateVulkanInstance(const VkInstanceCreateInfo *info, VkInstance *instance);
VkPhysicalDevice B2XR_PhysicalDevice(VkInstance instance);
VkResult B2XR_CreateVulkanDevice(const VkDeviceCreateInfo *info,
    VkPhysicalDevice physical_device, VkDevice *device);
bool B2XR_CreateSession(VkInstance instance, VkPhysicalDevice physical_device,
    VkDevice device, uint32_t queue_family, VkQueue queue);
bool B2XR_CopyEye(VkCommandBuffer commands, VkImage source, uint32_t eye,
    uint32_t width, uint32_t height);
bool B2XR_FinishEye(uint32_t eye);
void B2XR_Shutdown(void);
