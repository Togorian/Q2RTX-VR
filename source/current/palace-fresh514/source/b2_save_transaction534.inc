// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
// Included after stock save-copy helpers. The directory commit uses two moves;
// a process/power interruption between them leaves the prior slot at .b2-*.old.
// Recovery is explicit: never infer ownership of or remove an earlier stage.
#ifdef _WIN32
static bool B2_SaveComponent534(const char *name)
{
    if (!name || !name[0] || name[0] == '.' || strlen(name) > 64) return false;
    for (const unsigned char *p = (const unsigned char *)name; *p; ++p)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
            (*p >= '0' && *p <= '9') || *p == '-' || *p == '_' || *p == '.')) return false;
    return name[strlen(name)-1] != '.';
}
static bool B2_SaveMember534(const char *name)
{
    const char *ext = strrchr(name, '.');
    return ext && (!Q_stricmp(ext, ".ssv") || !Q_stricmp(ext, ".sav") || !Q_stricmp(ext, ".sv2"));
}

// Operation 0 validates, 1 copies to our exclusively created stage, 2 cleans
// known save files in a directory owned by this transaction. Unknown files stay.
static bool B2_SaveTree534(const char *src, const char *dst, unsigned depth, unsigned *entries, int operation)
{
    char pattern[MAX_OSPATH], from[MAX_OSPATH], to[MAX_OSPATH];
    WIN32_FIND_DATAA item;
    if (depth > 8 || !B2_SavePath534(src, false) ||
        Q_snprintf(pattern, sizeof(pattern), "%s/*", src) >= sizeof(pattern)) return false;
    HANDLE search = FindFirstFileA(pattern, &item);
    if (search == INVALID_HANDLE_VALUE) return GetLastError() == ERROR_FILE_NOT_FOUND;
    bool ok = true;
    do {
        if (!strcmp(item.cFileName, ".") || !strcmp(item.cFileName, "..")) continue;
        if (++*entries > 512 || (item.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) { ok = false; break; }
        // Unrecognised ordinary files are not save members. Leave them in the
        // original/backup directory, including names containing spaces. Never
        // follow links or relax the slot/directory component checks.
        if (!(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            (!B2_SaveComponent534(item.cFileName) || !B2_SaveMember534(item.cFileName))) continue;
        if (!B2_SaveComponent534(item.cFileName) ||
            Q_snprintf(from, sizeof(from), "%s/%s", src, item.cFileName) >= sizeof(from) ||
            !B2_SavePath534(from, false)) { ok = false; break; }
        if (operation == 1 && Q_snprintf(to, sizeof(to), "%s/%s", dst, item.cFileName) >= sizeof(to)) { ok = false; break; }
        if (item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (operation == 1 && !CreateDirectoryA(to, NULL)) { ok = false; break; }
            if (!B2_SaveTree534(from, operation == 1 ? to : NULL, depth+1, entries, operation)) { ok = false; break; }
            if (operation == 2 && !RemoveDirectoryA(from)) { ok = false; break; }
        } else if (B2_SaveMember534(item.cFileName)) {
            if (operation == 1) {
                if (!B2_SavePath534(to, true) || !CopyFileA(from, to, TRUE)) { ok = false; break; }
                HANDLE file = CreateFileA(to, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (file == INVALID_HANDLE_VALUE) { ok = false; break; }
                bool flushed = FlushFileBuffers(file) != 0;
                if (!CloseHandle(file) || !flushed) { ok = false; break; }
            } else if (operation == 2 && !DeleteFileA(from)) { ok = false; break; }
        }
    } while (FindNextFileA(search, &item));
    if (ok && GetLastError() != ERROR_NO_MORE_FILES) ok = false;
    if (!FindClose(search)) ok = false;
    return ok;
}

static bool B2_SaveRequired534(const char *dir)
{
    static const char *const required[] = {"server.ssv", "game.ssv"};
    for (unsigned i = 0; i < 2; ++i) {
        char path[MAX_OSPATH]; WIN32_FILE_ATTRIBUTE_DATA data;
        if (Q_snprintf(path, sizeof(path), "%s/%s", dir, required[i]) >= sizeof(path) ||
            !B2_SavePath534(path, false) || !GetFileAttributesExA(path, GetFileExInfoStandard, &data) ||
            (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !(data.nFileSizeHigh || data.nFileSizeLow)) return false;
    }
    return true;
}

static void B2_SaveCleanOwned534(const char *path)
{
    unsigned entries = 0;
    if (!B2_SaveTree534(path, NULL, 0, &entries, 2) || !RemoveDirectoryA(path))
        Com_EPrintf("B2 save: retained transaction directory for inspection: %s\n", path);
}

static int B2_SaveReplace534(const char *slot)
{
    char root[MAX_OSPATH], source[MAX_OSPATH], destination[MAX_OSPATH];
    char stage[MAX_OSPATH], backup[MAX_OSPATH];
    unsigned entries = 0;
    if (!B2_SaveComponent534(slot) ||
        Q_snprintf(root, sizeof(root), "%s/%s", fs_gamedir, sv_savedir->string) >= sizeof(root) ||
        Q_snprintf(source, sizeof(source), "%s/%s", root, SAVE_CURRENT) >= sizeof(source) ||
        Q_snprintf(destination, sizeof(destination), "%s/%s", root, slot) >= sizeof(destination) ||
        !B2_SavePath534(destination, true) || !B2_SaveRequired534(source) ||
        !B2_SaveTree534(source, NULL, 0, &entries, 0)) {
        Com_EPrintf("Save refused: invalid slot name, inaccessible save path, or incomplete current save.\n");
        return -1;
    }
    DWORD attributes = GetFileAttributesA(destination);
    bool existed = attributes != INVALID_FILE_ATTRIBUTES;
    entries = 0;
    if (existed && (!(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
        !B2_SaveTree534(destination, NULL, 0, &entries, 0))) {
        Com_EPrintf("Save refused: existing slot contains an unsafe or inaccessible entry; prior save retained.\n");
        return -1;
    }
    bool created = false;
    for (unsigned attempt = 0; attempt < 16; ++attempt) {
        if (!B2_SaveUnique534(stage, sizeof(stage), destination, "stage")) return -1;
        if (CreateDirectoryA(stage, NULL)) { created = true; break; }
        if (GetLastError() != ERROR_ALREADY_EXISTS) return -1;
    }
    if (!created) return -1;
    if (Q_snprintf(backup, sizeof(backup), "%s.old", stage) >= sizeof(backup) ||
        !B2_SavePath534(backup, true) || GetFileAttributesA(backup) != INVALID_FILE_ATTRIBUTES) {
        B2_SaveCleanOwned534(stage); return -1;
    }
    entries = 0;
    if (!B2_SaveTree534(source, stage, 0, &entries, 1) || !B2_SaveRequired534(stage)) {
        Com_EPrintf("B2 save: replacement copy failed; prior slot retained.\n");
        B2_SaveCleanOwned534(stage); return -1;
    }
    // No replacement flags on directory moves: never overwrite unknown state.
    if (existed && !MoveFileExA(destination, backup, MOVEFILE_WRITE_THROUGH)) {
        B2_SaveCleanOwned534(stage); return -1;
    }
    if (!MoveFileExA(stage, destination, MOVEFILE_WRITE_THROUGH)) {
        bool restored = !existed || MoveFileExA(backup, destination, MOVEFILE_WRITE_THROUGH);
        Com_EPrintf("B2 save: commit failed; prior slot %s. Recovery paths: %s and %s\n",
            restored ? "restored" : "retained in backup (restore required)", backup, stage);
        // Complete stage is retained for recovery; do not erase either copy.
        return restored?-1:-2; // Distinguish recovery needed in player feedback.
    }
    if (existed) B2_SaveCleanOwned534(backup);
    return 0;
}
#endif

static int replace_save_dir534(const char *slot)
{
#ifdef _WIN32
    if (B2_FreshRequested()) return B2_SaveReplace534(slot);
#endif
    if (wipe_save_dir(slot)) return -1;
    return copy_save_dir(SAVE_CURRENT, slot);
}
