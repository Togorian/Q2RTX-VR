/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/*
Copyright (C) 1997-2001 Id Software, Inc.
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
/* Modified by Togorian with AI assistance, 12 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */


//
// cl_precache.c
//

#include "client.h"

/*
================
CL_ParsePlayerSkin

Breaks up playerskin into name (optional), model and skin components.
If model or skin are found to be invalid, replaces them with sane defaults.
================
*/
void CL_ParsePlayerSkin(char *name, char *model, char *skin, const char *s)
{
    size_t len;
    char *t;

    len = strlen(s);
    Q_assert(len < MAX_QPATH);

    // isolate the player's name
    t = strchr(s, '\\');
    if (t) {
        len = t - s;
        strcpy(model, t + 1);
    } else {
        len = 0;
        strcpy(model, s);
    }

    // copy the player's name
    if (name) {
        memcpy(name, s, len);
        name[len] = 0;
    }

    // isolate the model name
    t = strchr(model, '/');
    if (!t)
        t = strchr(model, '\\');
    if (!t)
        goto default_model;
    *t = 0;

    // isolate the skin name
    strcpy(skin, t + 1);

    // fix empty model to male
    if (t == model)
        strcpy(model, "male");

    // apply restrictions on skins
    if (cl_noskins->integer == 2 || !COM_IsPath(skin))
        goto default_skin;

    if (cl_noskins->integer || !COM_IsPath(model))
        goto default_model;

    return;

default_skin:
    if (!Q_stricmp(model, "female")) {
        strcpy(model, "female");
        strcpy(skin, "athena");
    } else {
default_model:
        strcpy(model, "male");
        strcpy(skin, "grunt");
    }
}

/*
================
CL_LoadClientinfo

================
*/
void CL_LoadClientinfo(clientinfo_t *ci, const char *s)
{
    int         i;
    char        model_name[MAX_QPATH];
    char        skin_name[MAX_QPATH];
    char        model_filename[MAX_QPATH];
    char        skin_filename[MAX_QPATH];
    char        weapon_filename[MAX_QPATH];
    char        icon_filename[MAX_QPATH];

    CL_ParsePlayerSkin(ci->name, model_name, skin_name, s);

    // model file
    Q_concat(model_filename, sizeof(model_filename),
             "players/", model_name, "/tris.md2");
    ci->model = R_RegisterModel(model_filename);
    if (!ci->model && Q_stricmp(model_name, "male")) {
        strcpy(model_name, "male");
        strcpy(model_filename, "players/male/tris.md2");
        ci->model = R_RegisterModel(model_filename);
    }

    // skin file
    Q_concat(skin_filename, sizeof(skin_filename),
             "players/", model_name, "/", skin_name, ".pcx");
    ci->skin = R_RegisterSkin(skin_filename);

    // if we don't have the skin and the model was female,
    // see if athena skin exists
    if (!ci->skin && !Q_stricmp(model_name, "female")) {
        strcpy(skin_name, "athena");
        strcpy(skin_filename, "players/female/athena.pcx");
        ci->skin = R_RegisterSkin(skin_filename);
    }

    // if we don't have the skin and the model wasn't male,
    // see if the male has it (this is for CTF's skins)
    if (!ci->skin && Q_stricmp(model_name, "male")) {
        // change model to male
        strcpy(model_name, "male");
        strcpy(model_filename, "players/male/tris.md2");
        ci->model = R_RegisterModel(model_filename);

        // see if the skin exists for the male model
        Q_concat(skin_filename, sizeof(skin_filename),
                 "players/male/", skin_name, ".pcx");
        ci->skin = R_RegisterSkin(skin_filename);
    }

    // if we still don't have a skin, it means that the male model
    // didn't have it, so default to grunt
    if (!ci->skin) {
        // see if the skin exists for the male model
        strcpy(skin_name, "grunt");
        strcpy(skin_filename, "players/male/grunt.pcx");
        ci->skin = R_RegisterSkin(skin_filename);
    }

    // weapon file
    for (i = 0; i < cl.numWeaponModels; i++) {
        Q_concat(weapon_filename, sizeof(weapon_filename),
                 "players/", model_name, "/", cl.weaponModels[i]);
        ci->weaponmodel[i] = R_RegisterModel(weapon_filename);
        if (!ci->weaponmodel[i] && !Q_stricmp(model_name, "cyborg")) {
            // try male
            Q_concat(weapon_filename, sizeof(weapon_filename),
                     "players/male/", cl.weaponModels[i]);
            ci->weaponmodel[i] = R_RegisterModel(weapon_filename);
        }
    }

    // icon file
    Q_concat(icon_filename, sizeof(icon_filename),
             "/players/", model_name, "/", skin_name, "_i.pcx");
    ci->icon = R_RegisterPic2(icon_filename);

    strcpy(ci->model_name, model_name);
    strcpy(ci->skin_name, skin_name);

    // base info should be at least partially valid
    if (ci == &cl.baseclientinfo)
        return;

    // must have loaded all data types to be valid
    if (!ci->skin || !ci->icon || !ci->model || !ci->weaponmodel[0]) {
        ci->skin = 0;
        ci->icon = 0;
        ci->model = 0;
        ci->weaponmodel[0] = 0;
        ci->model_name[0] = 0;
        ci->skin_name[0] = 0;
    }
}

