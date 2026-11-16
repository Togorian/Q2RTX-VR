#include "q2rtx-product-version.h"
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Design/direction by Togorian; implementation with Codex assistance.
// Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
// Modified 14 November 2026: provenance/notice clarification only. See PROVENANCE.md.
// Modified 14 November 2026: Q2RTX VR release metadata.
// SPDX-License-Identifier: GPL-2.0-or-later
// Compile the real launcher logic with explicit in-process Windows API stand-ins.
// No process, file, message box or headset is created by this test translation unit.
// Optional payload-root mode reads only non-following metadata from a produced package.
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <stdint.h>

static struct {
    const wchar_t *module;
    int moduleError, moduleTooLong, missingAt, directoryAt, environmentError;
    int reportError, writeError, shortWrite, createError, waitError, exitError;
    int argumentMode;
    int attributes, environments, freedEnvironments, reports, writes, processes;
    int messages, reportClosed, threadClosed, processClosed, invalid;
    DWORD exitCode, attributeError;
} fake;
static const wchar_t *payload_root;
static wchar_t last_message[1024];
// Bind the real read-only metadata API before the launcher API substitution.
static DWORD physical_attributes(LPCWSTR path) { return GetFileAttributesW(path); }
static DWORD payload_attributes(const wchar_t *relative);
static HANDLE report_handle = (HANDLE)(uintptr_t)101;
static HANDLE thread_handle = (HANDLE)(uintptr_t)102;
static HANDLE process_handle = (HANDLE)(uintptr_t)103;
static DWORD WINAPI fake_module(HMODULE module, LPWSTR output, DWORD capacity);
static DWORD WINAPI fake_attributes(LPCWSTR path);
static LPWCH WINAPI fake_environment(void);
static BOOL WINAPI fake_free_environment(LPWCH block);
static HANDLE WINAPI fake_file(LPCWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
static BOOL WINAPI fake_write(HANDLE,LPCVOID,DWORD,LPDWORD,LPOVERLAPPED);
static BOOL WINAPI fake_close(HANDLE);
static BOOL WINAPI fake_create(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION);
static DWORD WINAPI fake_wait(HANDLE,DWORD);
static BOOL WINAPI fake_exit(HANDLE,LPDWORD);
static int WINAPI fake_message(HWND,LPCWSTR,LPCWSTR,UINT);

#define GetModuleFileNameW fake_module
#define GetFileAttributesW fake_attributes
#define GetEnvironmentStringsW fake_environment
#define FreeEnvironmentStringsW fake_free_environment
#define CreateFileW fake_file
#define WriteFile fake_write
#define CloseHandle fake_close
#define CreateProcessW fake_create
#define WaitForSingleObject fake_wait
#define GetExitCodeProcess fake_exit
#define MessageBoxW fake_message
#define wWinMain launcher_entry
#include "../launcher.c"

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"check failed at line %d: %s\n",__LINE__,#x); return 1; } } while (0)

static DWORD WINAPI fake_module(HMODULE module, LPWSTR output, DWORD capacity)
{
    (void)module;
    if (fake.moduleError) { SetLastError(ERROR_ACCESS_DENIED); return 0; }
    if (fake.moduleTooLong) { SetLastError(ERROR_INSUFFICIENT_BUFFER); return capacity; }
    size_t n = wcslen(fake.module);
    if (n >= capacity) { ++fake.invalid; return capacity; }
    memcpy(output, fake.module, (n+1)*sizeof(*output)); return (DWORD)n;
}
static DWORD WINAPI fake_attributes(LPCWSTR path)
{
    ++fake.attributes;
    if (wcsncmp(path,folder,wcslen(folder))) ++fake.invalid;
    if (fake.attributes==fake.missingAt) {
        SetLastError(fake.attributeError ? fake.attributeError : ERROR_FILE_NOT_FOUND);
        return INVALID_FILE_ATTRIBUTES;
    }
    if (fake.attributes==fake.directoryAt) return FILE_ATTRIBUTE_DIRECTORY;
    const wchar_t *relative=path+wcslen(folder)+1;
    // Parent game-data checks remain explicit stand-ins; never inspect an installed home.
    if (payload_root && wcsncmp(relative,L"..\\",3)) return payload_attributes(relative);
    return FILE_ATTRIBUTE_NORMAL;
}
static DWORD payload_attributes(const wchar_t *relative)
{
    wchar_t path[32768];
    if (_snwprintf_s(path,COUNT(path),_TRUNCATE,L"%s\\%s",payload_root,relative)<0) {
        SetLastError(ERROR_FILENAME_EXCED_RANGE); return INVALID_FILE_ATTRIBUTES;
    }
    // Reject reparse components before probing a child. Only metadata is read.
    for (wchar_t *p=path+3;*p;++p) if (*p==L'\\' || *p==L'/') {
        wchar_t saved=*p;*p=0;DWORD attributes=physical_attributes(path);*p=saved;
        if (attributes==INVALID_FILE_ATTRIBUTES) return attributes;
        if (attributes&FILE_ATTRIBUTE_REPARSE_POINT) { SetLastError(ERROR_CANT_ACCESS_FILE);return INVALID_FILE_ATTRIBUTES; }
        if (!(attributes&FILE_ATTRIBUTE_DIRECTORY)) { SetLastError(ERROR_DIRECTORY);return INVALID_FILE_ATTRIBUTES; }
    }
    DWORD attributes=physical_attributes(path);
    if (attributes!=INVALID_FILE_ATTRIBUTES && (attributes&FILE_ATTRIBUTE_REPARSE_POINT)) {
        SetLastError(ERROR_CANT_ACCESS_FILE);return INVALID_FILE_ATTRIBUTES;
    }
    return attributes;
}
static LPWCH WINAPI fake_environment(void)
{
    static const wchar_t original[] = L"=D:=D:\\Games\0Path=D:\\Tools\0TEMP=D:\\Temp path\0sdl_xinput_enabled=1\0SDL_XINPUT_ENABLED_EXTRA=keep\0OTHER=value%literal!\0";
    if (fake.environmentError) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return NULL; }
    wchar_t *p=malloc(sizeof(original));
    if (!p) return NULL;
    memcpy(p,original,sizeof(original)); ++fake.environments; return p;
}
static BOOL WINAPI fake_free_environment(LPWCH block)
{
    ++fake.freedEnvironments; free(block); return TRUE;
}
static HANDLE WINAPI fake_file(LPCWSTR path,DWORD access,DWORD share,LPSECURITY_ATTRIBUTES security,DWORD creation,DWORD attributes,HANDLE templateFile)
{
    (void)attributes;
    ++fake.reports;
    if (!wcsstr(path,L"\\startup-status.txt") || access!=GENERIC_WRITE || share!=FILE_SHARE_READ || security || creation!=CREATE_ALWAYS || templateFile) ++fake.invalid;
    if (fake.reportError) { SetLastError(ERROR_ACCESS_DENIED); return INVALID_HANDLE_VALUE; }
    return report_handle;
}
static BOOL WINAPI fake_write(HANDLE file,LPCVOID bytes,DWORD count,LPDWORD written,LPOVERLAPPED overlapped)
{
    ++fake.writes;
    if (file!=report_handle || overlapped || !strstr(bytes,Q2RTX_LOG_FILE) || strstr(bytes,"headset result")) ++fake.invalid;
    if (fake.writeError) { SetLastError(ERROR_DISK_FULL); *written=0; return FALSE; }
    *written=fake.shortWrite ? count-1 : count; return TRUE;
}
static BOOL WINAPI fake_close(HANDLE handle)
{
    if (handle==report_handle) { if (++fake.reportClosed!=1) ++fake.invalid; }
    else if (handle==thread_handle) { if (++fake.threadClosed!=1) ++fake.invalid; }
    else if (handle==process_handle) { if (++fake.processClosed!=1) ++fake.invalid; }
    else ++fake.invalid;
    return TRUE;
}
static BOOL WINAPI fake_create(LPCWSTR application,LPWSTR line,LPSECURITY_ATTRIBUTES processSecurity,LPSECURITY_ATTRIBUTES threadSecurity,BOOL inherit,DWORD flags,LPVOID env,LPCWSTR cwd,LPSTARTUPINFOW startup,LPPROCESS_INFORMATION process)
{
    ++fake.processes;
    wchar_t expected[32768];
    _snwprintf_s(expected,COUNT(expected),_TRUNCATE,L"%s\\q2rtx.exe",folder);
    if (wcscmp(application,expected) || wcscmp(cwd,folder) || processSecurity || threadSecurity || inherit || flags!=CREATE_UNICODE_ENVIRONMENT || !env || startup->cb!=sizeof(*startup) || fake.reportClosed!=1) ++fake.invalid;
    wchar_t malformed[32768];
    if(fake.argumentMode==1){_snwprintf_s(malformed,COUNT(malformed),_TRUNCATE,L"%s extra",line);line=malformed;}
    if(fake.argumentMode==2)line=L"program +set tm_enable 1 +set loc_draw 0";
    int argc=0; wchar_t **argv=CommandLineToArgvW(line,&argc);
    if (!argv || argc!=COUNT(startup_args)) ++fake.invalid;
    int tm_seen=0,loc_seen=0;
    for(int i=1;argv && i+2<argc;++i)if(!wcscmp(argv[i],L"+set")){
        if(!wcscmp(argv[i+1],L"tm_enable")){tm_seen++;if(wcscmp(argv[i+2],L"1"))++fake.invalid;}
        if(!wcscmp(argv[i+1],L"loc_draw")){loc_seen++;if(wcscmp(argv[i+2],L"0"))++fake.invalid;}
    }
    if(tm_seen!=1 || loc_seen!=1)++fake.invalid;
    if (argv && argc == COUNT(startup_args)) for (int i=0;i<argc;++i) {
        const wchar_t *value=startup_args[i];
        if (!wcsncmp(value,L"%CD%\\",5)) {
            _snwprintf_s(expected,COUNT(expected),_TRUNCATE,L"%s\\%s",folder,value+5); value=expected;
        }
        if (wcscmp(argv[i],value)) ++fake.invalid;
    }
    if (argv) LocalFree(argv);
    int hints[7]={0}, path=0, temp=0, drive=0, extra=0, other=0;
    const wchar_t *previous=NULL;
    for (const wchar_t *p=env;*p;p+=wcslen(p)+1) {
        if (previous && _wcsicmp(previous,p)>0) ++fake.invalid;
        previous=p;
        for (size_t i=0;i<COUNT(child_environment);++i) if (!_wcsicmp(p,child_environment[i])) ++hints[i];
        if (!wcscmp(p,L"Path=D:\\Tools")) ++path;
        if (!wcscmp(p,L"TEMP=D:\\Temp path")) ++temp;
        if (!wcscmp(p,L"=D:=D:\\Games")) ++drive;
        if (!wcscmp(p,L"SDL_XINPUT_ENABLED_EXTRA=keep")) ++extra;
        if (!wcscmp(p,L"OTHER=value%literal!")) ++other;
        if (!_wcsicmp(p,L"SDL_XINPUT_ENABLED=1")) ++fake.invalid;
    }
    for (size_t i=0;i<COUNT(hints);++i) if (hints[i]!=1) ++fake.invalid;
    if (path!=1 || temp!=1 || drive!=1 || extra!=1 || other!=1) ++fake.invalid;
    if (fake.createError) { SetLastError(ERROR_BAD_EXE_FORMAT); return FALSE; }
    process->hProcess=process_handle; process->hThread=thread_handle; return TRUE;
}
static DWORD WINAPI fake_wait(HANDLE process,DWORD timeout)
{
    if (process!=process_handle || timeout!=INFINITE) ++fake.invalid;
    if (fake.waitError) { SetLastError(ERROR_INVALID_HANDLE); return WAIT_FAILED; }
    return WAIT_OBJECT_0;
}
static BOOL WINAPI fake_exit(HANDLE process,LPDWORD code)
{
    if (process!=process_handle) ++fake.invalid;
    if (fake.exitError) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
    *code=fake.exitCode; return TRUE;
}
static int WINAPI fake_message(HWND parent,LPCWSTR text,LPCWSTR caption,UINT flags)
{
    (void)flags;
    if (parent || !text || wcscmp(caption,L"Q2RTX VR")) ++fake.invalid;
    wcsncpy_s(last_message,COUNT(last_message),text,_TRUNCATE);
    ++fake.messages; return IDOK;
}

