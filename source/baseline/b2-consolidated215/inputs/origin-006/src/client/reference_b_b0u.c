// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Reference-B B0-U private diagnostic adapter.
 *
 * This file is compiled only when CONFIG_REFERENCE_B_B0U=ON. It consumes a
 * strictly verified, deterministic binary plan produced from the authenticated
 * private JSONL stimulus by scripts/reference_b_b0u.py. It is not a renderer
 * fix, a release feature, or a general replay facility.
 */

#include "client.h"
#include "reference_b_b0u.h"
#include "refresh/images.h"

#include <windows.h>
#include <bcrypt.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define B0U_CONTRACT "B0U-CONTRACT-R1"
#define B0U_PINNED_BASE "f2526e9a165949f66e91e82f0d63aa7bb2567b4d"
#ifdef REFERENCE_B_B0P
#define B0P_BUILD_MARKER "REFERENCE-B-B0P-P0P1-R1"
#endif
#define B0U_RECORD_COUNT 4073u
#define B0U_FIRST_SOURCE 190u
#define B0U_LAST_SOURCE 4262u
#define B0U_EVIDENCE_COUNT 239u
#define B0U_WIDTH 1948
#define B0U_HEIGHT 1563

#define B0U_STIMULUS_BYTES UINT64_C(7936747)
#define B0U_MANIFEST_BYTES UINT64_C(3154)
#define B0U_VERIFICATION_BYTES UINT64_C(3428)

#define B0U_STIMULUS_SHA256 "9C62BBD15D08CE27D79AFA79DD643D34528EF80B2F1BD9C0155495492E62E8ED"
#define B0U_MANIFEST_SHA256 "BEBE137AB631975547599D5A8E112DC8B3E865E17C248D64354C6354485A3E90"
#define B0U_VERIFICATION_SHA256 "C9C8998C487F7A4C8D439A889FFBB851225D37A9FEE9CAEEE0B08EF08585D8C9"

#define B0U_ACCEPT_RENDERED_WORLD (1u << 0)
#define B0U_ACCEPT_ORIENTATION    (1u << 1)
#define B0U_ACCEPT_POSITION       (1u << 2)
#define B0U_ACCEPT_FOV            (1u << 3)
#define B0U_POSITION_FALLBACK     (1u << 4)

#pragma pack(push, 1)
typedef struct b0u_plan_header_s {
    char magic[8];
    char contract[24];
    char pinned_base[41];
    uint32_t version;
    uint32_t header_size;
    uint32_t record_size;
    uint32_t record_count;
    uint32_t first_source;
    uint32_t last_source;
    uint32_t evidence_count;
    uint64_t stimulus_bytes;
    byte stimulus_sha256[32];
} b0u_plan_header_t;

typedef struct b0u_plan_record_s {
    uint32_t stimulus_index;
    uint32_t source_frame_id;
    int64_t relative_time_ns;
    int64_t frame_duration_ns;
    uint32_t acceptance;
    float scene_time;
    int32_t rdflags;
    int32_t viewport[4];
    float view_origin[3];
    float view_angles[3];
    float fov_xy[2];
    uint32_t render_extent[2];
    uint32_t taa_extent[2];
    float V[16];
    float invV[16];
    float P[16];
    float invP[16];
    uint32_t evidence;
} b0u_plan_record_t;
#pragma pack(pop)

_Static_assert(sizeof(b0u_plan_header_t) == 141, "unexpected B0-U plan header layout");
_Static_assert(sizeof(b0u_plan_record_t) == 360, "unexpected B0-U plan record layout");

typedef struct b0u_state_s {
    bool loaded;
    bool frozen;
    bool current_rendered;
    bool frame_pending;
    bool complete;
    uint32_t current;
    uint32_t captured;
    b0u_plan_record_t *records;
    refdef_t frozen_refdef;
    entity_t *entities;
    particle_t *particles;
    dlight_t *dlights;
    lightstyle_t *lightstyles;
    byte *areabits;
    int areabytes;
    FILE *log;
} b0u_state_t;

