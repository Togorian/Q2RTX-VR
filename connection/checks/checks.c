// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
// Test glue, Togorian with Codex assistance, 11 November 2026.
// Actual production cvar/shared/utils functions; no engine/network/file/GPU run.
#include "q2rtx-product-version.h"
#include "shared/shared.h"
#include "common/common.h"
#include "common/cvar.h"
#include "common/cmd.h"
#include "common/files.h"
#include "common/zone.h"
#include "common/prompt.h"
#include "client/client.h"
bool com_initialized;
cvar_t *com_version;
static cvar_t restriction;
static cvar_t *sv_restrict_rtx = &restriction;
static const char *last_rejection;
static unsigned unexpected_boundary_calls;
// Only allocation/diagnostic/prompt/client effects are stand-ins.
void *Z_TagMalloc(size_t size, memtag_t tag)
{
    (void)tag;
    void *p = calloc(1, size);
    if (!p) abort();
    return p;
}
char *Z_CvarCopyString(const char *in)
{
    size_t n = strlen(in) + 1;
    char *p = malloc(n);
    if (!p) abort();
    memcpy(p, in, n); return p;
}
void Z_Free(void *p) { free(p); }
void Prompt_AddMatch(genctx_t *ctx, const char *s)
{
    (void)ctx; (void)s; ++unexpected_boundary_calls;
}
bool CL_CheatsOK(void) { ++unexpected_boundary_calls; return false; }
void CL_UpdateUserinfo(cvar_t *var, from_t from)
{
    (void)var; (void)from; ++unexpected_boundary_calls;
}
void Com_LPrintf(print_type_t type, const char *fmt, ...)
{
    (void)type; (void)fmt; ++unexpected_boundary_calls;
}
void Com_Error(error_type_t code, const char *fmt, ...)
{
    (void)code; (void)fmt; abort();
}
// Unrelated command/file imports from the complete cvar translation unit.
// These trap if reached: the tested producer/serializer/parser are not replaced.
cvar_t *sv_running;
cvar_t *fs_game;
int cmd_optind;
__declspec(noreturn) static void forbidden_effect(const char *name)
{
    fprintf(stderr, "Unexpected unrelated boundary: %s\n", name);
    abort();
}
void Cmd_Option_c(const cmd_option_t *opt, xgenerator_t g, genctx_t *ctx, int argnum)
{ (void)opt; (void)g; (void)ctx; (void)argnum; forbidden_effect("Cmd_Option_c"); }
void Cmd_Register(const cmdreg_t *reg)
{ (void)reg; forbidden_effect("Cmd_Register"); }
from_t Cmd_From(void) { forbidden_effect("Cmd_From"); }
int Cmd_Argc(void) { forbidden_effect("Cmd_Argc"); }
char *Cmd_Argv(int arg) { (void)arg; forbidden_effect("Cmd_Argv"); }
char *Cmd_ArgsFrom(int from) { (void)from; forbidden_effect("Cmd_ArgsFrom"); }
int Cmd_ParseOptions(const cmd_option_t *opt)
{ (void)opt; forbidden_effect("Cmd_ParseOptions"); }
void Cmd_PrintHelp(const cmd_option_t *opt)
{ (void)opt; forbidden_effect("Cmd_PrintHelp"); }
void Cmd_PrintUsage(const cmd_option_t *opt, const char *suffix)
{ (void)opt; (void)suffix; forbidden_effect("Cmd_PrintUsage"); }
void Z_Freep(void **ptr) { (void)ptr; forbidden_effect("Z_Freep"); }
int FS_FPrintf(qhandle_t f, const char *format, ...)
{ (void)f; (void)format; forbidden_effect("FS_FPrintf"); }
static bool reject(const char *message)
{
    last_rejection = message; return false;
}
#include "production-spans.inc"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "check failed at line %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main(int argc, char **argv)
{
    char userinfo[MAX_INFO_STRING];
    unsigned cases = 0;
    int numbers[] = { Q2RTX_PRODUCT_VERSION_RESOURCE };
    int major, minor, patch, rc;
    CHECK(argc == 3);
    const char *expected_version = argv[1], *old_rc3_value = argv[2];
    CHECK(strcmp(Q2RTX_PRODUCT_VERSION, expected_version) == 0);
    CHECK(sscanf(expected_version, "%d.%d.%d-rc.%d", &major, &minor, &patch, &rc) == 4);
    CHECK(numbers[0] == major && numbers[1] == minor && numbers[2] == patch && numbers[3] == rc);
    CHECK(strncmp(old_rc3_value, "Q2RTX VR 0.8.4-rc.3 ", 19) == 0);
    CHECK(strncmp(com_version_string, Q2RTX_PRODUCT_LABEL, strlen(Q2RTX_PRODUCT_LABEL)) == 0);
    CHECK(strcmp(Q2RTX_PRODUCT_LABEL, "Q2RTX VR " Q2RTX_PRODUCT_VERSION) == 0);
    CHECK(strcmp(Q2RTX_CLIENT_VERSION, "q2rtx " Q2RTX_PRODUCT_VERSION) == 0);
    CHECK(strcmp(Q2RTX_LOG_FILE, "console" Q2RTX_PRODUCT_VERSION ".log") == 0);
    restriction.integer = 1;
    register_actual_version();
    CHECK(com_version != NULL);
    CHECK(com_version->flags == (CVAR_SERVERINFO | CVAR_USERINFO | CVAR_ROM));
    CHECK(strcmp(com_version->string, com_client_version_string) == 0);
    CHECK(Cvar_Get("name", "ConnectionFixture", CVAR_USERINFO) != NULL);
    size_t generated_len = Cvar_BitInfo(userinfo, CVAR_USERINFO);
    CHECK(generated_len == strlen(userinfo));
    CHECK(Info_Validate(userinfo));
    CHECK(strcmp(Info_ValueForKey(userinfo, "version"), com_client_version_string) == 0);
    CHECK(actual_server_version_restriction(userinfo)); ++cases;
    generated_len = Cvar_BitInfo(userinfo, CVAR_SERVERINFO);
    CHECK(generated_len == strlen(userinfo));
    CHECK(strcmp(Info_ValueForKey(userinfo, "version"), com_client_version_string) == 0);
    CHECK(actual_server_version_restriction(userinfo)); ++cases;
    const char *values[] = { old_rc3_value, "q2rtx 1.8.1 legacy", "q2rtx", "incompatible-client", "", "Q2RTX 0.8.4-rc.4", "q2rt" };
    const bool expected[] = { false, true, true, false, false, false, false };
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        Cvar_BitInfo(userinfo, CVAR_USERINFO);
        CHECK(Info_SetValueForKey(userinfo, "version", values[i]));
        CHECK(Info_Validate(userinfo));
        CHECK(strcmp(Info_ValueForKey(userinfo, "version"), values[i]) == 0);
        last_rejection = NULL;
        CHECK(actual_server_version_restriction(userinfo) == expected[i]);
        if (!expected[i])
            CHECK(last_rejection && strcmp(last_rejection, "This server is only available to Q2RTX clients.\n") == 0);
        else
            CHECK(last_rejection == NULL);
        ++cases;
    }
    CHECK(restriction.integer == 1 && unexpected_boundary_calls == 0);
    printf("PASS %u actual-boundary cases; product=%s; display=%s; userinfo=%s; resource=%d,%d,%d,%d; old_rc3_refused=1; restriction_preserved=1\n",
        cases, Q2RTX_PRODUCT_VERSION, Q2RTX_PRODUCT_LABEL, com_version->string, numbers[0], numbers[1], numbers[2], numbers[3]);
    cvar_t *var = cvar_vars;
    while (var) {
        cvar_t *next = var->next;
        Z_Free(var->string); Z_Free(var->default_string); Z_Free(var->latched_string); Z_Free(var);
        var = next;
    }
    return 0;
}
