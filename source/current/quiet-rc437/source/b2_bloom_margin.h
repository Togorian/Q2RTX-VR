// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "b2_bloom_margin_math.h"
#include "b2_startup276.h"
#include "bloom_margin_spv.h"

typedef struct { VkBuffer buffer; VkDeviceMemory memory; VkDeviceSize bytes; } bm_buffer_t;
static struct {
    bm_buffer_t buffers[4];
    VkDescriptorSetLayout descriptors;
    VkDescriptorPool pool;
    VkDescriptorSet set;
    VkPipelineLayout layout;
    VkPipeline pipeline;
    bm_binding_t source[2];
    VkDeviceSize allocation_bytes;
    VkExtent2D extent;
    int ready;
} bm;
static uint32_t bm_resources;
static cvar_t *bm_control;
typedef struct {
    float h[12]; int extent[4],source[4]; float kernel[4]; int shape[4];
} bm_push_t;
typedef char bm_push_must_be_112[(sizeof(bm_push_t)==112)?1:-1];

/* At most twelve short events in the existing trial startup report. */
static void bm_note(const char *event,VkExtent2D requested,int eye,int result)
{
    static unsigned count;
    if(count<12){
        B2D_Record(0,"BLOOM_MARGIN event=%s requested=%ux%u images=%ux%u allocated=%ux%u generation=%u eye=%d result=%d recorded_only=1",
            event,requested.width,requested.height,qvk.extent_taa_images.width,qvk.extent_taa_images.height,
            bm.extent.width,bm.extent.height,bm_resources,eye,result);
        count++;
    }
}
static void bm_destroy(void)
{
    bm.ready=0;
    vkDestroyPipeline(qvk.device,bm.pipeline,NULL);
    vkDestroyDescriptorPool(qvk.device,bm.pool,NULL);
    vkDestroyPipelineLayout(qvk.device,bm.layout,NULL);
    vkDestroyDescriptorSetLayout(qvk.device,bm.descriptors,NULL);
    for(int i=0;i<4;i++) {
        vkDestroyBuffer(qvk.device,bm.buffers[i].buffer,NULL);
        vkFreeMemory(qvk.device,bm.buffers[i].memory,NULL);
    }
    memset(&bm,0,sizeof(bm));
}
static VkResult bm_buffer_create(bm_buffer_t *b,VkDeviceSize bytes)
{
    VkBufferCreateInfo ci={.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,.size=bytes,
        .usage=VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode=VK_SHARING_MODE_EXCLUSIVE};
    VkResult r=vkCreateBuffer(qvk.device,&ci,NULL,&b->buffer);
    if(r!=VK_SUCCESS) return r;
    VkMemoryRequirements req;vkGetBufferMemoryRequirements(qvk.device,b->buffer,&req);
    if(req.size<bytes || req.size>BM_BYTE_CAP-bm.allocation_bytes) return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    uint32_t type=UINT32_MAX;
    for(uint32_t i=0;i<qvk.mem_properties.memoryTypeCount;i++)
        if((req.memoryTypeBits&(1u<<i)) && (qvk.mem_properties.memoryTypes[i].propertyFlags&VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) { type=i;break; }
    if(type==UINT32_MAX) return VK_ERROR_FEATURE_NOT_PRESENT;
    VkMemoryAllocateInfo ai={.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,.allocationSize=req.size,.memoryTypeIndex=type};
    r=vkAllocateMemory(qvk.device,&ai,NULL,&b->memory);if(r!=VK_SUCCESS) return r;
    bm.allocation_bytes+=req.size;b->bytes=bytes;
    return vkBindBufferMemory(qvk.device,b->buffer,b->memory,0);
}
static void bm_create(void)
{
    if(!B2XR_Requested()||qvk.device_count!=1) return;
    VkPhysicalDeviceProperties properties;vkGetPhysicalDeviceProperties(qvk.physical_device,&properties);
    const VkPhysicalDeviceLimits *limits=&properties.limits;
    VkExtent2D extent=qvk.extent_taa_images;
    uint64_t source_bytes=0,scratch_bytes=0;
    const char *reason=bm_plan(extent.width,extent.height,limits->maxStorageBufferRange,
        limits->maxComputeWorkGroupCount[0],limits->maxComputeWorkGroupCount[1],&source_bytes,&scratch_bytes);
    if(reason){bm_note(reason,extent,-1,0);return;}
    if(extent.width>limits->maxImageDimension2D || extent.height>limits->maxImageDimension2D ||
        limits->maxComputeWorkGroupSize[0]<16 || limits->maxComputeWorkGroupSize[1]<16 ||
        limits->maxComputeWorkGroupInvocations<256 || limits->maxComputeWorkGroupCount[2]<1 ||
        limits->maxPushConstantsSize<sizeof(bm_push_t) || limits->maxBoundDescriptorSets<4 ||
        limits->maxPerStageDescriptorStorageBuffers<4 || limits->maxDescriptorSetStorageBuffers<4){
        bm_note("device-limit",extent,-1,0);return;
    }
    VkResult result=VK_SUCCESS;VkShaderModule module=VK_NULL_HANDLE;
    for(int i=0;i<4;i++) {
        result=bm_buffer_create(&bm.buffers[i],i<2?source_bytes:scratch_bytes);
        if(result!=VK_SUCCESS) goto fail;
    }
    VkDescriptorSetLayoutBinding bindings[4];memset(bindings,0,sizeof(bindings));
    for(int i=0;i<4;i++) bindings[i]=(VkDescriptorSetLayoutBinding){.binding=i,.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount=1,.stageFlags=VK_SHADER_STAGE_COMPUTE_BIT};
    VkDescriptorSetLayoutCreateInfo di={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,.bindingCount=4,.pBindings=bindings};
    result=vkCreateDescriptorSetLayout(qvk.device,&di,NULL,&bm.descriptors);if(result!=VK_SUCCESS) goto fail;
    VkDescriptorPoolSize ps={VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,4};
    VkDescriptorPoolCreateInfo pi={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,.maxSets=1,.poolSizeCount=1,.pPoolSizes=&ps};
    result=vkCreateDescriptorPool(qvk.device,&pi,NULL,&bm.pool);if(result!=VK_SUCCESS) goto fail;
    VkDescriptorSetAllocateInfo da={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,.descriptorPool=bm.pool,.descriptorSetCount=1,.pSetLayouts=&bm.descriptors};
    result=vkAllocateDescriptorSets(qvk.device,&da,&bm.set);if(result!=VK_SUCCESS) goto fail;
    VkDescriptorBufferInfo bi[4];VkWriteDescriptorSet writes[4];
    for(int i=0;i<4;i++) {
        bi[i]=(VkDescriptorBufferInfo){.buffer=bm.buffers[i].buffer,.offset=0,.range=bm.buffers[i].bytes};
        writes[i]=(VkWriteDescriptorSet){.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,.dstSet=bm.set,.dstBinding=i,
            .descriptorCount=1,.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,.pBufferInfo=bi+i};
    }
    vkUpdateDescriptorSets(qvk.device,4,writes,0,NULL);
    VkDescriptorSetLayout sets[4]={qvk.desc_set_layout_ubo,qvk.desc_set_layout_textures,qvk.desc_set_layout_vertex_buffer,bm.descriptors};
    VkPushConstantRange range={VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(bm_push_t)};
    VkPipelineLayoutCreateInfo li={.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,.setLayoutCount=4,.pSetLayouts=sets,
        .pushConstantRangeCount=1,.pPushConstantRanges=&range};
    result=vkCreatePipelineLayout(qvk.device,&li,NULL,&bm.layout);if(result!=VK_SUCCESS) goto fail;
    VkShaderModuleCreateInfo si={.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,.codeSize=sizeof(bloom_margin_spv),.pCode=bloom_margin_spv};
    result=vkCreateShaderModule(qvk.device,&si,NULL,&module);if(result!=VK_SUCCESS) goto fail;
    VkComputePipelineCreateInfo pci={.sType=VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage={.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_COMPUTE_BIT,.module=module,.pName="main"},.layout=bm.layout};
    result=vkCreateComputePipelines(qvk.device,0,1,&pci,NULL,&bm.pipeline);
    vkDestroyShaderModule(qvk.device,module,NULL);module=VK_NULL_HANDLE;
    if(result!=VK_SUCCESS) goto fail;
    bm.extent=extent;bm.ready=1;bm_resources++;
    bm_note("ready",extent,-1,0);
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_BLOOM_MARGIN resources=%u allocated=%llu source=%llu scratch=%llu ready=1 recorded_only=1\n",
        bm_resources,(unsigned long long)bm.allocation_bytes,(unsigned long long)source_bytes,(unsigned long long)scratch_bytes);
    return;
fail:
    if(module) vkDestroyShaderModule(qvk.device,module,NULL);
    Com_WPrintf("B2_BLOOM_MARGIN allocation/pipeline failure=%d; original bloom retained\n",result);
    bm_destroy();
    bm_note("setup-failed",extent,-1,(int)result);
}
static void bm_barrier(VkCommandBuffer cmd)
{
    /* Across serial graphics-queue submissions as well as within one view.
     * Includes transfer reset, retained writes, scratch reads/writes and the
     * final image overwrite. No descriptor updates occur during rendering. */
    VkMemoryBarrier b={.sType=VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        .srcAccessMask=VK_ACCESS_MEMORY_READ_BIT|VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask=VK_ACCESS_MEMORY_READ_BIT|VK_ACCESS_MEMORY_WRITE_BIT};
    vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,0,1,&b,0,NULL,0,NULL);
}
static void bm_dispatch(VkCommandBuffer cmd,bm_push_t *push,uint32_t w,uint32_t h)
{
    VkDescriptorSet sets[4]={qvk.desc_set_ubo,qvk_get_current_desc_set_textures(),qvk.desc_set_vertex_buffer,bm.set};
    vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,bm.pipeline);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,bm.layout,0,4,sets,0,NULL);
    vkCmdPushConstants(cmd,bm.layout,VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(*push),push);
    vkCmdDispatch(cmd,(w+15)/16,(h+15)/16,1);
    bm_barrier(cmd);
}
static const b2_frame_t *bm_frame(VkExtent2D extent)
{
    const refdef_t *fd=vkpt_refdef.fd;
    const b2_frame_t *f=fd&&fd->b2_active?fd->b2_frame:NULL;
    if(!f||!f->tracking_valid||!f->should_render||f->view_count!=4||fd->b2_eye>=4||
        !bm.ready||!bm_control||bm_control->integer!=1||qvk.frame_menu_mode||cvar_bloom_debug->integer!=0||
        !cvar_b2_bloom_scale||cvar_b2_bloom_scale->integer!=1||
        extent.width!=bm.extent.width||extent.height!=bm.extent.height||
        qvk.extent_taa_images.width!=extent.width||qvk.extent_taa_images.height!=extent.height) return NULL;
    return f;
}
static void bm_capture(VkCommandBuffer cmd,VkExtent2D extent)
{
    const refdef_t *fd=vkpt_refdef.fd;
    if(!fd||!fd->b2_active||fd->b2_eye==0) memset(bm.source,0,sizeof(bm.source));
    const b2_frame_t *f=bm_frame(extent);if(!f) {
        if(fd && fd->b2_active && fd->b2_eye==0 && bm.ready &&
           (extent.width!=bm.extent.width || extent.height!=bm.extent.height ||
            extent.width!=qvk.extent_taa_images.width || extent.height!=qvk.extent_taa_images.height)){
            static uint32_t last_mismatch;
            if(last_mismatch!=bm_resources){bm_note("extent-mismatch",extent,0,0);last_mismatch=bm_resources;}
        }
        memset(bm.source,0,sizeof(bm.source));return;
    }
    uint32_t eye=fd->b2_eye;if(eye>=2) return;
    bm_binding_t *b=bm.source+eye;memset(b,0,sizeof(*b));
    const b2_view_t *v=f->views+eye;
    if(!bm_projection(v->P)||!bm_rotation(v->V)) return;
    b->eye=eye;b->logical=f->logical_index;b->generation=f->history_generation;b->render=qvk.frame_counter;
    b->resources=bm_resources;b->width=extent.width;b->height=extent.height;b->predicted=f->predicted_time_ns;
    memcpy(b->origin,v->origin,sizeof(b->origin));memcpy(b->position,f->positions[eye],sizeof(b->position));
    memcpy(b->P,v->P,sizeof(b->P));memcpy(b->V,v->V,sizeof(b->V));
    b->sigma=bloom_sigma;b->intensity=bloom_intensity;b->exposure=Cvar_VariableValue("tm_exposure_bias");
    b->source_image=(uint64_t)(uintptr_t)qvk.images[VKPT_IMG_TAA_OUTPUT];
    bm_push_t push={0};push.source[0]=extent.width;push.source[1]=extent.height;push.source[2]=eye;
    bm_barrier(cmd);vkCmdFillBuffer(cmd,bm.buffers[eye].buffer,0,8,0);bm_barrier(cmd);
    bm_dispatch(cmd,&push,extent.width,extent.height);b->valid=1;
}
static void bm_apply(VkCommandBuffer cmd,VkExtent2D extent)
{
    const b2_frame_t *f=bm_frame(extent);const refdef_t *fd=vkpt_refdef.fd;
    if(!f||fd->b2_eye<2) return;
    uint32_t eye=fd->b2_eye-2;const b2_view_t *v=f->views+fd->b2_eye;
    bm_binding_t *b=bm.source+eye;bm_layout_t l;bm_push_t push={0};int status=-1;
    if(!bm_match(b,eye,f->logical_index,f->history_generation,qvk.frame_counter,bm_resources,extent.width,extent.height,
        f->predicted_time_ns,v->origin,f->positions[fd->b2_eye],bloom_sigma,bloom_intensity,Cvar_VariableValue("tm_exposure_bias"),
        (uint64_t)(uintptr_t)qvk.images[VKPT_IMG_TAA_OUTPUT])) goto report;
    if(!bm_layout(extent.width,extent.height,push_constants_hblur.num_samples,push_constants_vblur.num_samples,&l)||
        !bm_homography(v->P,v->invV,b->P,b->V,push.h)||!bm_support_valid(push.h,&l,extent.width,extent.height)) { status=-2;goto report; }
    push.extent[0]=extent.width;push.extent[1]=extent.height;push.extent[2]=l.px;push.extent[3]=l.py;
    push.source[0]=b->width;push.source[1]=b->height;push.source[2]=eye;
    push.kernel[0]=push_constants_hblur.argument_scale;push.kernel[1]=push_constants_hblur.normalization_scale;
    push.kernel[2]=push_constants_vblur.argument_scale;push.kernel[3]=push_constants_vblur.normalization_scale;
    push.shape[0]=push_constants_hblur.num_samples;push.shape[1]=push_constants_vblur.num_samples;push.shape[2]=l.ew;push.shape[3]=l.eh;
    bm_barrier(cmd);
    push.source[3]=1;bm_dispatch(cmd,&push,l.ew,l.eh);
    push.source[3]=2;bm_dispatch(cmd,&push,l.w,l.eh);
    push.source[3]=3;bm_dispatch(cmd,&push,l.w,l.h);
    push.source[3]=4;bm_dispatch(cmd,&push,l.w,l.h);
    status=1;
report:;
    static int previous[2]={-99,-99};
    static uint32_t last_generation[2];
    if(previous[eye]!=status||last_generation[eye]!=bm_resources||f->logical_index%120==0) {
        if(previous[eye]!=status || last_generation[eye]!=bm_resources){
            bm_note(status==1?"focus-recorded":status==-1?"capture-mismatch":"projection-or-kernel",extent,(int)fd->b2_eye,status);
            last_generation[eye]=bm_resources;
        }
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_BLOOM_MARGIN logical=%u generation=%u eye=%u status=%d recorded_only=1 invalid_hdr_uses_original=1\n",
            f->logical_index,f->history_generation,eye,status);previous[eye]=status;
    }
}
