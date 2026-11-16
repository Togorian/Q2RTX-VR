/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/*
Copyright (C) 1997-2001 Id Software, Inc.
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
/* Modified by Togorian with AI assistance, 12 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */


#pragma once

#include "common/cvar.h"
#include "common/error.h"

#define MAX_DLIGHTS     32
#define MAX_ENTITIES    2048
#define MAX_PARTICLES   16384
#define MAX_LIGHTSTYLES 256

#define POWERSUIT_SCALE     4.0f
#define WEAPONSHELL_SCALE   0.5f

#define SHELL_RED_COLOR     0xF2
#define SHELL_GREEN_COLOR   0xD0
#define SHELL_BLUE_COLOR    0xF3

#define SHELL_RG_COLOR      0xDC
//#define SHELL_RB_COLOR        0x86
#define SHELL_RB_COLOR      0x68
#define SHELL_BG_COLOR      0x78

//ROGUE
#define SHELL_DOUBLE_COLOR  0xDF // 223
#define SHELL_HALF_DAM_COLOR    0x90
#define SHELL_CYAN_COLOR    0x72
//ROGUE

#define SHELL_WHITE_COLOR   0xD7

#define RF_SHELL_MASK       (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | \
                             RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_LITE_GREEN)

#define DLIGHT_CUTOFF       64

typedef struct entity_s {
    qhandle_t           model;          // opaque type outside refresh
    vec3_t              angles;

    /*
    ** most recent data
    */
    vec3_t              origin;     // also used as RF_BEAM's "from"
    int                 frame;          // also used as RF_BEAM's diameter

    /*
    ** previous data for lerping
    */
    vec3_t              oldorigin;  // also used as RF_BEAM's "to"
    int                 oldframe;

    /*
    ** misc
    */
    float   backlerp;               // 0.0 = current, 1.0 = old
    int     skinnum;                // also used as RF_BEAM's palette index,
                                    // -1 => use rgba

    float   alpha;                  // ignore if RF_TRANSLUCENT isn't set
    color_t rgba;

    qhandle_t   skin;           // NULL for inline skin
    int         flags;

    int                 id;

	int tent_type;

	float scale;
} entity_t;

typedef enum dlight_type_e
{
    DLIGHT_SPHERE = 0,
    DLIGHT_SPOT
} dlight_type;

typedef enum dlight_spot_emission_profile_e
{
    DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF = 0,
    DLIGHT_SPOT_EMISSION_PROFILE_AXIS_ANGLE_TEXTURE
} dlight_spot_emission_profile;

typedef struct dlight_s {
    vec3_t  origin;
#if USE_REF == REF_GL
    vec3_t  transformed;
#endif
    vec3_t  color;
    float   intensity;
	float   radius;

    // VKPT light types support
    dlight_type light_type;
    // Spotlight options
    struct {
        // Spotlight emission profile
        dlight_spot_emission_profile emission_profile;
        // Spotlight direction
        vec3_t  direction;
        union {
            // Options for DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF
            struct {
                // Cosine of angle of spotlight cone width (no emission beyond that)
                float   cos_total_width;
                // Cosine of angle of start of falloff (full emission below that)
                float   cos_falloff_start;
            };
            // Options for DLIGHT_SPOT_EMISSION_PROFILE_AXIS_ANGLE_TEXTURE
            struct {
                // Angle of spotlight cone width (no emission beyond that), in radians
                float   total_width;
                // Emission profile texture, indexed by 'angle / total_width'
                qhandle_t texture;
            };
        };
    } spot;
} dlight_t;

typedef struct particle_s {
    vec3_t  origin;
    int     color;              // -1 => use rgba
    float   alpha;
    color_t rgba;
	float   brightness;
	float   radius;
} particle_t;

typedef struct lightstyle_s {
    float           white;          // highest of RGB
} lightstyle_t;

#ifdef USE_SMALL_GPU
#define MAX_DECALS 2
#else
#define MAX_DECALS 50
#endif
typedef struct decal_s {
    vec3_t pos;
    vec3_t dir;
    float spread;
    float length;
    float dummy;
} decal_t;

// passes information back from the RTX renderer to the engine for various development maros
typedef struct ref_feedback_s {
	int         viewcluster;
	int         lookatcluster;
	int         num_light_polys;
	int         resolution_scale;

	char        view_material[MAX_QPATH];
	char        view_material_override[MAX_QPATH];
    int         view_material_index;

	vec3_t      hdr_color;
	float       adapted_luminance;
} ref_feedback_t;

#ifdef REFERENCE_B_B1
typedef enum b1_renderer_phase_e {
    B1_RENDERER_PRIMARY_UPDATE = 0,
    B1_RENDERER_SECONDARY_REUSE = 1
} b1_renderer_phase_t;

