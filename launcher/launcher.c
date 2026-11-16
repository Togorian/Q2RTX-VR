// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Design/direction by Togorian; implementation with Codex assistance.
// Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
// Modified 14 November 2026: provenance/notice clarification only. See PROVENANCE.md.
// Modified 14 November 2026: Q2RTX VR release metadata.
// SPDX-License-Identifier: GPL-2.0-or-later
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include "accepted-startup.h"

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define LIMIT 32768
static wchar_t folder[LIMIT], command[LIMIT], joined[LIMIT];

static int fail(const wchar_t *message, DWORD code)
{
    wchar_t text[1024];
    _snwprintf_s(text, COUNT(text), _TRUNCATE, L"%s\n\nWindows error: %lu", message, code);
    MessageBoxW(NULL, text, L"Q2RTX VR", MB_OK | MB_ICONERROR);
    return 2;
}

static int append(wchar_t ch, size_t *used)
{
    if (*used + 1 >= COUNT(command)) return 0;
    command[(*used)++] = ch;
    command[*used] = 0;
    return 1;
}

// Windows CRT argument quoting: double trailing slashes and slashes before quotes.
static int argument(const wchar_t *text, size_t *used)
{
    if (*used && !append(L' ', used)) return 0;
    if (!append(L'"', used)) return 0;
    for (;;) {
        size_t slashes = 0;
        while (*text == L'\\') { ++slashes; ++text; }
        size_t emit = (*text == 0 || *text == L'"') ? slashes * 2 : slashes;
        while (emit--) if (!append(L'\\', used)) return 0;
        if (!*text) break;
        if (*text == L'"' && !append(L'\\', used)) return 0;
        if (!append(*text++, used)) return 0;
    }
    return append(L'"', used);
}

static int local_path(const wchar_t *relative)
{
    return _snwprintf_s(joined, COUNT(joined), _TRUNCATE, L"%s\\%s", folder, relative) >= 0;
}

static int ordinary_file(const wchar_t *relative)
{
    if (!local_path(relative)) { SetLastError(ERROR_FILENAME_EXCED_RANGE); return 0; }
    DWORD attributes = GetFileAttributesW(joined);
    if (attributes == INVALID_FILE_ATTRIBUTES) return 0;
    if (attributes & FILE_ATTRIBUTE_DIRECTORY) { SetLastError(ERROR_DIRECTORY); return 0; }
    return 1;
}

static int fail_package_file(const wchar_t *relative, DWORD code)
{
    wchar_t message[768];
    _snwprintf_s(message, COUNT(message), _TRUNCATE,
        L"This package cannot start. Restore the complete Q2RTX VR folder.\n\nRequired file: %s", relative);
    return fail(message, code);
}

static int overridden(const wchar_t *entry)
{
    for (size_t i = 0; i < COUNT(child_environment); ++i) {
        size_t length = (size_t)(wcschr(child_environment[i], L'=') - child_environment[i]);
        if (!_wcsnicmp(entry, child_environment[i], length) && entry[length] == L'=') return 1;
    }
    return 0;
}

static int compare_environment(const void *a, const void *b)
{
    return _wcsicmp(*(const wchar_t *const *)a, *(const wchar_t *const *)b);
}

