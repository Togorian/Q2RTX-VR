// Modified 14 November 2026: selected Quake II RTX/Q2RTX VR production code
// adapted into a CPU fixture; copied paths and spans are listed in its manifest.
// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Retained CPU fixture: selected production spans are verified by rc7-regression.py.
/*
Copyright (C) 2018 Christoph Schied
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

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
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#define REFERENCE_B_B2 1
#define Q_strcasecmp _stricmp
#define ALIAS_LOOP_COUNT 16
#define Com_Printf(...) ((void)0)
#define Com_WPrintf(...) ((void)0)
static bool b2_graphics_active534;static int mutations,finds;static bool changed_asset=true;
typedef struct{int aliasCount;}cmdbuf_t;typedef struct{void(*function)(void);}cmd_function_t;typedef struct{char*value;}cmdalias_t;typedef int cvar_t;
static int cmd_argc=1;static char*cmd_argv[1];static cmdbuf_t*cmd_current;
static void IMG_ReloadAll(void){if(changed_asset)mutations++;}
static void vkpt_reload_textures(void)
{
	IMG_ReloadAll();
}
static cmd_function_t*Cmd_Find(const char*n){static cmd_function_t c={vkpt_reload_textures};finds++;return !_stricmp(n,"reload_textures")?&c:NULL;}
static bool CL_ForwardToServer(void){return false;}static cmdalias_t*Cmd_AliasFind(const char*n){return NULL;}
static char*Cmd_MacroExpandString(char*s,bool b){return s;}static void Cbuf_InsertText(cmdbuf_t*b,char*s){}
static cvar_t*Cvar_FindVar(const char*n){return NULL;}static void Cvar_Command(cvar_t*v){}
bool Cvar_B2CommandLocked580(const char *name)
{
    return b2_graphics_active534 && name &&
        (!Q_strcasecmp(name, "vid_restart") || !Q_strcasecmp(name, "r_reload") ||
         !Q_strcasecmp(name, "fs_restart") || !Q_strcasecmp(name, "sky") ||
         !Q_strcasecmp(name, "timerefresh") || !Q_strcasecmp(name, "reload_textures") ||
         !Q_strcasecmp(name, "mat") || !Q_strcasecmp(name, "gun_model") ||
         !Q_strcasecmp(name, "skins"));
}
void Cmd_ExecuteCommand(cmdbuf_t *buf)
{
    cmd_function_t  *cmd;
    cmdalias_t      *a;
    cvar_t          *v;
    char            *text;

    // execute the command line
    if (!cmd_argc) {
        return;         // no tokens
    }

#ifdef REFERENCE_B_B2
    extern bool Cvar_B2CommandLocked580(const char *name);
    if (Cvar_B2CommandLocked580(cmd_argv[0])) {
        Com_Printf("%s is unavailable during VR; restart the application to change it.\n", cmd_argv[0]);
        return;
    }
#endif
    cmd_current = buf;

    // check functions
    cmd = Cmd_Find(cmd_argv[0]);
    if (cmd) {
        if (cmd->function) {
            cmd->function();
        } else if (!CL_ForwardToServer()) {
            Com_Printf("Can't \"%s\", not connected\n", cmd_argv[0]);
        }
        return;
    }

    // check aliases
    a = Cmd_AliasFind(cmd_argv[0]);
    if (a) {
        if (buf->aliasCount >= ALIAS_LOOP_COUNT) {
            Com_WPrintf("Runaway alias loop\n");
            return;
        }
        text = Cmd_MacroExpandString(a->value, true);
        if (text) {
            buf->aliasCount++;
            Cbuf_InsertText(buf, text);
        }
        return;
    }

    // check variables
    v = Cvar_FindVar(cmd_argv[0]);
    if (v) {
        Cvar_Command(v);
        return;
    }

    // send it as a server command if we are connected
    if (!CL_ForwardToServer()) {
        Com_Printf("Unknown command \"%s\"\n", cmd_argv[0]);
    }
}
int main(void){cmdbuf_t buffers[3]={{0}};char*names[]={"vid_restart","r_reload","fs_restart","sky","timerefresh","reload_textures","mat","gun_model","skins","MAT","GUN_MODEL","SKINS"};
for(int active=0;active<2;active++)for(int route=0;route<3;route++)for(unsigned n=0;n<sizeof(names)/sizeof(names[0]);n++){b2_graphics_active534=active;cmd_argv[0]=names[n];int m=mutations,f=finds;Cmd_ExecuteCommand(&buffers[route]);assert(finds==f+!active);assert(mutations==m+(!active&&!_stricmp(names[n],"reload_textures")));}
puts("actual dispatch function: nine locked names and upper-case additions refused before handler lookup; three inert buffers, no parsing exercised PASS");}