typedef enum b1_scene_scope_e {
    B1_SCENE_SCOPE_INVALID = 0,
    B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_V1 = 1
} b1_scene_scope_t;

#define B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_GENERATION 1u
#endif

#ifdef REFERENCE_B_B2
#include "refresh/reference_b_b2.h"
bool R_B2PairFinish(bool submitted_pair);
#endif

typedef struct refdef_s {
    int         x, y, width, height;// in virtual screen coordinates
    float       fov_x, fov_y;
    vec3_t      vieworg;
    vec3_t      viewangles;
    vec4_t      blend;          // rgba 0-1 full screen blend
    float       time;               // time is uesed to auto animate
    int         rdflags;            // RDF_UNDERWATER, etc

    byte        *areabits;          // if not NULL, only areas with set bits will be drawn

    lightstyle_t    *lightstyles;   // [MAX_LIGHTSTYLES]

    int         num_entities;
    entity_t    *entities;

    int         num_dlights;
    dlight_t    *dlights;

    int         num_particles;
    particle_t  *particles;

    int         decal_beg;
    int         decal_end;
    decal_t     decal[MAX_DECALS];

	ref_feedback_t feedback;
#ifdef REFERENCE_B_B2
    bool        b2_active;
    bool        b2_render_accepted;
    uint32_t    b2_eye;
    const b2_frame_t *b2_frame;
#endif
#ifdef REFERENCE_B_B0U
    // Private diagnostic input boundary. This structure is present only in the
    // opt-in B0-U build and is never part of an ordinary or packaged client.
    bool        b0u_camera_override;
    uint32_t    b0u_stimulus_index;
    float       b0u_V[16];
    float       b0u_invV[16];
    float       b0u_P[16];
    float       b0u_invP[16];
#endif
#ifdef REFERENCE_B_B1
    // Host-only B1 replay identity and role-owned camera history.  These fields
    // are never shader ABI: they select existing UBO members in the opt-in
    // diagnostic renderer.
    bool        b1_active;
    bool        b1_bootstrap_probe;
    bool        b1_material_secondary_advance_tamper;
    bool        b1_temporal_history_valid;
    bool        b1_render_accepted;
    bool        b1_capture_requested;
    uint32_t    b1_capture_request_schema_version;
    uint64_t    b1_capture_request_serial;
    uint32_t    b1_logical_index;
    uint32_t    b1_source_frame_id;
    uint32_t    b1_role;
    uint32_t    b1_phase;
    uint32_t    b1_scene_scope;
    uint32_t    b1_scene_scope_generation;
    uint32_t    b1_effective_particle_count;
    float       b1_logical_duration;
    uint64_t    b1_physical_frame;
    uint32_t    b1_physical_slot;
    float       b1_V[16];
    float       b1_invV[16];
    float       b1_P[16];
    float       b1_invP[16];
    float       b1_V_prev[16];
    float       b1_P_prev[16];
    float       b1_invP_prev[16];
    float       b1_projection_fov_scale[2];
    float       b1_projection_fov_scale_prev[2];
    float       b1_cylindrical_hfov;
    float       b1_cylindrical_hfov_prev;
    uint32_t    b1_render_extent_prev[2];
    uint32_t    b1_taa_extent_prev[2];
    uint32_t    b1_gpu_slice_width_prev;
    float       b1_pair_V[2][16];
    float       b1_pair_invV[2][16];
    float       b1_pair_fov_xy[2][2];
#endif
} refdef_t;

typedef struct {
    int     colorbits;
    int     depthbits;
    int     stencilbits;
    int     multisamples;
    bool    debug;
} r_opengl_config_t;

typedef enum {
    QVF_FULLSCREEN      = BIT(0),
    QVF_GAMMARAMP       = BIT(1),
} vidFlags_t;

typedef struct {
    int         width;
    int         height;
    vidFlags_t  flags;
} refcfg_t;

extern refcfg_t r_config;

typedef struct {
    int left, right, top, bottom;
} clipRect_t;

typedef enum {
    IF_NONE         = 0,
    IF_PERMANENT    = BIT(0),
    IF_TRANSPARENT  = BIT(1),
    IF_PALETTED     = BIT(2),
    IF_UPSCALED     = BIT(3),
    IF_SCRAP        = BIT(4),
    IF_TURBULENT    = BIT(5),
    IF_REPEAT       = BIT(6),
    IF_NEAREST      = BIT(7),
    IF_OPAQUE       = BIT(8),
    IF_SRGB         = BIT(9),
    IF_FAKE_EMISSIVE= BIT(10),
    IF_EXACT        = BIT(11),
    IF_NORMAL_MAP   = BIT(12),
    IF_BILERP       = BIT(13), // always lerp, independent of bilerp_pics cvar

    // Image source indicator/requirement flags
    IF_SRC_BASE     = (0x1 << 16),
    IF_SRC_GAME     = (0x2 << 16),
    IF_SRC_MASK     = (0x3 << 16),
} imageflags_t;