static b0u_state_t b0u;
static cvar_t *b0u_plan;
static cvar_t *b0u_plan_sha256;
static cvar_t *b0u_stimulus;
static cvar_t *b0u_manifest;
static cvar_t *b0u_verification;
static cvar_t *b0u_output;
static cvar_t *b0u_status;

typedef struct b0u_required_cvar_s {
    const char *name;
    float value;
} b0u_required_cvar_t;

static const b0u_required_cvar_t b0u_required_cvars[] = {
    { "vid_rtx", 1.f },
    { "vid_fullscreen", 0.f },
    { "vid_hdr", 0.f },
    { "vid_vsync", 0.f },
    { "viewsize", 100.f },
    { "drs_enable", 0.f },
    { "flt_fsr_enable", 0.f },
    { "flt_enable", 1.f },
    { "flt_taa", 2.f },
    { "flt_taa_anti_sparkle", 0.75f },
    { "flt_taa_variance", 1.f },
    { "flt_taa_history_weight", 0.95f },
    { "flt_temporal_hf", 1.f },
    { "flt_temporal_lf", 1.f },
    { "flt_temporal_spec", 1.f },
    { "bloom_enable", 1.f },
    { "pt_accumulation_rendering", 0.f },
    { "pt_projection", 0.f },
    { "scr_draw2d", 0.f },
};

static void b0u_fail(const char *format, ...)
{
    char message[MAX_STRING_CHARS];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    message[sizeof(message) - 1] = 0;

    if (b0u.log) {
        fprintf(b0u.log, "{\"event\":\"failure\",\"message\":\"B0U_FAIL\"}\n");
        fflush(b0u.log);
    }
    if (b0u_status)
        Cvar_SetByVar(b0u_status, "FAILED", FROM_CODE);
    Com_Error(ERR_FATAL, "B0U_FAIL: %s", message);
}

