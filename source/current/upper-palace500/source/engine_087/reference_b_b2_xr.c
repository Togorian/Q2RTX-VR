// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 14 November 2026: public OpenXR application name is Q2RTX VR.
/* Modified 14 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
#include "../../../runtime-recovery555/source/b2_runtime555.h"
#include "../../../bloom331/source/b2_startup276.h"
#include "../../../runtime-menu442/source/b2_graphics270.h"
// SPDX-License-Identifier: GPL-2.0-or-later
// Private B2 prototype. No OpenXR call is made unless b2_openxr was requested.
// Uses the public Khronos API; the runtime implementation remains external.
#include "baseline/b2-consolidated215/inputs/origin-008/compat/engine-087/historical/quad-views/source/src/refresh/vkpt/vkpt.h"
#include "../../../../baseline/b2-denoiser-state233/source/b2_denoiser_state.h"
#include "../../../../baseline/b2-resume-cue261/source/b2_resume_cue261.h"
#include "baseline/b2-consolidated215/inputs/origin-008/compat/engine-087/historical/quad-views/source/inc/system/system.h"
#include "baseline/b2-consolidated215/inputs/origin-008/compat/engine-087/historical/quad-views/source/src/refresh/vkpt/reference_b_b2_xr.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-085/include/refresh/b2_haptics.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-085/include/refresh/b2_weapon_observation.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-085/include/refresh/b2_weapon_display.h"
#include "baseline/b2-consolidated215/inputs/origin-008/compat/engine-087/historical/quad-views/source/src/client/reference_b_b1.h"
#include "baseline/b2-consolidated215/inputs/origin-008/compat/engine-087/historical/quad-views/source/src/client/reference_b_b0u.h"

#define XR_NO_PROTOTYPES
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
static void b2_xr_note_failure555(XrResult result, const char *reason);
#include "../../../../baseline/b2-consolidated215/inputs/origin-085/include/refresh/b2_controller.h"
#include "../../../recentre347/final/source/b2_bindings287.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-085/include/refresh/b2_roomscale_bridge.h"
#define B2C_NATIVE
#include "../../../reference-realignment397/source/b2_controller_native397.inc"
static b2c_native_t b2_actions;
#include "../../../reference-realignment397/source/b2_realign397.h"
static void b2_xr_refresh_resume_cue261(void);

// Read the actual producer field using its selected declaration. The client
// and renderer retain older declarations in which this byte is padding.
bool B2XR_DenoiserFoveated(const struct b2_frame_s *frame)
{
    return frame->runtime_foveated;
}
static cvar_t *b2_weapon_trace;
static bool b2wv_attempted,b2wv_valid;
static unsigned b2wv_hand;
static b2c_snapshot_t b2wv_sample;
// One short capture and one explicit dump per process. No formatting while capturing.
#define B2WJ_LIMIT 128u
typedef struct {
    bool begun, end_called, same_frame, should_render, tracking, rendered_pair;
    bool failed, loss;
    uint32_t logical, generation, now, layers;
    int result;
    int64_t predicted, period;
    b2wp_observation_t weapon;
} b2wj_row_t;
_Static_assert(sizeof(b2wj_row_t)<=256, "B2 weapon observation storage bound");
static b2wj_row_t b2wj_rows[B2WJ_LIMIT], *b2wj_pending;
static const b2_frame_t *b2wj_frame;
static unsigned b2wj_count;
static bool b2wj_dumped;
static void b2wj_dump(void)
{
    b2wj_dumped=true;
    Com_Printf("B2WJ dump count=%u cap=%u raw_logical_may_repeat=1 pose_ns=command_time display_ns=visual_time matrix=column_major_3x4 end_marker_required=1\n",
        b2wj_count,B2WJ_LIMIT);
    for (unsigned i=0;i<b2wj_count;i++) {
        const b2wj_row_t *r=&b2wj_rows[i];const b2wp_observation_t *w=&r->weapon;
        const float *m=w->matrix;char line[1536];
        Q_snprintf(line,sizeof(line),
            "B2WJ row=%u begun=%u end_called=%u same_frame=%u logical_raw=%u generation=%u eye_ns=%lld period_ns=%lld now_ms=%u "
            "render=%u tracking=%u pair=%u failed=%u loss=%u layers=%u result=%d layer_accepted=%u "
            "requested=%u valid=%u record=%u reads=%u seq=%u cmd=%u ordinal=%u serial=%llu pose_ns=%lld capture_ms=%u head_ms=%u "
            "display_mode=%u display_used=%u display_ns=%lld display_capture_ms=%u "
            "slot=%u frame=%d oldframe=%d backlerp=%.9g emission=%u idle=%u recoil=%.9g,%.9g,%.9g "
            "matrix=%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g end=1",
            i,(unsigned)r->begun,(unsigned)r->end_called,(unsigned)r->same_frame,r->logical,r->generation,
            (long long)r->predicted,(long long)r->period,r->now,
            (unsigned)r->should_render,(unsigned)r->tracking,(unsigned)r->rendered_pair,(unsigned)r->failed,(unsigned)r->loss,
            r->layers,r->result,(unsigned)(r->end_called && r->same_frame && r->layers && r->result==XR_SUCCESS),
            (unsigned)w->requested,(unsigned)w->valid,(unsigned)w->record_present,w->matrix_reads,
            w->sequence,w->number,w->ordinal,(unsigned long long)w->serial,(long long)w->pose_time,w->capture_ms,w->head_ms,
            (unsigned)w->display_mode,(unsigned)w->display_used,(long long)w->display_time,w->display_capture_ms,
            w->slot,w->frame,w->oldframe,w->backlerp,(unsigned)w->emission,(unsigned)w->idle,
            w->recoil[0],w->recoil[1],w->recoil[2],
            m[0],m[1],m[2],m[4],m[5],m[6],m[8],m[9],m[10],m[12],m[13],m[14]);
        Com_Printf("%s\n",line); // At most 1536 emitted bytes per row, including newline.
    }
}
static bool b2wj_capture(void)
{
    // Default 0: no copy, time query, formatting or gameplay callback.
    if (!b2_weapon_trace || b2_weapon_trace->integer==0 || b2wj_dumped) return false;
    if (b2_weapon_trace->integer==2) { b2wj_dump();return false; }
    return b2_weapon_trace->integer==1 && b2wj_count<B2WJ_LIMIT;
}
extern bool B2_HeadsetWaiting(void);
extern bool B2_ReferenceRecoveryEnabled(void);

#define B2_XR_FUNCTIONS(X) \
    X(xrDestroyInstance) X(xrGetInstanceProperties) X(xrGetSystem) \
    X(xrGetSystemProperties) X(xrEnumerateViewConfigurations) \
    X(xrEnumerateViewConfigurationViews) X(xrEnumerateEnvironmentBlendModes) \
    X(xrGetVulkanGraphicsRequirements2KHR) X(xrCreateVulkanInstanceKHR) \
    X(xrGetVulkanGraphicsDevice2KHR) X(xrCreateVulkanDeviceKHR) \
    X(xrCreateSession) X(xrDestroySession) X(xrBeginSession) X(xrEndSession) \
    X(xrRequestExitSession) X(xrPollEvent) X(xrEnumerateReferenceSpaces) \
    X(xrCreateReferenceSpace) X(xrDestroySpace) X(xrEnumerateSwapchainFormats) \
    X(xrCreateSwapchain) X(xrDestroySwapchain) X(xrEnumerateSwapchainImages) \
    X(xrAcquireSwapchainImage) X(xrWaitSwapchainImage) X(xrReleaseSwapchainImage) \
    X(xrWaitFrame) X(xrBeginFrame) X(xrLocateViews) X(xrEndFrame)

typedef struct b2_xr_eye_s {
    XrSwapchain swapchain;
    XrSwapchainImageVulkan2KHR *images;
    uint32_t count, index, width, height, allocation_width, allocation_height;
    bool acquired, waited, copied, released;
} b2_xr_eye_t;

static struct {
    struct {
        uint64_t serial;
        uint32_t generation,input_generation,time;
        char label[2][B2_RESUME_LABEL261];
    } resume_cue;
    void *loader;
    PFN_xrGetInstanceProcAddr get_proc;
#define B2_DECLARE(name) PFN_##name name;
    B2_XR_FUNCTIONS(B2_DECLARE)
#undef B2_DECLARE
    XrInstance instance;
    XrSystemId system;
    XrSession session;
    XrSpace space, rendering_gaze_space, replacement_space, reference_pending_space;
    XrPosef reference_offset, reference_pending_offset;
    XrTime reference_last_change, reference_pending_time;
    bool reference_realign391;
    uint32_t reference_generation, reference_pending_generation;
    PFN_xrLocateSpace xrLocateSpace;
    XrSessionState session_state;
    XrView views[B2_MAX_VIEWS];
    XrTime display_time, last_display_time, haptic_time, setup_reference_time;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue queue;
    VkFormat format;
    b2_xr_eye_t eyes[B2_MAX_VIEWS];
    b2_frame_t *frame;
    uint32_t generation, input_generation, interruption_epoch, view_count, max_width, max_height;
    uint32_t controller_head_ms, controller_head_generation;
    XrViewConfigurationType configuration;
    uint64_t begin_ticks;
    bool running, failed, loss_pending, frame_open, focused, tracking, exit_requested;
    bool frame_unavailable,reference_refused;
    bool dfr_requested, rendering_gaze_available, dfr_status_known;
    uint32_t mode_width[2][B2_MAX_VIEWS], mode_height[2][B2_MAX_VIEWS];
} xr;
#include "../../../quiet-teleport446/source/b2_graphics_xr270.inc"

uint32_t B2XR_InputGeneration(void) { return xr.input_generation; }

uint32_t B2XR_ActionGeneration594(void)
{
    // Profile events can advance this while the client's last snapshots stay
    // unchanged. Never use a cached snapshot as proof of the live contract.
    return b2_actions.input_enabled && b2_actions.attached && !b2_actions.exhausted &&
        b2_actions.generation!=UINT32_MAX ? b2_actions.generation : 0;
}

bool B2XR_Requested(void)
{
    return Cvar_Get("b2_openxr", "0", CVAR_NOSET | CVAR_NOARCHIVE)->integer != 0;
}

uint32_t B2XR_ViewCount(void)
{
    return xr.view_count ? xr.view_count : B2_EYE_COUNT;
}

void B2XR_InvalidateRenderHistory(const char *reason)
{
    if (++xr.generation == 0)
        xr.generation = 1;
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR history generation=%u reason=%s\n", xr.generation, reason); }
}

// Tracking/action epochs remain independent of a rendering-only teleport reset.
void B2XR_InvalidateHistory(const char *reason)
{
    if (++xr.input_generation == 0)
        xr.input_generation = 1;
    B2XR_InvalidateRenderHistory(reason);
}

static char b2_failure_message555[256];
static bool b2_stopped_notified555;

static const char *b2_xr_failure_text555(XrResult result, const char *reason)
{
    if (result == XR_ERROR_RUNTIME_UNAVAILABLE)
        return "No active OpenXR runtime is available. Start your headset software, then relaunch Q2RTX VR.";
    if (result == XR_ERROR_FORM_FACTOR_UNAVAILABLE)
        return "No VR headset is available. Connect and wake your headset, then relaunch Q2RTX VR.";
    if (result == XR_ERROR_FORM_FACTOR_UNSUPPORTED || result == XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED ||
        result == XR_ERROR_EXTENSION_NOT_PRESENT || result == XR_ERROR_API_VERSION_UNSUPPORTED ||
        (reason && (strstr(reason,"view-configuration") || strstr(reason,"quad-request") ||
         !strcmp(reason,"XR_KHR_vulkan_enable2-required") || !strcmp(reason,"opaque-headset-required") ||
         !strcmp(reason,"stereo-extent-or-six-dof-capability"))))
        return "This headset/runtime does not support the requested VR configuration. Check the selected VR mode and runtime, then relaunch.";
    if (reason && (!strcmp(reason,"local-loader-unavailable") ||
        !strcmp(reason,"loader-missing-xrGetInstanceProcAddr") || !strcmp(reason,"absolute-local-b2_openxr_loader-required")))
        return "The packaged OpenXR loader could not be loaded. Restore the complete Q2RTX VR package, then relaunch.";
    if (reason && !strcmp(reason,"runtime-events-lost-state-unconfirmed"))
        return "Q2RTX VR stopped itself because lost runtime events left the session state uncertain. Close the game window, restart Q2RTX VR and load a saved game. Existing saves are retained; unsaved progress is not recovered.";
    if (result == XR_ERROR_SESSION_LOST || result == XR_ERROR_INSTANCE_LOST || result == XR_SESSION_LOSS_PENDING ||
        (reason && (!strcmp(reason,"instance-loss-pending") || !strcmp(reason,"session-exit-or-loss"))))
        return "The VR session ended or was lost. Close the game window, restore your headset connection and relaunch. Unsaved progress is not recovered.";
    if (result == XR_SUCCESS && xr.running)
        return B2_InternalStopMessage642();
    return xr.running ? "VR stopped after a runtime error. Close the game window and relaunch, then load a normal save. Existing saves are retained." :
        "VR could not start. Check startup-status.txt for details, then relaunch after correcting the reported problem.";
}

static void b2_xr_note_failure555(XrResult result, const char *reason)
{
    if (b2_failure_message555[0]) return;
    Q_strlcpy(b2_failure_message555,b2_xr_failure_text555(result,reason),sizeof(b2_failure_message555));
    B2D_Record(1,"VR_REASON %s",b2_failure_message555);
}

const char *B2XR_FailureMessage555(void)
{
    return b2_failure_message555[0] ? b2_failure_message555 : NULL;
}

void B2XR_NotifyStopped555(void)
{
    if (xr.frame_open || b2_stopped_notified555) return;
    b2_stopped_notified555=true;
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Q2RTX VR stopped",
        B2XR_StartupMessage555(B2_InternalStopMessage642()),NULL);
}

static bool b2_xr_result(XrResult result, const char *operation)
{
    // Positive non-success statuses need their own call-specific handling.
    // In particular, XR_TIMEOUT_EXPIRED never grants image ownership.
    if (result == XR_SUCCESS)
        return true;
    if (result == XR_SESSION_LOSS_PENDING) {
        // This is a SUCCESS status. The caller must first record returned
        // handles/image ownership/frame obligations before terminal closure.
        if (!xr.loss_pending) {
            Com_Printf("B2XR session-loss-pending operation=%s\n", operation);
            B2XR_InvalidateHistory("session-loss-pending");
        }
        b2_xr_note_failure555(result,operation);
        xr.loss_pending = true;
        if (xr.frame)
            xr.frame->should_render = false;
        return true;
    }
    b2_xr_note_failure555(result,operation);
    if (xr.frame) xr.frame->should_render=false;
    B2D_Record(1,"XR_FAILURE operation=%s result=%d",operation,(int)result);
    Com_EPrintf("B2XR failure operation=%s result=%d\n", operation, (int)result);
    if (!xr.failed)
        B2XR_InvalidateHistory(operation);
    xr.failed = true;
    B2XR_RequestStop();
    return false;
}

static bool b2_xr_error(const char *reason)
{
    b2_xr_note_failure555(XR_SUCCESS,reason);
    if (xr.frame) xr.frame->should_render=false;
    B2D_Record(1,"XR_REFUSAL %s",reason);
    Com_EPrintf("B2XR inactive reason=%s\n", reason);
    if (!xr.failed)
        B2XR_InvalidateHistory(reason);
    xr.failed = true;
    B2XR_RequestStop();
    return false;
}

static bool b2_xr_proc(const char *name, PFN_xrVoidFunction *function)
{
    if (!b2_xr_result(xr.get_proc(xr.instance, name, function), name))
        return false;
    return *function != NULL || b2_xr_error(name);
}

static bool b2_xr_select_view_mode(int *quad)
{
    cvar_t *setting = Cvar_FindVar("b2_quad_views");
    if (!setting)
        return b2_xr_error("b2_quad_views-missing-startup-registration");
    int requested = setting->integer;
    bool registered = (setting->flags & CVAR_NOSET) &&
        !(setting->flags & (CVAR_CUSTOM | CVAR_WEAK));
    *quad = Cvar_Get("b2_quad_views", "2", CVAR_NOSET | CVAR_NOARCHIVE)->integer;
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR view-mode requested_setting=%d effective_setting=%d startup_registered=%d\n",
        requested, *quad, (int)registered); }
    if (requested != *quad)
        return b2_xr_error("b2_quad_views-request-effective-mismatch");
    if (!registered)
        return b2_xr_error("b2_quad_views-missing-startup-registration");
    if (*quad < 0 || *quad > 2)
        return b2_xr_error("b2_quad_views-must-be-zero-one-or-auto-two");
    // Resolve the requested setting only after the actual system advertises its views.
    return true;
}

static bool b2_xr_select_foveation_mode(int quad)
{
    cvar_t *setting = Cvar_FindVar("b2_dynamic_foveation");
    if (!setting || !(setting->flags & CVAR_NOSET) ||
        (setting->flags & (CVAR_CUSTOM | CVAR_WEAK)))
        return b2_xr_error("b2_dynamic_foveation-missing-startup-registration");
    int requested = setting->integer;
    int effective = Cvar_Get("b2_dynamic_foveation", "2", CVAR_NOSET | CVAR_NOARCHIVE)->integer;
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2DFR requested=%d startup_effective=%d quad_requested=%d\n", requested, effective, quad); }
    if (requested != effective || effective < 0 || effective > 2)
        return b2_xr_error("b2_dynamic_foveation-invalid-or-request-effective-mismatch");
    // 0: fixed; 1: prefer eye tracking; 2: automatically detect (default).
    // Quad Views remains a separate startup choice. Auto also accepts stereo.
    xr.dfr_requested = effective != 0 && quad;
    return true;
}

static bool b2_xr_resolve_view_mode360(int requested, bool quad_extension)
{
    uint32_t count = 0;
    XrViewConfigurationType configurations[16];
    if (!b2_xr_result(xr.xrEnumerateViewConfigurations(xr.instance, xr.system,
        LENGTH(configurations), &count, configurations), "startup-view-configurations"))
        return false;
    if (xr.loss_pending) return b2_xr_error("initialization-session-loss-pending");
    if (count > LENGTH(configurations)) return b2_xr_error("configuration-count-outside-bounds");
    bool stereo = false, quad = false;
    for (uint32_t i = 0; i < count; ++i) {
        stereo |= configurations[i] == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        quad |= configurations[i] == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO;
    }
    if (!stereo) return b2_xr_error("primary-stereo-view-configuration-required");
    if (requested == 1 && !(quad_extension && quad))
        return b2_xr_error("requested-quad-view-configuration-unavailable");
    bool selected_quad = requested != 0 && quad_extension && quad;
    xr.view_count = selected_quad ? 4u : 2u;
    xr.configuration = selected_quad ? XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO : XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    if (!selected_quad) xr.dfr_requested = false;
    B2D_Record(0,"XR_VIEW_SELECTION requested=%d selected=%s count=%u quad_extension=%d quad_configuration=%d",
        requested, selected_quad ? "quad" : "stereo", xr.view_count, quad_extension, quad);
    return true;
}

bool B2XR_Init(uint32_t vk_api_version)
{
    PFN_xrEnumerateInstanceExtensionProperties enumerate_extensions = NULL;
    PFN_xrCreateInstance create_instance = NULL;
    if (!B2XR_Requested())
        return true;
    if (xr.loader || xr.instance)
        return b2_xr_error("initialization-reentry");
    memset(&xr, 0, sizeof(xr));
    b2_failure_message555[0]=0;b2_stopped_notified555=false;
    b2_weapon_trace=Cvar_Get("b2_weapon_trace", "0", CVAR_PRIVATE | CVAR_NOARCHIVE);
    xr.generation = 1; xr.input_generation = 1;
    int requested_views;
    if (!b2_xr_select_view_mode(&requested_views) || !b2_xr_select_foveation_mode(requested_views != 0))
        return false;

    const char *loader = Cvar_Get("b2_openxr_loader", "",
        CVAR_NOSET | CVAR_NOARCHIVE)->string;
    // The independently qualified launch selects one local loader explicitly.
    if (strlen(loader) < 4 || loader[1] != ':' ||
        (loader[2] != '\\' && loader[2] != '/'))
        return b2_xr_error("absolute-local-b2_openxr_loader-required");
    xr.loader = SDL_LoadObject(loader);
    if (!xr.loader) {
        Com_EPrintf("OpenXR loader: %s\n",SDL_GetError());
        return b2_xr_error("local-loader-unavailable");
    }
    xr.get_proc = (PFN_xrGetInstanceProcAddr)SDL_LoadFunction(xr.loader,
        "xrGetInstanceProcAddr");
    if (!xr.get_proc)
        return b2_xr_error("loader-missing-xrGetInstanceProcAddr");
    if (!b2_xr_proc("xrEnumerateInstanceExtensionProperties",
        (PFN_xrVoidFunction *)&enumerate_extensions) ||
        !b2_xr_proc("xrCreateInstance", (PFN_xrVoidFunction *)&create_instance))
        return false;

    uint32_t count = 0;
    if (!b2_xr_result(enumerate_extensions(NULL, 0, &count, NULL),
        "enumerate-instance-extensions") || count == 0 || count > 4096)
        return b2_xr_error("extension-count-outside-bounds");
    XrExtensionProperties *extensions = calloc(count, sizeof(*extensions));
    if (!extensions)
        return b2_xr_error("extension-allocation");
    for (uint32_t i = 0; i < count; ++i)
        extensions[i].type = XR_TYPE_EXTENSION_PROPERTIES;
    bool ok = b2_xr_result(enumerate_extensions(NULL, count, &count, extensions),
        "enumerate-instance-extensions-data");
    bool enable2 = false, quad_extension = false, foveated_extension = false;
    for (uint32_t i = 0; ok && i < count; ++i)
        enable2 |= !strcmp(extensions[i].extensionName,
            XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME);
    for (uint32_t i = 0; ok && i < count; ++i)
        quad_extension |= !strcmp(extensions[i].extensionName, XR_VARJO_QUAD_VIEWS_EXTENSION_NAME);
    for (uint32_t i = 0; ok && i < count; ++i)
        foveated_extension |= !strcmp(extensions[i].extensionName, XR_VARJO_FOVEATED_RENDERING_EXTENSION_NAME);
    b2_actions.hp_extension316=false;
    for(uint32_t i=0;ok && i<count;i++)
        b2_actions.hp_extension316 |= !strcmp(extensions[i].extensionName,XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME);
    free(extensions);
    if (xr.dfr_requested && !foveated_extension) {
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2DFR reason=foveated-extension-absent eye_tracking_active=0\n");
        xr.dfr_requested = false;
    }
    if (requested_views == 1 && !quad_extension) return b2_xr_error("quad-request-unsupported-XR_VARJO_quad_views-absent");
    int quad = requested_views != 0 && quad_extension;
    if (!quad) xr.dfr_requested = false;
    if (!ok || !enable2)
        return b2_xr_error("XR_KHR_vulkan_enable2-required");

    const char *enabled_extensions[4] = { XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME, XR_VARJO_QUAD_VIEWS_EXTENSION_NAME, XR_VARJO_FOVEATED_RENDERING_EXTENSION_NAME };
    XrInstanceCreateInfo info = { .type = XR_TYPE_INSTANCE_CREATE_INFO };
    Q_strlcpy(info.applicationInfo.applicationName, "Q2RTX VR",
        sizeof(info.applicationInfo.applicationName));
    Q_strlcpy(info.applicationInfo.engineName, "Q2RTX",
        sizeof(info.applicationInfo.engineName));
    info.applicationInfo.applicationVersion = 1;
    info.applicationInfo.engineVersion = 1;
    info.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0);
    info.enabledExtensionCount = xr.dfr_requested ? 3 : quad ? 2 : 1;
    if(b2_actions.hp_extension316)
        enabled_extensions[info.enabledExtensionCount++]=XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME;
    info.enabledExtensionNames = enabled_extensions;
    if (!b2_xr_result(create_instance(&info, &xr.instance), "create-instance"))
        return false;
#define B2_LOAD(name) if (!b2_xr_proc(#name, (PFN_xrVoidFunction *)&xr.name)) return false;
    B2_XR_FUNCTIONS(B2_LOAD)
#undef B2_LOAD

    XrInstanceProperties properties = { .type = XR_TYPE_INSTANCE_PROPERTIES };
    if (!b2_xr_result(xr.xrGetInstanceProperties(xr.instance, &properties),
        "instance-properties"))
        return false;
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR runtime=%s version=%llu loader=%s\n", properties.runtimeName,
        (unsigned long long)properties.runtimeVersion, loader); }
    XrSystemGetInfo system_info = { .type = XR_TYPE_SYSTEM_GET_INFO,
        .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY };
    if (!b2_xr_result(xr.xrGetSystem(xr.instance, &system_info, &xr.system),
        "get-headset-system"))
        return false;
    if (!b2_xr_resolve_view_mode360(requested_views, quad_extension)) return false;
    quad = xr.view_count == 4;
    if (xr.dfr_requested && !b2_xr_proc("xrLocateSpace", (PFN_xrVoidFunction *)&xr.xrLocateSpace))
        return false;
    XrSystemFoveatedRenderingPropertiesVARJO foveated_system = {
        .type = XR_TYPE_SYSTEM_FOVEATED_RENDERING_PROPERTIES_VARJO };
    XrSystemProperties system = { .type = XR_TYPE_SYSTEM_PROPERTIES,
        .next = xr.dfr_requested ? &foveated_system : NULL };
    if (!b2_xr_result(xr.xrGetSystemProperties(xr.instance, xr.system, &system),
        "system-properties"))
        return false;
    if (xr.dfr_requested && !foveated_system.supportsFoveatedRendering) {
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2DFR mode=fixed-quad reason=system-eye-tracking-unsupported eye_tracking_active=0\n");
        xr.dfr_requested = false;
    }
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2DFR extension_enabled=%d system_support=%d mode=%s gaze_status=not-yet-queried\n",
        xr.dfr_requested, (int)foveated_system.supportsFoveatedRendering,
        xr.dfr_requested ? "runtime-foveated-request" : quad ? "fixed-quad" : "two-view"); }
    if (system.graphicsProperties.maxSwapchainImageWidth < B2_WIDTH ||
        system.graphicsProperties.maxSwapchainImageHeight < B2_HEIGHT ||
        system.graphicsProperties.maxLayerCount < 1 ||
        !system.trackingProperties.orientationTracking ||
        !system.trackingProperties.positionTracking)
        return b2_xr_error("stereo-extent-or-six-dof-capability");
    xr.max_width = system.graphicsProperties.maxSwapchainImageWidth;
    xr.max_height = system.graphicsProperties.maxSwapchainImageHeight;
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR system=%s vendor=%u max_extent=%ux%u\n", system.systemName,
        system.vendorId, system.graphicsProperties.maxSwapchainImageWidth,
        system.graphicsProperties.maxSwapchainImageHeight); }

    XrGraphicsRequirementsVulkan2KHR requirements = {
        .type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR };
    if (!b2_xr_result(xr.xrGetVulkanGraphicsRequirements2KHR(xr.instance,
        xr.system, &requirements), "vulkan-graphics-requirements"))
        return false;
    XrVersion version = XR_MAKE_VERSION(VK_VERSION_MAJOR(vk_api_version),
        VK_VERSION_MINOR(vk_api_version), 0);
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR Vulkan requested=%u.%u range=%llu..%llu\n",
        VK_VERSION_MAJOR(vk_api_version), VK_VERSION_MINOR(vk_api_version),
        (unsigned long long)requirements.minApiVersionSupported,
        (unsigned long long)requirements.maxApiVersionSupported); }
    if (version < requirements.minApiVersionSupported ||
        version > requirements.maxApiVersionSupported)
        return b2_xr_error("renderer-Vulkan-version-outside-runtime-range");
    return !xr.loss_pending || b2_xr_error("initialization-session-loss-pending");
}

VkResult B2XR_CreateVulkanInstance(const VkInstanceCreateInfo *info, VkInstance *instance)
{
    if (!B2XR_Requested())
        return vkCreateInstance(info, NULL, instance);
    if (xr.failed || !xr.instance)
        return VK_ERROR_INITIALIZATION_FAILED;
    XrVulkanInstanceCreateInfoKHR create = {
        .type = XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR,
        .systemId = xr.system, .pfnGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vulkanCreateInfo = info };
    VkResult result = VK_ERROR_INITIALIZATION_FAILED;
    if (!b2_xr_result(xr.xrCreateVulkanInstanceKHR(xr.instance, &create, instance,
        &result), "create-vulkan-instance"))
        return VK_ERROR_INITIALIZATION_FAILED;
    if (xr.loss_pending)
        return VK_ERROR_INITIALIZATION_FAILED;
    return result;
}

VkPhysicalDevice B2XR_PhysicalDevice(VkInstance instance)
{
    if (!B2XR_Requested() || xr.failed)
        return VK_NULL_HANDLE;
    XrVulkanGraphicsDeviceGetInfoKHR info = {
        .type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR,
        .systemId = xr.system, .vulkanInstance = instance };
    if (!b2_xr_result(xr.xrGetVulkanGraphicsDevice2KHR(xr.instance, &info,
        &xr.physical_device), "runtime-vulkan-device"))
        return VK_NULL_HANDLE;
    if (xr.loss_pending)
        return VK_NULL_HANDLE;
    return xr.physical_device;
}

VkResult B2XR_CreateVulkanDevice(const VkDeviceCreateInfo *info,
    VkPhysicalDevice physical_device, VkDevice *device)
{
    if (!B2XR_Requested())
        return vkCreateDevice(physical_device, info, NULL, device);
    if (xr.failed || physical_device != xr.physical_device)
        return VK_ERROR_INITIALIZATION_FAILED;
    XrVulkanDeviceCreateInfoKHR create = {
        .type = XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR,
        .systemId = xr.system, .pfnGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vulkanPhysicalDevice = physical_device, .vulkanCreateInfo = info };
    VkResult result = VK_ERROR_INITIALIZATION_FAILED;
    if (!b2_xr_result(xr.xrCreateVulkanDeviceKHR(xr.instance, &create, device,
        &result), "create-vulkan-device"))
        return VK_ERROR_INITIALIZATION_FAILED;
    if (xr.loss_pending)
        return VK_ERROR_INITIALIZATION_FAILED;
    return result;
}

bool B2XR_CreateSession(VkInstance instance, VkPhysicalDevice physical_device,
    VkDevice device, uint32_t queue_family, VkQueue queue)
{
    if (!B2XR_Requested())
        return true;
    if (xr.failed || xr.session || physical_device != xr.physical_device)
        return b2_xr_error("session-binding-or-reentry");
    xr.device = device;
    xr.queue = queue;
    uint32_t count = 0;
    XrViewConfigurationType configurations[16];
    if (!b2_xr_result(xr.xrEnumerateViewConfigurations(xr.instance, xr.system,
        LENGTH(configurations), &count, configurations), "view-configurations"))
        return false;
    if (count > LENGTH(configurations)) return b2_xr_error("configuration-count-outside-bounds");
    bool selected = false;
    for (uint32_t i = 0; i < count; ++i) selected |= configurations[i] == xr.configuration;
    if (!selected) return b2_xr_error("requested-view-configuration-unavailable");
    int runtime,percent;
    if(!b2g_settings364(&runtime,&percent))return b2_xr_error("render-setting-invalid");
    XrViewConfigurationView modes_views[2][B2_MAX_VIEWS] = {0};
    XrFoveatedViewConfigurationViewVARJO foveated_views[B2_MAX_VIEWS] = {0};
    unsigned mode_count = xr.dfr_requested ? 2u : 1u;
    for (unsigned mode = 0; mode < mode_count; ++mode) {
        XrViewConfigurationView *views = modes_views[mode];
        for (uint32_t i = 0; i < xr.view_count; ++i) {
            views[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
            if (xr.dfr_requested) {
                foveated_views[i].type = XR_TYPE_FOVEATED_VIEW_CONFIGURATION_VIEW_VARJO;
                foveated_views[i].foveatedRenderingActive = mode ? XR_TRUE : XR_FALSE;
                views[i].next = &foveated_views[i];
            }
        }
        if (!b2_xr_result(xr.xrEnumerateViewConfigurationViews(xr.instance, xr.system,
            xr.configuration, xr.view_count, &count, views), "selected-view-properties") || count != xr.view_count)
            return b2_xr_error("selected-view-count-mismatch");
        b2_view_extent_t extents[B2_MAX_VIEWS];
        for (uint32_t i = 0; i < xr.view_count; ++i)
            extents[i] = (b2_view_extent_t){ views[i].recommendedImageRectWidth, views[i].recommendedImageRectHeight,
                views[i].maxImageRectWidth, views[i].maxImageRectHeight, views[i].maxSwapchainSampleCount };
        uint32_t envelope_width, envelope_height;
        if (!B2_SelectViewExtents(xr.view_count == 4, runtime != 0, count, extents, xr.max_width, xr.max_height,
            &envelope_width, &envelope_height)) return b2_xr_error("selected-view-extents-exceed-runtime-or-signed-offset-limits");
        for (uint32_t i = 0; i < xr.view_count; ++i) {
            xr.mode_width[mode][i] = (xr.view_count == 4 || runtime) ? views[i].recommendedImageRectWidth : B2_WIDTH;
            xr.mode_height[mode][i] = (xr.view_count == 4 || runtime) ? views[i].recommendedImageRectHeight : B2_HEIGHT;
            B2D_Record(0,"XR_VIEW mode=%u view=%u recommended=%ux%u maximum=%ux%u",mode,i,views[i].recommendedImageRectWidth,views[i].recommendedImageRectHeight,views[i].maxImageRectWidth,views[i].maxImageRectHeight);
            { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR view=%u physical_eye=%u inset=%u output=%ux%u internal=pending full_coverage=1 foveated=%u\n",
                i, B2_PhysicalEye(i), i >= 2, xr.mode_width[mode][i], xr.mode_height[mode][i], mode); }
        }
    }
    for (uint32_t i = 0; i < xr.view_count; ++i) {
        b2_xr_eye_t *target = &xr.eyes[i];
        target->width = xr.mode_width[0][i]; target->height = xr.mode_height[0][i];
        target->allocation_width = max(target->width, xr.mode_width[1][i]);
        target->allocation_height = max(target->height, xr.mode_height[1][i]);
        for (unsigned mode = 0; mode < mode_count; ++mode)
            if (target->allocation_width > modes_views[mode][i].maxImageRectWidth ||
                target->allocation_height > modes_views[mode][i].maxImageRectHeight)
                return b2_xr_error("fixed-foveated-shared-swapchain-envelope-unsupported");
    }
    if (!b2g_select(physical_device)) return b2_xr_error("render-scale-or-device-limit-invalid-restart-required");
    XrEnvironmentBlendMode modes[16];
    if (!b2_xr_result(xr.xrEnumerateEnvironmentBlendModes(xr.instance, xr.system,
        xr.configuration, LENGTH(modes), &count, modes),
        "environment-blend-modes"))
        return false;
    if (count > LENGTH(modes)) return b2_xr_error("blend-mode-count-outside-bounds");
    bool opaque = false;
    for (uint32_t i = 0; i < count; ++i)
        opaque |= modes[i] == XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    if (!opaque)
        return b2_xr_error("opaque-headset-required");

    XrGraphicsBindingVulkan2KHR binding = { .type = XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR,
        .instance = instance, .physicalDevice = physical_device, .device = device,
        .queueFamilyIndex = queue_family, .queueIndex = 0 };
    XrSessionCreateInfo info = { .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = &binding, .systemId = xr.system };
    if (!b2_xr_result(xr.xrCreateSession(xr.instance, &info, &xr.session), "create-session"))
        return false;
    XrReferenceSpaceType spaces[16];
    if (!b2_xr_result(xr.xrEnumerateReferenceSpaces(xr.session, LENGTH(spaces),
        &count, spaces), "reference-spaces"))
        return false;
    if (count > LENGTH(spaces)) return b2_xr_error("reference-space-count-outside-bounds");
    bool local = false;
    for (uint32_t i = 0; i < count; ++i)
        local |= spaces[i] == XR_REFERENCE_SPACE_TYPE_LOCAL;
    if (!local)
        return b2_xr_error("LOCAL-reference-space-required");
    XrReferenceSpaceCreateInfo space = { .type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
        .referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL,
        .poseInReferenceSpace.orientation.w = 1.f };
    if (!b2_xr_result(xr.xrCreateReferenceSpace(xr.session, &space, &xr.space),
        "create-local-space"))
        return false;
    xr.reference_offset=space.poseInReferenceSpace;
    xr.reference_generation=1;

    cvar_t *controllers=Cvar_FindVar("b2_controller_input");
    cvar_t *dominant=Cvar_FindVar("b2_controller_dominant_hand");
    bool input=controllers && controllers->integer==1 && dominant &&
        (dominant->integer==0 || dominant->integer==1);
    // A single attachment also supports damage-only output when KBM owns gameplay.
    bool actions_ready=b2ca_setup_mode(&b2_actions,true,input,xr.instance,xr.session,xr.get_proc);
    if (input && b2_actions.cleanup_failed) return b2_xr_error("controller-partial-cleanup");
    if ((input && !actions_ready) || Cvar_VariableInteger("developer") > 0) Com_Printf("B2_CONTROLLER_SETUP ready=%d touch_suggested=%d index_suggested=%d initial_owner=keyboard-mouse\n",
        input && actions_ready,b2_actions.suggested[0],b2_actions.suggested[1]);
    if (!actions_ready || b2_actions.haptic == XR_NULL_HANDLE || b2_actions.cleanup_failed || Cvar_VariableInteger("developer") > 0) Com_Printf("B2_HAPTIC_SETUP ready=%d input_enabled=%d optional=1 cleanup_failed=%d\n",
        actions_ready && b2_actions.haptic!=XR_NULL_HANDLE,input,b2_actions.cleanup_failed);
    if (xr.dfr_requested) {
        bool rendering_gaze = false;
        for (uint32_t i = 0; i < count; ++i)
            rendering_gaze |= spaces[i] == XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO;
        if (rendering_gaze) {
            space.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO;
            if (!b2_xr_result(xr.xrCreateReferenceSpace(xr.session, &space, &xr.rendering_gaze_space),
                "create-rendering-gaze-status-space"))
                return false;
        } else {
            // This option promises tracked eye use only with observed validity.
            // Keep the already allocated fixed viewport when status is unavailable.
            xr.dfr_requested = false;
            if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2DFR mode=fixed-quad reason=combined-eye-space-absent eye_tracking_active=0\n");
        }
    }

    int64_t formats[256];
    if (!b2_xr_result(xr.xrEnumerateSwapchainFormats(xr.session, LENGTH(formats),
        &count, formats), "swapchain-formats"))
        return false;
    VkFormatProperties source_properties;
    vkGetPhysicalDeviceFormatProperties(physical_device, VK_FORMAT_R16G16B16A16_SFLOAT,
        &source_properties);
    if (!(source_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
        return b2_xr_error("RGBA16F-blit-source-unsupported");
    for (uint32_t i = 0; i < count; ++i) {
        if (formats[i] != VK_FORMAT_R8G8B8A8_SRGB && formats[i] != VK_FORMAT_B8G8R8A8_SRGB)
            continue;
        VkFormatProperties destination;
        vkGetPhysicalDeviceFormatProperties(physical_device, (VkFormat)formats[i], &destination);
        if (destination.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) {
            xr.format = (VkFormat)formats[i];
            break;
        }
    }
    if (xr.format == VK_FORMAT_UNDEFINED)
        return b2_xr_error("sRGB-blit-swapchain-format-required");
    for (uint32_t eye = 0; eye < xr.view_count; ++eye) {
        b2_xr_eye_t *target = &xr.eyes[eye];
        XrSwapchainCreateInfo swapchain = { .type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
            .usageFlags = XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
            .format = xr.format, .sampleCount = 1, .width = target->allocation_width, .height = target->allocation_height,
            .faceCount = 1, .arraySize = 1, .mipCount = 1 };
        if (!b2_xr_result(xr.xrCreateSwapchain(xr.session, &swapchain, &target->swapchain),
            "create-eye-swapchain") ||
            !b2_xr_result(xr.xrEnumerateSwapchainImages(target->swapchain, 0,
                &target->count, NULL), "eye-image-count"))
            return false;
        if (target->count == 0 || target->count > 32)
            return b2_xr_error("eye-image-count-outside-bounds");
        target->images = calloc(target->count, sizeof(*target->images));
        if (!target->images)
            return b2_xr_error("eye-image-allocation");
        for (uint32_t i = 0; i < target->count; ++i)
            target->images[i].type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR;
        if (!b2_xr_result(xr.xrEnumerateSwapchainImages(target->swapchain, target->count,
            &target->count, (XrSwapchainImageBaseHeader *)target->images), "eye-images"))
            return false;
        { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR eye=%u swapchain_images=%u format=%d\n", eye, target->count, (int)xr.format); }
    }
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR session-created graphics_family=%u serial_queue_completion=1\n", queue_family); }
    return !xr.loss_pending || b2_xr_error("session-creation-loss-pending");
}

void B2XR_HapticStop(void)
{
    b2_actions.haptic_focused582 = xr.running && xr.focused && !xr.failed &&
        !xr.exit_requested && !xr.loss_pending;
    b2ha_stop(&b2_actions);
}
#include "../../../../baseline/b2-menu-bindings316/source/b2_haptic_output308.h"
static void b2_xr_haptic_update(void)
{
    if (xr.display_time<=0 || xr.display_time<=xr.haptic_time) return;
    xr.haptic_time=xr.display_time;
    b2_actions.haptic_focused582 = xr.running && xr.focused && !xr.failed &&
        !xr.exit_requested && !xr.loss_pending;
    if(B2H_TakeStop316())B2XR_HapticStop();
    if(!B2H_Enabled316() || Cvar_VariableInteger("b2_input_mode") != 1) {
        b2_actions.haptic_rearm582 = true; B2H_Discard582(); return;
    }
    if (!B2XR_InteractionReady()) {
        b2_actions.haptic_rearm582 = true; B2H_Discard582();
    } else if (!b2ha_prepare(&b2_actions)) {
        B2H_Discard582();
    } else {
        float amplitude[2];uint32_t duration[2];
        B2H_Output308(Sys_Milliseconds(),amplitude,duration);
        b2ha_deliver(&b2_actions,amplitude,duration);
        if (b2_actions.haptic_failed) B2H_Discard582();
    }
    // Bounded logical-update diagnostics, outside the eye loop.
    uint32_t now=Sys_Milliseconds();
    unsigned state=(b2_actions.haptic!=XR_NULL_HANDLE)|(b2_actions.attached<<1)|
        (b2_actions.haptic_available[0]<<2)|(b2_actions.haptic_available[1]<<3)|
        (b2_actions.haptic_active[0]<<4)|(b2_actions.haptic_active[1]<<5)|
        (b2_actions.haptic_failed<<6);
    static unsigned previous=UINT_MAX,records;static uint32_t logged;
    static uint64_t seen;
    uint64_t attempts=b2_actions.haptic_apply_attempts[0]+b2_actions.haptic_apply_attempts[1];
    if (records<64 && (!records || ((uint32_t)(now-logged)>=1000 &&
        (state!=previous || (seen!=attempts && (uint32_t)(now-logged)>=5000))))) {
        previous=state;logged=now;seen=attempts;records++;
        if ((Cvar_VariableInteger("developer") > 0) || b2_actions.haptic_failed) Com_Printf("B2_HAPTIC_OUTPUT state=%u apply=%llu,%llu accepted=%llu,%llu stop=%llu,%llu stopped=%llu,%llu last_result=%d,%d errors=%u record=%u/64\n",
            state,(unsigned long long)b2_actions.haptic_apply_attempts[0],
            (unsigned long long)b2_actions.haptic_apply_attempts[1],
            (unsigned long long)b2_actions.haptic_apply_success[0],
            (unsigned long long)b2_actions.haptic_apply_success[1],
            (unsigned long long)b2_actions.haptic_stop_attempts[0],
            (unsigned long long)b2_actions.haptic_stop_attempts[1],
            (unsigned long long)b2_actions.haptic_stop_success[0],
            (unsigned long long)b2_actions.haptic_stop_success[1],
            b2_actions.haptic_last_result[0],b2_actions.haptic_last_result[1],
            b2_actions.haptic_errors,records);
    }
    if (b2_actions.haptic_errors!=b2_actions.haptic_reported) {
        b2_actions.haptic_reported=b2_actions.haptic_errors;
        Com_WPrintf("B2_HAPTIC optional output error_mask=%u disabled=%d\n",
            b2_actions.haptic_errors,b2_actions.haptic_failed);
    }
}

static bool b2_xr_setup_reference(const XrEventDataReferenceSpaceChangePending *change)
{
    if (!B2_HeadsetWaiting() || change->changeTime<=0 ||
        (change->poseValid!=XR_TRUE && change->poseValid!=XR_FALSE)) return false;
    if (change->poseValid) {
        const XrPosef *p=&change->poseInPreviousSpace;
        float q[4]={p->orientation.x,p->orientation.y,p->orientation.z,p->orientation.w},norm=0;
        if (!isfinite(p->position.x) || !isfinite(p->position.y) || !isfinite(p->position.z)) return false;
        for (unsigned i=0;i<4;i++) { if (!isfinite(q[i])) return false; norm+=q[i]*q[i]; }
        if (!isfinite(norm) || fabsf(norm-1.f)>.02f) return false;
    }
    // No pre-play anchor is retained, so an unavailable previous-space
    // transform needs no invented replacement. Wait until its effective time.
    if (change->changeTime>xr.setup_reference_time) xr.setup_reference_time=change->changeTime;
    xr.controller_head_generation=0;xr.tracking=false;
    if (++xr.interruption_epoch==0) xr.interruption_epoch=1;
    B2H_Reset();
    Com_Printf("B2XR setup-reference waiting effective_ns=%lld previous_pose_valid=%d\n",
        (long long)xr.setup_reference_time,change->poseValid);
    return true;
}

// Keep the application LOCAL coordinates continuous. OpenXR supplies the new
// LOCAL origin in the previous LOCAL space: newOffset = inverse(change)*oldOffset.
// LOCAL is gravity-aligned; a tilted, unbounded or non-rigid transform is refused.
// Locally authored GPL-2.0-or-later algebra; no runtime implementation is imported.
static bool b2_xr_continuity_offset(const XrPosef *old, const XrPosef *change, XrPosef *out)
{
    const XrPosef *poses[2]={old,change};
    for (unsigned i=0;i<2;i++) {
        const XrPosef *p=poses[i];
        const float q[4]={p->orientation.x,p->orientation.y,p->orientation.z,p->orientation.w};
        const float v[3]={p->position.x,p->position.y,p->position.z};
        float norm=0;
        for (unsigned j=0;j<3;j++) if (!isfinite(v[j]) || fabsf(v[j])>10.f) return false;
        for (unsigned j=0;j<4;j++) { if (!isfinite(q[j])) return false; norm+=q[j]*q[j]; }
        if (!isfinite(norm) || fabsf(norm-1.f)>.0001f ||
            fabsf(q[0])>.00001f || fabsf(q[2])>.00001f) return false;
    }
    float n=sqrtf(change->orientation.y*change->orientation.y+change->orientation.w*change->orientation.w);
    float y=change->orientation.y/n,w=change->orientation.w/n;
    float c=w*w-y*y,s=2.f*y*w;
    float dx=old->position.x-change->position.x,dz=old->position.z-change->position.z;
    XrPosef next={.orientation={0,w*old->orientation.y-y*old->orientation.w,0,
        w*old->orientation.w+y*old->orientation.y},
        .position={c*dx-s*dz,old->position.y-change->position.y,s*dx+c*dz}};
    n=sqrtf(next.orientation.y*next.orientation.y+next.orientation.w*next.orientation.w);
    if (!isfinite(n) || n<.999f || n>1.001f ||
        !isfinite(next.position.x) || !isfinite(next.position.y) || !isfinite(next.position.z) ||
        fabsf(next.position.x)>10.f || fabsf(next.position.y)>10.f || fabsf(next.position.z)>10.f) return false;
    next.orientation.y/=n;next.orientation.w/=n;
    *out=next;return true;
}

static bool b2_xr_recover_reference(const XrEventDataReferenceSpaceChangePending *change)
{
    XrPosef next;
    if (xr.reference_realign391 || !B2_ReferenceRecoveryEnabled() || xr.failed || xr.loss_pending || xr.exit_requested ||
        xr.frame_open || !xr.space || !xr.reference_generation || xr.reference_generation==UINT32_MAX ||
        change->session!=xr.session || change->referenceSpaceType!=XR_REFERENCE_SPACE_TYPE_LOCAL ||
        change->poseValid!=XR_TRUE || change->changeTime<=0 ||
        change->changeTime<=xr.last_display_time || change->changeTime<=xr.reference_last_change ||
        xr.reference_pending_time || xr.replacement_space ||
        !b2_xr_continuity_offset(&xr.reference_offset,&change->poseInPreviousSpace,&next)) return false;
    xr.reference_realign391=true;
    xr.reference_pending_offset=next;xr.reference_pending_time=change->changeTime;
    xr.reference_pending_space=xr.space;xr.reference_pending_generation=xr.reference_generation;
    xr.controller_head_generation=0;xr.tracking=false;
    if (++xr.interruption_epoch==0) xr.interruption_epoch=1;
    B2H_Reset();
    Com_Printf("B2XR reference-recovery queued effective_ns=%lld basis=%u continuity=validated pause=required\n",
        (long long)xr.reference_pending_time,xr.reference_generation);
    return true;
}


// An explicit fresh basis is a separate operation, not continuity algebra.
static bool b2_xr_queue_realignment391(const XrEventDataReferenceSpaceChangePending *change)
{
    if(!B2_ReferenceBasisAllowed397() || xr.failed || xr.loss_pending || xr.exit_requested ||
        xr.reference_refused || xr.frame_open || !xr.space || !xr.running ||
        !xr.reference_generation || xr.reference_generation==UINT32_MAX || xr.replacement_space ||
        change->session!=xr.session || change->referenceSpaceType!=XR_REFERENCE_SPACE_TYPE_LOCAL ||
        change->changeTime<=0 || (change->poseValid!=XR_FALSE && change->poseValid!=XR_TRUE))return false;
    for(unsigned i=0;i<xr.view_count;i++)if(xr.eyes[i].acquired)return false;
    // A malformed declared-valid pose is still an error. Undefined poses are
    // never inspected. A valid but late event needs explicit fresh alignment.
    if(change->poseValid==XR_TRUE){
        XrPosef checked;
        if(!b2_xr_continuity_offset(&xr.reference_offset,&change->poseInPreviousSpace,&checked))return false;
    }
    xr.reference_realign391=true;
    xr.reference_pending_offset=(XrPosef){.orientation.w=1};
    if(change->changeTime>xr.reference_pending_time)xr.reference_pending_time=change->changeTime;
    xr.reference_pending_space=xr.space;xr.reference_pending_generation=xr.reference_generation;
    xr.controller_head_generation=0;xr.tracking=false;
    if(++xr.interruption_epoch==0)xr.interruption_epoch=1;
    B2H_Reset();
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REALIGN queued effective_ns=%lld old_transform_used=0 previous_pose_valid=%d explicit_alignment=required\n",
        (long long)xr.reference_pending_time,change->poseValid==XR_TRUE);
    return true;
}
bool B2XR_RealignPending391(void){return xr.reference_realign391;}
bool B2XR_ReferenceApplied397(void){return xr.reference_realign391 && !xr.reference_pending_time;}
bool B2XR_CompleteRealign391(uint32_t generation)
{
    if(!xr.reference_realign391 || xr.reference_pending_time || xr.frame_open ||
        !generation || generation!=xr.input_generation || !xr.tracking || !xr.focused ||
        !xr.running || xr.failed || xr.loss_pending || xr.exit_requested || xr.reference_refused)return false;
    for(unsigned i=0;i<xr.view_count;i++)if(xr.eyes[i].acquired)return false;
    xr.reference_realign391=false;xr.controller_head_generation=0;
    B2XR_InvalidateHistory("explicit-realignment-committed");
    return true;
}

// Called after a fresh wait/begin and before any views/actions/images are acquired.
// The old reference remains owned until successful destruction; a replacement
// remains tracked for terminal cleanup even if creation/destroy reports loss.
static bool b2_xr_apply_reference(void)
{
    if (!xr.reference_pending_time) return true;
    if (xr.display_time<xr.reference_pending_time || !xr.reference_pending_space ||
        xr.space!=xr.reference_pending_space || xr.reference_generation!=xr.reference_pending_generation ||
        xr.replacement_space || (xr.reference_realign391 ? !B2_ReferenceBasisAllowed397() : !B2_ReferenceRecoveryEnabled())) return b2_xr_error("reference-recovery-generation");
    XrReferenceSpaceCreateInfo info={.type=XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
        .referenceSpaceType=XR_REFERENCE_SPACE_TYPE_LOCAL,.poseInReferenceSpace=xr.reference_pending_offset};
    if (!b2_xr_result(xr.xrCreateReferenceSpace(xr.session,&info,&xr.replacement_space),"create-continuous-local-space") ||
        xr.loss_pending) return false;
    if (!xr.replacement_space) return b2_xr_error("reference-replacement-handle");
    if (!b2_xr_result(xr.xrDestroySpace(xr.space),"destroy-prior-local-space")) return false;
    xr.space=xr.replacement_space;xr.replacement_space=XR_NULL_HANDLE;
    xr.reference_offset=xr.reference_pending_offset;
    xr.reference_last_change=xr.reference_pending_time;
    xr.reference_pending_time=0;xr.reference_pending_space=XR_NULL_HANDLE;
    ++xr.reference_generation;xr.controller_head_generation=0;xr.tracking=false;
    B2XR_InvalidateHistory(xr.reference_realign391 ? "fresh-reference-awaiting-alignment" : "reference-continuity-established");
    if(xr.reference_realign391){
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2_REALIGN fresh-space applied basis=%u alignment_pending=1 gameplay_held=1\n",xr.reference_generation);
        return !xr.loss_pending;
    }
    Com_Printf("B2XR reference-recovery applied effective_ns=%lld basis=%u body_unchanged=1 deliberate_resume=required LOCAL_relative=1\n",
        (long long)xr.reference_last_change,xr.reference_generation);
    return !xr.loss_pending;
}


// Invalid/ambiguous continuity remains a terminal gameplay boundary. Keep only
// a zero-layer tracked action pump so an operator can deliberately quit in VR.
// Fatal/device/session/instance/operator stops still use the ordinary exit path.
static bool b2_xr_refuse_reference(const XrEventDataReferenceSpaceChangePending *change)
{
    XrPosef unused;
    int transform=change->poseValid==XR_TRUE ? (int)b2_xr_continuity_offset(&xr.reference_offset,&change->poseInPreviousSpace,&unused) : -1;
    Com_Printf("B2XR reference-refused pose_valid=%d transform_checked_result=%d effective_ns=%lld last_display_ns=%lld last_change_ns=%lld pending_ns=%lld basis=%u frame_open=%d recovery_enabled=%d restart_required=1\n",
        change->poseValid==XR_TRUE,transform,(long long)change->changeTime,
        (long long)xr.last_display_time,(long long)xr.reference_last_change,
        (long long)xr.reference_pending_time,xr.reference_generation,xr.frame_open,B2_ReferenceRecoveryEnabled());
    if(xr.failed || xr.loss_pending || xr.exit_requested || !xr.running ||
       !b2_actions.input_enabled || !b2_actions.attached)
        return b2_xr_error("reference-space-changed-restart-required");
    xr.reference_refused=true;xr.controller_head_generation=0;xr.tracking=false;
    B2H_Reset();B2XR_InvalidateHistory("reference-continuity-refused-quit-only");
    if(++xr.interruption_epoch==0)xr.interruption_epoch=1;
    Com_Printf("B2XR terminal continuity refusal: release controls, then hold both dominant-hand face buttons for 1.2 seconds to quit. No resume/gameplay permitted. Close the game window to quit.\n");
    return true;
}

static bool b2_xr_poll(void)
{
    XrEventDataBuffer event = { .type = XR_TYPE_EVENT_DATA_BUFFER };
    XrResult result;
    bool uncertain=false;
    while ((result = xr.xrPollEvent(xr.instance, &event)) == XR_SUCCESS) {
        switch (event.type) {
        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
            memset(b2_actions.menu_active316,0,sizeof(b2_actions.menu_active316));
            if(b2_actions.generation==UINT32_MAX)b2_actions.exhausted=true;
            else ++b2_actions.generation;
            break;
        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            return b2_xr_error("instance-loss-pending");
        case XR_TYPE_EVENT_DATA_EVENTS_LOST:
            // Queue overflow is not a session-state query. Keep draining until
            // an actual subsequent session transition establishes current state.
            uncertain=true;xr.focused=false;xr.tracking=false;
            xr.controller_head_ms=0;B2H_Reset();
            if (++xr.interruption_epoch==0) xr.interruption_epoch=1;
            B2XR_InvalidateHistory("runtime-events-lost");
            break;
        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
            const XrEventDataReferenceSpaceChangePending *change = (const void *)&event;
            if (change->session == xr.session && change->referenceSpaceType == XR_REFERENCE_SPACE_TYPE_LOCAL) {
                // Capture eligibility before this event invalidates tracking/actions.
                bool setup=B2_HeadsetWaiting();
                bool stable=xr.focused && xr.tracking && xr.running && !xr.reference_realign391;
                uint32_t before=xr.interruption_epoch;
                B2XR_InvalidateHistory("reference-space-change-pending");
                bool accepted=setup ? b2_xr_setup_reference(change) : b2_xr_recover_reference(change);
                bool known=accepted && !setup;
                if(!accepted && !setup)accepted=b2_xr_queue_realignment391(change);
                if(accepted && !setup)B2_ReferenceReset397(stable,known,before);
                if (!accepted)
                    if(!b2_xr_refuse_reference(change))return false;
            }
            break;
        }
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
            const XrEventDataSessionStateChanged *change = (const void *)&event;
            if (change->session != xr.session)
                break;
            { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR session-state old=%d new=%d time_ns=%lld\n",
                (int)xr.session_state, (int)change->state, (long long)change->time); }
            if (uncertain && ((xr.running && (change->state==XR_SESSION_STATE_READY ||
                change->state==XR_SESSION_STATE_IDLE)) || (!xr.running &&
                (change->state==XR_SESSION_STATE_SYNCHRONIZED || change->state==XR_SESSION_STATE_VISIBLE ||
                 change->state==XR_SESSION_STATE_FOCUSED))))
                return b2_xr_error("runtime-events-lost-state-unconfirmed");
            uncertain=false;
            xr.session_state = change->state;
            bool focused = change->state == XR_SESSION_STATE_FOCUSED;
            if (focused != xr.focused) {
                xr.focused = focused;
                if (!focused) B2H_Reset();
                if (!focused && ++xr.interruption_epoch == 0) xr.interruption_epoch = 1;
                B2XR_InvalidateHistory(focused ? "focus-regained" : "focus-lost");
            }
            if (change->state == XR_SESSION_STATE_STOPPING && xr.running) {
                B2H_Reset();
                if (!b2_xr_result(xr.xrEndSession(xr.session), "end-session"))
                    return false;
                xr.running = false;
                B2XR_InvalidateHistory("session-stopped");
            } else if (change->state == XR_SESSION_STATE_LOSS_PENDING ||
                change->state == XR_SESSION_STATE_EXITING) {
                return b2_xr_error("session-exit-or-loss");
            }
            break;
        }
        default: break;
        }
        memset(&event, 0, sizeof(event));
        event.type = XR_TYPE_EVENT_DATA_BUFFER;
    }
    if (result != XR_EVENT_UNAVAILABLE && !b2_xr_result(result,"poll-events")) return false;
    if (uncertain) return b2_xr_error("runtime-events-lost-state-unconfirmed");
    return true;
}

void B2XR_PollEvents(void)
{
    if (xr.instance && xr.session && !xr.frame_open && xr.xrPollEvent)
        b2_xr_poll();
    if (xr.loss_pending && !xr.frame_open)
        B2XR_RequestStop();
}

static bool b2_xr_begin_frame(b2_frame_t *frame)
{
    if (!B2XR_Requested() || !xr.session || xr.failed || !frame)
        return false;
    if (xr.frame_open)
        return b2_xr_error("frame-begin-reentry");
    if (!b2_xr_poll() || xr.exit_requested || (xr.reference_refused && !xr.running))
        return false;
    // Wait for the client to freeze a scene before beginning the session.
    // READY may have arrived during the inactive event pump.
    if (xr.session_state == XR_SESSION_STATE_READY && !xr.running) {
        XrSessionBeginInfo session_begin = { .type = XR_TYPE_SESSION_BEGIN_INFO,
            .primaryViewConfigurationType = xr.configuration };
        if (!b2_xr_result(xr.xrBeginSession(xr.session, &session_begin), "begin-session"))
            return false;
        xr.running = true;
        B2XR_InvalidateHistory("session-begin");
    }
    if (!xr.running)
        return false;
    memset(frame, 0, sizeof(*frame));
    frame->view_count = xr.view_count;
    for (uint32_t i = 0; i < xr.view_count; ++i) {
        frame->output_width[i] = xr.eyes[i].width;
        frame->output_height[i] = xr.eyes[i].height;
    }
    xr.begin_ticks = SDL_GetPerformanceCounter();
    XrFrameWaitInfo wait = { .type = XR_TYPE_FRAME_WAIT_INFO };
    XrFrameState state = { .type = XR_TYPE_FRAME_STATE };
    if (!b2_xr_result(xr.xrWaitFrame(xr.session, &wait, &state), "wait-frame"))
        return false;
    XrFrameBeginInfo begin = { .type = XR_TYPE_FRAME_BEGIN_INFO };
    XrResult begin_result = xr.xrBeginFrame(xr.session, &begin);
    // XR_FRAME_DISCARDED is a successful begin with a discarded predecessor.
    if (begin_result != XR_FRAME_DISCARDED && !b2_xr_result(begin_result, "begin-frame"))
        return false;
    xr.frame_open = true;
    xr.frame = frame;
    bool fresh_time = state.predictedDisplayTime > xr.last_display_time && state.predictedDisplayPeriod > 0;
    if (fresh_time) xr.last_display_time = state.predictedDisplayTime;
    xr.display_time = state.predictedDisplayTime;
    frame->predicted_time_ns = state.predictedDisplayTime;
    frame->duration_ns = state.predictedDisplayPeriod;
    if (begin_result == XR_FRAME_DISCARDED)
        B2XR_InvalidateHistory("runtime-frame-discarded");
    // VISIBLE sessions still need composition layers even without XR input
    // focus. The runtime's shouldRender controls presentation; focus changes
    // continue to invalidate temporal history in the event handler above.
    frame->should_render = state.shouldRender && !xr.loss_pending && !xr.reference_refused;
    if (xr.loss_pending || (xr.setup_reference_time && xr.display_time<xr.setup_reference_time)) {
        frame->should_render=false;
        frame->history_generation = xr.generation;
        return true; // A successful begin owes a zero-layer end, also during setup.
    }
    xr.setup_reference_time=0;
    if (xr.reference_pending_time && !xr.reference_refused) {
        if (!fresh_time || xr.display_time<xr.reference_pending_time || !b2_xr_apply_reference()) {
            frame->should_render=false;frame->history_generation=xr.generation;
            return true; // Never locate/render a pose in an uncommitted reference.
        }
    }
    for (uint32_t eye = 0; eye < xr.view_count; ++eye) {
        b2_xr_eye_t *target = &xr.eyes[eye];
        // A timed-out acquire remains ours across frame bookkeeping. A waited
        // or copied image here is a different, unretired ownership failure.
        if (target->acquired && (target->waited || target->copied)) {
            b2_xr_error("unretired-eye-acquire");
            frame->should_render = false;
            frame->history_generation = xr.generation;
            return true; // Preserve copied/waited ownership for terminal cleanup.
        }
    }
    for (uint32_t eye = 0; eye < xr.view_count; ++eye) {
        b2_xr_eye_t *target = &xr.eyes[eye];
        if (!target->acquired) target->copied = target->released = false;
        xr.views[eye] = (XrView){ .type = XR_TYPE_VIEW };
    }
    XrViewLocateFoveatedRenderingVARJO foveated_locate = {
        .type = XR_TYPE_VIEW_LOCATE_FOVEATED_RENDERING_VARJO,
        .foveatedRenderingActive = XR_FALSE };
    // Only a current, valid, tracked rendering-gaze sample enables moving insets.
    // The runtime supplies their actual poses/FoVs; no synthetic gaze is invented.
    bool available = false;
    frame->runtime_foveated = false;
    if (xr.dfr_requested && xr.rendering_gaze_space && !xr.reference_refused) {
        XrSpaceLocation gaze_status = { .type = XR_TYPE_SPACE_LOCATION };
        bool status_ok = b2_xr_result(xr.xrLocateSpace(xr.rendering_gaze_space, xr.space,
            xr.display_time, &gaze_status), "locate-rendering-gaze-status");
        const XrSpaceLocationFlags required_gaze = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT |
            XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
        available = status_ok && !xr.loss_pending &&
            (gaze_status.locationFlags & required_gaze) == required_gaze;
        if (!status_ok || xr.loss_pending) {
            frame->should_render = frame->tracking_valid = false;
            frame->history_generation = xr.generation;
            return true; // Runtime failure still owes zero-layer end.
        }
    }
    if (xr.dfr_requested) {
        if (!xr.dfr_status_known || available != xr.rendering_gaze_available) {
            xr.dfr_status_known = true;
            xr.rendering_gaze_available = available;
            B2XR_InvalidateHistory(available ? "rendering-gaze-regained" : "rendering-gaze-lost");
            { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2DFR eye_tracking_active=%d loss_policy=fixed-quad-fallback\n", available); }
        }
        foveated_locate.foveatedRenderingActive = available ? XR_TRUE : XR_FALSE;
        frame->runtime_foveated = available;
        // Gaze loss, refused reference space, and regained tracking select matching
        // extents and history. Both modes were sized before swapchain allocation.
        unsigned mode = foveated_locate.foveatedRenderingActive ? 1u : 0u;
        for (uint32_t i = 0; i < xr.view_count; ++i) {
            xr.eyes[i].width = frame->output_width[i] = xr.mode_width[mode][i];
            xr.eyes[i].height = frame->output_height[i] = xr.mode_height[mode][i];
        }
    }
    XrViewLocateInfo locate = { .type = XR_TYPE_VIEW_LOCATE_INFO,
        .next = xr.dfr_requested ? &foveated_locate : NULL,
        .viewConfigurationType = xr.configuration,
        .displayTime = xr.display_time, .space = xr.space };
    XrViewState view_state = { .type = XR_TYPE_VIEW_STATE };
    uint32_t count = 0;
    bool located = b2_xr_result(xr.xrLocateViews(xr.session, &locate, &view_state,
        xr.view_count, &count, xr.views), "locate-stereo-views");
    const XrViewStateFlags required = XR_VIEW_STATE_ORIENTATION_VALID_BIT |
        XR_VIEW_STATE_POSITION_VALID_BIT | XR_VIEW_STATE_ORIENTATION_TRACKED_BIT |
        XR_VIEW_STATE_POSITION_TRACKED_BIT;
    frame->tracking_valid = fresh_time && located && !xr.loss_pending && count == xr.view_count &&
        (view_state.viewStateFlags & required) == required;
    if (frame->tracking_valid && xr.view_count == 4) {
        for (uint32_t i = 0; i < 2; ++i) {
            const XrPosef *a = &xr.views[i].pose, *b = &xr.views[i + 2].pose;
            float dot = a->orientation.x*b->orientation.x + a->orientation.y*b->orientation.y +
                a->orientation.z*b->orientation.z + a->orientation.w*b->orientation.w;
            if (!isfinite(dot) || fabsf(fabsf(dot)-1.f) > 0.0001f ||
                fabsf(a->position.x-b->position.x) > 0.00001f ||
                fabsf(a->position.y-b->position.y) > 0.00001f ||
                fabsf(a->position.z-b->position.z) > 0.00001f) {
                frame->tracking_valid = false;
                b2_xr_error("quad-same-eye-pose-mismatch");
            }
        }
    }
    if (frame->tracking_valid != xr.tracking) {
        xr.tracking = frame->tracking_valid;
        B2XR_InvalidateHistory(xr.tracking ? "tracking-regained" : "tracking-lost");
    }
    if (frame->tracking_valid) { xr.controller_head_ms=Sys_Milliseconds(); xr.controller_head_generation=xr.input_generation; }
    b2_xr_haptic_update(); // Once for the logical frame, before the eye loop.
    b2_xr_refresh_resume_cue261(); // Names follow the final existing action sync.
    frame->history_generation = xr.generation;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR frame-begin predicted_ns=%lld period_ns=%lld view_flags=%llu focused=%d render=%d runtime_render=%d state=%d\n",
        (long long)frame->predicted_time_ns, (long long)frame->duration_ns,
        (unsigned long long)view_state.viewStateFlags, xr.focused, frame->should_render,
        state.shouldRender, (int)xr.session_state);
    if (!frame->should_render || !frame->tracking_valid || xr.failed)
        return true; // The caller still owes a zero-layer end.
    for (uint32_t eye = 0; eye < xr.view_count; ++eye) {
        const XrView *view = &xr.views[eye];
        if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR pose eye=%u position=%g,%g,%g orientation=%g,%g,%g,%g fov=%g,%g,%g,%g\n",
            eye, view->pose.position.x, view->pose.position.y, view->pose.position.z,
            view->pose.orientation.x, view->pose.orientation.y, view->pose.orientation.z,
            view->pose.orientation.w, view->fov.angleLeft, view->fov.angleRight,
            view->fov.angleUp, view->fov.angleDown);
        frame->positions[eye][0] = view->pose.position.x;
        frame->positions[eye][1] = view->pose.position.y;
        frame->positions[eye][2] = view->pose.position.z;
        frame->orientations[eye][0] = view->pose.orientation.x;
        frame->orientations[eye][1] = view->pose.orientation.y;
        frame->orientations[eye][2] = view->pose.orientation.z;
        frame->orientations[eye][3] = view->pose.orientation.w;
        frame->fov_lrud[eye][0] = view->fov.angleLeft;
        frame->fov_lrud[eye][1] = view->fov.angleRight;
        frame->fov_lrud[eye][2] = view->fov.angleUp;
        frame->fov_lrud[eye][3] = view->fov.angleDown;
        b2_xr_eye_t *target = &xr.eyes[eye];
        if (!target->acquired) {
            XrSwapchainImageAcquireInfo acquire = { .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
            if (!b2_xr_result(xr.xrAcquireSwapchainImage(target->swapchain, &acquire,
                &target->index), "acquire-eye-image")) {
                frame->should_render = false;
                return true;
            }
            target->acquired = true;
            if (xr.loss_pending) {
                frame->should_render=false;
                return true; // Returned acquire recorded; never release unwaited.
            }
        }
        XrSwapchainImageWaitInfo image_wait = { .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
            .timeout = 100000000 }; // 100 ms diagnostic bound, never infinite.
        XrResult result = xr.xrWaitSwapchainImage(target->swapchain, &image_wait);
        if (result == XR_TIMEOUT_EXPIRED) {
            // End this frame with zero layers. Next wait retries THIS acquired
            // index, including when a later view timed out after earlier waits.
            frame->should_render=false;
            B2XR_InvalidateRenderHistory("swapchain-wait-timeout");
            frame->history_generation=xr.generation;
            return true;
        }
        if (!b2_xr_result(result, "wait-eye-image")) {
            frame->should_render = false;
            return true; // An unwaited acquire stays owned until swapchain teardown.
        }
        target->waited = true;
        if (target->index >= target->count) {
            b2_xr_error("acquired-eye-index-outside-enumerated-images");
            frame->should_render = false;
            return true;
        }
        if (xr.loss_pending) {
            frame->should_render = false;
            return true; // EndFrame retires this successful acquire/wait.
        }
    }
    return true;
}

bool B2XR_BeginFrame(b2_frame_t *frame)
{
    b2wv_attempted=b2wv_valid=false;memset(&b2wv_sample,0,sizeof(b2wv_sample));
    bool capture=b2wj_capture();
    B2WP_ObserveWindow(false);b2wj_pending=NULL;b2wj_frame=NULL;
    bool begun=b2_xr_begin_frame(frame); // Original implementation and return value.
    if (capture) {
        b2wj_row_t *r=&b2wj_rows[b2wj_count++];
        r->begun=begun;r->now=Sys_Milliseconds();
        r->failed=xr.failed;r->loss=xr.loss_pending;
        B2WP_ObserveSnapshot(&r->weapon);
        if (begun) {
            r->predicted=frame->predicted_time_ns;r->period=frame->duration_ns;
            r->generation=frame->history_generation;
            b2wj_pending=r;b2wj_frame=frame;B2WP_ObserveWindow(true);
        }
        // Failed begin has no logical frame/end result; preserve the row as such.
    }
    return begun;
}

bool B2XR_CopyEye(VkCommandBuffer commands, VkImage source, uint32_t eye,
    uint32_t width, uint32_t height)
{
    if (xr.failed || xr.loss_pending || !xr.frame_open || eye >= xr.view_count ||
        width != B2G_Width() || height != B2G_Height() || !source || !commands)
        return b2_xr_error("eye-copy-contract");
    b2_xr_eye_t *target = &xr.eyes[eye];
    if (!target->acquired || !target->waited || target->copied || target->released)
        return b2_xr_error("eye-copy-ownership");
    VkImage image = target->images[target->index].image;
    VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    VkImageMemoryBarrier barriers[2] = {
        { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
          .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
          .oldLayout = VK_IMAGE_LAYOUT_GENERAL, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .image = source, .subresourceRange = range },
        { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
          .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .image = image, .subresourceRange = range } };
    vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 2, barriers);
    VkImageBlit blit = { .srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        .dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        .srcOffsets[1] = { B2G_Width(), B2G_Height(), 1 },
        .dstOffsets[1] = { (int32_t)target->width, (int32_t)target->height, 1 } };
    // TAA_OUTPUT contains SDR tone-mapped linear RGB. Vulkan encodes sRGB on
    // this blit. Opaque composition ignores its alpha; there is no second tone map.
    vkCmdBlitImage(commands, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);
    barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    barriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barriers[0].newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barriers[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barriers[1].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 2, barriers);
    target->copied = true;
    return true;
}

bool B2XR_FinishEye(uint32_t eye)
{
    if (xr.failed || !xr.frame_open || eye >= xr.view_count)
        return b2_xr_error("eye-finish-contract");
    b2_xr_eye_t *target = &xr.eyes[eye];
    if (!target->acquired || !target->waited || !target->copied || target->released)
        return b2_xr_error("eye-finish-ownership");
    // Deliberately serial first prototype: all submitted graphics work has
    // completed before runtime release and before the other eye reuses scratch.
    VkResult completed = vkQueueWaitIdle(xr.queue);
    if (completed != VK_SUCCESS) {
        Com_EPrintf("B2XR graphics-completion result=%d eye=%u\n", completed, eye);
        // No ordinary recovery may abandon unknown submitted graphics work.
        Com_Error(ERR_FATAL,"VR graphics work could not complete. Restart the game after checking the graphics driver.");
        return false;
    }
    XrSwapchainImageReleaseInfo release = { .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    if (!b2_xr_result(xr.xrReleaseSwapchainImage(target->swapchain, &release), "release-eye-image"))
        return false;
    target->acquired = target->waited = false;
    target->released = true;
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR eye-complete eye=%u image=%u\n", eye, target->index);
    return true;
}

bool B2XR_EndFrame(bool rendered_pair)
{
    if (!xr.frame_open)
        return b2_xr_error("end-without-begin");
    bool complete = rendered_pair && !xr.failed && !xr.loss_pending && xr.frame->should_render &&
        xr.frame->tracking_valid;
    for (uint32_t eye = 0; eye < xr.view_count; ++eye)
        complete = complete && xr.eyes[eye].copied && xr.eyes[eye].released;
    // Retire waited but unused images on a client rejection. Never release an
    // image after a timeout, or copied work whose submission is not confirmed.
    for (uint32_t eye = 0; eye < xr.view_count; ++eye) {
        b2_xr_eye_t *target = &xr.eyes[eye];
        if (target->acquired && target->waited && !target->copied) {
            XrSwapchainImageReleaseInfo release = { .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
            if (b2_xr_result(xr.xrReleaseSwapchainImage(target->swapchain, &release),
                "release-unused-eye"))
                target->acquired = target->waited = false;
        }
    }
    XrCompositionLayerProjectionView views[B2_MAX_VIEWS];
    memset(views, 0, sizeof(views));
    for (uint32_t eye = 0; eye < xr.view_count; ++eye) {
        views[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        views[eye].pose = xr.views[eye].pose;
        views[eye].fov = xr.views[eye].fov;
        views[eye].subImage.swapchain = xr.eyes[eye].swapchain;
        views[eye].subImage.imageRect.extent = (XrExtent2Di){ (int32_t)xr.eyes[eye].width, (int32_t)xr.eyes[eye].height };
    }
    XrCompositionLayerProjection layer = { .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
        .space = xr.space, .viewCount = xr.view_count, .views = views };
    const XrCompositionLayerBaseHeader *layers[] = { (const void *)&layer };
    XrFrameEndInfo end = { .type = XR_TYPE_FRAME_END_INFO,
        .displayTime = xr.display_time, .environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
        .layerCount = complete ? 1 : 0, .layers = complete ? layers : NULL };
    XrResult result = xr.xrEndFrame(xr.session, &end);
    xr.frame_open = false;
    double milliseconds = 1000.0 * (double)(SDL_GetPerformanceCounter() - xr.begin_ticks) /
        (double)SDL_GetPerformanceFrequency();
    if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR frame-end logical=%u generation=%u layers=%u cpu_ms=%.3f result=%d\n",
        xr.frame->logical_index, xr.frame->history_generation, end.layerCount, milliseconds, (int)result);
    if (b2wj_pending) {
        b2wj_row_t *r=b2wj_pending;
        r->end_called=true;r->same_frame=xr.frame==b2wj_frame;
        if (r->same_frame) {
            r->logical=xr.frame->logical_index;r->generation=xr.frame->history_generation;
            r->predicted=xr.frame->predicted_time_ns;r->period=xr.frame->duration_ns;
            r->should_render=xr.frame->should_render;r->tracking=xr.frame->tracking_valid;
            r->rendered_pair=rendered_pair;r->layers=end.layerCount;r->result=(int)result;
            r->failed=xr.failed;r->loss=xr.loss_pending;r->now=Sys_Milliseconds();
            B2WP_ObserveSnapshot(&r->weapon);
        }
        b2wj_pending=NULL;b2wj_frame=NULL;B2WP_ObserveWindow(false);
    }
    xr.frame = NULL;
    bool success = b2_xr_result(result, "end-frame");
    if (!complete && !xr.frame_unavailable)
        B2XR_InvalidateHistory("pair-not-submitted");
    xr.frame_unavailable = !complete;
    if (rendered_pair && !complete)
        return b2_xr_error("incomplete-rendered-pair");
    if (xr.loss_pending)
        return b2_xr_error("session-loss-terminal-closure");
    return success && !xr.failed;
}

bool B2XR_AbortFrame555(void)
{
    // Called before an ERR_DROP unwinds a stack-owned frame. Unwaited images
    // remain recorded; EndFrame retires only successfully waited unused images.
    return (!xr.frame_open || B2XR_EndFrame(false)) && !B2XR_StopRequested();
}

void B2XR_RequestStop(void)
{
    // Preserve the initiating cause before cleanup or our own exit request can
    // produce a later session-loss event. A normal quit caches this text only:
    // it neither emits a diagnostic nor displays a stopped-session dialog.
    if ((xr.loader || xr.instance) && !xr.exit_requested && !b2_failure_message555[0])
        Q_strlcpy(b2_failure_message555,B2_InternalStopMessage642(),sizeof(b2_failure_message555));
    B2H_Reset();
    if ((!xr.loader && !xr.instance) || xr.exit_requested)
        return;
    xr.exit_requested = true;
    B2XR_InvalidateHistory("operator-stop");
    if (xr.running && xr.xrRequestExitSession)
        b2_xr_result(xr.xrRequestExitSession(xr.session), "request-exit-session");
}

bool B2XR_InitialReferenceReady(void)
{
    return xr.frame_open && xr.running && !xr.failed && !xr.exit_requested && !xr.loss_pending &&
        (xr.session_state == XR_SESSION_STATE_VISIBLE || xr.session_state == XR_SESSION_STATE_FOCUSED);
}

uint32_t B2XR_InterruptionEpoch(void)
{
    return xr.interruption_epoch;
}

// Focus/session availability without conflating it with transient tracking.
bool B2XR_Focused582(void)
{
    return xr.running && xr.focused && !xr.failed && !xr.reference_refused &&
        !xr.exit_requested && !xr.loss_pending;
}

bool B2XR_InteractionReady(void)
{
    return xr.running && xr.focused && xr.tracking && !xr.failed && !xr.reference_refused &&
        !xr.exit_requested && !xr.loss_pending;
}

bool B2XR_QuitOnly(void)
{
    return xr.reference_refused && xr.session && !xr.failed && !xr.loss_pending && !xr.exit_requested;
}

bool B2XR_StopRequested(void)
{
    return xr.exit_requested || xr.failed || xr.loss_pending || xr.reference_refused;
}

// Read-only start-panel status from the last fresh action sample; no extra XR polling.
const char *B2XR_ControllerNotice582(unsigned hand)
{
    if (hand>1) return NULL;
    if (!B2XR_Focused582() || !b2_actions.input_enabled || !b2_actions.sample_seen556 ||
        (uint32_t)(Sys_Milliseconds()-b2_actions.sample_ms556)>B2C_MAX_AGE_MS)
        return hand ? "Right: controller input unavailable" : "Left: controller input unavailable";
    if (!b2_actions.profile_kind582[hand])
        return hand ? "Right: no supported controller profile" : "Left: no supported controller profile";
    if (b2_actions.profile_kind582[hand]==4)
        return hand ? "Right: simple profile; keyboard needed" : "Left: simple profile; keyboard needed";
    return NULL;
}

void B2XR_ControllerDiagnostic(b2c_diagnostic_t *out)
{
    *out=b2_actions.diagnostic;
}

bool B2XR_ReadHeadInput(b2rs_head_t *out)
{
    if(!out)return false;memset(out,0,sizeof(*out));
    uint32_t now=Sys_Milliseconds();
    if(xr.frame_open || !B2XR_InteractionReady() || !xr.tracking || xr.view_count<2 ||
       xr.controller_head_generation!=xr.input_generation || (uint32_t)(now-xr.controller_head_ms)>100 ||
       xr.display_time<=0)return false;
    for(unsigned i=0;i<2;i++){
        const XrPosef *p=&xr.views[i].pose;
        if(!B2RS_CoordinateValid(p->position.x) || !B2RS_CoordinateValid(p->position.y) ||
           !B2RS_CoordinateValid(p->position.z))return false;
    }
    out->position[0]=B2RS_Midpoint(xr.views[0].pose.position.x,xr.views[1].pose.position.x);
    out->position[1]=B2RS_Midpoint(xr.views[0].pose.position.y,xr.views[1].pose.position.y);
    out->position[2]=B2RS_Midpoint(xr.views[0].pose.position.z,xr.views[1].pose.position.z);
    for(unsigned i=0;i<3;i++)if(!B2RS_CoordinateValid(out->position[i]))return false;
    out->serial=(uint64_t)xr.display_time;out->captured_ms=xr.controller_head_ms;
    out->generation=xr.input_generation;out->valid=true;return true;
}

static bool b2_xr_read_controller(b2c_snapshot_t *snapshot,bool quit_only)
{
    memset(&xr.resume_cue,0,sizeof(xr.resume_cue));
    memset(snapshot,0,sizeof(*snapshot));
    bool ready=quit_only ? (B2XR_QuitOnly() && xr.running && xr.focused && xr.tracking) : B2XR_InteractionReady();
    b2_actions.diagnostic=(b2c_diagnostic_t){!b2_actions.attached?"actions-detached":
        xr.frame_open?"frame-open":!ready?"interaction-not-ready":
        "head-generation",Sys_Milliseconds(),xr.controller_head_ms,-1};
    if (!b2_actions.input_enabled || !b2_actions.attached || xr.frame_open || !ready ||
        xr.controller_head_generation!=xr.input_generation) { b2ca_invalidate556(&b2_actions);return false; }
    b2_actions.diagnostic.reason="head-pose";
    for (unsigned eye=0;eye<xr.view_count;eye++) {
        const XrPosef *p=&xr.views[eye].pose;
        float q[4]={p->orientation.x,p->orientation.y,p->orientation.z,p->orientation.w}, norm=0;
        if (!isfinite(p->position.x) || !isfinite(p->position.y) || !isfinite(p->position.z)) return false;
        for (unsigned i=0;i<4;i++) { if (!isfinite(q[i])) return false; norm+=q[i]*q[i]; }
        if (fabsf(norm-1.f)>.02f) return false;
    }
    if (!b2ca_read(&b2_actions,true,xr.space,xr.display_time,Sys_Milliseconds(),
        xr.controller_head_ms,true,snapshot)) return false;
    snapshot->xr_generation=xr.input_generation;
    const XrPosef *left=&xr.views[0].pose, *right=&xr.views[1].pose;
    snapshot->head_position[0]=(left->position.x+right->position.x)*.5f;
    snapshot->head_position[1]=(left->position.y+right->position.y)*.5f;
    snapshot->head_position[2]=(left->position.z+right->position.z)*.5f;
    const float a[4]={left->orientation.x,left->orientation.y,left->orientation.z,left->orientation.w};
    const float b[4]={right->orientation.x,right->orientation.y,right->orientation.z,right->orientation.w};
    float dot=0, norm=0;
    for (unsigned i=0;i<4;i++) dot+=a[i]*b[i];
    for (unsigned i=0;i<4;i++) { snapshot->head_orientation[i]=a[i]+(dot<0?-b[i]:b[i]); norm+=snapshot->head_orientation[i]*snapshot->head_orientation[i]; }
    if (!isfinite(norm) || norm<1e-6f) { b2_actions.diagnostic.reason="head-mean-orientation"; memset(snapshot,0,sizeof(*snapshot)); return false; }
    for (unsigned i=0;i<4;i++) snapshot->head_orientation[i]/=sqrtf(norm);
    return true;
}

bool B2XR_ReadWeaponDisplay(const struct b2_frame_s *frame, unsigned hand,
    b2c_snapshot_t *out)
{
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (!frame || hand>1 || !xr.frame_open || frame!=xr.frame ||
        !frame->should_render || !frame->tracking_valid || !B2XR_InteractionReady() ||
        xr.reference_pending_time || xr.reference_realign391 || xr.reference_refused ||
        frame->predicted_time_ns!=xr.display_time || xr.display_time<=0 ||
        !b2_actions.input_enabled || !b2_actions.attached || b2_actions.exhausted ||
        !b2_actions.generation || !b2_actions.serial ||
        xr.controller_head_generation!=xr.input_generation) return false;
    if (b2wv_attempted) {
        if (!b2wv_valid || hand!=b2wv_hand || b2wv_sample.pose_time!=xr.display_time ||
            b2wv_sample.generation!=b2_actions.generation ||
            b2wv_sample.xr_generation!=xr.input_generation) return false;
        *out=b2wv_sample;return true;
    }
    b2wv_attempted=true;b2wv_hand=hand;
    uint32_t now=Sys_Milliseconds();
    if ((uint32_t)(now-xr.controller_head_ms)>B2C_MAX_AGE_MS) return false;
    XrInteractionProfileState profile={.type=XR_TYPE_INTERACTION_PROFILE_STATE};
    if (b2_actions.xrGetCurrentInteractionProfile(b2_actions.session,b2_actions.hands[hand],&profile)!=XR_SUCCESS ||
        profile.interactionProfile!=b2_actions.current[hand]) return false;
    bool supported=false;
    for (unsigned i=0;i<4;i++) supported|=b2_actions.suggested[i] && profile.interactionProfile==b2_actions.profiles[i];
    if (!supported) return false;
    b2c_snapshot_t value={0};
    if (!b2ca_pose(&b2_actions,hand,B2CA_AIM,xr.space,xr.display_time,
        value.hands[hand].aim_position,value.hands[hand].aim_orientation)) return false;
    // Display identity is the predicted target, not another gameplay read serial.
    value.serial=(uint64_t)xr.display_time;value.pose_time=xr.display_time;
    value.capture_ms=now;value.head_ms=xr.controller_head_ms;
    value.generation=b2_actions.generation;value.xr_generation=xr.input_generation;
    value.ready=true;value.hands[hand].aim_valid=value.hands[hand].controls_active=true;
    value.hands[hand].aim_active556=true;
    value.hands[hand].aim_tracked_ms556=b2_actions.pose_ms556[hand][B2CA_AIM];
    value.hands[hand].aim_tracked556=b2_actions.pose_tracked556[hand][B2CA_AIM];
    b2wv_sample=value;b2wv_valid=true;*out=value;return true;
}

#include "../../../controller-labels413/source/names413.inc"

bool B2XR_ReadControllerInput(b2c_snapshot_t *snapshot)
{ return b2_xr_read_controller(snapshot,false); }
bool B2XR_ReadQuitInput(b2c_snapshot_t *snapshot)
{ return b2_xr_read_controller(snapshot,true); }

void B2XR_Shutdown(void)
{
    if (!xr.loader && !xr.instance)
        return;
    if (xr.frame_open)
        B2XR_EndFrame(false);
    B2XR_RequestStop();
    if (xr.device) {
        VkResult result = vkDeviceWaitIdle(xr.device);
        if (result != VK_SUCCESS || Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR shutdown-device-idle result=%d\n", (int)result);
    }
    // Destroying the session is the terminal path even when the runtime never
    // delivers STOPPING. xrEndSession itself is used only in STOPPING above.
    for (uint32_t eye = 0; eye < xr.view_count; ++eye) {
        if (xr.eyes[eye].swapchain && xr.xrDestroySwapchain)
            b2_xr_result(xr.xrDestroySwapchain(xr.eyes[eye].swapchain), "destroy-eye-swapchain");
        free(xr.eyes[eye].images);
    }
    if (!b2ca_destroy(&b2_actions)) b2_xr_error("controller-shutdown-cleanup");
    if (xr.rendering_gaze_space && xr.xrDestroySpace)
        b2_xr_result(xr.xrDestroySpace(xr.rendering_gaze_space), "destroy-rendering-gaze-status-space");
    if (xr.replacement_space && xr.xrDestroySpace)
        b2_xr_result(xr.xrDestroySpace(xr.replacement_space), "destroy-uncommitted-reference-space");
    if (xr.space && xr.xrDestroySpace)
        b2_xr_result(xr.xrDestroySpace(xr.space), "destroy-space");
    if (xr.session && xr.xrDestroySession)
        b2_xr_result(xr.xrDestroySession(xr.session), "destroy-session");
    if (xr.instance && xr.xrDestroyInstance)
        b2_xr_result(xr.xrDestroyInstance(xr.instance), "destroy-instance");
    if (xr.loader)
        SDL_UnloadObject(xr.loader);
    memset(&xr, 0, sizeof(xr));
    { if (Cvar_VariableInteger("developer") > 0) Com_Printf("B2XR shutdown-complete\n"); }
}