// Shift amount for storing fake emissive synthesis threshold
#define IF_FAKE_EMISSIVE_THRESH_SHIFT  20

typedef enum {
    IT_PIC,
    IT_FONT,
    IT_SKIN,
    IT_SPRITE,
    IT_WALL,
    IT_SKY,

    IT_MAX
} imagetype_t;

typedef enum ref_type_e
{
    REF_TYPE_NONE = 0,
    REF_TYPE_GL,
    REF_TYPE_VKPT
} ref_type_t;

// called when the library is loaded
extern ref_type_t  (*R_Init)(bool total);

// called before the library is unloaded
extern void        (*R_Shutdown)(bool total);

// All data that will be used in a level should be
// registered before rendering any frames to prevent disk hits,
// but they can still be registered at a later time
// if necessary.
//
// EndRegistration will free any remaining data that wasn't registered.
// Any model_s or skin_s pointers from before the BeginRegistration
// are no longer valid after EndRegistration.
//
// Skins and images need to be differentiated, because skins
// are flood filled to eliminate mip map edge errors, and pics have
// an implicit "pics/" prepended to the name. (a pic name that starts with a
// slash will not use the "pics/" prefix or the ".pcx" postfix)
extern void    (*R_BeginRegistration)(const char *map);
qhandle_t R_RegisterModel(const char *name);
qhandle_t R_RegisterImage(const char *name, imagetype_t type,
                          imageflags_t flags);
qhandle_t R_RegisterRawImage(const char *name, int width, int height, byte* pic, imagetype_t type,
                          imageflags_t flags);
void R_UnregisterImage(qhandle_t handle);

extern void    (*R_SetSky)(const char *name, float rotate, int autorotate, const vec3_t axis);
extern void    (*R_EndRegistration)(void);

#define R_RegisterPic(name)     R_RegisterImage(name, IT_PIC, IF_PERMANENT | IF_SRGB)
#define R_RegisterPic2(name)    R_RegisterImage(name, IT_PIC, IF_SRGB)
#define R_RegisterFont(name)    R_RegisterImage(name, IT_FONT, IF_PERMANENT | IF_SRGB)
#define R_RegisterSkin(name)    R_RegisterImage(name, IT_SKIN, IF_SRGB)
#define R_RegisterSprite(name)  R_RegisterImage(name, IT_SPRITE, IF_SRGB)

extern void    (*R_RenderFrame)(refdef_t *fd);
extern void    (*R_LightPoint)(const vec3_t origin, vec3_t light);

#ifdef REFERENCE_B_B1
typedef void (*b1_renderer_log_fn)(const char *line, void *opaque);

typedef enum b1_scene_lifecycle_state_e {
    B1_SCENE_ENDPOINT_ACCEPTED = 1,
    B1_SCENE_CLIENT_SCENE_FROZEN,
    B1_SCENE_TEMPORAL_INFRASTRUCTURE_READY,
    B1_SCENE_RENDERER_SCENE_PREPARED,
    B1_SCENE_COMPLETE_SCENE_ACCEPTED,
    B1_SCENE_TRACE_PERMITTED,
    B1_SCENE_PAIR_COMMITTED
} b1_scene_lifecycle_state_t;

#define B1_RENDERER_SCENE_CLOSURE_VERSION 3u
#define B1_RENDERER_SCENE_LIGHT_CLUSTER_COUNT 1446u

/* Canonical client authority passed into the renderer before inert temporal
 * infrastructure is prepared.  Consumers must serialise the named fields;
 * the structure itself, including any padding, is never an identity. */
typedef struct b1_scene_authority_s {
    byte        fixture_sha256[32];
    byte        plan_sha256[32];
    byte        input_closure_sha256[32];
    byte        executable_sha256[32];
    byte        runtime_manifest_sha256[32];
    byte        runtime_inventory_sha256[32];
    byte        selected_message_sha256[32];
    byte        range_set_sha256[32];
    byte        endpoint_sha256[32];
    byte        client_scene_sha256[32];
    uint32_t    logical_index;
    uint32_t    source_frame_id;
    uint32_t    scene_scope;
    uint32_t    scene_scope_generation;
    uint32_t    effective_particle_count;
} b1_scene_authority_t;

