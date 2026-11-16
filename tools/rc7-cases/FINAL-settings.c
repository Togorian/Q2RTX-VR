// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 14 November 2026: selected Quake II RTX/Q2RTX VR cvar and crosshair
// functions in a CPU fixture; Windows/renderer/engine effects are inert seams.
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#define REFERENCE_B_B2 1
enum {CVAR_REFRESH=1,CVAR_FILES=2,CVAR_USERINFO=4,CVAR_MODIFIED=8,CVAR_ARCHIVE=16,CVAR_ROM=32,CVAR_CHEAT=64,CVAR_NOSET=128,CVAR_LATCH=256};
#define CVAR_INFOMASK CVAR_USERINFO
#define CVAR_MODIFYMASK (CVAR_REFRESH|CVAR_FILES)
#define CVAR_NOARCHIVEMASK (CVAR_ROM|CVAR_CHEAT|CVAR_NOSET)
typedef enum {FROM_CODE,FROM_CONSOLE,FROM_MENU,FROM_CMDLINE} from_t;
typedef struct cvar_s {const char *name;char *string,*latched_string;int flags,integer;float value;bool modified;void(*changed)(struct cvar_s*);} cvar_t;
static bool b2_graphics_active534,com_initialized;static int logs,callbacks,cvar_modified;
static cvar_t server,*sv_running=&server,*fs_game;
#define Com_Printf(...) (++logs)
#define Com_WPrintf(...) (++logs)
static cvar_t *Cvar_FindVar(const char*n){return NULL;}
static char *Z_CvarCopyString(const char*s){char*p=malloc(strlen(s)+1);assert(p);strcpy(p,s);return p;}
static void Z_Free(void*p){free(p);}
static void Z_Freep(void**p){free(*p);*p=NULL;}
static void parse_string_value(cvar_t*v){v->integer=atoi(v->string);v->value=(float)atof(v->string);}
static void set_back_cvar(cvar_t*v){}
static bool validate_info_cvar(const char*s){return true;}
static bool CL_CheatsOK(void){return true;}
static void CL_UpdateUserinfo(cvar_t*v,from_t f){}
static void callback(cvar_t*v){callbacks++;}
static cvar_t scale={.value=1},health,red={.value=1},green={.value=1},blue={.value=1},alpha={.value=1};
static cvar_t *scr_crosshair,*ch_scale=&scale,*ch_health=&health,*ch_red=&red,*ch_green=&green,*ch_blue=&blue,*ch_alpha=&alpha;
static struct {int crosshair_pic,crosshair_width,crosshair_height;struct{unsigned char u8[4];}crosshair_color;} scr;
static char cached[16];static int registrations,new_images;
static int R_RegisterPic(const char*n){registrations++;if(strcmp(cached,n)){strcpy(cached,n);new_images++;}return 1;}
static void R_GetPicSize(int*w,int*h,int p){*w=*h=8;}
#define Q_snprintf snprintf
static float Cvar_ClampValue(cvar_t*v,float lo,float hi){return v->value<lo?lo:v->value>hi?hi:v->value;}
static void SCR_SetCrosshairColor(void){}
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
static bool b2_graphics_refuse534(cvar_t *var, const char *value)
{
    if (!Cvar_B2GraphicsLocked534(var->name) || !strcmp(var->string, value)) return false;
    Com_Printf("%s is unavailable for changes during VR; current value retained.\n", var->name);
    return true;
}
static void change_string_value(cvar_t *var, const char *value, from_t from)
{
#ifdef REFERENCE_B_B2
    if (b2_graphics_refuse534(var, value)) return;
#endif
    // free the old value string
    Z_Free(var->string);

    var->string = Z_CvarCopyString(value);
    parse_string_value(var);

    if (var->flags & CVAR_USERINFO) {
        CL_UpdateUserinfo(var, from);
    }

    var->modified = true;
    if (from != FROM_CODE) {
        cvar_modified |= var->flags & CVAR_MODIFYMASK;
        var->flags |= CVAR_MODIFIED;
        if (from == FROM_MENU && !(var->flags & CVAR_NOARCHIVEMASK)) {
            var->flags |= CVAR_ARCHIVE;
        }
        if (var->changed) {
            var->changed(var);
        }
    }
}
void Cvar_SetByVar(cvar_t *var, const char *value, from_t from)
{
    if (!value) {
        value = "";
    }
#ifdef REFERENCE_B_B2
    if (b2_graphics_refuse534(var, value)) return;
#endif
    if (!strcmp(value, var->string)) {
        set_back_cvar(var);
        return; // not changed
    }

    if (var->flags & CVAR_INFOMASK) {
        if (!validate_info_cvar(value)) {
            return;
        }
    }

    // some cvars may not be changed by user at all
    if (from != FROM_CODE) {
        if (var->flags & CVAR_ROM) {
            Com_Printf("%s is read-only.\n", var->name);
            return;
        }

        if (var->flags & CVAR_CHEAT) {
            if (!CL_CheatsOK()) {
                Com_Printf("%s is cheat protected.\n", var->name);
                return;
            }
        }

        if ((var->flags & CVAR_NOSET) && com_initialized) {
            // FROM_CMDLINE while com_initialized == true means the second call
            // to Com_AddEarlyCommands() is changing the value, ignore.
            if (from != FROM_CMDLINE)
                Com_Printf("%s may be set from command line only.\n", var->name);
            return;
        }

        if (from == FROM_MENU && var == fs_game) {
            Com_WPrintf("Changing %s from menu is not allowed.\n", var->name);
            return;
        }

        if ((var->flags & CVAR_LATCH) && sv_running->integer) {
            if (var->latched_string && !strcmp(var->latched_string, value)) {
                return; // latched string not changed
            }
            Com_Printf("%s will be changed for next game.\n", var->name);
            Z_Free(var->latched_string);
            var->latched_string = Z_CvarCopyString(value);
            return;
        }
    }

    // free latched string, if any
    Z_Freep((void**)&var->latched_string);

    change_string_value(var, value, from);
}
static void scr_crosshair_changed(cvar_t *self)
{
    char buffer[16];
    int w, h;
    float scale;

    if (scr_crosshair->integer > 0) {
        Q_snprintf(buffer, sizeof(buffer), "ch%i", scr_crosshair->integer);
        scr.crosshair_pic = R_RegisterPic(buffer);
        R_GetPicSize(&w, &h, scr.crosshair_pic);

        // prescale
        scale = Cvar_ClampValue(ch_scale, 0.1f, 9.0f);
        scr.crosshair_width = w * scale;
        scr.crosshair_height = h * scale;
        if (scr.crosshair_width < 1)
            scr.crosshair_width = 1;
        if (scr.crosshair_height < 1)
            scr.crosshair_height = 1;

        if (ch_health->integer) {
            SCR_SetCrosshairColor();
        } else {
            scr.crosshair_color.u8[0] = Cvar_ClampValue(ch_red, 0, 1) * 255;
            scr.crosshair_color.u8[1] = Cvar_ClampValue(ch_green, 0, 1) * 255;
            scr.crosshair_color.u8[2] = Cvar_ClampValue(ch_blue, 0, 1) * 255;
        }
        scr.crosshair_color.u8[3] = Cvar_ClampValue(ch_alpha, 0, 1) * 255;
    } else {
        scr.crosshair_pic = 0;
    }
}