static int b0u_hex_value(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool b0u_parse_sha256(const char *text, byte digest[32])
{
    if (!text || strlen(text) != 64)
        return false;

    for (int i = 0; i < 32; i++) {
        int hi = b0u_hex_value(text[i * 2]);
        int lo = b0u_hex_value(text[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return false;
        digest[i] = (byte)((hi << 4) | lo);
    }
    return true;
}

static bool b0u_sha256_file(const char *path, uint64_t *bytes, byte digest[32])
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

    if (!path || !*path)
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

static void b0u_authenticate_file(const char *label, const char *path,
                                  uint64_t expected_bytes, const char *expected_hex)
{
    uint64_t bytes = 0;
    byte actual[32], expected[32];

    if (!b0u_parse_sha256(expected_hex, expected))
        b0u_fail("internal %s SHA-256 is malformed", label);
    if (!b0u_sha256_file(path, &bytes, actual))
        b0u_fail("cannot read or hash %s", label);
    if (bytes != expected_bytes)
        b0u_fail("%s byte count mismatch", label);
    if (memcmp(actual, expected, sizeof(actual)))
        b0u_fail("%s SHA-256 mismatch", label);
}

static bool b0u_is_evidence_source(uint32_t source)
{
    return (source >= 4006u && source <= 4117u) ||
           (source >= 4136u && source <= 4262u);
}

static bool b0u_finite_array(const float *values, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        if (!isfinite(values[i]))
            return false;
    }
    return true;
}

static void b0u_validate_and_log_cvars(void)
{
    for (size_t i = 0; i < q_countof(b0u_required_cvars); i++) {
        const b0u_required_cvar_t *required = b0u_required_cvars + i;
        cvar_t *actual = Cvar_FindVar(required->name);
        if (!actual || fabsf(actual->value - required->value) > 0.00001f)
            b0u_fail("required cvar %s differs or is unavailable", required->name);
    }

    fprintf(b0u.log, "{\"event\":\"configuration\",\"cvars\":{");
    for (size_t i = 0; i < q_countof(b0u_required_cvars); i++) {
        const b0u_required_cvar_t *required = b0u_required_cvars + i;
        cvar_t *actual = Cvar_FindVar(required->name);
        fprintf(b0u.log, "%s\"%s\":\"%s\"",
                i ? "," : "", required->name, actual->string);
    }
    fprintf(b0u.log,
            "},\"unsupported\":[\"CONFIG_OPENXR (not present in pinned upstream)\"]}\n");
    fflush(b0u.log);
}

static void b0u_validate_records(void)
{
    uint32_t evidence_count = 0;

    for (uint32_t i = 0; i < B0U_RECORD_COUNT; i++) {
        const b0u_plan_record_t *record = b0u.records + i;
        const uint32_t required_acceptance = B0U_ACCEPT_RENDERED_WORLD |
            B0U_ACCEPT_ORIENTATION | B0U_ACCEPT_POSITION | B0U_ACCEPT_FOV;

        if (record->stimulus_index != i)
            b0u_fail("misordered stimulus index at record %u", i);
        if (record->source_frame_id != i + B0U_FIRST_SOURCE)
            b0u_fail("misordered source frame at record %u", i);
        if (i == 0 && record->relative_time_ns != 0)
            b0u_fail("first relative time is not zero");
        if (i && record->relative_time_ns <= b0u.records[i - 1].relative_time_ns)
            b0u_fail("relative time is not strictly increasing at record %u", i);
        if (record->frame_duration_ns <= 0)
            b0u_fail("non-positive frame duration at record %u", i);
        if (record->acceptance != required_acceptance)
            b0u_fail("invalid current-frame acceptance at record %u", i);
        if (record->viewport[0] != 0 || record->viewport[1] != 0 ||
            record->viewport[2] != B0U_WIDTH || record->viewport[3] != B0U_HEIGHT)
            b0u_fail("renderer viewport mismatch at record %u", i);
        if (record->render_extent[0] != B0U_WIDTH || record->render_extent[1] != B0U_HEIGHT ||
            record->taa_extent[0] != B0U_WIDTH || record->taa_extent[1] != B0U_HEIGHT)
            b0u_fail("render extent mismatch at record %u", i);
        if (!isfinite(record->scene_time) ||
            !b0u_finite_array(record->view_origin, 3) ||
            !b0u_finite_array(record->view_angles, 3) ||
            !b0u_finite_array(record->fov_xy, 2) ||
            !b0u_finite_array(record->V, 16) || !b0u_finite_array(record->invV, 16) ||
            !b0u_finite_array(record->P, 16) || !b0u_finite_array(record->invP, 16))
            b0u_fail("non-finite camera value at record %u", i);
        if (record->fov_xy[0] <= 0.f || record->fov_xy[0] >= 179.f ||
            record->fov_xy[1] <= 0.f || record->fov_xy[1] >= 179.f)
            b0u_fail("invalid FOV at record %u", i);
        if (!!record->evidence != b0u_is_evidence_source(record->source_frame_id))
            b0u_fail("evidence membership mismatch at record %u", i);
        evidence_count += !!record->evidence;
    }

    if (evidence_count != B0U_EVIDENCE_COUNT)
        b0u_fail("evidence count mismatch");
}

static void b0u_open_log(void)
{
    char path[MAX_OSPATH];

    if (!b0u_output->string[0])
        b0u_fail("b0u_output is required");
    if (Q_snprintf(path, sizeof(path), "%s/adapter.jsonl", b0u_output->string) >= sizeof(path))
        b0u_fail("b0u_output path is too long");
    b0u.log = fopen(path, "wb");
    if (!b0u.log)
        b0u_fail("cannot create adapter log");
}

static void b0u_load_plan(void)
{
    FILE *file;
    b0u_plan_header_t header;
    byte plan_actual[32], plan_expected[32];
    byte stimulus_expected[32];
    uint64_t plan_bytes = 0;
    uint64_t expected_plan_bytes;

    b0u_open_log();
    b0u_authenticate_file("stimulus", b0u_stimulus->string,
                          B0U_STIMULUS_BYTES, B0U_STIMULUS_SHA256);
    b0u_authenticate_file("manifest", b0u_manifest->string,
                          B0U_MANIFEST_BYTES, B0U_MANIFEST_SHA256);
    b0u_authenticate_file("verification", b0u_verification->string,
                          B0U_VERIFICATION_BYTES, B0U_VERIFICATION_SHA256);

    if (!b0u_parse_sha256(b0u_plan_sha256->string, plan_expected))
        b0u_fail("b0u_plan_sha256 must be exactly 64 hexadecimal characters");
    if (!b0u_sha256_file(b0u_plan->string, &plan_bytes, plan_actual))
        b0u_fail("cannot read or hash replay plan");
    if (memcmp(plan_actual, plan_expected, sizeof(plan_actual)))
        b0u_fail("replay plan SHA-256 mismatch");

    file = fopen(b0u_plan->string, "rb");
    if (!file)
        b0u_fail("cannot open replay plan");
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        b0u_fail("truncated replay plan header");
    }

    if (memcmp(header.magic, "B0UPLN1", 8) ||
        !memchr(header.contract, 0, sizeof(header.contract)) ||
        !memchr(header.pinned_base, 0, sizeof(header.pinned_base)) ||
        strcmp(header.contract, B0U_CONTRACT) ||
        strcmp(header.pinned_base, B0U_PINNED_BASE) ||
        header.version != 1 || header.header_size != sizeof(header) ||
        header.record_size != sizeof(b0u_plan_record_t) ||
        header.record_count != B0U_RECORD_COUNT ||
        header.first_source != B0U_FIRST_SOURCE ||
        header.last_source != B0U_LAST_SOURCE ||
        header.evidence_count != B0U_EVIDENCE_COUNT ||
        header.stimulus_bytes != B0U_STIMULUS_BYTES) {
        fclose(file);
        b0u_fail("replay plan header identity mismatch");
    }
    b0u_parse_sha256(B0U_STIMULUS_SHA256, stimulus_expected);
    if (memcmp(header.stimulus_sha256, stimulus_expected, 32)) {
        fclose(file);
        b0u_fail("replay plan stimulus identity mismatch");
    }

    expected_plan_bytes = sizeof(header) + (uint64_t)header.record_count * header.record_size;
    if (plan_bytes != expected_plan_bytes) {
        fclose(file);
        b0u_fail("replay plan byte count mismatch");
    }

    b0u.records = Z_Malloc(sizeof(*b0u.records) * B0U_RECORD_COUNT);
    if (fread(b0u.records, sizeof(*b0u.records), B0U_RECORD_COUNT, file) != B0U_RECORD_COUNT) {
        fclose(file);
        b0u_fail("truncated replay plan records");
    }
    if (fgetc(file) != EOF) {
        fclose(file);
        b0u_fail("trailing replay plan data");
    }
    fclose(file);

    b0u_validate_records();
    b0u.loaded = true;
    Cvar_SetByVar(b0u_status, "AUTHENTICATED", FROM_CODE);
    fprintf(b0u.log,
            "{\"event\":\"authenticated\",\"contract\":\"%s\","
            "\"pinnedBase\":\"%s\",\"records\":%u,\"evidence\":%u,"
            "\"stimulusSha256\":\"%s\"}\n",
            B0U_CONTRACT, B0U_PINNED_BASE, B0U_RECORD_COUNT,
            B0U_EVIDENCE_COUNT, B0U_STIMULUS_SHA256);
#ifdef REFERENCE_B_B0P
    fprintf(b0u.log,
            "{\"event\":\"b0p_build\",\"marker\":\"%s\"}\n",
            B0P_BUILD_MARKER);
#endif
    fflush(b0u.log);
}

static void *b0u_copy_required(const void *source, size_t bytes, const char *label)
{
    void *copy;
    if (!source || !bytes)
        b0u_fail("missing frozen %s", label);
    copy = Z_Malloc(bytes);
    memcpy(copy, source, bytes);
    return copy;
}

static void b0u_freeze_scene(refdef_t *fd)
{
    if (strcmp(cl.mapname, "base1"))
        b0u_fail("expected base1, got another map");
    if (cl_paused->integer != 2 || !sv_paused->integer)
        b0u_fail("base1 must be manually paused before scene freeze");
    if (r_config.width != B0U_WIDTH || r_config.height != B0U_HEIGHT)
        b0u_fail("output geometry is not 1948x1563");
    b0u_validate_and_log_cvars();
    if (fd->num_entities < 0 || fd->num_entities > MAX_ENTITIES ||
        fd->num_particles < 0 || fd->num_particles > MAX_PARTICLES ||
        fd->num_dlights < 0 || fd->num_dlights > MAX_DLIGHTS)
        b0u_fail("scene count is outside upstream bounds");

    b0u.frozen_refdef = *fd;
    if (fd->num_entities) {
        b0u.entities = b0u_copy_required(fd->entities,
            sizeof(*fd->entities) * fd->num_entities, "entities");
        b0u.frozen_refdef.entities = b0u.entities;
    }
    if (fd->num_particles) {
        b0u.particles = b0u_copy_required(fd->particles,
            sizeof(*fd->particles) * fd->num_particles, "particles");
        b0u.frozen_refdef.particles = b0u.particles;
    }
    if (fd->num_dlights) {
        b0u.dlights = b0u_copy_required(fd->dlights,
            sizeof(*fd->dlights) * fd->num_dlights, "dynamic lights");
        b0u.frozen_refdef.dlights = b0u.dlights;
    }
    b0u.lightstyles = b0u_copy_required(fd->lightstyles,
        sizeof(*fd->lightstyles) * MAX_LIGHTSTYLES, "light styles");
    b0u.frozen_refdef.lightstyles = b0u.lightstyles;

    b0u.areabytes = cl.frame.areabytes;
    if (b0u.areabytes) {
        b0u.areabits = b0u_copy_required(fd->areabits, b0u.areabytes, "area bits");
        b0u.frozen_refdef.areabits = b0u.areabits;
    } else {
        b0u.frozen_refdef.areabits = NULL;
    }

    b0u.frozen = true;
    Cvar_SetByVar(b0u_status, "REPLAYING", FROM_CODE);
    fprintf(b0u.log,
            "{\"event\":\"scene_frozen\",\"map\":\"base1\","
            "\"entities\":%d,\"particles\":%d,\"dlights\":%d,"
            "\"areabytes\":%d,\"width\":%d,\"height\":%d}\n",
            fd->num_entities, fd->num_particles, fd->num_dlights,
            b0u.areabytes, r_config.width, r_config.height);
    fflush(b0u.log);
}

void B0U_Init(void)
{
    const int private_flags = CVAR_PRIVATE | CVAR_NOARCHIVE;

    memset(&b0u, 0, sizeof(b0u));
    b0u_plan = Cvar_Get("b0u_plan", "", private_flags);
    b0u_plan_sha256 = Cvar_Get("b0u_plan_sha256", "", private_flags);
    b0u_stimulus = Cvar_Get("b0u_stimulus", "", private_flags);
    b0u_manifest = Cvar_Get("b0u_manifest", "", private_flags);
    b0u_verification = Cvar_Get("b0u_verification", "", private_flags);
    b0u_output = Cvar_Get("b0u_output", "", private_flags);
    b0u_status = Cvar_Get("b0u_status", "DISABLED", CVAR_ROM | CVAR_NOARCHIVE);

    if (B0U_Requested())
        b0u_load_plan();
}

void B0U_Shutdown(void)
{
    if (b0u.log) {
        if (b0u.loaded && !b0u.complete)
            fprintf(b0u.log,
                    "{\"event\":\"terminated\",\"processed\":%u,\"captured\":%u}\n",
                    b0u.current, b0u.captured);
        fclose(b0u.log);
    }
    if (b0u.records) Z_Free(b0u.records);
    if (b0u.entities) Z_Free(b0u.entities);
    if (b0u.particles) Z_Free(b0u.particles);
    if (b0u.dlights) Z_Free(b0u.dlights);
    if (b0u.lightstyles) Z_Free(b0u.lightstyles);
    if (b0u.areabits) Z_Free(b0u.areabits);
    memset(&b0u, 0, sizeof(b0u));
}

bool B0U_Requested(void)
{
    return b0u_plan && b0u_plan->string[0];
}

bool B0U_HasFrozenScene(void)
{
    return b0u.frozen;
}

bool B0U_PrepareFrame(refdef_t *fd)
{
    const b0u_plan_record_t *record;
    ref_feedback_t feedback;

    if (!B0U_Requested())
        return true;
    if (!b0u.loaded)
        b0u_fail("replay requested without an authenticated plan");
    if (b0u.complete)
        return false;
    if (cls.state != ca_active || !cl.frame.valid)
        return false;
    // Startup commands create base1 and request a manual pause.  Never render
    // ordinary gameplay while those commands are still settling, and freeze
    // only after both client and local server report the pause.
    if (!b0u.frozen && (cl_paused->integer != 2 || !sv_paused->integer))
        return false;
    if (b0u.frame_pending || b0u.current_rendered)
        b0u_fail("frame advance attempted before prior completion");
    if (!b0u.frozen)
        b0u_freeze_scene(fd);
    if (b0u.current >= B0U_RECORD_COUNT)
        b0u_fail("record cursor overflow");

    record = b0u.records + b0u.current;
    feedback = fd->feedback;
    *fd = b0u.frozen_refdef;
    fd->feedback = feedback;
    fd->x = record->viewport[0];
    fd->y = record->viewport[1];
    fd->width = record->viewport[2];
    fd->height = record->viewport[3];
    fd->fov_x = record->fov_xy[0];
    fd->fov_y = record->fov_xy[1];
    VectorCopy(record->view_origin, fd->vieworg);
    VectorCopy(record->view_angles, fd->viewangles);
    fd->time = record->scene_time;
    fd->rdflags = record->rdflags;
    fd->b0u_camera_override = true;
    fd->b0u_stimulus_index = record->stimulus_index;
    memcpy(fd->b0u_V, record->V, sizeof(fd->b0u_V));
    memcpy(fd->b0u_invV, record->invV, sizeof(fd->b0u_invV));
    memcpy(fd->b0u_P, record->P, sizeof(fd->b0u_P));
    memcpy(fd->b0u_invP, record->invP, sizeof(fd->b0u_invP));

    b0u.current_rendered = true;
    fprintf(b0u.log,
            "{\"event\":\"frame\",\"stimulusIndex\":%u,"
            "\"sourceFrameId\":%u,\"relativeTimeNs\":%lld,\"evidence\":%s}\n",
            record->stimulus_index, record->source_frame_id,
            (long long)record->relative_time_ns,
            record->evidence ? "true" : "false");
    return true;
}

void B0U_AfterEndFrame(void)
{
    if (!B0U_Requested() || !b0u.current_rendered)
        return;
    if (b0u.frame_pending)
        b0u_fail("duplicate frame completion");
    b0u.current_rendered = false;
    b0u.frame_pending = true;
}

bool B0U_BeforeBeginFrame(void)
{
    const b0u_plan_record_t *record;
    char name[MAX_QPATH];

    if (!B0U_Requested() || !b0u.frame_pending)
        return true;

    record = b0u.records + b0u.current;
    if (record->evidence) {
        Q_snprintf(name, sizeof(name), "b0u/native/source-%08u", record->source_frame_id);
        if (!IMG_B0U_WritePNG(name, B0U_WIDTH, B0U_HEIGHT))
            b0u_fail("lossless capture failed for source frame %u", record->source_frame_id);
        b0u.captured++;
        fprintf(b0u.log,
                "{\"event\":\"capture\",\"stimulusIndex\":%u,"
                "\"sourceFrameId\":%u,\"ordinal\":%u}\n",
                record->stimulus_index, record->source_frame_id, b0u.captured);
        fflush(b0u.log);
    }

    b0u.frame_pending = false;
    b0u.current++;
    if (b0u.current != B0U_RECORD_COUNT)
        return true;

    if (b0u.captured != B0U_EVIDENCE_COUNT)
        b0u_fail("completed replay with the wrong evidence count");
    b0u.complete = true;
    Cvar_SetByVar(b0u_status, "COMPLETE", FROM_CODE);
    fprintf(b0u.log,
            "{\"event\":\"complete\",\"processed\":%u,\"captured\":%u,"
            "\"thirdTailNs\":1500032700}\n",
            b0u.current, b0u.captured);
    fflush(b0u.log);
    Cbuf_AddText(&cmd_buffer, "quit\n");
    return false;
}
