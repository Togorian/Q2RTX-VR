// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Retained CPU fixture: selected production spans are verified by rc7-regression.py.
#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#define MAX_OSPATH 1024
#define Q_stricmp _stricmp
#define SAVE_CURRENT ".current"
static char fs_gamedir[MAX_OSPATH];
static struct {char *string;} savedir={"save"}, *sv_savedir=&savedir;
static int logs, fail_move;
static void Com_EPrintf(const char *f,...){logs++;}
static int Q_snprintf(char *out,size_t n,const char *f,...){va_list a;va_start(a,f);int r=vsnprintf(out,n,f,a);va_end(a);return r;}

static int queries, mode;
static DWORD attrs(const char *name){assert(strlen(name)>strlen(fs_gamedir));queries++;if(mode==1)return FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT;if(mode==2)return FILE_ATTRIBUTE_NORMAL;return FILE_ATTRIBUTE_DIRECTORY;}
#define GetFileAttributesA attrs
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

int main(void){strcpy(fs_gamedir,"C:\\linked-home\\baseq2");assert(B2_SavePath534("C:\\linked-home\\baseq2\\save\\slot",false));assert(queries==2);
strcpy(fs_gamedir,"\\\\server\\share\\home\\baseq2");queries=0;assert(B2_SavePath534("\\\\server\\share\\home\\baseq2\\save\\slot",false));assert(queries==2);
mode=1;assert(!B2_SavePath534("\\\\server\\share\\home\\baseq2\\save\\slot",false));mode=2;assert(!B2_SavePath534("\\\\server\\share\\home\\baseq2\\save\\slot",false));
assert(!B2_SavePath534("\\\\server\\share\\home\\baseq2evil\\slot",false));puts("actual path function: no ancestor query, UNC accepted, owned reparse/non-directory refused");return 0;}
