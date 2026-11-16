// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 14 November 2026: historical RC6 fixture regenerated from the selected
// Q2RTX VR/Quake II RTX source spans listed in R6.json; external effects are seams.
/*
Copyright (C) 1997-2001 Id Software, Inc.

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
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#define FROM_CMDLINE 1
#define Com_Printf(...) ((void)0)
static int tm,loc;
static void Cvar_SetEx(const char *n,const char *v,int from){assert(from==FROM_CMDLINE);if(!strcmp(n,"tm_enable"))tm=!strcmp(v,"1");if(!strcmp(n,"loc_draw"))loc=atoi(v);}
#include <wchar.h>
#include "accepted-startup.h"
enum { STARTUP_COUNT = sizeof(startup_args)/sizeof(startup_args[0]) };
static char argument_storage[STARTUP_COUNT][256];
static char *com_argv[STARTUP_COUNT];
static int com_argc=STARTUP_COUNT;
static void actual_startup(void) {
    for (size_t i=0;i<STARTUP_COUNT;i++) {
        size_t n=wcstombs(argument_storage[i],startup_args[i],sizeof(argument_storage[i]));
        assert(n!=(size_t)-1 && n<sizeof(argument_storage[i]));
        com_argv[i]=argument_storage[i];
    }
    int logs=0;
    for (int i=1;i<com_argc;i++) {
        if (!strcmp(com_argv[i],"+set")) {
            assert(i+2<com_argc && com_argv[i+2][0]!='+');
            if (!strcmp(com_argv[i+1],"logfile_name")) {
                assert(!strcmp(com_argv[i+2],Q2RTX_LOG_STEM));logs++;
            }
            i+=2;
        } else if (!strcmp(com_argv[i],"+logfile_name")) {
            assert(i+1<com_argc && !strcmp(com_argv[i+1],Q2RTX_LOG_STEM));logs++;
        }
    }
    assert(logs==2);
}
static void Com_AddEarlyCommands(bool clear)
{
    int     i;
    char    *s;

    for (i = 1; i < com_argc; i++) {
        s = com_argv[i];
        if (!s) {
            continue;
        }
        if (strcmp(s, "+set")) {
            continue;
        }
        if (i + 2 >= com_argc) {
            Com_Printf("Usage: +set <variable> <value>\n");
            com_argc = i;
            break;
        }
        Cvar_SetEx(com_argv[i + 1], com_argv[i + 2], FROM_CMDLINE);
        if (clear) {
            com_argv[i] = com_argv[i + 1] = com_argv[i + 2] = NULL;
        }
        i += 2;
    }
}

int main(void){actual_startup();tm=0;loc=1;Com_AddEarlyCommands(false);assert(tm==1&&loc==0);tm=0;loc=1;/* persisted config seam */Com_AddEarlyCommands(true);assert(tm==1&&loc==0);for(int i=1;i<com_argc;i++)assert(!com_argv[i] || strcmp(com_argv[i],"+set"));puts("actual launcher arguments and version macros; early replay overrides saved tm_enable=0 and loc_draw=1: PASS");return 0;}
