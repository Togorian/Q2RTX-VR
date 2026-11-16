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
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#define BIT(x) (1u<<(x))
#define QMF_GRAYED 1
#define CVAR_REFRESH 2
#define CVAR_FILES 4
#define Q_strcasecmp _stricmp
static int logs;
#define Com_Printf(...) (++logs)
typedef struct {const char *name,*string;int flags;} cvar_t;
static cvar_t extra={"extra_restart","0",CVAR_REFRESH};
static cvar_t *Cvar_FindVar(const char *name){return !strcmp(name,"extra_restart")?&extra:NULL;}
static bool b2_graphics_active534;
void Cvar_B2GraphicsActive534(bool active) { b2_graphics_active534 = active; }
bool Cvar_B2GraphicsLocked534(const char *name)
{
    static const char *const locked[] = {
        "tm_enable", "drs_enable", "vid_hdr", "pt_accumulation_rendering", "pt_freecam",
        "flt_fsr_enable", "flt_enable", "pt_projection",
        "flt_temporal_hf", "flt_temporal_lf", "flt_temporal_spec",
        "vid_fullscreen", "vid_display", "vid_hwgamma", "vid_rtx", "vid_driver",
        "ray_tracing_api", "vk_validation", "pt_nearest", "pt_bilerp_chars", "pt_bilerp_pics",
        "pt_enable_surface_lights", "pt_enable_surface_lights_warp",
        "pt_surface_lights_fake_emissive_algo", "pt_surface_lights_threshold",
        "pt_bsp_radiance_scale", "r_override_textures", "r_texture_overrides",
        "bloom_debug", "b2_combat", "b2_advance_world", "b2_keyboard_locomotion",
        "b2_turning_mouselook", "b2_blaster_fire", "b2_jump_crouch", "b2_visual_target",
        "b2_same_map_ui", "b2_units_per_metre", "timedemo", "com_fatal_error",
        "b2_desktop_mirror", "b2_openxr", "b2_target_fault", "game",
        "loc_draw",
        "skin", "cl_noskins", "cl_vwep",
        "scr_font", "con_font", "con_background",
        "crosshair"
    };
    if (!b2_graphics_active534 || !name) return false;
    cvar_t *var = Cvar_FindVar(name);
    if (var && (var->flags & (CVAR_REFRESH | CVAR_FILES))) return true;
    for (size_t i = 0; i < sizeof(locked)/sizeof(locked[0]); ++i)
        if (!strcmp(name, locked[i])) return true;
    return false;
}
// R6-B2: reject user restart commands before any resource teardown, including
// aliases and commands read from bindings/configuration files.
bool Cvar_B2CommandLocked580(const char *name)
{
    return b2_graphics_active534 && name &&
        (!Q_strcasecmp(name, "vid_restart") || !Q_strcasecmp(name, "r_reload") ||
         !Q_strcasecmp(name, "fs_restart") || !Q_strcasecmp(name, "sky") ||
         !Q_strcasecmp(name, "timerefresh") || !Q_strcasecmp(name, "reload_textures") ||
         !Q_strcasecmp(name, "mat") || !Q_strcasecmp(name, "gun_model") ||
         !Q_strcasecmp(name, "skins"));
}
static bool b2_graphics_refuse534(cvar_t *var, const char *value)
{
    if (!Cvar_B2GraphicsLocked534(var->name) || !strcmp(var->string, value)) return false;
    Com_Printf("%s is unavailable for changes during VR; current value retained.\n", var->name);
    return true;
}