int main(void){
 const char*names[]={"crosshair","scr_font","con_font","con_background","skin","cl_noskins","cl_vwep","loc_draw"};
 for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++){
  cvar_t v={.name=names[i],.string=Z_CvarCopyString("0"),.changed=callback,.flags=CVAR_ARCHIVE};
  Cvar_B2GraphicsActive534(false);int cb=callbacks;Cvar_SetByVar(&v,"7",FROM_CONSOLE);assert(!strcmp(v.string,"7")&&callbacks==cb+1);
  Cvar_B2GraphicsActive534(true);v.modified=false;int flags=v.flags,lg=logs;cb=callbacks;char*value=v.string;
  Cvar_SetByVar(&v,"7",FROM_CONSOLE);assert(logs==lg&&callbacks==cb&&v.string==value&&!v.modified);
  for(int from=FROM_CODE;from<=FROM_CMDLINE;from++){Cvar_SetByVar(&v,"2",(from_t)from);assert(v.string==value&&v.flags==flags&&!v.modified&&callbacks==cb);}
  change_string_value(&v,"3",FROM_CONSOLE);assert(v.string==value&&callbacks==cb);
  Cvar_B2GraphicsActive534(false);Cvar_SetByVar(&v,"2",FROM_CONSOLE);assert(!strcmp(v.string,"2")&&callbacks==cb+1);Z_Free(v.string);
 }
 // Source-order proof separately checks config replay before lock arming. Here
 // saved none/nondefault values reach the actual initial media callback.
 for(int saved=0;saved<=7;saved+=7){
  cvar_t v={.name="crosshair",.string=Z_CvarCopyString("1"),.changed=scr_crosshair_changed};scr_crosshair=&v;char value[16];snprintf(value,sizeof(value),"%d",saved);cached[0]=0;registrations=new_images=0;
  Cvar_B2GraphicsActive534(false);Cvar_SetByVar(&v,value,FROM_CONSOLE);scr_crosshair_changed(&v);
  assert(new_images==(saved>0));int before=new_images;Cvar_B2GraphicsActive534(true);
  for(int choice=-1;choice<33;choice++){char n[16];snprintf(n,sizeof(n),"%d",choice);Cvar_SetByVar(&v,n,FROM_MENU);assert(v.integer==saved&&new_images==before);}
  // Other ch_* callbacks reuse the unchanged permanent picture, never a new name.
  scr_crosshair_changed(ch_scale);scr_crosshair_changed(ch_red);scr_crosshair_changed(ch_green);scr_crosshair_changed(ch_blue);scr_crosshair_changed(ch_alpha);scr_crosshair_changed(ch_health);assert(new_images==before);Z_Free(v.string);
 }
 // The post-config launcher override is exercised by R6-startup. A level change
 // does not disarm the renderer lock; repeated attempts cannot enable markers.
 cvar_t loc={.name="loc_draw",.string=Z_CvarCopyString("0"),.changed=callback};
 for(int map=0;map<3;map++){Cvar_SetByVar(&loc,"1",FROM_CONSOLE);assert(!strcmp(loc.string,"0"));}
 Z_Free(loc.string);puts("actual cvar mutation and crosshair callback: startup none/nondefault, same value, all write origins, cached ch_* callbacks and persistent loc_draw refusal PASS");return 0;
}