/* Renderer-derived, active-field-only scene closure.  Hashes deliberately
 * exclude raw pointers, allocation addresses, Vulkan handles, padding and
 * inactive capacity. */
typedef struct b1_renderer_scene_closure_s {
    uint32_t                schema_version;
    b1_scene_authority_t    authority;
    uint64_t                invocation_id;
    uint64_t                preparation_serial;
    uint32_t                preparation_count;
    uint32_t                active_entity_count;
    uint32_t                animated_index_count;
    uint32_t                model_instance_count;
    uint32_t                iqm_matrix_count;
    uint32_t                light_cluster_count;
    uint32_t                prepared_role_source_mask;
    uint64_t                role_storage_hash64[2];
    uint64_t                iqm_hash64;
    uint64_t                world_material_hash64;
    uint64_t                world_material_generation;
    uint64_t                model_light_hash64;
    uint64_t                model_light_generation;
    uint64_t                light_count_hash64;
    uint64_t                light_count_ledger_hash64;
    uint64_t                primary_left_tlas_hash64;
    uint64_t                renderer_build_identity_hash64;
    uint64_t                renderer_configuration_identity_hash64;
    uint64_t                temporal_infrastructure_identity_hash64;
    uint64_t                aggregate_hash64;
    bool                    primary_left_tlas_prepared;
    bool                    right_gpu_execution_observed;
    bool                    right_tlas_execution_observed;
} b1_renderer_scene_closure_t;

typedef bool (*b1_renderer_scene_accept_fn)(
    const b1_renderer_scene_closure_t *closure, void *opaque);
typedef bool (*b1_renderer_trace_permission_fn)(
    const b1_renderer_scene_closure_t *closure, void *opaque);
typedef bool (*b1_renderer_scene_consumed_fn)(
    const b1_renderer_scene_closure_t *closure, uint32_t role, void *opaque);

bool R_B1TemporalInfrastructurePrepare(
    const b1_scene_authority_t *authority,
    b1_renderer_log_fn log_line, void *log_opaque,
    b1_renderer_scene_accept_fn accept_scene,
    b1_renderer_trace_permission_fn permit_trace,
    b1_renderer_scene_consumed_fn consume_scene,
    void *admission_opaque);
void R_B1TemporalShutdown(bool completed);
void R_B1CadencePairCommit(uint32_t logical_index);
#endif

extern void    (*R_ClearColor)(void);
extern void    (*R_SetAlpha)(float clpha);
extern void    (*R_SetAlphaScale)(float alpha);
extern void    (*R_SetColor)(uint32_t color);
extern void    (*R_SetClipRect)(const clipRect_t *clip);
float   R_ClampScale(cvar_t *var);
extern void    (*R_SetScale)(float scale);
extern void    (*R_DrawChar)(int x, int y, int flags, int ch, qhandle_t font);
extern int     (*R_DrawString)(int x, int y, int flags, size_t maxChars,
                     const char *string, qhandle_t font);  // returns advanced x coord
bool R_GetPicSize(int *w, int *h, qhandle_t pic);   // returns transparency bit
extern void    (*R_DrawPic)(int x, int y, qhandle_t pic);
extern void    (*R_DrawStretchPic)(int x, int y, int w, int h, qhandle_t pic);
extern void    (*R_DrawKeepAspectPic)(int x, int y, int w, int h, qhandle_t pic);
extern void    (*R_DrawStretchRaw)(int x, int y, int w, int h);
extern void    (*R_TileClear)(int x, int y, int w, int h, qhandle_t pic);
extern void    (*R_DrawFill8)(int x, int y, int w, int h, int c);
extern void    (*R_DrawFill32)(int x, int y, int w, int h, uint32_t color);
extern void    (*R_UpdateRawPic)(int pic_w, int pic_h, const uint32_t *pic);
extern void    (*R_DiscardRawPic)(void);

// debug line entry points
extern bool (*R_SupportsDebugLines)(void);
extern void (*R_AddDebugText_)(const vec3_t origin, const vec3_t angles, const char *text,
                               float size, uint32_t color, uint32_t time, bool depth_test);

// video mode and refresh state management entry points
extern void    (*R_BeginFrame)(void);
extern void    (*R_EndFrame)(void);
extern void    (*R_ModeChanged)(int width, int height, int flags);

// add decal to ring buffer
extern void    (*R_AddDecal)(decal_t *d);

extern bool    (*R_InterceptKey)(unsigned key, bool down);
extern bool    (*R_IsHDR)(void);

#if REF_GL
void R_RegisterFunctionsGL(void);
#endif
#if REF_VKPT
void R_RegisterFunctionsRTX(void);
#endif

r_opengl_config_t *R_GetGLConfig(void);