/*
=================
CL_RegisterSounds
=================
*/
void CL_RegisterSounds(void)
{
    int i;
    char    *s;

    S_BeginRegistration();
    CL_RegisterTEntSounds();
    for (i = 1; i < cl.csr.max_sounds; i++) {
        s = cl.configstrings[cl.csr.sounds + i];
        if (!s[0])
            break;
        cl.sound_precache[i] = S_RegisterSound(s);
    }
    S_EndRegistration();
}

/*
=================
CL_RegisterBspModels

Registers main BSP file and inline models
=================
*/
void CL_RegisterBspModels(void)
{
    char *name = cl.configstrings[cl.csr.models + 1];
    int i, ret;

    if (!name[0]) {
        Com_Error(ERR_DROP, "%s: no map set", __func__);
    }
    ret = BSP_Load(name, &cl.bsp);
    if (cl.bsp == NULL) {
        Com_Error(ERR_DROP, "Couldn't load %s: %s", name, BSP_ErrorString(ret));
    }

    if (cl.bsp->checksum != Q_atoi(cl.configstrings[cl.csr.mapchecksum])) {
        if (cls.demo.playback) {
            Com_WPrintf("Local map version differs from demo: %i != %s\n",
                        cl.bsp->checksum, cl.configstrings[cl.csr.mapchecksum]);
        } else {
            Com_Error(ERR_DROP, "Local map version differs from server: %i != %s",
                      cl.bsp->checksum, cl.configstrings[cl.csr.mapchecksum]);
        }
    }

    for (i = 2; i < cl.csr.max_models; i++) {
        name = cl.configstrings[cl.csr.models + i];
        if (!name[0] && i != MODELINDEX_PLAYER) {
            break;
        }
        if (name[0] == '*')
            cl.model_clip[i] = BSP_InlineModel(cl.bsp, name);
        else
            cl.model_clip[i] = NULL;
    }
}

/*
=================
CL_RegisterVWepModels

Builds a list of visual weapon models
=================
*/
void CL_RegisterVWepModels(void)
{
    int         i;
    char        *name;

    cl.numWeaponModels = 1;
    strcpy(cl.weaponModels[0], "weapon.md2");

    // only default model when vwep is off
    if (!cl_vwep->integer) {
        return;
    }

    for (i = 2; i < cl.csr.max_models; i++) {
        name = cl.configstrings[cl.csr.models + i];
        if (!name[0] && i != MODELINDEX_PLAYER) {
            break;
        }
        if (name[0] != '#') {
            continue;
        }

        // special player weapon model
        Q_strlcpy(cl.weaponModels[cl.numWeaponModels++], name + 1, sizeof(cl.weaponModels[0]));

        if (cl.numWeaponModels == MAX_CLIENTWEAPONMODELS) {
            break;
        }
    }
}

