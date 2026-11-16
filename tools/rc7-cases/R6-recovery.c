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
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REFERENCE_B_B2 1
#define LENGTH(a) (sizeof(a)/sizeof((a)[0]))
#define ERR_FATAL 1
#define Com_Error(...) abort()
static int logs,descriptor_changes;
#define Com_Printf(...) (++logs)
typedef struct {const char *name;float value;int integer;} cvar_t;
static cvar_t vars[]={{"pt_nearest",1,1},{"pt_bilerp_chars",1,1},{"pt_bilerp_pics",1,1},{"flt_enable",1,1},{"flt_temporal_hf",.5f,0},{"flt_temporal_lf",1,1},{"flt_temporal_spec",1,1}};
static cvar_t zero={"safe",0,0},one={"tm_enable",1,1};
static cvar_t *cvar_hdr=&zero,*cvar_pt_projection=&zero,*cvar_drs_enable=&zero,*cvar_pt_freecam=&zero,*cvar_tm_enable=&one,*cvar_pt_accumulation_rendering=&zero;
static cvar_t *Cvar_FindVar(const char *name){for(int i=0;i<7;i++)if(!strcmp(name,vars[i].name))return &vars[i];return NULL;}
static bool B2XR_Requested(void){return true;}
static bool vkpt_fsr_is_enabled(void){return false;}
static int Cvar_VariableInteger(const char *n){return 0;}
static void vkpt_invalidate_texture_descriptors(void){descriptor_changes++;}
static struct {bool b2_temporal_activated;} qvk;
static bool temporal_frame_valid=true,b2_settings_changed580;
static struct {
    const char *name;
    float value;
    int integer;
} b2_callback_settings580[] = {
    {"pt_nearest"}, {"pt_bilerp_chars"}, {"pt_bilerp_pics"},
    {"flt_enable"}, {"flt_temporal_hf"}, {"flt_temporal_lf"}, {"flt_temporal_spec"}
};
static void b2_accept_callback_settings580(void)
{
    for (size_t i=0; i<LENGTH(b2_callback_settings580); i++) {
        cvar_t *var=Cvar_FindVar(b2_callback_settings580[i].name);
        if (!var) Com_Error(ERR_FATAL, "VR callback setting was not registered");
        b2_callback_settings580[i].value=var->value;
        b2_callback_settings580[i].integer=var->integer;
    }
}
static bool b2_callback_settings_compatible580(void)
{
    for (size_t i=0; i<LENGTH(b2_callback_settings580); i++) {
        cvar_t *var=Cvar_FindVar(b2_callback_settings580[i].name);
        if (!var || var->value!=b2_callback_settings580[i].value ||
            var->integer!=b2_callback_settings580[i].integer) return false;
    }
    return true;
}
static void b2_settings_notice580(void)
{
    static bool warned;
    if (!warned) Com_Printf("VR frame skipped: unsupported graphics settings. Restart with the VR launcher.\n");
    warned = true;
}
static void pt_nearest_changed(cvar_t* self)
{
#ifdef REFERENCE_B_B2
    if (B2XR_Requested() && qvk.b2_temporal_activated) {
        b2_settings_changed580 = true;
        b2_settings_notice580();
        return;
    }
#endif
	vkpt_invalidate_texture_descriptors();
}
static void temporal_cvar_changed(cvar_t *self)
{
#ifdef REFERENCE_B_B2
	if (qvk.b2_temporal_activated) {
        b2_settings_changed580 = true;
        b2_settings_notice580();
    }
#endif
	temporal_frame_valid = false;
}
static bool b2_frame_settings_valid580(void)
{
    return (!b2_settings_changed580 || b2_callback_settings_compatible580()) && !cvar_hdr->integer &&
        !cvar_pt_projection->integer && !cvar_drs_enable->integer &&
        !cvar_pt_freecam->integer && cvar_tm_enable->integer &&
        !cvar_pt_accumulation_rendering->integer && !vkpt_fsr_is_enabled() &&
        !Cvar_VariableInteger("bloom_debug");
}
int main(void){b2_accept_callback_settings580();qvk.b2_temporal_activated=true;
for(int i=0;i<7;i++){float old=vars[i].value;int oi=vars[i].integer;vars[i].value=old+.25f;vars[i].integer=oi+1;
if(i<3)pt_nearest_changed(&vars[i]);else temporal_cvar_changed(&vars[i]);assert(!b2_frame_settings_valid580());
vars[i].integer=oi;assert(!b2_frame_settings_valid580());vars[i].value=old;
if(i<3)pt_nearest_changed(&vars[i]);else temporal_cvar_changed(&vars[i]);assert(b2_frame_settings_valid580());}
assert(descriptor_changes==0 && logs==1 && !temporal_frame_valid);
puts("actual callbacks -> reject -> restore exact accepted numeric values -> valid admission; no resource change; one log: PASS");return 0;}