static void reset(void)
{
    memset(&fake,0,sizeof(fake));fake.module=L"D:\\Games & 测试 %!\\Q2RTX VR\\Q2RTX VR.exe"; fake.exitCode=17;
    command[0]=folder[0]=joined[0]=last_message[0]=0;
}
static int quote_checks(void)
{
    const wchar_t *values[]={L"",L"ordinary",L"with space",L"a\"b",L"trailing\\",L"\\\\\\\\",L"slash\\\"quote",L"\u6d4b\u8bd5 and space",L"%PATH% & !"};
    for (size_t i=0;i<COUNT(values);++i) {
        size_t used=0;command[0]=0;
        CHECK(argument(L"program.exe",&used));CHECK(argument(values[i],&used));
        int argc=0;wchar_t **argv=CommandLineToArgvW(command,&argc);
        CHECK(argv && argc==2);CHECK(!wcscmp(argv[1],values[i]));LocalFree(argv);
    }
    static wchar_t tooLong[22001];
    for (size_t i=0;i<COUNT(tooLong)-1;++i) tooLong[i]=L'\\';
    size_t used=0;command[0]=0;CHECK(!argument(tooLong,&used));CHECK(used<LIMIT);CHECK(command[used]==0);
    CHECK(overridden(L"sdl_xinput_enabled=anything"));CHECK(!overridden(L"SDL_XINPUT_ENABLED_EXTRA=keep"));CHECK(!overridden(L"S=short"));
    return 0;
}
static int package_contract_checks(void)
{
    static const wchar_t *configs[] = {L"home\\baseq2\\default-vr.cfg",L"home\\baseq2\\autoexec.cfg",L"home\\baseq2\\postexec.cfg",L"home\\baseq2\\postinit.cfg",L"home\\baseq2\\q2rtx.cfg",L"home\\baseq2\\RESET-KEYS476.cfg"};
    CHECK(COUNT(package_files)==51+5+COUNT(configs));
    for(size_t c=0;c<COUNT(configs);++c){size_t hits=0;for(size_t i=0;i<COUNT(package_files);++i)hits+=!wcscmp(configs[c],package_files[i]);CHECK(hits==1);}
    return 0;
}
static int entry_checks(void)
{
    CHECK(package_contract_checks()==0);
    reset();CHECK(launcher_entry(NULL,NULL,L"ignored",0)==17);
    CHECK(fake.attributes==COUNT(package_files)+COUNT(data_files));CHECK(fake.processes==1 && fake.messages==0);
    CHECK(fake.reportClosed==1 && fake.threadClosed==1 && fake.processClosed==1 && !fake.invalid);
    CHECK(fake.environments==fake.freedEnvironments);
    for (int scenario=0;scenario<12;++scenario) {
        reset();
        switch(scenario) {
        case 0:fake.moduleError=1;break;
        case 1:fake.moduleTooLong=1;break;
        case 2:fake.module=L"no-directory.exe";break;
        case 3:fake.missingAt=1;break;
        case 4:fake.missingAt=(int)COUNT(package_files)+1;break;
        case 5:fake.directoryAt=1;break;
        case 6:fake.environmentError=1;break;
        case 7:fake.reportError=1;break;
        case 8:fake.writeError=1;break;
        case 9:fake.shortWrite=1;break;
        case 10:fake.createError=1;break;
        case 11:fake.waitError=1;break;
        }
        CHECK(launcher_entry(NULL,NULL,L"",0)==2);CHECK(fake.messages==1 && !fake.invalid);
        CHECK(fake.environments==fake.freedEnvironments);
        CHECK(fake.processes==(scenario>=10));
        CHECK(fake.threadClosed==(scenario==11));CHECK(fake.processClosed==(scenario==11));
        CHECK(fake.reportClosed==(scenario>=8));
    }
    reset();fake.exitError=1;CHECK(launcher_entry(NULL,NULL,L"",0)==2);
    CHECK(fake.threadClosed==1 && fake.processClosed==1 && fake.messages==1 && !fake.invalid);
    reset();fake.exitCode=0;CHECK(launcher_entry(NULL,NULL,L"",0)==0);CHECK(!fake.invalid);
    // Both malformed counts retain one valid tm/loc override; neither may index
    // beyond the exact production array or silently pass the argument check.
    for(int mode=1;mode<=2;++mode){reset();fake.argumentMode=mode;CHECK(launcher_entry(NULL,NULL,L"",0)==17);CHECK(fake.invalid==1);}
    return 0;
}
static int payload_checks(int expect_stale)
{
    size_t absent=0;
    for (size_t i=0;i<COUNT(package_files);++i)
        if (payload_attributes(package_files[i])==INVALID_FILE_ATTRIBUTES) ++absent;
    reset();
    if (expect_stale) {
        CHECK(absent==2);CHECK(launcher_entry(NULL,NULL,L"",0)==2);
        CHECK(fake.messages==1 && fake.processes==0 && fake.reports==0);
        puts("Baseline actual launcher refuses the preserved RC5 payload: exactly two retired shader requirements absent.");
        return 0;
    }
    CHECK(absent==0);CHECK(package_contract_checks()==0);
    CHECK(launcher_entry(NULL,NULL,L"",0)==17);
    CHECK(fake.processes==1 && fake.messages==0 && !fake.invalid);
    for (size_t i=0;i<COUNT(package_files);++i) {
        reset();fake.missingAt=(int)i+1;
        CHECK(launcher_entry(NULL,NULL,L"",0)==2);
        CHECK(fake.processes==0 && fake.reports==0 && fake.messages==1 && !fake.invalid);
        CHECK(wcsstr(last_message,package_files[i]) && wcsstr(last_message,L"Windows error: 2"));
    }
    reset();fake.missingAt=1;fake.attributeError=ERROR_ACCESS_DENIED;
    CHECK(launcher_entry(NULL,NULL,L"",0)==2);
    CHECK(wcsstr(last_message,L"q2rtx.exe") && wcsstr(last_message,L"Windows error: 5"));
    CHECK(fake.processes==0 && fake.reports==0);
    reset();fake.directoryAt=1;CHECK(launcher_entry(NULL,NULL,L"",0)==2);
    CHECK(wcsstr(last_message,L"q2rtx.exe") && wcsstr(last_message,L"Windows error: 267"));
    CHECK(fake.processes==0 && fake.reports==0);
    reset();fake.missingAt=(int)COUNT(package_files)+1;
    CHECK(launcher_entry(NULL,NULL,L"",0)==2);
    CHECK(fake.processes==0 && fake.reports==0 && wcsstr(last_message,L"parent baseq2"));
    for (size_t i=0;i<COUNT(folder)-2;++i) folder[i]=L'x';
    folder[COUNT(folder)-2]=0;
    CHECK(!ordinary_file(L"q2rtx.exe") && GetLastError()==ERROR_FILENAME_EXCED_RANGE);
    puts("Produced-payload boundary passed: all required package paths, every required-file omission refused, named errors2/5/267, path206 and owned-parent refusal retained.");
    return 0;
}
int wmain(int argc,wchar_t **argv)
{
    if (argc==3 && !wcscmp(argv[2],L"--expect-stale")) {payload_root=argv[1];return payload_checks(1);}
    if (argc!=1 && argc!=2) return 2;
    if (quote_checks() || entry_checks()) return 1;
    puts("Actual launcher quoting, paths, environment, errors, handles and exit checks passed.");
    if (argc==2) {payload_root=argv[1];return payload_checks(0);}
    return 0;
}
