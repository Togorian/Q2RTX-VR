// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 11 November 2026: preserve diagnostic origin independently of product branding.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Reference-B B1 private sequential-two-view diagnostic adapter.
 *
 * Compiled only with CONFIG_REFERENCE_B_B1.  This host-only boundary
 * authenticates the exact six Builder-safe inputs and fixed-width B1 plan.
 * It is not a renderer feature, release facility, or general replay parser.
 */

#include "client.h"
#include "../server/server.h"
#include "reference_b_b0u.h"
#include "reference_b_b1.h"
#include "refresh/debug.h"
#include "refresh/images.h"
#include "refresh/models.h"
#include "format/sp2.h"
/* Inherited NVIDIA GPLv2-or-later constants; this B1-only translation unit
 * couples its canonical effective-shell byte to the renderer consumer. */
#include "../refresh/vkpt/shader/constants.h"

#include <windows.h>
#include <bcrypt.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define B1_CONTRACT "B1-PAIRED-CURRENT-R1"
#define B1_PINNED_BASE "f2526e9a165949f66e91e82f0d63aa7bb2567b4d"
#define B1_P1_FOUNDATION "322b252ecdd3d87c2456c1ebe0ea2f971234b1c8"
#define B1_SETTINGS_BYTES 4105u
#define B1_SETTINGS_SHA256 "BA8065C747EA11B2EF921547AFBD53F220AC720F709ED37E14D27D71F39CDA5A"
#define B1_PAYLOAD_BYTES UINT64_C(15027743)
#define B1_PAYLOAD_SHA256 "D0051CE08C25C25F78A3C25F3EC71FC134D06D7FBF3C9B05DDC753F370C64DA1"
#define B1_FIRST_SOURCE 190u
#define B1_LAST_SOURCE 4262u
#define B1_ACCEPTANCE_REQUIRED 0x0fu
#define B1_PHASE5A_SCHEMA "B1-SF-A1-PHASE5A-SEMANTIC-V1"
#define B1_PHASE5A_EXPECTED_MESSAGES 51u
#define B1_PHASE5A_MAX_CANDIDATES 128u
#define B1_PHASE5A_MAX_CANONICAL_BYTES (4u * 1024u * 1024u)
#define B1_PHASE5A_FIXTURE_SHA256 "62C0475537C6B6D07916D7F945A6112C7B448C83D5903923A5EC8C3482C4289F"
#define B1_PHASE5A_CLOSURE_SHA256 "B85C02C254A5DF34DD2DAEC860F10AEB3DCA1E3BA98510F35D06E3860250B0A8"
#define B1_PHASE5AR5_SCHEMA "B1-SF-A1-PHASE5A-ENDPOINT-SEMANTIC-V2"
#define B1_PHASE5AR5_SELECTED_MESSAGE_SHA256 "6389F2B146C16F87491B747E1E5FBFD607EF35FA2172F962DEFBF6B5B2B263C5"
#define B1_PHASE5AR5_SELECTED_FRAME 29
#define B1_PHASE5AR5_SELECTED_DELTA 28
#define B1_PHASE5AR5_SELECTED_SERVER_TIME 2800
#define B1_PHASE5AR5_SELECTED_ENTITIES 30
#define B1_PHASE5AR5_SELECTED_AREA_BYTES 1
#define B1_PHASE5AR5_SELECTED_CONFIGSTRINGS 2080u
#define B1_PHASE5AR5_EXPECTED_RANGES 2119u
#define B1_PHASE5AR5_ACCEPTED_ENDPOINT_SHA256 "5E6434A325DF5776ED088330B52A3C6AECD2A730CE90D3B07C01E94726B8573F"
#define B1_PHASE5AR5_ACCEPTED_RANGE_SET_SHA256 "0152DA7A53A005A806DD1B9C5B394602193F57EA4FE901085E7B7C11807E3039"
#define B1_PHASE5B_SCENE_SCHEMA "B1-SF-A1-PHASE5B-CLIENT-STATIC-CORE-SCENE-V1"
#define B1_PHASE5B_SCENE_COMPONENT_COUNT 9u
#define B1_STATIC_CORE_SCOPE_TOKEN "B1-SCENE-SCOPE-STATIC-ENVIRONMENT-PARTICLE-EXCLUDED-V1"
#define B1_STATIC_CORE_RAW_PARTICLE_SCHEMA "B1-STATIC-CORE-RAW-PARTICLE-TELEMETRY-V1"
#define B1_PHASE5B_FIXTURE_BYTES UINT64_C(11428)
#define B1_PHASE5B_FIXTURE_SHA256 "62C0475537C6B6D07916D7F945A6112C7B448C83D5903923A5EC8C3482C4289F"
#define B1_PHASE5BRPB_PERSISTENT_SCHEMA "B1-SF-A1-PHASE5BRPB-PERSISTENT-SCENE-V1"
#define B1_PHASE5BRPB_SEMANTIC_SCHEMA "B1-SF-A1-PHASE5BRPB-SEMANTIC-SCENE-V1"
#define B1_PHASE5BRPB_PARTICLE_SCHEMA "B1-SF-A1-PHASE5BRPB-PARTICLE-V2"
#define B1_PHASE5BRPB_EXPECTED_MESSAGES B1_PHASE5A_EXPECTED_MESSAGES
#define B1_PHASE5BRPB_MAX_BOUNDARIES B1_PHASE5A_EXPECTED_MESSAGES
#define B1_PHASE5BRPB_RANGE_COUNT 8u
#define B1_PHASE5BRPB_OLD_COMPONENT_COUNT 7u
/* vkpt/main.c::MAX_MODEL_LIGHTS is the active consumer capacity. */
#define B1_PHASE5BRPB_MAX_MODEL_LIGHT_POLYS 16384
/* vkpt/main.c::entity_hash_t assigns 14 bits to the semantic entity id. */
#define B1_PHASE5BRPB_MAX_ENTITY_ID (1u << 14)
#define B1_PHASE5BRPBR2_ENTITY_BYTES 2356u
#define B1_PHASE5BRPBR2_ENTITY_COUNT 28u
#define B1_PHASE5BRPBR2_ENTITY_SHA256 \
    "197D34D638615E5850504FFF7AB555ED85F42F98E4532A020CD47D9776A6B315"
#define B1_PHASE5BRPBR4_PROJECTED_ENTITY_SHA256 \
    "3BAC7370DECB0D48174C274694022F2B4D6FE4AC80350D519AEBDC26F746D8EE"
#define B1_PHASE5BRPBR4_MODEL_SEQUENCE_SHA256 \
    "530B8BFDDE1E3FE78C8E8496A69384C261BACAC955FB2B2F98C2C7D8804A3051"
#define B1_PHASE5BRPBR4_RAW_MEMBERSHIP_SHA256 \
    "1866037EFF5669D11B0025938E05E034CB8BE2CEE286FA8A398DE0167AF3B1F2"
#define B1_PHASE5BRPBR4_PROJECTED_MEMBERSHIP_SHA256 \
    "F473459A12796ADA49139B75511785E613329461EE8424AD52EC9D147E3A255D"
#define B1_PHASE5BRPBR6R3_ENTITY_ALIGNMENT_VERSION 1u
#define B1_PHASE5BRPBR6R4_ENTITY_ALIGNMENT_VERSION 2u
#define B1_PHASE5BRPB_KNOWN_MOREFX \
    (EFX_DUALFIRE | EFX_HOLOGRAM | EFX_FLASHLIGHT | \
     EFX_BARREL_EXPLODING | EFX_TELEPORTER2 | EFX_GRENADE_LIGHT)
#define B1_PHASE5BRPB_ACTIVE_REGULAR_FLAGS \
    (RF_VIEWERMODEL | RF_WEAPONMODEL | RF_TRANSLUCENT | RF_IR_VISIBLE | \
     RF_SHELL_MASK)
typedef enum b1_phase5brpb_mode_e {
    B1_PHASE5BRPB_MODE_OFF,
    B1_PHASE5BRPB_MODE_P0,
    B1_PHASE5BRPB_MODE_ENUMERATE
} b1_phase5brpb_mode_t;

typedef enum b1_phase5brpb_playback_state_e {
    B1_PHASE5BRPB_PLAYBACK_OFF,
    B1_PHASE5BRPB_PLAYBACK_OPEN,
    B1_PHASE5BRPB_PLAYBACK_PENDING_SCENE,
    B1_PHASE5BRPB_PLAYBACK_P0_CLOSED,
    B1_PHASE5BRPB_PLAYBACK_EOF,
    B1_PHASE5BRPB_PLAYBACK_SEALED,
    B1_PHASE5BRPB_PLAYBACK_FAILED
} b1_phase5brpb_playback_state_t;

typedef enum b1_phase5brpb_message_class_e {
    B1_PHASE5BRPB_MESSAGE_STARTUP,
    B1_PHASE5BRPB_MESSAGE_NO_VALID_FRAME,
    B1_PHASE5BRPB_MESSAGE_NO_NEW_FRAME,
    B1_PHASE5BRPB_MESSAGE_NO_PREDECESSOR,
    B1_PHASE5BRPB_MESSAGE_ELIGIBLE
} b1_phase5brpb_message_class_t;

typedef struct b1_phase5brpb_model_key_s {
    char name[MAX_QPATH];
    int32_t type;
    int32_t model_class;
    int32_t num_meshes;
    int32_t num_frames;
    int32_t num_light_polys;
    uint32_t iqm_vertexes;
    uint32_t iqm_triangles;
    uint32_t iqm_frames;
    uint32_t iqm_meshes;
    uint32_t iqm_joints;
    uint32_t iqm_poses;
    uint32_t iqm_animations;
    uint32_t sprite_frame_offset;
    uint32_t sprite_frame_count;
    int32_t registration_sequence;
    bool sprite_vertical;
    bool has_meshes;
    bool has_iqm;
    bool present;
} b1_phase5brpb_model_key_t;

typedef struct b1_phase5brpb_image_key_s {
    char name[MAX_QPATH];
    char filepath[MAX_QPATH];
    int32_t type;
    uint32_t flags;
    int16_t width;
    int16_t height;
    int16_t upload_width;
    int16_t upload_height;
    int32_t is_srgb;
    int32_t registration_sequence;
    bool present;
} b1_phase5brpb_image_key_t;

typedef struct b1_phase5brpb_sprite_frame_key_s {
    uint32_t image_key_index;
    int32_t width;
    int32_t height;
    int32_t origin_x;
    int32_t origin_y;
} b1_phase5brpb_sprite_frame_key_t;

typedef struct b1_phase5brpb_range_s {
    const char *name;
    uint32_t offset;
    uint32_t bytes;
    byte sha256[32];
} b1_phase5brpb_range_t;

typedef struct b1_phase5brpb_boundary_s {
    uint64_t ordinal;
    uint64_t notification_serial;
    uint64_t token_id;
    size_t message_offset;
    size_t config_offset;
    size_t endpoint_entity_offset;
    size_t raw_scene_entity_offset;
    size_t scene_entity_offset;
    size_t scene_particle_offset;
    size_t scene_dlight_offset;
    size_t scene_lightstyle_offset;
    size_t scene_area_offset;
    uint32_t message_bytes;
    uint32_t config_bytes;
    uint32_t config_logical_count;
    uint32_t endpoint_entity_count;
    uint32_t raw_scene_entity_count;
    uint32_t scene_entity_count;
    uint32_t scene_particle_count;
    uint32_t scene_dlight_count;
    uint32_t scene_lightstyle_count;
    uint32_t scene_area_bytes;
    uint32_t debug_primitive_count;
    uint32_t beam_entity_count;
    uint32_t active_beam_count;
    uint32_t resolved_sprite_count;
    uint32_t recovered_flags;
    int32_t observed_client_time;
    int32_t observed_server_time;
    uint32_t observed_lerpfrac_bits;
    float particle_size;
    server_frame_t frame;
    server_frame_t predecessor;
    cs_remap_t csr;
    refdef_t scene_refdef;
    char gamedir[MAX_QPATH];
    char mapname[MAX_QPATH];
    int32_t server_protocol;
    int32_t protocol_version;
    uint32_t es_flags;
    uint32_t ps_flags;
    int32_t server_count;
    int32_t server_state;
    int32_t client_num;
    int32_t num_entity_states;
    b1_phase5brpb_message_class_t message_class;
    bool frame_valid;
    bool new_frame;
    bool predecessor_present;
    bool predecessor_valid;
    bool scene_closed;
    bool selected_anchor;
    bool endpoint_identity_closed;
    bool endpoint_timing_closed;
    bool kick_angles_active;
    int32_t cl_gibs_value;
    int32_t s_ambient_value;
    bool finite_canonical;
    bool persistent_equal;
    bool zero_sources;
    bool qualifying;
    byte message_sha256[32];
    byte endpoint_sha256[32];
    byte endpoint_range_set_sha256[32];
    byte raw_entities_sha256[32];
    byte projected_entities_sha256[32];
    byte raw_model_sequence_sha256[32];
    byte projected_model_sequence_sha256[32];
    byte raw_membership_sha256[32];
    byte projected_membership_sha256[32];
    byte raw_record_sha256[B1_PHASE5BRPBR2_ENTITY_COUNT][32];
    byte projected_record_sha256[B1_PHASE5BRPBR2_ENTITY_COUNT][32];
    byte token_sha256[32];
    byte persistent_sha256[32];
    byte semantic_sha256[32];
    byte particle_v2_sha256[32];
    byte sprite_source_sha256[32];
    byte *particle_v2;
    byte *sprite_source;
    uint32_t persistent_bytes;
    uint32_t semantic_bytes;
    uint32_t particle_v2_bytes;
    uint32_t sprite_source_bytes;
    uint32_t endpoint_canonical_bytes;
    uint32_t endpoint_range_count;
    int32_t scene_build_client_time;
    int32_t scene_build_server_time;
    int32_t scene_build_frame;
    int32_t scene_build_delta;
    uint32_t scene_build_lerpfrac_bits;
    uint32_t scene_build_count;
    uint32_t entity_alignment_version;
    uint32_t entity_alignment_publication_count;
    uint32_t entity_alignment_revalidation_count;
    bool entity_alignment_closed;
    byte *raw_entities_v1;
    byte *projected_entities_v1;
    uint32_t raw_entities_v1_bytes;
    uint32_t projected_entities_v1_bytes;
    b1_phase5brpb_range_t ranges[B1_PHASE5BRPB_RANGE_COUNT];
} b1_phase5brpb_boundary_t;

typedef enum b1_technical_process_kind_e {
    B1_TECHNICAL_PROCESS_NONE = 0,
    B1_TECHNICAL_PROCESS_STATE_P0_P3 = 1,
    B1_TECHNICAL_PROCESS_FAILED_PAIR_AFTER_LEFT_P1 = 2,
    B1_TECHNICAL_PROCESS_TONE_POSITIVE_P0 = 3,
    B1_TECHNICAL_PROCESS_FEEDBACK_FAILURE_P1 = 4,
    B1_TECHNICAL_PROCESS_STATIC_CORE_SCENE_ADMISSION_WITNESS = 5,
    B1_TECHNICAL_PROCESS_CAPTURE_P0_P3 = 6,
    B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1 = 7,
    B1_TECHNICAL_PROCESS_COUNT
} b1_technical_process_kind_t;

/* Private cross-translation-unit handshake with vkpt/main.c.  Deliberately
 * kept out of the public refresh/client ABI and every shared header. */
uint32_t B1_PrivateTechnicalProcessKind(void);
const char *B1_PrivateTechnicalProcessToken(void);
uint64_t B1_PrivateTechnicalProcessIdentityHash64(void);
uint32_t B1_PrivateCaptureScalePairCount(void);
uint64_t B1_PrivateCaptureScaleIdentityHash64(void);
void B1_PrivateTechnicalExpectedStop(uint32_t kind, uint32_t logical_index);
void B1_PrivateCaptureQuiescenceQuitEnqueued(void);
void B1_PrivateCaptureQuiescenceMarker(const char *phase,
    const char *source_owner);
void B1_PrivateCaptureQuiescenceReadbackState(
    uint32_t active_phase, bool readback_mapped,
    uint64_t retired_request_serial, bool screenshot_image_allocated,
    bool screenshot_memory_allocated, uint64_t screenshot_memory_bytes,
    bool temporal_activated, bool initial_release_idle_required,
    bool expected_stop_idle_required, bool unconsumed_acquire_fatal_required,
    bool state_witness_destroy_required,
    uint64_t pending_client_request_serial,
    uint32_t pending_client_logical_index,
    uint32_t pending_client_source_frame_id,
    uint32_t pending_client_role);
void B1_PrivateCaptureQuiescenceResourceStateMarker(
    const char *phase, const char *source_owner,
    bool screenshot_image_allocated, bool screenshot_memory_allocated,
    uint64_t screenshot_memory_bytes, bool swapchain_live,
    bool device_live);
void R_B1PrivateCaptureQuiescenceStatus(
    uint32_t *validation_error_count, bool *device_lost);
void R_B1PrivateCaptureQuiescenceReadbackState(
    uint64_t pending_client_request_serial,
    uint32_t pending_client_logical_index,
    uint32_t pending_client_source_frame_id,
    uint32_t pending_client_role);
void B1_PrivateSceneAdmissionWitnessExpectedStop(
    const b1_renderer_scene_closure_t *closure,
    uint32_t route_observation_count, bool route_witness_complete,
    bool tagged_route_complete, bool zero_route_complete,
    bool canonical_zero_restored, uint32_t activation_submission_count,
    uint32_t activation_completion_count,
    uint32_t activation_expected_submission_count,
    uint32_t renderer_acceptance_count, bool validation_enabled,
    uint32_t validation_error_count, bool device_lost);

typedef enum b1_state_e {
    B1_STATE_OFF,
    B1_STATE_AUTHENTICATING,
    B1_STATE_READY,
    B1_STATE_LEFT_RENDER,
    B1_STATE_LEFT_CAPTURE,
    B1_STATE_RIGHT_RENDER,
    B1_STATE_RIGHT_CAPTURE,
    B1_STATE_PAIR_COMMIT,
    B1_STATE_COMPLETE,
    B1_STATE_FAILED
} b1_state_id_t;

typedef struct b1_role_context_s {
    b1_plan_view_t committed;
    b1_plan_view_t pending;
    uint32_t committed_logical;
    bool temporal_valid;
    bool pending_valid;
} b1_role_context_t;

typedef struct b1_file_identity_s {
    uint64_t bytes;
    byte sha256[32];
} b1_file_identity_t;

#pragma pack(push, 1)
typedef struct b1_plan_header_s {
    char magic[8];
    char contract[24];
    char pinned_base[41];
    char p1_foundation[41];
    uint32_t version;
    uint32_t header_size;
    uint32_t settings_bytes;
    uint32_t record_size;
    uint32_t record_count;
    uint32_t footer_size;
    uint32_t first_logical;
    uint32_t last_logical;
    uint32_t first_source;
    uint32_t last_source;
    uint32_t width;
    uint32_t height;
    uint64_t payload_bytes;
    byte payload_sha256[32];
    byte settings_sha256[32];
    b1_file_identity_t files[6];
} b1_plan_header_t;

typedef struct b1_plan_footer_s {
    char magic[8];
    uint32_t record_count;
    byte records_sha256[32];
    byte settings_sha256[32];
} b1_plan_footer_t;
#pragma pack(pop)

_Static_assert(sizeof(b1_plan_view_t) == 336, "unexpected B1 view layout");
_Static_assert(sizeof(b1_plan_record_t) == 720, "unexpected B1 record layout");
_Static_assert(sizeof(b1_plan_header_t) == 474, "unexpected B1 header layout");
_Static_assert(sizeof(b1_plan_footer_t) == 76, "unexpected B1 footer layout");

typedef struct b1_expected_file_s {
    const char *name;
    uint64_t bytes;
    const char *sha256;
} b1_expected_file_t;

typedef struct b1_candidate_buffer_s {
    byte *data;
    size_t bytes;
    size_t capacity;
} b1_candidate_buffer_t;

typedef struct b1_entity_projection_model_s {
    int32_t type;
    bool has_meshes;
    bool present;
} b1_entity_projection_model_t;

typedef struct b1_entity_alignment_calculation_s {
    b1_candidate_buffer_t raw;
    b1_candidate_buffer_t projected;
    uint32_t raw_count;
    uint32_t projected_count;
    byte endpoint_sha256[32];
    byte raw_entities_sha256[32];
    byte projected_entities_sha256[32];
    byte raw_model_sequence_sha256[32];
    byte projected_model_sequence_sha256[32];
    byte raw_membership_sha256[32];
    byte projected_membership_sha256[32];
    byte raw_record_sha256[B1_PHASE5BRPBR2_ENTITY_COUNT][32];
    byte projected_record_sha256[B1_PHASE5BRPBR2_ENTITY_COUNT][32];
} b1_entity_alignment_calculation_t;

typedef struct b1_entity_alignment_authority_s {
    uint32_t version;
    uint32_t publication_count;
    uint32_t revalidation_count;
    uint32_t raw_count;
    uint32_t projected_count;
    uint32_t raw_canonical_bytes;
    uint32_t projected_canonical_bytes;
    int32_t projection_rdflags;
    bool closed;
    byte *raw_entities_v1;
    byte *projected_entities_v1;
    byte endpoint_sha256[32];
    byte raw_entities_sha256[32];
    byte projected_entities_sha256[32];
    byte raw_model_sequence_sha256[32];
    byte projected_model_sequence_sha256[32];
    byte raw_membership_sha256[32];
    byte projected_membership_sha256[32];
    byte raw_record_sha256[B1_PHASE5BRPBR2_ENTITY_COUNT][32];
    byte projected_record_sha256[B1_PHASE5BRPBR2_ENTITY_COUNT][32];
} b1_entity_alignment_authority_t;

typedef struct b1_candidate_seen_s {
    byte digest[32];
    byte *canonical;
    size_t canonical_bytes;
    uint32_t repeat_count;
} b1_candidate_seen_t;

typedef struct b1_endpoint_range_s {
    const char *name;
    int32_t semantic_index;
    uint32_t offset;
    uint32_t bytes;
    byte sha256[32];
} b1_endpoint_range_t;

typedef struct b1_candidate_isolation_ledger_s {
    int sv_running_value;
    int server_state;
    int client_protocol;
    int server_address_type;
    uint64_t hook_complete_notifications;
    bool sv_running_present;
    bool sv_running_zero;
    bool server_state_dead;
    bool server_initialized;
    bool server_client_pool_present;
    bool server_entities_present;
    bool server_storage_absent;
    bool game_export_ge_present;
    bool game_export_gex_present;
    bool game_exports_absent;
    bool client_protocol_zero;
    bool game_module_present;
    bool game_module_absent;
    bool demo_playback_active;
    bool server_address_loopback;
    bool hook_active;
    bool hook_armed;
    bool hook_stop_requested;
    bool pass;
} b1_candidate_isolation_ledger_t;

static const b1_expected_file_t b1_expected_files[] = {
    { "reference-b-paired-current-v1.jsonl", UINT64_C(15028110), "50F99A03EFACDF3438236519E477DE8453D3798BC5B6EF2FC81FF2EADA0049F9" },
    { "reference-b-paired-current-v1.manifest.json", UINT64_C(5676), "811A67EEFB0D88223706AFB38F3C641D7C5C9A9D6187252CC7E1355FBC66817F" },
    { "reference-b-paired-current-v1.verification.json", UINT64_C(6417), "67F13702E12DE59B9B05AAE7BE5666AACC2CD8090EC1AE7E4AAA5B6A50207E33" },
    { "reference-b-paired-current-v1.sha256.txt", UINT64_C(587), "EF2E7529CA2F4CB27A3B8CFCE0D7685C7DDADE1812D16A5514D93649C7A52E93" },
    { "reference-b-paired-current-v1.pipeline-tests.json", UINT64_C(10886), "8BFEF5DA15B8DDF2037A0E116D0DC1F8F6D3E9B20CCFC10AA9663582D5C5955E" },
    { "reference-b-paired-current-v1.acceptance.md", UINT64_C(6589), "54307C36FBD6D4594AA0854529F2E0234BF259CECA3DB1B4127212B97CA1B0FA" },
};

typedef struct b1_runtime_s {
    b1_state_id_t state;
    b1_plan_record_t *records;
    char *settings;
    FILE *log;
    refdef_t frozen_refdef;
    entity_t *entities;
    particle_t *particles;
    dlight_t *dlights;
    lightstyle_t *lightstyles;
    byte *areabits;
    int areabytes;
    byte scene_sha256[32];
    byte scene_component_sha256[B1_PHASE5B_SCENE_COMPONENT_COUNT][32];
    uint32_t scene_component_bytes[B1_PHASE5B_SCENE_COMPONENT_COUNT];
    size_t scene_canonical_bytes;
    byte raw_particle_sha256[32];
    size_t raw_particle_canonical_bytes;
    uint32_t raw_particle_count;
    float raw_particle_default_size;
    b1_role_context_t roles[B1_ROLE_COUNT];
    b1_role_context_t frozen_roles[B1_ROLE_COUNT];
    uint64_t role_physical_frame[B1_ROLE_COUNT];
    uint32_t role_physical_slot[B1_ROLE_COUNT];
    uint32_t logical_index;
    b1_role_t active_role;
    uint64_t capture_request_serial;
    uint64_t pending_capture_request_serial;
    uint64_t pending_capture_physical_frame;
    uint32_t pending_capture_logical_index;
    uint32_t pending_capture_source_frame_id;
    uint32_t pending_capture_role;
    uint32_t pending_capture_physical_slot;
    bool frozen;
    bool descriptor_closed;
    bool temporal_infrastructure_ready;
    bool bootstrap_complete;
    bool settings_monitoring;
    bool pair_started;
    bool frame_pending;
    bool abandoned;
    bool validation_smoke;
    bool bootstrap_probe;
    bool material_secondary_advance_tamper;
    bool scene_admission_witness_authoritative;
    b1_technical_process_kind_t technical_process_kind;
    const char *technical_process_token;
    const char *technical_process_numeric;
    uint64_t technical_process_identity_hash;
    uint32_t capture_scale_pair_count;
    uint64_t capture_scale_identity_hash;
    bool scene_source_ready;
    b1_scene_authority_t source_authority;
    b1_entity_alignment_authority_t entity_alignment_authority;
    struct {
        b1_scene_lifecycle_state_t state;
        uint32_t transition_serial;
        byte transition_digest[32];
        uint64_t invocation_id;
        uint64_t preparation_serial;
        uint32_t preparation_count;
        uint32_t prepared_closure_consumption_count;
        byte consumption_digest[32];
        uint32_t publication_count;
        uint32_t pair_commit_count;
        bool command_buffer_begun;
        byte prepared_closure_sha256[32];
        b1_renderer_scene_closure_t prepared_closure;
        bool prepared_closure_valid;
    } scene_lifecycle;
    uint32_t rendered[B1_ROLE_COUNT];
    uint32_t captured[B1_ROLE_COUNT];
    uint32_t committed_pairs;
    struct {
        uint64_t complete_notifications;
        bool active;
        bool armed;
        bool stop_requested;
    } demo_hook;
    struct {
        FILE *log;
        b1_candidate_seen_t seen[B1_PHASE5A_MAX_CANDIDATES];
        uint64_t message_ordinal;
        uint32_t candidate_count;
        uint32_t repeat_count;
        uint32_t rng_observation;
        int last_frame_number;
        bool enabled;
        bool started;
        bool completed;
        bool last_frame_valid;
        bool failure_written;
        bool configstring_span_observed;
    } candidate;
    struct {
        FILE *log;
        b1_endpoint_range_t ranges[B1_PHASE5AR5_EXPECTED_RANGES];
        uint64_t message_ordinal;
        uint32_t range_count;
        uint32_t rng_observation;
        int last_frame_number;
        bool enabled;
        bool started;
        bool completed;
        bool last_frame_valid;
        bool failure_written;
        bool selected;
    } endpoint;
    struct {
        uint64_t message_ordinal;
        uint64_t accepted_notification_count;
        uint32_t selected_range_count;
        uint32_t observed_lerpfrac_bits;
        int observed_client_time;
        int accepted_server_time;
        int accepted_frame_number;
        byte message_sha256[32];
        byte endpoint_sha256[32];
        byte range_set_sha256[32];
        bool enabled;
        bool selected;
        bool endpoint_closed;
        bool scene_build_consumed;
        bool client_scene_closed;
        bool fixture_preflight_closed;
        bool last_frame_valid;
        int last_frame_number;
    } scene_controller;
    struct {
        b1_phase5brpb_mode_t mode;
        b1_phase5brpb_playback_state_t playback_state;
        b1_phase5brpb_boundary_t boundaries[B1_PHASE5BRPB_MAX_BOUNDARIES];
        b1_candidate_seen_t persistent_seen[B1_PHASE5BRPB_MAX_BOUNDARIES];
        b1_candidate_seen_t semantic_seen[B1_PHASE5BRPB_MAX_BOUNDARIES];
        byte *message_storage;
        byte *config_storage;
        centity_state_t *endpoint_entities;
        entity_t *raw_scene_entities;
        entity_t *scene_entities;
        particle_t *scene_particles;
        dlight_t *scene_dlights;
        lightstyle_t *scene_lightstyles;
        byte *scene_areabits;
        b1_phase5brpb_model_key_t *model_keys;
        b1_phase5brpb_image_key_t *image_keys;
        b1_phase5brpb_sprite_frame_key_t *sprite_frame_keys;
        cvar_t *particle_size_cvar;
        byte *selected_anchor_payload;
        byte *accepted_p0;
        b1_plan_record_t plan_record0;
        byte plan_sha256[32];
        byte fixture_sha256[32];
        byte closure_sha256[32];
        byte executable_sha256[32];
        byte runtime_manifest_sha256[32];
        byte runtime_inventory_sha256[32];
        byte runtime_data_sha256[32];
        byte study_plan_sha256[32];
        byte startup_sha256[32];
        byte r2_expected_entities_sha256[32];
        byte r2_producer_authority_sha256[32];
        byte accepted_p0_sha256[32];
        byte old_component_sha256[B1_PHASE5BRPB_OLD_COMPONENT_COUNT][32];
        uint32_t old_component_bytes[B1_PHASE5BRPB_OLD_COMPONENT_COUNT];
        size_t message_storage_used;
        size_t config_storage_used;
        size_t endpoint_entities_used;
        size_t raw_scene_entities_used;
        size_t scene_entities_used;
        size_t scene_particles_used;
        size_t scene_dlights_used;
        size_t scene_lightstyles_used;
        size_t scene_areabits_used;
        size_t sprite_frame_keys_used;
        size_t selected_anchor_bytes;
        size_t accepted_p0_bytes;
        uint64_t pending_token_id;
        uint64_t next_token_id;
        uint32_t pending_boundary;
        uint32_t message_count;
        uint32_t eligible_message_count;
        uint32_t scene_closure_count;
        uint32_t nonqualifying_message_count;
        uint32_t transparency_ineligible_count;
        uint32_t persistent_mismatch_count;
        uint32_t qualifying_occurrence_count;
        uint32_t recovery_count;
        uint32_t persistent_count;
        uint32_t semantic_count;
        uint32_t selected_anchor_count;
        uint32_t later_message_parse_count;
        uint32_t model_key_count;
        uint32_t image_key_count;
        uint32_t resource_registration_sequence;
        uint32_t rng_perturbation_count;
        uint32_t particle_size_bits;
        int last_frame_number;
        bool enabled;
        bool started;
        bool arena_ready;
        bool resources_latched;
        bool settings_latched;
        bool fixture_preflight_closed;
        bool last_frame_valid;
        bool normal_eof;
        bool controlled_stop_requested;
        bool controlled_disconnect_entered;
        bool output_sealed;
        bool failure_written;
        bool r2_entity_diagnostic;
    } rpb;
} b1_runtime_t;

static b1_runtime_t b1;
static cvar_t *b1_plan;
static cvar_t *b1_plan_sha256;
static cvar_t *b1_stimulus;
static cvar_t *b1_manifest;
static cvar_t *b1_verification;
static cvar_t *b1_sidecar;
static cvar_t *b1_pipeline_tests;
static cvar_t *b1_acceptance;
static cvar_t *b1_output;
static cvar_t *b1_technical_smoke;
static cvar_t *b1_technical_process_kind;
static cvar_t *b1_capture_scale_pair_count;
static cvar_t *b1_scene_admission_witness_authoritative;
static cvar_t *b1_technical_process_kind_early;
static cvar_t *b1_capture_scale_pair_count_early;
static b1_technical_process_kind_t b1_technical_process_kind_early_value;
static const char *b1_technical_process_numeric_early;
static const char *b1_technical_process_token_early;
static uint64_t b1_technical_process_identity_hash_early;
static uint32_t b1_capture_scale_pair_count_early_value;
static uint64_t b1_capture_scale_identity_hash_early;
static bool b1_technical_process_early_latched;
static cvar_t *b1_bootstrap_probe;
static cvar_t *b1_material_secondary_advance_tamper;
static cvar_t *b1_candidate_inventory;
static cvar_t *b1_candidate_inventory_output;
static cvar_t *b1_endpoint_identity;
static cvar_t *b1_endpoint_identity_output;
static cvar_t *b1_scene_controller;
static cvar_t *b1_phase5brpb_mode;
static cvar_t *b1_phase5brpb_output;
static cvar_t *b1_phase5brpb_plan;
static cvar_t *b1_phase5brpb_plan_sha256;
static cvar_t *b1_phase5brpb_p0;
static cvar_t *b1_phase5brpb_p0_sha256;
static cvar_t *b1_phase5brpb_executable_sha256;
static cvar_t *b1_phase5brpb_runtime_manifest_sha256;
static cvar_t *b1_phase5brpb_runtime_inventory_sha256;
static cvar_t *b1_phase5brpb_runtime_data_sha256;
static cvar_t *b1_phase5brpb_study_plan_sha256;
static cvar_t *b1_phase5brpb_startup_sha256;
static cvar_t *b1_phase5brpbr2_diagnostic;
static cvar_t *b1_phase5brpbr2_expected_entities;
static cvar_t *b1_phase5brpbr2_expected_entities_sha256;
static cvar_t *b1_phase5brpbr2_source_revision;
static cvar_t *b1_phase5brpbr2_producer_authority_sha256;
static cvar_t *b1_phase5br_executable_sha256;
static cvar_t *b1_phase5br_runtime_manifest_sha256;
static cvar_t *b1_phase5br_runtime_inventory_sha256;
static cvar_t *b1_status;

static cvar_t *b1_input_cvars[] = {
    NULL, NULL, NULL, NULL, NULL, NULL
};

static void b1_fail(const char *format, ...);
static void b1_candidate_complete_message(void);
static void b1_endpoint_complete_message(void);
static void b1_endpoint_failure_record(const char *code);
static void b1_scene_controller_complete_message(void);
static void b1_phase5brpbr4_close_entity_alignment(
    b1_phase5brpb_boundary_t *boundary);
static void b1_phase5brpb_finish(void);
static void b1_phase5brpb_release(void);
static void b1_phase5brpb_old_v1_crosschecks(
    const b1_phase5brpb_boundary_t *boundary);
static void b1_phase5brpbr2_entity_diagnostic(
    const b1_candidate_buffer_t *observed, const byte observed_sha256[32]);
static void b1_scene_controller_compute_endpoint(const server_frame_t *frame,
    const byte message_digest[32], byte endpoint_digest[32],
    byte range_set_digest[32], size_t *canonical_bytes);
static void b1_apply_committed_snapshot_endpoint(const char *label);
static void b1_scene_vec_f32(b1_candidate_buffer_t *buffer,
                             const float *values, size_t count);
static bool b1_any_request_value(void);
static bool b1_candidate_isolation_ok(const char *context, bool emit_pass);
static bool b1_sha256_file(const char *path, uint64_t *bytes,
                           byte digest[32]);
static void b1_verify_settings_serialization(bool validation_required,
                                             bool monitoring);
static void b1_scene_plan_view(b1_candidate_buffer_t *buffer,
                               const b1_plan_view_t *view);
static void b1_assert_scene_unchanged(const char *boundary);
static bool b1_phase5br_consume_renderer_scene(
    const b1_renderer_scene_closure_t *closure, uint32_t role, void *opaque);

static const char *b1_state_name(b1_state_id_t state)
{
    static const char *names[] = {
        "OFF", "AUTHENTICATING", "READY", "LEFT_RENDER", "LEFT_CAPTURE",
        "RIGHT_RENDER", "RIGHT_CAPTURE", "PAIR_COMMIT", "COMPLETE", "FAILED"
    };
    return (unsigned)state < q_countof(names) ? names[state] : "INVALID";
}

static const char *b1_scene_lifecycle_name(b1_scene_lifecycle_state_t state)
{
    static const char *names[] = {
        "INITIAL", "ENDPOINT_ACCEPTED", "CLIENT_SCENE_FROZEN",
        "TEMPORAL_INFRASTRUCTURE_READY", "RENDERER_SCENE_PREPARED",
        "COMPLETE_SCENE_ACCEPTED", "TRACE_PERMITTED", "PAIR_COMMITTED"
    };
    return (unsigned)state < q_countof(names) ? names[state] : "INVALID";
}

static const char *b1_role_name(b1_role_t role)
{
    return role == B1_ROLE_PRIMARY_LEFT ? "PRIMARY_LEFT" : "PRIMARY_RIGHT";
}

static const char *b1_technical_process_token_for_kind(uint32_t kind)
{
    static const char *tokens[B1_TECHNICAL_PROCESS_COUNT] = {
        "NONE",
        "STATE_P0_P3",
        "FAILED_PAIR_AFTER_LEFT_P1",
        "TONE_POSITIVE_P0",
        "FEEDBACK_FAILURE_P1",
        "STATIC_CORE_SCENE_ADMISSION_WITNESS",
        "CAPTURE_P0_P3",
        "CAPTURE_SCALE_V1"
    };
    return kind < B1_TECHNICAL_PROCESS_COUNT ? tokens[kind] : NULL;
}

static uint64_t b1_technical_process_hash64(uint32_t kind, const char *token)
{
    static const char schema[] = "B1-TECHNICAL-PROCESS-KIND-V1";
    uint64_t hash = UINT64_C(1469598103934665603);
    const byte encoded[4] = {
        (byte)(kind & 0xffu), (byte)((kind >> 8) & 0xffu),
        (byte)((kind >> 16) & 0xffu), (byte)((kind >> 24) & 0xffu)
    };
    const byte *cursor;

    for (cursor = (const byte *)schema; *cursor; cursor++) {
        hash ^= *cursor;
        hash *= UINT64_C(1099511628211);
    }
    for (size_t index = 0; index < sizeof(encoded); index++) {
        hash ^= encoded[index];
        hash *= UINT64_C(1099511628211);
    }
    for (cursor = (const byte *)token; *cursor; cursor++) {
        hash ^= *cursor;
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static b1_technical_process_kind_t b1_parse_technical_process_kind(
    const char *value, const char **numeric, const char **token)
{
    static const char *values[B1_TECHNICAL_PROCESS_COUNT] = {
        "0", "1", "2", "3", "4", "5", "6", "7"
    };
    for (uint32_t kind = 0; kind < B1_TECHNICAL_PROCESS_COUNT; kind++) {
        if (!strcmp(value, values[kind])) {
            *numeric = values[kind];
            *token = b1_technical_process_token_for_kind(kind);
            return (b1_technical_process_kind_t)kind;
        }
    }
    b1_fail("b1_technical_process_kind must be one canonical value 0 through 7");
    return B1_TECHNICAL_PROCESS_NONE;
}

static uint32_t b1_parse_capture_scale_pair_count(const char *value)
{
    uint32_t result = 0;

    if (!value || !value[0])
        b1_fail("b1_capture_scale_pair_count is missing");
    if (!strcmp(value, "0"))
        return 0;
    if (value[0] < '1' || value[0] > '9')
        b1_fail("b1_capture_scale_pair_count is not canonical unsigned base-10");
    for (const char *cursor = value; *cursor; cursor++) {
        uint32_t digit;
        if (*cursor < '0' || *cursor > '9')
            b1_fail("b1_capture_scale_pair_count is not canonical unsigned base-10");
        digit = (uint32_t)(*cursor - '0');
        if (result > (B1_RECORD_COUNT - digit) / 10u)
            b1_fail("b1_capture_scale_pair_count is outside 1 through B1_RECORD_COUNT");
        result = result * 10u + digit;
    }
    if (result == 0 || result > B1_RECORD_COUNT)
        b1_fail("b1_capture_scale_pair_count is outside 1 through B1_RECORD_COUNT");
    return result;
}

static uint64_t b1_capture_scale_hash64(uint32_t kind, uint32_t pair_count)
{
    static const char domain[] = "B1-CAPTURE-SCALE-EXECUTION-V1";
    uint64_t hash = UINT64_C(1469598103934665603);
    byte encoded[8];
    const byte *cursor;

    if (kind != B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1)
        return pair_count == 0 ? 0 : UINT64_MAX;
    for (cursor = (const byte *)domain; *cursor; cursor++) {
        hash ^= *cursor;
        hash *= UINT64_C(1099511628211);
    }
    for (uint32_t index = 0; index < 4u; index++) {
        encoded[index] = (byte)((kind >> (index * 8u)) & 0xffu);
        encoded[index + 4u] =
            (byte)((pair_count >> (index * 8u)) & 0xffu);
    }
    for (size_t index = 0; index < sizeof(encoded); index++) {
        hash ^= encoded[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

void B1_PrivateTechnicalProcessPreinitialise(void)
{
    const int private_flags = CVAR_PRIVATE | CVAR_NOARCHIVE;
    const char *numeric = NULL;
    const char *token = NULL;
    b1_technical_process_kind_t kind;
    uint32_t scale_pair_count;
    uint64_t scale_identity;

    if (!b1_technical_process_kind_early)
        b1_technical_process_kind_early = Cvar_Get(
            "b1_technical_process_kind", "0", private_flags);
    if (!b1_capture_scale_pair_count_early)
        b1_capture_scale_pair_count_early = Cvar_Get(
            "b1_capture_scale_pair_count", "0", private_flags);
    kind = b1_parse_technical_process_kind(
        b1_technical_process_kind_early->string, &numeric, &token);
    scale_pair_count = b1_parse_capture_scale_pair_count(
        b1_capture_scale_pair_count_early->string);
    if ((kind == B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1) !=
        (scale_pair_count != 0))
        b1_fail("b1_capture_scale_pair_count is active for exactly CAPTURE_SCALE_V1");
    scale_identity = b1_capture_scale_hash64((uint32_t)kind,
        scale_pair_count);
    if (b1_technical_process_early_latched) {
        if (kind != b1_technical_process_kind_early_value ||
            strcmp(numeric, b1_technical_process_numeric_early) ||
            strcmp(token, b1_technical_process_token_early) ||
            b1_technical_process_kind_early->modified ||
            b1_capture_scale_pair_count_early->modified ||
            b1_technical_process_identity_hash_early !=
                b1_technical_process_hash64((uint32_t)kind, token) ||
            scale_pair_count != b1_capture_scale_pair_count_early_value ||
            scale_identity != b1_capture_scale_identity_hash_early)
            b1_fail("technical-process/scale identity mutated after the early client latch");
        return;
    }
    b1_technical_process_kind_early_value = kind;
    b1_technical_process_numeric_early = numeric;
    b1_technical_process_token_early = token;
    b1_technical_process_identity_hash_early =
        b1_technical_process_hash64((uint32_t)kind, token);
    b1_capture_scale_pair_count_early_value = scale_pair_count;
    b1_capture_scale_identity_hash_early = scale_identity;
    b1_technical_process_kind_early->modified = false;
    b1_capture_scale_pair_count_early->modified = false;
    b1_technical_process_early_latched = true;
}

static bool b1_technical_process_active(void)
{
    return b1.technical_process_kind != B1_TECHNICAL_PROCESS_NONE;
}

static bool b1_capture_enabled(void)
{
    switch (b1.technical_process_kind) {
    case B1_TECHNICAL_PROCESS_NONE:
        return !b1.validation_smoke;
    case B1_TECHNICAL_PROCESS_CAPTURE_P0_P3:
    case B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1:
        return true;
    case B1_TECHNICAL_PROCESS_STATE_P0_P3:
    case B1_TECHNICAL_PROCESS_FAILED_PAIR_AFTER_LEFT_P1:
    case B1_TECHNICAL_PROCESS_TONE_POSITIVE_P0:
    case B1_TECHNICAL_PROCESS_FEEDBACK_FAILURE_P1:
    case B1_TECHNICAL_PROCESS_STATIC_CORE_SCENE_ADMISSION_WITNESS:
        return false;
    default:
        b1_fail("latched technical-process kind is outside the capture policy");
        return false;
    }
}

static uint32_t b1_required_complete_pairs(void)
{
    switch (b1.technical_process_kind) {
    case B1_TECHNICAL_PROCESS_STATE_P0_P3:
    case B1_TECHNICAL_PROCESS_CAPTURE_P0_P3:
        return 4;
    case B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1:
        return b1.capture_scale_pair_count;
    case B1_TECHNICAL_PROCESS_TONE_POSITIVE_P0:
        return 1;
    case B1_TECHNICAL_PROCESS_FAILED_PAIR_AFTER_LEFT_P1:
    case B1_TECHNICAL_PROCESS_FEEDBACK_FAILURE_P1:
    case B1_TECHNICAL_PROCESS_STATIC_CORE_SCENE_ADMISSION_WITNESS:
        return UINT32_MAX; /* these are required process-terminal paths */
    case B1_TECHNICAL_PROCESS_NONE:
        return b1.validation_smoke ? 1u : B1_RECORD_COUNT;
    default:
        b1_fail("latched technical-process kind is outside the closed enum");
        return UINT32_MAX;
    }
}

uint32_t B1_PrivateTechnicalProcessKind(void)
{
    if (!b1_technical_process_early_latched)
        b1_fail("technical-process identity requested before early client latch");
    return (uint32_t)b1_technical_process_kind_early_value;
}

const char *B1_PrivateTechnicalProcessToken(void)
{
    if (!b1_technical_process_early_latched)
        b1_fail("technical-process token requested before early client latch");
    return b1_technical_process_token_early;
}

uint64_t B1_PrivateTechnicalProcessIdentityHash64(void)
{
    if (!b1_technical_process_early_latched)
        b1_fail("technical-process identity requested before early client latch");
    return b1_technical_process_identity_hash_early;
}

uint32_t B1_PrivateCaptureScalePairCount(void)
{
    if (!b1_technical_process_early_latched)
        b1_fail("capture-scale count requested before early client latch");
    return b1_capture_scale_pair_count_early_value;
}

uint64_t B1_PrivateCaptureScaleIdentityHash64(void)
{
    if (!b1_technical_process_early_latched)
        b1_fail("capture-scale identity requested before early client latch");
    return b1_capture_scale_identity_hash_early;
}

#define B1_CAPTURE_QUIESCENCE_DOMAIN \
    "B1-CAPTURE-COMPLETION-QUIESCENCE-V1"

typedef struct b1_capture_quiescence_s {
    bool active;
    uint64_t sequence;
    uint64_t timestamp_frequency;
    uint64_t first_timestamp;
    uint64_t technical_process_identity_hash64;
    uint64_t correlation_digest64;
    uint64_t previous_record_digest64;
    uint32_t process_id;
    uint32_t technical_process_kind;
    uint32_t capture_scale_pair_count;
    uint32_t logical_index;
    uint32_t source_frame_id;
    uint32_t role;
    uint32_t left_captures;
    uint32_t right_captures;
    uint64_t capture_request_serial;
} b1_capture_quiescence_t;

static b1_capture_quiescence_t b1_capture_quiescence;

static uint64_t b1_capture_quiescence_hash_bytes(
    uint64_t hash, const void *data, size_t bytes)
{
    const byte *cursor = data;
    for (size_t index = 0; index < bytes; index++) {
        hash ^= cursor[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint64_t b1_capture_quiescence_hash_u64(uint64_t hash, uint64_t value)
{
    byte encoded[8];
    for (uint32_t index = 0; index < 8; index++)
        encoded[index] = (byte)((value >> (index * 8u)) & UINT64_C(0xff));
    return b1_capture_quiescence_hash_bytes(hash, encoded, sizeof(encoded));
}

static uint64_t b1_capture_quiescence_hash_string(
    uint64_t hash, const char *value)
{
    return b1_capture_quiescence_hash_bytes(
        hash, value, value ? strlen(value) : 0u);
}

static uint64_t b1_capture_quiescence_timestamp(void)
{
    LARGE_INTEGER counter;
    if (QueryPerformanceCounter(&counter))
        return (uint64_t)counter.QuadPart;
    return GetTickCount64();
}

static void b1_capture_quiescence_activate(void)
{
    LARGE_INTEGER frequency;
    uint64_t hash = UINT64_C(1469598103934665603);
    uint32_t required_pairs = b1_required_complete_pairs();
    if (b1_capture_quiescence.active)
        return;
    if ((b1.technical_process_kind != B1_TECHNICAL_PROCESS_STATE_P0_P3 &&
         b1.technical_process_kind != B1_TECHNICAL_PROCESS_CAPTURE_P0_P3 &&
         b1.technical_process_kind != B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1) ||
        b1.state != B1_STATE_COMPLETE || b1.logical_index != required_pairs ||
        b1.active_role != B1_ROLE_PRIMARY_RIGHT ||
        b1.committed_pairs != required_pairs ||
        b1.rendered[0] != required_pairs ||
        b1.rendered[1] != required_pairs ||
        (b1.technical_process_kind == B1_TECHNICAL_PROCESS_STATE_P0_P3 &&
         (b1.captured[0] != 0u || b1.captured[1] != 0u)) ||
        (b1.technical_process_kind == B1_TECHNICAL_PROCESS_CAPTURE_P0_P3 &&
         (b1.captured[0] != 4u || b1.captured[1] != 4u)))
        return;
    if (b1.technical_process_kind == B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1 &&
        (b1.captured[0] != required_pairs ||
         b1.captured[1] != required_pairs))
        return;
    memset(&b1_capture_quiescence, 0, sizeof(b1_capture_quiescence));
    b1_capture_quiescence.active = true;
    b1_capture_quiescence.process_id = GetCurrentProcessId();
    b1_capture_quiescence.technical_process_kind =
        (uint32_t)b1.technical_process_kind;
    b1_capture_quiescence.technical_process_identity_hash64 =
        b1.technical_process_identity_hash;
    b1_capture_quiescence.capture_scale_pair_count =
        b1.capture_scale_pair_count;
    b1_capture_quiescence.logical_index = b1.logical_index - 1u;
    b1_capture_quiescence.source_frame_id =
        B1_FIRST_SOURCE + b1_capture_quiescence.logical_index;
    b1_capture_quiescence.role = (uint32_t)b1.active_role;
    b1_capture_quiescence.left_captures = b1.captured[0];
    b1_capture_quiescence.right_captures = b1.captured[1];
    b1_capture_quiescence.capture_request_serial = b1.capture_request_serial;
    if (QueryPerformanceFrequency(&frequency))
        b1_capture_quiescence.timestamp_frequency =
            (uint64_t)frequency.QuadPart;
    else
        b1_capture_quiescence.timestamp_frequency = UINT64_C(1000);
    b1_capture_quiescence.first_timestamp =
        b1_capture_quiescence_timestamp();
    hash = b1_capture_quiescence_hash_string(
        hash, B1_CAPTURE_QUIESCENCE_DOMAIN);
    hash = b1_capture_quiescence_hash_u64(
        hash, b1_capture_quiescence.process_id);
    hash = b1_capture_quiescence_hash_u64(
        hash, b1_capture_quiescence.first_timestamp);
    hash = b1_capture_quiescence_hash_u64(
        hash, b1_capture_quiescence.technical_process_kind);
    hash = b1_capture_quiescence_hash_u64(
        hash, b1_capture_quiescence.technical_process_identity_hash64);
    if (b1.technical_process_kind == B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1)
        hash = b1_capture_quiescence_hash_u64(
            hash, b1.capture_scale_identity_hash);
    hash = b1_capture_quiescence_hash_u64(
        hash, b1_capture_quiescence.logical_index);
    hash = b1_capture_quiescence_hash_u64(
        hash, b1_capture_quiescence.source_frame_id);
    hash = b1_capture_quiescence_hash_u64(hash, b1.active_role);
    hash = b1_capture_quiescence_hash_u64(
        hash, b1.capture_request_serial);
    b1_capture_quiescence.correlation_digest64 = hash;
}

static void b1_capture_quiescence_emit(
    const char *phase, const char *source_owner, const char *details)
{
    char line[2048];
    char scale_field[96] = "";
    uint32_t validation_error_count = 0;
    bool device_lost = false;
    bool adapter_copy;
    uint64_t timestamp;
    uint64_t record_digest;
    uint64_t previous_digest;
    int length;

    if (!b1_capture_quiescence.active || !phase || !source_owner || !details)
        return;
    R_B1PrivateCaptureQuiescenceStatus(
        &validation_error_count, &device_lost);
    timestamp = b1_capture_quiescence_timestamp();
    previous_digest = b1_capture_quiescence.previous_record_digest64;
    b1_capture_quiescence.sequence++;
    record_digest = UINT64_C(1469598103934665603);
    record_digest = b1_capture_quiescence_hash_string(
        record_digest, B1_CAPTURE_QUIESCENCE_DOMAIN);
    record_digest = b1_capture_quiescence_hash_u64(
        record_digest, b1_capture_quiescence.correlation_digest64);
    record_digest = b1_capture_quiescence_hash_u64(
        record_digest, previous_digest);
    record_digest = b1_capture_quiescence_hash_u64(
        record_digest, b1_capture_quiescence.sequence);
    record_digest = b1_capture_quiescence_hash_u64(
        record_digest, timestamp);
    record_digest = b1_capture_quiescence_hash_u64(
        record_digest, validation_error_count);
    record_digest = b1_capture_quiescence_hash_u64(
        record_digest, device_lost ? 1u : 0u);
    record_digest = b1_capture_quiescence_hash_string(record_digest, phase);
    record_digest = b1_capture_quiescence_hash_string(
        record_digest, source_owner);
    record_digest = b1_capture_quiescence_hash_string(record_digest, details);
    adapter_copy = b1.log != NULL;
    if (b1_capture_quiescence.technical_process_kind ==
        B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1)
        Q_snprintf(scale_field, sizeof(scale_field),
            ",\"captureScalePairCount\":%u",
            b1_capture_quiescence.capture_scale_pair_count);
    length = Q_snprintf(line, sizeof(line),
        "{\"event\":\"b1_capture_completion_quiescence_v1\","
        "\"schemaVersion\":1,\"domain\":\"%s\","
        "\"phase\":\"%s\",\"sequence\":%llu,"
        "\"timestampTicks\":%llu,\"timestampFrequency\":%llu,"
        "\"correlationSeedTimestampTicks\":%llu,"
        "\"processId\":%u,\"technicalProcessKind\":%u,"
        "\"technicalProcessIdentityHash64\":\"%016llx\""
        "%s,\"captureComplete\":true,\"logicalIndex\":%u,"
        "\"sourceFrameId\":%u,\"roleIndex\":%u,\"role\":\"%s\","
        "\"leftCaptures\":%u,\"rightCaptures\":%u,"
        "\"captureRequestSerial\":%llu,"
        "\"validationErrorCount\":%u,\"deviceLost\":%s,"
        "\"correlationDigest64\":\"%016llx\","
        "\"previousRecordDigest64\":\"%016llx\","
        "\"recordDigest64\":\"%016llx\","
        "\"sourceOwner\":\"%s\",\"adapterCopy\":%s,"
        "\"rawHandlesInPortableIdentity\":false,\"details\":%s}",
        B1_CAPTURE_QUIESCENCE_DOMAIN, phase,
        (unsigned long long)b1_capture_quiescence.sequence,
        (unsigned long long)timestamp,
        (unsigned long long)b1_capture_quiescence.timestamp_frequency,
        (unsigned long long)b1_capture_quiescence.first_timestamp,
        b1_capture_quiescence.process_id,
        b1_capture_quiescence.technical_process_kind,
        (unsigned long long)
            b1_capture_quiescence.technical_process_identity_hash64,
        scale_field,
        b1_capture_quiescence.logical_index,
        b1_capture_quiescence.source_frame_id,
        b1_capture_quiescence.role,
        b1_role_name((b1_role_t)b1_capture_quiescence.role),
        b1_capture_quiescence.left_captures,
        b1_capture_quiescence.right_captures,
        (unsigned long long)b1_capture_quiescence.capture_request_serial,
        validation_error_count, device_lost ? "true" : "false",
        (unsigned long long)b1_capture_quiescence.correlation_digest64,
        (unsigned long long)previous_digest,
        (unsigned long long)record_digest,
        source_owner, adapter_copy ? "true" : "false", details);
    if (length < 0 || (size_t)length >= sizeof(line))
        return;
    b1_capture_quiescence.previous_record_digest64 = record_digest;
    if (adapter_copy) {
        fprintf(b1.log, "%s\n", line);
        fflush(b1.log);
    }
    Com_Printf("B1_CAPTURE_COMPLETION_QUIESCENCE_V1 %s\n", line);
}

void B1_PrivateCaptureQuiescenceQuitEnqueued(void)
{
    b1_capture_quiescence_activate();
    b1_capture_quiescence_emit(
        "QUIT_ENQUEUED", "src/client/reference_b_b1.c:B1_BeforeBeginFrame",
        "{}");
}

void B1_PrivateCaptureQuiescenceMarker(
    const char *phase, const char *source_owner)
{
    b1_capture_quiescence_emit(phase, source_owner, "{}");
}

void B1_PrivateCaptureQuiescenceReadbackState(
    uint32_t active_phase, bool readback_mapped,
    uint64_t retired_request_serial, bool screenshot_image_allocated,
    bool screenshot_memory_allocated, uint64_t screenshot_memory_bytes,
    bool temporal_activated, bool initial_release_idle_required,
    bool expected_stop_idle_required, bool unconsumed_acquire_fatal_required,
    bool state_witness_destroy_required,
    uint64_t pending_client_request_serial,
    uint32_t pending_client_logical_index,
    uint32_t pending_client_source_frame_id,
    uint32_t pending_client_role)
{
    char details[768];
    int length = Q_snprintf(details, sizeof(details),
        "{\"activeCapturePhase\":%u,\"activeCapturePhaseEmpty\":%s,"
        "\"readbackMapped\":%s,\"retiredRequestSerial\":%llu,"
        "\"retiredScalarOnly\":%s,\"screenshotImageAllocated\":%s,"
        "\"screenshotMemoryAllocated\":%s,"
        "\"screenshotMemoryBytes\":%llu,\"temporalActivated\":%s,"
        "\"initialReleaseIdleRequired\":%s,"
        "\"expectedStopIdleRequired\":%s,"
        "\"unconsumedAcquireFatalRequired\":%s,"
        "\"stateWitnessDestroyRequired\":%s,"
        "\"pendingClientRequestSerial\":%llu,"
        "\"pendingClientLogicalIndex\":%u,"
        "\"pendingClientSourceFrameId\":%u,"
        "\"pendingClientRole\":%u}",
        active_phase, active_phase == 0u ? "true" : "false",
        readback_mapped ? "true" : "false",
        (unsigned long long)retired_request_serial,
        (active_phase == 0u && !readback_mapped) ? "true" : "false",
        screenshot_image_allocated ? "true" : "false",
        screenshot_memory_allocated ? "true" : "false",
        (unsigned long long)screenshot_memory_bytes,
        temporal_activated ? "true" : "false",
        initial_release_idle_required ? "true" : "false",
        expected_stop_idle_required ? "true" : "false",
        unconsumed_acquire_fatal_required ? "true" : "false",
        state_witness_destroy_required ? "true" : "false",
        (unsigned long long)pending_client_request_serial,
        pending_client_logical_index, pending_client_source_frame_id,
        pending_client_role);
    if (length < 0 || (size_t)length >= sizeof(details))
        return;
    b1_capture_quiescence_emit(
        "PRE_PRESENT_READBACK_STATE_AT_SHUTDOWN",
        "src/refresh/vkpt/main.c:R_B1PrivateCaptureQuiescenceReadbackState",
        details);
}

void B1_PrivateCaptureQuiescenceResourceStateMarker(
    const char *phase, const char *source_owner,
    bool screenshot_image_allocated, bool screenshot_memory_allocated,
    uint64_t screenshot_memory_bytes, bool swapchain_live,
    bool device_live)
{
    char details[384];
    int length = Q_snprintf(details, sizeof(details),
        "{\"screenshotImageAllocated\":%s,"
        "\"screenshotMemoryAllocated\":%s,"
        "\"screenshotMemoryBytes\":%llu,\"swapchainLive\":%s,"
        "\"deviceLive\":%s}",
        screenshot_image_allocated ? "true" : "false",
        screenshot_memory_allocated ? "true" : "false",
        (unsigned long long)screenshot_memory_bytes,
        swapchain_live ? "true" : "false", device_live ? "true" : "false");
    if (length < 0 || (size_t)length >= sizeof(details))
        return;
    b1_capture_quiescence_emit(phase, source_owner, details);
}

void B1_PrivateTechnicalExpectedStop(uint32_t kind, uint32_t logical_index)
{
    const char *expected_failure;
    const char *failure_boundary;

    if (kind != B1_TECHNICAL_PROCESS_FAILED_PAIR_AFTER_LEFT_P1 &&
        kind != B1_TECHNICAL_PROCESS_FEEDBACK_FAILURE_P1)
        b1_fail("technical expected-stop callback used for a non-terminal process kind");
    if (!B1_Requested() || !B1_Ready() || !b1.log)
        b1_fail("technical expected-stop callback arrived outside authenticated B1");
    B1_VerifySettings();
    if ((uint32_t)b1.technical_process_kind != kind ||
        logical_index != 1u || b1.logical_index != logical_index)
        b1_fail("technical expected-stop kind or logical index differs");
    if (b1.state != B1_STATE_LEFT_RENDER ||
        b1.active_role != B1_ROLE_PRIMARY_LEFT || !b1.pair_started ||
        b1.frame_pending || b1.committed_pairs != 1u ||
        b1.rendered[B1_ROLE_PRIMARY_LEFT] != 1u ||
        b1.rendered[B1_ROLE_PRIMARY_RIGHT] != 1u ||
        b1.captured[B1_ROLE_PRIMARY_LEFT] != 0u ||
        b1.captured[B1_ROLE_PRIMARY_RIGHT] != 0u ||
        !b1.roles[B1_ROLE_PRIMARY_LEFT].pending_valid ||
        b1.roles[B1_ROLE_PRIMARY_RIGHT].pending_valid ||
        !b1.roles[B1_ROLE_PRIMARY_LEFT].temporal_valid ||
        !b1.roles[B1_ROLE_PRIMARY_RIGHT].temporal_valid ||
        b1.roles[B1_ROLE_PRIMARY_LEFT].committed_logical != 0u ||
        b1.roles[B1_ROLE_PRIMARY_RIGHT].committed_logical != 0u)
        b1_fail("technical expected-stop callback arrived from an illegal client state");

    if (kind == B1_TECHNICAL_PROCESS_FAILED_PAIR_AFTER_LEFT_P1) {
        expected_failure = "EXPECTED_PARTIAL_PAIR_STOP";
        failure_boundary = "AFTER_PRIMARY_LEFT_BEFORE_PRIMARY_RIGHT";
        b1.rendered[B1_ROLE_PRIMARY_LEFT]++;
    } else {
        expected_failure = "EXPECTED_FEEDBACK_READBACK_FAILURE";
        failure_boundary = "PROCESS_RENDER_FEEDBACK_ENTRY_BEFORE_READBACK";
    }

    if (fprintf(b1.log,
                "{\"event\":\"b1_technical_expected_failure_receipt\","
                "\"technicalProcessKind\":%u,\"technicalProcessNumeric\":\"%s\","
                "\"technicalProcessToken\":\"%s\","
                "\"technicalProcessIdentityHash64\":\"%016llx\","
                "\"expectedFailure\":\"%s\",\"failureLogicalIndex\":%u,"
                "\"failureBoundary\":\"%s\",\"committedPairs\":1,"
                "\"roleRendersAtTerminal\":[%u,%u],\"capturesPerRole\":[0,0],"
                "\"pairPublished\":false,\"rollback\":false,"
                "\"continuation\":false,\"requiredProcessResult\":\"NON_ZERO\","
                "\"terminal\":true}\n",
                kind, b1.technical_process_numeric,
                b1.technical_process_token,
                (unsigned long long)b1.technical_process_identity_hash,
                expected_failure, logical_index, failure_boundary,
                b1.rendered[B1_ROLE_PRIMARY_LEFT],
                b1.rendered[B1_ROLE_PRIMARY_RIGHT]) < 0 || fflush(b1.log))
        b1_fail("cannot persist technical expected-failure receipt");
    b1_fail("expected technical process %s reached its terminal boundary",
            b1.technical_process_token);
}

static void b1_set_state(b1_state_id_t state)
{
    b1.state = state;
    if (b1_status)
        Cvar_SetByVar(b1_status, b1_state_name(state), FROM_CODE);
}

static void b1_fail(const char *format, ...)
{
    char message[MAX_STRING_CHARS];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    message[sizeof(message) - 1] = 0;

    if (b1.pair_started) {
        b1.abandoned = true;
        if (b1.log)
            fprintf(b1.log,
                    "{\"event\":\"abandoned\",\"logicalIndex\":%u,"
                    "\"role\":\"%s\",\"committedPairs\":%u}\n",
                    b1.logical_index, b1_role_name(b1.active_role), b1.committed_pairs);
    }
    b1_set_state(B1_STATE_FAILED);
    if (b1.log) {
        fprintf(b1.log, "{\"event\":\"failure\",\"code\":\"B1_FAIL\"}\n");
        if (b1.scene_lifecycle.transition_serial &&
            b1.scene_lifecycle.state < B1_SCENE_PAIR_COMMITTED) {
            fprintf(b1.log,
                    "{\"event\":\"phase5br_lifecycle_failure\","
                    "\"schemaVersion\":1,\"failedState\":\"%s\","
                    "\"reasonCode\":\"B1_FAIL\",\"commandBufferBegun\":%s,"
                    "\"preparedClosureConsumptionCount\":%u,"
                    "\"historyReadCount\":0,\"publicationCount\":%u,"
                    "\"pairCommitCount\":%u,"
                    "\"invalidCommandBufferSubmitted\":false,\"terminal\":true}\n",
                    b1_scene_lifecycle_name(b1.scene_lifecycle.state),
                    b1.scene_lifecycle.command_buffer_begun ? "true" : "false",
                    b1.scene_lifecycle.prepared_closure_consumption_count,
                    b1.scene_lifecycle.publication_count,
                    b1.scene_lifecycle.pair_commit_count);
        }
        fflush(b1.log);
    }
    Com_Error(ERR_FATAL, "B1_FAIL: %s", message);
}

static void b1_phase4_runtime_status_f(void)
{
    int running = sv_running ? sv_running->integer : INT_MIN;
    bool server_dead = sv.state == ss_dead;
    bool server_storage_empty = !svs.initialized && !svs.client_pool && !svs.entities;
    bool game_exports_absent = !ge && !gex;
    bool client_netchan_absent = cls.netchan.protocol == 0;
    bool game_module_absent = GetModuleHandleW(L"gamex86_64.dll") == NULL;
    bool playback_active = cls.demo.playback != 0;
    bool playback_loopback = cls.serverAddress.type == NA_LOOPBACK;
    bool loopback_bookkeeping_only = playback_active && playback_loopback &&
        client_netchan_absent && running == 0 && server_dead && server_storage_empty;
    bool hook_unarmed = b1.demo_hook.active && !b1.demo_hook.armed &&
        !b1.demo_hook.stop_requested && !b1.demo_hook.complete_notifications;
    bool pass = running == 0 && server_dead && server_storage_empty &&
        game_exports_absent && client_netchan_absent && game_module_absent &&
        loopback_bookkeeping_only && hook_unarmed;

    Com_Printf(
        "B1_PHASE4_RUNTIME {\"pass\":%s,\"svRunning\":%d,"
        "\"serverStateDead\":%s,\"serverStorageEmpty\":%s,"
        "\"gameExportsAbsent\":%s,\"clientNetchanProtocol\":%d,"
        "\"gameModuleAbsent\":%s,\"demoPlaybackActive\":%s,"
        "\"serverAddressLoopback\":%s,\"hookActive\":%s,"
        "\"hookArmed\":%s,\"completeNotifications\":%llu,"
        "\"stopRequested\":%s,\"loopbackBookkeepingOnly\":%s}\n",
        pass ? "true" : "false", running,
        server_dead ? "true" : "false",
        server_storage_empty ? "true" : "false",
        game_exports_absent ? "true" : "false",
        cls.netchan.protocol,
        game_module_absent ? "true" : "false",
        playback_active ? "true" : "false",
        playback_loopback ? "true" : "false",
        b1.demo_hook.active ? "true" : "false",
        b1.demo_hook.armed ? "true" : "false",
        (unsigned long long)b1.demo_hook.complete_notifications,
        b1.demo_hook.stop_requested ? "true" : "false",
        loopback_bookkeeping_only ? "true" : "false");
    if (!pass)
        b1_fail("Phase-4 ordinary-demo runtime isolation proof failed");
}

static const cmdreg_t b1_commands[] = {
    { "b1_phase4_runtime_status", b1_phase4_runtime_status_f },
    { NULL }
};

static int b1_hex_value(int value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static bool b1_parse_sha256(const char *text, byte digest[32])
{
    if (!text || strlen(text) != 64)
        return false;
    for (int i = 0; i < 32; i++) {
        int hi = b1_hex_value(text[i * 2]);
        int lo = b1_hex_value(text[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return false;
        digest[i] = (byte)((hi << 4) | lo);
    }
    return true;
}

static bool b1_sha256_data(const void *data, size_t bytes, byte digest[32])
{
    BCRYPT_ALG_HANDLE algorithm = NULL;
    BCRYPT_HASH_HANDLE hash = NULL;
    PUCHAR object = NULL;
    DWORD object_size = 0;
    DWORD result_size = 0;
    bool ok = false;

    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0) < 0)
        goto done;
    if (BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, (PUCHAR)&object_size,
                          sizeof(object_size), &result_size, 0) < 0)
        goto done;
    object = Z_Malloc(object_size);
    if (BCryptCreateHash(algorithm, &hash, object, object_size, NULL, 0, 0) < 0)
        goto done;
    if (bytes && BCryptHashData(hash, (PUCHAR)data, (ULONG)bytes, 0) < 0)
        goto done;
    if (BCryptFinishHash(hash, digest, 32, 0) < 0)
        goto done;
    ok = true;
done:
    if (hash) BCryptDestroyHash(hash);
    if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
    if (object) Z_Free(object);
    return ok;
}

static void *b1_copy_required(const void *source, size_t bytes, const char *label)
{
    void *copy;
    if (!source || !bytes)
        b1_fail("missing frozen %s", label);
    copy = Z_Malloc(bytes);
    memcpy(copy, source, bytes);
    return copy;
}

static void b1_scene_sha256(byte digest[32],
                            byte component_digest[B1_PHASE5B_SCENE_COMPONENT_COUNT][32],
                            uint32_t component_bytes[B1_PHASE5B_SCENE_COMPONENT_COUNT],
                            size_t *canonical_bytes);

static void b1_format_sha256(const byte digest[32], char text[65])
{
    static const char digits[] = "0123456789ABCDEF";
    for (int i = 0; i < 32; i++) {
        text[i * 2] = digits[digest[i] >> 4];
        text[i * 2 + 1] = digits[digest[i] & 15];
    }
    text[64] = 0;
}

static bool b1_scene_authority_equal(const b1_scene_authority_t *left,
                                     const b1_scene_authority_t *right)
{
    return left && right &&
        !memcmp(left->fixture_sha256, right->fixture_sha256, 32) &&
        !memcmp(left->plan_sha256, right->plan_sha256, 32) &&
        !memcmp(left->input_closure_sha256,
                right->input_closure_sha256, 32) &&
        !memcmp(left->executable_sha256, right->executable_sha256, 32) &&
        !memcmp(left->runtime_manifest_sha256,
                right->runtime_manifest_sha256, 32) &&
        !memcmp(left->runtime_inventory_sha256,
                right->runtime_inventory_sha256, 32) &&
        !memcmp(left->selected_message_sha256,
                right->selected_message_sha256, 32) &&
        !memcmp(left->range_set_sha256, right->range_set_sha256, 32) &&
        !memcmp(left->endpoint_sha256, right->endpoint_sha256, 32) &&
        !memcmp(left->client_scene_sha256, right->client_scene_sha256, 32) &&
        left->logical_index == right->logical_index &&
        left->source_frame_id == right->source_frame_id &&
        left->scene_scope == right->scene_scope &&
        left->scene_scope_generation == right->scene_scope_generation &&
        left->effective_particle_count == right->effective_particle_count;
}

static void b1_phase5br_recheck_source_authority(const char *boundary,
                                                  bool emit)
{
    byte plan_digest[32], executable[32], runtime_manifest[32], runtime_inventory[32];
    byte selected_message[32], range_set[32], endpoint[32], fixture[32];
    byte zero[32] = { 0 };
    char fixture_text[65], plan_text[65], input_closure_text[65];
    char executable_text[65], runtime_manifest_text[65], runtime_inventory_text[65];
    char selected_message_text[65], range_set_text[65];
    char endpoint_text[65], client_scene_text[65];

    if (!boundary || !b1_phase5br_executable_sha256 ||
        !b1_phase5br_runtime_manifest_sha256 ||
        !b1_phase5br_runtime_inventory_sha256 ||
        !b1_plan || !b1_plan_sha256 || b1_plan->modified ||
        b1_plan_sha256->modified ||
        b1_phase5br_executable_sha256->modified ||
        b1_phase5br_runtime_manifest_sha256->modified ||
        b1_phase5br_runtime_inventory_sha256->modified ||
        !b1_parse_sha256(b1_plan_sha256->string, plan_digest) ||
        !b1_parse_sha256(b1_phase5br_executable_sha256->string, executable) ||
        !b1_parse_sha256(b1_phase5br_runtime_manifest_sha256->string,
                         runtime_manifest) ||
        !b1_parse_sha256(b1_phase5br_runtime_inventory_sha256->string,
                         runtime_inventory) ||
        !b1_parse_sha256(B1_PHASE5AR5_SELECTED_MESSAGE_SHA256,
                         selected_message) ||
        !b1_parse_sha256(B1_PHASE5AR5_ACCEPTED_RANGE_SET_SHA256, range_set) ||
        !b1_parse_sha256(B1_PHASE5AR5_ACCEPTED_ENDPOINT_SHA256, endpoint) ||
        !b1_parse_sha256(B1_PHASE5B_FIXTURE_SHA256, fixture) ||
        memcmp(plan_digest, b1.source_authority.plan_sha256, 32) ||
        !memcmp(executable, zero, 32) || !memcmp(runtime_manifest, zero, 32) ||
        !memcmp(runtime_inventory, zero, 32) ||
        memcmp(executable, b1.source_authority.executable_sha256, 32) ||
        memcmp(runtime_manifest,
               b1.source_authority.runtime_manifest_sha256, 32) ||
        memcmp(runtime_inventory,
               b1.source_authority.runtime_inventory_sha256, 32) ||
        memcmp(selected_message,
               b1.source_authority.selected_message_sha256, 32) ||
        memcmp(range_set, b1.source_authority.range_set_sha256, 32) ||
        memcmp(endpoint, b1.source_authority.endpoint_sha256, 32) ||
        memcmp(fixture, b1.source_authority.fixture_sha256, 32) ||
        !memcmp(b1.source_authority.plan_sha256, zero, 32) ||
        !memcmp(b1.source_authority.input_closure_sha256, zero, 32) ||
        b1.source_authority.logical_index != 0u ||
        b1.source_authority.source_frame_id != B1_FIRST_SOURCE ||
        b1.source_authority.scene_scope !=
            B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_V1 ||
        b1.source_authority.scene_scope_generation !=
            B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_GENERATION ||
        b1.source_authority.effective_particle_count != 0u ||
        (!b1.scene_lifecycle.state &&
         memcmp(b1.source_authority.client_scene_sha256, zero, 32)) ||
        (b1.scene_lifecycle.state &&
         memcmp(b1.source_authority.client_scene_sha256, b1.scene_sha256, 32)))
        b1_fail("B1 Phase-5B-R source authority changed at %s", boundary);
    if (!emit)
        return;
    b1_format_sha256(b1.source_authority.fixture_sha256, fixture_text);
    b1_format_sha256(b1.source_authority.plan_sha256, plan_text);
    b1_format_sha256(b1.source_authority.input_closure_sha256,
                     input_closure_text);
    b1_format_sha256(b1.source_authority.executable_sha256, executable_text);
    b1_format_sha256(b1.source_authority.runtime_manifest_sha256,
                     runtime_manifest_text);
    b1_format_sha256(b1.source_authority.runtime_inventory_sha256,
                     runtime_inventory_text);
    b1_format_sha256(b1.source_authority.selected_message_sha256,
                     selected_message_text);
    b1_format_sha256(b1.source_authority.range_set_sha256, range_set_text);
    b1_format_sha256(b1.source_authority.endpoint_sha256, endpoint_text);
    b1_format_sha256(b1.source_authority.client_scene_sha256,
                     client_scene_text);
    if (!b1.log || fprintf(b1.log,
            "{\"event\":\"phase5br_source_authority\",\"schemaVersion\":2,"
            "\"boundary\":\"%s\",\"fixtureSha256\":\"%s\","
            "\"planSha256\":\"%s\",\"executableSha256\":\"%s\","
            "\"inputClosureSha256\":\"%s\","
            "\"runtimeManifestSha256\":\"%s\","
            "\"runtimeInventorySha256\":\"%s\","
            "\"selectedMessageSha256\":\"%s\",\"rangeSetSha256\":\"%s\","
            "\"endpointSha256\":\"%s\",\"clientSceneSha256\":\"%s\","
            "\"logicalIndex\":%u,\"sourceFrameId\":%u,"
            "\"sceneScope\":\"%s\",\"sceneScopeValue\":%u,"
            "\"sceneScopeGeneration\":%u,\"effectiveParticleCount\":%u,"
            "\"cvarsUnmodified\":true,\"pass\":true}\n",
            boundary, fixture_text, plan_text, executable_text,
            input_closure_text, runtime_manifest_text, runtime_inventory_text,
            selected_message_text, range_set_text, endpoint_text, client_scene_text,
            b1.source_authority.logical_index,
            b1.source_authority.source_frame_id, B1_STATIC_CORE_SCOPE_TOKEN,
            b1.source_authority.scene_scope,
            b1.source_authority.scene_scope_generation,
            b1.source_authority.effective_particle_count) < 0 || fflush(b1.log))
        b1_fail("cannot persist B1 Phase-5B-R source authority");
}

static void b1_candidate_failure_record(const char *code)
{
    if (!b1.candidate.enabled || b1.candidate.failure_written)
        return;
    b1.candidate.failure_written = true;
    if (b1.candidate.log) {
        fprintf(b1.candidate.log,
                "{\"event\":\"candidate_inventory_failure\",\"schemaVersion\":1,"
                "\"code\":\"%s\",\"completed\":false,"
                "\"candidateAcceptanceAuthorised\":false,"
                "\"sceneSourceReady\":false,\"temporalActivated\":false}\n",
                code);
        fflush(b1.candidate.log);
    }
}

static void b1_candidate_fail(const char *code, const char *message)
{
    if (b1.rpb.enabled) {
        b1_fail("Phase-5B-RP-B %s: %s", code, message);
        return;
    }
    if (b1.endpoint.enabled) {
        b1_endpoint_failure_record(code);
        b1_fail("Phase-5A-R5 %s: %s", code, message);
        return;
    }
    b1_candidate_failure_record(code);
    b1_fail("Phase-5A %s: %s", code, message);
}

static void b1_candidate_buffer_release(b1_candidate_buffer_t *buffer)
{
    if (buffer->data)
        Z_Free(buffer->data);
    memset(buffer, 0, sizeof(*buffer));
}

static void b1_candidate_buffer_reserve(b1_candidate_buffer_t *buffer, size_t added)
{
    size_t required;
    size_t capacity;
    byte *replacement;

    if (added > B1_PHASE5A_MAX_CANONICAL_BYTES - buffer->bytes)
        b1_candidate_fail("B1_PHASE5A_CANONICAL_BOUNDS",
                          "canonical serialisation exceeded its exact bound");
    required = buffer->bytes + added;
    if (required <= buffer->capacity)
        return;
    capacity = buffer->capacity ? buffer->capacity : 4096u;
    while (capacity < required) {
        if (capacity > B1_PHASE5A_MAX_CANONICAL_BYTES / 2u) {
            capacity = B1_PHASE5A_MAX_CANONICAL_BYTES;
            break;
        }
        capacity *= 2u;
    }
    replacement = Z_Malloc(capacity);
    if (buffer->bytes)
        memcpy(replacement, buffer->data, buffer->bytes);
    if (buffer->data)
        Z_Free(buffer->data);
    buffer->data = replacement;
    buffer->capacity = capacity;
}

static void b1_candidate_append(b1_candidate_buffer_t *buffer,
                                const void *data, size_t bytes)
{
    b1_candidate_buffer_reserve(buffer, bytes);
    if (bytes)
        memcpy(buffer->data + buffer->bytes, data, bytes);
    buffer->bytes += bytes;
}

static void b1_candidate_u8(b1_candidate_buffer_t *buffer, uint8_t value)
{
    b1_candidate_append(buffer, &value, sizeof(value));
}

static void b1_candidate_u16(b1_candidate_buffer_t *buffer, uint16_t value)
{
    const byte encoded[2] = { (byte)value, (byte)(value >> 8) };
    b1_candidate_append(buffer, encoded, sizeof(encoded));
}

static void b1_candidate_i16(b1_candidate_buffer_t *buffer, int16_t value)
{
    b1_candidate_u16(buffer, (uint16_t)value);
}

static void b1_candidate_u32(b1_candidate_buffer_t *buffer, uint32_t value)
{
    const byte encoded[4] = {
        (byte)value, (byte)(value >> 8), (byte)(value >> 16), (byte)(value >> 24)
    };
    b1_candidate_append(buffer, encoded, sizeof(encoded));
}

static void b1_candidate_i32(b1_candidate_buffer_t *buffer, int32_t value)
{
    b1_candidate_u32(buffer, (uint32_t)value);
}

static void b1_candidate_u64(b1_candidate_buffer_t *buffer, uint64_t value)
{
    const byte encoded[8] = {
        (byte)value, (byte)(value >> 8), (byte)(value >> 16), (byte)(value >> 24),
        (byte)(value >> 32), (byte)(value >> 40), (byte)(value >> 48),
        (byte)(value >> 56)
    };
    b1_candidate_append(buffer, encoded, sizeof(encoded));
}

static void b1_candidate_i64(b1_candidate_buffer_t *buffer, int64_t value)
{
    b1_candidate_u64(buffer, (uint64_t)value);
}

static void b1_candidate_f32(b1_candidate_buffer_t *buffer, float value)
{
    uint32_t bits;
    if (!isfinite(value))
        b1_candidate_fail("B1_PHASE5A_NONFINITE", "non-finite semantic float");
    memcpy(&bits, &value, sizeof(bits));
    b1_candidate_u32(buffer, bits);
}

static void b1_candidate_string_bytes(b1_candidate_buffer_t *buffer,
                                      const char *value, size_t bytes)
{
    if (!value || bytes > UINT32_MAX)
        b1_candidate_fail("B1_PHASE5A_STRING", "semantic string is malformed");
    b1_candidate_u32(buffer, (uint32_t)bytes);
    b1_candidate_append(buffer, value, bytes);
}

static void b1_candidate_host_string(b1_candidate_buffer_t *buffer,
                                     const char *value, size_t extent)
{
    const char *end;
    if (!value || !extent || extent > UINT32_MAX)
        b1_candidate_fail("B1_PHASE5A_STRING", "fixed host string bound is malformed");
    end = memchr(value, 0, extent);
    if (!end)
        b1_candidate_fail("B1_PHASE5A_STRING", "fixed host string lacks a bounded terminator");
    b1_candidate_string_bytes(buffer, value, (size_t)(end - value));
}

static b1_phase5brpb_mode_t b1_phase5brpb_parse_mode(const char *value)
{
    if (!value || !strcmp(value, "0"))
        return B1_PHASE5BRPB_MODE_OFF;
    if (!strcmp(value, "p0"))
        return B1_PHASE5BRPB_MODE_P0;
    if (!strcmp(value, "enumerate"))
        return B1_PHASE5BRPB_MODE_ENUMERATE;
    b1_fail("b1_phase5brpb_mode must be exactly 0, p0 or enumerate");
    return B1_PHASE5BRPB_MODE_OFF;
}

static void *b1_phase5brpb_allocate(size_t count, size_t size,
                                    const char *label)
{
    if (!count || !size || count > SIZE_MAX / size)
        b1_fail("Phase-5B-RP-B %s capacity is malformed", label);
    return Z_Mallocz(count * size);
}

static void b1_phase5brpb_allocate_arenas(void)
{
    if (!b1.rpb.enabled || b1.rpb.arena_ready)
        b1_fail("Phase-5B-RP-B arena allocation entered from an illegal state");
    b1.rpb.message_storage = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES, MAX_MSGLEN, "message");
    b1.rpb.config_storage = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES, sizeof(cl.configstrings), "configstring");
    b1.rpb.endpoint_entities = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES * (size_t)MAX_PACKET_ENTITIES,
        sizeof(*b1.rpb.endpoint_entities), "endpoint-entity");
    b1.rpb.raw_scene_entities = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES * (size_t)MAX_ENTITIES,
        sizeof(*b1.rpb.raw_scene_entities), "raw-scene-entity");
    b1.rpb.scene_entities = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES * (size_t)MAX_ENTITIES,
        sizeof(*b1.rpb.scene_entities), "scene-entity");
    b1.rpb.scene_particles = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES * (size_t)MAX_PARTICLES,
        sizeof(*b1.rpb.scene_particles), "scene-particle");
    b1.rpb.scene_dlights = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES * (size_t)MAX_DLIGHTS,
        sizeof(*b1.rpb.scene_dlights), "scene-dlight");
    b1.rpb.scene_lightstyles = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES * (size_t)MAX_LIGHTSTYLES,
        sizeof(*b1.rpb.scene_lightstyles), "scene-lightstyle");
    b1.rpb.scene_areabits = b1_phase5brpb_allocate(
        B1_PHASE5BRPB_MAX_BOUNDARIES, MAX_MAP_AREA_BYTES, "scene-area");
    b1.rpb.model_keys = b1_phase5brpb_allocate(
        (size_t)MAX_MODELS + 1u, sizeof(*b1.rpb.model_keys), "model-key");
    b1.rpb.image_keys = b1_phase5brpb_allocate(
        MAX_RIMAGES, sizeof(*b1.rpb.image_keys), "image-key");
    b1.rpb.sprite_frame_keys = b1_phase5brpb_allocate(
        ((size_t)MAX_MODELS + 1u) * SP2_MAX_FRAMES,
        sizeof(*b1.rpb.sprite_frame_keys), "sprite-frame-key");
    b1.rpb.arena_ready = true;
}

static void b1_phase5brpb_copy_bounded_string(char *destination,
                                               size_t destination_size,
                                               const char *source,
                                               size_t source_bound,
                                               const char *label)
{
    const char *end;
    size_t bytes;
    if (!destination || destination_size < 2u || !source || !source_bound)
        b1_fail("Phase-5B-RP-B %s string boundary is malformed", label);
    end = memchr(source, 0, source_bound);
    if (!end)
        b1_fail("Phase-5B-RP-B %s string lacks a bounded terminator", label);
    bytes = (size_t)(end - source);
    if (bytes >= destination_size)
        b1_fail("Phase-5B-RP-B %s string exceeds its semantic key", label);
    memcpy(destination, source, bytes);
    destination[bytes] = 0;
}

static void b1_phase5brpb_copy_player_state(player_state_t *destination,
                                             const player_state_t *source,
                                             bool current,
                                             bool kick_angles_active)
{
    memset(destination, 0, sizeof(*destination));
    memcpy(destination->pmove.origin, source->pmove.origin,
           sizeof(destination->pmove.origin));
    destination->pmove.pm_flags = source->pmove.pm_flags & PMF_TELEPORT_BIT;
    memcpy(destination->viewangles, source->viewangles,
           sizeof(destination->viewangles));
    memcpy(destination->viewoffset, source->viewoffset,
           sizeof(destination->viewoffset));
    if (kick_angles_active)
        memcpy(destination->kick_angles, source->kick_angles,
               sizeof(destination->kick_angles));
    memcpy(destination->gunangles, source->gunangles,
           sizeof(destination->gunangles));
    memcpy(destination->gunoffset, source->gunoffset,
           sizeof(destination->gunoffset));
    if (current)
        destination->gunindex = source->gunindex;
    destination->gunframe = source->gunframe;
    if (current)
        memcpy(destination->blend, source->blend, sizeof(destination->blend));
}

static void b1_phase5brpb_copy_server_frame(server_frame_t *destination,
                                             const server_frame_t *source,
                                             bool current,
                                             bool kick_angles_active)
{
    if (!destination || !source || source->areabytes < 0 ||
        source->areabytes > MAX_MAP_AREA_BYTES)
        b1_fail("Phase-5B-RP-B server-frame active area range is malformed");
    memset(destination, 0, sizeof(*destination));
    destination->valid = source->valid;
    destination->number = source->number;
    destination->delta = source->delta;
    destination->areabytes = source->areabytes;
    if (source->areabytes)
        memcpy(destination->areabits, source->areabits,
               (size_t)source->areabytes);
    b1_phase5brpb_copy_player_state(&destination->ps, &source->ps, current,
                                    kick_angles_active);
    destination->clientNum = source->clientNum;
    destination->numEntities = source->numEntities;
    /* firstEntity is parser-ring bookkeeping and is intentionally excluded. */
}

static void b1_phase5brpb_copy_csr(cs_remap_t *destination,
                                    const cs_remap_t *source)
{
    memset(destination, 0, sizeof(*destination));
    destination->extended = source->extended;
    destination->max_edicts = source->max_edicts;
    destination->max_models = source->max_models;
    destination->max_sounds = source->max_sounds;
    destination->max_images = source->max_images;
    destination->airaccel = source->airaccel;
    destination->maxclients = source->maxclients;
    destination->mapchecksum = source->mapchecksum;
    destination->models = source->models;
    destination->sounds = source->sounds;
    destination->images = source->images;
    destination->lights = source->lights;
    destination->items = source->items;
    destination->playerskins = source->playerskins;
    destination->general = source->general;
    destination->end = source->end;
}

static bool b1_phase5brpb_csr_matches_protocol34(const cs_remap_t *csr)
{
    return csr && csr->extended == cs_remap_old.extended &&
        csr->max_edicts == cs_remap_old.max_edicts &&
        csr->max_models == cs_remap_old.max_models &&
        csr->max_sounds == cs_remap_old.max_sounds &&
        csr->max_images == cs_remap_old.max_images &&
        csr->airaccel == cs_remap_old.airaccel &&
        csr->maxclients == cs_remap_old.maxclients &&
        csr->mapchecksum == cs_remap_old.mapchecksum &&
        csr->models == cs_remap_old.models &&
        csr->sounds == cs_remap_old.sounds &&
        csr->images == cs_remap_old.images &&
        csr->lights == cs_remap_old.lights &&
        csr->items == cs_remap_old.items &&
        csr->playerskins == cs_remap_old.playerskins &&
        csr->general == cs_remap_old.general && csr->end == cs_remap_old.end;
}

static const char *b1_phase5brpb_logical_configstring(
    const byte *storage, size_t storage_bytes, const cs_remap_t *csr,
    uint32_t index, size_t *active_bytes, uint32_t *next_index)
{
    const char *value;
    const char *end;
    size_t offset;
    size_t bound;
    uint32_t next;

    if (!storage || !active_bytes || !next_index ||
        !b1_phase5brpb_csr_matches_protocol34(csr) || !csr->end ||
        index >= csr->end || storage_bytes != (size_t)csr->end * MAX_QPATH)
        b1_fail("Phase-5B-RP-B logical configstring remap/storage is malformed");
    if (index > CS_STATUSBAR && index < csr->airaccel)
        b1_fail("Phase-5B-RP-B configstring continuation became a logical start");

    offset = (size_t)index * MAX_QPATH;
    bound = CS_SIZE(csr, (int)index);
    if (!bound || offset > storage_bytes || bound > storage_bytes - offset)
        b1_fail("Phase-5B-RP-B logical configstring span exceeds active storage");
    value = (const char *)storage + offset;
    end = memchr(value, 0, bound);
    if (!end)
        b1_fail("Phase-5B-RP-B logical configstring %u lacks a bounded terminator",
                index);
    *active_bytes = (size_t)(end - value);

    if (index == CS_STATUSBAR) {
        next = csr->airaccel;
    } else if (index >= csr->general) {
        size_t owned_slots = (*active_bytes + 1u + MAX_QPATH - 1u) / MAX_QPATH;
        if (!owned_slots || owned_slots > UINT32_MAX - index)
            b1_fail("Phase-5B-RP-B general configstring ownership overflow");
        next = index + (uint32_t)owned_slots;
    } else {
        next = index + 1u;
    }
    if (next <= index || next > csr->end ||
        *active_bytes + 1u > (size_t)(next - index) * MAX_QPATH)
        b1_fail("Phase-5B-RP-B logical configstring ownership is malformed");
    *next_index = next;
    return value;
}

static bool b1_phase5brpb_endpoint_reaches_base_entity(
    const b1_phase5brpb_boundary_t *boundary,
    const centity_state_t *entity)
{
    if (!entity->modelindex ||
        (entity->effects & EF_GIB && !boundary->cl_gibs_value))
        return false;
    if (boundary->csr.extended &&
        (entity->renderfx & (RF_FLARE | RF_CUSTOM_LIGHT)))
        return false;
    if (boundary->csr.extended && (entity->renderfx & RF_BEAM) &&
        entity->modelindex > 1)
        return false;
    return true;
}

static bool b1_phase5brpb_endpoint_alpha_active(
    const b1_phase5brpb_boundary_t *boundary,
    const centity_state_t *entity)
{
    uint32_t colour_shell_effects = EF_COLOR_SHELL | EF_PENT | EF_QUAD |
        EF_DOUBLE | EF_HALF_DAMAGE;
    bool colour_shell = (entity->effects & colour_shell_effects) != 0 ||
        (entity->morefx & EFX_DUALFIRE) != 0;
    bool prior_owner = (!colour_shell &&
        (entity->renderfx & RF_TRANSLUCENT)) ||
        (entity->effects & (EF_BFG | EF_PLASMA | EF_SPHERETRANS));

    return b1_phase5brpb_endpoint_reaches_base_entity(boundary, entity) &&
        !prior_owner && entity->alpha > 0.0f && entity->alpha < 1.0f;
}

static bool b1_phase5brpb_endpoint_loop_admitted(
    const b1_phase5brpb_boundary_t *boundary,
    const centity_state_t *entity)
{
    int ambient = boundary->s_ambient_value;

    if (!entity->sound)
        return false;
    if (ambient == 2 && !entity->modelindex)
        return false;
    if (ambient == 3 &&
        entity->number != boundary->frame.clientNum + 1)
        return false;
    return true;
}

static void b1_phase5brpb_copy_endpoint_entity(centity_state_t *destination,
                                                const centity_state_t *source,
                                                const b1_phase5brpb_boundary_t *boundary)
{
    bool reaches_base;
    bool loop_admitted;

    memset(destination, 0, sizeof(*destination));
    destination->number = source->number;
    memcpy(destination->origin, source->origin, sizeof(destination->origin));
    memcpy(destination->angles, source->angles, sizeof(destination->angles));
    memcpy(destination->old_origin, source->old_origin,
           sizeof(destination->old_origin));
    destination->modelindex = source->modelindex;
    destination->modelindex2 = source->modelindex2;
    destination->modelindex3 = source->modelindex3;
    destination->modelindex4 = source->modelindex4;
    destination->frame = source->frame;
    destination->skinnum = source->skinnum;
    destination->effects = source->effects;
    destination->renderfx = source->renderfx;
    destination->solid = source->solid;
    destination->sound = source->sound;
    destination->event = source->event;
    if ((uint32_t)source->morefx & ~(uint32_t)B1_PHASE5BRPB_KNOWN_MOREFX)
        b1_fail("Phase-5B-RP-B endpoint entity has unknown morefx bits");
    destination->morefx = (uint32_t)source->morefx &
        (uint32_t)B1_PHASE5BRPB_KNOWN_MOREFX;

    reaches_base = b1_phase5brpb_endpoint_reaches_base_entity(boundary, source);
    if (reaches_base && (!isfinite(source->alpha) || source->alpha < 0.0f ||
                         source->alpha > 1.0f))
        b1_fail("Phase-5B-RP-B endpoint entity alpha is malformed");
    if (b1_phase5brpb_endpoint_alpha_active(boundary, source))
        destination->alpha = source->alpha;
    if (reaches_base && !(source->renderfx & RF_BEAM)) {
        if (!isfinite(source->scale))
            b1_fail("Phase-5B-RP-B endpoint entity scale is non-finite");
        destination->scale = source->scale;
    }
    loop_admitted = b1_phase5brpb_endpoint_loop_admitted(boundary, source);
    if (loop_admitted) {
        if (!isfinite(source->loop_volume) ||
            !isfinite(source->loop_attenuation))
            b1_fail("Phase-5B-RP-B endpoint loop semantics are non-finite");
        destination->loop_volume = source->loop_volume;
        destination->loop_attenuation = source->loop_attenuation;
    }
}

static const b1_phase5brpb_model_key_t *b1_phase5brpb_model_key(
    qhandle_t handle, bool allow_null);

static void b1_phase5brpbr4_project_scene_entity(
    entity_t *destination, const entity_t *source,
    const b1_entity_projection_model_t *model, int32_t rdflags)
{
    memset(destination, 0, sizeof(*destination));
    if (source->flags & RF_BEAM) {
        destination->flags = RF_BEAM;
        destination->frame = source->frame;
        return;
    }
    destination->model = source->model;
    if (source->model & (int)UINT32_C(0x80000000)) {
        if (!isfinite(source->scale))
            b1_fail("Phase-5B-RP-B inline entity scale is non-finite");
        destination->flags = source->flags & RF_TRANSLUCENT;
        destination->frame = source->frame;
        memcpy(destination->angles, source->angles,
               sizeof(destination->angles));
        memcpy(destination->origin, source->origin,
               sizeof(destination->origin));
        memcpy(destination->oldorigin, source->oldorigin,
               sizeof(destination->oldorigin));
        destination->backlerp = source->backlerp;
        destination->id = source->id;
        destination->scale = source->scale;
        if (destination->flags & RF_TRANSLUCENT)
            destination->alpha = source->alpha;
        return;
    }
    if (!source->model)
        return;
    if (!model || !model->present)
        b1_fail("Phase-5B-RP-B-R4 explicit model semantics are absent");
    if (model->type == MOD_SPRITE) {
        destination->frame = source->frame;
        memcpy(destination->origin, source->origin,
               sizeof(destination->origin));
        destination->alpha = source->alpha;
        return;
    }
    if (!model->has_meshes)
        return;
    if (!isfinite(source->scale))
        b1_fail("Phase-5B-RP-B regular entity scale is non-finite");
    destination->flags = source->flags & B1_PHASE5BRPB_ACTIVE_REGULAR_FLAGS;
    destination->frame = source->frame;
    if (!(rdflags & RDF_IRGOGGLES))
        destination->flags &= ~RF_IR_VISIBLE;
    memcpy(destination->angles, source->angles, sizeof(destination->angles));
    memcpy(destination->origin, source->origin, sizeof(destination->origin));
    memcpy(destination->oldorigin, source->oldorigin,
           sizeof(destination->oldorigin));
    destination->oldframe = source->oldframe;
    destination->backlerp = source->backlerp;
    destination->id = source->id;
    destination->scale = source->scale;
    if (destination->flags & RF_TRANSLUCENT)
        destination->alpha = source->alpha;
    if (source->skin)
        destination->skin = source->skin;
    else
        destination->skinnum = source->skinnum;
}

static void b1_phase5brpb_copy_scene_entity(entity_t *destination,
                                              const entity_t *source)
{
    b1_entity_projection_model_t semantics = { 0 };
    const b1_phase5brpb_model_key_t *model = NULL;

    /* qhandles retained here are bounded indices into the immutable semantic
     * key tables latched before the first eligible scene; no raw value enters
     * P/E, and the complete tables are revalidated at EOF. */
    if (!(source->flags & RF_BEAM) && source->model &&
        !(source->model & (int)UINT32_C(0x80000000))) {
        model = b1_phase5brpb_model_key(source->model, false);
        semantics.type = model->type;
        semantics.has_meshes = model->has_meshes;
        semantics.present = model->present;
    }
    b1_phase5brpbr4_project_scene_entity(destination, source,
        model ? &semantics : NULL, b1.rpb.plan_record0.rdflags);
}

static void b1_phase5brpbr6r4_project_normal_scene_entity(
    entity_t *destination, const entity_t *source, int32_t rdflags)
{
    b1_entity_projection_model_t semantics = { 0 };
    const model_t *model = NULL;

    if (!(source->flags & RF_BEAM) && source->model &&
        !(source->model & (int)UINT32_C(0x80000000))) {
        if (source->model <= 0 || source->model > r_numModels ||
            source->model > MAX_MODELS)
            b1_fail("B1 R6-R4 normal entity model handle is outside the registered table");
        model = r_models + source->model - 1;
        if (model->type == MOD_FREE ||
            model->registration_sequence != registration_sequence)
            b1_fail("B1 R6-R4 normal entity model semantics are not registered");
        semantics.type = model->type;
        semantics.has_meshes = model->meshes != NULL;
        semantics.present = true;
    }
    b1_phase5brpbr4_project_scene_entity(destination, source,
        model ? &semantics : NULL, rdflags);
}

static void b1_phase5brpbr4_copy_raw_scene_entity(entity_t *destination,
                                                   const entity_t *source)
{
    memset(destination, 0, sizeof(*destination));
    destination->model = source->model;
    memcpy(destination->angles, source->angles, sizeof(destination->angles));
    memcpy(destination->origin, source->origin, sizeof(destination->origin));
    destination->frame = source->frame;
    memcpy(destination->oldorigin, source->oldorigin,
           sizeof(destination->oldorigin));
    destination->oldframe = source->oldframe;
    destination->backlerp = source->backlerp;
    destination->skinnum = source->skinnum;
    destination->alpha = source->alpha;
    destination->rgba.u32 = source->rgba.u32;
    destination->skin = source->skin;
    destination->flags = source->flags;
    destination->id = source->id;
    destination->tent_type = source->tent_type;
    destination->scale = source->scale;
}

static void b1_phase5brpb_copy_particle(particle_t *destination,
                                         const particle_t *source)
{
    memset(destination, 0, sizeof(*destination));
    memcpy(destination->origin, source->origin, sizeof(destination->origin));
    if (source->color >= 0) {
        destination->color = (uint8_t)source->color;
    } else {
        destination->color = -1;
        destination->rgba.u8[0] = source->rgba.u8[0];
        destination->rgba.u8[1] = source->rgba.u8[1];
        destination->rgba.u8[2] = source->rgba.u8[2];
    }
    destination->alpha = source->alpha;
    destination->brightness = source->brightness;
    destination->radius = source->radius;
}

static void b1_phase5brpb_copy_dlight(dlight_t *destination,
                                       const dlight_t *source)
{
    memset(destination, 0, sizeof(*destination));
    memcpy(destination->origin, source->origin, sizeof(destination->origin));
    memcpy(destination->color, source->color, sizeof(destination->color));
    destination->intensity = source->intensity;
    destination->radius = source->radius;
    destination->light_type = source->light_type;
    if (source->light_type != DLIGHT_SPOT)
        return;
    destination->spot.emission_profile = source->spot.emission_profile;
    memcpy(destination->spot.direction, source->spot.direction,
           sizeof(destination->spot.direction));
    if (source->spot.emission_profile == DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF) {
        destination->spot.cos_total_width = source->spot.cos_total_width;
        destination->spot.cos_falloff_start = source->spot.cos_falloff_start;
    } else if (source->spot.emission_profile ==
               DLIGHT_SPOT_EMISSION_PROFILE_AXIS_ANGLE_TEXTURE) {
        destination->spot.total_width = source->spot.total_width;
        destination->spot.texture = source->spot.texture;
    }
}

static void b1_phase5brpb_latch_resources(void)
{
    cvar_t *particle_size;

    if (b1.rpb.resources_latched || b1.rpb.settings_latched)
        b1_fail("Phase-5B-RP-B resource/settings latch was repeated");
    if (!b1.rpb.arena_ready || r_numModels < 0 || r_numModels > MAX_MODELS ||
        r_numImages < 0 || r_numImages > MAX_RIMAGES)
        b1_fail("Phase-5B-RP-B resource-table range is malformed");
    b1.rpb.model_key_count = (uint32_t)r_numModels;
    b1.rpb.image_key_count = (uint32_t)r_numImages;
    b1.rpb.resource_registration_sequence = (uint32_t)registration_sequence;
    for (int index = 0; index < r_numModels; index++) {
        const model_t *model = r_models + index;
        b1_phase5brpb_model_key_t *key = b1.rpb.model_keys + index + 1;
        if (model->type == MOD_FREE)
            continue;
        if (model->nummeshes < 0 || model->numframes < 0 ||
            model->num_light_polys < 0 ||
            model->num_light_polys > B1_PHASE5BRPB_MAX_MODEL_LIGHT_POLYS ||
            (model->num_light_polys > 0 && !model->light_polys))
            b1_fail("Phase-5B-RP-B model semantic range is malformed");
        b1_phase5brpb_copy_bounded_string(key->name, sizeof(key->name),
            model->name, sizeof(model->name), "model-name");
        key->type = model->type;
        key->model_class = model->model_class;
        key->num_meshes = model->nummeshes;
        key->num_frames = model->numframes;
        key->num_light_polys = model->num_light_polys;
        key->sprite_vertical = model->sprite_vertical;
        key->has_meshes = model->meshes != NULL;
        key->has_iqm = model->iqmData != NULL;
        key->registration_sequence = model->registration_sequence;
        if (model->iqmData) {
            key->iqm_vertexes = model->iqmData->num_vertexes;
            key->iqm_triangles = model->iqmData->num_triangles;
            key->iqm_frames = model->iqmData->num_frames;
            key->iqm_meshes = model->iqmData->num_meshes;
            key->iqm_joints = model->iqmData->num_joints;
            key->iqm_poses = model->iqmData->num_poses;
            key->iqm_animations = model->iqmData->num_animations;
        }
        if (model->type == MOD_SPRITE) {
            size_t capacity = ((size_t)MAX_MODELS + 1u) * SP2_MAX_FRAMES;
            if (model->numframes <= 0 || model->numframes > SP2_MAX_FRAMES ||
                !model->spriteframes || b1.rpb.sprite_frame_keys_used >
                    capacity - (size_t)model->numframes)
                b1_fail("Phase-5B-RP-B sprite-frame resource range is malformed");
            key->sprite_frame_offset =
                (uint32_t)b1.rpb.sprite_frame_keys_used;
            key->sprite_frame_count = (uint32_t)model->numframes;
            for (int frame_index = 0; frame_index < model->numframes;
                 frame_index++) {
                const mspriteframe_t *frame = model->spriteframes + frame_index;
                b1_phase5brpb_sprite_frame_key_t *frame_key =
                    b1.rpb.sprite_frame_keys + b1.rpb.sprite_frame_keys_used++;
                ptrdiff_t image_index;
                if (!frame->image)
                    b1_fail("Phase-5B-RP-B sprite-frame image is absent");
                image_index = frame->image - r_images;
                if (image_index < 0 || image_index >= r_numImages)
                    b1_fail("Phase-5B-RP-B sprite-frame image is outside its table");
                frame_key->image_key_index = (uint32_t)image_index;
                frame_key->width = frame->width;
                frame_key->height = frame->height;
                frame_key->origin_x = frame->origin_x;
                frame_key->origin_y = frame->origin_y;
            }
        }
        key->present = true;
    }
    for (int index = 0; index < r_numImages; index++) {
        const image_t *image = r_images + index;
        b1_phase5brpb_image_key_t *key = b1.rpb.image_keys + index;
        if (!image->registration_sequence)
            continue;
        b1_phase5brpb_copy_bounded_string(key->name, sizeof(key->name),
            image->name, sizeof(image->name), "image-name");
        b1_phase5brpb_copy_bounded_string(key->filepath, sizeof(key->filepath),
            image->filepath, sizeof(image->filepath), "image-filepath");
        key->type = image->type;
        key->flags = (uint32_t)image->flags;
        key->width = image->width;
        key->height = image->height;
        key->upload_width = image->upload_width;
        key->upload_height = image->upload_height;
        key->is_srgb = image->is_srgb;
        key->registration_sequence = image->registration_sequence;
        key->present = true;
    }
    particle_size = Cvar_FindVar("pt_particle_size");
    if (!particle_size || !isfinite(particle_size->value) ||
        particle_size->value <= 0.0f)
        b1_fail("Phase-5B-RP-B pt_particle_size cannot be latched");
    b1.rpb.particle_size_cvar = particle_size;
    memcpy(&b1.rpb.particle_size_bits, &particle_size->value,
           sizeof(b1.rpb.particle_size_bits));
    particle_size->modified = false;
    b1_verify_settings_serialization(true, false);
    b1.rpb.settings_latched = true;
    b1.rpb.resources_latched = true;
}

static void b1_phase5brpb_verify_resources(void)
{
    uint32_t particle_size_bits;

    if (!b1.rpb.resources_latched || !b1.rpb.settings_latched ||
        r_numModels != (int)b1.rpb.model_key_count ||
        r_numImages != (int)b1.rpb.image_key_count ||
        registration_sequence != (int)b1.rpb.resource_registration_sequence ||
        !b1.rpb.particle_size_cvar ||
        b1.rpb.particle_size_cvar->modified ||
        !isfinite(b1.rpb.particle_size_cvar->value) ||
        b1.rpb.particle_size_cvar->value <= 0.0f)
        b1_fail("Phase-5B-RP-B resource table changed after its semantic latch");
    memcpy(&particle_size_bits, &b1.rpb.particle_size_cvar->value,
           sizeof(particle_size_bits));
    if (particle_size_bits != b1.rpb.particle_size_bits)
        b1_fail("Phase-5B-RP-B pt_particle_size changed after its semantic latch");
    for (uint32_t handle = 1; handle <= b1.rpb.model_key_count; handle++) {
        const model_t *model = r_models + handle - 1u;
        const b1_phase5brpb_model_key_t *key = b1.rpb.model_keys + handle;
        if (key->present != (model->type != MOD_FREE))
            b1_fail("Phase-5B-RP-B model presence changed after latch");
        if (!key->present)
            continue;
        if (strcmp(key->name, model->name) || key->type != model->type ||
            key->model_class != model->model_class ||
            key->num_meshes != model->nummeshes ||
            key->num_frames != model->numframes ||
            key->num_light_polys != model->num_light_polys ||
            model->nummeshes < 0 || model->numframes < 0 ||
            model->num_light_polys < 0 ||
            model->num_light_polys > B1_PHASE5BRPB_MAX_MODEL_LIGHT_POLYS ||
            (model->num_light_polys > 0 && !model->light_polys) ||
            key->sprite_vertical != model->sprite_vertical ||
            key->has_meshes != (model->meshes != NULL) ||
            key->has_iqm != (model->iqmData != NULL) ||
            key->registration_sequence != model->registration_sequence)
            b1_fail("Phase-5B-RP-B model semantic key changed after latch");
        if (model->iqmData &&
            (key->iqm_vertexes != model->iqmData->num_vertexes ||
             key->iqm_triangles != model->iqmData->num_triangles ||
             key->iqm_frames != model->iqmData->num_frames ||
             key->iqm_meshes != model->iqmData->num_meshes ||
             key->iqm_joints != model->iqmData->num_joints ||
             key->iqm_poses != model->iqmData->num_poses ||
             key->iqm_animations != model->iqmData->num_animations))
            b1_fail("Phase-5B-RP-B IQM semantic key changed after latch");
        if (model->type == MOD_SPRITE) {
            if (!model->spriteframes || model->numframes <= 0 ||
                key->sprite_frame_count != (uint32_t)model->numframes)
                b1_fail("Phase-5B-RP-B sprite-frame count changed after latch");
            for (int frame_index = 0; frame_index < model->numframes;
                 frame_index++) {
                const mspriteframe_t *frame = model->spriteframes + frame_index;
                const b1_phase5brpb_sprite_frame_key_t *frame_key =
                    b1.rpb.sprite_frame_keys + key->sprite_frame_offset +
                    frame_index;
                ptrdiff_t image_index;
                if (!frame->image)
                    b1_fail("Phase-5B-RP-B sprite-frame image changed after latch");
                image_index = frame->image - r_images;
                if (image_index < 0 || image_index >= r_numImages ||
                    frame_key->image_key_index != (uint32_t)image_index ||
                    frame_key->width != frame->width ||
                    frame_key->height != frame->height ||
                    frame_key->origin_x != frame->origin_x ||
                    frame_key->origin_y != frame->origin_y)
                    b1_fail("Phase-5B-RP-B sprite-frame semantics changed after latch");
            }
        }
    }
    for (uint32_t handle = 0; handle < b1.rpb.image_key_count; handle++) {
        const image_t *image = r_images + handle;
        const b1_phase5brpb_image_key_t *key = b1.rpb.image_keys + handle;
        if (key->present != (image->registration_sequence != 0))
            b1_fail("Phase-5B-RP-B image presence changed after latch");
        if (!key->present)
            continue;
        if (strcmp(key->name, image->name) || strcmp(key->filepath, image->filepath) ||
            key->type != image->type || key->flags != (uint32_t)image->flags ||
            key->width != image->width || key->height != image->height ||
            key->upload_width != image->upload_width ||
            key->upload_height != image->upload_height ||
            key->is_srgb != image->is_srgb ||
            key->registration_sequence != image->registration_sequence)
            b1_fail("Phase-5B-RP-B image semantic key changed after latch");
    }
}

static bool b1_phase5brpb_resources_ready(void)
{
    return cls.state == ca_active && cl.bsp && cl.bsp->nummodels > 1 &&
        r_numModels > 0 && r_numModels <= MAX_MODELS &&
        r_numImages > 0 && r_numImages <= MAX_RIMAGES &&
        registration_sequence > 0 && cl.csr.end > 0 &&
        cl.csr.end <= q_countof(cl.configstrings) &&
        cl.csr.models + 1u < cl.csr.end && !strcmp(cl.mapname, "base1") &&
        !strcmp(cl.configstrings[cl.csr.models + 1u], "maps/base1.bsp");
}

static const b1_phase5brpb_model_key_t *b1_phase5brpb_model_key(
    qhandle_t handle, bool allow_null)
{
    if (!handle && allow_null)
        return NULL;
    if (handle <= 0 || (uint32_t)handle > b1.rpb.model_key_count ||
        !b1.rpb.model_keys[handle].present)
        b1_fail("Phase-5B-RP-B model handle does not resolve to a stable key");
    return b1.rpb.model_keys + handle;
}

static const b1_phase5brpb_image_key_t *b1_phase5brpb_image_key(qhandle_t handle)
{
    if (handle < 0 || (uint32_t)handle >= b1.rpb.image_key_count ||
        !b1.rpb.image_keys[handle].present)
        b1_fail("Phase-5B-RP-B image handle does not resolve to a stable key");
    return b1.rpb.image_keys + handle;
}

static uint32_t b1_phase5brpb_f32_bits(float value, const char *label)
{
    uint32_t bits;
    if (!isfinite(value))
        b1_fail("Phase-5B-RP-B %s is non-finite", label);
    memcpy(&bits, &value, sizeof(bits));
    if ((bits & UINT32_C(0x7fffffff)) == 0)
        bits = 0;
    return bits;
}

static void b1_phase5brpb_f32(b1_candidate_buffer_t *buffer, float value,
                              const char *label)
{
    b1_candidate_u32(buffer, b1_phase5brpb_f32_bits(value, label));
}

static void b1_phase5brpb_vec(b1_candidate_buffer_t *buffer,
                              const float *values, size_t count,
                              const char *label)
{
    for (size_t index = 0; index < count; index++)
        b1_phase5brpb_f32(buffer, values[index], label);
}

static float b1_phase5brpb_effective_light_style(float white)
{
    if (!isfinite(white))
        b1_fail("Phase-5B-RP-B active light style is non-finite");
    if (white < 0.0f)
        return 0.0f;
    if (white > 2.0f)
        return 2.0f;
    return white == 0.0f ? 0.0f : white;
}

static bool b1_phase5brpb_digest_control_matches(
    const cvar_t *value, const byte expected[32])
{
    byte current[32];

    return value && !value->modified &&
        b1_parse_sha256(value->string, current) &&
        !memcmp(current, expected, sizeof(current));
}

static void b1_phase5brpb_require_controls(void)
{
    if (!b1.rpb.enabled || b1.rpb.mode == B1_PHASE5BRPB_MODE_OFF ||
        !b1_phase5brpb_mode || b1_phase5brpb_mode->modified ||
        b1_phase5brpb_parse_mode(b1_phase5brpb_mode->string) != b1.rpb.mode ||
        !b1_phase5brpb_output || !b1_phase5brpb_output->string[0] ||
        b1_phase5brpb_output->modified || !b1_phase5brpb_plan ||
        !b1_phase5brpb_plan->string[0] || b1_phase5brpb_plan->modified ||
        !b1_phase5brpb_digest_control_matches(
            b1_phase5brpb_plan_sha256, b1.rpb.plan_sha256) ||
        !b1_phase5brpb_digest_control_matches(
            b1_phase5brpb_executable_sha256, b1.rpb.executable_sha256) ||
        !b1_phase5brpb_digest_control_matches(
            b1_phase5brpb_runtime_manifest_sha256,
            b1.rpb.runtime_manifest_sha256) ||
        !b1_phase5brpb_digest_control_matches(
            b1_phase5brpb_runtime_inventory_sha256,
            b1.rpb.runtime_inventory_sha256) ||
        !b1_phase5brpb_digest_control_matches(
            b1_phase5brpb_runtime_data_sha256,
            b1.rpb.runtime_data_sha256) ||
        !b1_phase5brpb_digest_control_matches(
            b1_phase5brpb_study_plan_sha256, b1.rpb.study_plan_sha256) ||
        !b1_phase5brpb_digest_control_matches(
            b1_phase5brpb_startup_sha256, b1.rpb.startup_sha256) ||
        !b1_phase5brpbr2_diagnostic || b1_phase5brpbr2_diagnostic->modified ||
        b1.rpb.r2_entity_diagnostic !=
            !strcmp(b1_phase5brpbr2_diagnostic->string, "1") ||
        (b1.rpb.r2_entity_diagnostic && (
            !b1_phase5brpbr2_expected_entities ||
            !b1_phase5brpbr2_expected_entities->string[0] ||
            b1_phase5brpbr2_expected_entities->modified ||
            !b1_phase5brpb_digest_control_matches(
                b1_phase5brpbr2_expected_entities_sha256,
                b1.rpb.r2_expected_entities_sha256) ||
            !b1_phase5brpbr2_source_revision ||
            b1_phase5brpbr2_source_revision->modified ||
            strlen(b1_phase5brpbr2_source_revision->string) != 40u ||
            strncmp(b1_phase5brpbr2_source_revision->string,
                    Q2RTX_UPSTREAM_REVISION, 8u) ||
            !b1_phase5brpb_digest_control_matches(
                b1_phase5brpbr2_producer_authority_sha256,
                b1.rpb.r2_producer_authority_sha256))) ||
        B1_Requested() || b1_any_request_value() ||
        b1.candidate.enabled || b1.endpoint.enabled ||
        b1.scene_controller.enabled || b1.scene_source_ready ||
        b1.temporal_infrastructure_ready || b1.bootstrap_complete)
        b1_fail("Phase-5B-RP-B controls or inactive B1 boundaries changed");
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0) {
        if (b1_phase5brpb_p0->string[0] || b1_phase5brpb_p0->modified ||
            b1_phase5brpb_p0_sha256->string[0] ||
            b1_phase5brpb_p0_sha256->modified)
            b1_fail("Phase-5B-RP-B P0 input controls changed");
    } else if (!b1_phase5brpb_p0->string[0] ||
               b1_phase5brpb_p0->modified ||
               !b1_phase5brpb_digest_control_matches(
                   b1_phase5brpb_p0_sha256,
                   b1.rpb.accepted_p0_sha256)) {
        b1_fail("Phase-5B-RP-B accepted P0 controls changed");
    }
}

static bool b1_phase5brpb_frame_predecessor(const server_frame_t *frame,
                                             const server_frame_t **predecessor)
{
    if (!frame->valid || frame->delta <= 0 || frame->delta == frame->number)
        return false;
    *predecessor = &cl.frames[frame->delta & UPDATE_MASK];
    return (*predecessor)->number == frame->delta && (*predecessor)->valid &&
        (*predecessor)->firstEntity >= 0 &&
        cl.numEntityStates >= (*predecessor)->firstEntity &&
        cl.numEntityStates - (*predecessor)->firstEntity <=
            MAX_PARSE_ENTITIES - MAX_PACKET_ENTITIES;
}

static void b1_phase5brpb_validate_selected_anchor(
    const server_frame_t *frame)
{
    static const int16_t expected_origin[3] = { 1024, -2560, 193 };
    static const float expected_angles[3] = { 0.0f, 135.0f, 0.0f };

    if (!frame || !frame->valid ||
        frame->number != B1_PHASE5AR5_SELECTED_FRAME ||
        frame->delta != B1_PHASE5AR5_SELECTED_DELTA ||
        cl.servertime != B1_PHASE5AR5_SELECTED_SERVER_TIME ||
        frame->numEntities != B1_PHASE5AR5_SELECTED_ENTITIES ||
        frame->areabytes != B1_PHASE5AR5_SELECTED_AREA_BYTES ||
        cl.csr.end != B1_PHASE5AR5_SELECTED_CONFIGSTRINGS ||
        cl.csr.models + 1u >= cl.csr.end ||
        cls.serverProtocol != PROTOCOL_VERSION_DEFAULT ||
        strcmp(cl.mapname, "base1") ||
        strcmp(cl.configstrings[cl.csr.models + 1u], "maps/base1.bsp"))
        b1_fail("Phase-5B-RP-B selected anchor semantic guards differ");
    for (uint32_t index = 0; index < 3u; index++) {
        if (frame->ps.pmove.origin[index] != expected_origin[index] ||
            b1_phase5brpb_f32_bits(frame->ps.viewangles[index],
                "selected anchor view angle") !=
            b1_phase5brpb_f32_bits(expected_angles[index],
                "selected anchor expected view angle"))
            b1_fail("Phase-5B-RP-B selected anchor pose guards differ");
    }
    if (b1_phase5brpb_f32_bits(frame->ps.fov,
            "selected anchor FOV") !=
        b1_phase5brpb_f32_bits(75.0f, "selected anchor expected FOV"))
        b1_fail("Phase-5B-RP-B selected anchor FOV guard differs");
}

static void b1_phase5brpb_snapshot_endpoint(
    b1_phase5brpb_boundary_t *boundary, const server_frame_t *predecessor)
{
    size_t config_capacity = (size_t)B1_PHASE5BRPB_MAX_BOUNDARIES *
        sizeof(cl.configstrings);
    size_t config_bytes;
    size_t endpoint_capacity = (size_t)B1_PHASE5BRPB_MAX_BOUNDARIES *
        MAX_PACKET_ENTITIES;

    if (!b1_phase5brpb_csr_matches_protocol34(&cl.csr) ||
        !cl.csr.end || cl.csr.end > q_countof(cl.configstrings))
        b1_fail("Phase-5B-RP-B endpoint configstring span is malformed");
    config_bytes = (size_t)cl.csr.end * sizeof(configstring_t);
    if (cl.csr.end == 0 || cl.csr.end > q_countof(cl.configstrings) ||
        cl.frame.numEntities < 0 || cl.frame.numEntities > MAX_PACKET_ENTITIES ||
        cl.frame.firstEntity < 0 || cl.numEntityStates < cl.frame.firstEntity ||
        cl.numEntityStates - cl.frame.firstEntity < cl.frame.numEntities ||
        cl.numEntityStates - cl.frame.firstEntity > MAX_PARSE_ENTITIES ||
        b1.rpb.config_storage_used > config_capacity - config_bytes ||
        b1.rpb.endpoint_entities_used > endpoint_capacity -
            (size_t)cl.frame.numEntities)
        b1_fail("Phase-5B-RP-B endpoint snapshot range is stale or exhausted");
    b1_phase5brpb_copy_server_frame(&boundary->frame, &cl.frame, true,
                                    boundary->kick_angles_active);
    b1_phase5brpb_copy_server_frame(&boundary->predecessor, predecessor, false,
                                    boundary->kick_angles_active);
    b1_phase5brpb_copy_csr(&boundary->csr, &cl.csr);
    boundary->config_offset = b1.rpb.config_storage_used;
    boundary->config_bytes = (uint32_t)config_bytes;
    boundary->config_logical_count = 0;
    memset(b1.rpb.config_storage + boundary->config_offset, 0, config_bytes);
    for (uint32_t index = 0; index < cl.csr.end;) {
        const char *source;
        char *destination = (char *)(b1.rpb.config_storage +
            boundary->config_offset + (size_t)index * sizeof(configstring_t));
        size_t active_bytes;
        uint32_t next;

        source = b1_phase5brpb_logical_configstring(
            (const byte *)cl.configstrings, config_bytes, &cl.csr, index,
            &active_bytes, &next);
        memcpy(destination, source, active_bytes + 1u);
        boundary->config_logical_count++;
        index = next;
    }
    b1.rpb.config_storage_used += config_bytes;
    boundary->endpoint_entity_offset = b1.rpb.endpoint_entities_used;
    boundary->endpoint_entity_count = (uint32_t)cl.frame.numEntities;
    for (int index = 0; index < cl.frame.numEntities; index++) {
        const centity_state_t *source = cl.entityStates +
            ((cl.frame.firstEntity + index) & PARSE_ENTITIES_MASK);
        b1_phase5brpb_copy_endpoint_entity(
            b1.rpb.endpoint_entities + b1.rpb.endpoint_entities_used++, source,
            boundary);
    }
    boundary->server_protocol = cls.serverProtocol;
    boundary->protocol_version = cls.protocolVersion;
    boundary->es_flags = (uint32_t)cl.esFlags;
    boundary->ps_flags = (uint32_t)cl.psFlags;
    boundary->server_count = cl.servercount;
    boundary->server_state = cl.serverstate;
    boundary->client_num = cl.clientNum;
    boundary->num_entity_states = cl.numEntityStates;
    boundary->observed_client_time = cl.time;
    boundary->observed_server_time = cl.servertime;
    memcpy(&boundary->observed_lerpfrac_bits, &cl.lerpfrac,
           sizeof(boundary->observed_lerpfrac_bits));
    b1_phase5brpb_copy_bounded_string(boundary->gamedir,
        sizeof(boundary->gamedir), cl.gamedir, sizeof(cl.gamedir), "gamedir");
    b1_phase5brpb_copy_bounded_string(boundary->mapname,
        sizeof(boundary->mapname), cl.mapname, sizeof(cl.mapname), "mapname");
}

static void b1_phase5brpb_complete_message(void)
{
    b1_phase5brpb_boundary_t *boundary;
    const server_frame_t *predecessor = NULL;
    byte endpoint_digest[32], range_set_digest[32];
    size_t endpoint_bytes = 0;
    uint32_t index;
    uint32_t recovered;
    bool selected = false;
    bool open_token = false;

    b1_phase5brpb_require_controls();
    if (b1.rpb.settings_latched)
        b1_verify_settings_serialization(true, true);
    if (!b1.rpb.started || !b1.demo_hook.active || !b1.demo_hook.armed ||
        b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_FAILED ||
        b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_SEALED ||
        b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_EOF)
        b1_fail("Phase-5B-RP-B complete-message notification is out of state");
    if (b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_PENDING_SCENE)
        b1_fail("Phase-5B-RP-B parsed a later message before scene closure");
    if (b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_P0_CLOSED) {
        b1.rpb.later_message_parse_count++;
        b1_fail("Phase-5B-RP-B parsed a later message after P0 closure");
    }
    if (msg_read.readcount != msg_read.cursize || !msg_read.data ||
        !msg_read.cursize || msg_read.cursize > MAX_MSGLEN ||
        b1.rpb.message_count >= B1_PHASE5BRPB_EXPECTED_MESSAGES ||
        b1.rpb.message_storage_used >
            (size_t)B1_PHASE5BRPB_MAX_BOUNDARIES * MAX_MSGLEN -
            msg_read.cursize)
        b1_fail("Phase-5B-RP-B complete-message byte boundary is malformed");
    index = b1.rpb.message_count++;
    boundary = b1.rpb.boundaries + index;
    boundary->ordinal = (uint64_t)index + 1u;
    boundary->notification_serial = b1.demo_hook.complete_notifications;
    boundary->message_offset = b1.rpb.message_storage_used;
    boundary->message_bytes = (uint32_t)msg_read.cursize;
    memcpy(b1.rpb.message_storage + boundary->message_offset,
           msg_read.data, msg_read.cursize);
    b1.rpb.message_storage_used += msg_read.cursize;

    boundary->kick_angles_active =
        Cvar_VariableInteger("cl_kickangles") != 0;
    boundary->cl_gibs_value = Cvar_VariableInteger("cl_gibs");
    boundary->s_ambient_value = Cvar_VariableInteger("s_ambient");
    b1_phase5brpb_copy_server_frame(&boundary->frame, &cl.frame, true,
                                    boundary->kick_angles_active);
    boundary->frame_valid = cl.frame.valid;
    boundary->new_frame = cl.frame.valid && (!b1.rpb.last_frame_valid ||
        cl.frame.number != b1.rpb.last_frame_number);
    if (!boundary->frame_valid)
        boundary->message_class = B1_PHASE5BRPB_MESSAGE_NO_VALID_FRAME;
    else if (!boundary->new_frame)
        boundary->message_class = B1_PHASE5BRPB_MESSAGE_NO_NEW_FRAME;
    else {
        b1.rpb.last_frame_valid = true;
        b1.rpb.last_frame_number = cl.frame.number;
        boundary->predecessor_present = cl.frame.delta > 0;
        if (cl.frame.delta <= 0) {
            boundary->message_class = B1_PHASE5BRPB_MESSAGE_NO_PREDECESSOR;
        } else {
            boundary->predecessor_valid =
                b1_phase5brpb_frame_predecessor(&cl.frame, &predecessor);
            if (!boundary->predecessor_valid)
                b1_fail("Phase-5B-RP-B declared predecessor is invalid or stale");
            boundary->message_class = B1_PHASE5BRPB_MESSAGE_ELIGIBLE;
        }
    }
    recovered = cl.frameflags &
        (FF_SERVERDROP | FF_BADFRAME | FF_OLDFRAME | FF_OLDENT | FF_NODELTA);
    boundary->recovered_flags = recovered;
    if (recovered) {
        b1.rpb.recovery_count++;
        b1_fail("Phase-5B-RP-B observed a recovered or invalid delta path");
    }
    if (!b1.rpb.resources_latched &&
        boundary->message_class != B1_PHASE5BRPB_MESSAGE_ELIGIBLE &&
        b1_phase5brpb_resources_ready())
        b1_phase5brpb_latch_resources();
    if (boundary->message_class == B1_PHASE5BRPB_MESSAGE_ELIGIBLE &&
        !b1.rpb.resources_latched)
        b1_fail("Phase-5B-RP-B reached an eligible boundary before resource latch");
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0 &&
        msg_read.cursize == b1.rpb.selected_anchor_bytes &&
        !memcmp(msg_read.data, b1.rpb.selected_anchor_payload,
                msg_read.cursize)) {
        selected = true;
        boundary->selected_anchor = true;
        b1.rpb.selected_anchor_count++;
        if (b1.rpb.selected_anchor_count != 1u ||
            boundary->message_class != B1_PHASE5BRPB_MESSAGE_ELIGIBLE)
            b1_fail("Phase-5B-RP-B selected P0 anchor is duplicated or ineligible");
        b1_phase5brpb_validate_selected_anchor(&cl.frame);
    }
    open_token = boundary->message_class == B1_PHASE5BRPB_MESSAGE_ELIGIBLE &&
        (b1.rpb.mode == B1_PHASE5BRPB_MODE_ENUMERATE || selected);
    if (!open_token) {
        b1.rpb.nonqualifying_message_count++;
        return;
    }
    if (!predecessor)
        b1_fail("Phase-5B-RP-B eligible boundary lacks its predecessor");
    b1_phase5brpb_snapshot_endpoint(boundary, predecessor);
    if (selected) {
        if (!b1_sha256_data(msg_read.data, msg_read.cursize,
                            boundary->message_sha256))
            b1_fail("Phase-5B-RP-B cannot hash the selected complete message");
        b1_scene_controller_compute_endpoint(&cl.frame,
            boundary->message_sha256, endpoint_digest, range_set_digest,
            &endpoint_bytes);
        if (endpoint_bytes > UINT32_MAX)
            b1_fail("Phase-5B-RP-B endpoint canonical byte count overflows");
        memcpy(boundary->endpoint_sha256, endpoint_digest, 32);
        memcpy(boundary->endpoint_range_set_sha256, range_set_digest, 32);
        boundary->endpoint_canonical_bytes = (uint32_t)endpoint_bytes;
        boundary->endpoint_range_count = b1.endpoint.range_count;
        boundary->endpoint_identity_closed = true;
        b1_apply_committed_snapshot_endpoint("Phase-5B-RP-B selected endpoint");
        boundary->endpoint_timing_closed = true;
    }
    if (b1.rpb.next_token_id == UINT64_MAX)
        b1_fail("Phase-5B-RP-B scene-token counter overflow");
    boundary->token_id = ++b1.rpb.next_token_id;
    b1.rpb.pending_token_id = boundary->token_id;
    b1.rpb.pending_boundary = index;
    b1.rpb.eligible_message_count++;
    b1.rpb.playback_state = B1_PHASE5BRPB_PLAYBACK_PENDING_SCENE;
}

static bool b1_phase5brpb_snapshot_scene(const refdef_t *fd)
{
    b1_phase5brpb_boundary_t *boundary;
    refdef_t *snapshot;
    size_t entity_capacity = (size_t)B1_PHASE5BRPB_MAX_BOUNDARIES * MAX_ENTITIES;
    size_t particle_capacity = (size_t)B1_PHASE5BRPB_MAX_BOUNDARIES * MAX_PARTICLES;
    size_t dlight_capacity = (size_t)B1_PHASE5BRPB_MAX_BOUNDARIES * MAX_DLIGHTS;
    size_t style_capacity = (size_t)B1_PHASE5BRPB_MAX_BOUNDARIES * MAX_LIGHTSTYLES;
    size_t area_capacity = (size_t)B1_PHASE5BRPB_MAX_BOUNDARIES * MAX_MAP_AREA_BYTES;

    if (b1.rpb.playback_state != B1_PHASE5BRPB_PLAYBACK_PENDING_SCENE ||
        b1.rpb.pending_boundary >= b1.rpb.message_count ||
        !b1.rpb.pending_token_id || !fd)
        b1_fail("Phase-5B-RP-B scene callback lacks one pending token");
    boundary = b1.rpb.boundaries + b1.rpb.pending_boundary;
    if (boundary->token_id != b1.rpb.pending_token_id ||
        boundary->scene_closed || !boundary->predecessor_valid ||
        boundary->notification_serial != b1.demo_hook.complete_notifications ||
        !cl.frame.valid || cl.frame.number != boundary->frame.number ||
        cl.frame.delta != boundary->frame.delta ||
        cl.servertime != boundary->observed_server_time ||
        strcmp(cl.gamedir, boundary->gamedir) ||
        strcmp(cl.mapname, boundary->mapname) ||
        strcmp(boundary->mapname, "base1") ||
        fd->num_entities < 0 || fd->num_entities > MAX_ENTITIES ||
        fd->num_particles < 0 || fd->num_particles > MAX_PARTICLES ||
        fd->num_dlights < 0 || fd->num_dlights > MAX_DLIGHTS ||
        !isfinite(fd->time) ||
        (fd->num_entities && !fd->entities) ||
        (fd->num_particles && !fd->particles) ||
        (fd->num_dlights && !fd->dlights) || !fd->lightstyles ||
        boundary->frame.areabytes < 0 ||
        boundary->frame.areabytes > MAX_MAP_AREA_BYTES ||
        (boundary->frame.areabytes && !fd->areabits) ||
        b1.rpb.raw_scene_entities_used > entity_capacity -
            (size_t)fd->num_entities ||
        b1.rpb.scene_entities_used > entity_capacity -
            (size_t)fd->num_entities ||
        b1.rpb.scene_particles_used > particle_capacity -
            (size_t)fd->num_particles ||
        b1.rpb.scene_dlights_used > dlight_capacity -
            (size_t)fd->num_dlights ||
        b1.rpb.scene_lightstyles_used > style_capacity - MAX_LIGHTSTYLES ||
        b1.rpb.scene_areabits_used > area_capacity -
            (size_t)boundary->frame.areabytes ||
        !b1.rpb.resources_latched || !b1.rpb.settings_latched ||
        !b1.rpb.particle_size_cvar ||
        b1.rpb.particle_size_cvar->modified)
        b1_fail("Phase-5B-RP-B scene boundary is stale, malformed or exhausted");

    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0) {
        uint32_t lerp_bits;
        memcpy(&lerp_bits, &cl.lerpfrac, sizeof(lerp_bits));
        boundary->scene_build_count++;
        boundary->scene_build_client_time = cl.time;
        boundary->scene_build_server_time = cl.servertime;
        boundary->scene_build_frame = cl.frame.number;
        boundary->scene_build_delta = cl.frame.delta;
        boundary->scene_build_lerpfrac_bits = lerp_bits;
        if (boundary->scene_build_count != 1u ||
            !boundary->endpoint_identity_closed ||
            !boundary->endpoint_timing_closed ||
            cl.time != B1_PHASE5AR5_SELECTED_SERVER_TIME ||
            cl.servertime != B1_PHASE5AR5_SELECTED_SERVER_TIME ||
            lerp_bits != UINT32_C(0x3f800000) ||
            cl.frame.number != B1_PHASE5AR5_SELECTED_FRAME ||
            cl.frame.delta != B1_PHASE5AR5_SELECTED_DELTA ||
            cl_paused->integer != 2 || sv_paused->integer != 1 ||
            com_timedemo->integer)
            b1_fail("Phase-5B-RP-B first scene build did not consume the committed endpoint");
    }

    boundary->raw_scene_entity_offset = b1.rpb.raw_scene_entities_used;
    boundary->raw_scene_entity_count = (uint32_t)fd->num_entities;
    for (int index = 0; index < fd->num_entities; index++)
        b1_phase5brpbr4_copy_raw_scene_entity(
            b1.rpb.raw_scene_entities + b1.rpb.raw_scene_entities_used++,
            fd->entities + index);
    boundary->scene_entity_offset = b1.rpb.scene_entities_used;
    boundary->scene_entity_count = (uint32_t)fd->num_entities;
    for (int index = 0; index < fd->num_entities; index++)
        b1_phase5brpb_copy_scene_entity(
            b1.rpb.scene_entities + b1.rpb.scene_entities_used++,
            fd->entities + index);
    boundary->scene_particle_offset = b1.rpb.scene_particles_used;
    boundary->scene_particle_count = (uint32_t)fd->num_particles;
    for (int index = 0; index < fd->num_particles; index++)
        b1_phase5brpb_copy_particle(
            b1.rpb.scene_particles + b1.rpb.scene_particles_used++,
            fd->particles + index);
    boundary->scene_dlight_offset = b1.rpb.scene_dlights_used;
    boundary->scene_dlight_count = (uint32_t)fd->num_dlights;
    for (int index = 0; index < fd->num_dlights; index++)
        b1_phase5brpb_copy_dlight(
            b1.rpb.scene_dlights + b1.rpb.scene_dlights_used++,
            fd->dlights + index);
    boundary->scene_lightstyle_offset = b1.rpb.scene_lightstyles_used;
    boundary->scene_lightstyle_count = MAX_LIGHTSTYLES;
    for (int index = 0; index < MAX_LIGHTSTYLES; index++)
        b1.rpb.scene_lightstyles[b1.rpb.scene_lightstyles_used++].white =
            fd->lightstyles[index].white;
    boundary->scene_area_offset = b1.rpb.scene_areabits_used;
    boundary->scene_area_bytes = (uint32_t)boundary->frame.areabytes;
    if (boundary->scene_area_bytes) {
        memcpy(b1.rpb.scene_areabits + b1.rpb.scene_areabits_used,
               fd->areabits, boundary->scene_area_bytes);
        b1.rpb.scene_areabits_used += boundary->scene_area_bytes;
    }

    /* The post-build observer must not copy live pointers, feedback, private
     * B0/B1 state, padding or inactive decal capacity.  Retain only the
     * bounded active fields consumed later by the RP-B serializers. */
    snapshot = &boundary->scene_refdef;
    memset(snapshot, 0, sizeof(*snapshot));
    memcpy(snapshot->blend, fd->blend, sizeof(snapshot->blend));
    /* Raw ordinary refdef time is one finite telemetry scalar only.  The P/E
     * serializers exclude it; paired-plan scene time is the renderer input. */
    snapshot->time = fd->time;
    snapshot->num_entities = fd->num_entities;
    snapshot->entities = b1.rpb.scene_entities + boundary->scene_entity_offset;
    snapshot->num_particles = fd->num_particles;
    snapshot->particles = b1.rpb.scene_particles +
        boundary->scene_particle_offset;
    snapshot->num_dlights = fd->num_dlights;
    snapshot->dlights = b1.rpb.scene_dlights + boundary->scene_dlight_offset;
    snapshot->lightstyles = b1.rpb.scene_lightstyles +
        boundary->scene_lightstyle_offset;
    snapshot->areabits = boundary->scene_area_bytes ?
        b1.rpb.scene_areabits + boundary->scene_area_offset : NULL;
    /* The accepted legacy decal cross-check is exactly 12 zero bytes; the
     * RP-B decal semantic is the canonical empty set, so no ring storage is
     * observed or copied. */
    memcpy(&boundary->particle_size, &b1.rpb.particle_size_bits,
           sizeof(boundary->particle_size));
    /* vkpt_debugdraw_have and VKPT_DrawDebugLines consume this exact active-list
     * predicate.  The hot observer records it as 0/1: only zero can satisfy the
     * frozen zero-source selector, and enumerating its nodes here is forbidden. */
    boundary->debug_primitive_count =
        LIST_EMPTY(&r_debug_lines_active) ? 0u : 1u;
    boundary->scene_closed = true;
    b1.rpb.scene_closure_count++;
    b1_phase5brpbr4_close_entity_alignment(boundary);
    b1.rpb.pending_token_id = 0;
    b1.rpb.pending_boundary = 0;
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0) {
        b1.rpb.controlled_stop_requested = true;
        b1.rpb.playback_state = B1_PHASE5BRPB_PLAYBACK_P0_CLOSED;
        B1_RequestDemoPlaybackStop();
        return false;
    }
    b1.rpb.playback_state = B1_PHASE5BRPB_PLAYBACK_OPEN;
    return true;
}

static const char *b1_phase5brpb_configstring(
    const b1_phase5brpb_boundary_t *boundary, uint32_t index, size_t *bytes,
    uint32_t *next_index)
{
    if (boundary->config_bytes != (size_t)boundary->csr.end * MAX_QPATH ||
        !boundary->csr.end || boundary->csr.end > q_countof(cl.configstrings) ||
        index >= boundary->csr.end)
        b1_fail("Phase-5B-RP-B configstring index/storage is malformed");
    return b1_phase5brpb_logical_configstring(
        b1.rpb.config_storage + boundary->config_offset,
        boundary->config_bytes, &boundary->csr, index, bytes, next_index);
}

static void b1_phase5brpb_model_semantics(
    b1_candidate_buffer_t *buffer, const b1_phase5brpb_model_key_t *model)
{
    if (!model || !model->present)
        b1_fail("Phase-5B-RP-B active model key is absent");
    b1_candidate_host_string(buffer, model->name, sizeof(model->name));
    b1_candidate_i32(buffer, model->type);
    b1_candidate_i32(buffer, model->model_class);
    b1_candidate_i32(buffer, model->num_meshes);
    b1_candidate_i32(buffer, model->num_frames);
    b1_candidate_i32(buffer, model->num_light_polys);
    b1_candidate_u8(buffer, model->has_meshes ? 1u : 0u);
    b1_candidate_u8(buffer, model->has_iqm ? 1u : 0u);
    if (model->has_iqm) {
        b1_candidate_u32(buffer, model->iqm_vertexes);
        b1_candidate_u32(buffer, model->iqm_triangles);
        b1_candidate_u32(buffer, model->iqm_frames);
        b1_candidate_u32(buffer, model->iqm_meshes);
        b1_candidate_u32(buffer, model->iqm_joints);
        b1_candidate_u32(buffer, model->iqm_poses);
        b1_candidate_u32(buffer, model->iqm_animations);
    }
}

static void b1_phase5brpb_image_semantics(
    b1_candidate_buffer_t *buffer, const b1_phase5brpb_image_key_t *image)
{
    if (!image || !image->present)
        b1_fail("Phase-5B-RP-B active image key is absent");
    b1_candidate_host_string(buffer, image->name, sizeof(image->name));
    b1_candidate_host_string(buffer, image->filepath, sizeof(image->filepath));
    b1_candidate_i32(buffer, image->type);
    b1_candidate_u32(buffer, image->flags);
    b1_candidate_i16(buffer, image->width);
    b1_candidate_i16(buffer, image->height);
    b1_candidate_i16(buffer, image->upload_width);
    b1_candidate_i16(buffer, image->upload_height);
    b1_candidate_i32(buffer, image->is_srgb);
}

static void b1_phase5brpb_plan_view_semantics(
    b1_candidate_buffer_t *buffer, const b1_plan_view_t *view)
{
    b1_candidate_u32(buffer, view->acceptance);
    for (uint32_t index = 0; index < 4u; index++)
        b1_candidate_i32(buffer, view->viewport[index]);
    b1_phase5brpb_vec(buffer, view->view_origin, 3u,
                      "paired-plan view origin");
    b1_phase5brpb_vec(buffer, view->view_angles, 3u,
                      "paired-plan view angles");
    b1_phase5brpb_vec(buffer, view->fov_xy, 2u, "paired-plan FOV");
    b1_candidate_u32(buffer, view->render_extent[0]);
    b1_candidate_u32(buffer, view->render_extent[1]);
    b1_candidate_u32(buffer, view->taa_extent[0]);
    b1_candidate_u32(buffer, view->taa_extent[1]);
    b1_phase5brpb_vec(buffer, view->projection_fov_scale, 2u,
                      "paired-plan projection scale");
    b1_phase5brpb_f32(buffer, view->cylindrical_hfov,
                      "paired-plan cylindrical HFOV");
    b1_phase5brpb_vec(buffer, view->V, 16u, "paired-plan V");
    b1_phase5brpb_vec(buffer, view->invV, 16u, "paired-plan inverse V");
    b1_phase5brpb_vec(buffer, view->P, 16u, "paired-plan P");
    b1_phase5brpb_vec(buffer, view->invP, 16u, "paired-plan inverse P");
}

static bool b1_phase5brpb_persistent_entity(
    const entity_t *entity, const b1_phase5brpb_model_key_t **model)
{
    if (entity->flags & RF_BEAM)
        return false;
    if (entity->model & (int)UINT32_C(0x80000000)) {
        *model = NULL;
        return true;
    }
    *model = b1_phase5brpb_model_key(entity->model, true);
    if (!*model || !(*model)->has_meshes)
        return false;
    return (*model)->type != MOD_SPRITE;
}

static uint8_t b1_phase5brpb_entity_route(const entity_t *entity)
{
    /* prepare_entities routes VIEWER before WEAPON and every remaining model
     * through its ordinary/model-class branch (vkpt/main.c:8642-8673). */
    if (entity->flags & RF_VIEWERMODEL)
        return 1u;
    if (entity->flags & RF_WEAPONMODEL)
        return 2u;
    return 0u;
}

static uint8_t b1_phase5brpb_effective_shell(const entity_t *entity)
{
    uint8_t shell = 0u;

    /* compute_mesh_material_flags applies this input only to non-glass mesh
     * material branches.  Runtime-data authority binds that derived material
     * classification; this stream records the exact effective entity input. */
    if ((b1.rpb.plan_record0.rdflags & RDF_IRGOGGLES) &&
        (entity->flags & RF_IR_VISIBLE))
        return SHELL_RED;
    if (entity->flags & RF_SHELL_HALF_DAM)
        shell |= SHELL_HALF_DAM;
    if (entity->flags & RF_SHELL_DOUBLE)
        shell |= SHELL_DOUBLE;
    if (entity->flags & RF_SHELL_RED)
        shell |= SHELL_RED;
    if (entity->flags & RF_SHELL_GREEN)
        shell |= SHELL_GREEN;
    if (entity->flags & RF_SHELL_BLUE)
        shell |= SHELL_BLUE;
    return shell;
}

static void b1_phase5brpb_entity_semantics(b1_candidate_buffer_t *buffer,
                                           const entity_t *entity)
{
    const b1_phase5brpb_model_key_t *model = NULL;
    bool translucent;
    float scale;
    if (!b1_phase5brpb_persistent_entity(entity, &model))
        b1_fail("Phase-5B-RP-B inactive entity reached P serialization");
    if (!isfinite(entity->scale) || entity->id < 0 ||
        (uint32_t)entity->id >= B1_PHASE5BRPB_MAX_ENTITY_ID)
        b1_fail("Phase-5B-RP-B active entity scale or id is malformed");
    if (!model) {
        uint32_t submodel = (uint32_t)(~entity->model);
        translucent = (entity->flags & RF_TRANSLUCENT) != 0;
        if (!cl.bsp || submodel < 1u ||
            submodel >= (uint32_t)cl.bsp->nummodels || entity->frame < 0)
            b1_fail("Phase-5B-RP-B inline BSP submodel is out of range");
        b1_candidate_u8(buffer, 0u);
        b1_candidate_u32(buffer, submodel);
        b1_phase5brpb_vec(buffer, entity->origin, 3, "inline entity origin");
        b1_phase5brpb_vec(buffer, entity->oldorigin, 3,
                          "inline entity old origin");
        b1_phase5brpb_vec(buffer, entity->angles, 3, "inline entity angles");
        scale = entity->scale > 0.0f ? entity->scale : 1.0f;
        b1_phase5brpb_f32(buffer, scale, "inline entity effective scale");
        b1_phase5brpb_f32(buffer, entity->backlerp,
                          "inline entity backlerp");
        b1_candidate_i32(buffer, entity->frame);
        b1_candidate_i32(buffer, entity->id);
        b1_candidate_u8(buffer, translucent ? 1u : 0u);
        if (translucent)
            b1_phase5brpb_f32(buffer, entity->alpha,
                              "inline entity alpha");
        return;
    }
    if (entity->frame < 0 || entity->oldframe < 0)
        b1_fail("Phase-5B-RP-B regular entity frame is negative");
    b1_candidate_u8(buffer, 1u);
    b1_phase5brpb_model_semantics(buffer, model);
    b1_phase5brpb_vec(buffer, entity->origin, 3, "entity origin");
    b1_phase5brpb_vec(buffer, entity->oldorigin, 3, "entity old origin");
    b1_phase5brpb_vec(buffer, entity->angles, 3, "entity angles");
    scale = entity->scale > 0.0f ? entity->scale : 1.0f;
    b1_phase5brpb_f32(buffer, scale, "entity effective scale");
    b1_candidate_i32(buffer, entity->frame);
    b1_candidate_i32(buffer, entity->oldframe);
    b1_phase5brpb_f32(buffer, entity->backlerp, "entity backlerp");
    b1_candidate_i32(buffer, entity->id);
    b1_candidate_u8(buffer, b1_phase5brpb_entity_route(entity));
    if (model->num_light_polys > 0)
        b1_candidate_u8(buffer,
            (entity->flags & RF_WEAPONMODEL) ? 1u : 0u);
    translucent = (entity->flags & RF_TRANSLUCENT) != 0;
    b1_candidate_u8(buffer, translucent ? 1u : 0u);
    if (translucent)
        b1_phase5brpb_f32(buffer, entity->alpha, "entity alpha");
    b1_candidate_u8(buffer, b1_phase5brpb_effective_shell(entity));
    if (entity->skin) {
        b1_candidate_u8(buffer, 1u);
        b1_phase5brpb_image_semantics(buffer,
            b1_phase5brpb_image_key(entity->skin));
    } else {
        b1_candidate_u8(buffer, 0u);
        b1_candidate_i32(buffer, entity->skinnum);
    }
}

static void b1_phase5brpb_dlight_semantics(b1_candidate_buffer_t *buffer,
                                           const dlight_t *light)
{
    if (light->light_type != DLIGHT_SPHERE &&
        light->light_type != DLIGHT_SPOT)
        b1_fail("Phase-5B-RP-B dynamic-light type is malformed");
    b1_phase5brpb_vec(buffer, light->origin, 3, "dynamic-light origin");
    b1_phase5brpb_vec(buffer, light->color, 3, "dynamic-light colour");
    b1_phase5brpb_f32(buffer, light->intensity, "dynamic-light intensity");
    b1_phase5brpb_f32(buffer, light->radius, "dynamic-light radius");
    b1_candidate_i32(buffer, light->light_type);
    if (light->light_type != DLIGHT_SPOT)
        return;
    if (light->spot.emission_profile != DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF &&
        light->spot.emission_profile !=
            DLIGHT_SPOT_EMISSION_PROFILE_AXIS_ANGLE_TEXTURE)
        b1_fail("Phase-5B-RP-B spotlight profile is malformed");
    b1_candidate_i32(buffer, light->spot.emission_profile);
    b1_phase5brpb_vec(buffer, light->spot.direction, 3,
                      "dynamic-light direction");
    if (light->spot.emission_profile == DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF) {
        b1_phase5brpb_f32(buffer, light->spot.cos_total_width,
                          "spotlight total width cosine");
        b1_phase5brpb_f32(buffer, light->spot.cos_falloff_start,
                          "spotlight falloff cosine");
    } else {
        b1_phase5brpb_f32(buffer, light->spot.total_width,
                          "spotlight total width");
        b1_phase5brpb_image_semantics(buffer,
            b1_phase5brpb_image_key(light->spot.texture));
    }
}

static void b1_phase5brpb_close_range(
    b1_candidate_buffer_t *buffer, b1_phase5brpb_boundary_t *boundary,
    uint32_t ordinal, const char *name, size_t start)
{
    b1_phase5brpb_range_t *range;
    size_t bytes;
    if (ordinal >= B1_PHASE5BRPB_RANGE_COUNT || start >= buffer->bytes ||
        start > UINT32_MAX || buffer->bytes - start > UINT32_MAX)
        b1_fail("Phase-5B-RP-B P range bounds are malformed");
    bytes = buffer->bytes - start;
    range = boundary->ranges + ordinal;
    range->name = name;
    range->offset = (uint32_t)start;
    range->bytes = (uint32_t)bytes;
    if (!b1_sha256_data(buffer->data + start, bytes, range->sha256))
        b1_fail("Phase-5B-RP-B cannot hash a P component range");
}

static void b1_phase5brpb_serialise_persistent(
    b1_candidate_buffer_t *buffer, b1_phase5brpb_boundary_t *boundary)
{
    static const char *const names[B1_PHASE5BRPB_RANGE_COUNT] = {
        "authority", "blend", "entities", "dynamic-lights", "light-styles",
        "decals-empty", "area-pvs", "paired-plan-replacement"
    };
    const refdef_t *fd = &boundary->scene_refdef;
    size_t start;
    size_t bytes;
    uint32_t persistent_entities = 0;
    byte settings_digest[32];

    if (!b1_sha256_data(b1.settings, B1_SETTINGS_BYTES, settings_digest))
        b1_fail("Phase-5B-RP-B cannot hash paired settings");
    start = buffer->bytes;
    b1_candidate_string_bytes(buffer, B1_PHASE5BRPB_PERSISTENT_SCHEMA,
                              strlen(B1_PHASE5BRPB_PERSISTENT_SCHEMA));
    b1_candidate_u32(buffer, 1u);
    b1_candidate_append(buffer, b1.rpb.plan_sha256, 32);
    b1_candidate_append(buffer, settings_digest, 32);
    b1_candidate_append(buffer, b1.rpb.fixture_sha256, 32);
    /* This is the frozen four-member fixture-closure authority from plan
     * §4.1, not a producer, executable or observer-tool identity. */
    b1_candidate_append(buffer, b1.rpb.closure_sha256, 32);
    /* Producer/tool identities remain in the evidence envelope.  P binds only
     * the versioned runtime-data closure supplied by the authenticated host. */
    b1_candidate_append(buffer, b1.rpb.runtime_data_sha256, 32);
    b1_candidate_append(buffer, b1.rpb.startup_sha256, 32);
    b1_candidate_i32(buffer, boundary->server_protocol);
    b1_candidate_i32(buffer, boundary->protocol_version);
    b1_candidate_u32(buffer, boundary->es_flags);
    b1_candidate_u32(buffer, boundary->ps_flags);
    b1_candidate_i32(buffer, boundary->server_count);
    b1_candidate_i32(buffer, boundary->server_state);
    b1_candidate_i32(buffer, boundary->client_num);
    b1_candidate_host_string(buffer, boundary->gamedir,
                             sizeof(boundary->gamedir));
    b1_candidate_host_string(buffer, boundary->mapname,
                             sizeof(boundary->mapname));
    b1_candidate_u8(buffer, boundary->csr.extended ? 1u : 0u);
    b1_candidate_u16(buffer, boundary->csr.max_edicts);
    b1_candidate_u16(buffer, boundary->csr.max_models);
    b1_candidate_u16(buffer, boundary->csr.max_sounds);
    b1_candidate_u16(buffer, boundary->csr.max_images);
    b1_candidate_u16(buffer, boundary->csr.airaccel);
    b1_candidate_u16(buffer, boundary->csr.maxclients);
    b1_candidate_u16(buffer, boundary->csr.mapchecksum);
    b1_candidate_u16(buffer, boundary->csr.models);
    b1_candidate_u16(buffer, boundary->csr.sounds);
    b1_candidate_u16(buffer, boundary->csr.images);
    b1_candidate_u16(buffer, boundary->csr.lights);
    b1_candidate_u16(buffer, boundary->csr.items);
    b1_candidate_u16(buffer, boundary->csr.playerskins);
    b1_candidate_u16(buffer, boundary->csr.general);
    b1_candidate_u16(buffer, boundary->csr.end);
    b1_candidate_u32(buffer, boundary->config_logical_count);
    uint32_t emitted_configstrings = 0;
    for (uint32_t index = 0, next; index < boundary->csr.end; index = next) {
        const char *value = b1_phase5brpb_configstring(
            boundary, index, &bytes, &next);
        b1_candidate_u32(buffer, index);
        b1_candidate_string_bytes(buffer, value, bytes);
        emitted_configstrings++;
    }
    if (emitted_configstrings != boundary->config_logical_count)
        b1_fail("Phase-5B-RP-B persistent configstring count differs");
    b1_phase5brpb_close_range(buffer, boundary, 0u, names[0], start);

    start = buffer->bytes;
    b1_phase5brpb_vec(buffer, fd->blend, 4, "scene blend");
    b1_phase5brpb_close_range(buffer, boundary, 1u, names[1], start);

    for (uint32_t index = 0; index < boundary->scene_entity_count; index++) {
        const b1_phase5brpb_model_key_t *model;
        if (b1_phase5brpb_persistent_entity(fd->entities + index, &model))
            persistent_entities++;
    }
    start = buffer->bytes;
    b1_candidate_u32(buffer, persistent_entities);
    for (uint32_t index = 0; index < boundary->scene_entity_count; index++) {
        const b1_phase5brpb_model_key_t *model;
        if (b1_phase5brpb_persistent_entity(fd->entities + index, &model))
            b1_phase5brpb_entity_semantics(buffer, fd->entities + index);
    }
    b1_phase5brpb_close_range(buffer, boundary, 2u, names[2], start);

    start = buffer->bytes;
    b1_candidate_u32(buffer, boundary->scene_dlight_count);
    for (uint32_t index = 0; index < boundary->scene_dlight_count; index++)
        b1_phase5brpb_dlight_semantics(buffer, fd->dlights + index);
    b1_phase5brpb_close_range(buffer, boundary, 3u, names[3], start);

    start = buffer->bytes;
    b1_candidate_u32(buffer, MAX_LIGHTSTYLES);
    for (uint32_t index = 0; index < MAX_LIGHTSTYLES; index++) {
        float white = b1_phase5brpb_effective_light_style(
            fd->lightstyles[index].white);
        b1_phase5brpb_f32(buffer, white, "effective light style");
    }
    b1_phase5brpb_close_range(buffer, boundary, 4u, names[4], start);

    start = buffer->bytes;
    b1_candidate_u32(buffer, 0u);
    b1_phase5brpb_close_range(buffer, boundary, 5u, names[5], start);

    start = buffer->bytes;
    b1_candidate_u32(buffer, boundary->scene_area_bytes);
    if (boundary->scene_area_bytes)
        b1_candidate_append(buffer, fd->areabits, boundary->scene_area_bytes);
    b1_phase5brpb_close_range(buffer, boundary, 6u, names[6], start);

    start = buffer->bytes;
    b1_candidate_u32(buffer, b1.rpb.plan_record0.logical_index);
    b1_candidate_u32(buffer, b1.rpb.plan_record0.source_frame_id);
    b1_candidate_i64(buffer, b1.rpb.plan_record0.relative_time_ns);
    b1_candidate_i64(buffer, b1.rpb.plan_record0.frame_duration_ns);
    b1_phase5brpb_f32(buffer, b1.rpb.plan_record0.scene_time,
                      "paired-plan scene time");
    b1_candidate_i32(buffer, b1.rpb.plan_record0.rdflags);
    for (uint32_t index = 0; index < 4; index++)
        b1_candidate_i32(buffer, b1.rpb.plan_record0.gameplay_viewport[index]);
    for (uint32_t role = 0; role < B1_ROLE_COUNT; role++)
        b1_phase5brpb_plan_view_semantics(
            buffer, b1.rpb.plan_record0.views + role);
    b1_phase5brpb_close_range(buffer, boundary, 7u, names[7], start);

    for (uint32_t index = 0; index < B1_PHASE5BRPB_RANGE_COUNT; index++) {
        const b1_phase5brpb_range_t *range = boundary->ranges + index;
        if (strcmp(range->name, names[index]) ||
            range->offset != (index ? boundary->ranges[index - 1u].offset +
                boundary->ranges[index - 1u].bytes : 0u))
            b1_fail("Phase-5B-RP-B P range coverage/order differs");
    }
    if ((size_t)boundary->ranges[B1_PHASE5BRPB_RANGE_COUNT - 1u].offset +
        boundary->ranges[B1_PHASE5BRPB_RANGE_COUNT - 1u].bytes != buffer->bytes ||
        !buffer->bytes || buffer->bytes > UINT32_MAX ||
        !b1_sha256_data(buffer->data, buffer->bytes,
                        boundary->persistent_sha256))
        b1_fail("Phase-5B-RP-B P canonical closure differs");
    boundary->persistent_bytes = (uint32_t)buffer->bytes;
}

static void b1_phase5brpb_player_semantics(b1_candidate_buffer_t *buffer,
                                             const player_state_t *player,
                                             bool current,
                                             bool kick_angles_active)
{
    static const char schema[] =
        "B1-SF-A1-PHASE5BRPB-ENDPOINT-PLAYER-V1";

    b1_candidate_string_bytes(buffer, schema, sizeof(schema) - 1u);
    b1_candidate_u32(buffer, 1u);
    b1_candidate_u8(buffer, current ? 1u : 0u);
    for (uint32_t i = 0; i < 3; i++)
        b1_candidate_i16(buffer, player->pmove.origin[i]);
    b1_candidate_u8(buffer,
        (player->pmove.pm_flags & PMF_TELEPORT_BIT) ? 1u : 0u);
    b1_phase5brpb_vec(buffer, player->viewangles, 3, "player view angles");
    b1_phase5brpb_vec(buffer, player->viewoffset, 3, "player view offset");
    b1_candidate_u8(buffer, kick_angles_active ? 1u : 0u);
    if (kick_angles_active)
        b1_phase5brpb_vec(buffer, player->kick_angles, 3,
                          "player kick angles");
    b1_phase5brpb_vec(buffer, player->gunangles, 3, "player gun angles");
    b1_phase5brpb_vec(buffer, player->gunoffset, 3, "player gun offset");
    if (current)
        b1_candidate_i32(buffer, player->gunindex);
    b1_candidate_i32(buffer, player->gunframe);
    if (current)
        b1_phase5brpb_vec(buffer, player->blend, 4, "player blend");
}

static void b1_phase5brpb_endpoint_entity_semantics(
    b1_candidate_buffer_t *buffer, const b1_phase5brpb_boundary_t *boundary,
    const centity_state_t *entity)
{
    bool alpha_active;
    bool scale_active;
    bool loop_admitted;
    float effective;

    b1_candidate_i32(buffer, entity->number);
    b1_phase5brpb_vec(buffer, entity->origin, 3, "endpoint entity origin");
    b1_phase5brpb_vec(buffer, entity->angles, 3, "endpoint entity angles");
    b1_phase5brpb_vec(buffer, entity->old_origin, 3, "endpoint entity old origin");
    b1_candidate_i32(buffer, entity->modelindex);
    b1_candidate_i32(buffer, entity->modelindex2);
    b1_candidate_i32(buffer, entity->modelindex3);
    b1_candidate_i32(buffer, entity->modelindex4);
    b1_candidate_i32(buffer, entity->frame);
    b1_candidate_i32(buffer, entity->skinnum);
    b1_candidate_u32(buffer, entity->effects);
    b1_candidate_i32(buffer, entity->renderfx);
    b1_candidate_i32(buffer, entity->solid);
    b1_candidate_i32(buffer, entity->sound);
    b1_candidate_i32(buffer, entity->event);
    if ((uint32_t)entity->morefx & ~(uint32_t)B1_PHASE5BRPB_KNOWN_MOREFX)
        b1_fail("Phase-5B-RP-B endpoint entity has unknown morefx bits");
    b1_candidate_u32(buffer,
        (uint32_t)entity->morefx & (uint32_t)B1_PHASE5BRPB_KNOWN_MOREFX);

    alpha_active = b1_phase5brpb_endpoint_alpha_active(boundary, entity);
    b1_candidate_u8(buffer, alpha_active ? 1u : 0u);
    if (alpha_active)
        b1_phase5brpb_f32(buffer, entity->alpha,
                          "active endpoint entity alpha");

    scale_active = b1_phase5brpb_endpoint_reaches_base_entity(boundary, entity) &&
        !(entity->renderfx & RF_BEAM);
    b1_candidate_u8(buffer, scale_active ? 1u : 0u);
    if (scale_active) {
        effective = entity->scale > 0.0f ? entity->scale : 1.0f;
        b1_phase5brpb_f32(buffer, effective,
                          "effective endpoint entity scale");
    }

    loop_admitted = b1_phase5brpb_endpoint_loop_admitted(boundary, entity);
    b1_candidate_u8(buffer, loop_admitted ? 1u : 0u);
    if (loop_admitted) {
        effective = entity->loop_volume != 0.0f ?
            entity->loop_volume : 1.0f;
        b1_phase5brpb_f32(buffer, effective,
                          "effective endpoint loop volume");
        if (entity->loop_attenuation == ATTN_LOOP_NONE)
            effective = 0.0f;
        else if (entity->loop_attenuation != 0.0f &&
                 entity->loop_attenuation != ATTN_STATIC)
            effective = entity->loop_attenuation * SOUND_LOOPATTENUATE_MULT;
        else
            effective = SOUND_LOOPATTENUATE;
        b1_phase5brpb_f32(buffer, effective,
                          "effective endpoint loop distance multiplier");
    }
}

static void b1_phase5brpb_particle_semantics(
    b1_candidate_buffer_t *buffer, const b1_phase5brpb_boundary_t *boundary)
{
    b1_candidate_string_bytes(buffer, B1_PHASE5BRPB_PARTICLE_SCHEMA,
                              strlen(B1_PHASE5BRPB_PARTICLE_SCHEMA));
    b1_candidate_u32(buffer, 2u);
    b1_candidate_u32(buffer, boundary->scene_particle_count);
    for (uint32_t index = 0; index < boundary->scene_particle_count; index++) {
        const particle_t *particle = boundary->scene_refdef.particles + index;
        uint32_t rgba = particle->rgba.u32;
        b1_phase5brpb_vec(buffer, particle->origin, 3, "particle origin");
        if (particle->color >= 0) {
            b1_candidate_u8(buffer, 0u);
            b1_candidate_u8(buffer, (uint8_t)particle->color);
        } else {
            b1_candidate_u8(buffer, 1u);
            b1_candidate_u8(buffer, (uint8_t)rgba);
            b1_candidate_u8(buffer, (uint8_t)(rgba >> 8));
            b1_candidate_u8(buffer, (uint8_t)(rgba >> 16));
        }
        if (!isfinite(particle->alpha) || particle->alpha <= 0.0f ||
            particle->alpha > 1.0f || !isfinite(particle->brightness) ||
            particle->brightness < 0.0f || !isfinite(particle->radius) ||
            particle->radius < 0.0f)
            b1_fail("Phase-5B-RP-B particle semantic range is malformed");
        b1_phase5brpb_f32(buffer, particle->alpha, "particle alpha");
        b1_phase5brpb_f32(buffer, particle->brightness, "particle brightness");
        if (particle->radius == 0.0f) {
            if (!isfinite(boundary->particle_size) ||
                boundary->particle_size <= 0.0f)
                b1_fail("Phase-5B-RP-B default particle size is malformed");
            b1_candidate_u8(buffer, 0u);
            b1_phase5brpb_f32(buffer, boundary->particle_size,
                              "default particle size");
        } else {
            b1_candidate_u8(buffer, 1u);
            b1_phase5brpb_f32(buffer, particle->radius, "particle radius");
        }
    }
}

static void b1_phase5brpb_classify_sources(
    b1_phase5brpb_boundary_t *boundary)
{
    if (boundary->beam_entity_count || boundary->active_beam_count ||
        boundary->resolved_sprite_count)
        b1_fail("Phase-5B-RP-B source classification was repeated");
    /* beamEntities is the raw protocol-source inventory.  It is deliberately
     * independent from activeBeams: extended CL_DrawBeam paths can remove the
     * RF_BEAM flag from generated scene entities, while tent/laser paths can
     * create renderer-facing beams without one raw endpoint beam. */
    for (uint32_t index = 0; index < boundary->endpoint_entity_count; index++) {
        const centity_state_t *entity = b1.rpb.endpoint_entities +
            boundary->endpoint_entity_offset + index;
        if (entity->renderfx & RF_BEAM)
            boundary->beam_entity_count++;
    }
    for (uint32_t index = 0; index < boundary->scene_entity_count; index++) {
        const entity_t *entity = boundary->scene_refdef.entities + index;
        const b1_phase5brpb_model_key_t *model;

        if (entity->flags & RF_BEAM) {
            if (entity->frame > 0)
                boundary->active_beam_count++;
            continue;
        }
        if (entity->model & (int)UINT32_C(0x80000000))
            continue;
        model = b1_phase5brpb_model_key(entity->model, true);
        if (model && model->type == MOD_SPRITE)
            boundary->resolved_sprite_count++;
    }
}

static void b1_phase5brpb_sprite_source_semantics(
    b1_candidate_buffer_t *buffer, const b1_phase5brpb_boundary_t *boundary)
{
    static const char schema[] =
        "B1-SF-A1-PHASE5BRPB-RESOLVED-SPRITE-SOURCES-V1";

    b1_candidate_string_bytes(buffer, schema, sizeof(schema) - 1u);
    b1_candidate_u32(buffer, 1u);
    b1_candidate_u32(buffer, boundary->resolved_sprite_count);
    uint32_t emitted = 0;
    for (uint32_t index = 0; index < boundary->scene_entity_count; index++) {
        const entity_t *entity = boundary->scene_refdef.entities + index;
        const b1_phase5brpb_model_key_t *model;
        const b1_phase5brpb_sprite_frame_key_t *frame;
        const b1_phase5brpb_image_key_t *image;
        uint32_t resolved_frame;

        if ((entity->flags & RF_BEAM) ||
            (entity->model & (int)UINT32_C(0x80000000)))
            continue;
        model = b1_phase5brpb_model_key(entity->model, true);
        if (!model || model->type != MOD_SPRITE)
            continue;
        if (entity->frame < 0 || !model->sprite_frame_count ||
            model->sprite_frame_count != (uint32_t)model->num_frames)
            b1_fail("Phase-5B-RP-B sprite-source frame range is malformed");
        resolved_frame = (uint32_t)entity->frame % model->sprite_frame_count;
        frame = b1.rpb.sprite_frame_keys + model->sprite_frame_offset +
            resolved_frame;
        if (frame->image_key_index >= b1.rpb.image_key_count ||
            frame->width <= 0 || frame->height <= 0 ||
            !isfinite(entity->alpha) || entity->alpha < 0.0f ||
            entity->alpha > 1.0f)
            b1_fail("Phase-5B-RP-B sprite-source semantics are malformed");
        image = b1.rpb.image_keys + frame->image_key_index;
        b1_phase5brpb_model_semantics(buffer, model);
        b1_candidate_u8(buffer, model->sprite_vertical ? 1u : 0u);
        b1_candidate_u32(buffer, resolved_frame);
        b1_candidate_i32(buffer, frame->width);
        b1_candidate_i32(buffer, frame->height);
        b1_candidate_i32(buffer, frame->origin_x);
        b1_candidate_i32(buffer, frame->origin_y);
        b1_phase5brpb_image_semantics(buffer, image);
        b1_phase5brpb_vec(buffer, entity->origin, 3, "sprite-source origin");
        b1_phase5brpb_f32(buffer, entity->alpha, "sprite-source alpha");
        emitted++;
    }
    if (emitted != boundary->resolved_sprite_count)
        b1_fail("Phase-5B-RP-B sprite-source count differs from its ledger");
}

static void b1_phase5brpb_serialise_endpoint_scene(
    b1_candidate_buffer_t *buffer, const b1_phase5brpb_boundary_t *boundary)
{
    static const char schema[] =
        "B1-SF-A1-PHASE5BRPB-ENDPOINT-SCENE-COMPONENTS-V1";
    size_t bytes;
    int previous_number = 0;

    b1_candidate_string_bytes(buffer, schema, sizeof(schema) - 1u);
    b1_candidate_u32(buffer, 1u);
    b1_candidate_u8(buffer, boundary->frame.valid ? 1u : 0u);
    b1_candidate_u8(buffer, boundary->predecessor.valid ? 1u : 0u);
    b1_candidate_u32(buffer, boundary->recovered_flags &
        (FF_SERVERDROP | FF_BADFRAME | FF_OLDFRAME | FF_OLDENT | FF_NODELTA));
    b1_candidate_i32(buffer, boundary->frame.clientNum);
    b1_phase5brpb_player_semantics(buffer, &boundary->frame.ps, true,
                                   boundary->kick_angles_active);
    b1_phase5brpb_player_semantics(buffer, &boundary->predecessor.ps, false,
                                   boundary->kick_angles_active);
    b1_candidate_u32(buffer, boundary->frame.areabytes);
    if (boundary->frame.areabytes)
        b1_candidate_append(buffer, boundary->frame.areabits,
                            (size_t)boundary->frame.areabytes);
    b1_candidate_u32(buffer, boundary->endpoint_entity_count);
    for (uint32_t index = 0; index < boundary->endpoint_entity_count; index++) {
        const centity_state_t *entity = b1.rpb.endpoint_entities +
            boundary->endpoint_entity_offset + index;
        if (entity->number <= previous_number ||
            entity->number >= boundary->csr.max_edicts)
            b1_fail("Phase-5B-RP-B endpoint entity order/range differs");
        previous_number = entity->number;
        b1_phase5brpb_endpoint_entity_semantics(buffer, boundary, entity);
    }
    b1_candidate_u32(buffer, boundary->config_logical_count);
    uint32_t emitted_configstrings = 0;
    for (uint32_t index = 0, next; index < boundary->csr.end; index = next) {
        const char *value = b1_phase5brpb_configstring(
            boundary, index, &bytes, &next);
        b1_candidate_u32(buffer, index);
        b1_candidate_string_bytes(buffer, value, bytes);
        emitted_configstrings++;
    }
    if (emitted_configstrings != boundary->config_logical_count)
        b1_fail("Phase-5B-RP-B endpoint configstring count differs");
    if (!boundary->particle_v2 || !boundary->particle_v2_bytes)
        b1_fail("Phase-5B-RP-B endpoint scene lacks particle-V2 closure");
    b1_candidate_append(buffer, boundary->particle_v2,
                        boundary->particle_v2_bytes);
}

static uint32_t b1_phase5brpb_register_seen(
    b1_candidate_seen_t *seen, uint32_t *count,
    const byte digest[32], const byte *canonical, size_t canonical_bytes,
    const char *label)
{
    for (uint32_t index = 0; index < *count; index++) {
        if (memcmp(seen[index].digest, digest, 32))
            continue;
        if (seen[index].canonical_bytes != canonical_bytes ||
            memcmp(seen[index].canonical, canonical, canonical_bytes))
            b1_fail("Phase-5B-RP-B %s SHA-256 collision was detected", label);
        seen[index].repeat_count++;
        return index;
    }
    if (*count >= B1_PHASE5BRPB_MAX_BOUNDARIES || !canonical_bytes)
        b1_fail("Phase-5B-RP-B %s identity table is exhausted", label);
    b1_candidate_seen_t *entry = seen + (*count)++;
    memcpy(entry->digest, digest, 32);
    entry->canonical = b1_phase5brpb_allocate(canonical_bytes, 1u, label);
    memcpy(entry->canonical, canonical, canonical_bytes);
    entry->canonical_bytes = canonical_bytes;
    entry->repeat_count = 1u;
    return *count - 1u;
}

static void b1_phase5brpb_token_identity(b1_phase5brpb_boundary_t *boundary)
{
    b1_candidate_buffer_t token = { 0 };
    static const char schema[] = "B1-SF-A1-PHASE5BRPB-PENDING-TOKEN-V1";
    if (!b1_sha256_data(b1.rpb.message_storage + boundary->message_offset,
                        boundary->message_bytes, boundary->message_sha256))
        b1_fail("Phase-5B-RP-B cannot hash a complete message");
    if (!boundary->token_id)
        return;
    b1_candidate_string_bytes(&token, schema, sizeof(schema) - 1u);
    b1_candidate_u64(&token, boundary->token_id);
    b1_candidate_u64(&token, boundary->ordinal);
    b1_candidate_append(&token, boundary->message_sha256, 32);
    if (!b1_sha256_data(token.data, token.bytes, boundary->token_sha256)) {
        b1_candidate_buffer_release(&token);
        b1_fail("Phase-5B-RP-B cannot hash a pending-scene token");
    }
    b1_candidate_buffer_release(&token);
}

static void b1_phase5brpbr4_serialise_entity(
    b1_candidate_buffer_t *buffer, const entity_t *entity)
{
    b1_candidate_i32(buffer, entity->model);
    b1_scene_vec_f32(buffer, entity->angles, 3);
    b1_scene_vec_f32(buffer, entity->origin, 3);
    b1_candidate_i32(buffer, entity->frame);
    b1_scene_vec_f32(buffer, entity->oldorigin, 3);
    b1_candidate_i32(buffer, entity->oldframe);
    b1_candidate_f32(buffer, entity->backlerp);
    b1_candidate_i32(buffer, entity->skinnum);
    b1_candidate_f32(buffer, entity->alpha);
    b1_candidate_u32(buffer, entity->rgba.u32);
    b1_candidate_i32(buffer, entity->skin);
    b1_candidate_i32(buffer, entity->flags);
    b1_candidate_i32(buffer, entity->id);
    b1_candidate_i32(buffer, entity->tent_type);
    b1_candidate_f32(buffer, entity->scale);
}

static void b1_phase5brpbr4_calculate_entity_alignment(
    const byte endpoint_sha256[32], const entity_t *raw_entities,
    uint32_t raw_count, const entity_t *projected_entities,
    uint32_t projected_count, b1_entity_alignment_calculation_t *result)
{
    b1_candidate_buffer_t raw_models = { 0 }, projected_models = { 0 };
    b1_candidate_buffer_t raw_membership = { 0 }, projected_membership = { 0 };
    byte expected[32], expected_endpoint[32];

    if (!result || !endpoint_sha256 || !raw_entities || !projected_entities ||
        raw_count != B1_PHASE5BRPBR2_ENTITY_COUNT ||
        projected_count != B1_PHASE5BRPBR2_ENTITY_COUNT ||
        !b1_parse_sha256(B1_PHASE5AR5_ACCEPTED_ENDPOINT_SHA256,
                         expected_endpoint) ||
        memcmp(endpoint_sha256, expected_endpoint, 32))
        b1_fail("B1 R4 canonical authority inputs are malformed");
    memset(result, 0, sizeof(*result));
    result->raw_count = raw_count;
    result->projected_count = projected_count;
    memcpy(result->endpoint_sha256, endpoint_sha256, 32);
    b1_candidate_u32(&result->raw, raw_count);
    b1_candidate_u32(&result->projected, projected_count);
    b1_candidate_u32(&raw_models, raw_count);
    b1_candidate_u32(&projected_models, projected_count);
    b1_candidate_u32(&raw_membership, raw_count);
    b1_candidate_u32(&projected_membership, projected_count);
    for (uint32_t index = 0; index < raw_count; index++) {
        b1_phase5brpbr4_serialise_entity(&result->raw, raw_entities + index);
        b1_phase5brpbr4_serialise_entity(
            &result->projected, projected_entities + index);
        if (!b1_sha256_data(result->raw.data + 4u + index * 84u, 84u,
                            result->raw_record_sha256[index]) ||
            !b1_sha256_data(result->projected.data + 4u + index * 84u, 84u,
                            result->projected_record_sha256[index]))
            b1_fail("B1 R4 cannot hash an entity record");
        b1_candidate_u32(&raw_models, index);
        b1_candidate_i32(&raw_models, raw_entities[index].model);
        b1_candidate_u32(&projected_models, index);
        b1_candidate_i32(&projected_models, projected_entities[index].model);
        b1_candidate_u32(&raw_membership, index);
        b1_candidate_i32(&raw_membership, raw_entities[index].model);
        b1_candidate_i32(&raw_membership, raw_entities[index].id);
        b1_candidate_u32(&projected_membership, index);
        b1_candidate_i32(&projected_membership,
                         projected_entities[index].model);
        b1_candidate_i32(&projected_membership, projected_entities[index].id);
    }
    if (result->raw.bytes != B1_PHASE5BRPBR2_ENTITY_BYTES ||
        result->projected.bytes != B1_PHASE5BRPBR2_ENTITY_BYTES ||
        !b1_sha256_data(result->raw.data, result->raw.bytes,
                        result->raw_entities_sha256) ||
        !b1_sha256_data(result->projected.data, result->projected.bytes,
                        result->projected_entities_sha256) ||
        !b1_sha256_data(raw_models.data, raw_models.bytes,
                        result->raw_model_sequence_sha256) ||
        !b1_sha256_data(projected_models.data, projected_models.bytes,
                        result->projected_model_sequence_sha256) ||
        !b1_sha256_data(raw_membership.data, raw_membership.bytes,
                        result->raw_membership_sha256) ||
        !b1_sha256_data(projected_membership.data, projected_membership.bytes,
                        result->projected_membership_sha256))
        b1_fail("B1 R4 cannot close entity identities");
    if (!b1_parse_sha256(B1_PHASE5BRPBR2_ENTITY_SHA256, expected) ||
        memcmp(result->raw_entities_sha256, expected, 32) ||
        !b1_parse_sha256(B1_PHASE5BRPBR4_PROJECTED_ENTITY_SHA256, expected) ||
        memcmp(result->projected_entities_sha256, expected, 32) ||
        !b1_parse_sha256(B1_PHASE5BRPBR4_MODEL_SEQUENCE_SHA256, expected) ||
        memcmp(result->raw_model_sequence_sha256, expected, 32) ||
        memcmp(result->projected_model_sequence_sha256, expected, 32) ||
        !b1_parse_sha256(B1_PHASE5BRPBR4_RAW_MEMBERSHIP_SHA256, expected) ||
        memcmp(result->raw_membership_sha256, expected, 32) ||
        !b1_parse_sha256(B1_PHASE5BRPBR4_PROJECTED_MEMBERSHIP_SHA256, expected) ||
        memcmp(result->projected_membership_sha256, expected, 32))
        b1_fail("B1 R4 entity alignment differs from frozen expectations");
    b1_candidate_buffer_release(&raw_models);
    b1_candidate_buffer_release(&projected_models);
    b1_candidate_buffer_release(&raw_membership);
    b1_candidate_buffer_release(&projected_membership);
}

static void b1_phase5brpbr4_release_entity_calculation(
    b1_entity_alignment_calculation_t *result)
{
    if (!result)
        return;
    b1_candidate_buffer_release(&result->raw);
    b1_candidate_buffer_release(&result->projected);
    memset(result, 0, sizeof(*result));
}

static void b1_phase5brpbr4_close_entity_alignment(
    b1_phase5brpb_boundary_t *boundary)
{
    b1_entity_alignment_calculation_t result = { 0 };
    const entity_t *raw_entities;
    const entity_t *projected_entities;
    byte zero[32] = { 0 };
    byte zero_records[B1_PHASE5BRPBR2_ENTITY_COUNT][32] = { { 0 } };
    byte *raw_copy = NULL, *projected_copy = NULL;
    char endpoint_text[65], raw_text[65], projected_text[65];
    char raw_models_text[65], projected_models_text[65];
    char raw_membership_text[65], projected_membership_text[65];
    bool revalidation;

    if (b1.rpb.mode != B1_PHASE5BRPB_MODE_P0)
        return;
    if (!boundary || !boundary->selected_anchor || !boundary->scene_closed ||
        !boundary->endpoint_identity_closed ||
        !boundary->endpoint_timing_closed || boundary->scene_build_count != 1u)
        b1_fail("Phase-5B-RP-B-R4 entity authority lacks the selected endpoint build");
    revalidation = boundary->entity_alignment_closed;
    if (!revalidation &&
        (boundary->entity_alignment_version != 0u ||
         boundary->entity_alignment_publication_count != 0u ||
         boundary->entity_alignment_revalidation_count != 0u ||
         boundary->raw_entities_v1 || boundary->projected_entities_v1 ||
         boundary->raw_entities_v1_bytes != 0u ||
         boundary->projected_entities_v1_bytes != 0u ||
         memcmp(boundary->raw_entities_sha256, zero, 32) ||
         memcmp(boundary->projected_entities_sha256, zero, 32) ||
         memcmp(boundary->raw_model_sequence_sha256, zero, 32) ||
         memcmp(boundary->projected_model_sequence_sha256, zero, 32) ||
         memcmp(boundary->raw_membership_sha256, zero, 32) ||
         memcmp(boundary->projected_membership_sha256, zero, 32) ||
         memcmp(boundary->raw_record_sha256, zero_records,
                sizeof(zero_records)) ||
         memcmp(boundary->projected_record_sha256, zero_records,
                sizeof(zero_records))))
        b1_fail("Phase-5B-RP-B-R6-R3 first authority close found partial publication");
    if (revalidation &&
        (boundary->entity_alignment_version !=
             B1_PHASE5BRPBR6R3_ENTITY_ALIGNMENT_VERSION ||
         boundary->entity_alignment_publication_count != 1u ||
         boundary->entity_alignment_revalidation_count != 0u ||
         !boundary->raw_entities_v1 || !boundary->projected_entities_v1 ||
         boundary->raw_entities_v1_bytes != B1_PHASE5BRPBR2_ENTITY_BYTES ||
         boundary->projected_entities_v1_bytes !=
             B1_PHASE5BRPBR2_ENTITY_BYTES))
        b1_fail("Phase-5B-RP-B-R6-R3 revalidation found noncanonical authority state");
    raw_entities = b1.rpb.raw_scene_entities +
        boundary->raw_scene_entity_offset;
    projected_entities = b1.rpb.scene_entities + boundary->scene_entity_offset;
    b1_phase5brpbr4_calculate_entity_alignment(
        boundary->endpoint_sha256, raw_entities,
        boundary->raw_scene_entity_count, projected_entities,
        boundary->scene_entity_count, &result);

    if (!revalidation) {
        raw_copy = b1_copy_required(result.raw.data, result.raw.bytes,
                                    "R4 raw entity authority");
        projected_copy = b1_copy_required(result.projected.data,
            result.projected.bytes, "R4 projected entity authority");
        boundary->raw_entities_v1 = raw_copy;
        boundary->projected_entities_v1 = projected_copy;
        boundary->raw_entities_v1_bytes = (uint32_t)result.raw.bytes;
        boundary->projected_entities_v1_bytes =
            (uint32_t)result.projected.bytes;
        memcpy(boundary->raw_entities_sha256,
               result.raw_entities_sha256, 32);
        memcpy(boundary->projected_entities_sha256,
               result.projected_entities_sha256, 32);
        memcpy(boundary->raw_model_sequence_sha256,
               result.raw_model_sequence_sha256, 32);
        memcpy(boundary->projected_model_sequence_sha256,
               result.projected_model_sequence_sha256, 32);
        memcpy(boundary->raw_membership_sha256,
               result.raw_membership_sha256, 32);
        memcpy(boundary->projected_membership_sha256,
               result.projected_membership_sha256, 32);
        memcpy(boundary->raw_record_sha256, result.raw_record_sha256,
               sizeof(result.raw_record_sha256));
        memcpy(boundary->projected_record_sha256,
               result.projected_record_sha256,
               sizeof(result.projected_record_sha256));
        boundary->entity_alignment_version =
            B1_PHASE5BRPBR6R3_ENTITY_ALIGNMENT_VERSION;
        boundary->entity_alignment_publication_count = 1u;
        boundary->entity_alignment_closed = true;
    } else {
        if (memcmp(boundary->raw_entities_v1,
                   result.raw.data, result.raw.bytes) ||
            memcmp(boundary->projected_entities_v1,
                   result.projected.data, result.projected.bytes) ||
            memcmp(boundary->raw_entities_sha256,
                   result.raw_entities_sha256, 32) ||
            memcmp(boundary->projected_entities_sha256,
                   result.projected_entities_sha256, 32) ||
            memcmp(boundary->raw_model_sequence_sha256,
                   result.raw_model_sequence_sha256, 32) ||
            memcmp(boundary->projected_model_sequence_sha256,
                   result.projected_model_sequence_sha256, 32) ||
            memcmp(boundary->raw_membership_sha256,
                   result.raw_membership_sha256, 32) ||
            memcmp(boundary->projected_membership_sha256,
                   result.projected_membership_sha256, 32) ||
            memcmp(boundary->raw_record_sha256,
                   result.raw_record_sha256,
                   sizeof(result.raw_record_sha256)) ||
            memcmp(boundary->projected_record_sha256,
                   result.projected_record_sha256,
                   sizeof(result.projected_record_sha256)))
            b1_fail("Phase-5B-RP-B-R6-R3 entity authority changed at revalidation");
        boundary->entity_alignment_revalidation_count = 1u;
    }
    b1_format_sha256(result.endpoint_sha256, endpoint_text);
    b1_format_sha256(result.raw_entities_sha256, raw_text);
    b1_format_sha256(result.projected_entities_sha256, projected_text);
    b1_format_sha256(result.raw_model_sequence_sha256, raw_models_text);
    b1_format_sha256(result.projected_model_sequence_sha256,
                     projected_models_text);
    b1_format_sha256(result.raw_membership_sha256, raw_membership_text);
    b1_format_sha256(result.projected_membership_sha256,
                     projected_membership_text);
    if (fprintf(b1.log,
        "{\"event\":\"%s\",\"schemaVersion\":%u,"
        "\"endpointSha256\":\"%s\",\"rawEntityCount\":%u,"
        "\"projectedEntityCount\":%u,\"rawCanonicalBytes\":%u,"
        "\"projectedCanonicalBytes\":%u,\"rawEntitiesSha256\":\"%s\","
        "\"projectedEntitiesSha256\":\"%s\","
        "\"rawModelSequenceSha256\":\"%s\","
        "\"projectedModelSequenceSha256\":\"%s\","
        "\"rawMembershipSha256\":\"%s\","
        "\"projectedMembershipSha256\":\"%s\","
        "\"publicationCount\":%u,\"revalidationCount\":%u,"
        "\"replacementAllocation\":false,\"identityOverwrite\":false,"
        "\"pointersInIdentity\":false,\"closed\":true}\n",
        revalidation ? "b1_r6r3_entity_alignment_revalidated" :
            "b1_r6r3_entity_alignment_closed",
        B1_PHASE5BRPBR6R3_ENTITY_ALIGNMENT_VERSION, endpoint_text,
        result.raw_count, result.projected_count,
        boundary->raw_entities_v1_bytes, boundary->projected_entities_v1_bytes,
        raw_text, projected_text, raw_models_text, projected_models_text,
        raw_membership_text, projected_membership_text,
        boundary->entity_alignment_publication_count,
        boundary->entity_alignment_revalidation_count) < 0 || fflush(b1.log))
        b1_fail("Phase-5B-RP-B-R6-R3 cannot persist entity authority closure");
    b1_phase5brpbr4_release_entity_calculation(&result);
}

static bool b1_phase5brpbr6r4_normal_authority_is_zero(
    const b1_entity_alignment_authority_t *authority)
{
    byte zero[32] = { 0 };
    byte zero_records[B1_PHASE5BRPBR2_ENTITY_COUNT][32] = { { 0 } };

    return authority && authority->version == 0u &&
        authority->publication_count == 0u &&
        authority->revalidation_count == 0u && authority->raw_count == 0u &&
        authority->projected_count == 0u &&
        authority->raw_canonical_bytes == 0u &&
        authority->projected_canonical_bytes == 0u &&
        authority->projection_rdflags == 0 && !authority->closed &&
        !authority->raw_entities_v1 && !authority->projected_entities_v1 &&
        !memcmp(authority->endpoint_sha256, zero, 32) &&
        !memcmp(authority->raw_entities_sha256, zero, 32) &&
        !memcmp(authority->projected_entities_sha256, zero, 32) &&
        !memcmp(authority->raw_model_sequence_sha256, zero, 32) &&
        !memcmp(authority->projected_model_sequence_sha256, zero, 32) &&
        !memcmp(authority->raw_membership_sha256, zero, 32) &&
        !memcmp(authority->projected_membership_sha256, zero, 32) &&
        !memcmp(authority->raw_record_sha256, zero_records,
                sizeof(zero_records)) &&
        !memcmp(authority->projected_record_sha256, zero_records,
                sizeof(zero_records));
}

static void b1_phase5brpbr6r4_log_normal_authority(
    const char *event, const b1_entity_alignment_authority_t *authority,
    uint32_t payload_allocation_count)
{
    char endpoint[65], raw[65], projected[65];
    char raw_models[65], projected_models[65];
    char raw_membership[65], projected_membership[65];

    if (!event || !authority || !authority->closed)
        b1_fail("B1 R6-R4 normal authority event lacks a closed record");
    b1_format_sha256(authority->endpoint_sha256, endpoint);
    b1_format_sha256(authority->raw_entities_sha256, raw);
    b1_format_sha256(authority->projected_entities_sha256, projected);
    b1_format_sha256(authority->raw_model_sequence_sha256, raw_models);
    b1_format_sha256(authority->projected_model_sequence_sha256,
                     projected_models);
    b1_format_sha256(authority->raw_membership_sha256, raw_membership);
    b1_format_sha256(authority->projected_membership_sha256,
                     projected_membership);
    if (fprintf(b1.log,
        "{\"event\":\"%s\",\"schemaVersion\":%u,"
        "\"owner\":\"NORMAL_B1_SCENE_CONTROLLER\","
        "\"endpointSha256\":\"%s\",\"rawEntityCount\":%u,"
        "\"projectedEntityCount\":%u,\"rawCanonicalBytes\":%u,"
        "\"projectedCanonicalBytes\":%u,\"rawEntitiesSha256\":\"%s\","
        "\"projectedEntitiesSha256\":\"%s\","
        "\"rawModelSequenceSha256\":\"%s\","
        "\"projectedModelSequenceSha256\":\"%s\","
        "\"rawMembershipSha256\":\"%s\","
        "\"projectedMembershipSha256\":\"%s\","
        "\"projectionRdflags\":%d,\"publicationCount\":%u,"
        "\"revalidationCount\":%u,\"payloadAllocationCount\":%u,"
        "\"replacementAllocation\":false,\"identityOverwrite\":false,"
        "\"rpbEnabled\":false,\"rpbAuthorityRead\":false,"
        "\"pointersInIdentity\":false,\"closed\":true}\n",
        event, authority->version, endpoint, authority->raw_count,
        authority->projected_count, authority->raw_canonical_bytes,
        authority->projected_canonical_bytes, raw, projected, raw_models,
        projected_models, raw_membership, projected_membership,
        authority->projection_rdflags, authority->publication_count,
        authority->revalidation_count, payload_allocation_count) < 0 ||
        fflush(b1.log))
        b1_fail("cannot persist B1 R6-R4 normal entity authority event");
}

static void b1_phase5brpbr6r4_close_normal_entity_authority(bool revalidation)
{
    b1_entity_alignment_authority_t *authority =
        &b1.entity_alignment_authority;
    b1_entity_alignment_calculation_t result = { 0 };
    entity_t *projected;
    byte expected_raw[32];
    byte *raw_copy = NULL, *projected_copy = NULL;
    uint32_t count;
    int32_t rdflags;

    if (!B1_Requested() || !B1_Ready() || !b1.log ||
        !b1.scene_controller.enabled || b1.rpb.enabled ||
        b1.rpb.mode != B1_PHASE5BRPB_MODE_OFF ||
        !b1.scene_controller.endpoint_closed ||
        !b1.scene_controller.scene_build_consumed ||
        !b1.scene_controller.client_scene_closed || !b1.frozen ||
        !b1.records || !b1.entities ||
        b1.frozen_refdef.entities != b1.entities ||
        b1.frozen_refdef.num_entities != B1_PHASE5BRPBR2_ENTITY_COUNT ||
        !b1_parse_sha256(B1_PHASE5BRPBR2_ENTITY_SHA256, expected_raw) ||
        b1.scene_component_bytes[1] != B1_PHASE5BRPBR2_ENTITY_BYTES ||
        memcmp(b1.scene_component_sha256[1], expected_raw, 32))
        b1_fail("B1 R6-R4 normal authority lacks the closed raw client scene");
    if ((!revalidation &&
         b1.scene_lifecycle.state != B1_SCENE_ENDPOINT_ACCEPTED) ||
        (revalidation && b1.scene_lifecycle.state !=
            B1_SCENE_TEMPORAL_INFRASTRUCTURE_READY))
        b1_fail("B1 R6-R4 normal authority entered from the wrong lifecycle state");
    if ((!revalidation &&
         !b1_phase5brpbr6r4_normal_authority_is_zero(authority)) ||
        (revalidation &&
         (!authority->closed || authority->version !=
              B1_PHASE5BRPBR6R4_ENTITY_ALIGNMENT_VERSION ||
          authority->publication_count != 1u ||
          authority->revalidation_count != 0u ||
          !authority->raw_entities_v1 || !authority->projected_entities_v1 ||
          authority->raw_count != B1_PHASE5BRPBR2_ENTITY_COUNT ||
          authority->projected_count != B1_PHASE5BRPBR2_ENTITY_COUNT ||
          authority->raw_canonical_bytes != B1_PHASE5BRPBR2_ENTITY_BYTES ||
          authority->projected_canonical_bytes !=
              B1_PHASE5BRPBR2_ENTITY_BYTES)))
        b1_fail("B1 R6-R4 normal authority state is partial, stale or duplicated");

    count = (uint32_t)b1.frozen_refdef.num_entities;
    rdflags = b1.records[0].rdflags;
    projected = Z_Mallocz(sizeof(*projected) * count);
    for (uint32_t index = 0; index < count; index++)
        b1_phase5brpbr6r4_project_normal_scene_entity(
            projected + index, b1.entities + index, rdflags);
    b1_phase5brpbr4_calculate_entity_alignment(
        b1.source_authority.endpoint_sha256, b1.entities, count,
        projected, count, &result);

    if (!revalidation) {
        raw_copy = b1_copy_required(result.raw.data, result.raw.bytes,
            "normal-B1 raw entity authority");
        projected_copy = b1_copy_required(
            result.projected.data, result.projected.bytes,
            "normal-B1 projected entity authority");
        authority->raw_entities_v1 = raw_copy;
        authority->projected_entities_v1 = projected_copy;
        authority->raw_count = result.raw_count;
        authority->projected_count = result.projected_count;
        authority->raw_canonical_bytes = (uint32_t)result.raw.bytes;
        authority->projected_canonical_bytes =
            (uint32_t)result.projected.bytes;
        authority->projection_rdflags = rdflags;
        memcpy(authority->endpoint_sha256, result.endpoint_sha256, 32);
        memcpy(authority->raw_entities_sha256,
               result.raw_entities_sha256, 32);
        memcpy(authority->projected_entities_sha256,
               result.projected_entities_sha256, 32);
        memcpy(authority->raw_model_sequence_sha256,
               result.raw_model_sequence_sha256, 32);
        memcpy(authority->projected_model_sequence_sha256,
               result.projected_model_sequence_sha256, 32);
        memcpy(authority->raw_membership_sha256,
               result.raw_membership_sha256, 32);
        memcpy(authority->projected_membership_sha256,
               result.projected_membership_sha256, 32);
        memcpy(authority->raw_record_sha256, result.raw_record_sha256,
               sizeof(result.raw_record_sha256));
        memcpy(authority->projected_record_sha256,
               result.projected_record_sha256,
               sizeof(result.projected_record_sha256));
        authority->version = B1_PHASE5BRPBR6R4_ENTITY_ALIGNMENT_VERSION;
        authority->publication_count = 1u;
        authority->closed = true;
        b1_phase5brpbr6r4_log_normal_authority(
            "b1_r6r4_normal_entity_authority_closed", authority, 2u);
    } else {
        if (authority->projection_rdflags != rdflags ||
            memcmp(authority->raw_entities_v1,
                   result.raw.data, result.raw.bytes) ||
            memcmp(authority->projected_entities_v1,
                   result.projected.data, result.projected.bytes) ||
            memcmp(authority->endpoint_sha256, result.endpoint_sha256, 32) ||
            memcmp(authority->raw_entities_sha256,
                   result.raw_entities_sha256, 32) ||
            memcmp(authority->projected_entities_sha256,
                   result.projected_entities_sha256, 32) ||
            memcmp(authority->raw_model_sequence_sha256,
                   result.raw_model_sequence_sha256, 32) ||
            memcmp(authority->projected_model_sequence_sha256,
                   result.projected_model_sequence_sha256, 32) ||
            memcmp(authority->raw_membership_sha256,
                   result.raw_membership_sha256, 32) ||
            memcmp(authority->projected_membership_sha256,
                   result.projected_membership_sha256, 32) ||
            memcmp(authority->raw_record_sha256,
                   result.raw_record_sha256,
                   sizeof(result.raw_record_sha256)) ||
            memcmp(authority->projected_record_sha256,
                   result.projected_record_sha256,
                   sizeof(result.projected_record_sha256)))
            b1_fail("B1 R6-R4 normal entity authority changed at revalidation");
        authority->revalidation_count = 1u;
        b1_phase5brpbr6r4_log_normal_authority(
            "b1_r6r4_normal_entity_authority_revalidated", authority, 0u);
    }
    b1_phase5brpbr4_release_entity_calculation(&result);
    Z_Free(projected);
}

static void b1_phase5brpb_canonicalise_boundary(
    b1_phase5brpb_boundary_t *boundary)
{
    b1_candidate_buffer_t persistent = { 0 };
    b1_candidate_buffer_t particle_v2 = { 0 };
    b1_candidate_buffer_t sprite_source = { 0 };
    b1_candidate_buffer_t endpoint_scene = { 0 };
    b1_candidate_buffer_t semantic = { 0 };

    b1_phase5brpb_token_identity(boundary);
    if (!boundary->scene_closed)
        return;
    b1_phase5brpbr4_close_entity_alignment(boundary);
    b1_phase5brpb_classify_sources(boundary);
    b1_phase5brpb_particle_semantics(&particle_v2, boundary);
    if (!particle_v2.bytes || particle_v2.bytes > UINT32_MAX ||
        !b1_sha256_data(particle_v2.data, particle_v2.bytes,
                        boundary->particle_v2_sha256)) {
        b1_candidate_buffer_release(&particle_v2);
        b1_fail("Phase-5B-RP-B cannot close particle-V2 source semantics");
    }
    boundary->particle_v2 = b1_phase5brpb_allocate(
        particle_v2.bytes, 1u, "particle-V2 source semantics");
    memcpy(boundary->particle_v2, particle_v2.data, particle_v2.bytes);
    boundary->particle_v2_bytes = (uint32_t)particle_v2.bytes;
    b1_candidate_buffer_release(&particle_v2);
    b1_phase5brpb_sprite_source_semantics(&sprite_source, boundary);
    if (!sprite_source.bytes || sprite_source.bytes > UINT32_MAX ||
        !b1_sha256_data(sprite_source.data, sprite_source.bytes,
                        boundary->sprite_source_sha256)) {
        b1_candidate_buffer_release(&sprite_source);
        b1_fail("Phase-5B-RP-B cannot close resolved-sprite source semantics");
    }
    boundary->sprite_source = b1_phase5brpb_allocate(
        sprite_source.bytes, 1u, "resolved-sprite source semantics");
    memcpy(boundary->sprite_source, sprite_source.data, sprite_source.bytes);
    boundary->sprite_source_bytes = (uint32_t)sprite_source.bytes;
    b1_candidate_buffer_release(&sprite_source);
    b1_phase5brpb_serialise_persistent(&persistent, boundary);
    b1_phase5brpb_register_seen(b1.rpb.persistent_seen,
        &b1.rpb.persistent_count, boundary->persistent_sha256,
        persistent.data, persistent.bytes, "persistent-scene");
    boundary->finite_canonical = true;
    boundary->zero_sources = boundary->scene_particle_count == 0u &&
        boundary->beam_entity_count == 0u &&
        boundary->active_beam_count == 0u &&
        boundary->resolved_sprite_count == 0u &&
        boundary->debug_primitive_count == 0u;

    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0) {
        b1_phase5brpb_old_v1_crosschecks(boundary);
        b1.rpb.accepted_p0 = b1_phase5brpb_allocate(
            persistent.bytes, 1u, "materialised-P0");
        memcpy(b1.rpb.accepted_p0, persistent.data, persistent.bytes);
        b1.rpb.accepted_p0_bytes = persistent.bytes;
        memcpy(b1.rpb.accepted_p0_sha256, boundary->persistent_sha256, 32);
        boundary->persistent_equal = true;
        b1_candidate_buffer_release(&persistent);
        return;
    }
    boundary->persistent_equal =
        persistent.bytes == b1.rpb.accepted_p0_bytes &&
        !memcmp(persistent.data, b1.rpb.accepted_p0, persistent.bytes);
    if (!boundary->zero_sources) {
        b1.rpb.transparency_ineligible_count++;
        b1_candidate_buffer_release(&persistent);
        return;
    }
    if (!boundary->persistent_equal) {
        b1.rpb.persistent_mismatch_count++;
        b1_candidate_buffer_release(&persistent);
        return;
    }
    b1_phase5brpb_serialise_endpoint_scene(&endpoint_scene, boundary);
    b1_candidate_string_bytes(&semantic, B1_PHASE5BRPB_SEMANTIC_SCHEMA,
                              strlen(B1_PHASE5BRPB_SEMANTIC_SCHEMA));
    b1_candidate_u32(&semantic, 1u);
    b1_candidate_append(&semantic, b1.rpb.startup_sha256, 32);
    b1_candidate_u32(&semantic, (uint32_t)persistent.bytes);
    b1_candidate_append(&semantic, persistent.data, persistent.bytes);
    b1_candidate_u32(&semantic, (uint32_t)endpoint_scene.bytes);
    b1_candidate_append(&semantic, endpoint_scene.data, endpoint_scene.bytes);
    b1_candidate_string_bytes(&semantic,
        "B1-SF-A1-PHASE5BRPB-SOURCE-PROOF-V1",
        sizeof("B1-SF-A1-PHASE5BRPB-SOURCE-PROOF-V1") - 1u);
    b1_candidate_u32(&semantic, 1u);
    b1_candidate_u32(&semantic, boundary->particle_v2_bytes);
    b1_candidate_append(&semantic, boundary->particle_v2,
                        boundary->particle_v2_bytes);
    b1_candidate_u32(&semantic, boundary->beam_entity_count);
    b1_candidate_u32(&semantic, boundary->active_beam_count);
    b1_candidate_u32(&semantic, boundary->sprite_source_bytes);
    b1_candidate_append(&semantic, boundary->sprite_source,
                        boundary->sprite_source_bytes);
    b1_candidate_u32(&semantic, boundary->resolved_sprite_count);
    b1_candidate_u32(&semantic, boundary->debug_primitive_count);
    b1_candidate_string_bytes(&semantic,
        "B1-SF-A1-PHASE5BRPB-SELECTOR-PROOF-V1",
        sizeof("B1-SF-A1-PHASE5BRPB-SELECTOR-PROOF-V1") - 1u);
    b1_candidate_u32(&semantic, 1u);
    /* These eight fixed behaviour values are admitted to E; occurrence
     * position, message identity and timing remain telemetry-only. */
    b1_candidate_u8(&semantic, 1u); /* completeMessageClosure */
    b1_candidate_u8(&semantic, 1u); /* ordinaryClientSceneClosure */
    b1_candidate_u8(&semantic, 1u); /* base1Authority */
    b1_candidate_u8(&semantic, 1u); /* startupConfigurationAuthority */
    b1_candidate_u8(&semantic, 1u); /* persistentSceneEquality */
    b1_candidate_u8(&semantic, 1u); /* zeroTransparencySources */
    b1_candidate_u8(&semantic, 1u); /* finiteCanonicalState */
    b1_candidate_u8(&semantic, 1u); /* freezeClosable */
    if (semantic.bytes > UINT32_MAX ||
        !b1_sha256_data(semantic.data, semantic.bytes,
                        boundary->semantic_sha256))
        b1_fail("Phase-5B-RP-B semantic candidate cannot be hashed");
    boundary->semantic_bytes = (uint32_t)semantic.bytes;
    boundary->qualifying = true;
    b1.rpb.qualifying_occurrence_count++;
    b1_phase5brpb_register_seen(b1.rpb.semantic_seen,
        &b1.rpb.semantic_count, boundary->semantic_sha256,
        semantic.data, semantic.bytes, "semantic-candidate");
    b1_candidate_buffer_release(&semantic);
    b1_candidate_buffer_release(&endpoint_scene);
    b1_candidate_buffer_release(&persistent);
}

static const char *const b1_scene_component_name[B1_PHASE5B_SCENE_COMPONENT_COUNT] = {
    "refdef-view", "entities", "admitted-particle-scope", "dynamic-lights", "light-styles",
    "decals", "area-pvs", "role-primary-left", "role-primary-right"
};

static void b1_scene_vec_f32(b1_candidate_buffer_t *buffer,
                             const float *values, size_t count)
{
    for (size_t index = 0; index < count; index++)
        b1_candidate_f32(buffer, values[index]);
}

static void b1_scene_component_close(const b1_candidate_buffer_t *buffer,
                                     size_t start, uint32_t index,
                                     byte component_digest[B1_PHASE5B_SCENE_COMPONENT_COUNT][32],
                                     uint32_t component_bytes[B1_PHASE5B_SCENE_COMPONENT_COUNT])
{
    size_t bytes;

    if (index >= B1_PHASE5B_SCENE_COMPONENT_COUNT || start > buffer->bytes)
        b1_fail("B1 client-scene component boundary is malformed");
    bytes = buffer->bytes - start;
    if (bytes > UINT32_MAX)
        b1_fail("B1 client-scene component exceeds its canonical bound");
    if (component_bytes)
        component_bytes[index] = (uint32_t)bytes;
    if (component_digest &&
        !b1_sha256_data(buffer->data + start, bytes, component_digest[index]))
        b1_fail("cannot hash B1 client-scene component");
}

static void b1_scene_plan_view(b1_candidate_buffer_t *buffer,
                               const b1_plan_view_t *view)
{
    b1_candidate_u32(buffer, view->acceptance);
    for (int index = 0; index < 4; index++)
        b1_candidate_i32(buffer, view->viewport[index]);
    b1_scene_vec_f32(buffer, view->view_origin, 3);
    b1_scene_vec_f32(buffer, view->view_angles, 3);
    b1_scene_vec_f32(buffer, view->fov_xy, 2);
    b1_candidate_u32(buffer, view->render_extent[0]);
    b1_candidate_u32(buffer, view->render_extent[1]);
    b1_candidate_u32(buffer, view->taa_extent[0]);
    b1_candidate_u32(buffer, view->taa_extent[1]);
    b1_scene_vec_f32(buffer, view->projection_fov_scale, 2);
    b1_candidate_f32(buffer, view->cylindrical_hfov);
    b1_scene_vec_f32(buffer, view->V, 16);
    b1_scene_vec_f32(buffer, view->invV, 16);
    b1_scene_vec_f32(buffer, view->P, 16);
    b1_scene_vec_f32(buffer, view->invP, 16);
}

static void b1_scene_role_context(b1_candidate_buffer_t *buffer, b1_role_t role,
                                  const b1_role_context_t *context,
                                  const b1_plan_record_t *record)
{
    if (role < B1_ROLE_PRIMARY_LEFT || role >= B1_ROLE_COUNT ||
        !context || !record || record->logical_index >= B1_RECORD_COUNT ||
        record->views[role].acceptance != B1_ACCEPTANCE_REQUIRED)
        b1_fail("B1 canonical role execution context is malformed");
    b1_candidate_u32(buffer, (uint32_t)role);
    b1_candidate_u32(buffer, context->committed_logical);
    b1_candidate_u8(buffer, context->temporal_valid ? 1u : 0u);
    b1_candidate_u8(buffer, context->pending_valid ? 1u : 0u);
    b1_scene_plan_view(buffer, &context->committed);
    if (context->pending_valid)
        b1_scene_plan_view(buffer, &context->pending);
    b1_candidate_u32(buffer, record->logical_index);
    b1_candidate_u32(buffer, record->source_frame_id);
    b1_candidate_i64(buffer, record->relative_time_ns);
    b1_candidate_i64(buffer, record->frame_duration_ns);
    b1_candidate_f32(buffer, record->scene_time);
    b1_candidate_i32(buffer, record->rdflags);
    for (int index = 0; index < 4; index++)
        b1_candidate_i32(buffer, record->gameplay_viewport[index]);
    b1_scene_plan_view(buffer, &record->views[role]);
}

static void b1_phase5brpb_old_v1_close(
    b1_candidate_buffer_t *buffer, uint32_t index, uint32_t expected_bytes,
    const char *expected_sha256)
{
    byte expected[32];
    bool valid = index < B1_PHASE5BRPB_OLD_COMPONENT_COUNT &&
        b1_parse_sha256(expected_sha256, expected) &&
        b1_sha256_data(buffer->data, buffer->bytes,
                       b1.rpb.old_component_sha256[index]);
    if (!valid || buffer->bytes != expected_bytes ||
        memcmp(b1.rpb.old_component_sha256[index], expected, 32)) {
        if (valid && index == 0u && b1.rpb.r2_entity_diagnostic)
            b1_phase5brpbr2_entity_diagnostic(
                buffer, b1.rpb.old_component_sha256[index]);
        b1_fail("Phase-5B-RP-B preserved V1 component %u differs", index);
    }
    b1.rpb.old_component_bytes[index] = (uint32_t)buffer->bytes;
    b1_candidate_buffer_release(buffer);
}

static void b1_phase5brpb_old_v1_crosschecks(
    const b1_phase5brpb_boundary_t *boundary)
{
    const refdef_t *fd = &boundary->scene_refdef;
    const entity_t *raw_entities = b1.rpb.raw_scene_entities +
        boundary->raw_scene_entity_offset;
    b1_candidate_buffer_t buffer = { 0 };
    b1_role_context_t context = { 0 };
    uint32_t decal_count;

    b1_candidate_u32(&buffer, boundary->scene_entity_count);
    for (uint32_t index = 0; index < boundary->scene_entity_count; index++) {
        b1_phase5brpbr4_serialise_entity(&buffer, raw_entities + index);
    }
    b1_phase5brpb_old_v1_close(&buffer, 0u, 2356u,
        "197D34D638615E5850504FFF7AB555ED85F42F98E4532A020CD47D9776A6B315");

    b1_candidate_u32(&buffer, boundary->scene_dlight_count);
    for (uint32_t index = 0; index < boundary->scene_dlight_count; index++) {
        const dlight_t *light = fd->dlights + index;
        b1_scene_vec_f32(&buffer, light->origin, 3);
        b1_scene_vec_f32(&buffer, light->color, 3);
        b1_candidate_f32(&buffer, light->intensity);
        b1_candidate_f32(&buffer, light->radius);
        b1_candidate_i32(&buffer, light->light_type);
        if (light->light_type == DLIGHT_SPOT) {
            b1_candidate_i32(&buffer, light->spot.emission_profile);
            b1_scene_vec_f32(&buffer, light->spot.direction, 3);
            if (light->spot.emission_profile ==
                DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF) {
                b1_candidate_f32(&buffer, light->spot.cos_total_width);
                b1_candidate_f32(&buffer, light->spot.cos_falloff_start);
            } else {
                b1_candidate_f32(&buffer, light->spot.total_width);
                b1_candidate_i32(&buffer, light->spot.texture);
            }
        }
    }
    b1_phase5brpb_old_v1_close(&buffer, 1u, 4u,
        "DF3F619804A92FDB4057192DC43DD748EA778ADC52BC498CE80524C014B81119");

    b1_candidate_u32(&buffer, MAX_LIGHTSTYLES);
    for (uint32_t index = 0; index < MAX_LIGHTSTYLES; index++)
        b1_candidate_f32(&buffer, fd->lightstyles[index].white);
    b1_phase5brpb_old_v1_close(&buffer, 2u, 1028u,
        "D373C5AE86DCC6E25E6BF6CB36DC5516D610A0816D73CDE1B950D5371088D62C");

    decal_count = (uint32_t)((fd->decal_end - fd->decal_beg + MAX_DECALS) %
                             MAX_DECALS);
    b1_candidate_i32(&buffer, fd->decal_beg);
    b1_candidate_i32(&buffer, fd->decal_end);
    b1_candidate_u32(&buffer, decal_count);
    for (uint32_t offset = 0; offset < decal_count; offset++) {
        const decal_t *decal = fd->decal +
            (((uint32_t)fd->decal_beg + offset) % MAX_DECALS);
        b1_scene_vec_f32(&buffer, decal->pos, 3);
        b1_scene_vec_f32(&buffer, decal->dir, 3);
        b1_candidate_f32(&buffer, decal->spread);
        b1_candidate_f32(&buffer, decal->length);
        b1_candidate_f32(&buffer, decal->dummy);
    }
    b1_phase5brpb_old_v1_close(&buffer, 3u, 12u,
        "15EC7BF0B50732B49F8228E07D24365338F9E3AB994B00AF08E5A3BFFE55FD8B");

    b1_candidate_u32(&buffer, boundary->scene_area_bytes);
    if (boundary->scene_area_bytes)
        b1_candidate_append(&buffer, fd->areabits, boundary->scene_area_bytes);
    b1_phase5brpb_old_v1_close(&buffer, 4u, 5u,
        "06DABC1C16AA6BAA394CD5D356B6EAC101811B0BF78CE32A1EE893CAD4B0A83F");

    for (uint32_t role = 0; role < B1_ROLE_COUNT; role++) {
        context.committed = b1.rpb.plan_record0.views[role];
        context.committed_logical = UINT32_MAX;
        context.temporal_valid = false;
        context.pending_valid = false;
        b1_scene_role_context(&buffer, (b1_role_t)role, &context,
                              &b1.rpb.plan_record0);
        b1_phase5brpb_old_v1_close(&buffer, 5u + role, 730u,
            role == B1_ROLE_PRIMARY_LEFT ?
            "9FBA3FD8562E20FEDA8459C3F47FE91E673CD242B75D57F076E50AC247558DFC" :
            "6B0E2671EF49EB632CF32055AC794EB31C8EE5C91FF3A3E5E2274FF24CA64F57");
    }
}

static void b1_phase5brpb_path(char path[MAX_OSPATH], const char *relative)
{
    if (!relative || !relative[0] || strstr(relative, "..") ||
        strchr(relative, ':') || relative[0] == '/' || relative[0] == '\\' ||
        Q_snprintf(path, MAX_OSPATH, "%s/%s",
                   b1_phase5brpb_output->string, relative) >= MAX_OSPATH)
        b1_fail("Phase-5B-RP-B output relative path is malformed");
}

static void b1_phase5brpb_create_directory(const char *relative)
{
    char path[MAX_OSPATH];
    DWORD attributes;
    b1_phase5brpb_path(path, relative);
    if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES ||
        !CreateDirectoryA(path, NULL))
        b1_fail("Phase-5B-RP-B output directory collision at %s", relative);
    attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
        b1_fail("Phase-5B-RP-B output directory is not regular");
}

static void b1_phase5brpb_publish_partial(const char *relative,
                                          const byte expected[32],
                                          uint64_t expected_bytes)
{
    char final_path[MAX_OSPATH];
    char partial_path[MAX_OSPATH];
    byte first[32], second[32], final_digest[32];
    uint64_t first_bytes = 0, second_bytes = 0, final_bytes = 0;
    DWORD attributes;

    b1_phase5brpb_path(final_path, relative);
    if (Q_snprintf(partial_path, sizeof(partial_path), "%s.partial",
                   final_path) >= sizeof(partial_path) ||
        !b1_sha256_file(partial_path, &first_bytes, first) ||
        !b1_sha256_file(partial_path, &second_bytes, second) ||
        first_bytes != expected_bytes || second_bytes != expected_bytes ||
        memcmp(first, expected, 32) || memcmp(second, expected, 32) ||
        GetFileAttributesA(final_path) != INVALID_FILE_ATTRIBUTES ||
        !MoveFileExA(partial_path, final_path, MOVEFILE_WRITE_THROUGH))
        b1_fail("Phase-5B-RP-B output cannot publish %s atomically", relative);
    attributes = GetFileAttributesA(final_path);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
        GetFileAttributesA(partial_path) != INVALID_FILE_ATTRIBUTES ||
        !b1_sha256_file(final_path, &final_bytes, final_digest) ||
        final_bytes != expected_bytes || memcmp(final_digest, expected, 32))
        b1_fail("Phase-5B-RP-B published output differs for %s", relative);
}

static void b1_phase5brpb_write_blob(const char *relative,
                                     const byte *data, size_t bytes,
                                     const byte digest[32])
{
    char final_path[MAX_OSPATH];
    char partial_path[MAX_OSPATH];
    FILE *file;
    if (!data || !bytes)
        b1_fail("Phase-5B-RP-B cannot publish an empty canonical blob");
    b1_phase5brpb_path(final_path, relative);
    if (Q_snprintf(partial_path, sizeof(partial_path), "%s.partial",
                   final_path) >= sizeof(partial_path) ||
        GetFileAttributesA(final_path) != INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesA(partial_path) != INVALID_FILE_ATTRIBUTES)
        b1_fail("Phase-5B-RP-B blob path collides for %s", relative);
    file = Q_fopen(partial_path, "wxb");
    if (!file || fwrite(data, 1, bytes, file) != bytes || fflush(file) ||
        fclose(file))
        b1_fail("Phase-5B-RP-B cannot persist blob %s", relative);
    b1_phase5brpb_publish_partial(relative, digest, (uint64_t)bytes);
}

static FILE *b1_phase5brpb_open_json(const char *relative,
                                     char partial_path[MAX_OSPATH])
{
    char final_path[MAX_OSPATH];
    FILE *file;
    b1_phase5brpb_path(final_path, relative);
    if (Q_snprintf(partial_path, MAX_OSPATH, "%s.partial", final_path) >=
        MAX_OSPATH ||
        GetFileAttributesA(final_path) != INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesA(partial_path) != INVALID_FILE_ATTRIBUTES)
        b1_fail("Phase-5B-RP-B JSONL path collides");
    file = Q_fopen(partial_path, "wxb");
    if (!file)
        b1_fail("Phase-5B-RP-B cannot create JSONL partial");
    return file;
}

static void b1_phase5brpb_close_json(FILE *file, const char *relative,
                                     const char *partial_path)
{
    byte digest[32];
    uint64_t bytes = 0;
    if (!file || fflush(file) || fclose(file) ||
        !b1_sha256_file(partial_path, &bytes, digest) || !bytes)
        b1_fail("Phase-5B-RP-B cannot close/authenticate JSONL partial");
    b1_phase5brpb_publish_partial(relative, digest, bytes);
}

typedef enum b1_phase5brpbr2_field_kind_e {
    B1_PHASE5BRPBR2_I32,
    B1_PHASE5BRPBR2_U32,
    B1_PHASE5BRPBR2_F32
} b1_phase5brpbr2_field_kind_t;

typedef struct b1_phase5brpbr2_field_s {
    const char *name;
    const char *classification;
    uint32_t offset;
    b1_phase5brpbr2_field_kind_t kind;
} b1_phase5brpbr2_field_t;

static const b1_phase5brpbr2_field_t b1_phase5brpbr2_entity_fields[] = {
    { "model", "renderer-semantic", 0u, B1_PHASE5BRPBR2_I32 },
    { "angles[0]", "interpolation/animation", 4u, B1_PHASE5BRPBR2_F32 },
    { "angles[1]", "interpolation/animation", 8u, B1_PHASE5BRPBR2_F32 },
    { "angles[2]", "interpolation/animation", 12u, B1_PHASE5BRPBR2_F32 },
    { "origin[0]", "interpolation/animation", 16u, B1_PHASE5BRPBR2_F32 },
    { "origin[1]", "interpolation/animation", 20u, B1_PHASE5BRPBR2_F32 },
    { "origin[2]", "interpolation/animation", 24u, B1_PHASE5BRPBR2_F32 },
    { "frame", "interpolation/animation", 28u, B1_PHASE5BRPBR2_I32 },
    { "oldorigin[0]", "interpolation/animation", 32u, B1_PHASE5BRPBR2_F32 },
    { "oldorigin[1]", "interpolation/animation", 36u, B1_PHASE5BRPBR2_F32 },
    { "oldorigin[2]", "interpolation/animation", 40u, B1_PHASE5BRPBR2_F32 },
    { "oldframe", "interpolation/animation", 44u, B1_PHASE5BRPBR2_I32 },
    { "backlerp", "interpolation/animation", 48u, B1_PHASE5BRPBR2_F32 },
    { "skinnum", "renderer-semantic", 52u, B1_PHASE5BRPBR2_I32 },
    { "alpha", "renderer-semantic", 56u, B1_PHASE5BRPBR2_F32 },
    { "rgba", "renderer-semantic", 60u, B1_PHASE5BRPBR2_U32 },
    { "skin", "renderer-semantic", 64u, B1_PHASE5BRPBR2_I32 },
    { "flags", "renderer-semantic", 68u, B1_PHASE5BRPBR2_I32 },
    { "id", "renderer-semantic", 72u, B1_PHASE5BRPBR2_I32 },
    { "tent_type", "source-proven dormant", 76u, B1_PHASE5BRPBR2_I32 },
    { "scale", "renderer-semantic", 80u, B1_PHASE5BRPBR2_F32 },
};

static void b1_phase5brpbr2_print_digest(FILE *file, const byte digest[32])
{
    for (uint32_t index = 0; index < 32u; index++)
        fprintf(file, "%02X", digest[index]);
}

static uint32_t b1_phase5brpbr2_word(const byte *data)
{
    uint32_t value;
    memcpy(&value, data, sizeof(value));
    return LittleLong(value);
}

static void b1_phase5brpbr2_entity_diagnostic(
    const b1_candidate_buffer_t *observed, const byte observed_sha256[32])
{
    byte expected[B1_PHASE5BRPBR2_ENTITY_BYTES];
    byte expected_digest[32], compiled_digest[32];
    uint64_t expected_bytes = 0;
    uint32_t expected_count = 0, observed_count = 0;
    size_t first = SIZE_MAX, range_end = 0, common;
    bool shape_valid, first_field = true;
    char partial[MAX_OSPATH];
    FILE *input, *log;

    if (!b1.rpb.r2_entity_diagnostic || !observed || !observed->data ||
        !observed->bytes || !b1_phase5brpbr2_expected_entities ||
        !b1_phase5brpbr2_expected_entities->string[0] ||
        b1_phase5brpbr2_expected_entities->modified ||
        !b1_parse_sha256(B1_PHASE5BRPBR2_ENTITY_SHA256, compiled_digest) ||
        !b1_sha256_file(b1_phase5brpbr2_expected_entities->string,
                        &expected_bytes, expected_digest) ||
        expected_bytes != sizeof(expected) ||
        memcmp(expected_digest, compiled_digest, 32) ||
        memcmp(expected_digest, b1.rpb.r2_expected_entities_sha256, 32))
        b1_fail("Phase-5B-RP-B-R2 accepted entity reference is malformed");
    input = fopen(b1_phase5brpbr2_expected_entities->string, "rb");
    if (!input || fread(expected, 1, sizeof(expected), input) != sizeof(expected) ||
        fgetc(input) != EOF) {
        if (input) fclose(input);
        b1_fail("Phase-5B-RP-B-R2 accepted entity reference cannot be read exactly");
    }
    fclose(input);

    b1_phase5brpb_write_blob("r2-observed-entities-v1.bin", observed->data,
                             observed->bytes, observed_sha256);
    common = min(sizeof(expected), observed->bytes);
    for (size_t index = 0; index < common; index++) {
        if (expected[index] != observed->data[index]) {
            first = index;
            range_end = index;
            while (range_end < common &&
                   expected[range_end] != observed->data[range_end])
                range_end++;
            break;
        }
    }
    if (first == SIZE_MAX && sizeof(expected) != observed->bytes) {
        first = common;
        range_end = max(sizeof(expected), observed->bytes);
    }
    expected_count = b1_phase5brpbr2_word(expected);
    if (observed->bytes >= sizeof(uint32_t))
        observed_count = b1_phase5brpbr2_word(observed->data);
    shape_valid = expected_count == B1_PHASE5BRPBR2_ENTITY_COUNT &&
        sizeof(expected) == sizeof(uint32_t) +
            expected_count * sizeof(entity_t) &&
        observed_count <= B1_PHASE5A_MAX_CANDIDATES &&
        observed->bytes == sizeof(uint32_t) +
            (size_t)observed_count * sizeof(entity_t);

    log = b1_phase5brpb_open_json("r2-entity-diagnostic.json", partial);
    fprintf(log,
        "{\"schema\":\"q2rtx.reference_b.b1_phase5brpbr2_entity_diagnostic\","
        "\"schemaVersion\":1,\"diagnosticOnly\":true,"
        "\"expectedReconstructedFromThreePreservedAdapters\":true,"
        "\"expectedRelativePath\":\"../expected-entities-v1.bin\","
        "\"expectedBytes\":%u,\"expectedSha256\":\"",
        (unsigned)sizeof(expected));
    b1_phase5brpbr2_print_digest(log, expected_digest);
    fprintf(log, "\",\"observedRelativePath\":\"r2-observed-entities-v1.bin\","
        "\"observedBytes\":%zu,\"observedSha256\":\"", observed->bytes);
    b1_phase5brpbr2_print_digest(log, observed_sha256);
    fprintf(log, "\",\"expectedEntityCount\":%u,\"observedEntityCount\":%u,",
            expected_count, observed_count);
    if (first == SIZE_MAX)
        fprintf(log, "\"firstDifferingByteRange\":null,");
    else
        fprintf(log, "\"firstDifferingByteRange\":{\"offset\":%zu,\"bytes\":%zu},",
                first, range_end - first);
    fprintf(log, "\"fieldDifferences\":[");
    if (!shape_valid) {
        fprintf(log, "{\"entityOrdinal\":null,\"field\":\"stream-shape\","
                     "\"classification\":\"unresolved\"}");
        first_field = false;
    } else {
        uint32_t entity_limit = min(expected_count, observed_count);
        for (uint32_t ordinal = 0; ordinal < entity_limit; ordinal++) {
            const byte *expected_entity = expected + sizeof(uint32_t) +
                (size_t)ordinal * sizeof(entity_t);
            const byte *observed_entity = observed->data + sizeof(uint32_t) +
                (size_t)ordinal * sizeof(entity_t);
            for (uint32_t field_index = 0;
                 field_index < q_countof(b1_phase5brpbr2_entity_fields);
                 field_index++) {
                const b1_phase5brpbr2_field_t *field =
                    b1_phase5brpbr2_entity_fields + field_index;
                uint32_t expected_word = b1_phase5brpbr2_word(
                    expected_entity + field->offset);
                uint32_t observed_word = b1_phase5brpbr2_word(
                    observed_entity + field->offset);
                int32_t expected_i32, observed_i32;
                float expected_f32, observed_f32;
                if (expected_word == observed_word)
                    continue;
                if (field->kind == B1_PHASE5BRPBR2_F32) {
                    memcpy(&expected_f32, &expected_word, sizeof(expected_f32));
                    memcpy(&observed_f32, &observed_word, sizeof(observed_f32));
                    if (!isfinite(expected_f32) || !isfinite(observed_f32))
                        b1_fail("Phase-5B-RP-B-R2 entity diagnostic contains non-finite state");
                }
                if (!first_field) fputc(',', log);
                fprintf(log, "{\"entityOrdinal\":%u,\"field\":\"%s\","
                             "\"classification\":\"%s\",",
                        ordinal, field->name, field->classification);
                if (field->kind == B1_PHASE5BRPBR2_F32) {
                    fprintf(log, "\"expectedBits\":\"%08X\","
                                 "\"observedBits\":\"%08X\"}",
                            expected_word, observed_word);
                } else if (field->kind == B1_PHASE5BRPBR2_U32) {
                    fprintf(log, "\"expectedU32\":%u,\"observedU32\":%u}",
                            expected_word, observed_word);
                } else {
                    memcpy(&expected_i32, &expected_word, sizeof(expected_i32));
                    memcpy(&observed_i32, &observed_word, sizeof(observed_i32));
                    fprintf(log, "\"expectedI32\":%d,\"observedI32\":%d}",
                            expected_i32, observed_i32);
                }
                first_field = false;
            }
        }
        if (expected_count != observed_count) {
            if (!first_field) fputc(',', log);
            fprintf(log, "{\"entityOrdinal\":null,\"field\":\"entity-count\","
                         "\"classification\":\"unresolved\","
                         "\"expectedU32\":%u,\"observedU32\":%u}",
                    expected_count, observed_count);
            first_field = false;
        }
    }
    fprintf(log, "],\"selectedMessageSha256\":\"%s\","
        "\"endpointSha256\":\"5E6434A325DF5776ED088330B52A3C6AECD2A730CE90D3B07C01E94726B8573F\","
        "\"fixtureSha256\":\"%s\",\"runtimeManifestSha256\":\"%s\","
        "\"runtimeInventorySha256\":\"%s\",\"runtimeDataSha256\":\"%s\","
        "\"executableSha256\":\"%s\",\"sourceRevision\":\"%s\","
        "\"producerAuthoritySha256\":\"%s\",\"processId\":%lu,"
        "\"candidateAcceptance\":false,\"sceneSourceReady\":false,"
        "\"temporalActivationCount\":0,\"pairZeroCount\":0,"
        "\"captureCount\":0,\"pixelCount\":0,\"terminalMismatch\":true}\n",
        B1_PHASE5AR5_SELECTED_MESSAGE_SHA256, B1_PHASE5B_FIXTURE_SHA256,
        b1_phase5brpb_runtime_manifest_sha256->string,
        b1_phase5brpb_runtime_inventory_sha256->string,
        b1_phase5brpb_runtime_data_sha256->string,
        b1_phase5brpb_executable_sha256->string,
        b1_phase5brpbr2_source_revision->string,
        b1_phase5brpbr2_producer_authority_sha256->string,
        (unsigned long)GetCurrentProcessId());
    b1_phase5brpb_close_json(log, "r2-entity-diagnostic.json", partial);
}

static void b1_phase5brpb_range_set_sha256(
    const b1_phase5brpb_boundary_t *boundary, byte digest[32])
{
    static const char schema[] = "B1-SF-A1-PHASE5BRPB-RANGE-SET-V1";
    b1_candidate_buffer_t canonical = { 0 };
    b1_candidate_string_bytes(&canonical, schema, sizeof(schema) - 1u);
    b1_candidate_u32(&canonical, B1_PHASE5BRPB_RANGE_COUNT);
    for (uint32_t index = 0; index < B1_PHASE5BRPB_RANGE_COUNT; index++) {
        const b1_phase5brpb_range_t *range = boundary->ranges + index;
        b1_candidate_u32(&canonical, index);
        b1_candidate_string_bytes(&canonical, range->name, strlen(range->name));
        b1_candidate_u32(&canonical, range->offset);
        b1_candidate_u32(&canonical, range->bytes);
        b1_candidate_append(&canonical, range->sha256, 32);
    }
    if (!b1_sha256_data(canonical.data, canonical.bytes, digest)) {
        b1_candidate_buffer_release(&canonical);
        b1_fail("Phase-5B-RP-B cannot hash P range-set identity");
    }
    b1_candidate_buffer_release(&canonical);
}

static void b1_phase5brpb_sorted_semantic_indices(uint32_t indices[51])
{
    for (uint32_t index = 0; index < b1.rpb.semantic_count; index++) {
        uint32_t insertion = index;
        indices[index] = index;
        while (insertion && memcmp(
            b1.rpb.semantic_seen[indices[insertion - 1u]].digest,
            b1.rpb.semantic_seen[indices[insertion]].digest, 32) > 0) {
            uint32_t swap = indices[insertion - 1u];
            indices[insertion - 1u] = indices[insertion];
            indices[insertion] = swap;
            insertion--;
        }
    }
}

static void b1_phase5brpb_candidate_set_sha256(byte digest[32])
{
    static const char schema[] = "B1-SF-A1-PHASE5BRPB-CANDIDATE-SET-V1";
    b1_candidate_buffer_t canonical = { 0 };
    uint32_t indices[51] = { 0 };
    b1_phase5brpb_sorted_semantic_indices(indices);
    b1_candidate_string_bytes(&canonical, schema, sizeof(schema) - 1u);
    b1_candidate_u32(&canonical, b1.rpb.semantic_count);
    for (uint32_t ordinal = 0; ordinal < b1.rpb.semantic_count; ordinal++) {
        const b1_candidate_seen_t *entry =
            b1.rpb.semantic_seen + indices[ordinal];
        b1_candidate_append(&canonical, entry->digest, 32);
        b1_candidate_u32(&canonical, (uint32_t)entry->canonical_bytes);
        b1_candidate_append(&canonical, entry->canonical,
                            entry->canonical_bytes);
    }
    if (!b1_sha256_data(canonical.data, canonical.bytes, digest)) {
        b1_candidate_buffer_release(&canonical);
        b1_fail("Phase-5B-RP-B cannot hash candidate-set identity");
    }
    b1_candidate_buffer_release(&canonical);
}

static const char *b1_phase5brpb_message_classification(
    const b1_phase5brpb_boundary_t *boundary)
{
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0 && boundary->selected_anchor)
        return "P0_SELECTED_ANCHOR";
    switch (boundary->message_class) {
    case B1_PHASE5BRPB_MESSAGE_STARTUP:
    case B1_PHASE5BRPB_MESSAGE_NO_VALID_FRAME:
        return "INELIGIBLE_NO_VALID_FRAME";
    case B1_PHASE5BRPB_MESSAGE_NO_NEW_FRAME:
        return "INELIGIBLE_NO_NEW_FRAME";
    case B1_PHASE5BRPB_MESSAGE_NO_PREDECESSOR:
        return "INELIGIBLE_NO_DELTA_PREDECESSOR";
    case B1_PHASE5BRPB_MESSAGE_ELIGIBLE:
        return b1.rpb.mode == B1_PHASE5BRPB_MODE_P0 ?
            "P0_NONSELECTED_COMPLETE_MESSAGE" : "ELIGIBLE_CANDIDATE";
    default:
        b1_fail("Phase-5B-RP-B message classification is outside its enum");
        return NULL;
    }
}

static const char *b1_phase5brpb_scene_classification(
    const b1_phase5brpb_boundary_t *boundary)
{
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0)
        return "P0_SELECTED_SCENE_CLOSED";
    if (!boundary->zero_sources)
        return "INELIGIBLE_ACTIVE_TRANSPARENCY";
    if (!boundary->persistent_equal)
        return "PERSISTENT_SCENE_MISMATCH";
    if (boundary->qualifying)
        return "QUALIFYING_SEMANTIC_OCCURRENCE";
    b1_fail("Phase-5B-RP-B scene classification is incomplete");
    return NULL;
}

static int b1_phase5brpb_emit_message(FILE *log,
                                      const b1_phase5brpb_boundary_t *boundary)
{
    char message[65], token[65];
    b1_format_sha256(boundary->message_sha256, message);
    if (boundary->token_id)
        b1_format_sha256(boundary->token_sha256, token);
    return fprintf(log,
        "{\"event\":\"complete_message\",\"schemaVersion\":1,"
        "\"ordinal\":%llu,\"messageBytes\":%u,\"messageSha256\":\"%s\","
        "\"readCount\":%u,\"cursorSize\":%u,\"frameValid\":%s,"
        "\"newFrame\":%s,\"frameNumber\":%d,\"deltaFrame\":%d,"
        "\"predecessorPresent\":%s,\"predecessorValid\":%s,"
        "\"recoveredFlags\":%u,\"classification\":\"%s\","
        "\"tokenSha256\":%s%s%s}\n",
        (unsigned long long)boundary->ordinal, boundary->message_bytes, message,
        boundary->message_bytes, boundary->message_bytes,
        boundary->frame_valid ? "true" : "false",
        boundary->new_frame ? "true" : "false", boundary->frame.number,
        boundary->frame.delta,
        boundary->predecessor_present ? "true" : "false",
        boundary->predecessor_valid ? "true" : "false",
        boundary->recovered_flags,
        b1_phase5brpb_message_classification(boundary),
        boundary->token_id ? "\"" : "null",
        boundary->token_id ? token : "", boundary->token_id ? "\"" : "");
}

static int b1_phase5brpb_emit_scene(FILE *log,
                                    const b1_phase5brpb_boundary_t *boundary)
{
    char token[65], message[65], persistent[65], semantic[65], startup[65];
    char particle_v2[65], sprite_source[65];
    char persistent_path[96];
    char semantic_path[96];
    char particle_path[112];
    char sprite_path[112];
    char semantic_bytes_text[16];
    uint32_t refdef_time_bits;
    bool has_semantic = boundary->qualifying;
    b1_format_sha256(boundary->token_sha256, token);
    b1_format_sha256(boundary->message_sha256, message);
    b1_format_sha256(boundary->persistent_sha256, persistent);
    b1_format_sha256(boundary->semantic_sha256, semantic);
    b1_format_sha256(boundary->particle_v2_sha256, particle_v2);
    b1_format_sha256(boundary->sprite_source_sha256, sprite_source);
    b1_format_sha256(b1.rpb.startup_sha256, startup);
    memcpy(&refdef_time_bits, &boundary->scene_refdef.time,
           sizeof(refdef_time_bits));
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0)
        Q_strlcpy(persistent_path, "accepted-persistent-scene-v1.bin",
                  sizeof(persistent_path));
    else
        Q_snprintf(persistent_path, sizeof(persistent_path),
                   "persistent/%s.bin", persistent);
    Q_snprintf(semantic_path, sizeof(semantic_path),
               "candidates/%s.bin", semantic);
    Q_snprintf(particle_path, sizeof(particle_path),
               "particle-v2/%s.bin", particle_v2);
    Q_snprintf(sprite_path, sizeof(sprite_path),
               "resolved-sprites/%s.bin", sprite_source);
    if (has_semantic)
        Q_snprintf(semantic_bytes_text, sizeof(semantic_bytes_text), "%u",
                   boundary->semantic_bytes);
    return fprintf(log,
        "{\"event\":\"scene_boundary\",\"schemaVersion\":1,"
        "\"tokenSha256\":\"%s\",\"classification\":\"%s\","
        "\"eligible\":true,\"completeMessage\":true,\"parseSucceeded\":true,"
        "\"messageClosed\":true,\"sceneBuilt\":true,\"freezeClosable\":true,"
        "\"partialPending\":false,\"mapIdentity\":\"base1\","
        "\"startupSha256\":\"%s\",\"persistentRelativePath\":\"%s\","
        "\"persistentBytes\":%u,\"persistentSha256\":\"%s\","
        "\"semanticRelativePath\":%s%s%s,\"semanticBytes\":%s,"
        "\"semanticSha256\":%s%s%s,\"sourceCounts\":{"
        "\"particles\":%u,\"beamEntities\":%u,\"activeBeams\":%u,"
        "\"resolvedSprites\":%u,\"debugPrimitives\":%u},"
        "\"particleV2RelativePath\":\"%s\",\"particleV2Bytes\":%u,"
        "\"particleV2Sha256\":\"%s\","
        "\"resolvedSpriteRelativePath\":\"%s\","
        "\"resolvedSpriteBytes\":%u,\"resolvedSpriteSha256\":\"%s\","
        "\"finiteCanonical\":true,\"selectorFields\":["
        "\"completeMessageClosure\",\"ordinaryClientSceneClosure\","
        "\"base1Authority\",\"startupConfigurationAuthority\","
        "\"persistentSceneEquality\",\"zeroTransparencySources\","
        "\"finiteCanonicalState\",\"freezeClosable\"],"
        "\"candidateRelativePath\":%s%s%s,\"candidateCanonicalBytes\":%s,"
        "\"candidateSha256\":%s%s%s,\"telemetry\":{"
        "\"ordinal\":%llu,\"messageSha256\":\"%s\",\"frameNumber\":%d,"
        "\"deltaFrame\":%d,\"serverTime\":%d,\"clientTime\":%d,"
        "\"refdefTimeBits\":%u,\"lerpfracBits\":%u}}\n",
        token, b1_phase5brpb_scene_classification(boundary), startup,
        persistent_path, boundary->persistent_bytes, persistent,
        has_semantic ? "\"" : "null", has_semantic ? semantic_path : "",
        has_semantic ? "\"" : "", has_semantic ? semantic_bytes_text : "null",
        has_semantic ? "\"" : "null", has_semantic ? semantic : "",
        has_semantic ? "\"" : "", boundary->scene_particle_count,
        boundary->beam_entity_count, boundary->active_beam_count,
        boundary->resolved_sprite_count, boundary->debug_primitive_count,
        particle_path, boundary->particle_v2_bytes, particle_v2,
        sprite_path, boundary->sprite_source_bytes, sprite_source,
        has_semantic ? "\"" : "null", has_semantic ? semantic_path : "",
        has_semantic ? "\"" : "", has_semantic ? semantic_bytes_text : "null",
        has_semantic ? "\"" : "null", has_semantic ? semantic : "",
        has_semantic ? "\"" : "", (unsigned long long)boundary->ordinal,
        message, boundary->frame.number, boundary->frame.delta,
        boundary->observed_server_time, boundary->observed_client_time,
        refdef_time_bits, boundary->observed_lerpfrac_bits);
}

static void b1_phase5brpb_publish_source_blobs(void)
{
    b1_phase5brpb_create_directory("particle-v2");
    b1_phase5brpb_create_directory("resolved-sprites");
    for (uint32_t index = 0; index < b1.rpb.message_count; index++) {
        const b1_phase5brpb_boundary_t *boundary = b1.rpb.boundaries + index;
        bool particle_first = true;
        bool sprite_first = true;

        if (!boundary->scene_closed)
            continue;
        if (!boundary->particle_v2 || !boundary->particle_v2_bytes ||
            !boundary->sprite_source || !boundary->sprite_source_bytes)
            b1_fail("Phase-5B-RP-B source blob closure is incomplete");
        for (uint32_t prior_index = 0; prior_index < index; prior_index++) {
            const b1_phase5brpb_boundary_t *prior =
                b1.rpb.boundaries + prior_index;
            if (!prior->scene_closed)
                continue;
            if (!memcmp(prior->particle_v2_sha256,
                        boundary->particle_v2_sha256, 32)) {
                if (prior->particle_v2_bytes != boundary->particle_v2_bytes ||
                    memcmp(prior->particle_v2, boundary->particle_v2,
                           boundary->particle_v2_bytes))
                    b1_fail("Phase-5B-RP-B particle-V2 SHA-256 collision detected");
                particle_first = false;
            }
            if (!memcmp(prior->sprite_source_sha256,
                        boundary->sprite_source_sha256, 32)) {
                if (prior->sprite_source_bytes != boundary->sprite_source_bytes ||
                    memcmp(prior->sprite_source, boundary->sprite_source,
                           boundary->sprite_source_bytes))
                    b1_fail("Phase-5B-RP-B sprite-source SHA-256 collision detected");
                sprite_first = false;
            }
        }
        if (particle_first) {
            char digest[65], relative[112];
            b1_format_sha256(boundary->particle_v2_sha256, digest);
            Q_snprintf(relative, sizeof(relative), "particle-v2/%s.bin", digest);
            b1_phase5brpb_write_blob(relative, boundary->particle_v2,
                boundary->particle_v2_bytes, boundary->particle_v2_sha256);
        }
        if (sprite_first) {
            char digest[65], relative[112];
            b1_format_sha256(boundary->sprite_source_sha256, digest);
            Q_snprintf(relative, sizeof(relative),
                       "resolved-sprites/%s.bin", digest);
            b1_phase5brpb_write_blob(relative, boundary->sprite_source,
                boundary->sprite_source_bytes, boundary->sprite_source_sha256);
        }
    }
}

static void b1_phase5brpbr4_emit_alignment(
    const b1_phase5brpb_boundary_t *boundary)
{
    static const char output_name[] = "r4-entity-alignment.jsonl";
    static const char raw_name[] = "raw-entities-v1.bin";
    static const char projected_name[] = "projected-entities-v1.bin";
    char partial[MAX_OSPATH];
    char message[65], endpoint[65], range_set[65];
    char raw[65], projected[65], raw_models[65], projected_models[65];
    char raw_membership[65], projected_membership[65];
    FILE *log;

    if (!boundary || !boundary->selected_anchor || !boundary->scene_closed ||
        !boundary->endpoint_identity_closed || !boundary->endpoint_timing_closed ||
        boundary->scene_build_count != 1u ||
        !boundary->raw_entities_v1 || !boundary->projected_entities_v1)
        b1_fail("Phase-5B-RP-B-R4 alignment output lacks a closed endpoint build");
    b1_phase5brpb_write_blob(raw_name, boundary->raw_entities_v1,
        boundary->raw_entities_v1_bytes, boundary->raw_entities_sha256);
    b1_phase5brpb_write_blob(projected_name, boundary->projected_entities_v1,
        boundary->projected_entities_v1_bytes,
        boundary->projected_entities_sha256);
    b1_format_sha256(boundary->message_sha256, message);
    b1_format_sha256(boundary->endpoint_sha256, endpoint);
    b1_format_sha256(boundary->endpoint_range_set_sha256, range_set);
    b1_format_sha256(boundary->raw_entities_sha256, raw);
    b1_format_sha256(boundary->projected_entities_sha256, projected);
    b1_format_sha256(boundary->raw_model_sequence_sha256, raw_models);
    b1_format_sha256(boundary->projected_model_sequence_sha256,
                     projected_models);
    b1_format_sha256(boundary->raw_membership_sha256, raw_membership);
    b1_format_sha256(boundary->projected_membership_sha256,
                     projected_membership);
    log = b1_phase5brpb_open_json(output_name, partial);
    if (fprintf(log,
        "{\"event\":\"r4_header\",\"schemaVersion\":1,"
        "\"classification\":\"B1_RPB_R4_ENDPOINT_ENTITY_ALIGNMENT_ONLY\","
        "\"sourceRevision\":\"%s\",\"processId\":%lu,"
        "\"fixtureSha256\":\"%s\",\"selectedMessageSha256\":\"%s\","
        "\"discoveryAuthorised\":false,\"holdoutAuthorised\":false,"
        "\"sceneAdmissionAuthorised\":false,\"temporalActivated\":false,"
        "\"pairZeroCount\":0,\"pixelCount\":0}\n",
        Q2RTX_UPSTREAM_REVISION, (unsigned long)GetCurrentProcessId(),
        B1_PHASE5B_FIXTURE_SHA256, message) < 0 ||
        fprintf(log,
        "{\"event\":\"r4_endpoint\",\"schemaVersion\":1,"
        "\"frameNumber\":%d,\"deltaFrame\":%d,\"serverTime\":%d,"
        "\"observedClientTime\":%d,\"observedLerpfracBits\":%u,"
        "\"endpointClientTime\":%d,\"endpointLerpfracBits\":1065353216,"
        "\"endpointSha256\":\"%s\",\"rangeSetSha256\":\"%s\","
        "\"canonicalBytes\":%u,\"rangeCount\":%u,"
        "\"endpointIdentityClosed\":true,\"pauseClosed\":true,"
        "\"laterMessageParseCount\":0}\n",
        boundary->frame.number, boundary->frame.delta,
        boundary->observed_server_time, boundary->observed_client_time,
        boundary->observed_lerpfrac_bits, boundary->scene_build_client_time,
        endpoint, range_set, boundary->endpoint_canonical_bytes,
        boundary->endpoint_range_count) < 0 ||
        fprintf(log,
        "{\"event\":\"r4_scene_build\",\"schemaVersion\":1,"
        "\"ordinarySortedSceneBuildOrdinal\":1,\"sceneBuildCount\":%u,"
        "\"clientTime\":%d,\"serverTime\":%d,\"lerpfracBits\":%u,"
        "\"frameNumber\":%d,\"deltaFrame\":%d,"
        "\"notificationSerial\":%llu,\"firstBuildAtEndpoint\":true,"
        "\"noLaterMessageParsed\":true}\n",
        boundary->scene_build_count, boundary->scene_build_client_time,
        boundary->scene_build_server_time, boundary->scene_build_lerpfrac_bits,
        boundary->scene_build_frame, boundary->scene_build_delta,
        (unsigned long long)boundary->notification_serial) < 0 ||
        fprintf(log,
        "{\"event\":\"r4_entity_sets\",\"schemaVersion\":1,"
        "\"raw\":{\"relativePath\":\"%s\",\"count\":%u,\"bytes\":%u,"
        "\"sha256\":\"%s\",\"modelSequenceSha256\":\"%s\","
        "\"membershipOrderSha256\":\"%s\"},"
        "\"projected\":{\"relativePath\":\"%s\",\"count\":%u,"
        "\"bytes\":%u,\"sha256\":\"%s\","
        "\"modelSequenceSha256\":\"%s\","
        "\"membershipOrderSha256\":\"%s\"}}\n",
        raw_name, boundary->raw_scene_entity_count,
        boundary->raw_entities_v1_bytes, raw, raw_models, raw_membership,
        projected_name, boundary->scene_entity_count,
        boundary->projected_entities_v1_bytes, projected, projected_models,
        projected_membership) < 0)
        b1_fail("Phase-5B-RP-B-R4 cannot emit alignment header");
    for (uint32_t index = 0; index < boundary->scene_entity_count; index++) {
        const entity_t *raw_entity = b1.rpb.raw_scene_entities +
            boundary->raw_scene_entity_offset + index;
        const entity_t *projected_entity = b1.rpb.scene_entities +
            boundary->scene_entity_offset + index;
        char raw_record[65], projected_record[65];
        b1_format_sha256(boundary->raw_record_sha256[index], raw_record);
        b1_format_sha256(boundary->projected_record_sha256[index],
                         projected_record);
        if (fprintf(log,
            "{\"event\":\"r4_entity_record\",\"schemaVersion\":1,"
            "\"ordinal\":%u,\"rawModel\":%d,\"rawId\":%d,"
            "\"rawFlags\":%d,\"rawRecordSha256\":\"%s\","
            "\"projectedModel\":%d,\"projectedId\":%d,"
            "\"projectedFlags\":%d,\"projectedRecordSha256\":\"%s\"}\n",
            index, raw_entity->model, raw_entity->id, raw_entity->flags,
            raw_record, projected_entity->model, projected_entity->id,
            projected_entity->flags, projected_record) < 0)
            b1_fail("Phase-5B-RP-B-R4 cannot emit an entity record");
    }
    if (fprintf(log,
        "{\"event\":\"r4_complete\",\"schemaVersion\":1,"
        "\"disposition\":\"ENTITY_ALIGNMENT_CLOSED\","
        "\"rawExpectationMatch\":true,\"projectedExpectationMatch\":true,"
        "\"modelSequenceMatch\":true,\"membershipOrderMatch\":true,"
        "\"sceneBuildCount\":1,\"laterMessageParseCount\":0,"
        "\"discoveryCount\":0,\"holdoutCount\":0,"
        "\"sceneAdmissionCount\":0,\"temporalActivationCount\":0,"
        "\"pairZeroCount\":0,\"captureCount\":0,\"pixelCount\":0,"
        "\"completed\":true}\n") < 0)
        b1_fail("Phase-5B-RP-B-R4 cannot emit alignment closure");
    b1_phase5brpb_close_json(log, output_name, partial);
}

static void b1_phase5brpb_emit_p0(void)
{
    static const char *const old_names[B1_PHASE5BRPB_OLD_COMPONENT_COUNT] = {
        "entities", "dynamicLights", "lightStyles", "decals", "areaPvs",
        "leftRole", "rightRole"
    };
    static const char output_name[] = "p0-output.jsonl";
    static const char blob_name[] = "accepted-persistent-scene-v1.bin";
    b1_phase5brpb_boundary_t *selected = NULL;
    byte range_set[32];
    char range_set_text[65], persistent_text[65], fixture[65], closure[65];
    char startup[65], runtime_data[65], selected_message[65], partial[MAX_OSPATH];
    FILE *log;

    for (uint32_t index = 0; index < b1.rpb.message_count; index++)
        if (b1.rpb.boundaries[index].selected_anchor)
            selected = b1.rpb.boundaries + index;
    if (!selected || !selected->scene_closed ||
        selected->persistent_bytes != b1.rpb.accepted_p0_bytes ||
        memcmp(selected->persistent_sha256, b1.rpb.accepted_p0_sha256, 32))
        b1_fail("Phase-5B-RP-B P0 output lacks one materialised selection");
    b1_phase5brpb_range_set_sha256(selected, range_set);
    b1_format_sha256(range_set, range_set_text);
    b1_format_sha256(selected->persistent_sha256, persistent_text);
    b1_format_sha256(b1.rpb.fixture_sha256, fixture);
    b1_format_sha256(b1.rpb.closure_sha256, closure);
    b1_format_sha256(b1.rpb.startup_sha256, startup);
    b1_format_sha256(b1.rpb.runtime_data_sha256, runtime_data);
    b1_format_sha256(selected->message_sha256, selected_message);
    b1_phase5brpb_publish_source_blobs();
    b1_phase5brpb_write_blob(blob_name, b1.rpb.accepted_p0,
        b1.rpb.accepted_p0_bytes, b1.rpb.accepted_p0_sha256);
    b1_phase5brpbr4_emit_alignment(selected);
    log = b1_phase5brpb_open_json(output_name, partial);
    if (fprintf(log,
        "{\"event\":\"study_header\",\"schemaVersion\":1,"
        "\"classification\":\"B1_PHASE5BRPB_P0_BASELINE_ONLY\","
        "\"sourceRevision\":\"%s\",\"persistentSchema\":\"%s\","
        "\"particleSchema\":\"%s\",\"fixtureSha256\":\"%s\","
        "\"closureSha256\":\"%s\",\"selectedAnchorSha256\":\"%s\","
        "\"startupSha256\":\"%s\",\"runtimeDataSha256\":\"%s\","
        "\"processId\":%lu,"
        "\"rngPerturbationCount\":0,\"demoOnly\":true,\"mapCommand\":false,"
        "\"localServer\":false,\"gameDllLoaded\":false,"
        "\"candidateAcceptanceAuthorised\":false,\"sceneSourceReady\":false,"
        "\"temporalActivated\":false,\"pixelCaptureAuthorised\":false}\n",
        Q2RTX_UPSTREAM_REVISION, B1_PHASE5BRPB_PERSISTENT_SCHEMA,
        B1_PHASE5BRPB_PARTICLE_SCHEMA, fixture, closure,
        B1_PHASE5AR5_SELECTED_MESSAGE_SHA256, startup, runtime_data,
        (unsigned long)GetCurrentProcessId()) < 0)
        b1_fail("Phase-5B-RP-B cannot emit P0 header");
    for (uint32_t index = 0; index < b1.rpb.message_count; index++) {
        const b1_phase5brpb_boundary_t *boundary = b1.rpb.boundaries + index;
        if (b1_phase5brpb_emit_message(log, boundary) < 0)
            b1_fail("Phase-5B-RP-B cannot emit P0 message");
        if (boundary->selected_anchor &&
            b1_phase5brpb_emit_scene(log, boundary) < 0)
            b1_fail("Phase-5B-RP-B cannot emit P0 scene boundary");
    }
    if (fprintf(log,
        "{\"event\":\"p0_selected_guards\",\"schemaVersion\":1,"
        "\"messageSha256\":\"%s\",\"frameNumber\":%d,"
        "\"deltaFrame\":%d,\"serverTime\":%d,"
        "\"playerOriginQ8\":[%d,%d,%d],"
        "\"viewAngles\":[0,135,0],\"fov\":75,"
        "\"activeEntities\":%d,\"areaBytes\":%d,"
        "\"activeConfigstrings\":%u,"
        "\"worldModel\":\"maps/base1.bsp\"}\n",
        selected_message, selected->frame.number, selected->frame.delta,
        selected->observed_server_time, selected->frame.ps.pmove.origin[0],
        selected->frame.ps.pmove.origin[1],
        selected->frame.ps.pmove.origin[2],
        selected->frame.numEntities, selected->frame.areabytes,
        selected->csr.end) < 0)
        b1_fail("Phase-5B-RP-B cannot emit selected P0 guards");
    for (uint32_t index = 0; index < B1_PHASE5BRPB_RANGE_COUNT; index++) {
        const b1_phase5brpb_range_t *range = selected->ranges + index;
        char digest[65];
        b1_format_sha256(range->sha256, digest);
        if (fprintf(log,
            "{\"event\":\"p0_component_range\",\"schemaVersion\":1,"
            "\"ordinal\":%u,\"name\":\"%s\",\"offset\":%u,"
            "\"bytes\":%u,\"sha256\":\"%s\"}\n",
            index, range->name, range->offset, range->bytes, digest) < 0)
            b1_fail("Phase-5B-RP-B cannot emit P0 component range");
    }
    if (fprintf(log,
        "{\"event\":\"p0_materialised\",\"schemaVersion\":1,"
        "\"persistentSchema\":\"%s\",\"canonicalRelativePath\":\"%s\","
        "\"canonicalBytes\":%u,\"persistentSha256\":\"%s\","
        "\"rangeCount\":%u,\"rangeSetSha256\":\"%s\"}\n",
        B1_PHASE5BRPB_PERSISTENT_SCHEMA, blob_name, selected->persistent_bytes,
        persistent_text, B1_PHASE5BRPB_RANGE_COUNT, range_set_text) < 0)
        b1_fail("Phase-5B-RP-B cannot emit P0 identity");
    if (fprintf(log,
        "{\"event\":\"study_complete\",\"schemaVersion\":1,"
        "\"classification\":\"B1_PHASE5BRPB_P0_BASELINE_ONLY\","
        "\"disposition\":\"P0_MATERIALISED\",\"selectedOccurrenceCount\":1,"
        "\"selectedBoundaryClosed\":true,\"laterMessageParseCount\":0,"
        "\"controlledStop\":true,\"normalDemoEof\":false,"
        "\"oldComponentCrosschecks\":{") < 0)
        b1_fail("Phase-5B-RP-B cannot emit P0 completion prefix");
    for (uint32_t index = 0; index < B1_PHASE5BRPB_OLD_COMPONENT_COUNT; index++) {
        char digest[65];
        b1_format_sha256(b1.rpb.old_component_sha256[index], digest);
        if (fprintf(log, "%s\"%s\":{\"bytes\":%u,\"sha256\":\"%s\"}",
            index ? "," : "", old_names[index],
            b1.rpb.old_component_bytes[index], digest) < 0)
            b1_fail("Phase-5B-RP-B cannot emit P0 old-component cross-check");
    }
    if (fprintf(log,
        "},\"acceptanceCount\":0,\"sceneReadyCount\":0,\"freezeCount\":0,"
        "\"temporalActivationCount\":0,\"pairZeroCount\":0,\"captureCount\":0,"
        "\"pixelCount\":0,\"rngPerturbationCount\":0,\"completed\":true}\n") < 0)
        b1_fail("Phase-5B-RP-B cannot emit P0 completion");
    b1_phase5brpb_close_json(log, output_name, partial);
}

static void b1_phase5brpb_rng_outcome_sha256(byte digest[32])
{
    static const char schema[] =
        "B1-SF-A1-PHASE5BRPB-RNG-SENSITIVE-OUTCOME-V1";
    b1_candidate_buffer_t canonical = { 0 };
    uint32_t observation_count = 0;

    for (uint32_t index = 0; index < b1.rpb.message_count; index++) {
        const b1_phase5brpb_boundary_t *boundary =
            b1.rpb.boundaries + index;
        if (boundary->scene_closed && boundary->scene_particle_count)
            observation_count++;
    }
    b1_candidate_string_bytes(&canonical, schema, sizeof(schema) - 1u);
    b1_candidate_u32(&canonical, 1u);
    b1_candidate_u32(&canonical, observation_count);
    for (uint32_t index = 0; index < b1.rpb.message_count; index++) {
        const b1_phase5brpb_boundary_t *boundary =
            b1.rpb.boundaries + index;
        if (!boundary->scene_closed || !boundary->scene_particle_count)
            continue;
        if (!boundary->particle_v2 || !boundary->particle_v2_bytes)
            b1_fail("Phase-5B-RP-B RNG outcome lacks particle-V2 bytes");
        b1_candidate_u32(&canonical, boundary->particle_v2_bytes);
        b1_candidate_append(&canonical, boundary->particle_v2,
                            boundary->particle_v2_bytes);
    }
    if (!b1_sha256_data(canonical.data, canonical.bytes, digest)) {
        b1_candidate_buffer_release(&canonical);
        b1_fail("Phase-5B-RP-B cannot hash natural RNG-sensitive outcome");
    }
    b1_candidate_buffer_release(&canonical);
}

static void b1_phase5brpb_emit_enumeration(void)
{
    static const char output_name[] = "semantic-inventory.jsonl";
    byte candidate_set[32], rng_outcome[32];
    char candidate_set_text[65], rng_text[65], fixture[65], closure[65];
    char startup[65], runtime_data[65], accepted[65], partial[MAX_OSPATH];
    uint32_t indices[51] = { 0 };
    FILE *log;

    b1_phase5brpb_publish_source_blobs();
    b1_phase5brpb_create_directory("persistent");
    b1_phase5brpb_create_directory("candidates");
    for (uint32_t index = 0; index < b1.rpb.persistent_count; index++) {
        const b1_candidate_seen_t *entry = b1.rpb.persistent_seen + index;
        char digest[65], relative[96];
        b1_format_sha256(entry->digest, digest);
        Q_snprintf(relative, sizeof(relative), "persistent/%s.bin", digest);
        b1_phase5brpb_write_blob(relative, entry->canonical,
                                 entry->canonical_bytes, entry->digest);
    }
    for (uint32_t index = 0; index < b1.rpb.semantic_count; index++) {
        const b1_candidate_seen_t *entry = b1.rpb.semantic_seen + index;
        char digest[65], relative[96];
        b1_format_sha256(entry->digest, digest);
        Q_snprintf(relative, sizeof(relative), "candidates/%s.bin", digest);
        b1_phase5brpb_write_blob(relative, entry->canonical,
                                 entry->canonical_bytes, entry->digest);
    }
    b1_phase5brpb_candidate_set_sha256(candidate_set);
    b1_phase5brpb_rng_outcome_sha256(rng_outcome);
    b1_format_sha256(candidate_set, candidate_set_text);
    b1_format_sha256(rng_outcome, rng_text);
    b1_format_sha256(b1.rpb.fixture_sha256, fixture);
    b1_format_sha256(b1.rpb.closure_sha256, closure);
    b1_format_sha256(b1.rpb.startup_sha256, startup);
    b1_format_sha256(b1.rpb.runtime_data_sha256, runtime_data);
    b1_format_sha256(b1.rpb.accepted_p0_sha256, accepted);
    b1_phase5brpb_sorted_semantic_indices(indices);
    log = b1_phase5brpb_open_json(output_name, partial);
    if (fprintf(log,
        "{\"event\":\"study_header\",\"schemaVersion\":1,"
        "\"classification\":\"B1_PHASE5BRPB_SEMANTIC_INVENTORY_ONLY\","
        "\"sourceRevision\":\"%s\",\"persistentSchema\":\"%s\","
        "\"semanticSchema\":\"%s\",\"particleSchema\":\"%s\","
        "\"fixtureSha256\":\"%s\",\"closureSha256\":\"%s\","
        "\"startupSha256\":\"%s\",\"runtimeDataSha256\":\"%s\","
        "\"acceptedP0Sha256\":\"%s\","
        "\"processId\":%lu,\"selectorFields\":["
        "\"completeMessageClosure\",\"ordinaryClientSceneClosure\","
        "\"base1Authority\",\"startupConfigurationAuthority\","
        "\"persistentSceneEquality\",\"zeroTransparencySources\","
        "\"finiteCanonicalState\",\"freezeClosable\"],"
        "\"rngPerturbationCount\":0,\"demoOnly\":true,\"mapCommand\":false,"
        "\"localServer\":false,\"gameDllLoaded\":false,"
        "\"candidateAcceptanceAuthorised\":false,\"sceneSourceReady\":false,"
        "\"temporalActivated\":false,\"pixelCaptureAuthorised\":false}\n",
        Q2RTX_UPSTREAM_REVISION, B1_PHASE5BRPB_PERSISTENT_SCHEMA,
        B1_PHASE5BRPB_SEMANTIC_SCHEMA, B1_PHASE5BRPB_PARTICLE_SCHEMA,
        fixture, closure, startup, runtime_data, accepted,
        (unsigned long)GetCurrentProcessId()) < 0)
        b1_fail("Phase-5B-RP-B cannot emit enumeration header");
    for (uint32_t index = 0; index < b1.rpb.message_count; index++) {
        const b1_phase5brpb_boundary_t *boundary = b1.rpb.boundaries + index;
        if (b1_phase5brpb_emit_message(log, boundary) < 0 ||
            (boundary->scene_closed &&
             b1_phase5brpb_emit_scene(log, boundary) < 0))
            b1_fail("Phase-5B-RP-B cannot emit enumeration message/scene");
    }
    for (uint32_t ordinal = 0; ordinal < b1.rpb.semantic_count; ordinal++) {
        const b1_candidate_seen_t *entry =
            b1.rpb.semantic_seen + indices[ordinal];
        char digest[65];
        b1_format_sha256(entry->digest, digest);
        if (fprintf(log,
            "{\"event\":\"semantic_class\",\"schemaVersion\":1,"
            "\"ordinal\":%u,\"candidateRelativePath\":\"candidates/%s.bin\","
            "\"canonicalBytes\":%zu,\"candidateSha256\":\"%s\","
            "\"occurrenceCount\":%u}\n",
            ordinal, digest, entry->canonical_bytes, digest,
            entry->repeat_count) < 0)
            b1_fail("Phase-5B-RP-B cannot emit semantic class");
    }
    if (fprintf(log,
        "{\"event\":\"normal_eof\",\"schemaVersion\":1,"
        "\"messageCount\":%u,\"eligibleMessageCount\":%u,"
        "\"sceneClosureCount\":%u,\"pendingTokenCount\":0,"
        "\"recoveryCount\":%u,\"earlyStop\":false,"
        "\"rngPerturbationCount\":0,\"normalDemoEof\":true,"
        "\"complete\":true}\n",
        b1.rpb.message_count, b1.rpb.eligible_message_count,
        b1.rpb.scene_closure_count, b1.rpb.recovery_count) < 0 ||
        fprintf(log,
        "{\"event\":\"study_complete\",\"schemaVersion\":1,"
        "\"classification\":\"B1_PHASE5BRPB_SEMANTIC_INVENTORY_ONLY\","
        "\"disposition\":\"%s\",\"nonqualifyingMessageCount\":%u,"
        "\"transparencyIneligibleCount\":%u,\"qualifyingOccurrenceCount\":%u,"
        "\"candidateCount\":%u,\"candidateSetSha256\":\"%s\","
        "\"rngOutcomeSha256\":\"%s\",\"candidateAcceptanceCount\":0,"
        "\"sceneReadyCount\":0,\"freezeCount\":0,"
        "\"temporalActivationCount\":0,\"pairZeroCount\":0,"
        "\"captureCount\":0,\"pixelCount\":0,\"localServer\":false,"
        "\"gameDllLoaded\":false,\"controlledStop\":false,"
        "\"normalDemoEof\":true,\"completed\":true}\n",
        b1.rpb.persistent_mismatch_count ?
            "PERSISTENT_SCENE_MISMATCH" : "PROCESS_SEALED",
        b1.rpb.nonqualifying_message_count,
        b1.rpb.transparency_ineligible_count,
        b1.rpb.qualifying_occurrence_count, b1.rpb.semantic_count,
        candidate_set_text, rng_text) < 0)
        b1_fail("Phase-5B-RP-B cannot emit enumeration closure");
    b1_phase5brpb_close_json(log, output_name, partial);
}

static void b1_phase5brpb_finish(void)
{
    b1_phase5brpb_require_controls();
    if (!b1.rpb.settings_latched)
        b1_fail("Phase-5B-RP-B finish precedes settings latch");
    b1_verify_settings_serialization(true, true);
    if (!b1.rpb.started || b1.rpb.output_sealed ||
        b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_PENDING_SCENE ||
        b1.rpb.pending_token_id || !b1_candidate_isolation_ok("finish", false))
        b1_fail("Phase-5B-RP-B finish boundary is incomplete or non-isolated");
    if (b1.rpb.resources_latched)
        b1_phase5brpb_verify_resources();
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0) {
        if (b1.rpb.playback_state != B1_PHASE5BRPB_PLAYBACK_P0_CLOSED ||
            b1.rpb.normal_eof || !b1.rpb.controlled_stop_requested ||
            !b1.rpb.controlled_disconnect_entered ||
            !b1.demo_hook.stop_requested || b1.rpb.selected_anchor_count != 1u ||
            b1.rpb.scene_closure_count != 1u ||
            b1.rpb.later_message_parse_count != 0u)
            b1_fail("Phase-5B-RP-B P0 stop lifecycle differs");
    } else if (b1.rpb.playback_state != B1_PHASE5BRPB_PLAYBACK_EOF ||
               !b1.rpb.normal_eof || b1.rpb.controlled_stop_requested ||
               b1.demo_hook.stop_requested ||
               b1.rpb.message_count != B1_PHASE5BRPB_EXPECTED_MESSAGES ||
               b1.rpb.eligible_message_count != b1.rpb.scene_closure_count) {
        b1_fail("Phase-5B-RP-B enumeration EOF lifecycle differs");
    }
    for (uint32_t index = 0; index < b1.rpb.message_count; index++)
        b1_phase5brpb_canonicalise_boundary(b1.rpb.boundaries + index);
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0)
        b1_phase5brpb_emit_p0();
    else
        b1_phase5brpb_emit_enumeration();
    b1.rpb.output_sealed = true;
    b1.rpb.playback_state = B1_PHASE5BRPB_PLAYBACK_SEALED;
}

static void b1_phase5brpb_release(void)
{
    for (uint32_t index = 0; index < B1_PHASE5BRPB_MAX_BOUNDARIES; index++) {
        if (b1.rpb.boundaries[index].particle_v2)
            Z_Free(b1.rpb.boundaries[index].particle_v2);
        if (b1.rpb.boundaries[index].sprite_source)
            Z_Free(b1.rpb.boundaries[index].sprite_source);
        if (b1.rpb.boundaries[index].raw_entities_v1)
            Z_Free(b1.rpb.boundaries[index].raw_entities_v1);
        if (b1.rpb.boundaries[index].projected_entities_v1)
            Z_Free(b1.rpb.boundaries[index].projected_entities_v1);
        if (b1.rpb.persistent_seen[index].canonical)
            Z_Free(b1.rpb.persistent_seen[index].canonical);
        if (b1.rpb.semantic_seen[index].canonical)
            Z_Free(b1.rpb.semantic_seen[index].canonical);
    }
    if (b1.rpb.message_storage) Z_Free(b1.rpb.message_storage);
    if (b1.rpb.config_storage) Z_Free(b1.rpb.config_storage);
    if (b1.rpb.endpoint_entities) Z_Free(b1.rpb.endpoint_entities);
    if (b1.rpb.raw_scene_entities) Z_Free(b1.rpb.raw_scene_entities);
    if (b1.rpb.scene_entities) Z_Free(b1.rpb.scene_entities);
    if (b1.rpb.scene_particles) Z_Free(b1.rpb.scene_particles);
    if (b1.rpb.scene_dlights) Z_Free(b1.rpb.scene_dlights);
    if (b1.rpb.scene_lightstyles) Z_Free(b1.rpb.scene_lightstyles);
    if (b1.rpb.scene_areabits) Z_Free(b1.rpb.scene_areabits);
    if (b1.rpb.model_keys) Z_Free(b1.rpb.model_keys);
    if (b1.rpb.image_keys) Z_Free(b1.rpb.image_keys);
    if (b1.rpb.sprite_frame_keys) Z_Free(b1.rpb.sprite_frame_keys);
    if (b1.rpb.selected_anchor_payload) Z_Free(b1.rpb.selected_anchor_payload);
    if (b1.rpb.accepted_p0) Z_Free(b1.rpb.accepted_p0);
    memset(&b1.rpb, 0, sizeof(b1.rpb));
}

static void b1_scene_role_sha256(b1_role_t role, const b1_plan_view_t *view,
                                 byte digest[32])
{
    b1_candidate_buffer_t buffer = { 0 };

    if (role < B1_ROLE_PRIMARY_LEFT || role >= B1_ROLE_COUNT || !view)
        b1_fail("B1 canonical role context is malformed");
    b1_candidate_u32(&buffer, (uint32_t)role);
    b1_scene_plan_view(&buffer, view);
    if (!b1_sha256_data(buffer.data, buffer.bytes, digest)) {
        b1_candidate_buffer_release(&buffer);
        b1_fail("cannot hash canonical B1 role context");
    }
    b1_candidate_buffer_release(&buffer);
}

static void b1_phase5br_input_closure_sha256(
    const b1_file_identity_t identities[6], byte digest[32])
{
    static const char schema[] = "B1-SF-A1-PHASE5BR-PLAN-INPUT-CLOSURE-V1";
    b1_candidate_buffer_t buffer = { 0 };

    if (!identities || q_countof(b1_expected_files) != 6u)
        b1_fail("B1 Phase-5B-R input closure cardinality differs");
    b1_candidate_string_bytes(&buffer, schema, sizeof(schema) - 1u);
    b1_candidate_u32(&buffer, (uint32_t)q_countof(b1_expected_files));
    for (uint32_t index = 0; index < q_countof(b1_expected_files); index++) {
        b1_candidate_u32(&buffer, index);
        b1_candidate_string_bytes(&buffer, b1_expected_files[index].name,
                                  strlen(b1_expected_files[index].name));
        b1_candidate_u64(&buffer, identities[index].bytes);
        b1_candidate_append(&buffer, identities[index].sha256, 32);
    }
    if (!b1_sha256_data(buffer.data, buffer.bytes, digest)) {
        b1_candidate_buffer_release(&buffer);
        b1_fail("cannot hash canonical B1 Phase-5B-R input closure");
    }
    b1_candidate_buffer_release(&buffer);
}

static void b1_phase5br_closure_sha256(
    const b1_renderer_scene_closure_t *closure, byte digest[32])
{
    static const char schema[] = "B1-SF-A1-PHASE5BR-RENDERER-CLOSURE-V3";
    b1_candidate_buffer_t buffer = { 0 };

    if (!closure)
        b1_fail("B1 renderer closure is absent");
    b1_candidate_string_bytes(&buffer, schema, sizeof(schema) - 1u);
    b1_candidate_u32(&buffer, closure->schema_version);
    b1_candidate_append(&buffer, closure->authority.fixture_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.plan_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.input_closure_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.executable_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.runtime_manifest_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.runtime_inventory_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.selected_message_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.range_set_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.endpoint_sha256, 32);
    b1_candidate_append(&buffer, closure->authority.client_scene_sha256, 32);
    b1_candidate_u32(&buffer, closure->authority.logical_index);
    b1_candidate_u32(&buffer, closure->authority.source_frame_id);
    b1_candidate_u32(&buffer, closure->authority.scene_scope);
    b1_candidate_u32(&buffer, closure->authority.scene_scope_generation);
    b1_candidate_u32(&buffer, closure->authority.effective_particle_count);
    b1_candidate_u64(&buffer, closure->invocation_id);
    b1_candidate_u64(&buffer, closure->preparation_serial);
    b1_candidate_u32(&buffer, closure->preparation_count);
    b1_candidate_u32(&buffer, closure->active_entity_count);
    b1_candidate_u32(&buffer, closure->animated_index_count);
    b1_candidate_u32(&buffer, closure->model_instance_count);
    b1_candidate_u32(&buffer, closure->iqm_matrix_count);
    b1_candidate_u32(&buffer, closure->light_cluster_count);
    b1_candidate_u32(&buffer, closure->prepared_role_source_mask);
    for (uint32_t role = 0; role < B1_ROLE_COUNT; role++)
        b1_candidate_u64(&buffer, closure->role_storage_hash64[role]);
    b1_candidate_u64(&buffer, closure->iqm_hash64);
    b1_candidate_u64(&buffer, closure->world_material_hash64);
    b1_candidate_u64(&buffer, closure->world_material_generation);
    b1_candidate_u64(&buffer, closure->model_light_hash64);
    b1_candidate_u64(&buffer, closure->model_light_generation);
    b1_candidate_u64(&buffer, closure->light_count_hash64);
    b1_candidate_u64(&buffer, closure->light_count_ledger_hash64);
    b1_candidate_u64(&buffer, closure->primary_left_tlas_hash64);
    b1_candidate_u64(&buffer, closure->renderer_build_identity_hash64);
    b1_candidate_u64(&buffer, closure->renderer_configuration_identity_hash64);
    b1_candidate_u64(&buffer, closure->temporal_infrastructure_identity_hash64);
    b1_candidate_u64(&buffer, closure->aggregate_hash64);
    b1_candidate_u8(&buffer, closure->primary_left_tlas_prepared ? 1u : 0u);
    b1_candidate_u8(&buffer, closure->right_gpu_execution_observed ? 1u : 0u);
    b1_candidate_u8(&buffer, closure->right_tlas_execution_observed ? 1u : 0u);
    if (!b1_sha256_data(buffer.data, buffer.bytes, digest)) {
        b1_candidate_buffer_release(&buffer);
        b1_fail("cannot hash canonical B1 renderer closure");
    }
    b1_candidate_buffer_release(&buffer);
}

static void b1_phase5br_transition(b1_scene_lifecycle_state_t next)
{
    char fixture_text[65], plan_text[65], input_closure_text[65], executable_text[65];
    char runtime_manifest_text[65], runtime_inventory_text[65];
    char selected_message_text[65], range_set_text[65];
    char endpoint_text[65], client_text[65];
    char predecessor_text[65], digest_text[65];
    char canonical[4096];
    byte digest[32];
    byte zero_digest[32] = { 0 };
    const byte *client_digest = next == B1_SCENE_ENDPOINT_ACCEPTED ?
        zero_digest : b1.source_authority.client_scene_sha256;
    b1_scene_lifecycle_state_t predecessor = b1.scene_lifecycle.state;
    uint32_t serial = b1.scene_lifecycle.transition_serial + 1u;
    bool scene_ready = next >= B1_SCENE_COMPLETE_SCENE_ACCEPTED;
    bool command_buffer_begun = next == B1_SCENE_PAIR_COMMITTED;
    uint32_t preparation_count = next >= B1_SCENE_RENDERER_SCENE_PREPARED ?
        b1.scene_lifecycle.preparation_count : 0u;
    uint64_t preparation_serial = next >= B1_SCENE_RENDERER_SCENE_PREPARED ?
        b1.scene_lifecycle.preparation_serial : 0u;
    uint64_t invocation_id = next >= B1_SCENE_RENDERER_SCENE_PREPARED ?
        b1.scene_lifecycle.invocation_id : 0u;
    uint32_t prepared_closure_consumption_count =
        next >= B1_SCENE_TRACE_PERMITTED ?
            b1.scene_lifecycle.prepared_closure_consumption_count : 0u;
    uint64_t prepared_identity = next >= B1_SCENE_RENDERER_SCENE_PREPARED ?
        b1.scene_lifecycle.prepared_closure.aggregate_hash64 : 0u;
    uint64_t consumed_identity = prepared_closure_consumption_count ?
        prepared_identity : 0u;
    const char *histories_valid = next == B1_SCENE_PAIR_COMMITTED ?
        "[true,true]" : "[false,false]";
    size_t length;

    if (!b1.log || next < B1_SCENE_ENDPOINT_ACCEPTED ||
        next > B1_SCENE_PAIR_COMMITTED || (unsigned)next != (unsigned)predecessor + 1u ||
        serial == 0 || (scene_ready != b1.scene_source_ready) ||
        (command_buffer_begun && !b1.scene_lifecycle.command_buffer_begun))
        b1_fail("B1 scene lifecycle transition is skipped, repeated or contradictory");
    b1_phase5br_recheck_source_authority("lifecycle-transition", false);
    b1_format_sha256(b1.source_authority.fixture_sha256, fixture_text);
    b1_format_sha256(b1.source_authority.plan_sha256, plan_text);
    b1_format_sha256(b1.source_authority.input_closure_sha256,
                     input_closure_text);
    b1_format_sha256(b1.source_authority.executable_sha256, executable_text);
    b1_format_sha256(b1.source_authority.runtime_manifest_sha256,
                     runtime_manifest_text);
    b1_format_sha256(b1.source_authority.runtime_inventory_sha256,
                     runtime_inventory_text);
    b1_format_sha256(b1.source_authority.selected_message_sha256,
                     selected_message_text);
    b1_format_sha256(b1.source_authority.range_set_sha256, range_set_text);
    b1_format_sha256(b1.source_authority.endpoint_sha256, endpoint_text);
    b1_format_sha256(client_digest, client_text);
    b1_format_sha256(predecessor ? b1.scene_lifecycle.transition_digest : zero_digest,
                     predecessor_text);
    length = Q_snprintf(canonical, sizeof(canonical),
        "{\"clientSceneSha256\":\"%s\",\"commandBufferBegun\":%s,"
        "\"consumedIdentityHash64\":\"%016llx\","
        "\"endpointSha256\":\"%s\","
        "\"executableSha256\":\"%s\",\"fixtureSha256\":\"%s\","
        "\"historiesValid\":%s,"
        "\"historyReadCount\":0,\"inputClosureSha256\":\"%s\","
        "\"invocationId\":%llu,\"logicalIndex\":0,"
        "\"nextState\":\"%s\",\"pairCommitCount\":%u,"
        "\"planSha256\":\"%s\","
        "\"predecessorDigestSha256\":\"%s\",\"predecessorState\":\"%s\","
        "\"preparationCount\":%u,\"preparationSerial\":%llu,"
        "\"preparedClosureConsumptionCount\":%u,"
        "\"preparedIdentityHash64\":\"%016llx\","
        "\"publicationCount\":%u,\"rangeSetSha256\":\"%s\","
        "\"runtimeInventorySha256\":\"%s\","
        "\"runtimeManifestSha256\":\"%s\","
        "\"sceneSourceReady\":%s,\"schemaVersion\":1,"
        "\"selectedMessageSha256\":\"%s\",\"sourceFrameId\":%u,"
        "\"terminal\":false,\"transitionSerial\":%u}",
        client_text, command_buffer_begun ? "true" : "false",
        (unsigned long long)consumed_identity, endpoint_text, executable_text,
        fixture_text, histories_valid, input_closure_text,
        (unsigned long long)invocation_id,
        b1_scene_lifecycle_name(next), b1.scene_lifecycle.pair_commit_count,
        plan_text, predecessor_text, b1_scene_lifecycle_name(predecessor),
        preparation_count,
        (unsigned long long)preparation_serial,
        prepared_closure_consumption_count,
        (unsigned long long)prepared_identity,
        b1.scene_lifecycle.publication_count, range_set_text,
        runtime_inventory_text, runtime_manifest_text,
        scene_ready ? "true" : "false",
        selected_message_text, b1.source_authority.source_frame_id, serial);
    if (length >= sizeof(canonical) ||
        !b1_sha256_data(canonical, length, digest))
        b1_fail("cannot close canonical B1 scene lifecycle transition");
    b1_format_sha256(digest, digest_text);
    if (fprintf(b1.log,
            "{\"event\":\"phase5br_lifecycle_transition\",\"schemaVersion\":1,"
            "\"transitionSerial\":%u,\"predecessorState\":\"%s\","
            "\"nextState\":\"%s\",\"logicalIndex\":0,"
            "\"endpointSha256\":\"%s\",\"clientSceneSha256\":\"%s\","
            "\"fixtureSha256\":\"%s\",\"planSha256\":\"%s\","
            "\"inputClosureSha256\":\"%s\","
            "\"executableSha256\":\"%s\","
            "\"runtimeManifestSha256\":\"%s\","
            "\"runtimeInventorySha256\":\"%s\","
            "\"selectedMessageSha256\":\"%s\",\"rangeSetSha256\":\"%s\","
            "\"sourceFrameId\":%u,"
            "\"predecessorDigestSha256\":\"%s\","
            "\"transitionDigestSha256\":\"%s\",\"invocationId\":%llu,"
            "\"preparationSerial\":%llu,\"preparationCount\":%u,"
            "\"preparedClosureConsumptionCount\":%u,"
            "\"preparedIdentityHash64\":\"%016llx\","
            "\"consumedIdentityHash64\":\"%016llx\","
            "\"commandBufferBegun\":%s,"
            "\"historyReadCount\":0,\"publicationCount\":%u,"
            "\"pairCommitCount\":%u,\"historiesValid\":%s,"
            "\"sceneSourceReady\":%s,\"terminal\":false}\n",
            serial, b1_scene_lifecycle_name(predecessor),
            b1_scene_lifecycle_name(next), endpoint_text, client_text,
            fixture_text, plan_text, input_closure_text, executable_text,
            runtime_manifest_text, runtime_inventory_text,
            selected_message_text, range_set_text,
            b1.source_authority.source_frame_id,
            predecessor_text, digest_text, (unsigned long long)invocation_id,
            (unsigned long long)preparation_serial, preparation_count,
            prepared_closure_consumption_count,
            (unsigned long long)prepared_identity,
            (unsigned long long)consumed_identity,
            command_buffer_begun ? "true" : "false",
            b1.scene_lifecycle.publication_count,
            b1.scene_lifecycle.pair_commit_count,
            histories_valid,
            scene_ready ? "true" : "false") < 0 || fflush(b1.log))
        b1_fail("cannot persist B1 scene lifecycle transition");
    b1.scene_lifecycle.state = next;
    b1.scene_lifecycle.transition_serial = serial;
    memcpy(b1.scene_lifecycle.transition_digest, digest, sizeof(digest));
}

static bool b1_phase5br_accept_renderer_scene(
    const b1_renderer_scene_closure_t *closure, void *opaque)
{
    byte closure_digest[32];
    (void)opaque;

    if (!closure || b1.scene_lifecycle.state !=
            B1_SCENE_TEMPORAL_INFRASTRUCTURE_READY ||
        !b1.temporal_infrastructure_ready || b1.scene_source_ready ||
        b1.bootstrap_complete || b1.scene_lifecycle.prepared_closure_valid)
        b1_fail("B1 renderer scene acceptance arrived from an illegal lifecycle state");
    b1_assert_scene_unchanged("renderer-scene-prepared");
    b1_phase5brpbr6r4_close_normal_entity_authority(true);
    if (closure->schema_version != B1_RENDERER_SCENE_CLOSURE_VERSION ||
        !b1_scene_authority_equal(&closure->authority,
                                  &b1.source_authority) ||
        closure->invocation_id == 0 || closure->preparation_serial != 1u ||
        closure->preparation_count != 1u ||
        closure->active_entity_count != (uint32_t)b1.frozen_refdef.num_entities ||
        closure->animated_index_count > closure->model_instance_count ||
        closure->model_instance_count == 0 ||
        closure->light_cluster_count != B1_RENDERER_SCENE_LIGHT_CLUSTER_COUNT ||
        closure->prepared_role_source_mask != 3u ||
        closure->role_storage_hash64[0] == 0 ||
        closure->role_storage_hash64[1] == 0 || closure->iqm_hash64 == 0 ||
        closure->world_material_hash64 == 0 ||
        closure->model_light_hash64 == 0 ||
        closure->model_light_generation == 0 ||
        closure->light_count_hash64 == 0 ||
        closure->light_count_ledger_hash64 == 0 ||
        closure->primary_left_tlas_hash64 == 0 || closure->aggregate_hash64 == 0 ||
        closure->renderer_build_identity_hash64 == 0 ||
        closure->renderer_configuration_identity_hash64 == 0 ||
        closure->temporal_infrastructure_identity_hash64 == 0 ||
        !closure->primary_left_tlas_prepared ||
        closure->right_gpu_execution_observed ||
        closure->right_tlas_execution_observed)
        b1_fail("B1 renderer-derived scene closure or authority differs");
    b1_phase5br_closure_sha256(closure, closure_digest);
    b1.scene_lifecycle.invocation_id = closure->invocation_id;
    b1.scene_lifecycle.preparation_serial = closure->preparation_serial;
    b1.scene_lifecycle.preparation_count = closure->preparation_count;
    b1.scene_lifecycle.prepared_closure = *closure;
    memcpy(b1.scene_lifecycle.prepared_closure_sha256, closure_digest, 32);
    b1.scene_lifecycle.prepared_closure_valid = true;
    b1_phase5br_transition(B1_SCENE_RENDERER_SCENE_PREPARED);
    b1.scene_source_ready = true;
    b1.bootstrap_complete = true;
    b1_phase5br_transition(B1_SCENE_COMPLETE_SCENE_ACCEPTED);
    return true;
}

void B1_PrivateSceneAdmissionWitnessExpectedStop(
    const b1_renderer_scene_closure_t *closure,
    uint32_t route_observation_count, bool route_witness_complete,
    bool tagged_route_complete, bool zero_route_complete,
    bool canonical_zero_restored, uint32_t activation_submission_count,
    uint32_t activation_completion_count,
    uint32_t activation_expected_submission_count,
    uint32_t renderer_acceptance_count, bool validation_enabled,
    uint32_t validation_error_count, bool device_lost)
{
    static const char terminal[] =
        "B1_EXPECTED_STOP: STATIC_CORE_SCENE_ADMISSION_WITNESS";
    const b1_entity_alignment_authority_t *authority =
        &b1.entity_alignment_authority;
    const char *classification;
    char fixture[65], plan[65], input_closure[65], executable[65];
    char runtime_manifest[65], runtime_inventory[65], selected_message[65];
    char range_set[65], endpoint[65], client_scene[65];
    char raw_entities[65], projected_entities[65], closure_sha256[65];
    char staged_left_view[65], expected_left_view[65];
    byte expected_raw[32], expected_projected[32], expected_models[32];
    byte expected_raw_membership[32], expected_projected_membership[32];
    byte actual_closure_sha256[32];
    byte actual_raw_sha256[32], actual_projected_sha256[32];
    byte staged_left_view_sha256[32], expected_left_view_sha256[32];

    if (!closure || !B1_Requested() || !B1_Ready() || !b1.log ||
        b1.technical_process_kind !=
            B1_TECHNICAL_PROCESS_STATIC_CORE_SCENE_ADMISSION_WITNESS)
        b1_fail("scene-admission witness callback arrived outside its authenticated process");
    B1_VerifySettings();
    b1_phase5br_recheck_source_authority(
        "static-core-scene-admission-witness-terminal", false);
    b1_assert_scene_unchanged("static-core-scene-admission-witness-terminal");

    if (!b1.scene_controller.enabled || !b1_scene_controller ||
        strcmp(b1_scene_controller->string, "1") ||
        b1_scene_controller->modified || b1.rpb.enabled ||
        b1.rpb.mode != B1_PHASE5BRPB_MODE_OFF || !b1_phase5brpb_mode ||
        strcmp(b1_phase5brpb_mode->string, "0") ||
        b1_phase5brpb_mode->modified || !authority->closed ||
        authority->version != B1_PHASE5BRPBR6R4_ENTITY_ALIGNMENT_VERSION ||
        authority->publication_count != 1u ||
        authority->revalidation_count != 1u ||
        !authority->raw_entities_v1 || !authority->projected_entities_v1 ||
        authority->raw_count != B1_PHASE5BRPBR2_ENTITY_COUNT ||
        authority->projected_count != B1_PHASE5BRPBR2_ENTITY_COUNT ||
        authority->raw_canonical_bytes != B1_PHASE5BRPBR2_ENTITY_BYTES ||
        authority->projected_canonical_bytes !=
            B1_PHASE5BRPBR2_ENTITY_BYTES ||
        !b1_parse_sha256(B1_PHASE5BRPBR2_ENTITY_SHA256, expected_raw) ||
        !b1_parse_sha256(B1_PHASE5BRPBR4_PROJECTED_ENTITY_SHA256,
                         expected_projected) ||
        !b1_parse_sha256(B1_PHASE5BRPBR4_MODEL_SEQUENCE_SHA256,
                         expected_models) ||
        !b1_parse_sha256(B1_PHASE5BRPBR4_RAW_MEMBERSHIP_SHA256,
                         expected_raw_membership) ||
        !b1_parse_sha256(B1_PHASE5BRPBR4_PROJECTED_MEMBERSHIP_SHA256,
                         expected_projected_membership) ||
        memcmp(authority->raw_entities_sha256, expected_raw, 32) ||
        memcmp(authority->projected_entities_sha256, expected_projected, 32) ||
        memcmp(authority->raw_model_sequence_sha256, expected_models, 32) ||
        memcmp(authority->projected_model_sequence_sha256,
               expected_models, 32) ||
        memcmp(authority->raw_membership_sha256,
               expected_raw_membership, 32) ||
        memcmp(authority->projected_membership_sha256,
               expected_projected_membership, 32) ||
        authority->projection_rdflags != b1.records[0].rdflags ||
        !b1_sha256_data(authority->raw_entities_v1,
                        authority->raw_canonical_bytes,
                        actual_raw_sha256) ||
        !b1_sha256_data(authority->projected_entities_v1,
                        authority->projected_canonical_bytes,
                        actual_projected_sha256) ||
        memcmp(actual_raw_sha256, authority->raw_entities_sha256, 32) ||
        memcmp(actual_projected_sha256,
               authority->projected_entities_sha256, 32) ||
        memcmp(authority->endpoint_sha256,
               b1.source_authority.endpoint_sha256, 32))
        b1_fail("scene-admission witness endpoint/entity authority differs");

    if (!b1.roles[B1_ROLE_PRIMARY_LEFT].pending_valid ||
        b1.roles[B1_ROLE_PRIMARY_RIGHT].pending_valid)
        b1_fail("scene-admission witness pending-role state differs");
    b1_scene_role_sha256(B1_ROLE_PRIMARY_LEFT,
                         &b1.roles[B1_ROLE_PRIMARY_LEFT].pending,
                         staged_left_view_sha256);
    b1_scene_role_sha256(B1_ROLE_PRIMARY_LEFT,
                         &b1.records[0].views[B1_ROLE_PRIMARY_LEFT],
                         expected_left_view_sha256);
    b1_phase5br_closure_sha256(closure, actual_closure_sha256);
    if (memcmp(actual_closure_sha256,
               b1.scene_lifecycle.prepared_closure_sha256, 32) ||
        closure->aggregate_hash64 == 0 ||
        closure->temporal_infrastructure_identity_hash64 == 0 ||
        closure->authority.scene_scope !=
            B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_V1 ||
        closure->authority.scene_scope_generation !=
            B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_GENERATION ||
        closure->authority.effective_particle_count != 0u ||
        b1.scene_lifecycle.state != B1_SCENE_COMPLETE_SCENE_ACCEPTED ||
        b1.scene_lifecycle.transition_serial != 5u ||
        b1.scene_lifecycle.preparation_count != 1u ||
        b1.scene_lifecycle.prepared_closure_consumption_count != 0u ||
        b1.scene_lifecycle.publication_count != 0u ||
        b1.scene_lifecycle.pair_commit_count != 0u ||
        b1.scene_lifecycle.command_buffer_begun || !b1.scene_source_ready ||
        !b1.bootstrap_complete || !b1.temporal_infrastructure_ready ||
        !b1.descriptor_closed || b1.logical_index != 0u ||
        b1.active_role != B1_ROLE_PRIMARY_LEFT ||
        b1.state != B1_STATE_LEFT_RENDER || !b1.pair_started ||
        b1.frame_pending || b1.committed_pairs != 0u ||
        b1.rendered[0] != 0u || b1.rendered[1] != 0u ||
        b1.captured[0] != 0u || b1.captured[1] != 0u ||
        !b1.roles[B1_ROLE_PRIMARY_LEFT].pending_valid ||
        b1.roles[B1_ROLE_PRIMARY_RIGHT].pending_valid ||
        memcmp(staged_left_view_sha256, expected_left_view_sha256, 32))
        b1_fail("scene-admission witness lifecycle or scene closure differs");

    if (!route_witness_complete || !tagged_route_complete ||
        !zero_route_complete || !canonical_zero_restored ||
        route_observation_count != 264u ||
        activation_submission_count != 9u ||
        activation_completion_count != 9u ||
        activation_expected_submission_count != 9u ||
        renderer_acceptance_count != 1u || !validation_enabled ||
        validation_error_count != 0u || device_lost)
        b1_fail("scene-admission witness activation, route or validation ledger differs");

    classification = b1.scene_admission_witness_authoritative ?
        "AUTHORITATIVE_R6R4R2_STAGED_LEFT_SCENE_BOOTSTRAP_WITNESS" :
        "NON_AUTHORITATIVE_R6R4R2_STAGED_LEFT_REHEARSAL";
    b1_format_sha256(closure->authority.fixture_sha256, fixture);
    b1_format_sha256(closure->authority.plan_sha256, plan);
    b1_format_sha256(closure->authority.input_closure_sha256, input_closure);
    b1_format_sha256(closure->authority.executable_sha256, executable);
    b1_format_sha256(closure->authority.runtime_manifest_sha256,
                     runtime_manifest);
    b1_format_sha256(closure->authority.runtime_inventory_sha256,
                     runtime_inventory);
    b1_format_sha256(closure->authority.selected_message_sha256,
                     selected_message);
    b1_format_sha256(closure->authority.range_set_sha256, range_set);
    b1_format_sha256(closure->authority.endpoint_sha256, endpoint);
    b1_format_sha256(closure->authority.client_scene_sha256, client_scene);
    b1_format_sha256(authority->raw_entities_sha256, raw_entities);
    b1_format_sha256(authority->projected_entities_sha256, projected_entities);
    b1_format_sha256(actual_closure_sha256, closure_sha256);
    b1_format_sha256(staged_left_view_sha256, staged_left_view);
    b1_format_sha256(expected_left_view_sha256, expected_left_view);

    if (fprintf(b1.log,
        "{\"event\":\"b1_static_core_scene_admission_witness_terminal\","
        "\"schemaVersion\":3,\"classification\":\"%s\","
        "\"technicalProcessKind\":5,\"technicalProcessNumeric\":\"5\","
        "\"technicalProcessToken\":\"STATIC_CORE_SCENE_ADMISSION_WITNESS\","
        "\"technicalProcessIdentityHash64\":\"%016llx\","
        "\"terminalSeam\":\"POST_SCENE_ACCEPTANCE_PRE_TRACE_LEFT_STAGED\","
        "\"activeRole\":\"PRIMARY_LEFT\",\"leftPendingValid\":true,"
        "\"rightPendingValid\":false,\"stagedLeftViewSha256\":\"%s\","
        "\"expectedLeftViewSha256\":\"%s\","
        "\"stagedLeftViewMatchesPlan\":true,\"roleBeginCount\":1,"
        "\"roleCompleteCount\":0,\"stagedOnlyNotRendered\":true,"
        "\"sceneScope\":\"%s\",\"sceneScopeGeneration\":%u,"
        "\"effectiveParticleCount\":0,\"fixtureSha256\":\"%s\","
        "\"planSha256\":\"%s\",\"inputClosureSha256\":\"%s\","
        "\"executableSha256\":\"%s\",\"runtimeManifestSha256\":\"%s\","
        "\"runtimeInventorySha256\":\"%s\",\"selectedMessageSha256\":\"%s\","
        "\"rangeSetSha256\":\"%s\",\"endpointSha256\":\"%s\","
        "\"clientSceneSha256\":\"%s\",\"rawEntitiesSha256\":\"%s\","
        "\"projectedEntitiesSha256\":\"%s\","
        "\"rendererSceneClosureHash64\":\"%016llx\","
        "\"rendererSceneClosureSha256\":\"%s\","
        "\"temporalInfrastructureIdentityHash64\":\"%016llx\","
        "\"routeWitnessComplete\":true,\"routeObservationCount\":264,"
        "\"taggedRouteComplete\":true,\"zeroRouteComplete\":true,"
        "\"canonicalZeroRestored\":true,\"activationSubmissionCount\":9,"
        "\"activationCompletionCount\":9,\"activationExpectedSubmissionCount\":9,"
        "\"rendererPreparationCount\":1,\"rendererAcceptanceCount\":1,"
        "\"clientAcceptanceCount\":1,\"lifecycleTransitionCount\":5,"
        "\"lifecycleState\":\"COMPLETE_SCENE_ACCEPTED\","
        "\"tracePermissionRequested\":false,\"tracePermissionEffective\":false,"
        "\"commandBufferBegun\":false,\"commandBufferSubmitted\":false,"
        "\"rayTracingObserved\":false,\"transferRecordingObserved\":false,"
        "\"committedPairs\":0,\"roleRenderCount\":[0,0],\"captureCount\":[0,0],"
        "\"pixelCount\":0,\"validationEnabled\":true,"
        "\"validationErrorCount\":0,\"deviceLost\":false,"
        "\"canonicalInputsUnchanged\":true,\"requiredEngineTerminal\":\"%s\","
        "\"terminalMarkerEmission\":\"EXPLICIT_COM_PRINTF_BEFORE_ORDERLY_QUIT\","
        "\"terminalMarkerEmissionCount\":1,"
        "\"requiredProcessResult\":\"ORDERLY_EXIT_0_WITH_EXPLICIT_MARKER_AND_SCHEMA3_RECEIPT\","
        "\"terminal\":true,\"authoritative\":%s}\n",
        classification,
        (unsigned long long)b1.technical_process_identity_hash,
        staged_left_view, expected_left_view,
        B1_STATIC_CORE_SCOPE_TOKEN,
        closure->authority.scene_scope_generation,
        fixture, plan, input_closure, executable, runtime_manifest,
        runtime_inventory, selected_message, range_set, endpoint, client_scene,
        raw_entities, projected_entities,
        (unsigned long long)closure->aggregate_hash64, closure_sha256,
        (unsigned long long)closure->temporal_infrastructure_identity_hash64,
        terminal,
        b1.scene_admission_witness_authoritative ? "true" : "false") < 0 ||
        fflush(b1.log))
        b1_fail("cannot persist scene-admission witness terminal receipt");

    Com_Printf("%s\n", terminal);
    Com_Quit(terminal, ERR_DISCONNECT);
    b1_fail("scene-admission witness orderly quit returned");
}

static bool b1_phase5br_permit_trace(
    const b1_renderer_scene_closure_t *closure, void *opaque)
{
    byte closure_digest[32];
    (void)opaque;

    if (!closure || b1.scene_lifecycle.state !=
            B1_SCENE_COMPLETE_SCENE_ACCEPTED || !b1.scene_source_ready ||
        !b1.bootstrap_complete || !b1.scene_lifecycle.prepared_closure_valid ||
        b1.scene_lifecycle.prepared_closure_consumption_count != 0u)
        b1_fail("B1 trace permission arrived from an illegal lifecycle state");
    b1_assert_scene_unchanged("before-trace-permission");
    b1_phase5br_closure_sha256(closure, closure_digest);
    if (memcmp(closure_digest, b1.scene_lifecycle.prepared_closure_sha256, 32))
        b1_fail("B1 prepared renderer scene changed at its trace-consumption boundary");
    b1_phase5br_transition(B1_SCENE_TRACE_PERMITTED);
    return true;
}

static bool b1_phase5br_consume_renderer_scene(
    const b1_renderer_scene_closure_t *closure, uint32_t role, void *opaque)
{
    char fixture[65], plan[65], input_closure[65], executable[65];
    char runtime_manifest[65], runtime_inventory[65], selected_message[65];
    char range_set[65], endpoint[65], client_scene[65], predecessor[65], digest_text[65];
    char canonical[3072];
    byte closure_digest[32], digest[32];
    size_t length;
    (void)opaque;

    if (!closure || role != B1_ROLE_PRIMARY_LEFT ||
        b1.scene_lifecycle.state != B1_SCENE_TRACE_PERMITTED ||
        !b1.scene_source_ready || !b1.bootstrap_complete ||
        !b1.scene_lifecycle.prepared_closure_valid ||
        b1.scene_lifecycle.prepared_closure_consumption_count != 0u)
        b1_fail("B1 prepared renderer scene consumption is repeated or out of order");
    b1_assert_scene_unchanged("actual-prepared-scene-consumption");
    b1_phase5br_recheck_source_authority(
        "actual-prepared-scene-consumption", false);
    if (!b1_scene_authority_equal(&closure->authority,
                                  &b1.source_authority))
        b1_fail("B1 prepared renderer scene consumption authority differs");
    b1_phase5br_closure_sha256(closure, closure_digest);
    if (memcmp(closure_digest, b1.scene_lifecycle.prepared_closure_sha256, 32))
        b1_fail("B1 actual consumer received a different renderer scene closure");

    b1_format_sha256(b1.source_authority.fixture_sha256, fixture);
    b1_format_sha256(b1.source_authority.plan_sha256, plan);
    b1_format_sha256(b1.source_authority.input_closure_sha256, input_closure);
    b1_format_sha256(b1.source_authority.executable_sha256, executable);
    b1_format_sha256(b1.source_authority.runtime_manifest_sha256,
                     runtime_manifest);
    b1_format_sha256(b1.source_authority.runtime_inventory_sha256,
                     runtime_inventory);
    b1_format_sha256(b1.source_authority.selected_message_sha256,
                     selected_message);
    b1_format_sha256(b1.source_authority.range_set_sha256, range_set);
    b1_format_sha256(b1.source_authority.endpoint_sha256, endpoint);
    b1_format_sha256(b1.source_authority.client_scene_sha256, client_scene);
    b1_format_sha256(b1.scene_lifecycle.transition_digest, predecessor);
    length = Q_snprintf(canonical, sizeof(canonical),
        "{\"clientSceneSha256\":\"%s\",\"consumptionSerial\":1,"
        "\"endpointSha256\":\"%s\",\"executableSha256\":\"%s\","
        "\"fixtureSha256\":\"%s\",\"inputClosureSha256\":\"%s\","
        "\"invocationId\":%llu,\"logicalIndex\":0,\"planSha256\":\"%s\","
        "\"predecessorDigestSha256\":\"%s\",\"preparationSerial\":%llu,"
        "\"preparedIdentityHash64\":\"%016llx\",\"rangeSetSha256\":\"%s\","
        "\"role\":0,\"runtimeInventorySha256\":\"%s\","
        "\"runtimeManifestSha256\":\"%s\",\"schemaVersion\":1,"
        "\"selectedMessageSha256\":\"%s\",\"sourceFrameId\":%u,"
        "\"terminal\":false}",
        client_scene, endpoint, executable, fixture, input_closure,
        (unsigned long long)b1.scene_lifecycle.invocation_id, plan,
        predecessor, (unsigned long long)b1.scene_lifecycle.preparation_serial,
        (unsigned long long)b1.scene_lifecycle.prepared_closure.aggregate_hash64,
        range_set, runtime_inventory, runtime_manifest, selected_message,
        b1.source_authority.source_frame_id);
    if (length >= sizeof(canonical) || !b1_sha256_data(canonical, length, digest))
        b1_fail("cannot close canonical B1 prepared-scene consumption");
    b1_format_sha256(digest, digest_text);
    b1.scene_lifecycle.prepared_closure_consumption_count = 1u;
    memcpy(b1.scene_lifecycle.consumption_digest, digest, 32);
    if (!b1.log || fprintf(b1.log,
            "{\"event\":\"phase5br_prepared_scene_consumption\","
            "\"schemaVersion\":1,\"consumptionSerial\":1,\"role\":0,"
            "\"fixtureSha256\":\"%s\",\"planSha256\":\"%s\","
            "\"inputClosureSha256\":\"%s\",\"executableSha256\":\"%s\","
            "\"runtimeManifestSha256\":\"%s\","
            "\"runtimeInventorySha256\":\"%s\","
            "\"selectedMessageSha256\":\"%s\",\"rangeSetSha256\":\"%s\","
            "\"endpointSha256\":\"%s\",\"clientSceneSha256\":\"%s\","
            "\"logicalIndex\":0,\"sourceFrameId\":%u,\"invocationId\":%llu,"
            "\"preparationSerial\":%llu,"
            "\"preparedIdentityHash64\":\"%016llx\","
            "\"predecessorDigestSha256\":\"%s\","
            "\"consumptionDigestSha256\":\"%s\",\"terminal\":false}\n",
            fixture, plan, input_closure, executable, runtime_manifest,
            runtime_inventory, selected_message, range_set, endpoint, client_scene,
            b1.source_authority.source_frame_id,
            (unsigned long long)b1.scene_lifecycle.invocation_id,
            (unsigned long long)b1.scene_lifecycle.preparation_serial,
            (unsigned long long)b1.scene_lifecycle.prepared_closure.aggregate_hash64,
            predecessor, digest_text) < 0 || fflush(b1.log))
        b1_fail("cannot persist B1 prepared-scene consumption");
    return true;
}

static void b1_static_core_raw_particle_telemetry(
    const particle_t *particles, uint32_t count, float default_size,
    byte digest[32], size_t *canonical_bytes)
{
    b1_candidate_buffer_t buffer = { 0 };

    if (!digest || !canonical_bytes || count > MAX_PARTICLES ||
        (count != 0u && !particles))
        b1_fail("B1 static-core raw particle range is malformed");
    b1_candidate_string_bytes(&buffer, B1_STATIC_CORE_RAW_PARTICLE_SCHEMA,
                              strlen(B1_STATIC_CORE_RAW_PARTICLE_SCHEMA));
    b1_candidate_u32(&buffer, 1u);
    b1_candidate_u32(&buffer, count);
    for (uint32_t index = 0; index < count; index++) {
        const particle_t *particle = particles + index;
        uint32_t rgba = particle->rgba.u32;

        b1_phase5brpb_vec(&buffer, particle->origin, 3u,
                          "static-core raw particle origin");
        if (particle->color >= 0) {
            b1_candidate_u8(&buffer, 0u);
            b1_candidate_u8(&buffer, (uint8_t)particle->color);
        } else {
            b1_candidate_u8(&buffer, 1u);
            b1_candidate_u8(&buffer, (uint8_t)rgba);
            b1_candidate_u8(&buffer, (uint8_t)(rgba >> 8));
            b1_candidate_u8(&buffer, (uint8_t)(rgba >> 16));
        }
        if (!isfinite(particle->alpha) || particle->alpha <= 0.0f ||
            particle->alpha > 1.0f || !isfinite(particle->brightness) ||
            particle->brightness < 0.0f || !isfinite(particle->radius) ||
            particle->radius < 0.0f)
            b1_fail("B1 static-core raw particle semantic range is malformed");
        b1_phase5brpb_f32(&buffer, particle->alpha,
                          "static-core raw particle alpha");
        b1_phase5brpb_f32(&buffer, particle->brightness,
                          "static-core raw particle brightness");
        if (particle->radius == 0.0f) {
            if (!isfinite(default_size) || default_size <= 0.0f)
                b1_fail("B1 static-core default particle size is malformed");
            b1_candidate_u8(&buffer, 0u);
            b1_phase5brpb_f32(&buffer, default_size,
                              "static-core default particle size");
        } else {
            b1_candidate_u8(&buffer, 1u);
            b1_phase5brpb_f32(&buffer, particle->radius,
                              "static-core raw particle radius");
        }
    }
    if (!b1_sha256_data(buffer.data, buffer.bytes, digest)) {
        b1_candidate_buffer_release(&buffer);
        b1_fail("cannot hash B1 static-core raw particle telemetry");
    }
    *canonical_bytes = buffer.bytes;
    b1_candidate_buffer_release(&buffer);
}

static void b1_scene_sha256(byte digest[32],
                            byte component_digest[B1_PHASE5B_SCENE_COMPONENT_COUNT][32],
                            uint32_t component_bytes[B1_PHASE5B_SCENE_COMPONENT_COUNT],
                            size_t *canonical_bytes)
{
    b1_candidate_buffer_t buffer = { 0 };
    const refdef_t *fd = &b1.frozen_refdef;
    size_t start;
    uint32_t decal_count;

    if (fd->num_entities < 0 || fd->num_entities > MAX_ENTITIES ||
        fd->num_particles < 0 || fd->num_particles > MAX_PARTICLES ||
        fd->num_dlights < 0 || fd->num_dlights > MAX_DLIGHTS ||
        b1.areabytes < 0 || b1.areabytes > MAX_MAP_AREAS / 8 ||
        fd->width <= 0 || fd->height <= 0 ||
        fd->decal_beg < 0 || fd->decal_beg >= MAX_DECALS ||
        fd->decal_end < 0 || fd->decal_end >= MAX_DECALS ||
        b1.source_authority.scene_scope !=
            B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_V1 ||
        b1.source_authority.scene_scope_generation !=
            B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_GENERATION ||
        b1.source_authority.effective_particle_count != 0u)
        b1_fail("B1 client-scene semantic range is malformed");

    start = buffer.bytes;
    b1_candidate_string_bytes(&buffer, B1_PHASE5B_SCENE_SCHEMA,
                              strlen(B1_PHASE5B_SCENE_SCHEMA));
    b1_candidate_i32(&buffer, fd->x);
    b1_candidate_i32(&buffer, fd->y);
    b1_candidate_i32(&buffer, fd->width);
    b1_candidate_i32(&buffer, fd->height);
    b1_candidate_f32(&buffer, fd->fov_x);
    b1_candidate_f32(&buffer, fd->fov_y);
    b1_scene_vec_f32(&buffer, fd->vieworg, 3);
    b1_scene_vec_f32(&buffer, fd->viewangles, 3);
    b1_scene_vec_f32(&buffer, fd->blend, 4);
    b1_candidate_f32(&buffer, fd->time);
    b1_candidate_i32(&buffer, fd->rdflags);
    b1_candidate_i32(&buffer, fd->num_entities);
    b1_candidate_i32(&buffer, 0);
    b1_candidate_i32(&buffer, fd->num_dlights);
    b1_candidate_i32(&buffer, b1.areabytes);
    b1_candidate_i32(&buffer, fd->feedback.viewcluster);
    b1_candidate_i32(&buffer, fd->feedback.lookatcluster);
    b1_candidate_i32(&buffer, fd->feedback.num_light_polys);
    b1_candidate_i32(&buffer, fd->feedback.resolution_scale);
    b1_candidate_host_string(&buffer, fd->feedback.view_material,
                             sizeof(fd->feedback.view_material));
    b1_candidate_host_string(&buffer, fd->feedback.view_material_override,
                             sizeof(fd->feedback.view_material_override));
    b1_candidate_i32(&buffer, fd->feedback.view_material_index);
    b1_scene_vec_f32(&buffer, fd->feedback.hdr_color, 3);
    b1_candidate_f32(&buffer, fd->feedback.adapted_luminance);
    b1_scene_component_close(&buffer, start, 0, component_digest, component_bytes);

    start = buffer.bytes;
    b1_candidate_u32(&buffer, (uint32_t)fd->num_entities);
    for (int index = 0; index < fd->num_entities; index++) {
        const entity_t *entity = b1.entities + index;
        b1_candidate_i32(&buffer, entity->model);
        b1_scene_vec_f32(&buffer, entity->angles, 3);
        b1_scene_vec_f32(&buffer, entity->origin, 3);
        b1_candidate_i32(&buffer, entity->frame);
        b1_scene_vec_f32(&buffer, entity->oldorigin, 3);
        b1_candidate_i32(&buffer, entity->oldframe);
        b1_candidate_f32(&buffer, entity->backlerp);
        b1_candidate_i32(&buffer, entity->skinnum);
        b1_candidate_f32(&buffer, entity->alpha);
        b1_candidate_u32(&buffer, entity->rgba.u32);
        b1_candidate_i32(&buffer, entity->skin);
        b1_candidate_i32(&buffer, entity->flags);
        b1_candidate_i32(&buffer, entity->id);
        b1_candidate_i32(&buffer, entity->tent_type);
        b1_candidate_f32(&buffer, entity->scale);
    }
    b1_scene_component_close(&buffer, start, 1, component_digest, component_bytes);

    start = buffer.bytes;
    b1_candidate_string_bytes(&buffer, B1_STATIC_CORE_SCOPE_TOKEN,
                              strlen(B1_STATIC_CORE_SCOPE_TOKEN));
    b1_candidate_u32(&buffer, b1.source_authority.scene_scope);
    b1_candidate_u32(&buffer, b1.source_authority.scene_scope_generation);
    b1_candidate_u32(&buffer, b1.source_authority.effective_particle_count);
    b1_scene_component_close(&buffer, start, 2, component_digest, component_bytes);

    start = buffer.bytes;
    b1_candidate_u32(&buffer, (uint32_t)fd->num_dlights);
    for (int index = 0; index < fd->num_dlights; index++) {
        const dlight_t *light = b1.dlights + index;
        if (light->light_type != DLIGHT_SPHERE && light->light_type != DLIGHT_SPOT)
            b1_fail("B1 client-scene dynamic-light type is malformed");
        b1_scene_vec_f32(&buffer, light->origin, 3);
        b1_scene_vec_f32(&buffer, light->color, 3);
        b1_candidate_f32(&buffer, light->intensity);
        b1_candidate_f32(&buffer, light->radius);
        b1_candidate_i32(&buffer, light->light_type);
        if (light->light_type == DLIGHT_SPOT) {
            if (light->spot.emission_profile != DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF &&
                light->spot.emission_profile !=
                    DLIGHT_SPOT_EMISSION_PROFILE_AXIS_ANGLE_TEXTURE)
                b1_fail("B1 client-scene spotlight profile is malformed");
            b1_candidate_i32(&buffer, light->spot.emission_profile);
            b1_scene_vec_f32(&buffer, light->spot.direction, 3);
            if (light->spot.emission_profile == DLIGHT_SPOT_EMISSION_PROFILE_FALLOFF) {
                b1_candidate_f32(&buffer, light->spot.cos_total_width);
                b1_candidate_f32(&buffer, light->spot.cos_falloff_start);
            } else {
                b1_candidate_f32(&buffer, light->spot.total_width);
                b1_candidate_i32(&buffer, light->spot.texture);
            }
        }
    }
    b1_scene_component_close(&buffer, start, 3, component_digest, component_bytes);

    start = buffer.bytes;
    b1_candidate_u32(&buffer, MAX_LIGHTSTYLES);
    for (int index = 0; index < MAX_LIGHTSTYLES; index++)
        b1_candidate_f32(&buffer, b1.lightstyles[index].white);
    b1_scene_component_close(&buffer, start, 4, component_digest, component_bytes);

    start = buffer.bytes;
    decal_count = (uint32_t)((fd->decal_end - fd->decal_beg + MAX_DECALS) % MAX_DECALS);
    b1_candidate_i32(&buffer, fd->decal_beg);
    b1_candidate_i32(&buffer, fd->decal_end);
    b1_candidate_u32(&buffer, decal_count);
    for (uint32_t offset = 0; offset < decal_count; offset++) {
        uint32_t index = ((uint32_t)fd->decal_beg + offset) % MAX_DECALS;
        const decal_t *decal = fd->decal + index;
        b1_scene_vec_f32(&buffer, decal->pos, 3);
        b1_scene_vec_f32(&buffer, decal->dir, 3);
        b1_candidate_f32(&buffer, decal->spread);
        b1_candidate_f32(&buffer, decal->length);
        b1_candidate_f32(&buffer, decal->dummy);
    }
    b1_scene_component_close(&buffer, start, 5, component_digest, component_bytes);

    start = buffer.bytes;
    b1_candidate_u32(&buffer, (uint32_t)b1.areabytes);
    if (b1.areabytes)
        b1_candidate_append(&buffer, b1.areabits, (size_t)b1.areabytes);
    b1_scene_component_close(&buffer, start, 6, component_digest, component_bytes);

    for (uint32_t role = 0; role < B1_ROLE_COUNT; role++) {
        start = buffer.bytes;
        b1_scene_role_context(&buffer, (b1_role_t)role, &b1.frozen_roles[role],
                              &b1.records[0]);
        b1_scene_component_close(&buffer, start, 7u + role,
                                 component_digest, component_bytes);
    }

    if (!b1_sha256_data(buffer.data, buffer.bytes, digest)) {
        b1_candidate_buffer_release(&buffer);
        b1_fail("cannot hash canonical B1 client scene");
    }
    if (canonical_bytes)
        *canonical_bytes = buffer.bytes;
    b1_candidate_buffer_release(&buffer);
}

static const char *b1_candidate_configstring_classification(
    const cs_remap_t *csr, uint32_t index)
{
    if (index >= CS_STATUSBAR && index < csr->airaccel)
        return "STATUSBAR_SPAN";
    if (index >= csr->general && index < csr->end)
        return "GENERAL_SPAN";
    return "FIXED_SLOT";
}

static void b1_candidate_configstring_failure(
    uint32_t index, size_t bound, const char *classification,
    const char *message)
{
    Com_Printf(
        "B1_PHASE5AR4_CONFIGSTRING_FAILURE {\"event\":\"configstring_failure\","
        "\"schemaVersion\":1,\"index\":%u,\"bound\":%zu,\"activeEnd\":%u,"
        "\"remapMode\":\"PROTOCOL34_OLD\",\"rangeClassification\":\"%s\"}\n",
        index, bound, cl.csr.end, classification);
    b1_candidate_fail("B1_PHASE5A_STRING", message);
}

static void b1_candidate_configstring(b1_candidate_buffer_t *buffer,
                                      uint32_t index)
{
    const cs_remap_t *csr = &cl.csr;
    const char *classification;
    const char *value;
    const char *end;
    size_t bound;
    size_t active_capacity;
    size_t bytes;

    if (csr->end > q_countof(cl.configstrings) || index >= csr->end)
        b1_candidate_configstring_failure(
            index, 0u, "INVALID_INDEX", "configstring index is outside active storage");
    classification = b1_candidate_configstring_classification(csr, index);
    bound = CS_SIZE(csr, (int)index);
    active_capacity = (size_t)(csr->end - index) * sizeof(cl.configstrings[0]);
    if (!bound || bound > active_capacity)
        b1_candidate_configstring_failure(
            index, bound, classification, "configstring bound exceeds active storage");
    value = cl.configstrings[index];
    end = memchr(value, 0, bound);
    if (!end)
        b1_candidate_configstring_failure(
            index, bound, classification, "configstring lacks a bounded terminator");
    bytes = (size_t)(end - value);
    if (bytes >= sizeof(cl.configstrings[0]) &&
        !b1.candidate.configstring_span_observed) {
        b1.candidate.configstring_span_observed = true;
        Com_Printf(
            "B1_PHASE5AR4_CONFIGSTRING_BOUND {\"event\":\"configstring_bound\","
            "\"schemaVersion\":1,\"index\":%u,\"bound\":%zu,\"activeEnd\":%u,"
            "\"remapMode\":\"PROTOCOL34_OLD\",\"rangeClassification\":\"%s\","
            "\"terminatorOffset\":%zu}\n",
            index, bound, csr->end, classification, bytes);
    }
    b1_candidate_string_bytes(buffer, value, bytes);
}

static void b1_candidate_player(b1_candidate_buffer_t *buffer,
                                const player_state_t *player)
{
    b1_candidate_i32(buffer, player->pmove.pm_type);
    for (int i = 0; i < 3; i++) b1_candidate_i16(buffer, player->pmove.origin[i]);
    for (int i = 0; i < 3; i++) b1_candidate_i16(buffer, player->pmove.velocity[i]);
    b1_candidate_u8(buffer, player->pmove.pm_flags);
    b1_candidate_u8(buffer, player->pmove.pm_time);
    b1_candidate_i16(buffer, player->pmove.gravity);
    for (int i = 0; i < 3; i++) b1_candidate_i16(buffer, player->pmove.delta_angles[i]);
    for (int i = 0; i < 3; i++) b1_candidate_f32(buffer, player->viewangles[i]);
    for (int i = 0; i < 3; i++) b1_candidate_f32(buffer, player->viewoffset[i]);
    for (int i = 0; i < 3; i++) b1_candidate_f32(buffer, player->kick_angles[i]);
    for (int i = 0; i < 3; i++) b1_candidate_f32(buffer, player->gunangles[i]);
    for (int i = 0; i < 3; i++) b1_candidate_f32(buffer, player->gunoffset[i]);
    b1_candidate_i32(buffer, player->gunindex);
    b1_candidate_i32(buffer, player->gunframe);
    for (int i = 0; i < 4; i++) b1_candidate_f32(buffer, player->blend[i]);
    b1_candidate_f32(buffer, player->fov);
    b1_candidate_i32(buffer, player->rdflags);
    for (int i = 0; i < MAX_STATS; i++) b1_candidate_i16(buffer, player->stats[i]);
}

static void b1_candidate_entity(b1_candidate_buffer_t *buffer,
                                const centity_state_t *entity)
{
    b1_candidate_i32(buffer, entity->number);
    for (int i = 0; i < 3; i++) b1_candidate_f32(buffer, entity->origin[i]);
    for (int i = 0; i < 3; i++) b1_candidate_f32(buffer, entity->angles[i]);
    for (int i = 0; i < 3; i++) b1_candidate_f32(buffer, entity->old_origin[i]);
    b1_candidate_i32(buffer, entity->modelindex);
    b1_candidate_i32(buffer, entity->modelindex2);
    b1_candidate_i32(buffer, entity->modelindex3);
    b1_candidate_i32(buffer, entity->modelindex4);
    b1_candidate_i32(buffer, entity->frame);
    b1_candidate_i32(buffer, entity->skinnum);
    b1_candidate_u32(buffer, entity->effects);
    b1_candidate_i32(buffer, entity->renderfx);
    b1_candidate_i32(buffer, entity->solid);
    b1_candidate_i32(buffer, entity->sound);
    b1_candidate_i32(buffer, entity->event);
    b1_candidate_i32(buffer, entity->morefx);
    b1_candidate_f32(buffer, entity->alpha);
    b1_candidate_f32(buffer, entity->scale);
    b1_candidate_f32(buffer, entity->loop_volume);
    b1_candidate_f32(buffer, entity->loop_attenuation);
}

static b1_candidate_isolation_ledger_t b1_candidate_isolation_capture(void)
{
    b1_candidate_isolation_ledger_t ledger = { 0 };

    ledger.sv_running_present = sv_running != NULL;
    ledger.sv_running_value = sv_running ? sv_running->integer : INT_MIN;
    ledger.sv_running_zero = ledger.sv_running_present &&
        ledger.sv_running_value == 0;
    ledger.server_state = sv.state;
    ledger.server_state_dead = sv.state == ss_dead;
    ledger.server_initialized = svs.initialized;
    ledger.server_client_pool_present = svs.client_pool != NULL;
    ledger.server_entities_present = svs.entities != NULL;
    ledger.server_storage_absent = !ledger.server_initialized &&
        !ledger.server_client_pool_present && !ledger.server_entities_present;
    ledger.game_export_ge_present = ge != NULL;
    ledger.game_export_gex_present = gex != NULL;
    ledger.game_exports_absent = !ledger.game_export_ge_present &&
        !ledger.game_export_gex_present;
    ledger.client_protocol = cls.netchan.protocol;
    ledger.client_protocol_zero = ledger.client_protocol == 0;
    ledger.game_module_present =
        GetModuleHandleW(L"gamex86_64.dll") != NULL;
    ledger.game_module_absent = !ledger.game_module_present;
    ledger.demo_playback_active = cls.demo.playback != 0;
    ledger.server_address_type = cls.serverAddress.type;
    ledger.server_address_loopback =
        ledger.server_address_type == NA_LOOPBACK;
    ledger.hook_active = b1.demo_hook.active;
    ledger.hook_armed = b1.demo_hook.armed;
    ledger.hook_stop_requested = b1.demo_hook.stop_requested;
    ledger.hook_complete_notifications =
        b1.demo_hook.complete_notifications;
    ledger.pass = ledger.sv_running_zero && ledger.server_state_dead &&
        ledger.server_storage_absent && ledger.game_exports_absent &&
        ledger.client_protocol_zero && ledger.game_module_absent &&
        ledger.demo_playback_active && ledger.server_address_loopback;
    return ledger;
}

static void b1_candidate_isolation_emit(
    const b1_candidate_isolation_ledger_t *ledger, const char *context)
{
    Com_Printf(
        "B1_PHASE5AR3_ISOLATION_LEDGER {\"event\":\"isolation_ledger\","
        "\"schemaVersion\":1,\"context\":\"%s\","
        "\"svRunningPresent\":%s,\"svRunningValue\":%d,"
        "\"svRunningZero\":%s,\"serverState\":%d,"
        "\"serverStateDead\":%s,\"serverInitialised\":%s,"
        "\"serverClientPoolPresent\":%s,\"serverEntitiesPresent\":%s,"
        "\"serverStorageAbsent\":%s,\"gameExportGePresent\":%s,"
        "\"gameExportGexPresent\":%s,\"gameExportsAbsent\":%s,"
        "\"clientProtocol\":%d,\"clientProtocolZero\":%s,"
        "\"gameModulePresent\":%s,\"gameModuleAbsent\":%s,"
        "\"demoPlaybackActive\":%s,\"serverAddressType\":%d,"
        "\"serverAddressLoopback\":%s,\"hookActive\":%s,"
        "\"hookArmed\":%s,\"hookStopRequested\":%s,"
        "\"hookCompleteNotifications\":%llu,\"pass\":%s}\n",
        context,
        ledger->sv_running_present ? "true" : "false",
        ledger->sv_running_value,
        ledger->sv_running_zero ? "true" : "false",
        ledger->server_state,
        ledger->server_state_dead ? "true" : "false",
        ledger->server_initialized ? "true" : "false",
        ledger->server_client_pool_present ? "true" : "false",
        ledger->server_entities_present ? "true" : "false",
        ledger->server_storage_absent ? "true" : "false",
        ledger->game_export_ge_present ? "true" : "false",
        ledger->game_export_gex_present ? "true" : "false",
        ledger->game_exports_absent ? "true" : "false",
        ledger->client_protocol,
        ledger->client_protocol_zero ? "true" : "false",
        ledger->game_module_present ? "true" : "false",
        ledger->game_module_absent ? "true" : "false",
        ledger->demo_playback_active ? "true" : "false",
        ledger->server_address_type,
        ledger->server_address_loopback ? "true" : "false",
        ledger->hook_active ? "true" : "false",
        ledger->hook_armed ? "true" : "false",
        ledger->hook_stop_requested ? "true" : "false",
        (unsigned long long)ledger->hook_complete_notifications,
        ledger->pass ? "true" : "false");
}

static bool b1_candidate_isolation_ok(const char *context, bool emit_pass)
{
    b1_candidate_isolation_ledger_t ledger = b1_candidate_isolation_capture();
    if (emit_pass || !ledger.pass)
        b1_candidate_isolation_emit(&ledger, context);
    return ledger.pass;
}

static void b1_candidate_require_controls(void)
{
    if (!b1.candidate.enabled || !b1_candidate_inventory ||
        strcmp(b1_candidate_inventory->string, "1") ||
        b1_candidate_inventory->modified || !b1_candidate_inventory_output ||
        !b1_candidate_inventory_output->string[0] ||
        b1_candidate_inventory_output->modified || B1_Requested() ||
        b1.scene_source_ready || b1.descriptor_closed ||
        b1.temporal_infrastructure_ready || b1.bootstrap_complete)
        b1_candidate_fail("B1_PHASE5A_CONTROL_DRIFT",
                          "candidate-inventory controls or inactive boundaries changed");
}

static void b1_candidate_serialise_remap(b1_candidate_buffer_t *buffer)
{
    const cs_remap_t *csr = &cl.csr;
    if (csr->extended != cs_remap_old.extended ||
        csr->max_edicts != cs_remap_old.max_edicts ||
        csr->max_models != cs_remap_old.max_models ||
        csr->max_sounds != cs_remap_old.max_sounds ||
        csr->max_images != cs_remap_old.max_images ||
        csr->airaccel != cs_remap_old.airaccel ||
        csr->maxclients != cs_remap_old.maxclients ||
        csr->mapchecksum != cs_remap_old.mapchecksum ||
        csr->models != cs_remap_old.models ||
        csr->sounds != cs_remap_old.sounds ||
        csr->images != cs_remap_old.images ||
        csr->lights != cs_remap_old.lights ||
        csr->items != cs_remap_old.items ||
        csr->playerskins != cs_remap_old.playerskins ||
        csr->general != cs_remap_old.general || csr->end != cs_remap_old.end)
        b1_candidate_fail("B1_PHASE5A_REMAP",
                          "active protocol-34 configstring remap differs");
    b1_candidate_u8(buffer, csr->extended ? 1u : 0u);
    b1_candidate_u16(buffer, csr->max_edicts);
    b1_candidate_u16(buffer, csr->max_models);
    b1_candidate_u16(buffer, csr->max_sounds);
    b1_candidate_u16(buffer, csr->max_images);
    b1_candidate_u16(buffer, csr->airaccel);
    b1_candidate_u16(buffer, csr->maxclients);
    b1_candidate_u16(buffer, csr->mapchecksum);
    b1_candidate_u16(buffer, csr->models);
    b1_candidate_u16(buffer, csr->sounds);
    b1_candidate_u16(buffer, csr->images);
    b1_candidate_u16(buffer, csr->lights);
    b1_candidate_u16(buffer, csr->items);
    b1_candidate_u16(buffer, csr->playerskins);
    b1_candidate_u16(buffer, csr->general);
    b1_candidate_u16(buffer, csr->end);
}

static void b1_candidate_serialise_semantics(b1_candidate_buffer_t *buffer,
                                             const server_frame_t *frame,
                                             const server_frame_t *predecessor,
                                             const byte message_digest[32])
{
    int previous_number = 0;
    const char schema[] = B1_PHASE5A_SCHEMA;

    b1_candidate_string_bytes(buffer, schema, sizeof(schema) - 1u);
    b1_candidate_u32(buffer, 1u);
    b1_candidate_i32(buffer, cls.serverProtocol);
    b1_candidate_i32(buffer, cls.protocolVersion);
    b1_candidate_u32(buffer, (uint32_t)cl.esFlags);
    b1_candidate_u32(buffer, (uint32_t)cl.psFlags);
    b1_candidate_i32(buffer, cl.servercount);
    b1_candidate_i32(buffer, cl.serverstate);
    b1_candidate_i32(buffer, cl.clientNum);
    b1_candidate_serialise_remap(buffer);
    b1_candidate_host_string(buffer, cl.gamedir, sizeof(cl.gamedir));
    b1_candidate_host_string(buffer, cl.mapname, sizeof(cl.mapname));
    b1_candidate_configstring(buffer, CS_NAME);
    b1_candidate_configstring(buffer, cl.csr.models + 1u);
    b1_candidate_configstring(buffer, cl.csr.mapchecksum);
    b1_candidate_i32(buffer, frame->number);
    b1_candidate_i32(buffer, frame->delta);
    b1_candidate_i32(buffer, cl.servertime);
    b1_candidate_i32(buffer, cl.time);
    b1_candidate_u8(buffer, frame->valid ? 1u : 0u);
    b1_candidate_u32(buffer, cl.frameflags &
                     (FF_SERVERDROP | FF_BADFRAME | FF_OLDFRAME | FF_OLDENT | FF_NODELTA));
    b1_candidate_i32(buffer, frame->clientNum);
    b1_candidate_u32(buffer, (uint32_t)msg_read.cursize);
    b1_candidate_append(buffer, message_digest, 32u);
    b1_candidate_player(buffer, &frame->ps);
    b1_candidate_player(buffer, &predecessor->ps);
    if (frame->areabytes < 0 || frame->areabytes > MAX_MAP_AREA_BYTES)
        b1_candidate_fail("B1_PHASE5A_AREA_BITS", "active area-byte count is malformed");
    b1_candidate_u32(buffer, (uint32_t)frame->areabytes);
    b1_candidate_append(buffer, frame->areabits, (size_t)frame->areabytes);
    if (frame->numEntities < 0 || frame->numEntities > cl.csr.max_edicts ||
        frame->firstEntity < 0 || cl.numEntityStates < frame->firstEntity ||
        cl.numEntityStates - frame->firstEntity < frame->numEntities ||
        cl.numEntityStates - frame->firstEntity > MAX_PARSE_ENTITIES)
        b1_candidate_fail("B1_PHASE5A_ENTITY_RANGE", "active packet-entity range is stale");
    b1_candidate_u32(buffer, (uint32_t)frame->numEntities);
    for (int i = 0; i < frame->numEntities; i++) {
        const centity_state_t *entity =
            &cl.entityStates[(frame->firstEntity + i) & PARSE_ENTITIES_MASK];
        if (entity->number <= previous_number || entity->number >= cl.csr.max_edicts)
            b1_candidate_fail("B1_PHASE5A_ENTITY_ORDER",
                              "packet entities are duplicated, skipped or unstable");
        previous_number = entity->number;
        b1_candidate_entity(buffer, entity);
    }
    b1_candidate_u32(buffer, cl.csr.end);
    for (uint32_t index = 0; index < cl.csr.end; index++) {
        b1_candidate_u32(buffer, index);
        b1_candidate_configstring(buffer, index);
    }
}

static void b1_candidate_emit_complete(uint64_t ordinal, const char *classification,
                                       const byte message_digest[32], bool frame_valid,
                                       int frame_number, bool new_frame,
                                       const char *candidate_digest)
{
    char message_text[65];
    b1_format_sha256(message_digest, message_text);
    if (fprintf(b1.candidate.log,
                "{\"event\":\"complete_message\",\"schemaVersion\":1,"
                "\"ordinal\":%llu,\"messageBytes\":%zu,\"messageSha256\":\"%s\","
                "\"readCount\":%zu,\"cursorSize\":%zu,\"frameValid\":%s,"
                "\"frameNumber\":%d,\"newFrame\":%s,\"classification\":\"%s\","
                "\"candidateSha256\":%s%s%s}\n",
                (unsigned long long)ordinal, msg_read.cursize, message_text,
                msg_read.readcount, msg_read.cursize, frame_valid ? "true" : "false",
                frame_number, new_frame ? "true" : "false", classification,
                candidate_digest ? "\"" : "null",
                candidate_digest ? candidate_digest : "",
                candidate_digest ? "\"" : "") < 0 || fflush(b1.candidate.log))
        b1_candidate_fail("B1_PHASE5A_OUTPUT", "cannot persist complete-message record");
}

static void b1_candidate_complete_message(void)
{
    byte message_digest[32];
    char candidate_text[65];
    bool new_frame;
    server_frame_t *frame = &cl.frame;
    server_frame_t *predecessor;
    b1_candidate_buffer_t canonical = { 0 };

    b1_candidate_require_controls();
    if (!b1.candidate.started || !b1.candidate.log || !b1.demo_hook.active ||
        !b1.demo_hook.armed ||
        !b1_candidate_isolation_ok("complete_message", false))
        b1_candidate_fail("B1_PHASE5A_LIFECYCLE", "candidate notification is outside isolated playback");
    if (msg_read.readcount != msg_read.cursize || msg_read.cursize > UINT32_MAX ||
        !b1_sha256_data(msg_read.data, msg_read.cursize, message_digest))
        b1_candidate_fail("B1_PHASE5A_MESSAGE", "complete-message identity is malformed");
    if (++b1.candidate.message_ordinal > B1_PHASE5A_EXPECTED_MESSAGES)
        b1_candidate_fail("B1_PHASE5A_MESSAGE_COUNT", "fixture produced too many complete messages");
    if (cl.frameflags & (FF_SERVERDROP | FF_BADFRAME | FF_OLDFRAME | FF_OLDENT))
        b1_candidate_fail("B1_PHASE5A_RECOVERED_DELTA", "invalid or recovered delta state was observed");

    new_frame = frame->valid && (!b1.candidate.last_frame_valid ||
                                frame->number != b1.candidate.last_frame_number);
    if (!new_frame) {
        b1_candidate_emit_complete(b1.candidate.message_ordinal,
            frame->valid ? "INELIGIBLE_NO_NEW_FRAME" : "INELIGIBLE_NO_VALID_FRAME",
            message_digest, frame->valid, frame->number, false, NULL);
        return;
    }
    b1.candidate.last_frame_valid = true;
    b1.candidate.last_frame_number = frame->number;
    if (frame->delta <= 0) {
        b1_candidate_emit_complete(b1.candidate.message_ordinal,
            "INELIGIBLE_NO_DELTA_PREDECESSOR", message_digest, true,
            frame->number, true, NULL);
        return;
    }
    if (frame->delta == frame->number)
        b1_candidate_fail("B1_PHASE5A_PREDECESSOR", "frame delta points to itself");
    predecessor = &cl.frames[frame->delta & UPDATE_MASK];
    if (predecessor->number != frame->delta || !predecessor->valid ||
        predecessor->firstEntity < 0 || cl.numEntityStates < predecessor->firstEntity ||
        cl.numEntityStates - predecessor->firstEntity >
            MAX_PARSE_ENTITIES - MAX_PACKET_ENTITIES)
        b1_candidate_fail("B1_PHASE5A_PREDECESSOR", "declared predecessor is missing, invalid or stale");
    if (cls.serverProtocol != PROTOCOL_VERSION_DEFAULT || strcmp(cl.mapname, "base1") ||
        strcmp(cl.configstrings[cl.csr.models + 1u], "maps/base1.bsp"))
        b1_candidate_fail("B1_PHASE5A_MAP", "protocol or map identity differs from the accepted fixture");

    b1_candidate_serialise_semantics(&canonical, frame, predecessor, message_digest);
    byte semantic_digest[32];
    if (!canonical.bytes || !b1_sha256_data(canonical.data, canonical.bytes, semantic_digest)) {
        b1_candidate_buffer_release(&canonical);
        b1_candidate_fail("B1_PHASE5A_DIGEST", "cannot hash canonical semantic stream");
    }
    b1_format_sha256(semantic_digest, candidate_text);
    for (uint32_t index = 0; index < b1.candidate.candidate_count; index++) {
        b1_candidate_seen_t *seen = &b1.candidate.seen[index];
        if (memcmp(seen->digest, semantic_digest, 32))
            continue;
        if (seen->canonical_bytes != canonical.bytes ||
            memcmp(seen->canonical, canonical.data, canonical.bytes)) {
            b1_candidate_buffer_release(&canonical);
            b1_candidate_fail("B1_PHASE5A_DIGEST_COLLISION",
                              "distinct semantic records produced the same digest");
        }
        seen->repeat_count++;
        b1.candidate.repeat_count++;
        b1_candidate_emit_complete(b1.candidate.message_ordinal, "ELIGIBLE_REPEAT",
                                   message_digest, true, frame->number, true, candidate_text);
        if (fprintf(b1.candidate.log,
                    "{\"event\":\"candidate_repeat\",\"schemaVersion\":1,"
                    "\"candidateSha256\":\"%s\",\"repeatCount\":%u}\n",
                    candidate_text, seen->repeat_count) < 0 || fflush(b1.candidate.log)) {
            b1_candidate_buffer_release(&canonical);
            b1_candidate_fail("B1_PHASE5A_OUTPUT", "cannot persist candidate repeat");
        }
        b1_candidate_buffer_release(&canonical);
        return;
    }
    if (b1.candidate.candidate_count >= B1_PHASE5A_MAX_CANDIDATES) {
        b1_candidate_buffer_release(&canonical);
        b1_candidate_fail("B1_PHASE5A_CANDIDATE_COUNT", "candidate count exceeded the exact bound");
    }
    b1_candidate_seen_t *seen = &b1.candidate.seen[b1.candidate.candidate_count++];
    memcpy(seen->digest, semantic_digest, 32);
    seen->canonical = canonical.data;
    seen->canonical_bytes = canonical.bytes;
    canonical.data = NULL;
    b1_candidate_emit_complete(b1.candidate.message_ordinal, "ELIGIBLE_CANDIDATE",
                               message_digest, true, frame->number, true, candidate_text);
    if (fprintf(b1.candidate.log,
                "{\"event\":\"candidate\",\"schemaVersion\":1,"
                "\"candidateSha256\":\"%s\",\"canonicalBytes\":%zu,"
                "\"frameNumber\":%d,\"deltaFrame\":%d,\"clientTime\":%d,"
                "\"serverTime\":%d,\"playerOriginQ8\":[%d,%d,%d],"
                "\"viewAngles\":[%.9g,%.9g,%.9g],\"fov\":%.9g,"
                "\"activeEntities\":%d,\"areaBytes\":%d,"
                "\"activeConfigstrings\":%u,\"worldModel\":\"maps/base1.bsp\","
                "\"messageOrdinal\":%llu}\n",
                candidate_text, seen->canonical_bytes, frame->number, frame->delta,
                cl.time, cl.servertime, frame->ps.pmove.origin[0],
                frame->ps.pmove.origin[1], frame->ps.pmove.origin[2],
                frame->ps.viewangles[0], frame->ps.viewangles[1],
                frame->ps.viewangles[2], frame->ps.fov, frame->numEntities,
                frame->areabytes, cl.csr.end,
                (unsigned long long)b1.candidate.message_ordinal) < 0 ||
        fflush(b1.candidate.log))
        b1_candidate_fail("B1_PHASE5A_OUTPUT", "cannot persist candidate record");
}

static void b1_candidate_open(void)
{
    char canonical[MAX_OSPATH];
    char parent[MAX_OSPATH];
    char *slash;
    char *backslash;
    char *separator;
    DWORD attributes;
    DWORD length;
    byte rng_bytes[4];
    byte rng_digest[32];
    char rng_text[65];

    b1_candidate_require_controls();
    if (b1.candidate.started || b1.candidate.completed || b1.candidate.log)
        b1_candidate_fail("B1_PHASE5A_REENTRY", "candidate inventory began more than once");
    if (!b1_candidate_isolation_ok("open", true))
        b1_candidate_fail("B1_PHASE5A_ISOLATION", "demo playback is not server/game-DLL isolated");
    if ((strlen(b1_candidate_inventory_output->string) < 4u ||
         b1_candidate_inventory_output->string[1] != ':' ||
         (b1_candidate_inventory_output->string[2] != '\\' &&
          b1_candidate_inventory_output->string[2] != '/')) &&
        strncmp(b1_candidate_inventory_output->string, "\\\\", 2))
        b1_candidate_fail("B1_PHASE5A_OUTPUT_PATH", "inventory output path is not absolute");
    length = GetFullPathNameA(b1_candidate_inventory_output->string,
                              sizeof(canonical), canonical, NULL);
    if (!length || length >= sizeof(canonical) ||
        GetFileAttributesA(canonical) != INVALID_FILE_ATTRIBUTES)
        b1_candidate_fail("B1_PHASE5A_OUTPUT_PATH", "inventory output exists or is not canonical");
    Q_strlcpy(parent, canonical, sizeof(parent));
    slash = strrchr(parent, '/');
    backslash = strrchr(parent, '\\');
    separator = slash > backslash ? slash : backslash;
    if (!separator)
        b1_candidate_fail("B1_PHASE5A_OUTPUT_PATH", "inventory output lacks a parent");
    *separator = 0;
    attributes = GetFileAttributesA(parent);
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
        b1_candidate_fail("B1_PHASE5A_OUTPUT_PATH", "inventory parent is not a regular directory");
    b1.candidate.log = Q_fopen(canonical, "wxb");
    if (!b1.candidate.log)
        b1_candidate_fail("B1_PHASE5A_OUTPUT", "cannot create candidate inventory output");
    b1.candidate.started = true;
    b1.candidate.rng_observation = Q_rand();
    rng_bytes[0] = (byte)b1.candidate.rng_observation;
    rng_bytes[1] = (byte)(b1.candidate.rng_observation >> 8);
    rng_bytes[2] = (byte)(b1.candidate.rng_observation >> 16);
    rng_bytes[3] = (byte)(b1.candidate.rng_observation >> 24);
    if (!b1_sha256_data(rng_bytes, sizeof(rng_bytes), rng_digest))
        b1_candidate_fail("B1_PHASE5A_RNG", "cannot hash ordinary RNG observation");
    b1_format_sha256(rng_digest, rng_text);
    if (fprintf(b1.candidate.log,
                "{\"event\":\"candidate_inventory_header\",\"schemaVersion\":1,"
                "\"classification\":\"B1_SF_A1_PHASE5A_CANDIDATE_INVENTORY_ONLY\","
                "\"sourceRevision\":\"%s\",\"semanticSchema\":\"%s\","
                "\"fixtureSha256\":\"%s\",\"closureSha256\":\"%s\","
                "\"processId\":%lu,\"rngObservationWord\":%u,"
                "\"rngObservationSha256\":\"%s\",\"rngSeedSet\":false,"
                "\"candidateAcceptanceAuthorised\":false,\"sceneSourceReady\":false,"
                "\"temporalActivated\":false,\"demoOnly\":true,"
                "\"mapCommand\":false,\"localServer\":false,\"gameDllLoaded\":false}\n",
                Q2RTX_DIAGNOSTIC_ORIGIN, B1_PHASE5A_SCHEMA, B1_PHASE5A_FIXTURE_SHA256,
                B1_PHASE5A_CLOSURE_SHA256, (unsigned long)GetCurrentProcessId(),
                b1.candidate.rng_observation, rng_text) < 0 ||
        fflush(b1.candidate.log))
        b1_candidate_fail("B1_PHASE5A_OUTPUT", "cannot persist candidate header");
}

static void b1_candidate_finish(void)
{
    byte ordered[B1_PHASE5A_MAX_CANDIDATES * 32u];
    byte digest[32];
    char digest_text[65];
    size_t ordered_bytes;

    if (!b1.candidate.enabled || !b1.candidate.started || !b1.candidate.log)
        return;
    if (b1.candidate.failure_written ||
        b1.candidate.message_ordinal != B1_PHASE5A_EXPECTED_MESSAGES ||
        !b1.candidate.candidate_count ||
        !b1_candidate_isolation_ok("finish", false)) {
        b1_candidate_failure_record("B1_PHASE5A_INCOMPLETE");
        fclose(b1.candidate.log);
        b1.candidate.log = NULL;
        return;
    }
    ordered_bytes = (size_t)b1.candidate.candidate_count * 32u;
    for (uint32_t index = 0; index < b1.candidate.candidate_count; index++)
        memcpy(ordered + index * 32u, b1.candidate.seen[index].digest, 32u);
    if (!b1_sha256_data(ordered, ordered_bytes, digest)) {
        b1_candidate_failure_record("B1_PHASE5A_ORDERED_SET_DIGEST");
        fclose(b1.candidate.log);
        b1.candidate.log = NULL;
        return;
    }
    b1_format_sha256(digest, digest_text);
    if (fprintf(b1.candidate.log,
                "{\"event\":\"candidate_inventory_complete\",\"schemaVersion\":1,"
                "\"messageCount\":%llu,\"candidateCount\":%u,\"repeatCount\":%u,"
                "\"orderedCandidateSetSha256\":\"%s\",\"normalDemoEof\":true,"
                "\"recoveryCount\":0,\"acceptanceCount\":0,\"sceneReadyCount\":0,"
                "\"freezeCount\":0,\"temporalActivationCount\":0,"
                "\"pairZeroCount\":0,\"captureCount\":0,\"completed\":true}\n",
                (unsigned long long)b1.candidate.message_ordinal,
                b1.candidate.candidate_count, b1.candidate.repeat_count,
                digest_text) < 0 || fflush(b1.candidate.log)) {
        b1_candidate_failure_record("B1_PHASE5A_OUTPUT");
        fclose(b1.candidate.log);
        b1.candidate.log = NULL;
        return;
    }
    b1.candidate.completed = true;
    fclose(b1.candidate.log);
    b1.candidate.log = NULL;
}

static void b1_candidate_release(void)
{
    if (b1.candidate.log) {
        b1_candidate_failure_record("B1_PHASE5A_SHUTDOWN_BEFORE_COMPLETION");
        fclose(b1.candidate.log);
        b1.candidate.log = NULL;
    }
    for (uint32_t index = 0; index < B1_PHASE5A_MAX_CANDIDATES; index++) {
        if (b1.candidate.seen[index].canonical)
            Z_Free(b1.candidate.seen[index].canonical);
    }
}

static void b1_endpoint_failure_record(const char *code)
{
    if (!b1.endpoint.enabled || b1.endpoint.failure_written)
        return;
    b1.endpoint.failure_written = true;
    if (b1.endpoint.log) {
        fprintf(b1.endpoint.log,
                "{\"event\":\"endpoint_identity_failure\",\"schemaVersion\":2,"
                "\"code\":\"%s\",\"completed\":false,"
                "\"selectionCount\":%u,\"sceneSourceReady\":false,"
                "\"temporalActivated\":false,\"pairZeroCount\":0,"
                "\"captureCount\":0,\"pixelCount\":0}\n",
                code, b1.endpoint.selected ? 1u : 0u);
        fflush(b1.endpoint.log);
    }
}

static void b1_endpoint_fail(const char *code, const char *message)
{
    b1_endpoint_failure_record(code);
    b1_fail("Phase-5A-R5 %s: %s", code, message);
}

static void b1_endpoint_require_controls(void)
{
    if (!b1.endpoint.enabled || !b1_endpoint_identity ||
        strcmp(b1_endpoint_identity->string, "1") ||
        b1_endpoint_identity->modified || !b1_endpoint_identity_output ||
        !b1_endpoint_identity_output->string[0] ||
        b1_endpoint_identity_output->modified || b1.candidate.enabled ||
        B1_Requested() || b1.scene_source_ready || b1.descriptor_closed ||
        b1.temporal_infrastructure_ready || b1.bootstrap_complete)
        b1_endpoint_fail("B1_PHASE5AR5_CONTROL_DRIFT",
                         "endpoint controls or inactive boundaries changed");
}

static uint32_t b1_endpoint_f32_bits(float value, const char *label)
{
    uint32_t bits;
    if (!isfinite(value))
        b1_endpoint_fail("B1_PHASE5AR5_NONFINITE_TELEMETRY", label);
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static void b1_endpoint_close_range(b1_candidate_buffer_t *buffer,
                                    const char *name, int32_t semantic_index,
                                    size_t start)
{
    b1_endpoint_range_t *range;
    size_t bytes;
    if (!name || start >= buffer->bytes ||
        b1.endpoint.range_count >= B1_PHASE5AR5_EXPECTED_RANGES)
        b1_endpoint_fail("B1_PHASE5AR5_RANGE_BOUNDS",
                         "endpoint range bounds or count are malformed");
    bytes = buffer->bytes - start;
    if (start > UINT32_MAX || bytes > UINT32_MAX)
        b1_endpoint_fail("B1_PHASE5AR5_RANGE_BOUNDS",
                         "endpoint range does not fit its fixed-width ledger");
    range = &b1.endpoint.ranges[b1.endpoint.range_count++];
    range->name = name;
    range->semantic_index = semantic_index;
    range->offset = (uint32_t)start;
    range->bytes = (uint32_t)bytes;
    if (!b1_sha256_data(buffer->data + start, bytes, range->sha256))
        b1_endpoint_fail("B1_PHASE5AR5_RANGE_DIGEST",
                         "cannot hash endpoint canonical range");
}

static void b1_endpoint_serialise_semantics(b1_candidate_buffer_t *buffer,
                                            const server_frame_t *frame,
                                            const server_frame_t *predecessor,
                                            const byte message_digest[32])
{
    size_t start;
    int previous_number = 0;
    const char schema[] = B1_PHASE5AR5_SCHEMA;

    b1.endpoint.range_count = 0;

    start = buffer->bytes;
    b1_candidate_string_bytes(buffer, schema, sizeof(schema) - 1u);
    b1_candidate_u32(buffer, 2u);
    b1_endpoint_close_range(buffer, "schema", -1, start);

    start = buffer->bytes;
    b1_candidate_i32(buffer, cls.serverProtocol);
    b1_candidate_i32(buffer, cls.protocolVersion);
    b1_candidate_u32(buffer, (uint32_t)cl.esFlags);
    b1_candidate_u32(buffer, (uint32_t)cl.psFlags);
    b1_candidate_i32(buffer, cl.servercount);
    b1_candidate_i32(buffer, cl.serverstate);
    b1_candidate_i32(buffer, cl.clientNum);
    b1_endpoint_close_range(buffer, "protocol", -1, start);

    start = buffer->bytes;
    b1_candidate_serialise_remap(buffer);
    b1_endpoint_close_range(buffer, "configstring-remap", -1, start);

    start = buffer->bytes;
    b1_candidate_host_string(buffer, cl.gamedir, sizeof(cl.gamedir));
    b1_candidate_host_string(buffer, cl.mapname, sizeof(cl.mapname));
    b1_candidate_configstring(buffer, CS_NAME);
    b1_candidate_configstring(buffer, cl.csr.models + 1u);
    b1_candidate_configstring(buffer, cl.csr.mapchecksum);
    b1_endpoint_close_range(buffer, "map-identity", -1, start);

    start = buffer->bytes;
    b1_candidate_i32(buffer, frame->number);
    b1_candidate_i32(buffer, frame->delta);
    b1_candidate_i32(buffer, cl.servertime);
    b1_candidate_i32(buffer, cl.servertime);
    b1_candidate_f32(buffer, 1.0f);
    b1_candidate_u8(buffer, frame->valid ? 1u : 0u);
    b1_candidate_u32(buffer, cl.frameflags &
                     (FF_SERVERDROP | FF_BADFRAME | FF_OLDFRAME | FF_OLDENT | FF_NODELTA));
    b1_candidate_i32(buffer, frame->clientNum);
    b1_endpoint_close_range(buffer, "frame-endpoint-timing", -1, start);

    start = buffer->bytes;
    b1_candidate_u32(buffer, (uint32_t)msg_read.cursize);
    b1_candidate_append(buffer, message_digest, 32u);
    b1_endpoint_close_range(buffer, "complete-message", -1, start);

    start = buffer->bytes;
    b1_candidate_player(buffer, &frame->ps);
    b1_endpoint_close_range(buffer, "current-player", -1, start);

    start = buffer->bytes;
    b1_candidate_player(buffer, &predecessor->ps);
    b1_endpoint_close_range(buffer, "predecessor-player", -1, start);

    if (frame->areabytes < 0 || frame->areabytes > MAX_MAP_AREA_BYTES)
        b1_endpoint_fail("B1_PHASE5AR5_AREA_BITS",
                         "active area-byte count is malformed");
    start = buffer->bytes;
    b1_candidate_u32(buffer, (uint32_t)frame->areabytes);
    b1_candidate_append(buffer, frame->areabits, (size_t)frame->areabytes);
    b1_endpoint_close_range(buffer, "area-bits", -1, start);

    if (frame->numEntities <= 0 || frame->numEntities > cl.csr.max_edicts ||
        frame->firstEntity < 0 || cl.numEntityStates < frame->firstEntity ||
        cl.numEntityStates - frame->firstEntity < frame->numEntities ||
        cl.numEntityStates - frame->firstEntity > MAX_PARSE_ENTITIES)
        b1_endpoint_fail("B1_PHASE5AR5_ENTITY_RANGE",
                         "active packet-entity range is stale");
    for (int i = 0; i < frame->numEntities; i++) {
        const centity_state_t *entity =
            &cl.entityStates[(frame->firstEntity + i) & PARSE_ENTITIES_MASK];
        start = buffer->bytes;
        if (!i)
            b1_candidate_u32(buffer, (uint32_t)frame->numEntities);
        if (entity->number <= previous_number || entity->number >= cl.csr.max_edicts)
            b1_endpoint_fail("B1_PHASE5AR5_ENTITY_ORDER",
                             "packet entities are duplicated, skipped or unstable");
        previous_number = entity->number;
        b1_candidate_entity(buffer, entity);
        b1_endpoint_close_range(buffer, "entity", entity->number, start);
    }

    if (!cl.csr.end)
        b1_endpoint_fail("B1_PHASE5AR5_CONFIGSTRINGS",
                         "active configstring range is empty");
    for (uint32_t index = 0; index < cl.csr.end; index++) {
        start = buffer->bytes;
        if (!index)
            b1_candidate_u32(buffer, cl.csr.end);
        b1_candidate_u32(buffer, index);
        b1_candidate_configstring(buffer, index);
        b1_endpoint_close_range(buffer, "configstring", (int32_t)index, start);
    }
}

static void b1_endpoint_validate_selected(const server_frame_t *frame,
                                          const server_frame_t **predecessor)
{
    static const float expected_angles[3] = { 0.0f, 135.0f, 0.0f };
    static const int16_t expected_origin[3] = { 1024, -2560, 193 };

    if (!frame->valid || frame->number != B1_PHASE5AR5_SELECTED_FRAME ||
        frame->delta != B1_PHASE5AR5_SELECTED_DELTA ||
        cl.servertime != B1_PHASE5AR5_SELECTED_SERVER_TIME ||
        frame->numEntities != B1_PHASE5AR5_SELECTED_ENTITIES ||
        frame->areabytes != B1_PHASE5AR5_SELECTED_AREA_BYTES ||
        cl.csr.end != B1_PHASE5AR5_SELECTED_CONFIGSTRINGS ||
        cls.serverProtocol != PROTOCOL_VERSION_DEFAULT || strcmp(cl.mapname, "base1") ||
        strcmp(cl.configstrings[cl.csr.models + 1u], "maps/base1.bsp"))
        b1_endpoint_fail("B1_PHASE5AR5_SELECTION_GUARD",
                         "selected frame/map/count guard differs");
    for (int i = 0; i < 3; i++) {
        if (frame->ps.pmove.origin[i] != expected_origin[i] ||
            b1_endpoint_f32_bits(frame->ps.viewangles[i],
                                 "selected view angle is non-finite") !=
            b1_endpoint_f32_bits(expected_angles[i],
                                 "expected view angle is non-finite"))
            b1_endpoint_fail("B1_PHASE5AR5_SELECTION_GUARD",
                             "selected origin or view-angle guard differs");
    }
    if (b1_endpoint_f32_bits(frame->ps.fov, "selected FOV is non-finite") !=
        b1_endpoint_f32_bits(75.0f, "expected FOV is non-finite"))
        b1_endpoint_fail("B1_PHASE5AR5_SELECTION_GUARD",
                         "selected FOV guard differs");
    *predecessor = &cl.frames[frame->delta & UPDATE_MASK];
    if ((*predecessor)->number != frame->delta || !(*predecessor)->valid ||
        (*predecessor)->firstEntity < 0 ||
        cl.numEntityStates < (*predecessor)->firstEntity ||
        cl.numEntityStates - (*predecessor)->firstEntity >
            MAX_PARSE_ENTITIES - MAX_PACKET_ENTITIES)
        b1_endpoint_fail("B1_PHASE5AR5_SELECTION_GUARD",
                         "selected delta predecessor is missing, invalid or stale");
}

static void b1_endpoint_range_set_digest(byte digest[32])
{
    b1_candidate_buffer_t canonical = { 0 };
    b1_candidate_u32(&canonical, b1.endpoint.range_count);
    for (uint32_t index = 0; index < b1.endpoint.range_count; index++) {
        const b1_endpoint_range_t *range = &b1.endpoint.ranges[index];
        b1_candidate_u32(&canonical, index);
        b1_candidate_string_bytes(&canonical, range->name, strlen(range->name));
        b1_candidate_i32(&canonical, range->semantic_index);
        b1_candidate_u32(&canonical, range->offset);
        b1_candidate_u32(&canonical, range->bytes);
        b1_candidate_append(&canonical, range->sha256, sizeof(range->sha256));
    }
    if (!b1_sha256_data(canonical.data, canonical.bytes, digest)) {
        b1_candidate_buffer_release(&canonical);
        b1_endpoint_fail("B1_PHASE5AR5_RANGE_SET_DIGEST",
                         "cannot hash canonical endpoint range set");
    }
    b1_candidate_buffer_release(&canonical);
}

static size_t b1_endpoint_compute_selected(const server_frame_t *frame,
                                           const server_frame_t *predecessor,
                                           const byte message_digest[32],
                                           byte endpoint_digest[32],
                                           byte range_set_digest[32])
{
    b1_candidate_buffer_t canonical = { 0 };
    size_t canonical_bytes;

    b1_endpoint_serialise_semantics(&canonical, frame, predecessor, message_digest);
    if (b1.endpoint.range_count != B1_PHASE5AR5_EXPECTED_RANGES ||
        !canonical.bytes || b1.endpoint.ranges[0].offset != 0u) {
        b1_candidate_buffer_release(&canonical);
        b1_endpoint_fail("B1_PHASE5AR5_RANGE_COVERAGE",
                         "endpoint canonical range count/start differs");
    }
    for (uint32_t index = 1; index < b1.endpoint.range_count; index++) {
        const b1_endpoint_range_t *previous = &b1.endpoint.ranges[index - 1u];
        const b1_endpoint_range_t *current = &b1.endpoint.ranges[index];
        if (current->offset != previous->offset + previous->bytes) {
            b1_candidate_buffer_release(&canonical);
            b1_endpoint_fail("B1_PHASE5AR5_RANGE_COVERAGE",
                             "endpoint canonical ranges have a gap or overlap");
        }
    }
    const b1_endpoint_range_t *last =
        &b1.endpoint.ranges[b1.endpoint.range_count - 1u];
    if ((size_t)last->offset + last->bytes != canonical.bytes) {
        b1_candidate_buffer_release(&canonical);
        b1_endpoint_fail("B1_PHASE5AR5_RANGE_COVERAGE",
                         "endpoint canonical range end differs");
    }
    if (!b1_sha256_data(canonical.data, canonical.bytes, endpoint_digest)) {
        b1_candidate_buffer_release(&canonical);
        b1_endpoint_fail("B1_PHASE5AR5_ENDPOINT_DIGEST",
                         "cannot hash endpoint canonical stream");
    }
    b1_endpoint_range_set_digest(range_set_digest);
    canonical_bytes = canonical.bytes;
    b1_candidate_buffer_release(&canonical);
    return canonical_bytes;
}

static void b1_endpoint_emit_selected(const server_frame_t *frame,
                                      const server_frame_t *predecessor,
                                      const byte message_digest[32])
{
    byte endpoint_digest[32];
    byte range_set_digest[32];
    char endpoint_text[65];
    char range_set_text[65];
    uint32_t observed_lerp_bits =
        b1_endpoint_f32_bits(cl.lerpfrac, "observed interpolation is non-finite");
    size_t canonical_bytes = b1_endpoint_compute_selected(frame, predecessor,
        message_digest, endpoint_digest, range_set_digest);

    b1_format_sha256(endpoint_digest, endpoint_text);
    b1_format_sha256(range_set_digest, range_set_text);

    if (fprintf(b1.endpoint.log,
                "{\"event\":\"endpoint_telemetry\",\"schemaVersion\":2,"
                "\"messageOrdinal\":%llu,\"observedClientTime\":%d,"
                "\"observedLerpfracBits\":%u,\"endpointClientTime\":%d,"
                "\"endpointLerpfracBits\":1065353216,\"serverTime\":%d,"
                "\"observedValuesInEndpointDigest\":false}\n",
                (unsigned long long)b1.endpoint.message_ordinal, cl.time,
                observed_lerp_bits, cl.servertime, cl.servertime) < 0)
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT",
                         "cannot persist endpoint timing telemetry");

    for (uint32_t index = 0; index < b1.endpoint.range_count; index++) {
        const b1_endpoint_range_t *range = &b1.endpoint.ranges[index];
        char digest_text[65];
        b1_format_sha256(range->sha256, digest_text);
        if (fprintf(b1.endpoint.log,
                    "{\"event\":\"endpoint_range\",\"schemaVersion\":2,"
                    "\"ordinal\":%u,\"name\":\"%s\",\"semanticIndex\":%d,"
                    "\"offset\":%u,\"bytes\":%u,\"sha256\":\"%s\"}\n",
                    index, range->name, range->semantic_index, range->offset,
                    range->bytes, digest_text) < 0)
            b1_endpoint_fail("B1_PHASE5AR5_OUTPUT",
                             "cannot persist endpoint canonical range");
    }

    if (fprintf(b1.endpoint.log,
                "{\"event\":\"endpoint_identity\",\"schemaVersion\":2,"
                "\"semanticSchema\":\"%s\",\"messageSha256\":\"%s\","
                "\"messageOrdinal\":%llu,\"frameNumber\":%d,\"deltaFrame\":%d,"
                "\"serverTime\":%d,\"endpointClientTime\":%d,"
                "\"endpointLerpfracBits\":1065353216,"
                "\"playerOriginQ8\":[%d,%d,%d],"
                "\"viewAngles\":[%.9g,%.9g,%.9g],\"fov\":%.9g,"
                "\"activeEntities\":%d,\"areaBytes\":%d,"
                "\"activeConfigstrings\":%u,\"worldModel\":\"maps/base1.bsp\","
                "\"canonicalBytes\":%zu,\"rangeCount\":%u,"
                "\"rangeSetSha256\":\"%s\",\"endpointSha256\":\"%s\"}\n",
                B1_PHASE5AR5_SCHEMA, B1_PHASE5AR5_SELECTED_MESSAGE_SHA256,
                (unsigned long long)b1.endpoint.message_ordinal, frame->number,
                frame->delta, cl.servertime, cl.servertime,
                frame->ps.pmove.origin[0], frame->ps.pmove.origin[1],
                frame->ps.pmove.origin[2], frame->ps.viewangles[0],
                frame->ps.viewangles[1], frame->ps.viewangles[2], frame->ps.fov,
                frame->numEntities, frame->areabytes, cl.csr.end, canonical_bytes,
                b1.endpoint.range_count, range_set_text, endpoint_text) < 0 ||
        fprintf(b1.endpoint.log,
                "{\"event\":\"endpoint_identity_complete\",\"schemaVersion\":2,"
                "\"messageCount\":%llu,\"selectionCount\":1,\"rangeCount\":%u,"
                "\"selectedBoundaryClosed\":true,\"laterMessageParseCount\":0,"
                "\"normalDemoEof\":false,\"candidateAcceptanceCount\":0,"
                "\"sceneReadyCount\":0,\"temporalActivationCount\":0,"
                "\"pairZeroCount\":0,\"captureCount\":0,\"pixelCount\":0,"
                "\"completed\":true}\n",
                 (unsigned long long)b1.endpoint.message_ordinal,
                 b1.endpoint.range_count) < 0 || fflush(b1.endpoint.log)) {
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT",
                         "cannot persist complete endpoint identity");
    }
    b1.endpoint.completed = true;
    fclose(b1.endpoint.log);
    b1.endpoint.log = NULL;
}

static void b1_endpoint_open(void)
{
    char canonical[MAX_OSPATH];
    char parent[MAX_OSPATH];
    char *slash;
    char *backslash;
    char *separator;
    DWORD attributes;
    DWORD length;
    byte rng_bytes[4];
    byte rng_digest[32];
    char rng_text[65];

    b1_endpoint_require_controls();
    if (b1.endpoint.started || b1.endpoint.completed || b1.endpoint.log)
        b1_endpoint_fail("B1_PHASE5AR5_REENTRY",
                         "endpoint identity began more than once");
    if (!b1_candidate_isolation_ok("r5-open", true))
        b1_endpoint_fail("B1_PHASE5AR5_ISOLATION",
                         "endpoint playback is not server/game-DLL isolated");
    if ((strlen(b1_endpoint_identity_output->string) < 4u ||
         b1_endpoint_identity_output->string[1] != ':' ||
         (b1_endpoint_identity_output->string[2] != '\\' &&
          b1_endpoint_identity_output->string[2] != '/')) &&
        strncmp(b1_endpoint_identity_output->string, "\\\\", 2))
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT_PATH",
                         "endpoint output path is not absolute");
    length = GetFullPathNameA(b1_endpoint_identity_output->string,
                              sizeof(canonical), canonical, NULL);
    if (!length || length >= sizeof(canonical) ||
        GetFileAttributesA(canonical) != INVALID_FILE_ATTRIBUTES)
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT_PATH",
                         "endpoint output exists or is not canonical");
    Q_strlcpy(parent, canonical, sizeof(parent));
    slash = strrchr(parent, '/');
    backslash = strrchr(parent, '\\');
    separator = slash > backslash ? slash : backslash;
    if (!separator)
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT_PATH",
                         "endpoint output lacks a parent");
    *separator = 0;
    attributes = GetFileAttributesA(parent);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT_PATH",
                         "endpoint output parent is not a regular directory");
    b1.endpoint.log = Q_fopen(canonical, "wxb");
    if (!b1.endpoint.log)
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT",
                         "cannot create endpoint identity output");
    b1.endpoint.started = true;
    b1.endpoint.rng_observation = Q_rand();
    rng_bytes[0] = (byte)b1.endpoint.rng_observation;
    rng_bytes[1] = (byte)(b1.endpoint.rng_observation >> 8);
    rng_bytes[2] = (byte)(b1.endpoint.rng_observation >> 16);
    rng_bytes[3] = (byte)(b1.endpoint.rng_observation >> 24);
    if (!b1_sha256_data(rng_bytes, sizeof(rng_bytes), rng_digest))
        b1_endpoint_fail("B1_PHASE5AR5_RNG",
                         "cannot hash ordinary RNG observation");
    b1_format_sha256(rng_digest, rng_text);
    if (fprintf(b1.endpoint.log,
                "{\"event\":\"endpoint_identity_header\",\"schemaVersion\":2,"
                "\"classification\":\"B1_SF_A1_PHASE5AR5_ENDPOINT_IDENTITY_ONLY\","
                "\"sourceRevision\":\"%s\",\"semanticSchema\":\"%s\","
                "\"fixtureSha256\":\"%s\",\"closureSha256\":\"%s\","
                "\"selectedMessageSha256\":\"%s\","
                "\"processId\":%lu,\"rngObservationWord\":%u,"
                "\"rngObservationSha256\":\"%s\",\"rngSeedSet\":false,"
                "\"candidateAcceptanceAuthorised\":false,"
                "\"sceneSourceReady\":false,\"temporalActivated\":false,"
                "\"demoOnly\":true,\"mapCommand\":false,"
                "\"localServer\":false,\"gameDllLoaded\":false,"
                "\"pixelCaptureAuthorised\":false}\n",
                Q2RTX_DIAGNOSTIC_ORIGIN, B1_PHASE5AR5_SCHEMA,
                B1_PHASE5A_FIXTURE_SHA256, B1_PHASE5A_CLOSURE_SHA256,
                B1_PHASE5AR5_SELECTED_MESSAGE_SHA256,
                (unsigned long)GetCurrentProcessId(), b1.endpoint.rng_observation,
                rng_text) < 0 || fflush(b1.endpoint.log))
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT",
                         "cannot persist endpoint identity header");
}

static void b1_endpoint_complete_message(void)
{
    byte message_digest[32];
    char message_text[65];
    bool selected;
    bool new_frame;
    server_frame_t *frame = &cl.frame;
    const server_frame_t *predecessor = NULL;

    b1_endpoint_require_controls();
    if (!b1.endpoint.started || !b1.endpoint.log || !b1.demo_hook.active ||
        !b1.demo_hook.armed || b1.demo_hook.stop_requested ||
        !b1_candidate_isolation_ok("r5-complete-message", false))
        b1_endpoint_fail("B1_PHASE5AR5_LIFECYCLE",
                         "endpoint notification is outside isolated playback");
    if (msg_read.readcount != msg_read.cursize || msg_read.cursize > UINT32_MAX ||
        !b1_sha256_data(msg_read.data, msg_read.cursize, message_digest))
        b1_endpoint_fail("B1_PHASE5AR5_MESSAGE",
                         "complete-message identity is malformed");
    if (++b1.endpoint.message_ordinal > B1_PHASE5A_EXPECTED_MESSAGES)
        b1_endpoint_fail("B1_PHASE5AR5_SELECTION_MISSING",
                         "selected message was not observed within the exact fixture count");
    if (cl.frameflags & (FF_SERVERDROP | FF_BADFRAME | FF_OLDFRAME | FF_OLDENT))
        b1_endpoint_fail("B1_PHASE5AR5_RECOVERED_DELTA",
                         "invalid or recovered delta state was observed");
    new_frame = frame->valid && (!b1.endpoint.last_frame_valid ||
                                frame->number != b1.endpoint.last_frame_number);
    if (new_frame) {
        b1.endpoint.last_frame_valid = true;
        b1.endpoint.last_frame_number = frame->number;
    }
    b1_format_sha256(message_digest, message_text);
    selected = !strcmp(message_text, B1_PHASE5AR5_SELECTED_MESSAGE_SHA256);
    if (fprintf(b1.endpoint.log,
                "{\"event\":\"endpoint_message\",\"schemaVersion\":2,"
                "\"ordinal\":%llu,\"messageBytes\":%zu,"
                "\"messageSha256\":\"%s\",\"readCount\":%zu,"
                "\"cursorSize\":%zu,\"frameValid\":%s,\"frameNumber\":%d,"
                "\"newFrame\":%s,\"selectedDigestMatch\":%s}\n",
                (unsigned long long)b1.endpoint.message_ordinal, msg_read.cursize,
                message_text, msg_read.readcount, msg_read.cursize,
                frame->valid ? "true" : "false", frame->number,
                new_frame ? "true" : "false", selected ? "true" : "false") < 0 ||
        fflush(b1.endpoint.log))
        b1_endpoint_fail("B1_PHASE5AR5_OUTPUT",
                         "cannot persist endpoint navigation record");
    if (!selected)
        return;
    if (b1.endpoint.selected)
        b1_endpoint_fail("B1_PHASE5AR5_SELECTION_DUPLICATE",
                         "selected complete-message digest occurred more than once");
    b1.endpoint.selected = true;
    if (!new_frame)
        b1_endpoint_fail("B1_PHASE5AR5_SELECTION_GUARD",
                         "selected message did not commit a new valid frame");
    b1_endpoint_validate_selected(frame, &predecessor);
    b1_endpoint_emit_selected(frame, predecessor, message_digest);
    B1_RequestDemoPlaybackStop();
    if (!b1_candidate_isolation_ok("r5-selected-stop", true))
        b1_endpoint_fail("B1_PHASE5AR5_ISOLATION",
                         "selected parser-stop isolation proof failed");
    Com_Printf("B1_PHASE5AR5_ENDPOINT_COMPLETE "
               "{\"messageOrdinal\":%llu,\"completeNotifications\":%llu,"
               "\"stopRequested\":true,\"rangeCount\":%u,\"pass\":true}\n",
               (unsigned long long)b1.endpoint.message_ordinal,
               (unsigned long long)b1.demo_hook.complete_notifications,
               b1.endpoint.range_count);
}

static void b1_endpoint_finish(void)
{
    if (!b1.endpoint.enabled || !b1.endpoint.started || b1.endpoint.completed)
        return;
    b1_endpoint_failure_record(b1.endpoint.selected ?
        "B1_PHASE5AR5_INCOMPLETE" : "B1_PHASE5AR5_SELECTION_MISSING");
    if (b1.endpoint.log) {
        fclose(b1.endpoint.log);
        b1.endpoint.log = NULL;
    }
}

static void b1_endpoint_release(void)
{
    if (b1.endpoint.log) {
        b1_endpoint_failure_record("B1_PHASE5AR5_SHUTDOWN_BEFORE_COMPLETION");
        fclose(b1.endpoint.log);
        b1.endpoint.log = NULL;
    }
}

static void b1_scene_controller_require_controls(void)
{
    if (!b1.scene_controller.enabled || !b1_scene_controller ||
        strcmp(b1_scene_controller->string, "1") ||
        b1_scene_controller->modified || !B1_Requested() || !B1_Ready() ||
        b1.candidate.enabled || b1.endpoint.enabled)
        b1_fail("B1 scene-controller controls or activation state changed");
}

static void b1_scene_controller_compute_endpoint(const server_frame_t *frame,
                                                  const byte message_digest[32],
                                                  byte endpoint_digest[32],
                                                  byte range_set_digest[32],
                                                  size_t *canonical_bytes)
{
    const server_frame_t *predecessor = NULL;
    char endpoint_text[65], range_text[65];

    b1_endpoint_validate_selected(frame, &predecessor);
    *canonical_bytes = b1_endpoint_compute_selected(frame, predecessor,
        message_digest, endpoint_digest, range_set_digest);
    b1_format_sha256(endpoint_digest, endpoint_text);
    b1_format_sha256(range_set_digest, range_text);
    if (strcmp(endpoint_text, B1_PHASE5AR5_ACCEPTED_ENDPOINT_SHA256) ||
        strcmp(range_text, B1_PHASE5AR5_ACCEPTED_RANGE_SET_SHA256) ||
        *canonical_bytes != 25450u ||
        b1.endpoint.range_count != B1_PHASE5AR5_EXPECTED_RANGES)
        b1_fail("B1 scene-controller accepted endpoint or ordered range-set identity differs");
}

static void b1_apply_committed_snapshot_endpoint(const char *label)
{
    if (!cl.frame.valid || cl.frame.number != B1_PHASE5AR5_SELECTED_FRAME ||
        cl.frame.delta != B1_PHASE5AR5_SELECTED_DELTA ||
        cl.servertime != B1_PHASE5AR5_SELECTED_SERVER_TIME ||
        !label || !*label)
        b1_fail("%s cannot apply an invalid committed endpoint",
                label ? label : "B1 endpoint");
    cl.time = cl.servertime;
    cl.lerpfrac = 1.0f;
    Cvar_SetByVar(cl_paused, "2", FROM_CODE);
    CL_CheckForPause();
    if (cl.time != B1_PHASE5AR5_SELECTED_SERVER_TIME ||
        cl.servertime != B1_PHASE5AR5_SELECTED_SERVER_TIME ||
        b1_endpoint_f32_bits(cl.lerpfrac,
            "committed endpoint interpolation is non-finite") !=
            UINT32_C(0x3f800000) ||
        cl_paused->integer != 2 || sv_paused->integer != 1 ||
        com_timedemo->integer)
        b1_fail("%s ordinary demo endpoint/pause did not close", label);
}

static void b1_scene_controller_monitor_endpoint(const char *boundary,
                                                  bool emit)
{
    byte endpoint_digest[32], range_set_digest[32];
    size_t canonical_bytes = 0;
    char endpoint_text[65], range_text[65];

    b1_scene_controller_require_controls();
    if (!b1.scene_controller.selected || !b1.scene_controller.endpoint_closed ||
        !b1.scene_controller.fixture_preflight_closed ||
        !b1.demo_hook.active || !b1.demo_hook.armed ||
        !b1.demo_hook.stop_requested ||
        b1.demo_hook.complete_notifications !=
            b1.scene_controller.accepted_notification_count ||
        cl.time != b1.scene_controller.accepted_server_time ||
        cl.servertime != b1.scene_controller.accepted_server_time ||
        b1_endpoint_f32_bits(cl.lerpfrac,
            "scene-controller interpolation became non-finite") != UINT32_C(0x3f800000) ||
        cl.frame.number != b1.scene_controller.accepted_frame_number ||
        cl_paused->integer != 2 || sv_paused->integer != 1)
        b1_fail("B1 scene-controller endpoint advanced at %s", boundary);
    b1_scene_controller_compute_endpoint(&cl.frame,
        b1.scene_controller.message_sha256, endpoint_digest, range_set_digest,
        &canonical_bytes);
    if (memcmp(endpoint_digest, b1.scene_controller.endpoint_sha256, 32) ||
        memcmp(range_set_digest, b1.scene_controller.range_set_sha256, 32) ||
        b1.endpoint.range_count != b1.scene_controller.selected_range_count)
        b1_fail("B1 scene-controller semantic endpoint changed at %s", boundary);
    if (!emit)
        return;
    b1_format_sha256(endpoint_digest, endpoint_text);
    b1_format_sha256(range_set_digest, range_text);
    fprintf(b1.log,
            "{\"event\":\"b1_scene_controller_no_advance\","
            "\"boundary\":\"%s\",\"endpointSha256\":\"%s\","
            "\"rangeSetSha256\":\"%s\",\"canonicalBytes\":%zu,"
            "\"rangeCount\":%u,\"clientTime\":%d,\"serverTime\":%d,"
            "\"lerpfracBits\":1065353216,\"completeNotifications\":%llu,"
            "\"laterMessageParseCount\":0,\"paused\":true,"
            "\"snapshotAdvanced\":false,\"pass\":true}\n",
            boundary, endpoint_text, range_text, canonical_bytes,
            b1.endpoint.range_count, cl.time, cl.servertime,
            (unsigned long long)b1.demo_hook.complete_notifications);
    if (fflush(b1.log))
        b1_fail("cannot persist B1 scene-controller no-advance proof");
}

static void b1_scene_controller_complete_message(void)
{
    byte message_digest[32], endpoint_digest[32], range_set_digest[32];
    char message_text[65], endpoint_text[65], range_text[65];
    server_frame_t *frame = &cl.frame;
    size_t canonical_bytes = 0;
    bool selected;
    bool new_frame;

    b1_scene_controller_require_controls();
    if (!b1.demo_hook.active || !b1.demo_hook.armed ||
        b1.demo_hook.stop_requested ||
        !b1.scene_controller.fixture_preflight_closed ||
        !b1_candidate_isolation_ok("phase5b-complete-message", false))
        b1_fail("B1 scene-controller notification is outside isolated playback");
    if (msg_read.readcount != msg_read.cursize || msg_read.cursize > UINT32_MAX ||
        !b1_sha256_data(msg_read.data, msg_read.cursize, message_digest))
        b1_fail("B1 scene-controller complete-message identity is malformed");
    if (++b1.scene_controller.message_ordinal > B1_PHASE5A_EXPECTED_MESSAGES)
        b1_fail("B1 scene-controller selected endpoint was not observed");
    if (cl.frameflags & (FF_SERVERDROP | FF_BADFRAME | FF_OLDFRAME | FF_OLDENT))
        b1_fail("B1 scene-controller observed invalid or recovered delta state");
    new_frame = frame->valid && (!b1.scene_controller.last_frame_valid ||
                                frame->number != b1.scene_controller.last_frame_number);
    if (new_frame) {
        b1.scene_controller.last_frame_valid = true;
        b1.scene_controller.last_frame_number = frame->number;
    }
    b1_format_sha256(message_digest, message_text);
    selected = !strcmp(message_text, B1_PHASE5AR5_SELECTED_MESSAGE_SHA256);
    fprintf(b1.log,
            "{\"event\":\"b1_scene_controller_message\",\"messageOrdinal\":%llu,"
            "\"messageBytes\":%zu,\"messageSha256\":\"%s\","
            "\"frameValid\":%s,\"frameNumber\":%d,"
            "\"newFrame\":%s,\"selectedDigestMatch\":%s}\n",
            (unsigned long long)b1.scene_controller.message_ordinal,
            msg_read.cursize, message_text, frame->valid ? "true" : "false",
            frame->number, new_frame ? "true" : "false",
            selected ? "true" : "false");
    if (fflush(b1.log))
        b1_fail("cannot persist B1 scene-controller navigation record");
    if (!selected)
        return;
    if (b1.scene_controller.selected)
        b1_fail("B1 scene-controller selected complete message occurred more than once");
    if (!new_frame)
        b1_fail("B1 scene-controller selected message did not commit a new frame");

    b1_scene_controller_compute_endpoint(frame, message_digest, endpoint_digest,
        range_set_digest, &canonical_bytes);
    b1_format_sha256(endpoint_digest, endpoint_text);
    b1_format_sha256(range_set_digest, range_text);
    b1.scene_controller.observed_client_time = cl.time;
    b1.scene_controller.observed_lerpfrac_bits = b1_endpoint_f32_bits(
        cl.lerpfrac, "scene-controller observed interpolation is non-finite");
    b1.scene_controller.accepted_server_time = cl.servertime;
    b1.scene_controller.accepted_frame_number = frame->number;
    b1.scene_controller.accepted_notification_count =
        b1.demo_hook.complete_notifications;
    b1.scene_controller.selected_range_count = b1.endpoint.range_count;
    memcpy(b1.scene_controller.message_sha256, message_digest, 32);
    memcpy(b1.scene_controller.endpoint_sha256, endpoint_digest, 32);
    memcpy(b1.scene_controller.range_set_sha256, range_set_digest, 32);
    memcpy(b1.source_authority.selected_message_sha256, message_digest, 32);
    memcpy(b1.source_authority.range_set_sha256, range_set_digest, 32);
    memcpy(b1.source_authority.endpoint_sha256, endpoint_digest, 32);
    b1.scene_controller.selected = true;

    fprintf(b1.log,
            "{\"event\":\"b1_scene_controller_endpoint_telemetry\","
            "\"messageOrdinal\":%llu,\"observedClientTime\":%d,"
            "\"observedLerpfracBits\":%u,\"endpointClientTime\":%d,"
            "\"endpointLerpfracBits\":1065353216,"
            "\"observedValuesInEndpointDigest\":false}\n",
            (unsigned long long)b1.scene_controller.message_ordinal,
            b1.scene_controller.observed_client_time,
            b1.scene_controller.observed_lerpfrac_bits, cl.servertime);

    b1_apply_committed_snapshot_endpoint("B1 scene-controller");
    B1_RequestDemoPlaybackStop();
    b1.scene_controller.endpoint_closed = true;
    if (!b1_candidate_isolation_ok("phase5b-selected-stop", true))
        b1_fail("B1 scene-controller selected parser-stop isolation failed");
    fprintf(b1.log,
            "{\"event\":\"b1_scene_controller_endpoint_closed\","
            "\"messageSha256\":\"%s\",\"endpointSha256\":\"%s\","
            "\"rangeSetSha256\":\"%s\",\"canonicalBytes\":%zu,"
            "\"rangeCount\":%u,\"selectionCount\":1,"
            "\"fixtureUniqueOccurrenceAuthenticated\":true,"
            "\"endpointApplied\":true,\"pauseClosed\":true,"
            "\"stopRequestedBeforeNextParse\":true,"
            "\"sceneSourceReady\":false,\"pass\":true}\n",
            message_text, endpoint_text, range_text, canonical_bytes,
            b1.endpoint.range_count);
    if (fflush(b1.log))
        b1_fail("cannot persist B1 scene-controller endpoint closure");
    b1_phase5br_transition(B1_SCENE_ENDPOINT_ACCEPTED);
    b1_scene_controller_monitor_endpoint("endpoint-closed", true);
}

static void b1_log_scene_storage_component(const char *component, uint32_t index,
                                           size_t serialization_offset,
                                           const void *data, size_t bytes)
{
    const byte *cursor = data;
    static const char digits[] = "0123456789abcdef";
    if (!b1.log || !data || !bytes)
        b1_fail("invalid scene-storage localization component");
    fprintf(b1.log,
            "{\"event\":\"b1_scene_storage_component\",\"component\":\"%s\","
            "\"elementIndex\":%u,\"serializationOffset\":%llu,\"bytes\":%llu,"
            "\"purpose\":\"RAW_STORAGE_LOCALIZATION_NOT_IDENTITY\",\"rawHex\":\"",
            component, index, (unsigned long long)serialization_offset,
            (unsigned long long)bytes);
    for (size_t offset = 0; offset < bytes; offset++) {
        fputc(digits[cursor[offset] >> 4], b1.log);
        fputc(digits[cursor[offset] & 15], b1.log);
    }
    fprintf(b1.log, "\"}\n");
    if (fflush(b1.log))
        b1_fail("cannot persist scene-storage localization component");
}

static void b1_log_scene_storage(void)
{
    refdef_t scalar = b1.frozen_refdef;
    size_t offset = 0;

    /* Pointer representation is neither logged nor used as identity. */
    scalar.areabits = NULL;
    scalar.lightstyles = NULL;
    scalar.entities = NULL;
    scalar.dlights = NULL;
    scalar.particles = NULL;
    b1_log_scene_storage_component("refdef-sanitized", 0, offset,
                                   &scalar, sizeof(scalar));
    offset += sizeof(scalar);
    for (int index = 0; index < scalar.num_entities; index++) {
        b1_log_scene_storage_component("entity", (uint32_t)index, offset,
                                       b1.entities + index, sizeof(*b1.entities));
        offset += sizeof(*b1.entities);
    }
    for (int index = 0; index < scalar.num_particles; index++) {
        b1_log_scene_storage_component("particle", (uint32_t)index, offset,
                                       b1.particles + index, sizeof(*b1.particles));
        offset += sizeof(*b1.particles);
    }
    for (int index = 0; index < scalar.num_dlights; index++) {
        b1_log_scene_storage_component("dlight", (uint32_t)index, offset,
                                       b1.dlights + index, sizeof(*b1.dlights));
        offset += sizeof(*b1.dlights);
    }
    b1_log_scene_storage_component("lightstyles-active", 0, offset,
                                   b1.lightstyles,
                                   sizeof(*b1.lightstyles) * MAX_LIGHTSTYLES);
    offset += sizeof(*b1.lightstyles) * MAX_LIGHTSTYLES;
    if (b1.areabytes) {
        b1_log_scene_storage_component("areabits-active", 0, offset,
                                       b1.areabits, (size_t)b1.areabytes);
        offset += (size_t)b1.areabytes;
    }
    fprintf(b1.log,
            "{\"event\":\"b1_scene_storage_complete\",\"serializedBytes\":%llu,"
            "\"pointersExcluded\":true,\"activeCounts\":[%d,%d,%d,%d,%d]}\n",
            (unsigned long long)offset, scalar.num_entities, scalar.num_particles,
            scalar.num_dlights, MAX_LIGHTSTYLES, b1.areabytes);
    fflush(b1.log);
}

static bool b1_sha256_file(const char *path, uint64_t *bytes, byte digest[32])
{
    BCRYPT_ALG_HANDLE algorithm = NULL;
    BCRYPT_HASH_HANDLE hash = NULL;
    PUCHAR object = NULL;
    DWORD object_size = 0;
    DWORD result_size = 0;
    FILE *file = NULL;
    byte buffer[65536];
    uint64_t total = 0;
    bool ok = false;

    if (!path || !path[0])
        goto done;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0) < 0)
        goto done;
    if (BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, (PUCHAR)&object_size,
                          sizeof(object_size), &result_size, 0) < 0)
        goto done;
    object = Z_Malloc(object_size);
    if (BCryptCreateHash(algorithm, &hash, object, object_size, NULL, 0, 0) < 0)
        goto done;
    file = fopen(path, "rb");
    if (!file)
        goto done;
    for (;;) {
        size_t count = fread(buffer, 1, sizeof(buffer), file);
        if (count) {
            if (BCryptHashData(hash, buffer, (ULONG)count, 0) < 0)
                goto done;
            total += count;
        }
        if (count != sizeof(buffer)) {
            if (ferror(file))
                goto done;
            break;
        }
    }
    if (BCryptFinishHash(hash, digest, 32, 0) < 0)
        goto done;
    *bytes = total;
    ok = true;
done:
    if (file) fclose(file);
    if (hash) BCryptDestroyHash(hash);
    if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
    if (object) Z_Free(object);
    return ok;
}

static void b1_authenticate_file(const b1_expected_file_t *expected,
                                 const char *path, b1_file_identity_t *identity)
{
    byte digest[32];
    byte expected_digest[32];
    uint64_t bytes = 0;

    if (!b1_parse_sha256(expected->sha256, expected_digest))
        b1_fail("internal input digest is malformed");
    if (!b1_sha256_file(path, &bytes, digest))
        b1_fail("cannot read or hash required input %s", expected->name);
    if (bytes != expected->bytes || memcmp(digest, expected_digest, 32))
        b1_fail("required input identity differs for %s", expected->name);
    identity->bytes = bytes;
    memcpy(identity->sha256, digest, 32);
}

static const char *b1_leaf_name(const char *path)
{
    const char *slash = strrchr(path, '/');
    const char *backslash = strrchr(path, '\\');
    const char *leaf = slash > backslash ? slash : backslash;
    return leaf ? leaf + 1 : path;
}

static void b1_validate_input_names(void)
{
    char common_directory[MAX_OSPATH] = { 0 };
    char canonical[MAX_OSPATH];
    WIN32_FIND_DATAA data;
    HANDLE find;
    char pattern[MAX_OSPATH];
    uint32_t members = 0;

    for (size_t i = 0; i < q_countof(b1_expected_files); i++) {
        DWORD attributes;
        DWORD length;
        char *slash;
        char *backslash;
        char *separator;
        if (!b1_input_cvars[i]->string[0])
            b1_fail("all six input paths are required");
        if (strcmp(b1_leaf_name(b1_input_cvars[i]->string), b1_expected_files[i].name))
            b1_fail("input filename/order differs at member %u", (unsigned)i);
        attributes = GetFileAttributesA(b1_input_cvars[i]->string);
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
            b1_fail("input member is not a regular non-reparse file");
        length = GetFullPathNameA(b1_input_cvars[i]->string, sizeof(canonical), canonical, NULL);
        if (!length || length >= sizeof(canonical))
            b1_fail("input path cannot be canonicalized");
        slash = strrchr(canonical, '/');
        backslash = strrchr(canonical, '\\');
        separator = slash > backslash ? slash : backslash;
        if (!separator)
            b1_fail("input path has no parent directory");
        *separator = 0;
        if (!i)
            Q_strlcpy(common_directory, canonical, sizeof(common_directory));
        else if (_stricmp(common_directory, canonical))
            b1_fail("all six inputs must share one Builder-safe directory");
    }

    if (Q_snprintf(pattern, sizeof(pattern), "%s/*", common_directory) >= sizeof(pattern))
        b1_fail("input directory path is too long");
    find = FindFirstFileA(pattern, &data);
    if (find == INVALID_HANDLE_VALUE)
        b1_fail("cannot enumerate Builder-safe input directory");
    do {
        bool expected = false;
        if (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, ".."))
            continue;
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
            FindClose(find);
            b1_fail("Builder-safe directory contains a non-regular member");
        }
        for (size_t i = 0; i < q_countof(b1_expected_files); i++)
            expected |= !strcmp(data.cFileName, b1_expected_files[i].name);
        if (!expected) {
            FindClose(find);
            b1_fail("Builder-safe directory contains an unexpected member");
        }
        members++;
    } while (FindNextFileA(find, &data));
    FindClose(find);
    if (members != q_countof(b1_expected_files))
        b1_fail("Builder-safe directory member count differs");
}

static void b1_require_empty_output(void)
{
    DWORD attributes;
    WIN32_FIND_DATAA data;
    HANDLE find;
    char pattern[MAX_OSPATH];
    char log_path[MAX_OSPATH];

    if (!b1_output->string[0])
        b1_fail("b1_output is required");
    attributes = GetFileAttributesA(b1_output->string);
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
        b1_fail("b1_output must be an existing regular directory");
    if (Q_snprintf(pattern, sizeof(pattern), "%s/*", b1_output->string) >= sizeof(pattern))
        b1_fail("b1_output path is too long");
    find = FindFirstFileA(pattern, &data);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, "..")) {
                FindClose(find);
                b1_fail("b1_output is not empty");
            }
        } while (FindNextFileA(find, &data));
        FindClose(find);
    }
    if (Q_snprintf(log_path, sizeof(log_path), "%s/adapter.jsonl", b1_output->string) >= sizeof(log_path))
        b1_fail("b1_output log path is too long");
    b1.log = Q_fopen(log_path, "wxb");
    if (!b1.log)
        b1_fail("cannot create B1 adapter log");
}

static bool b1_finite_array(const float *values, size_t count)
{
    for (size_t i = 0; i < count; i++)
        if (!isfinite(values[i]))
            return false;
    return true;
}

static void b1_validate_records(void)
{
    for (uint32_t i = 0; i < B1_RECORD_COUNT; i++) {
        const b1_plan_record_t *record = b1.records + i;
        if (record->logical_index != i || record->source_frame_id != B1_FIRST_SOURCE + i)
            b1_fail("plan record identity differs at logical %u", i);
        if ((i == 0 && record->relative_time_ns != 0) ||
            (i && record->relative_time_ns <= b1.records[i - 1].relative_time_ns) ||
            record->frame_duration_ns <= 0 || !isfinite(record->scene_time) ||
            record->scene_time < 0.f || record->rdflags != 0)
            b1_fail("plan shared timing/scene differs at logical %u", i);
        if (memcmp(record->gameplay_viewport, (int32_t[4]){ 0, 0, 1792, 720 }, sizeof(record->gameplay_viewport)))
            b1_fail("plan gameplay viewport differs at logical %u", i);
        for (int role = 0; role < B1_ROLE_COUNT; role++) {
            const b1_plan_view_t *view = record->views + role;
            if (view->acceptance != B1_ACCEPTANCE_REQUIRED ||
                memcmp(view->viewport, (int32_t[4]){ 0, 0, B1_WIDTH, B1_HEIGHT }, sizeof(view->viewport)) ||
                view->render_extent[0] != B1_WIDTH || view->render_extent[1] != B1_HEIGHT ||
                view->taa_extent[0] != B1_WIDTH || view->taa_extent[1] != B1_HEIGHT ||
                view->fov_xy[0] <= 0.f || view->fov_xy[0] >= 179.f ||
                view->fov_xy[1] <= 0.f || view->fov_xy[1] >= 179.f ||
                !b1_finite_array(view->view_origin, 3) ||
                !b1_finite_array(view->view_angles, 3) ||
                !b1_finite_array(view->fov_xy, 2) ||
                !b1_finite_array(view->V, 16) || !b1_finite_array(view->invV, 16) ||
                !b1_finite_array(view->P, 16) || !b1_finite_array(view->invP, 16))
                b1_fail("plan view differs at logical %u role %d", i, role);
        }
    }
}

static void b1_phase5brpb_require_digest(cvar_t *value, byte digest[32],
                                         const char *label)
{
    static const byte zero[32];
    if (!value || !b1_parse_sha256(value->string, digest) ||
        !memcmp(digest, zero, sizeof(zero)))
        b1_fail("Phase-5B-RP-B %s authority is malformed", label);
    value->modified = false;
}

static void b1_phase5brpb_require_empty_output(void)
{
    DWORD attributes;
    WIN32_FIND_DATAA data;
    HANDLE find;
    char canonical[MAX_OSPATH];
    char pattern[MAX_OSPATH];
    DWORD length;

    if (!b1_phase5brpb_output || !b1_phase5brpb_output->string[0] ||
        ((strlen(b1_phase5brpb_output->string) < 4u ||
          b1_phase5brpb_output->string[1] != ':' ||
          (b1_phase5brpb_output->string[2] != '\\' &&
           b1_phase5brpb_output->string[2] != '/')) &&
         strncmp(b1_phase5brpb_output->string, "\\\\", 2)))
        b1_fail("Phase-5B-RP-B output root must be absolute");
    length = GetFullPathNameA(b1_phase5brpb_output->string,
                              sizeof(canonical), canonical, NULL);
    attributes = length && length < sizeof(canonical) ?
        GetFileAttributesA(canonical) : INVALID_FILE_ATTRIBUTES;
    if (!length || length >= sizeof(canonical) ||
        attributes == INVALID_FILE_ATTRIBUTES ||
        !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
        b1_fail("Phase-5B-RP-B output root is not one regular directory");
    if (Q_snprintf(pattern, sizeof(pattern), "%s/*", canonical) >=
        sizeof(pattern))
        b1_fail("Phase-5B-RP-B output-root path is too long");
    find = FindFirstFileA(pattern, &data);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, "..")) {
                FindClose(find);
                b1_fail("Phase-5B-RP-B output root is not empty");
            }
        } while (FindNextFileA(find, &data));
        FindClose(find);
    }
    b1_phase5brpb_output->modified = false;
}

static void b1_phase5brpb_load_plan(void)
{
    b1_plan_header_t header;
    b1_plan_footer_t footer;
    byte actual[32], expected[32], settings_expected[32];
    byte records_actual[32], payload_expected[32];
    uint64_t plan_bytes = 0;
    uint64_t expected_plan_bytes;
    DWORD attributes;
    FILE *file;

    if (!b1_phase5brpb_plan || !b1_phase5brpb_plan->string[0] ||
        !b1_phase5brpb_plan_sha256 ||
        !b1_parse_sha256(b1_phase5brpb_plan_sha256->string, expected))
        b1_fail("Phase-5B-RP-B paired-plan authority is incomplete");
    attributes = GetFileAttributesA(b1_phase5brpb_plan->string);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
        !b1_sha256_file(b1_phase5brpb_plan->string, &plan_bytes, actual) ||
        memcmp(actual, expected, sizeof(actual)))
        b1_fail("Phase-5B-RP-B paired-plan identity differs");
    memcpy(b1.rpb.plan_sha256, actual, sizeof(actual));
    b1_phase5brpb_plan->modified = false;
    b1_phase5brpb_plan_sha256->modified = false;

    file = fopen(b1_phase5brpb_plan->string, "rb");
    if (!file || fread(&header, sizeof(header), 1, file) != 1)
        b1_fail("Phase-5B-RP-B paired-plan header is unreadable");
    if (memcmp(header.magic, "B1PLANV2", 8) ||
        !memchr(header.contract, 0, sizeof(header.contract)) ||
        !memchr(header.pinned_base, 0, sizeof(header.pinned_base)) ||
        !memchr(header.p1_foundation, 0, sizeof(header.p1_foundation)) ||
        strcmp(header.contract, B1_CONTRACT) ||
        strcmp(header.pinned_base, B1_PINNED_BASE) ||
        strcmp(header.p1_foundation, B1_P1_FOUNDATION) ||
        header.version != 2u || header.header_size != sizeof(header) ||
        header.settings_bytes != B1_SETTINGS_BYTES ||
        header.record_size != sizeof(b1_plan_record_t) ||
        header.record_count != B1_RECORD_COUNT ||
        header.footer_size != sizeof(footer) || header.first_logical != 0u ||
        header.last_logical != B1_RECORD_COUNT - 1u ||
        header.first_source != B1_FIRST_SOURCE ||
        header.last_source != B1_LAST_SOURCE || header.width != B1_WIDTH ||
        header.height != B1_HEIGHT || header.payload_bytes != B1_PAYLOAD_BYTES)
        b1_fail("Phase-5B-RP-B paired-plan header differs");
    if (!b1_parse_sha256(B1_PAYLOAD_SHA256, payload_expected) ||
        !b1_parse_sha256(B1_SETTINGS_SHA256, settings_expected) ||
        memcmp(header.payload_sha256, payload_expected, 32) ||
        memcmp(header.settings_sha256, settings_expected, 32))
        b1_fail("Phase-5B-RP-B paired-plan payload/settings identity differs");
    for (uint32_t index = 0; index < q_countof(b1_expected_files); index++) {
        byte member_digest[32];
        if (!b1_parse_sha256(b1_expected_files[index].sha256, member_digest) ||
            header.files[index].bytes != b1_expected_files[index].bytes ||
            memcmp(header.files[index].sha256, member_digest, 32))
            b1_fail("Phase-5B-RP-B paired-plan embedded input differs at %u",
                    index);
    }
    expected_plan_bytes = sizeof(header) + B1_SETTINGS_BYTES +
        (uint64_t)B1_RECORD_COUNT * sizeof(b1_plan_record_t) + sizeof(footer);
    if (plan_bytes != expected_plan_bytes)
        b1_fail("Phase-5B-RP-B paired-plan byte count differs");
    if (b1.records || b1.settings)
        b1_fail("Phase-5B-RP-B paired-plan storage was already populated");
    b1.settings = Z_Malloc(B1_SETTINGS_BYTES + 1u);
    if (fread(b1.settings, 1, B1_SETTINGS_BYTES, file) != B1_SETTINGS_BYTES)
        b1_fail("Phase-5B-RP-B settings serialization is truncated");
    b1.settings[B1_SETTINGS_BYTES] = 0;
    if (!b1_sha256_data(b1.settings, B1_SETTINGS_BYTES, records_actual) ||
        memcmp(records_actual, settings_expected, 32))
        b1_fail("Phase-5B-RP-B settings serialization differs");
    b1.records = Z_Malloc(sizeof(*b1.records) * B1_RECORD_COUNT);
    if (fread(b1.records, sizeof(*b1.records), B1_RECORD_COUNT, file) !=
        B1_RECORD_COUNT || fread(&footer, sizeof(footer), 1, file) != 1 ||
        fgetc(file) != EOF) {
        fclose(file);
        b1_fail("Phase-5B-RP-B paired-plan records/footer are truncated or extended");
    }
    fclose(file);
    if (memcmp(footer.magic, "B1FOOTV2", 8) ||
        footer.record_count != B1_RECORD_COUNT ||
        memcmp(footer.settings_sha256, settings_expected, 32) ||
        !b1_sha256_data(b1.records,
            sizeof(*b1.records) * B1_RECORD_COUNT, records_actual) ||
        memcmp(footer.records_sha256, records_actual, 32))
        b1_fail("Phase-5B-RP-B paired-plan footer/record identity differs");
    b1_validate_records();
    b1.rpb.plan_record0 = b1.records[0];
}

static void b1_phase5brpb_load_accepted_p0(void)
{
    uint64_t bytes = 0;
    byte actual[32];
    DWORD attributes;
    FILE *file;

    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0) {
        if ((b1_phase5brpb_p0 && b1_phase5brpb_p0->string[0]) ||
            (b1_phase5brpb_p0_sha256 && b1_phase5brpb_p0_sha256->string[0]))
            b1_fail("Phase-5B-RP-B P0 creation cannot consume an accepted P0");
        b1_phase5brpb_p0->modified = false;
        b1_phase5brpb_p0_sha256->modified = false;
        return;
    }
    if (!b1_phase5brpb_p0 || !b1_phase5brpb_p0->string[0])
        b1_fail("Phase-5B-RP-B enumeration requires the accepted P0 blob");
    attributes = GetFileAttributesA(b1_phase5brpb_p0->string);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)))
        b1_fail("Phase-5B-RP-B accepted P0 is not one regular file");
    b1_phase5brpb_require_digest(b1_phase5brpb_p0_sha256,
        b1.rpb.accepted_p0_sha256, "accepted-P0");
    if (!b1_sha256_file(b1_phase5brpb_p0->string, &bytes, actual) || !bytes ||
        bytes > B1_PHASE5A_MAX_CANONICAL_BYTES ||
        memcmp(actual, b1.rpb.accepted_p0_sha256, 32))
        b1_fail("Phase-5B-RP-B accepted P0 identity differs");
    b1.rpb.accepted_p0 = b1_phase5brpb_allocate((size_t)bytes, 1u,
                                                 "accepted-P0");
    file = fopen(b1_phase5brpb_p0->string, "rb");
    if (!file || fread(b1.rpb.accepted_p0, 1, (size_t)bytes, file) != bytes ||
        fgetc(file) != EOF) {
        if (file) fclose(file);
        b1_fail("Phase-5B-RP-B accepted P0 cannot be read exactly");
    }
    fclose(file);
    b1.rpb.accepted_p0_bytes = (size_t)bytes;
    b1_phase5brpb_p0->modified = false;
}

static void b1_phase5brpb_initialise(void)
{
    byte compiled_entity_sha256[32];
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_OFF)
        return;
    if (B0U_Requested() || b1_any_request_value() || b1.candidate.enabled ||
        b1.endpoint.enabled || b1.scene_controller.enabled)
        b1_fail("Phase-5B-RP-B cannot combine with B0/B1 historical modes");
    b1_phase5brpb_require_empty_output();
    b1_phase5brpb_load_plan();
    b1_phase5brpb_require_digest(b1_phase5brpb_executable_sha256,
        b1.rpb.executable_sha256, "executable");
    b1_phase5brpb_require_digest(b1_phase5brpb_runtime_manifest_sha256,
        b1.rpb.runtime_manifest_sha256, "runtime-manifest");
    b1_phase5brpb_require_digest(b1_phase5brpb_runtime_inventory_sha256,
        b1.rpb.runtime_inventory_sha256, "runtime-inventory");
    b1_phase5brpb_require_digest(b1_phase5brpb_runtime_data_sha256,
        b1.rpb.runtime_data_sha256, "runtime-data");
    b1_phase5brpb_require_digest(b1_phase5brpb_study_plan_sha256,
        b1.rpb.study_plan_sha256, "study-plan");
    b1_phase5brpb_require_digest(b1_phase5brpb_startup_sha256,
        b1.rpb.startup_sha256, "startup");
    if (strcmp(b1_phase5brpbr2_diagnostic->string, "0") &&
        strcmp(b1_phase5brpbr2_diagnostic->string, "1"))
        b1_fail("b1_phase5brpbr2_diagnostic must be exactly 0 or 1");
    b1.rpb.r2_entity_diagnostic =
        !strcmp(b1_phase5brpbr2_diagnostic->string, "1");
    if (b1.rpb.r2_entity_diagnostic) {
        if (b1.rpb.mode != B1_PHASE5BRPB_MODE_P0 ||
            !b1_phase5brpbr2_expected_entities->string[0] ||
            strlen(b1_phase5brpbr2_source_revision->string) != 40u ||
            strncmp(b1_phase5brpbr2_source_revision->string,
                    Q2RTX_UPSTREAM_REVISION, 8u))
            b1_fail("Phase-5B-RP-B-R2 diagnostic authority is incomplete");
        for (uint32_t index = 0; index < 40u; index++)
            if (!((b1_phase5brpbr2_source_revision->string[index] >= '0' &&
                   b1_phase5brpbr2_source_revision->string[index] <= '9') ||
                  (b1_phase5brpbr2_source_revision->string[index] >= 'a' &&
                   b1_phase5brpbr2_source_revision->string[index] <= 'f')))
                b1_fail("Phase-5B-RP-B-R2 full source revision is malformed");
        b1_phase5brpb_require_digest(
            b1_phase5brpbr2_expected_entities_sha256,
            b1.rpb.r2_expected_entities_sha256, "R2 expected entities");
        b1_phase5brpb_require_digest(
            b1_phase5brpbr2_producer_authority_sha256,
            b1.rpb.r2_producer_authority_sha256, "R2 producer authority");
        if (!b1_parse_sha256(B1_PHASE5BRPBR2_ENTITY_SHA256,
                             compiled_entity_sha256) ||
            memcmp(compiled_entity_sha256,
                   b1.rpb.r2_expected_entities_sha256, 32))
            b1_fail("Phase-5B-RP-B-R2 expected entity identity differs");
        b1_phase5brpbr2_expected_entities->modified = false;
        b1_phase5brpbr2_source_revision->modified = false;
    } else if (b1_phase5brpbr2_expected_entities->string[0] ||
               b1_phase5brpbr2_expected_entities_sha256->string[0] ||
               b1_phase5brpbr2_source_revision->string[0] ||
               b1_phase5brpbr2_producer_authority_sha256->string[0]) {
        b1_fail("Phase-5B-RP-B-R2 controls are partial while diagnostic is disabled");
    }
    b1_phase5brpbr2_diagnostic->modified = false;
    if (!b1_parse_sha256(B1_PHASE5A_FIXTURE_SHA256,
                         b1.rpb.fixture_sha256) ||
        !b1_parse_sha256(B1_PHASE5A_CLOSURE_SHA256,
                         b1.rpb.closure_sha256))
        b1_fail("Phase-5B-RP-B compiled fixture authority is malformed");
    b1_phase5brpb_load_accepted_p0();
    b1.rpb.enabled = true;
    b1_phase5brpb_allocate_arenas();
    b1.rpb.playback_state = B1_PHASE5BRPB_PLAYBACK_OFF;
    b1_phase5brpb_mode->modified = false;
}

static void b1_load_plan(void)
{
    b1_plan_header_t header;
    b1_plan_footer_t footer;
    b1_file_identity_t actual_files[6];
    byte plan_actual[32], plan_expected[32], settings_expected[32];
    byte payload_expected[32], records_actual[32], zero_authority[32] = { 0 };
    uint64_t plan_bytes = 0;
    uint64_t expected_plan_bytes;
    FILE *file;

    b1_set_state(B1_STATE_AUTHENTICATING);
    b1_require_empty_output();
    b1_validate_input_names();
    for (size_t i = 0; i < q_countof(b1_expected_files); i++)
        b1_authenticate_file(b1_expected_files + i, b1_input_cvars[i]->string, actual_files + i);
    b1_phase5br_input_closure_sha256(actual_files,
        b1.source_authority.input_closure_sha256);
    if (B0U_Requested())
        b1_fail("B0-U replay and B1 cannot be active together");
    if (!b1_parse_sha256(b1_plan_sha256->string, plan_expected) ||
        !b1_sha256_file(b1_plan->string, &plan_bytes, plan_actual) ||
        memcmp(plan_actual, plan_expected, 32))
        b1_fail("B1 plan identity differs");
    if (!b1_parse_sha256(b1_phase5br_executable_sha256->string,
                         b1.source_authority.executable_sha256) ||
        !b1_parse_sha256(b1_phase5br_runtime_manifest_sha256->string,
                         b1.source_authority.runtime_manifest_sha256) ||
        !b1_parse_sha256(b1_phase5br_runtime_inventory_sha256->string,
                         b1.source_authority.runtime_inventory_sha256) ||
        !memcmp(b1.source_authority.executable_sha256, zero_authority, 32) ||
        !memcmp(b1.source_authority.runtime_manifest_sha256, zero_authority, 32) ||
        !memcmp(b1.source_authority.runtime_inventory_sha256, zero_authority, 32))
        b1_fail("B1 Phase-5B-R executable/runtime authority is malformed");
    memcpy(b1.source_authority.plan_sha256, plan_actual, 32);
    b1.source_authority.logical_index = 0u;
    b1.source_authority.source_frame_id = B1_FIRST_SOURCE;
    b1.source_authority.scene_scope =
        B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_V1;
    b1.source_authority.scene_scope_generation =
        B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_GENERATION;
    b1.source_authority.effective_particle_count = 0u;
    b1_phase5br_executable_sha256->modified = false;
    b1_phase5br_runtime_manifest_sha256->modified = false;
    b1_phase5br_runtime_inventory_sha256->modified = false;
    b1_plan->modified = false;
    b1_plan_sha256->modified = false;

    file = fopen(b1_plan->string, "rb");
    if (!file)
        b1_fail("cannot open B1 plan");
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        b1_fail("truncated B1 plan header");
    }
    if (memcmp(header.magic, "B1PLANV2", 8) ||
        !memchr(header.contract, 0, sizeof(header.contract)) ||
        !memchr(header.pinned_base, 0, sizeof(header.pinned_base)) ||
        !memchr(header.p1_foundation, 0, sizeof(header.p1_foundation)) ||
        strcmp(header.contract, B1_CONTRACT) || strcmp(header.pinned_base, B1_PINNED_BASE) ||
        strcmp(header.p1_foundation, B1_P1_FOUNDATION) || header.version != 2 ||
        header.header_size != sizeof(header) || header.settings_bytes != B1_SETTINGS_BYTES ||
        header.record_size != sizeof(b1_plan_record_t) || header.record_count != B1_RECORD_COUNT ||
        header.footer_size != sizeof(footer) || header.first_logical != 0 ||
        header.last_logical != B1_RECORD_COUNT - 1 || header.first_source != B1_FIRST_SOURCE ||
        header.last_source != B1_LAST_SOURCE || header.width != B1_WIDTH || header.height != B1_HEIGHT ||
        header.payload_bytes != B1_PAYLOAD_BYTES)
        b1_fail("B1 plan header differs");
    b1_parse_sha256(B1_PAYLOAD_SHA256, payload_expected);
    b1_parse_sha256(B1_SETTINGS_SHA256, settings_expected);
    if (memcmp(header.payload_sha256, payload_expected, 32) ||
        memcmp(header.settings_sha256, settings_expected, 32) ||
        memcmp(header.files, actual_files, sizeof(actual_files)))
        b1_fail("B1 plan embedded input/settings identity differs");
    expected_plan_bytes = sizeof(header) + B1_SETTINGS_BYTES +
        (uint64_t)B1_RECORD_COUNT * sizeof(b1_plan_record_t) + sizeof(footer);
    if (plan_bytes != expected_plan_bytes)
        b1_fail("B1 plan byte count differs");

    b1.settings = Z_Malloc(B1_SETTINGS_BYTES + 1);
    if (fread(b1.settings, 1, B1_SETTINGS_BYTES, file) != B1_SETTINGS_BYTES)
        b1_fail("truncated B1 settings serialization");
    b1.settings[B1_SETTINGS_BYTES] = 0;
    if (!b1_sha256_data(b1.settings, B1_SETTINGS_BYTES, records_actual) ||
        memcmp(records_actual, settings_expected, 32))
        b1_fail("B1 settings serialization differs");
    b1.records = Z_Malloc(sizeof(*b1.records) * B1_RECORD_COUNT);
    if (fread(b1.records, sizeof(*b1.records), B1_RECORD_COUNT, file) != B1_RECORD_COUNT)
        b1_fail("truncated B1 plan records");
    if (fread(&footer, sizeof(footer), 1, file) != 1 || fgetc(file) != EOF) {
        fclose(file);
        b1_fail("B1 plan footer/trailing data differs");
    }
    fclose(file);
    if (memcmp(footer.magic, "B1FOOTV2", 8) || footer.record_count != B1_RECORD_COUNT ||
        memcmp(footer.settings_sha256, settings_expected, 32) ||
        !b1_sha256_data(b1.records, sizeof(*b1.records) * B1_RECORD_COUNT, records_actual) ||
        memcmp(footer.records_sha256, records_actual, 32))
        b1_fail("B1 plan records/footer identity differs");
    b1_validate_records();
    if (strcmp(b1_technical_smoke->string, "0") &&
        strcmp(b1_technical_smoke->string, "1"))
        b1_fail("b1_technical_smoke must be exactly 0 or 1");
    if (strcmp(b1_bootstrap_probe->string, "0") &&
        strcmp(b1_bootstrap_probe->string, "1"))
        b1_fail("b1_bootstrap_probe must be exactly 0 or 1");
    if (strcmp(b1_material_secondary_advance_tamper->string, "0") &&
        strcmp(b1_material_secondary_advance_tamper->string, "1"))
        b1_fail("b1_material_secondary_advance_tamper must be exactly 0 or 1");
    if (strcmp(b1_scene_admission_witness_authoritative->string, "0") &&
        strcmp(b1_scene_admission_witness_authoritative->string, "1"))
        b1_fail("b1_scene_admission_witness_authoritative must be exactly 0 or 1");
    b1.technical_process_kind = b1_parse_technical_process_kind(
        b1_technical_process_kind->string, &b1.technical_process_numeric,
        &b1.technical_process_token);
    b1.technical_process_identity_hash = b1_technical_process_hash64(
        (uint32_t)b1.technical_process_kind, b1.technical_process_token);
    b1.capture_scale_pair_count = b1_parse_capture_scale_pair_count(
        b1_capture_scale_pair_count->string);
    b1.capture_scale_identity_hash = b1_capture_scale_hash64(
        (uint32_t)b1.technical_process_kind, b1.capture_scale_pair_count);
    if ((b1.technical_process_kind ==
            B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1) !=
        (b1.capture_scale_pair_count != 0))
        b1_fail("b1_capture_scale_pair_count is active for exactly CAPTURE_SCALE_V1");
    if (!b1_technical_process_early_latched ||
        b1_technical_process_kind != b1_technical_process_kind_early ||
        b1.technical_process_kind != b1_technical_process_kind_early_value ||
        strcmp(b1.technical_process_numeric,
            b1_technical_process_numeric_early) ||
        strcmp(b1.technical_process_token, b1_technical_process_token_early) ||
        b1.technical_process_identity_hash !=
            b1_technical_process_identity_hash_early ||
        b1_capture_scale_pair_count != b1_capture_scale_pair_count_early ||
        b1.capture_scale_pair_count !=
            b1_capture_scale_pair_count_early_value ||
        b1.capture_scale_identity_hash !=
            b1_capture_scale_identity_hash_early)
        b1_fail("authenticated client technical-process/scale latch differs from its early latch");
    b1.validation_smoke = b1_technical_smoke->integer == 1;
    b1.bootstrap_probe = b1_bootstrap_probe->integer == 1;
    b1.material_secondary_advance_tamper =
        b1_material_secondary_advance_tamper->integer == 1;
    b1.scene_admission_witness_authoritative =
        b1_scene_admission_witness_authoritative->integer == 1;
    if (b1_technical_process_active() && (b1.validation_smoke ||
        b1.bootstrap_probe || b1.material_secondary_advance_tamper))
        b1_fail("b1_technical_process_kind cannot combine with a legacy technical control");
    if (b1_technical_process_active()) {
        b1_technical_smoke->modified = false;
        b1_bootstrap_probe->modified = false;
        b1_material_secondary_advance_tamper->modified = false;
    }
    if (b1.technical_process_kind ==
            B1_TECHNICAL_PROCESS_STATIC_CORE_SCENE_ADMISSION_WITNESS)
        b1_scene_admission_witness_authoritative->modified = false;
    else {
        if (strcmp(b1_scene_admission_witness_authoritative->string, "0"))
            b1_fail("scene-admission witness classification used with another process kind");
        b1_scene_admission_witness_authoritative->modified = false;
    }
    if (b1.bootstrap_probe && !b1.validation_smoke)
        b1_fail("b1_bootstrap_probe requires the validation-layer technical smoke boundary");
    if (b1.material_secondary_advance_tamper && !b1.validation_smoke)
        b1_fail("secondary material-advance tamper requires the technical smoke boundary");
    if (b1.material_secondary_advance_tamper && b1.bootstrap_probe)
        b1_fail("secondary material-advance tamper is incompatible with bootstrap-only mode");
    b1_technical_process_kind->modified = false;
    b1_capture_scale_pair_count->modified = false;
    b1.logical_index = 0;
    b1.active_role = B1_ROLE_PRIMARY_LEFT;
    b1.role_physical_slot[0] = UINT32_MAX;
    b1.role_physical_slot[1] = UINT32_MAX;
    b1_set_state(B1_STATE_READY);
    fprintf(b1.log,
            "{\"event\":\"authenticated\",\"contract\":\"%s\",\"records\":%u,"
            "\"settingsBytes\":%u,\"validationSmoke\":%s,\"bootstrapProbe\":%s,"
            "\"materialSecondaryAdvanceTamper\":%s,"
            "\"technicalProcessKind\":%u,\"technicalProcessNumeric\":\"%s\","
            "\"technicalProcessToken\":\"%s\","
            "\"technicalProcessIdentityHash64\":\"%016llx\","
            "\"classification\":\"%s\"}\n",
            B1_CONTRACT, B1_RECORD_COUNT, B1_SETTINGS_BYTES,
            b1.validation_smoke ? "true" : "false",
            b1.bootstrap_probe ? "true" : "false",
            b1.material_secondary_advance_tamper ? "true" : "false",
            (unsigned)b1.technical_process_kind, b1.technical_process_numeric,
            b1.technical_process_token,
            (unsigned long long)b1.technical_process_identity_hash,
            b1.technical_process_kind ==
                B1_TECHNICAL_PROCESS_STATIC_CORE_SCENE_ADMISSION_WITNESS ?
                (b1.scene_admission_witness_authoritative ?
                    "AUTHORITATIVE_R6R1_SCENE_BOOTSTRAP_WITNESS" :
                    "NON_AUTHORITATIVE_R6R1_REPLACEMENT_REHEARSAL") :
            b1.bootstrap_probe ? "B1_BOOTSTRAP_DIAGNOSTIC_ONLY" :
            b1.validation_smoke ? "VALIDATION_LAYER_SMOKE_TECHNICAL_ONLY" :
            b1_technical_process_active() ? "B1_WAVE2_TECHNICAL_WITNESS_ONLY" :
                "NON-AUTHORITATIVE_TECHNICAL_VALIDATION_RENDER");
    if (b1.technical_process_kind == B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1)
        fprintf(b1.log,
            "{\"event\":\"b1_capture_scale_latch_v1\","
            "\"schemaVersion\":1,\"domain\":\"B1-CAPTURE-SCALE-EXECUTION-V1\","
            "\"technicalProcessKind\":7,\"technicalProcessToken\":\"CAPTURE_SCALE_V1\","
            "\"pairCount\":%u,\"executionIdentityHash64\":\"%016llx\","
            "\"clientLatched\":true,\"rendererMatched\":false}\n",
            b1.capture_scale_pair_count,
            (unsigned long long)b1.capture_scale_identity_hash);
    fprintf(b1.log,
            "{\"event\":\"b1_scene_source_pending\","
            "\"controller\":\"OPTION_A_NATIVE_DM2\",\"runtimeEnabled\":false,"
            "\"classification\":\"PRE_CURATED_FOUNDATION_ONLY\"}\n");
    fflush(b1.log);
}

static bool b1_any_request_value(void)
{
    return b1_plan->string[0] || b1_plan_sha256->string[0] ||
        b1_stimulus->string[0] || b1_manifest->string[0] ||
        b1_verification->string[0] || b1_sidecar->string[0] ||
        b1_pipeline_tests->string[0] || b1_acceptance->string[0] ||
        b1_output->string[0] || b1_technical_smoke->integer ||
        strcmp(b1_technical_process_kind->string, "0") ||
        strcmp(b1_capture_scale_pair_count->string, "0") ||
        strcmp(b1_scene_admission_witness_authoritative->string, "0") ||
        b1_bootstrap_probe->integer || b1_material_secondary_advance_tamper->integer ||
        (b1_scene_controller && b1_scene_controller->integer) ||
        (b1_phase5br_executable_sha256 &&
         b1_phase5br_executable_sha256->string[0]) ||
        (b1_phase5br_runtime_manifest_sha256 &&
         b1_phase5br_runtime_manifest_sha256->string[0]) ||
        (b1_phase5br_runtime_inventory_sha256 &&
         b1_phase5br_runtime_inventory_sha256->string[0]);
}

static void b1_seed_feedback(ref_feedback_t *feedback)
{
    memset(feedback, 0, sizeof(*feedback));
    feedback->viewcluster = -1; // renderer replaces this with pair-zero current leaf
    feedback->lookatcluster = -1;
    feedback->num_light_polys = 0;
    feedback->resolution_scale = 100;
    feedback->view_material_index = -1;
    Q_strlcpy(feedback->view_material, "<unknown>", sizeof(feedback->view_material));
    Q_strlcpy(feedback->view_material_override, "<unknown>", sizeof(feedback->view_material_override));
    feedback->adapted_luminance = 0.005f;
}

static void b1_renderer_log_line(const char *line, void *opaque)
{
    FILE *log = opaque;

    if (!log || !line || fprintf(log, "%s\n", line) < 0 || fflush(log))
        b1_fail("cannot persist renderer temporal audit");
}

static void b1_freeze_scene(refdef_t *fd)
{
    char scene_text[65], left_text[65], right_text[65];
    char raw_particle_text[65];
    cvar_t *particle_size;

    b1_scene_controller_require_controls();
    b1_scene_controller_monitor_endpoint("client-scene-freeze-entry", true);
    if (!b1.scene_controller.endpoint_closed ||
        !b1.scene_controller.scene_build_consumed || b1.scene_source_ready ||
        b1.scene_controller.client_scene_closed ||
        b1.scene_lifecycle.state != B1_SCENE_ENDPOINT_ACCEPTED)
        b1_fail("B1 client-scene freeze entered from an illegal controller state");
    if (strcmp(cl.mapname, "base1"))
        b1_fail("expected base1 before B1 scene freeze");
    if (r_config.width != B1_WIDTH || r_config.height != B1_HEIGHT)
        b1_fail("B1 output geometry is not 1948x1563");
    if (fd->num_entities < 0 || fd->num_entities > MAX_ENTITIES ||
        fd->num_particles < 0 || fd->num_particles > MAX_PARTICLES ||
        fd->num_dlights < 0 || fd->num_dlights > MAX_DLIGHTS)
        b1_fail("B1 frozen scene count is outside upstream bounds");
    B1_VerifySettings();

    b1.frozen_refdef = *fd;
    if (fd->num_entities) {
        b1.entities = b1_copy_required(fd->entities,
            sizeof(*fd->entities) * (size_t)fd->num_entities, "entities");
        b1.frozen_refdef.entities = b1.entities;
    }
    if (fd->num_particles) {
        b1.particles = b1_copy_required(fd->particles,
            sizeof(*fd->particles) * (size_t)fd->num_particles, "particles");
        b1.frozen_refdef.particles = b1.particles;
    }
    particle_size = Cvar_FindVar("pt_particle_size");
    if (!particle_size || !isfinite(particle_size->value) ||
        particle_size->value <= 0.0f)
        b1_fail("B1 static-core particle-size telemetry input is malformed");
    b1.raw_particle_count = (uint32_t)fd->num_particles;
    b1.raw_particle_default_size = particle_size->value;
    b1_static_core_raw_particle_telemetry(
        b1.particles, b1.raw_particle_count, b1.raw_particle_default_size,
        b1.raw_particle_sha256, &b1.raw_particle_canonical_bytes);
    if (fd->num_dlights) {
        b1.dlights = b1_copy_required(fd->dlights,
            sizeof(*fd->dlights) * (size_t)fd->num_dlights, "dynamic lights");
        b1.frozen_refdef.dlights = b1.dlights;
    }
    b1.lightstyles = b1_copy_required(fd->lightstyles,
        sizeof(*fd->lightstyles) * MAX_LIGHTSTYLES, "light styles");
    b1.frozen_refdef.lightstyles = b1.lightstyles;
    b1.areabytes = cl.frame.areabytes;
    if (b1.areabytes) {
        b1.areabits = b1_copy_required(fd->areabits, (size_t)b1.areabytes, "area bits");
        b1.frozen_refdef.areabits = b1.areabits;
    } else {
        b1.frozen_refdef.areabits = NULL;
    }
    b1_seed_feedback(&b1.frozen_refdef.feedback);
    b1.frozen_refdef.b1_active = false;
    b1.frozen_refdef.b1_bootstrap_probe = false;
    b1.frozen_refdef.b1_material_secondary_advance_tamper = false;
    b1.frozen_refdef.b1_temporal_history_valid = false;
    b1.frozen_refdef.b1_render_accepted = false;
    b1.frozen_refdef.b1_logical_index = 0;
    b1.frozen_refdef.b1_source_frame_id = 0;
    b1.frozen_refdef.b1_role = 0;
    b1.frozen_refdef.b1_phase = B1_RENDERER_PRIMARY_UPDATE;
    b1.frozen_refdef.b1_scene_scope =
        B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_V1;
    b1.frozen_refdef.b1_scene_scope_generation =
        B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_GENERATION;
    b1.frozen_refdef.b1_effective_particle_count = 0u;
    b1.frozen_refdef.b1_logical_duration = 0.f;
    b1.frozen_refdef.b1_physical_frame = 0;
    b1.frozen_refdef.b1_physical_slot = 0;
    memset(b1.frozen_refdef.b1_V, 0, sizeof(b1.frozen_refdef.b1_V));
    memset(b1.frozen_refdef.b1_invV, 0, sizeof(b1.frozen_refdef.b1_invV));
    memset(b1.frozen_refdef.b1_P, 0, sizeof(b1.frozen_refdef.b1_P));
    memset(b1.frozen_refdef.b1_invP, 0, sizeof(b1.frozen_refdef.b1_invP));
    memset(b1.frozen_refdef.b1_V_prev, 0, sizeof(b1.frozen_refdef.b1_V_prev));
    memset(b1.frozen_refdef.b1_P_prev, 0, sizeof(b1.frozen_refdef.b1_P_prev));
    memset(b1.frozen_refdef.b1_invP_prev, 0, sizeof(b1.frozen_refdef.b1_invP_prev));
    memset(b1.frozen_refdef.b1_projection_fov_scale, 0,
           sizeof(b1.frozen_refdef.b1_projection_fov_scale));
    memset(b1.frozen_refdef.b1_projection_fov_scale_prev, 0,
           sizeof(b1.frozen_refdef.b1_projection_fov_scale_prev));
    b1.frozen_refdef.b1_cylindrical_hfov = 0.f;
    b1.frozen_refdef.b1_cylindrical_hfov_prev = 0.f;
    memset(b1.frozen_refdef.b1_render_extent_prev, 0,
           sizeof(b1.frozen_refdef.b1_render_extent_prev));
    memset(b1.frozen_refdef.b1_taa_extent_prev, 0,
           sizeof(b1.frozen_refdef.b1_taa_extent_prev));
    b1.frozen_refdef.b1_gpu_slice_width_prev = 0;
    memset(b1.frozen_refdef.b1_pair_V, 0, sizeof(b1.frozen_refdef.b1_pair_V));
    memset(b1.frozen_refdef.b1_pair_invV, 0, sizeof(b1.frozen_refdef.b1_pair_invV));
    memset(b1.frozen_refdef.b1_pair_fov_xy, 0, sizeof(b1.frozen_refdef.b1_pair_fov_xy));
    for (int role = 0; role < B1_ROLE_COUNT; role++) {
        b1.roles[role].committed = b1.records[0].views[role];
        b1.roles[role].committed_logical = UINT32_MAX;
        b1.roles[role].temporal_valid = false;
        b1.roles[role].pending_valid = false;
        b1.frozen_roles[role] = b1.roles[role];
    }
    b1_scene_sha256(b1.scene_sha256, b1.scene_component_sha256,
                    b1.scene_component_bytes, &b1.scene_canonical_bytes);
    memcpy(b1.source_authority.client_scene_sha256, b1.scene_sha256, 32);
    if (b1.bootstrap_probe)
        b1_log_scene_storage();
    b1_format_sha256(b1.scene_sha256, scene_text);
    b1_format_sha256(b1.raw_particle_sha256, raw_particle_text);
    b1_format_sha256(b1.scene_component_sha256[7], left_text);
    b1_format_sha256(b1.scene_component_sha256[8], right_text);
    b1.settings_monitoring = true;
    b1.frozen = true;
    b1.scene_controller.client_scene_closed = true;
    b1_phase5brpbr6r4_close_normal_entity_authority(false);
    /* The client-owned closure is necessary but not sufficient.  Renderer-
     * derived ModelInstance/IQM/TLAS/light state must close in a separately
     * authorised preactivation lifecycle before this flag may become true. */
    b1.scene_source_ready = false;
    fprintf(b1.log,
            "{\"event\":\"b1_static_core_raw_particle_telemetry\","
            "\"schema\":\"%s\",\"sceneScope\":\"%s\","
            "\"observedCount\":%u,\"canonicalBytes\":%zu,"
            "\"sha256\":\"%s\",\"rendererAuthority\":false,"
            "\"selectionInput\":false,\"ordinaryProducerUnchanged\":true,"
            "\"pointersExcluded\":true,\"paddingExcluded\":true,"
            "\"inactiveCapacityExcluded\":true}\n",
            B1_STATIC_CORE_RAW_PARTICLE_SCHEMA, B1_STATIC_CORE_SCOPE_TOKEN,
            b1.raw_particle_count, b1.raw_particle_canonical_bytes,
            raw_particle_text);
    for (uint32_t index = 0; index < B1_PHASE5B_SCENE_COMPONENT_COUNT; index++) {
        char component_text[65];
        b1_format_sha256(b1.scene_component_sha256[index], component_text);
        fprintf(b1.log,
                "{\"event\":\"b1_client_scene_component\","
                "\"schema\":\"%s\",\"index\":%u,\"name\":\"%s\","
                "\"canonicalBytes\":%u,\"sha256\":\"%s\","
                "\"activeFieldsOnly\":true,\"pointersExcluded\":true,"
                "\"paddingExcluded\":true,\"inactiveCapacityExcluded\":true}\n",
                B1_PHASE5B_SCENE_SCHEMA, index, b1_scene_component_name[index],
                b1.scene_component_bytes[index], component_text);
    }
    fprintf(b1.log,
            "{\"event\":\"scene_freeze\",\"logicalIndex\":0,"
            "\"ordinaryFeedbackConsumed\":false,\"adaptedLuminance\":0.005,"
            "\"rolesTemporalValid\":[false,false],\"sceneSha256\":\"%s\","
            "\"roleContextSha256\":[\"%s\",\"%s\"]}\n",
            scene_text, left_text, right_text);
    fprintf(b1.log,
            "{\"event\":\"b1_client_scene_closure\","
            "\"schema\":\"%s\",\"sceneSha256\":\"%s\","
            "\"canonicalBytes\":%zu,\"componentCount\":%u,"
            "\"endpointSha256\":\"%s\",\"rangeSetSha256\":\"%s\","
            "\"ordinarySceneBuildCount\":1,\"clientSceneClosed\":true,"
             "\"sceneSourceReady\":false,\"laterInputParsed\":false,"
             "\"laterInterpolationApplied\":false,"
             "\"rendererDerivedClosure\":\"DEFERRED_NORMAL_PAIR_ZERO_PREPARATION\","
             "\"temporalActivated\":false,\"clientClosurePass\":true,"
             "\"completeSceneClosurePass\":false,\"pass\":true}\n",
            B1_PHASE5B_SCENE_SCHEMA, scene_text, b1.scene_canonical_bytes,
            B1_PHASE5B_SCENE_COMPONENT_COUNT,
            B1_PHASE5AR5_ACCEPTED_ENDPOINT_SHA256,
            B1_PHASE5AR5_ACCEPTED_RANGE_SET_SHA256);
    if (fflush(b1.log))
        b1_fail("cannot persist B1 client-scene closure");
    b1_phase5br_transition(B1_SCENE_CLIENT_SCENE_FROZEN);
    b1_scene_controller_monitor_endpoint("client-scene-closure", true);
}

static void b1_assert_scene_unchanged(const char *boundary)
{
    byte digest[32];
    byte component_digest[B1_PHASE5B_SCENE_COMPONENT_COUNT][32];
    uint32_t component_bytes[B1_PHASE5B_SCENE_COMPONENT_COUNT];
    size_t canonical_bytes = 0;
    char digest_text[65];
    byte raw_particle_digest[32];
    size_t raw_particle_bytes = 0;

    b1_scene_controller_monitor_endpoint(boundary, true);
    b1_scene_sha256(digest, component_digest, component_bytes, &canonical_bytes);
    b1_static_core_raw_particle_telemetry(
        b1.particles, b1.raw_particle_count, b1.raw_particle_default_size,
        raw_particle_digest, &raw_particle_bytes);
    if (memcmp(digest, b1.scene_sha256, sizeof(digest)) ||
        memcmp(component_digest, b1.scene_component_sha256,
               sizeof(component_digest)) ||
        memcmp(component_bytes, b1.scene_component_bytes,
               sizeof(component_bytes)) ||
        canonical_bytes != b1.scene_canonical_bytes ||
        memcmp(raw_particle_digest, b1.raw_particle_sha256, 32) ||
        raw_particle_bytes != b1.raw_particle_canonical_bytes)
        b1_fail("B1 frozen scene mutated at %s", boundary);
    b1_format_sha256(digest, digest_text);
    fprintf(b1.log,
            "{\"event\":\"b1_scene_freeze_unchanged\","
            "\"boundary\":\"%s\",\"sceneSha256\":\"%s\","
            "\"rendererSceneMutation\":false}\n",
            boundary, digest_text);
    if (fflush(b1.log))
        b1_fail("cannot persist B1 scene-freeze proof");
}

static void b1_apply_role(refdef_t *fd, const b1_plan_record_t *record, b1_role_t role)
{
    const b1_plan_view_t *current = &record->views[role];
    const b1_plan_view_t *previous = &b1.roles[role].committed;

    *fd = b1.frozen_refdef;
    if (fd->b1_scene_scope !=
            B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_V1 ||
        fd->b1_scene_scope_generation !=
            B1_SCENE_SCOPE_STATIC_ENVIRONMENT_PARTICLE_EXCLUDED_GENERATION ||
        fd->b1_effective_particle_count != 0u)
        b1_fail("B1 static-core role authority is malformed");
    fd->particles = NULL;
    fd->num_particles = 0;
    fd->x = current->viewport[0];
    fd->y = current->viewport[1];
    fd->width = current->viewport[2];
    fd->height = current->viewport[3];
    fd->fov_x = current->fov_xy[0];
    fd->fov_y = current->fov_xy[1];
    VectorCopy(current->view_origin, fd->vieworg);
    VectorCopy(current->view_angles, fd->viewangles);
    fd->time = record->scene_time;
    fd->rdflags = record->rdflags;
    fd->b1_active = true;
    fd->b1_bootstrap_probe = b1.bootstrap_probe;
    fd->b1_material_secondary_advance_tamper =
        b1.material_secondary_advance_tamper;
    fd->b1_temporal_history_valid = b1.roles[role].temporal_valid;
    fd->b1_render_accepted = false;
    fd->b1_capture_requested = false;
    fd->b1_capture_request_schema_version = 0;
    fd->b1_capture_request_serial = 0;
    fd->b1_logical_index = record->logical_index;
    fd->b1_source_frame_id = record->source_frame_id;
    fd->b1_role = (uint32_t)role;
    fd->b1_phase = role == B1_ROLE_PRIMARY_LEFT ?
        B1_RENDERER_PRIMARY_UPDATE : B1_RENDERER_SECONDARY_REUSE;
    fd->b1_logical_duration = (float)((double)record->frame_duration_ns * 1e-9);
    memcpy(fd->b1_V, current->V, sizeof(fd->b1_V));
    memcpy(fd->b1_invV, current->invV, sizeof(fd->b1_invV));
    memcpy(fd->b1_P, current->P, sizeof(fd->b1_P));
    memcpy(fd->b1_invP, current->invP, sizeof(fd->b1_invP));
    memcpy(fd->b1_V_prev, previous->V, sizeof(fd->b1_V_prev));
    memcpy(fd->b1_P_prev, previous->P, sizeof(fd->b1_P_prev));
    memcpy(fd->b1_invP_prev, previous->invP, sizeof(fd->b1_invP_prev));
    memcpy(fd->b1_projection_fov_scale, current->projection_fov_scale,
           sizeof(fd->b1_projection_fov_scale));
    memcpy(fd->b1_projection_fov_scale_prev, previous->projection_fov_scale,
           sizeof(fd->b1_projection_fov_scale_prev));
    fd->b1_cylindrical_hfov = current->cylindrical_hfov;
    fd->b1_cylindrical_hfov_prev = previous->cylindrical_hfov;
    memcpy(fd->b1_render_extent_prev, previous->render_extent,
           sizeof(fd->b1_render_extent_prev));
    memcpy(fd->b1_taa_extent_prev, previous->taa_extent,
           sizeof(fd->b1_taa_extent_prev));
    fd->b1_gpu_slice_width_prev = previous->render_extent[0];
    for (int pair_role = 0; pair_role < B1_ROLE_COUNT; pair_role++) {
        memcpy(fd->b1_pair_V[pair_role], record->views[pair_role].V,
               sizeof(fd->b1_pair_V[pair_role]));
        memcpy(fd->b1_pair_invV[pair_role], record->views[pair_role].invV,
               sizeof(fd->b1_pair_invV[pair_role]));
        memcpy(fd->b1_pair_fov_xy[pair_role], record->views[pair_role].fov_xy,
               sizeof(fd->b1_pair_fov_xy[pair_role]));
    }
    b1.roles[role].pending = *current;
    b1.roles[role].pending_valid = true;
}

void B1_Init(void)
{
    const int private_flags = CVAR_PRIVATE | CVAR_NOARCHIVE;

    B1_PrivateTechnicalProcessPreinitialise();
    memset(&b1, 0, sizeof(b1));
    Cmd_Register(b1_commands);
    b1_plan = Cvar_Get("b1_plan", "", private_flags);
    b1_plan_sha256 = Cvar_Get("b1_plan_sha256", "", private_flags);
    b1_stimulus = Cvar_Get("b1_stimulus", "", private_flags);
    b1_manifest = Cvar_Get("b1_manifest", "", private_flags);
    b1_verification = Cvar_Get("b1_verification", "", private_flags);
    b1_sidecar = Cvar_Get("b1_sidecar", "", private_flags);
    b1_pipeline_tests = Cvar_Get("b1_pipeline_tests", "", private_flags);
    b1_acceptance = Cvar_Get("b1_acceptance", "", private_flags);
    b1_output = Cvar_Get("b1_output", "", private_flags);
    b1_technical_smoke = Cvar_Get("b1_technical_smoke", "0", private_flags);
    b1_technical_process_kind = Cvar_Get(
        "b1_technical_process_kind", "0", private_flags);
    b1_capture_scale_pair_count = Cvar_Get(
        "b1_capture_scale_pair_count", "0", private_flags);
    b1_scene_admission_witness_authoritative = Cvar_Get(
        "b1_scene_admission_witness_authoritative", "0", private_flags);
    if (b1_technical_process_kind != b1_technical_process_kind_early ||
        b1_technical_process_kind->modified ||
        strcmp(b1_technical_process_kind->string,
            b1_technical_process_numeric_early) ||
        b1_capture_scale_pair_count != b1_capture_scale_pair_count_early ||
        b1_capture_scale_pair_count->modified ||
        b1_parse_capture_scale_pair_count(
            b1_capture_scale_pair_count->string) !=
            b1_capture_scale_pair_count_early_value)
        b1_fail("B1 initialisation differs from the early client technical-process/scale latch");
    b1_bootstrap_probe = Cvar_Get("b1_bootstrap_probe", "0", private_flags);
    b1_material_secondary_advance_tamper = Cvar_Get(
        "b1_material_secondary_advance_tamper", "0", private_flags);
    b1_candidate_inventory = Cvar_Get(
        "b1_candidate_inventory", "0", private_flags);
    b1_candidate_inventory_output = Cvar_Get(
        "b1_candidate_inventory_output", "", private_flags);
    b1_endpoint_identity = Cvar_Get(
        "b1_endpoint_identity", "0", private_flags);
    b1_endpoint_identity_output = Cvar_Get(
        "b1_endpoint_identity_output", "", private_flags);
    b1_scene_controller = Cvar_Get(
        "b1_scene_controller", "0", private_flags);
    b1_phase5brpb_mode = Cvar_Get(
        "b1_phase5brpb_mode", "0", private_flags);
    b1_phase5brpb_output = Cvar_Get(
        "b1_phase5brpb_output", "", private_flags);
    b1_phase5brpb_plan = Cvar_Get(
        "b1_phase5brpb_plan", "", private_flags);
    b1_phase5brpb_plan_sha256 = Cvar_Get(
        "b1_phase5brpb_plan_sha256", "", private_flags);
    b1_phase5brpb_p0 = Cvar_Get(
        "b1_phase5brpb_p0", "", private_flags);
    b1_phase5brpb_p0_sha256 = Cvar_Get(
        "b1_phase5brpb_p0_sha256", "", private_flags);
    b1_phase5brpb_executable_sha256 = Cvar_Get(
        "b1_phase5brpb_executable_sha256", "", private_flags);
    b1_phase5brpb_runtime_manifest_sha256 = Cvar_Get(
        "b1_phase5brpb_runtime_manifest_sha256", "", private_flags);
    b1_phase5brpb_runtime_inventory_sha256 = Cvar_Get(
        "b1_phase5brpb_runtime_inventory_sha256", "", private_flags);
    b1_phase5brpb_runtime_data_sha256 = Cvar_Get(
        "b1_phase5brpb_runtime_data_sha256", "", private_flags);
    b1_phase5brpb_study_plan_sha256 = Cvar_Get(
        "b1_phase5brpb_study_plan_sha256", "", private_flags);
    b1_phase5brpb_startup_sha256 = Cvar_Get(
        "b1_phase5brpb_startup_sha256", "", private_flags);
    b1_phase5brpbr2_diagnostic = Cvar_Get(
        "b1_phase5brpbr2_diagnostic", "0", private_flags);
    b1_phase5brpbr2_expected_entities = Cvar_Get(
        "b1_phase5brpbr2_expected_entities", "", private_flags);
    b1_phase5brpbr2_expected_entities_sha256 = Cvar_Get(
        "b1_phase5brpbr2_expected_entities_sha256", "", private_flags);
    b1_phase5brpbr2_source_revision = Cvar_Get(
        "b1_phase5brpbr2_source_revision", "", private_flags);
    b1_phase5brpbr2_producer_authority_sha256 = Cvar_Get(
        "b1_phase5brpbr2_producer_authority_sha256", "", private_flags);
    b1_phase5br_executable_sha256 = Cvar_Get(
        "b1_phase5br_executable_sha256", "", private_flags);
    b1_phase5br_runtime_manifest_sha256 = Cvar_Get(
        "b1_phase5br_runtime_manifest_sha256", "", private_flags);
    b1_phase5br_runtime_inventory_sha256 = Cvar_Get(
        "b1_phase5br_runtime_inventory_sha256", "", private_flags);
    b1_status = Cvar_Get("b1_status", "DISABLED", private_flags | CVAR_ROM);
    b1_input_cvars[0] = b1_stimulus;
    b1_input_cvars[1] = b1_manifest;
    b1_input_cvars[2] = b1_verification;
    b1_input_cvars[3] = b1_sidecar;
    b1_input_cvars[4] = b1_pipeline_tests;
    b1_input_cvars[5] = b1_acceptance;
    if (strcmp(b1_candidate_inventory->string, "0") &&
        strcmp(b1_candidate_inventory->string, "1"))
        b1_fail("b1_candidate_inventory must be exactly 0 or 1");
    b1.candidate.enabled = !strcmp(b1_candidate_inventory->string, "1");
    if (b1.candidate.enabled) {
        if (!b1_candidate_inventory_output->string[0] || b1_any_request_value())
            b1_fail("Phase-5A candidate inventory cannot combine with B1 activation");
        b1_candidate_inventory->modified = false;
        b1_candidate_inventory_output->modified = false;
    } else if (b1_candidate_inventory_output->string[0]) {
        b1_fail("candidate inventory output is set while candidate mode is disabled");
    }
    if (strcmp(b1_endpoint_identity->string, "0") &&
        strcmp(b1_endpoint_identity->string, "1"))
        b1_fail("b1_endpoint_identity must be exactly 0 or 1");
    b1.endpoint.enabled = !strcmp(b1_endpoint_identity->string, "1");
    if (b1.endpoint.enabled) {
        if (!b1_endpoint_identity_output->string[0] || b1_any_request_value() ||
            b1.candidate.enabled)
            b1_fail("Phase-5A-R5 endpoint identity cannot combine with B1 or V1 candidate activation");
        b1_endpoint_identity->modified = false;
        b1_endpoint_identity_output->modified = false;
    } else if (b1_endpoint_identity_output->string[0]) {
        b1_fail("endpoint identity output is set while endpoint mode is disabled");
    }
    if (strcmp(b1_scene_controller->string, "0") &&
        strcmp(b1_scene_controller->string, "1"))
        b1_fail("b1_scene_controller must be exactly 0 or 1");
    b1.scene_controller.enabled = !strcmp(b1_scene_controller->string, "1");
    if (b1.scene_controller.enabled) {
        if (!b1_plan->string[0] || b1.candidate.enabled || b1.endpoint.enabled)
            b1_fail("B1 scene controller requires exclusive full B1 activation");
        b1_scene_controller->modified = false;
    }
    b1.rpb.mode = b1_phase5brpb_parse_mode(b1_phase5brpb_mode->string);
    if (b1.rpb.mode == B1_PHASE5BRPB_MODE_OFF) {
        if (b1_phase5brpb_output->string[0] || b1_phase5brpb_plan->string[0] ||
            b1_phase5brpb_plan_sha256->string[0] ||
            b1_phase5brpb_p0->string[0] ||
            b1_phase5brpb_p0_sha256->string[0] ||
            b1_phase5brpb_executable_sha256->string[0] ||
            b1_phase5brpb_runtime_manifest_sha256->string[0] ||
            b1_phase5brpb_runtime_inventory_sha256->string[0] ||
             b1_phase5brpb_runtime_data_sha256->string[0] ||
             b1_phase5brpb_study_plan_sha256->string[0] ||
             b1_phase5brpb_startup_sha256->string[0] ||
             strcmp(b1_phase5brpbr2_diagnostic->string, "0") ||
             b1_phase5brpbr2_expected_entities->string[0] ||
             b1_phase5brpbr2_expected_entities_sha256->string[0] ||
             b1_phase5brpbr2_source_revision->string[0] ||
             b1_phase5brpbr2_producer_authority_sha256->string[0])
            b1_fail("Phase-5B-RP-B controls are partial while mode is disabled");
        b1_phase5brpb_mode->modified = false;
    } else {
        b1_phase5brpb_initialise();
    }
    if (b1_any_request_value()) {
        if (!b1_plan->string[0] || !b1_plan_sha256->string[0] ||
            !b1_output->string[0] ||
            !b1_phase5br_executable_sha256->string[0] ||
            !b1_phase5br_runtime_manifest_sha256->string[0] ||
            !b1_phase5br_runtime_inventory_sha256->string[0])
            b1_fail("partial B1 activation request");
        b1_load_plan();
    }
}

void B1_Shutdown(void)
{
    B1_PrivateCaptureQuiescenceMarker(
        "B1_SHUTDOWN_ENTERED", "src/client/reference_b_b1.c:B1_Shutdown");
    Cmd_Deregister(b1_commands);
    B1_PrivateCaptureQuiescenceMarker(
        "BEFORE_B1_RELEASE_OPERATIONS",
        "src/client/reference_b_b1.c:B1_Shutdown");
    b1_phase5brpb_release();
    b1_candidate_release();
    b1_endpoint_release();
    B1_PrivateCaptureQuiescenceMarker(
        "AFTER_B1_RELEASE_OPERATIONS",
        "src/client/reference_b_b1.c:B1_Shutdown");
    if (b1.log) {
        R_B1PrivateCaptureQuiescenceReadbackState(
            b1.pending_capture_request_serial,
            b1.pending_capture_logical_index,
            b1.pending_capture_source_frame_id,
            b1.pending_capture_role);
        if (b1.descriptor_closed) {
            B1_PrivateCaptureQuiescenceMarker(
                "BEFORE_TEMPORAL_SHUTDOWN",
                "src/client/reference_b_b1.c:B1_Shutdown");
            R_B1TemporalShutdown(b1.state == B1_STATE_COMPLETE);
            B1_PrivateCaptureQuiescenceMarker(
                "AFTER_TEMPORAL_SHUTDOWN",
                "src/client/reference_b_b1.c:B1_Shutdown");
        }
        if (b1.state != B1_STATE_OFF)
            fprintf(b1.log,
                    "{\"event\":\"shutdown\",\"state\":\"%s\","
                    "\"committedPairs\":%u,\"captures\":[%u,%u],\"abandoned\":%s}\n",
                    b1_state_name(b1.state), b1.committed_pairs,
                    b1.captured[0], b1.captured[1],
                    b1.abandoned ? "true" : "false");
        B1_PrivateCaptureQuiescenceMarker(
            "B1_LOG_CLOSING", "src/client/reference_b_b1.c:B1_Shutdown");
        fclose(b1.log);
        b1.log = NULL;
    }
    if (b1.records) Z_Free(b1.records);
    if (b1.settings) Z_Free(b1.settings);
    if (b1.entities) Z_Free(b1.entities);
    if (b1.particles) Z_Free(b1.particles);
    if (b1.dlights) Z_Free(b1.dlights);
    if (b1.lightstyles) Z_Free(b1.lightstyles);
    if (b1.areabits) Z_Free(b1.areabits);
    if (b1.entity_alignment_authority.raw_entities_v1)
        Z_Free(b1.entity_alignment_authority.raw_entities_v1);
    if (b1.entity_alignment_authority.projected_entities_v1)
        Z_Free(b1.entity_alignment_authority.projected_entities_v1);
    memset(&b1, 0, sizeof(b1));
}

bool B1_Requested(void)
{
    return b1_plan && b1_plan->string[0];
}

bool B1_Ready(void)
{
    return B1_Requested() && b1.state != B1_STATE_OFF &&
        b1.state != B1_STATE_AUTHENTICATING && b1.state != B1_STATE_FAILED;
}

static bool b1_phase5brpb_preflight(qhandle_t file)
{
    byte *bytes;
    byte digest[32];
    byte payload_digest[32];
    byte extra;
    size_t offset = 0;
    uint32_t message_count = 0;
    uint32_t selected_count = 0;
    bool exact_end = false;
    int read;

    if (!b1.rpb.enabled || b1.rpb.fixture_preflight_closed ||
        b1.rpb.started || b1.demo_hook.active || FS_Tell(file) != 0)
        b1_fail("Phase-5B-RP-B fixture preflight entered illegally");
    bytes = Z_Malloc((size_t)B1_PHASE5B_FIXTURE_BYTES);
    read = FS_Read(bytes, (size_t)B1_PHASE5B_FIXTURE_BYTES, file);
    if (read != (int)B1_PHASE5B_FIXTURE_BYTES ||
        FS_Read(&extra, 1u, file) != 0 ||
        !b1_sha256_data(bytes, (size_t)B1_PHASE5B_FIXTURE_BYTES, digest) ||
        memcmp(digest, b1.rpb.fixture_sha256, 32)) {
        Z_Free(bytes);
        b1_fail("Phase-5B-RP-B fixture byte envelope/identity differs");
    }
    while (offset < (size_t)B1_PHASE5B_FIXTURE_BYTES) {
        uint32_t encoded;
        uint32_t payload_bytes;
        if ((size_t)B1_PHASE5B_FIXTURE_BYTES - offset < sizeof(encoded)) {
            Z_Free(bytes);
            b1_fail("Phase-5B-RP-B fixture ends inside a length field");
        }
        memcpy(&encoded, bytes + offset, sizeof(encoded));
        offset += sizeof(encoded);
        if (encoded == UINT32_MAX) {
            exact_end = offset == (size_t)B1_PHASE5B_FIXTURE_BYTES;
            break;
        }
        payload_bytes = LittleLong(encoded);
        if (!payload_bytes || payload_bytes > MAX_MSGLEN ||
            payload_bytes > (size_t)B1_PHASE5B_FIXTURE_BYTES - offset ||
            !b1_sha256_data(bytes + offset, payload_bytes, payload_digest)) {
            Z_Free(bytes);
            b1_fail("Phase-5B-RP-B native demo framing is malformed");
        }
        if (!memcmp(payload_digest, (byte[32]){
                0x63,0x89,0xF2,0xB1,0x46,0xC1,0x6F,0x87,
                0x49,0x1B,0x74,0x7E,0x1E,0x5F,0xBF,0xD6,
                0x07,0xEF,0x35,0xFA,0x21,0x72,0xF9,0x62,
                0xDE,0xFB,0xF6,0xB5,0xB2,0xB2,0x63,0xC5 }, 32)) {
            selected_count++;
            if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0) {
                if (b1.rpb.selected_anchor_payload)
                    b1_fail("Phase-5B-RP-B selected payload is duplicated");
                b1.rpb.selected_anchor_payload =
                    b1_phase5brpb_allocate(payload_bytes, 1u,
                                            "selected-anchor-payload");
                memcpy(b1.rpb.selected_anchor_payload, bytes + offset,
                       payload_bytes);
                b1.rpb.selected_anchor_bytes = payload_bytes;
            }
        }
        message_count++;
        offset += payload_bytes;
    }
    Z_Free(bytes);
    if (!exact_end || offset != (size_t)B1_PHASE5B_FIXTURE_BYTES ||
        message_count != B1_PHASE5BRPB_EXPECTED_MESSAGES ||
        selected_count != 1u ||
        (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0 &&
         (!b1.rpb.selected_anchor_payload || !b1.rpb.selected_anchor_bytes)))
        b1_fail("Phase-5B-RP-B framing/message/anchor cardinality differs");
    if (FS_Seek(file, 0, SEEK_SET) || FS_Tell(file) != 0)
        b1_fail("Phase-5B-RP-B fixture cannot reset to byte zero");
    b1.rpb.fixture_preflight_closed = true;
    return true;
}

bool B1_PreflightDemoPlayback(qhandle_t file)
{
    byte *bytes;
    byte fixture_digest[32];
    byte payload_digest[32];
    byte extra;
    char fixture_text[65];
    char payload_text[65];
    size_t offset = 0;
    uint32_t message_count = 0;
    uint32_t selected_count = 0;
    bool exact_end_marker = false;
    int read;

    if (b1.rpb.enabled)
        return b1_phase5brpb_preflight(file);
    if (!B1_Requested())
        return true;
    if (!b1.scene_controller.enabled)
        b1_fail("full B1 demo playback requires b1_scene_controller=1");
    b1_scene_controller_require_controls();
    if (b1.scene_controller.fixture_preflight_closed ||
        b1.demo_hook.active || FS_Tell(file) != 0)
        b1_fail("B1 fixture preflight entered from an illegal file boundary");

    bytes = Z_Malloc((size_t)B1_PHASE5B_FIXTURE_BYTES);
    read = FS_Read(bytes, (size_t)B1_PHASE5B_FIXTURE_BYTES, file);
    if (read != (int)B1_PHASE5B_FIXTURE_BYTES || FS_Read(&extra, 1u, file) != 0 ||
        !b1_sha256_data(bytes, (size_t)B1_PHASE5B_FIXTURE_BYTES, fixture_digest)) {
        Z_Free(bytes);
        b1_fail("B1 fixture byte envelope is truncated, extended or unreadable");
    }
    b1_format_sha256(fixture_digest, fixture_text);
    if (strcmp(fixture_text, B1_PHASE5B_FIXTURE_SHA256)) {
        Z_Free(bytes);
        b1_fail("B1 fixture whole-file identity differs");
    }
    memcpy(b1.source_authority.fixture_sha256, fixture_digest, 32);

    while (offset < (size_t)B1_PHASE5B_FIXTURE_BYTES) {
        uint32_t encoded_length;
        uint32_t payload_bytes;

        if ((size_t)B1_PHASE5B_FIXTURE_BYTES - offset < sizeof(encoded_length)) {
            Z_Free(bytes);
            b1_fail("B1 fixture ends inside a native message-length field");
        }
        memcpy(&encoded_length, bytes + offset, sizeof(encoded_length));
        offset += sizeof(encoded_length);
        if (encoded_length == UINT32_MAX) {
            if (offset != (size_t)B1_PHASE5B_FIXTURE_BYTES) {
                Z_Free(bytes);
                b1_fail("B1 fixture has trailing bytes after its exact end marker");
            }
            exact_end_marker = true;
            break;
        }
        payload_bytes = LittleLong(encoded_length);
        if (!payload_bytes || payload_bytes > MAX_MSGLEN ||
            payload_bytes > (size_t)B1_PHASE5B_FIXTURE_BYTES - offset) {
            Z_Free(bytes);
            b1_fail("B1 fixture native message framing is malformed or truncated");
        }
        if (!b1_sha256_data(bytes + offset, payload_bytes, payload_digest)) {
            Z_Free(bytes);
            b1_fail("cannot hash B1 fixture native message payload");
        }
        b1_format_sha256(payload_digest, payload_text);
        if (!strcmp(payload_text, B1_PHASE5AR5_SELECTED_MESSAGE_SHA256))
            selected_count++;
        message_count++;
        offset += payload_bytes;
    }
    if (!exact_end_marker || offset != (size_t)B1_PHASE5B_FIXTURE_BYTES ||
        message_count != B1_PHASE5A_EXPECTED_MESSAGES || selected_count != 1u) {
        Z_Free(bytes);
        b1_fail("B1 fixture framing, message cardinality or selected occurrence differs");
    }
    Z_Free(bytes);
    if (FS_Seek(file, 0, SEEK_SET) || FS_Tell(file) != 0)
        b1_fail("B1 fixture cannot be reset to native playback byte zero");

    b1.scene_controller.fixture_preflight_closed = true;
    fprintf(b1.log,
            "{\"event\":\"b1_scene_controller_fixture_preflight\","
            "\"fixtureBytes\":%llu,\"fixtureSha256\":\"%s\","
            "\"nativeMessageCount\":%u,\"selectedPayloadSha256\":\"%s\","
            "\"selectedOccurrenceCount\":%u,\"exactEndMarker\":true,"
            "\"trailingBytes\":0,\"nativePayloadsParsed\":false,"
            "\"seekResetOffset\":0,\"pass\":true}\n",
            (unsigned long long)B1_PHASE5B_FIXTURE_BYTES,
            B1_PHASE5B_FIXTURE_SHA256, message_count,
            B1_PHASE5AR5_SELECTED_MESSAGE_SHA256, selected_count);
    if (fflush(b1.log))
        b1_fail("cannot persist B1 fixture preflight closure");
    return true;
}

void B1_DemoPlaybackBegin(void)
{
    if (b1.demo_hook.active)
        b1_fail("demo playback hook began while already active");
    memset(&b1.demo_hook, 0, sizeof(b1.demo_hook));
    b1.demo_hook.active = true;
    b1.demo_hook.armed = B1_Requested() || b1.candidate.enabled ||
        b1.endpoint.enabled || b1.scene_controller.enabled || b1.rpb.enabled;
    if (b1.rpb.enabled) {
        b1_phase5brpb_require_controls();
        if (!b1.rpb.fixture_preflight_closed || b1.rpb.started ||
            b1.rpb.playback_state != B1_PHASE5BRPB_PLAYBACK_OFF)
            b1_fail("Phase-5B-RP-B playback began outside its preflight boundary");
        b1.rpb.started = true;
        b1.rpb.playback_state = B1_PHASE5BRPB_PLAYBACK_OPEN;
    }
    if (b1.scene_controller.enabled &&
        !b1.scene_controller.fixture_preflight_closed)
        b1_fail("B1 demo playback began before fixture preflight closure");
    if (b1.candidate.enabled)
        b1_candidate_open();
    if (b1.endpoint.enabled)
        b1_endpoint_open();
}

void B1_DemoPlaybackEnd(void)
{
    if (b1.rpb.enabled) {
        if (b1.rpb.mode == B1_PHASE5BRPB_MODE_P0 &&
            !b1.rpb.output_sealed)
            b1_phase5brpb_finish();
        if (!b1.rpb.output_sealed ||
            b1.rpb.playback_state != B1_PHASE5BRPB_PLAYBACK_SEALED)
            b1_fail("Phase-5B-RP-B playback ended before its sealed output");
    }
    if (b1.candidate.enabled)
        b1_candidate_finish();
    if (b1.endpoint.enabled)
        b1_endpoint_finish();
    if (b1.scene_controller.enabled &&
        (!b1.scene_controller.selected ||
         !b1.scene_controller.endpoint_closed))
        b1_fail("B1 scene-controller playback ended before endpoint closure");
    memset(&b1.demo_hook, 0, sizeof(b1.demo_hook));
}

void B1_NotifyCompleteDemoMessageParseReturn(void)
{
    if (!b1.demo_hook.armed)
        return;
    if (!b1.demo_hook.active)
        b1_fail("demo parser notification arrived outside playback");
    if (b1.demo_hook.stop_requested)
        b1_fail("native demo message was parsed after the stop boundary closed");
    if (b1.demo_hook.complete_notifications == UINT64_MAX)
        b1_fail("native demo parser notification counter overflow");
    b1.demo_hook.complete_notifications++;
    if (b1.rpb.enabled)
        b1_phase5brpb_complete_message();
    if (b1.candidate.enabled)
        b1_candidate_complete_message();
    if (b1.endpoint.enabled)
        b1_endpoint_complete_message();
    if (b1.scene_controller.enabled)
        b1_scene_controller_complete_message();
}

void B1_RejectIncompleteDemoMessageParseReturn(void)
{
    if (!b1.demo_hook.armed)
        return;
    if (!b1.demo_hook.active)
        b1_fail("incomplete demo parser return arrived outside playback");
    if (b1.candidate.enabled)
        b1_candidate_failure_record("B1_PHASE5A_INCOMPLETE_MESSAGE");
    if (b1.endpoint.enabled)
        b1_endpoint_failure_record("B1_PHASE5AR5_INCOMPLETE_MESSAGE");
    if (b1.scene_controller.enabled)
        fprintf(b1.log,
                "{\"event\":\"b1_scene_controller_failure\","
                "\"code\":\"INCOMPLETE_MESSAGE\",\"terminal\":true}\n");
    if (b1.rpb.enabled)
        b1.rpb.playback_state = B1_PHASE5BRPB_PLAYBACK_FAILED;
    b1_fail("native demo parser returned before the complete message was consumed");
}

void B1_NotifyNormalDemoEOF(void)
{
    if (!b1.demo_hook.armed)
        return;
    if (!b1.demo_hook.active)
        b1_fail("normal demo EOF arrived outside playback");
    if (!b1.rpb.enabled)
        return;
    b1_phase5brpb_require_controls();
    if (!b1.rpb.settings_latched)
        b1_fail("Phase-5B-RP-B EOF precedes settings latch");
    b1_verify_settings_serialization(true, true);
    if (b1.rpb.mode != B1_PHASE5BRPB_MODE_ENUMERATE ||
        b1.rpb.playback_state != B1_PHASE5BRPB_PLAYBACK_OPEN ||
        b1.rpb.normal_eof || b1.rpb.pending_token_id ||
        b1.demo_hook.stop_requested)
        b1_fail("Phase-5B-RP-B normal EOF arrived from an illegal state");
    b1.rpb.normal_eof = true;
    b1.rpb.playback_state = B1_PHASE5BRPB_PLAYBACK_EOF;
    b1_phase5brpb_finish();
}

void B1_RequestDemoPlaybackStop(void)
{
    if (b1.candidate.enabled)
        b1_candidate_fail("B1_PHASE5A_ACCEPTANCE_FORBIDDEN",
                          "candidate inventory cannot request acceptance or stop");
    if (!b1.demo_hook.active || !b1.demo_hook.armed)
        b1_fail("demo playback stop requested while the B1 hook was unarmed");
    if (!b1.demo_hook.complete_notifications)
        b1_fail("demo playback stop requested before a complete-message notification");
    if (b1.demo_hook.stop_requested)
        b1_fail("demo playback stop was requested more than once");
    if (b1.rpb.enabled &&
        (b1.rpb.mode != B1_PHASE5BRPB_MODE_P0 ||
         b1.rpb.playback_state != B1_PHASE5BRPB_PLAYBACK_P0_CLOSED ||
         !b1.rpb.controlled_stop_requested))
        b1_fail("Phase-5B-RP-B stop request is outside P0 closure");
    b1.demo_hook.stop_requested = true;
}

bool B1_DemoPlaybackStopRequested(void)
{
    return b1.demo_hook.active && b1.demo_hook.armed &&
        b1.demo_hook.stop_requested;
}

bool B1_DemoPlaybackStopRequiresDisconnect(void)
{
    bool required = b1.rpb.enabled && b1.rpb.mode == B1_PHASE5BRPB_MODE_P0 &&
        b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_P0_CLOSED &&
        b1.rpb.controlled_stop_requested && b1.demo_hook.active &&
        b1.demo_hook.armed && b1.demo_hook.stop_requested &&
        b1.rpb.scene_closure_count == 1u &&
        b1.rpb.later_message_parse_count == 0u;
    if (!required)
        return false;
    if (b1.rpb.controlled_disconnect_entered)
        b1_fail("Phase-5B-RP-B controlled disconnect was entered more than once");
    /* demo.c calls CL_Disconnect synchronously immediately after this exact
     * transition.  B1_DemoPlaybackEnd requires it before P0 can be sealed. */
    b1.rpb.controlled_disconnect_entered = true;
    return true;
}

bool B1_RPBSceneBuildPermitted(void)
{
    if (!b1.rpb.enabled)
        return true;
    b1_phase5brpb_require_controls();
    if (b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_PENDING_SCENE) {
        if (!b1.rpb.settings_latched)
            b1_fail("Phase-5B-RP-B scene build precedes settings latch");
    }
    if (b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_P0_CLOSED ||
        b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_SEALED ||
        b1.rpb.playback_state == B1_PHASE5BRPB_PLAYBACK_FAILED)
        return false;
    return true;
}

bool B1_NotifyNaturalSceneBuildComplete(const refdef_t *fd)
{
    if (!b1.rpb.enabled)
        return true;
    b1_phase5brpb_require_controls();
    if (b1.rpb.playback_state != B1_PHASE5BRPB_PLAYBACK_PENDING_SCENE)
        return true;
    return b1_phase5brpb_snapshot_scene(fd);
}

bool B1_SceneBuildPermitted(void)
{
    if (!B1_Requested())
        return true;
    if (!B1_Ready())
        b1_fail("B1 scene-build query arrived before authenticated readiness");
    if (!b1.scene_controller.enabled)
        b1_fail("B1 Option-A scene build requires b1_scene_controller=1");
    b1_scene_controller_require_controls();
    if (!b1.scene_controller.endpoint_closed)
        return false;
    if (b1.scene_lifecycle.state != B1_SCENE_ENDPOINT_ACCEPTED)
        b1_fail("B1 ordinary scene build lacks the endpoint lifecycle transition");
    b1_scene_controller_monitor_endpoint("before-ordinary-scene-build", true);
    if (b1.scene_controller.scene_build_consumed || b1.frozen)
        b1_fail("B1 ordinary client scene build was requested more than once");
    return true;
}

static void b1_verify_settings_serialization(bool validation_required,
                                             bool monitoring)
{
    const char *cursor;
    uint32_t count = 0;

    if (!b1.settings)
        b1_fail("canonical settings serialization is unavailable");
    cursor = b1.settings;
    while (*cursor) {
        const char *equals = strchr(cursor, '=');
        const char *newline = strchr(cursor, '\n');
        char name[MAX_QPATH];
        char value[MAX_STRING_CHARS];
        size_t name_bytes, value_bytes;
        cvar_t *actual;
        const char *expected;

        if (!equals || !newline || equals > newline)
            b1_fail("canonical setting line is malformed");
        name_bytes = (size_t)(equals - cursor);
        value_bytes = (size_t)(newline - equals - 1);
        if (!name_bytes || name_bytes >= sizeof(name) ||
            value_bytes >= sizeof(value))
            b1_fail("canonical setting field exceeds runtime bounds");
        memcpy(name, cursor, name_bytes);
        name[name_bytes] = 0;
        memcpy(value, equals + 1, value_bytes);
        value[value_bytes] = 0;
        actual = Cvar_FindVar(name);
        expected = validation_required && !strcmp(name, "vk_validation") ?
            "1" : value;
        if (!actual || strcmp(actual->string, expected))
            b1_fail("canonical setting %s differs", name);
        if (monitoring && actual->modified)
            b1_fail("canonical setting %s was modified after its latch", name);
        if (!monitoring)
            actual->modified = false;
        cursor = newline + 1;
        count++;
    }
    if (count != 215u)
        b1_fail("canonical setting count differs");
}

void B1_VerifySettings(void)
{
    if (!B1_Requested())
        return;
    if (!B1_Ready() || !b1.settings)
        b1_fail("settings requested before authenticated READY state");
    if (!b1.technical_process_numeric || !b1.technical_process_token ||
        strcmp(b1_technical_process_kind->string, b1.technical_process_numeric) ||
        b1_technical_process_kind->modified ||
        b1.technical_process_identity_hash != b1_technical_process_hash64(
            (uint32_t)b1.technical_process_kind, b1.technical_process_token))
        b1_fail("b1_technical_process_kind mutated after its authenticated latch");
    if (!b1_capture_scale_pair_count ||
        b1_capture_scale_pair_count->modified ||
        b1_parse_capture_scale_pair_count(
            b1_capture_scale_pair_count->string) !=
            b1.capture_scale_pair_count ||
        b1.capture_scale_identity_hash != b1_capture_scale_hash64(
            (uint32_t)b1.technical_process_kind,
            b1.capture_scale_pair_count))
        b1_fail("b1_capture_scale_pair_count mutated after its authenticated latch");
    if (b1_technical_process_active() &&
        (strcmp(b1_technical_smoke->string, "0") ||
         strcmp(b1_bootstrap_probe->string, "0") ||
         strcmp(b1_material_secondary_advance_tamper->string, "0") ||
         b1_technical_smoke->modified || b1_bootstrap_probe->modified ||
         b1_material_secondary_advance_tamper->modified))
        b1_fail("active technical process combined with a mutated legacy control");
    if (!b1_scene_admission_witness_authoritative ||
        strcmp(b1_scene_admission_witness_authoritative->string,
               b1.scene_admission_witness_authoritative ? "1" : "0") ||
        b1_scene_admission_witness_authoritative->modified ||
        (b1.technical_process_kind !=
             B1_TECHNICAL_PROCESS_STATIC_CORE_SCENE_ADMISSION_WITNESS &&
         b1.scene_admission_witness_authoritative))
        b1_fail("scene-admission witness classification mutated after its latch");
    b1_verify_settings_serialization(
        b1.validation_smoke || b1_technical_process_active(),
        b1.settings_monitoring);
}

const b1_plan_record_t *B1_GetPlanRecord(uint32_t logical_index)
{
    if (!B1_Ready() || logical_index >= B1_RECORD_COUNT)
        b1_fail("plan record request is outside READY bounds");
    return b1.records + logical_index;
}

bool B1_HasFrozenScene(void)
{
    return b1.frozen;
}

bool B1_PrepareFrame(refdef_t *fd)
{
    const b1_plan_record_t *record;
    b1_scene_authority_t authority;
    byte scene_digest[32];
    byte current_digest[32], previous_digest[32];
    char scene_text[65], current_text[65], previous_text[65];

    if (!B1_Requested())
        return true;
    if (!B1_Ready())
        b1_fail("B1 render requested outside authenticated state");
    if (b1.state == B1_STATE_COMPLETE)
        return false;
    if (cls.state != ca_active || !cl.frame.valid)
        return false;
    if (!b1.frozen) {
        b1_scene_controller_require_controls();
        if (!b1.scene_controller.endpoint_closed ||
            b1.scene_controller.scene_build_consumed ||
            b1.scene_controller.client_scene_closed || b1.scene_source_ready ||
            b1.scene_lifecycle.state != B1_SCENE_ENDPOINT_ACCEPTED)
            b1_fail("B1 ordinary scene build reached an illegal closure boundary");
        b1_scene_controller_monitor_endpoint("ordinary-scene-build-complete", true);
        b1.scene_controller.scene_build_consumed = true;
        b1_freeze_scene(fd);
    }
    if (!b1.scene_controller.client_scene_closed || !b1.frozen)
        b1_fail("B1 client-scene closure is unavailable");
    b1_scene_controller_monitor_endpoint(b1.temporal_infrastructure_ready ?
        "before-role-consumption" : "before-temporal-infrastructure", false);
    if (!b1.temporal_infrastructure_ready) {
        if (cls.ref_type != REF_TYPE_VKPT)
            b1_fail("B1 temporal infrastructure requires the VKPT renderer");
        if (b1.scene_lifecycle.state != B1_SCENE_CLIENT_SCENE_FROZEN ||
            b1.scene_source_ready || b1.bootstrap_complete || b1.descriptor_closed)
            b1_fail("B1 temporal infrastructure entered from an illegal lifecycle state");
        b1_phase5br_recheck_source_authority(
            "before-temporal-infrastructure", true);
        authority = b1.source_authority;
        if (authority.source_frame_id != b1.records[0].source_frame_id)
            b1_fail("B1 Phase-5B-R source-frame authority differs from the plan");
        if (!R_B1TemporalInfrastructurePrepare(&authority,
                b1_renderer_log_line, b1.log,
                b1_phase5br_accept_renderer_scene,
                b1_phase5br_permit_trace,
                b1_phase5br_consume_renderer_scene, &b1))
            b1_fail("renderer temporal infrastructure did not close");
        b1.descriptor_closed = true;
        b1.temporal_infrastructure_ready = true;
        b1_phase5br_transition(B1_SCENE_TEMPORAL_INFRASTRUCTURE_READY);
        fprintf(b1.log,
                "{\"event\":\"temporal_infrastructure_ready\",\"logicalIndex\":0,"
                "\"descriptorSets\":4,\"ownedImagesPerRole\":33,"
                "\"allOwnedImagesCleared\":true,\"historiesValid\":[false,false],"
                "\"sceneSourceReady\":false,\"completeSceneAccepted\":false}\n");
        fflush(b1.log);
    }
    if (!b1.temporal_infrastructure_ready || !b1.descriptor_closed)
        b1_fail("B1 role requested before temporal infrastructure readiness");
    if ((b1.logical_index != 0u || b1.active_role != B1_ROLE_PRIMARY_LEFT) &&
        (!b1.bootstrap_complete || !b1.scene_source_ready ||
         b1.scene_lifecycle.state < B1_SCENE_TRACE_PERMITTED))
        b1_fail("B1 role requested before complete scene acceptance");
    if (b1.frame_pending)
        b1_fail("B1 role advance attempted before prior physical completion");
    if (b1.state != B1_STATE_READY)
        b1_fail("B1 role render entered from an illegal state");
    if (b1.logical_index >= B1_RECORD_COUNT)
        b1_fail("B1 logical cursor overflow");
    if (b1.active_role == B1_ROLE_PRIMARY_LEFT)
        B1_VerifySettings();
    if (b1.roles[b1.active_role].pending_valid)
        b1_fail("B1 role has uncommitted pending state");
    b1_scene_sha256(scene_digest, NULL, NULL, NULL);
    if (memcmp(scene_digest, b1.scene_sha256, sizeof(scene_digest)))
        b1_fail("B1 frozen scene mutated between roles");

    record = b1.records + b1.logical_index;
    if (record->logical_index != b1.logical_index ||
        record->source_frame_id != B1_FIRST_SOURCE + b1.logical_index)
        b1_fail("B1 logical/source identity drifted before role render");
    if (b1.active_role == B1_ROLE_PRIMARY_LEFT)
        b1.pair_started = true;
    else if (!b1.pair_started || !b1.roles[B1_ROLE_PRIMARY_LEFT].pending_valid)
        b1_fail("B1 right role attempted before left role");
    b1_apply_role(fd, record, b1.active_role);
    if (b1_capture_enabled()) {
        if (b1.capture_request_serial == UINT64_MAX)
            b1_fail("B1 capture request serial overflow");
        fd->b1_capture_requested = true;
        fd->b1_capture_request_schema_version = 1;
        fd->b1_capture_request_serial = ++b1.capture_request_serial;
        fprintf(b1.log,
                "{\"event\":\"b1_pre_present_capture_request_v1\","
                "\"schemaVersion\":1,\"requestSerial\":%llu,"
                "\"logicalIndex\":%u,\"sourceFrameId\":%u,"
                "\"role\":\"%s\",\"boundBeforeRoleRender\":true,"
                "\"rawHandlesInIdentity\":false}\n",
                (unsigned long long)fd->b1_capture_request_serial,
                fd->b1_logical_index, fd->b1_source_frame_id,
                b1_role_name(b1.active_role));
        fflush(b1.log);
    }
    b1_scene_role_sha256(b1.active_role, &record->views[b1.active_role],
                         current_digest);
    b1_scene_role_sha256(b1.active_role, &b1.roles[b1.active_role].committed,
                         previous_digest);
    b1_format_sha256(b1.scene_sha256, scene_text);
    b1_format_sha256(current_digest, current_text);
    b1_format_sha256(previous_digest, previous_text);
    b1_set_state(b1.active_role == B1_ROLE_PRIMARY_LEFT ?
                 B1_STATE_LEFT_RENDER : B1_STATE_RIGHT_RENDER);
    fprintf(b1.log,
            "{\"event\":\"role_begin\",\"logicalIndex\":%u,"
            "\"sourceFrameId\":%u,\"role\":\"%s\","
            "\"temporalValid\":%s,\"logicalParity\":%u,"
            "\"jitterIdentity\":%u,\"blueNoiseIdentity\":%u,"
            "\"descriptorRole\":%u,\"sceneSha256\":\"%s\","
            "\"currentViewSha256\":\"%s\",\"previousViewSha256\":\"%s\"}\n",
            b1.logical_index, record->source_frame_id,
            b1_role_name(b1.active_role),
            b1.roles[b1.active_role].temporal_valid ? "true" : "false",
            b1.logical_index & 1u, b1.logical_index, b1.logical_index,
            (unsigned)b1.active_role, scene_text, current_text, previous_text);
    return true;
}

void B1_AfterEndFrame(refdef_t *fd)
{
    b1_role_t role;
    b1_state_id_t expected;

    if (!B1_Requested() || !fd->b1_active)
        return;
    b1.scene_lifecycle.command_buffer_begun = true;
    b1_scene_controller_monitor_endpoint("after-render-before-role-completion", false);
    role = (b1_role_t)fd->b1_role;
    if (role >= B1_ROLE_COUNT || role != b1.active_role)
        b1_fail("B1 renderer returned an invalid semantic role");
    expected = role == B1_ROLE_PRIMARY_LEFT ? B1_STATE_LEFT_RENDER : B1_STATE_RIGHT_RENDER;
    if (b1.state != expected || b1.frame_pending)
        b1_fail("B1 physical completion occurred from an illegal state");
    if (!b1.scene_source_ready || !b1.bootstrap_complete ||
        b1.scene_lifecycle.state < B1_SCENE_TRACE_PERMITTED ||
        b1.scene_lifecycle.prepared_closure_consumption_count != 1u)
        b1_fail("B1 renderer returned before complete scene acceptance and trace permission");
    if (!fd->b1_render_accepted)
        b1_fail("B1 renderer did not accept the complete physical role frame");
    if (fd->b1_logical_index != b1.logical_index ||
        fd->b1_source_frame_id != B1_FIRST_SOURCE + b1.logical_index)
        b1_fail("B1 renderer physical/logical/source identity mismatch");
    if (role == B1_ROLE_PRIMARY_LEFT && b1.logical_index == 1u &&
        (b1.technical_process_kind ==
             B1_TECHNICAL_PROCESS_FAILED_PAIR_AFTER_LEFT_P1 ||
         b1.technical_process_kind ==
             B1_TECHNICAL_PROCESS_FEEDBACK_FAILURE_P1))
        b1_fail("renderer returned past the required technical expected-stop boundary");

    if (b1_capture_enabled()) {
        if (!fd->b1_capture_requested ||
            fd->b1_capture_request_schema_version != 1 ||
            fd->b1_capture_request_serial == 0 ||
            b1.pending_capture_request_serial != 0)
            b1_fail("B1 completed role lacks its exact capture request");
        b1.pending_capture_request_serial = fd->b1_capture_request_serial;
        b1.pending_capture_logical_index = fd->b1_logical_index;
        b1.pending_capture_source_frame_id = fd->b1_source_frame_id;
        b1.pending_capture_role = (uint32_t)role;
        b1.pending_capture_physical_frame = fd->b1_physical_frame;
        b1.pending_capture_physical_slot = fd->b1_physical_slot;
    } else if (fd->b1_capture_requested ||
               fd->b1_capture_request_schema_version != 0 ||
               fd->b1_capture_request_serial != 0) {
        b1_fail("capture-suppressed role exposed a capture request");
    }

    if (b1.logical_index == 0) {
        if (b1.role_physical_slot[role] != UINT32_MAX)
            b1_fail("B1 pair-zero physical role was bound twice");
        b1.role_physical_slot[role] = fd->b1_physical_slot;
        b1.role_physical_frame[role] = fd->b1_physical_frame;
        if (role == B1_ROLE_PRIMARY_RIGHT &&
            b1.role_physical_slot[0] == b1.role_physical_slot[1])
            b1_fail("B1 pair-zero roles resolved to one physical slot");
    } else if (fd->b1_physical_slot != b1.role_physical_slot[role]) {
        b1_fail("B1 semantic role returned to a different physical slot");
    }
    b1.rendered[role]++;
    if (b1.rendered[role] != b1.logical_index + 1)
        b1_fail("B1 role render count is not exactly once per logical pair");
    b1.frame_pending = true;
    b1_set_state(role == B1_ROLE_PRIMARY_LEFT ?
                 B1_STATE_LEFT_CAPTURE : B1_STATE_RIGHT_CAPTURE);
    fprintf(b1.log,
            "{\"event\":\"role_complete\",\"physicalFrame\":%llu,"
            "\"physicalSlot\":%u,\"logicalIndex\":%u,"
            "\"sourceFrameId\":%u,\"role\":\"%s\"}\n",
            (unsigned long long)fd->b1_physical_frame, fd->b1_physical_slot,
             b1.logical_index, fd->b1_source_frame_id, b1_role_name(role));
    fflush(b1.log);
    if (b1.bootstrap_probe) {
        if (role != B1_ROLE_PRIMARY_LEFT || b1.logical_index != 0 ||
            b1.rendered[0] != 1 || b1.rendered[1] != 0 ||
            b1.captured[0] != 0 || b1.captured[1] != 0)
            b1_fail("bootstrap-only probe crossed its pair-zero left-role boundary");
        b1_assert_scene_unchanged("bootstrap-probe-complete");
        b1.frame_pending = false;
        b1.pair_started = false;
        b1_set_state(B1_STATE_COMPLETE);
        fprintf(b1.log,
                "{\"event\":\"bootstrap_probe_complete\",\"logicalIndex\":0,"
                "\"sourceFrameId\":%u,\"committedPairs\":0,"
                "\"rendered\":[1,0],\"captures\":[0,0],"
                "\"trajectoryAdvanced\":false,\"nativeOutput\":false}\n",
                B1_FIRST_SOURCE);
        fflush(b1.log);
        Cbuf_AddText(&cmd_buffer, "quit\n");
    }
}

bool B1_BeforeBeginFrame(void)
{
    const b1_plan_record_t *record;
    b1_role_t captured_role;
    char name[MAX_QPATH];

    if (!B1_Requested())
        return true;
    if (b1.state == B1_STATE_COMPLETE || b1.state == B1_STATE_FAILED)
        return false;
    if (!b1.frame_pending)
        return true;

    b1_scene_controller_monitor_endpoint("before-capture-or-role-commit", false);

    captured_role = b1.state == B1_STATE_LEFT_CAPTURE ?
        B1_ROLE_PRIMARY_LEFT : B1_ROLE_PRIMARY_RIGHT;
    if (b1.state != B1_STATE_LEFT_CAPTURE && b1.state != B1_STATE_RIGHT_CAPTURE)
        b1_fail("B1 capture requested from an illegal state");
    record = b1.records + b1.logical_index;
    if (b1_capture_enabled()) {
        if (b1.pending_capture_request_serial == 0 ||
            b1.pending_capture_logical_index != b1.logical_index ||
            b1.pending_capture_source_frame_id != record->source_frame_id ||
            b1.pending_capture_role != (uint32_t)captured_role ||
            b1.pending_capture_physical_slot !=
                b1.role_physical_slot[captured_role])
            b1_fail("B1 pending capture identity differs before PNG consumption");
        if (Q_snprintf(name, sizeof(name),
                       "native/logical-%08u-source-%08u-%s",
                       b1.logical_index, record->source_frame_id,
                       b1_role_name(captured_role)) >= sizeof(name))
            b1_fail("B1 native capture name exceeds fixed path bounds");
        if (!IMG_B1_WritePNG(name, B1_WIDTH, B1_HEIGHT,
                b1.pending_capture_request_serial, b1.logical_index,
                record->source_frame_id, (uint32_t)captured_role,
                b1.pending_capture_physical_frame,
                b1.pending_capture_physical_slot))
            b1_fail("lossless capture failed for logical %u role %s",
                    b1.logical_index, b1_role_name(captured_role));
        b1.captured[captured_role]++;
        if (b1.captured[captured_role] != b1.logical_index + 1)
            b1_fail("B1 role capture count is not exactly once per logical pair");
        fprintf(b1.log,
                "{\"event\":\"capture\",\"logicalIndex\":%u,"
                "\"sourceFrameId\":%u,\"role\":\"%s\","
                "\"ordinal\":%u,\"filename\":\"%s.png\","
                "\"synchronous\":true,\"beforeNextRole\":true}\n",
                b1.logical_index, record->source_frame_id,
                b1_role_name(captured_role), b1.captured[captured_role], name);
        fflush(b1.log);
        b1.pending_capture_request_serial = 0;
        b1.pending_capture_physical_frame = 0;
        b1.pending_capture_logical_index = 0;
        b1.pending_capture_source_frame_id = 0;
        b1.pending_capture_role = 0;
        b1.pending_capture_physical_slot = 0;
    } else {
        if (b1.pending_capture_request_serial != 0)
            b1_fail("capture-suppressed role retained a pending capture request");
        fprintf(b1.log,
                "{\"event\":\"capture_suppressed\",\"logicalIndex\":%u,"
                "\"sourceFrameId\":%u,\"role\":\"%s\","
                "\"reason\":\"%s\"}\n",
                b1.logical_index, record->source_frame_id,
                b1_role_name(captured_role), b1_technical_process_active() ?
                    "wave2-technical-witness" : "validation-layer-smoke");
        fflush(b1.log);
    }
    b1.frame_pending = false;
    if (b1.state == B1_STATE_LEFT_CAPTURE) {
        if (!b1.roles[B1_ROLE_PRIMARY_LEFT].pending_valid ||
            b1.roles[B1_ROLE_PRIMARY_RIGHT].pending_valid)
            b1_fail("B1 left completion exposed an invalid partial pair");
        b1.active_role = B1_ROLE_PRIMARY_RIGHT;
        b1_set_state(B1_STATE_READY);
        return true;
    }
    if (b1.state != B1_STATE_RIGHT_CAPTURE ||
        !b1.roles[B1_ROLE_PRIMARY_LEFT].pending_valid ||
        !b1.roles[B1_ROLE_PRIMARY_RIGHT].pending_valid)
        b1_fail("B1 pair commit attempted without two complete roles");

    b1_set_state(B1_STATE_PAIR_COMMIT);
    R_B1CadencePairCommit(b1.logical_index);
    for (int role = 0; role < B1_ROLE_COUNT; role++) {
        b1.roles[role].committed = b1.roles[role].pending;
        b1.roles[role].committed_logical = b1.logical_index;
        b1.roles[role].temporal_valid = true;
        b1.roles[role].pending_valid = false;
    }
    b1.committed_pairs++;
    if (b1.logical_index == 0u) {
        if (b1.scene_lifecycle.state != B1_SCENE_TRACE_PERMITTED ||
            b1.scene_lifecycle.prepared_closure_consumption_count != 1u ||
            b1.rendered[B1_ROLE_PRIMARY_LEFT] != 1u ||
            b1.rendered[B1_ROLE_PRIMARY_RIGHT] != 1u)
            b1_fail("B1 pair-zero lifecycle commit lacks two accepted role renders");
        b1.scene_lifecycle.publication_count = 1u;
        b1.scene_lifecycle.pair_commit_count = 1u;
        b1_phase5br_transition(B1_SCENE_PAIR_COMMITTED);
    }
    fprintf(b1.log,
            "{\"event\":\"pair_commit\",\"logicalIndex\":%u,"
            "\"sourceFrameId\":%u,\"roleCommits\":2,\"logicalAdvance\":1}\n",
            b1.logical_index, B1_FIRST_SOURCE + b1.logical_index);
    b1.logical_index++;
    b1.pair_started = false;
    if (b1.logical_index == b1_required_complete_pairs()) {
        uint32_t expected_captures = b1_capture_enabled() ?
            b1_required_complete_pairs() : 0u;
        if (b1.captured[0] != expected_captures ||
            b1.captured[1] != expected_captures)
            b1_fail("B1 completion capture cardinality differs");
        b1_assert_scene_unchanged("series-complete");
        b1_set_state(B1_STATE_COMPLETE);
        fprintf(b1.log,
                "{\"event\":\"complete\",\"logicalPairs\":%u,"
                "\"leftRenders\":%u,\"rightRenders\":%u,"
                "\"leftCaptures\":%u,\"rightCaptures\":%u,"
                "\"validationSmoke\":%s,\"technicalProcessKind\":%u,"
                "\"technicalProcessToken\":\"%s\",\"classification\":\"%s\"}\n",
                b1.committed_pairs, b1.rendered[0], b1.rendered[1],
                b1.captured[0], b1.captured[1],
                b1.validation_smoke ? "true" : "false",
                (unsigned)b1.technical_process_kind,
                b1.technical_process_token,
                b1.validation_smoke ? "VALIDATION_LAYER_SMOKE_TECHNICAL_ONLY" :
                b1_technical_process_active() ? "B1_WAVE2_TECHNICAL_WITNESS_ONLY" :
                    "NON-AUTHORITATIVE_TECHNICAL_VALIDATION_RENDER");
        if (b1.technical_process_kind ==
            B1_TECHNICAL_PROCESS_CAPTURE_SCALE_V1)
            fprintf(b1.log,
                "{\"event\":\"b1_capture_scale_complete_v1\","
                "\"schemaVersion\":1,\"domain\":\"B1-CAPTURE-SCALE-EXECUTION-V1\","
                "\"pairCount\":%u,\"executionIdentityHash64\":\"%016llx\","
                "\"completedLogicalIndex\":%u,\"advancedCursor\":%u,"
                "\"committedPairs\":%u,\"leftRenders\":%u,\"rightRenders\":%u,"
                "\"leftCaptures\":%u,\"rightCaptures\":%u,"
                "\"extraRoles\":0,\"extraCaptures\":0}\n",
                b1.capture_scale_pair_count,
                (unsigned long long)b1.capture_scale_identity_hash,
                b1.logical_index - 1u, b1.logical_index,
                b1.committed_pairs, b1.rendered[0], b1.rendered[1],
                b1.captured[0], b1.captured[1]);
        fflush(b1.log);
        Cbuf_AddText(&cmd_buffer, "quit\n");
        B1_PrivateCaptureQuiescenceQuitEnqueued();
        return false;
    }
    b1.active_role = B1_ROLE_PRIMARY_LEFT;
    b1_set_state(B1_STATE_READY);
    fflush(b1.log);
    return true;
}
