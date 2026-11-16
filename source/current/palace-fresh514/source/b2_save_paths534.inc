// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
// Windows saves: check game-owned descendants, allowing linked home ancestors. These
// local save operations are not a security boundary against a hostile process.
#ifdef _WIN32
#include <io.h>
static bool B2_SavePath534(const char *path, bool missing_leaf)
{
    char full[MAX_OSPATH], root[MAX_OSPATH];
    DWORD n = GetFullPathNameA(path, sizeof(full), full, NULL);
    DWORD r = GetFullPathNameA(fs_gamedir, sizeof(root), root, NULL);
    const char *reason = "invalid or overlong path";
    if (!n || n >= sizeof(full) || !r || r >= sizeof(root)) goto refuse;
    for (DWORD i = 0; i < n; ++i) if (full[i] == '/') full[i] = '\\';
    for (DWORD i = 0; i < r; ++i) if (root[i] == '/') root[i] = '\\';
    while (r && root[r-1] == '\\') root[--r] = 0;
    reason = "path is outside the game's writable folder";
    if (!r || n <= r || _strnicmp(full, root, r) || full[r] != '\\') goto refuse;
    // Installation/home ancestors may be junctions or network shares. Only
    // game-owned descendants are checked; the home location remains user-owned.
    for (DWORD i = r + 1; i <= n; ++i) {
        if (full[i] != '\\' && full[i]) continue;
        char separator = full[i]; full[i] = 0;
        DWORD attr = GetFileAttributesA(full);
        DWORD error = GetLastError();
        full[i] = separator;
        if (attr == INVALID_FILE_ATTRIBUTES) {
            if (i == n && missing_leaf && error == ERROR_FILE_NOT_FOUND) return true;
            reason = "a save folder or file is missing or inaccessible"; goto refuse;
        }
        if (attr & FILE_ATTRIBUTE_REPARSE_POINT) {
            reason = "a save folder or file is a link"; goto refuse;
        }
        if (i < n && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            reason = "a save folder is not a directory"; goto refuse;
        }
    }
    return true;
refuse:
    Com_EPrintf("Save refused: %s (%s).\n", reason, path ? path : "no path");
    return false;
}

static unsigned b2_save_sequence534;
static bool B2_SaveUnique534(char *out, size_t capacity, const char *base, const char *kind)
{
    if (b2_save_sequence534 == UINT_MAX) return false;
    return Q_snprintf(out, capacity, "%s.b2-%s-%lu-%llu-%u", base, kind,
        (unsigned long)GetCurrentProcessId(), (unsigned long long)GetTickCount64(), ++b2_save_sequence534) < capacity;
}
#endif