enum {MTYPE_SPINCONTROL,MTYPE_BITFIELD,MTYPE_PAIRS,MTYPE_VALUES,MTYPE_STRINGS,MTYPE_TOGGLE,MTYPE_SLIDER,MTYPE_FIELD};
typedef struct menuFrameWork_s menuFrameWork_t;
typedef struct {int type;unsigned flags;menuFrameWork_t *parent;const char *name;} menuCommon_t;
typedef struct {menuCommon_t common;cvar_t *cvar;} menuSpinControl_t;
typedef menuSpinControl_t menuSlider_t;typedef menuSpinControl_t menuField_t;
struct menuFrameWork_s {int nitems;menuCommon_t **items;const char *name;};
static struct {menuCommon_t *mouseTracker;} uis;
#define QMF_B2_LOCK534 BIT(7)
#define QMF_B2_WAS_GREY534 BIT(8)
extern bool Cvar_B2GraphicsLocked534(const char *name);
static bool B2VR_StockLocked534(const menuCommon_t *item)
{
    // Player Setup commits model/skin through its custom Pop, without row cvars.
    // Keep its handedness, aiming and view controls available.
    if (item->type == MTYPE_SPINCONTROL && item->parent && item->parent->name &&
        !strcmp(item->parent->name, "players") && item->name &&
        (!strcmp(item->name, "model") || !strcmp(item->name, "skin")))
        return Cvar_B2GraphicsLocked534("skin");
    cvar_t *var = NULL;
    switch (item->type) {
    case MTYPE_SPINCONTROL: case MTYPE_BITFIELD: case MTYPE_PAIRS:
    case MTYPE_VALUES: case MTYPE_STRINGS: case MTYPE_TOGGLE:
        var = ((const menuSpinControl_t *)item)->cvar; break;
    case MTYPE_SLIDER: var = ((const menuSlider_t *)item)->cvar; break;
    case MTYPE_FIELD: var = ((const menuField_t *)item)->cvar; break;
    default: break;
    }
    return var && Cvar_B2GraphicsLocked534(var->name);
}
static bool B2VR_StockDependencies534(menuFrameWork_t *menu)
{
    bool any = false;
    if (!menu) return false;
    for (int i = 0; i < menu->nitems; ++i) {
        menuCommon_t *item = menu->items[i];
        if (B2VR_StockLocked534(item)) {
            any = true;
            if (!(item->flags & QMF_B2_LOCK534)) {
                if (item->flags & QMF_GRAYED) item->flags |= QMF_B2_WAS_GREY534;
                item->flags |= QMF_B2_LOCK534 | QMF_GRAYED;
            }
            if (uis.mouseTracker == item) uis.mouseTracker = NULL;
        } else if (item->flags & QMF_B2_LOCK534) {
            if (!(item->flags & QMF_B2_WAS_GREY534)) item->flags &= ~QMF_GRAYED;
            item->flags &= ~(QMF_B2_LOCK534 | QMF_B2_WAS_GREY534);
        }
    }
    return any;
}

int main(void){ const char *names[]={"crosshair","scr_font","con_font","con_background","skin","cl_noskins","cl_vwep","loc_draw","drs_enable","vid_hdr","pt_accumulation_rendering","pt_freecam","flt_fsr_enable","flt_enable","pt_projection","flt_temporal_hf","flt_temporal_lf","flt_temporal_spec","vid_fullscreen","vid_display","vid_hwgamma","vid_rtx","vid_driver","ray_tracing_api","vk_validation","pt_nearest","pt_bilerp_chars","pt_bilerp_pics","pt_enable_surface_lights","pt_enable_surface_lights_warp","pt_surface_lights_fake_emissive_algo","pt_surface_lights_threshold","pt_bsp_radiance_scale","r_override_textures","r_texture_overrides","bloom_debug","b2_combat","b2_advance_world","b2_keyboard_locomotion","b2_turning_mouselook","b2_blaster_fire","b2_jump_crouch","b2_visual_target","b2_same_map_ui","b2_units_per_metre","timedemo","com_fatal_error","b2_desktop_mirror","b2_openxr","b2_target_fault","game"};
for(int active=0;active<2;active++){Cvar_B2GraphicsActive534(active);assert(Cvar_B2GraphicsLocked534("extra_restart")==active);assert(Cvar_B2CommandLocked580("VID_RESTART")==active);assert(Cvar_B2CommandLocked580("TimeRefresh")==active);assert(Cvar_B2GraphicsLocked534("game")==active);
for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++){assert(Cvar_B2GraphicsLocked534(names[i])==active);cvar_t v={names[i],"0"};assert(b2_graphics_refuse534(&v,"1")==active);assert(!b2_graphics_refuse534(&v,"0"));
for(int kind=0;kind<=MTYPE_FIELD;kind++){menuSpinControl_t item={{kind,0},&v};menuCommon_t *items[]={&item.common};menuFrameWork_t menu={1,items};uis.mouseTracker=items[0];assert(B2VR_StockDependencies534(&menu)==active);assert(!!(item.common.flags&QMF_GRAYED)==active);if(active){assert(!uis.mouseTracker);Cvar_B2GraphicsActive534(false);B2VR_StockDependencies534(&menu);assert(!item.common.flags);Cvar_B2GraphicsActive534(true);}}}
assert(Cvar_B2CommandLocked580("vid_restart")==active);assert(Cvar_B2CommandLocked580("sky")==active);assert(Cvar_B2CommandLocked580("fs_restart")==active);assert(Cvar_B2CommandLocked580("r_reload")==active);assert(!Cvar_B2CommandLocked580("map"));}
const char *safe[]={"vid_vsync","flt_taa","profiler","viewsize","b2_render_percent","sli"};for(unsigned i=0;i<sizeof(safe)/sizeof(safe[0]);i++)assert(!Cvar_B2GraphicsLocked534(safe[i]));puts("actual cvar refusal and menu greying functions: pass");return 0;}