/*
=================
CL_SetSky

=================
*/
void CL_SetSky(void)
{
    float       rotate = 0;
    int         autorotate = 1;
    vec3_t      axis;

    sscanf(cl.configstrings[CS_SKYROTATE], "%f %d", &rotate, &autorotate);
    if (sscanf(cl.configstrings[CS_SKYAXIS], "%f %f %f",
               &axis[0], &axis[1], &axis[2]) != 3) {
        Com_DPrintf("Couldn't parse CS_SKYAXIS\n");
        VectorClear(axis);
    }

    R_SetSky(cl.configstrings[CS_SKY], rotate, autorotate, axis);
}

/*
=================
CL_RegisterImage

Hack to handle RF_CUSTOMSKIN for remaster
=================
*/
static qhandle_t CL_RegisterImage(const char *s)
{
    // if it's in a subdir and has an extension, it's either a sprite or a skin
    // allow /some/pic.pcx escape syntax
    if (cl.csr.extended && *s != '/' && *s != '\\' && *COM_FileExtension(s)) {
        if (!FS_pathcmpn(s, CONST_STR_LEN("sprites/")))
            return R_RegisterSprite(s);
        if (strchr(s, '/'))
            return R_RegisterSkin(s);
    }

    return R_RegisterPic2(s);
}

/*
=================
CL_PrepRefresh

Call before entering a new level, or after changing dlls
=================
*/
void CL_PrepRefresh(void)
{
    int         i;
    char        *name;

    if (!cls.ref_initialized)
        return;
    if (!cl.mapname[0])
        return;     // no map loaded

    // register models, pics, and skins
    R_BeginRegistration(cl.mapname);

    CL_LoadState(LOAD_MODELS);

    CL_RegisterTEntModels();

	if (cl_testmodel->string && cl_testmodel->string[0])
	{
		cl_testmodel_handle = R_RegisterModel(cl_testmodel->string);
		if (cl_testmodel_handle)
			Com_Printf("Loaded the test model: %s\n", cl_testmodel->string);
		else
			Com_WPrintf("Failed to load the test model from %s\n", cl_testmodel->string);
	}
	else
		cl_testmodel_handle = -1;

    for (i = 2; i < cl.csr.max_models; i++) {
        name = cl.configstrings[cl.csr.models + i];
        if (!name[0] && i != MODELINDEX_PLAYER) {
            break;
        }
        if (name[0] == '#') {
            continue;
        }
        cl.model_draw[i] = R_RegisterModel(name);
    }

    CL_LoadState(LOAD_IMAGES);
    for (i = 1; i < cl.csr.max_images; i++) {
        name = cl.configstrings[cl.csr.images + i];
        if (!name[0]) {
            break;
        }
        cl.image_precache[i] = CL_RegisterImage(name);
    }

    CL_LoadState(LOAD_CLIENTS);
    for (i = 0; i < MAX_CLIENTS; i++) {
        name = cl.configstrings[cl.csr.playerskins + i];
        if (!name[0]) {
            continue;
        }
        CL_LoadClientinfo(&cl.clientinfo[i], name);
    }

    CL_LoadClientinfo(&cl.baseclientinfo, "unnamed\\male/grunt");

    // set sky textures and speed
    CL_SetSky();

#ifdef REFERENCE_B_B2
    /* Stock manual give-all spawns the Power Shield and publishes its model
     * and pickup icon even on maps without that item. Register just that media
     * while this world owns registration, so its later configstring update is
     * a cache reference rather than forbidden active-session texture work. */
    if (Cvar_VariableInteger("b2_openxr") == 1 &&
        Cvar_VariableInteger("b2_combat") == 1 &&
        cls.serverAddress.type == NA_LOOPBACK && sv_running->integer &&
        cl.maxclients == 1 && !cls.demo.playback) {
        if (!R_RegisterModel("models/items/armor/shield/tris.md2") ||
            !CL_RegisterImage("i_powershield"))
            Com_Error(ERR_DROP, "B2: could not register manual give-all shield media");

        /* A stock armour-shard drop may publish its model after the world
         * starts. Resolve its MD2 skins/materials and pickup icon here, so
         * later model/image configstrings only reference registered media. */
        if (!R_RegisterModel("models/items/armor/shard/tris.md2") ||
            !CL_RegisterImage("i_jacketarmor"))
            Com_Error(ERR_DROP, "B2: could not register armour shard media");

        /* Manual give-all also grants Body Armor, weapons, ammo and usable
         * powerups without calling PrecacheItem. G_SetStats can publish these
         * icons later for armour, ammo, the selected item or a powerup timer.
         * Keep this stock-item set inside the world's registration owner. */
        static const char *const give_all_hud_icons[] = {
            "i_bodyarmor",
            "w_shotgun", "w_sshotgun", "w_machinegun", "w_chaingun",
            "a_grenades", "w_glauncher", "w_rlauncher", "w_hyperblaster",
            "w_railgun", "w_bfg",
            "a_shells", "a_bullets", "a_cells", "a_rockets", "a_slugs",
            "p_quad", "p_invulnerability", "p_silencer",
            "p_rebreather", "p_envirosuit"
        };
        for (size_t icon = 0; icon < q_countof(give_all_hud_icons); icon++) {
            if (!CL_RegisterImage(give_all_hud_icons[icon]))
                Com_Error(ERR_DROP, "B2: could not register manual give-all HUD icon %s",
                          give_all_hud_icons[icon]);
        }
    }
#endif

    // the renderer can now free unneeded stuff
    R_EndRegistration();

    // clear any lines of console text
    Con_ClearNotify_f();

    SCR_UpdateScreen();

    // start the cd track
    OGG_Play();
}

