// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Local saved-file format only. Network/message buffers and game save fields
// are unchanged. No legacy importer or persistent staging state.
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#include <limits.h>
#include "b2_save_paths534.inc"
#endif

#define B2_SAVE_VERSION 2u
#define B2_SAVE_REMAP 0x58453242u /* little-endian B2EX */
#define B2_SAVE_FILE_CAP (1024u * 1024u)
#define B2_SAVE_HEADER_BYTES 28u

typedef struct {
    const configstring_t *strings;
    const byte *portals;
    size_t portal_bytes;
} b2_save_view_t;

static void B2_SavePut32(byte *p, uint32_t v)
{
    p[0] = (byte)v; p[1] = (byte)(v >> 8);
    p[2] = (byte)(v >> 16); p[3] = (byte)(v >> 24);
}

static uint32_t B2_SaveGet32(const byte *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static bool B2_SaveStringsValid(const cs_remap_t *r, const configstring_t *strings)
{
    if (!B2_FreshRemap(r) || !strings || r->end != MAX_CONFIGSTRINGS)
        return false;
    for (int i = 0; i < r->end; ++i) {
        size_t bound = CS_SIZE(r, i);
        size_t remaining = (size_t)(r->end - i) * MAX_QPATH;
        if (!bound || bound > remaining || !memchr(strings[i], 0, bound))
            return false;
    }
    return true;
}

// Preserve the indices cached by this map's ordinary SpawnEntities. A new
// profile save must contain that exact precache prefix; late additions may
// extend it. Refuse changed identities instead of renumbering live references.
static bool B2_SaveResourcePrefix(const cs_remap_t *r,
    const configstring_t *fresh, const configstring_t *saved)
{
    const int bases[] = { r->models, r->sounds, r->images };
    const int counts[] = { r->max_models, r->max_sounds, r->max_images };
    if (strcmp(fresh[r->models + 1], saved[r->models + 1]) ||
        strcmp(fresh[r->mapchecksum], saved[r->mapchecksum]))
        return false;
    for (unsigned kind = 0; kind < 3; ++kind)
        for (int i = 0; i < counts[kind]; ++i)
            if (fresh[bases[kind] + i][0] &&
                strcmp(fresh[bases[kind] + i], saved[bases[kind] + i]))
                return false;
    return true;
}

static size_t B2_SaveEncode(byte *out, size_t capacity, const cs_remap_t *r,
    const configstring_t *strings, const byte *portals, size_t portal_bytes)
{
    const size_t table_bytes = (size_t)MAX_CONFIGSTRINGS * MAX_QPATH;
    if (!out || !B2_SaveStringsValid(r, strings) || !portals ||
        portal_bytes > MAX_MAP_PORTAL_BYTES ||
        table_bytes > B2_SAVE_FILE_CAP - B2_SAVE_HEADER_BYTES ||
        portal_bytes > B2_SAVE_FILE_CAP - B2_SAVE_HEADER_BYTES - table_bytes)
        return 0;
    size_t total = B2_SAVE_HEADER_BYTES + table_bytes + portal_bytes;
    if (capacity < total)
        return 0;
    B2_SavePut32(out, SAVE_MAGIC2);
    B2_SavePut32(out + 4, B2_SAVE_VERSION);
    B2_SavePut32(out + 8, B2_SAVE_REMAP);
    B2_SavePut32(out + 12, (uint32_t)r->end);
    B2_SavePut32(out + 16, MAX_QPATH);
    B2_SavePut32(out + 20, (uint32_t)table_bytes);
    B2_SavePut32(out + 24, (uint32_t)portal_bytes);
    memcpy(out + B2_SAVE_HEADER_BYTES, strings, table_bytes);
    memcpy(out + B2_SAVE_HEADER_BYTES + table_bytes, portals, portal_bytes);
    return total;
}

static bool B2_SaveDecode(const byte *data, size_t bytes, const cs_remap_t *r,
    const configstring_t *fresh, size_t expected_portals, b2_save_view_t *view)
{
    const size_t table_bytes = (size_t)MAX_CONFIGSTRINGS * MAX_QPATH;
    if (!view)
        return false;
    memset(view, 0, sizeof(*view));
    if (!data || bytes < B2_SAVE_HEADER_BYTES || bytes > B2_SAVE_FILE_CAP ||
        !B2_SaveStringsValid(r, fresh) ||
        B2_SaveGet32(data) != SAVE_MAGIC2 ||
        B2_SaveGet32(data + 4) != B2_SAVE_VERSION ||
        B2_SaveGet32(data + 8) != B2_SAVE_REMAP ||
        B2_SaveGet32(data + 12) != (uint32_t)r->end ||
        B2_SaveGet32(data + 16) != MAX_QPATH ||
        B2_SaveGet32(data + 20) != table_bytes)
        return false;
    size_t portal_bytes = B2_SaveGet32(data + 24);
    if (portal_bytes > MAX_MAP_PORTAL_BYTES || portal_bytes != expected_portals ||
        table_bytes > bytes - B2_SAVE_HEADER_BYTES ||
        portal_bytes != bytes - B2_SAVE_HEADER_BYTES - table_bytes)
        return false;
    const configstring_t *saved = (const configstring_t *)(data + B2_SAVE_HEADER_BYTES);
    if (!B2_SaveStringsValid(r, saved) || !B2_SaveResourcePrefix(r, fresh, saved))
        return false;
    view->strings = saved;
    view->portals = data + B2_SAVE_HEADER_BYTES + table_bytes;
    view->portal_bytes = portal_bytes;
    return true;
}

static int B2_SaveAtomic(const char *name, const void *data, size_t bytes)
{
    char temporary[MAX_OSPATH];
    if (!name || !data || bytes > B2_SAVE_FILE_CAP) return -1;
    FILE *f = NULL;
#ifdef _WIN32
    if (!B2_SavePath534(name, true)) return -1;
    for (unsigned attempt = 0; attempt < 16; ++attempt) {
        if (!B2_SaveUnique534(temporary, sizeof(temporary), name, "new")) return -1;
        f = fopen(temporary, "wbx");
        if (f || errno != EEXIST) break;
    }
#else
    // This candidate targets Windows; retain the existing non-Windows path.
    if (Q_snprintf(temporary, sizeof(temporary), "%s.b2-new", name) >= sizeof(temporary)) return -1;
    f = fopen(temporary, "wbx");
#endif
    if (!f) return -1;
    bool ok = fwrite(data, 1, bytes, f) == bytes;
    if (ok && fflush(f)) ok = false;
#ifdef _WIN32
    if (ok && _commit(_fileno(f))) ok = false;
#endif
    if (fclose(f)) ok = false;
    if (ok) {
#ifdef _WIN32
        ok = MoveFileExA(temporary, name, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        ok = rename(temporary, name) == 0;
#endif
    }
    if (!ok) {
        remove(temporary); // Only the temporary created exclusively above.
        return -1;
    }
    return 0;
}

static bool B2_SaveServerHeader(void)
{
    int version = MSG_ReadLong();
    if (B2_FreshRequested()) {
        if (version == B2_SAVE_VERSION && MSG_ReadLong() == B2_SAVE_REMAP)
            return true;
        Com_Printf("B2 save incompatible: this candidate requires a fresh extended save; old saves were not changed.\n");
        return false;
    }
    return version == SAVE_VERSION;
}

// Return1 only for a genuinely absent file; malformed/read errors are distinct.
// All local memory/files are closed before the caller can raise ERR_DROP.
static int B2_ReadExtendedLevel(const char *name)
{
    byte portal_bits[MAX_MAP_PORTAL_BYTES];
    size_t expected_portals = CM_WritePortalBits(&sv.cm, portal_bits);
    FILE *f = fopen(name, "rb");
    if (!f)
        return errno == ENOENT ? 1 : -1;
    byte *data = malloc(B2_SAVE_FILE_CAP + 1u);
    if (!data) { fclose(f); return -1; }
    size_t bytes = fread(data, 1, B2_SAVE_FILE_CAP + 1u, f);
    bool ok = !ferror(f);
    if (fclose(f)) ok = false;
    b2_save_view_t view;
    if (!ok || !B2_SaveDecode(data, bytes, &svs.csr, sv.configstrings,
                              expected_portals, &view)) {
        free(data);
        return -1;
    }
    memcpy(sv.configstrings, view.strings, (size_t)svs.csr.end * MAX_QPATH);
    SV_ClearWorld();
    // Selected CM_SetPortalStates only reads this owned mutable buffer.
    CM_SetPortalStates(&sv.cm, (byte *)view.portals, (int)view.portal_bytes);
    free(data);
    return 0;
}
