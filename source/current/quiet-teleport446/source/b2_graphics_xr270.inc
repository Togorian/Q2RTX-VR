// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
static b2g_info_t b2g={.width=1948,.height=1563,.percent=100};
uint32_t B2G_Width(void){return b2g.width;}
uint32_t B2G_Height(void){return b2g.height;}
void B2G_Info(b2g_info_t *out)
{
    if(!out)return;
    *out=b2g;
    for(uint32_t i=0;i<xr.view_count;i++) {
        out->output_width[i]=xr.eyes[i].width;out->output_height[i]=xr.eyes[i].height;
    }
}
static bool b2g_settings364(int *runtime,int *percent)
{
    cvar_t *mode=Cvar_Get("b2_render_runtime","1",CVAR_ARCHIVE);
    cvar_t *scale=Cvar_Get("b2_render_percent","100",CVAR_ARCHIVE);
    if(!isfinite(mode->value) || mode->value!=mode->integer ||
       (mode->integer!=0 && mode->integer!=1) ||
       !B2G_Percent364(scale->value,scale->integer,percent))return false;
    if(scale->integer!=*percent) {
        B2D_Record(0,"GRAPHICS_SCALE_NORMALISED previous=%d selected=%d maximum=100",scale->integer,*percent);
        Cvar_SetByVar(scale,"100",FROM_CODE);
        if(scale->integer!=*percent || scale->value!=*percent)return false;
    }
    // Accept archived0 for compatibility, but this candidate always scales runtime.
    *runtime=1;return true;
}
// Private cached session data; the shared56-byte b2g_info_t ABI is unchanged.
static uint32_t b2g_envelope_w442,b2g_envelope_h442;
static VkPhysicalDeviceProperties b2g_limits442;
static bool b2g_extent442(int percent,uint32_t w,uint32_t h,
    const VkPhysicalDeviceProperties *limits,uint32_t *out_w,uint32_t *out_h)
{
    const VkPhysicalDeviceProperties p=*limits;
    if(p.limits.maxImageArrayLayers<1 || !B2G_Extent(1,percent,w,h,
       min(p.limits.maxImageDimension2D,p.limits.maxFramebufferWidth),
       min(p.limits.maxImageDimension2D,p.limits.maxFramebufferHeight),&w,&h))return false;
    if(w>p.limits.maxViewportDimensions[0] || h>p.limits.maxViewportDimensions[1] ||
       (double)w>p.limits.viewportBoundsRange[1] || (double)h>p.limits.viewportBoundsRange[1] ||
       p.limits.viewportBoundsRange[0]>0 || w>p.limits.maxComputeWorkGroupCount[0] ||
       h>p.limits.maxComputeWorkGroupCount[1] || p.limits.maxComputeWorkGroupCount[2]<2)return false;
    *out_w=w;*out_h=h;return true;
}
bool B2G_Pending442(int percent,uint32_t *width,uint32_t *height)
{
    if(!width || !height || !b2g.applied || xr.session==XR_NULL_HANDLE ||
       (xr.view_count!=2 && xr.view_count!=4) || xr.view_count!=b2g.views)return false;
    for(uint32_t i=0;i<xr.view_count;i++)
        if(!xr.eyes[i].width || !xr.eyes[i].height)return false;
    return b2g_extent442(percent,b2g_envelope_w442,b2g_envelope_h442,
        &b2g_limits442,width,height);
}
static bool b2g_select(VkPhysicalDevice physical_device)
{
    int runtime,percent;
    if(!b2g_settings364(&runtime,&percent))return false;
    B2D_Record(0,"GRAPHICS_REQUEST runtime=%d percent=%d views=%u",runtime,percent,xr.view_count);
    uint32_t w=0,h=0;
    VkPhysicalDeviceProperties p;vkGetPhysicalDeviceProperties(physical_device,&p);
    for(uint32_t i=0;i<xr.view_count;i++) {
        w=max(w,xr.eyes[i].allocation_width);h=max(h,xr.eyes[i].allocation_height);
        if(xr.eyes[i].allocation_width>p.limits.maxImageDimension2D ||
           xr.eyes[i].allocation_height>p.limits.maxImageDimension2D)return false;
    }
    B2D_Record(0,"GRAPHICS_ENVELOPE output=%ux%u image_limit=%u framebuffer=%ux%u viewport=%ux%u",w,h,p.limits.maxImageDimension2D,p.limits.maxFramebufferWidth,p.limits.maxFramebufferHeight,p.limits.maxViewportDimensions[0],p.limits.maxViewportDimensions[1]);
    const uint32_t envelope_w=w,envelope_h=h;
    if(!b2g_extent442(percent,w,h,&p,&w,&h))return false;
    // A renderer restart may recreate the session, but changing this immutable
    // process selection requires a complete restart, as stated in the menu.
    if(b2g.applied && (b2g.width!=w || b2g.height!=h || b2g.runtime!=runtime || b2g.percent!=percent))return false;
    b2g.width=w;b2g.height=h;b2g.runtime=runtime;b2g.percent=percent;b2g.views=xr.view_count;
    for(uint32_t i=0;i<xr.view_count;i++) {
        b2g.output_width[i]=xr.eyes[i].allocation_width;b2g.output_height[i]=xr.eyes[i].allocation_height;
    }
    B2D_Record(0,"GRAPHICS_SELECTED internal=%ux%u per_extra_view_raw=%llu",w,h,(unsigned long long)B2G_TemporalBytes(w,h));
    b2g_envelope_w442=envelope_w;b2g_envelope_h442=envelope_h;b2g_limits442=p;
    b2g.applied=true;return true;
}