/*
=================
CL_UpdateConfigstring

A configstring update has been parsed.
=================
*/
void CL_UpdateConfigstring(int index)
{
    const char *s = cl.configstrings[index];

    if (index == cl.csr.maxclients) {
        cl.maxclients = Q_atoi(s);
        return;
    }

    if (index == cl.csr.airaccel) {
        cl.pmp.airaccelerate = cl.pmp.qwmode || Q_atoi(s);
        return;
    }

    if (index == cl.csr.models + 1) {
        if (!Com_ParseMapName(cl.mapname, s, sizeof(cl.mapname)))
            Com_Error(ERR_DROP, "%s: bad world model: %s", __func__, s);
        return;
    }

    if (index >= cl.csr.lights && index < cl.csr.lights + MAX_LIGHTSTYLES) {
        CL_SetLightStyle(index - cl.csr.lights, s);
        return;
    }

    if (cls.state < ca_precached) {
        return;
    }

    if (index >= cl.csr.models + 2 && index < cl.csr.models + cl.csr.max_models) {
        int i = index - cl.csr.models;

        cl.model_draw[i] = R_RegisterModel(s);
        if (*s == '*')
            cl.model_clip[i] = BSP_InlineModel(cl.bsp, s);
        else
            cl.model_clip[i] = NULL;
        return;
    }

    if (index >= cl.csr.sounds && index < cl.csr.sounds + cl.csr.max_sounds) {
        cl.sound_precache[index - cl.csr.sounds] = S_RegisterSound(s);
        return;
    }

    if (index >= cl.csr.images && index < cl.csr.images + cl.csr.max_images) {
        cl.image_precache[index - cl.csr.images] = CL_RegisterImage(s);
        return;
    }

    if (index >= cl.csr.playerskins && index < cl.csr.playerskins + MAX_CLIENTS) {
        CL_LoadClientinfo(&cl.clientinfo[index - cl.csr.playerskins], s);
        return;
    }

    if (index == CS_CDTRACK) {
        OGG_Play();
        return;
    }
}