// Clone inherited discovery/profile variables and override only the accepted SDL hints.
static wchar_t *environment(void)
{
    wchar_t *original = GetEnvironmentStringsW();
    if (!original) return NULL;
    size_t count = COUNT(child_environment), chars = 1;
    for (const wchar_t *p = original; *p; p += wcslen(p) + 1)
        if (!overridden(p)) { ++count; chars += wcslen(p) + 1; }
    for (size_t i = 0; i < COUNT(child_environment); ++i) chars += wcslen(child_environment[i]) + 1;
    const wchar_t **entries = calloc(count, sizeof(*entries));
    wchar_t *block = calloc(chars, sizeof(*block));
    if (!entries || !block) { free(entries); free(block); FreeEnvironmentStringsW(original); return NULL; }
    size_t n = 0;
    for (const wchar_t *p = original; *p; p += wcslen(p) + 1)
        if (!overridden(p)) entries[n++] = p;
    for (size_t i = 0; i < COUNT(child_environment); ++i) entries[n++] = child_environment[i];
    qsort(entries, n, sizeof(*entries), compare_environment);
    wchar_t *next = block;
    for (size_t i = 0; i < n; ++i) {
        size_t length = wcslen(entries[i]) + 1;
        memcpy(next, entries[i], length * sizeof(*next)); next += length;
    }
    *next = 0;
    free(entries); FreeEnvironmentStringsW(original);
    return block;
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR commandLine, int show)
{
    (void)instance; (void)previous; (void)commandLine; (void)show;
    DWORD length = GetModuleFileNameW(NULL, folder, COUNT(folder));
    if (!length || length >= COUNT(folder)) return fail(L"The launcher location could not be read.", GetLastError());
    wchar_t *slash = wcsrchr(folder, L'\\');
    if (!slash) return fail(L"Place the complete Q2RTX VR folder in your owned Quake II RTX installation.", ERROR_BAD_PATHNAME);
    *slash = 0;
    for (size_t i = 0; i < COUNT(package_files); ++i)
        if (!ordinary_file(package_files[i])) return fail_package_file(package_files[i], GetLastError());
    for (size_t i = 0; i < COUNT(data_files); ++i)
        if (!ordinary_file(data_files[i])) return fail(L"Place Q2RTX VR directly inside your owned Quake II RTX installation. Its parent baseq2 needs the full game and RTX media.", ERROR_FILE_NOT_FOUND);

    size_t used = 0;
    for (size_t i = 0; i < COUNT(startup_args); ++i) {
        const wchar_t *value = startup_args[i];
        if (!wcsncmp(value, L"%CD%\\", 5)) {
            if (!local_path(value + 5)) return fail(L"The installation path is too long.", ERROR_FILENAME_EXCED_RANGE);
            value = joined;
        }
        if (!argument(value, &used)) return fail(L"The startup command is too long.", ERROR_FILENAME_EXCED_RANGE);
    }
    wchar_t *env = environment();
    if (!env) return fail(L"The child environment could not be prepared.", ERROR_NOT_ENOUGH_MEMORY);
    if (!local_path(L"startup-status.txt")) { free(env); return fail(L"The installation path is too long.", ERROR_FILENAME_EXCED_RANGE); }
    HANDLE report = CreateFileW(joined, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (report == INVALID_HANDLE_VALUE) { DWORD error = GetLastError(); free(env); return fail(L"This folder needs write access for its own startup report and saves.", error); }
    static const char status[] = "Q2RTX VR v" Q2RTX_PRODUCT_VERSION "\r\n"
        "Package and parent game-data checks passed.\r\n"
        "Starting q2rtx.exe with the local home directory.\r\n"
        "Game log: home/baseq2/logs/" Q2RTX_LOG_FILE " (append within this version).\r\n";
    DWORD written = 0;
    BOOL recorded = WriteFile(report, status, (DWORD)(sizeof(status) - 1), &written, NULL);
    DWORD writeError = recorded ? ERROR_WRITE_FAULT : GetLastError();
    CloseHandle(report);
    if (!recorded || written != sizeof(status) - 1) { free(env); return fail(L"The startup report could not be written.", writeError); }
    if (!local_path(L"q2rtx.exe")) { free(env); return fail(L"The installation path is too long.", ERROR_FILENAME_EXCED_RANGE); }
    STARTUPINFOW startup = { sizeof(startup) };
    PROCESS_INFORMATION process = { 0 };
    BOOL launched = CreateProcessW(joined, command, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT,
                                  env, folder, &startup, &process);
    DWORD error = launched ? ERROR_SUCCESS : GetLastError();
    free(env);
    if (!launched) return fail(L"Q2RTX VR could not start.", error);
    CloseHandle(process.hThread);
    DWORD waited = WaitForSingleObject(process.hProcess, INFINITE), result = 2;
    if (waited == WAIT_FAILED) error = GetLastError();
    else if (!GetExitCodeProcess(process.hProcess, &result)) error = GetLastError();
    else error = ERROR_SUCCESS;
    CloseHandle(process.hProcess);
    if (error) return fail(L"The game exit status could not be read.", error);
    return (int)result;
}
