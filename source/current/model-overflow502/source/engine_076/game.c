/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
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
/* Modified by Togorian with AI assistance, 12 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */

// sv_game.c -- interface to the game dll

#include "server.h"
#include "shared/debug.h"
#if USE_CLIENT && defined(REFERENCE_B_B2)
#include "../../../../baseline/b2-consolidated215/inputs/origin-002/include/refresh/b2_weapon_ui.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-002/include/refresh/b2_roomscale_bridge.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-002/include/refresh/b2_teleport_bridge.h"
#endif

const game_export_t     *ge;
const game_export_ex_t  *gex;

static void PF_configstring(int index, const char *val);

/*
================
PF_FindIndex

================
*/
// Error-only evidence; never changes the model table or overflow policy.
static void PF_ReportModelOverflow(const char *name, int start, int max, int skip)
{
    const int capacity = sizeof(sv.configstrings) / sizeof(sv.configstrings[0]);
    int occupied = 0, free_slots = 0;

    if (start < 0 || start >= capacity || max <= 1 || max > capacity - start ||
        start >= svs.csr.end || max > svs.csr.end - start) {
        Com_Printf("B2_MODEL_OVERFLOW table_bounds_invalid base=%d max=%d reserved=%d\n", start, max, skip);
        return;
    }
    for (int i = 1; i < max; i++) {
        if (i == skip)
            continue;
        if (sv.configstrings[start + i][0]) occupied++;
        else free_slots++;
    }
    Com_Printf("B2_MODEL_OVERFLOW map=%.*s requested=%.*s base=%d max=%d reserved=%d occupied=%d free=%d counts_exclude_zero_and_reserved=1\n",
        (int)sizeof(sv.name) - 1, sv.name, MAX_QPATH - 1, name,
        start, max, skip, occupied, free_slots);
    for (int i = 1; i < max; i++) {
        const char *entry = sv.configstrings[start + i];
        if (entry[0])
            Com_Printf("B2_MODEL_OVERFLOW index=%d configstring=%d reserved=%d name=%.*s\n",
                i, start + i, i == skip, MAX_QPATH - 1, entry);
    }
}

static int PF_FindIndex(const char *name, int start, int max, int skip, const char *func)
{
    char *string;
    int i;

    if (!name || !name[0])
        return 0;

    for (i = 1; i < max; i++) {
        if (i == skip) {
            continue;
        }
        string = sv.configstrings[start + i];
        if (!string[0]) {
            break;
        }
        if (!strcmp(string, name)) {
            return i;
        }
    }

    if (i == max) {
        if (g_features->integer & GMF_ALLOW_INDEX_OVERFLOW) {
            Com_DPrintf("%s(%s): overflow\n", func, name);
            return 0;
        }
        if (start == svs.csr.models && max == svs.csr.max_models && skip == MODELINDEX_PLAYER)
            PF_ReportModelOverflow(name, start, max, skip);
        Com_Error(ERR_DROP, "%s(%s): overflow", func, name);
    }

    PF_configstring(i + start, name);

    return i;
}

static int PF_ModelIndex(const char *name)
{
    return PF_FindIndex(name, svs.csr.models, svs.csr.max_models, MODELINDEX_PLAYER, __func__);
}

static int PF_SoundIndex(const char *name)
{
    return PF_FindIndex(name, svs.csr.sounds, svs.csr.max_sounds, 0, __func__);
}

static int PF_ImageIndex(const char *name)
{
    return PF_FindIndex(name, svs.csr.images, svs.csr.max_images, 0, __func__);
}

/*
===============
PF_Unicast

Sends the contents of the mutlicast buffer to a single client.
Archived in MVD stream.
===============
*/
static void PF_Unicast(edict_t *ent, qboolean reliable)
{
    client_t    *client;
    int         cmd, flags, clientNum;

    if (!ent) {
        goto clear;
    }

    clientNum = NUM_FOR_EDICT(ent) - 1;
    if (clientNum < 0 || clientNum >= sv_maxclients->integer) {
        Com_WPrintf("%s to a non-client %d\n", __func__, clientNum);
        goto clear;
    }

    client = svs.client_pool + clientNum;
    if (client->state <= cs_zombie) {
        Com_WPrintf("%s to a free/zombie client %d\n", __func__, clientNum);
        goto clear;
    }

    if (!msg_write.cursize) {
        Com_DPrintf("%s with empty data\n", __func__);
        goto clear;
    }

    cmd = msg_write.data[0];

    flags = 0;
    if (reliable) {
        flags |= MSG_RELIABLE;
    }

    if (cmd == svc_layout || (cmd == svc_configstring && RL16(&msg_write.data[1]) == CS_STATUSBAR)) {
        flags |= MSG_COMPRESS_AUTO;
    }

    SV_ClientAddMessage(client, flags);

    // fix anti-kicking exploit for broken mods
    if (cmd == svc_disconnect) {
        client->drop_hack = true;
        goto clear;
    }

    SV_MvdUnicast(ent, clientNum, reliable);

clear:
    SZ_Clear(&msg_write);
}

/*
=================
PF_bprintf

Sends text to all active clients.
Archived in MVD stream.
=================
*/
static void PF_bprintf(int level, const char *fmt, ...)
{
    va_list     argptr;
    char        string[MAX_STRING_CHARS];
    client_t    *client;
    size_t      len;
    int         i;

    va_start(argptr, fmt);
    len = Q_vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(string)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    SV_MvdBroadcastPrint(level, string);

    MSG_WriteByte(svc_print);
    MSG_WriteByte(level);
    MSG_WriteData(string, len + 1);

    // echo to console
    if (COM_DEDICATED) {
        // mask off high bits
        for (i = 0; i < len; i++)
            string[i] &= 127;
        Com_Printf("%s", string);
    }

    FOR_EACH_CLIENT(client) {
        if (client->state != cs_spawned)
            continue;
        if (level >= client->messagelevel) {
            SV_ClientAddMessage(client, MSG_RELIABLE);
        }
    }

    SZ_Clear(&msg_write);
}

/*
===============
PF_dprintf

Debug print to server console.
===============
*/
static void PF_dprintf(const char *fmt, ...)
{
    char        msg[MAXPRINTMSG];
    va_list     argptr;

#if USE_SAVEGAMES
    // detect YQ2 game lib by unique first two messages
    if (!sv.gamedetecthack)
        sv.gamedetecthack = 1 + !strcmp(fmt, "Game is starting up.\n");
    else if (sv.gamedetecthack == 2)
        sv.gamedetecthack = 3 + !strcmp(fmt, "Game is %s built on %s.\n");
#endif

    va_start(argptr, fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    Com_Printf("%s", msg);
}

/*
===============
PF_cprintf

Print to a single client if the level passes.
Archived in MVD stream.
===============
*/
static void PF_cprintf(edict_t *ent, int level, const char *fmt, ...)
{
    char        msg[MAX_STRING_CHARS];
    va_list     argptr;
    int         clientNum;
    size_t      len;
    client_t    *client;

    va_start(argptr, fmt);
    len = Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(msg)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    if (!ent) {
        Com_LPrintf(level == PRINT_CHAT ? PRINT_TALK : PRINT_ALL, "%s", msg);
        return;
    }

    clientNum = NUM_FOR_EDICT(ent) - 1;
    if (clientNum < 0 || clientNum >= sv_maxclients->integer) {
        Com_Error(ERR_DROP, "%s to a non-client %d", __func__, clientNum);
    }

    client = svs.client_pool + clientNum;
    if (client->state <= cs_zombie) {
        Com_WPrintf("%s to a free/zombie client %d\n", __func__, clientNum);
        return;
    }

    MSG_WriteByte(svc_print);
    MSG_WriteByte(level);
    MSG_WriteData(msg, len + 1);

    if (level >= client->messagelevel) {
        SV_ClientAddMessage(client, MSG_RELIABLE);
    }

    SV_MvdUnicast(ent, clientNum, true);

    SZ_Clear(&msg_write);
}

/*
===============
PF_centerprintf

Centerprint to a single client.
Archived in MVD stream.
===============
*/
static void PF_centerprintf(edict_t *ent, const char *fmt, ...)
{
    char        msg[MAX_STRING_CHARS];
    va_list     argptr;
    int         n;
    size_t      len;

    if (!ent) {
        return;
    }

    n = NUM_FOR_EDICT(ent);
    if (n < 1 || n > sv_maxclients->integer) {
        Com_WPrintf("%s to a non-client %d\n", __func__, n - 1);
        return;
    }

    va_start(argptr, fmt);
    len = Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(msg)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    MSG_WriteByte(svc_centerprint);
    MSG_WriteData(msg, len + 1);

    PF_Unicast(ent, true);
}

/*
===============
PF_error

Abort the server with a game error
===============
*/
static q_noreturn void PF_error(const char *fmt, ...)
{
    char        msg[MAXERRORMSG];
    va_list     argptr;

    va_start(argptr, fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    Com_Error(ERR_DROP, "Game Error: %s", msg);
}

/*
=================
PF_setmodel

Also sets mins and maxs for inline bmodels
=================
*/
static void PF_setmodel(edict_t *ent, const char *name)
{
    mmodel_t    *mod;

    if (!ent || !name)
        Com_Error(ERR_DROP, "PF_setmodel: NULL");

    ent->s.modelindex = PF_ModelIndex(name);

// if it is an inline model, get the size information for it
    if (name[0] == '*') {
        mod = CM_InlineModel(&sv.cm, name);
        VectorCopy(mod->mins, ent->mins);
        VectorCopy(mod->maxs, ent->maxs);
        PF_LinkEdict(ent);
    }
}

/*
===============
PF_configstring

If game is actively running, broadcasts configstring change.
Archived in MVD stream.
===============
*/
static void PF_configstring(int index, const char *val)
{
    size_t len, maxlen;
    client_t *client;
    char *dst;

    if (index < 0 || index >= svs.csr.end)
        Com_Error(ERR_DROP, "%s: bad index: %d", __func__, index);

    if (sv.state == ss_dead) {
        Com_WPrintf("%s: not yet initialized\n", __func__);
        return;
    }

    if (!val)
        val = "";

    // error out entirely if it exceedes array bounds
    len = strlen(val);
    maxlen = (svs.csr.end - index) * MAX_QPATH;
    if (len >= maxlen) {
        Com_Error(ERR_DROP,
                  "%s: index %d overflowed: %zu > %zu",
                  __func__, index, len, maxlen - 1);
    }

    // print a warning and truncate everything else
    maxlen = CS_SIZE(&svs.csr, index);
    if (len >= maxlen) {
        Com_WPrintf(
            "%s: index %d overflowed: %zu > %zu\n",
            __func__, index, len, maxlen - 1);
        len = maxlen - 1;
    }

    dst = sv.configstrings[index];
    if (!strncmp(dst, val, maxlen)) {
        return;
    }

    // change the string in sv
    memcpy(dst, val, len);
    dst[len] = 0;

    if (sv.state == ss_loading) {
        return;
    }

    SV_MvdConfigstring(index, val, len);

    // send the update to everyone
    MSG_WriteByte(svc_configstring);
    MSG_WriteShort(index);
    MSG_WriteData(val, len);
    MSG_WriteByte(0);

    FOR_EACH_CLIENT(client) {
        if (client->state < cs_primed) {
            continue;
        }
        SV_ClientAddMessage(client, MSG_RELIABLE);
    }

    SZ_Clear(&msg_write);
}

static void PF_WriteFloat(float f)
{
    Com_Error(ERR_DROP, "PF_WriteFloat not implemented");
}

static qboolean PF_inVIS(const vec3_t p1, const vec3_t p2, int vis)
{
    mleaf_t *leaf1, *leaf2;
    byte mask[VIS_MAX_BYTES];
    bsp_t *bsp = sv.cm.cache;

    if (!bsp) {
        Com_Error(ERR_DROP, "%s: no map loaded", __func__);
    }

    leaf1 = BSP_PointLeaf(bsp->nodes, p1);
    BSP_ClusterVis(bsp, mask, leaf1->cluster, vis);

    leaf2 = BSP_PointLeaf(bsp->nodes, p2);
    if (leaf2->cluster == -1)
        return false;
    if (!Q_IsBitSet(mask, leaf2->cluster))
        return false;
    if (!CM_AreasConnected(&sv.cm, leaf1->area, leaf2->area))
        return false;       // a door blocks it
    return true;
}

/*
=================
PF_inPVS

Also checks portalareas so that doors block sight
=================
*/
static qboolean PF_inPVS(const vec3_t p1, const vec3_t p2)
{
    return PF_inVIS(p1, p2, DVIS_PVS);
}

/*
=================
PF_inPHS

Also checks portalareas so that doors block sound
=================
*/
static qboolean PF_inPHS(const vec3_t p1, const vec3_t p2)
{
    return PF_inVIS(p1, p2, DVIS_PHS);
}

/*
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

If channel & 8, the sound will be sent to everyone, not just
things in the PHS.

FIXME: if entity isn't in PHS, they must be forced to be sent or
have the origin explicitly sent.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)

Timeofs can range from 0.0 to 0.1 to cause sounds to be started
later in the frame than they normally would.

If origin is NULL, the origin is determined from the entity origin
or the midpoint of the entity box for bmodels.
==================
*/
static void SV_StartSound(const vec3_t origin, edict_t *edict,
                          int channel, int soundindex, float volume,
                          float attenuation, float timeofs)
{
    int         i, ent, vol, att, ofs, flags, sendchan;
    vec3_t      origin_v;
    client_t    *client;
    byte        mask[VIS_MAX_BYTES];
    mleaf_t     *leaf1, *leaf2;
    message_packet_t    *msg;
    bool        force_pos;

    if (!edict)
        Com_Error(ERR_DROP, "%s: edict = NULL", __func__);
    if (volume < 0 || volume > 1)
        Com_Error(ERR_DROP, "%s: volume = %f", __func__, volume);
    if (attenuation < 0 || attenuation > 4)
        Com_Error(ERR_DROP, "%s: attenuation = %f", __func__, attenuation);
    if (timeofs < 0 || timeofs > 0.255f)
        Com_Error(ERR_DROP, "%s: timeofs = %f", __func__, timeofs);
    if (soundindex < 0 || soundindex >= svs.csr.max_sounds)
        Com_Error(ERR_DROP, "%s: soundindex = %d", __func__, soundindex);

    vol = volume * 255;
    att = min(attenuation * 64, 255);   // need to clip due to check above
    ofs = timeofs * 1000;

    ent = NUM_FOR_EDICT(edict);

    sendchan = (ent << 3) | (channel & 7);

    // always send the entity number for channel overrides
    flags = SND_ENT;
    if (vol != 255)
        flags |= SND_VOLUME;
    if (att != 64)
        flags |= SND_ATTENUATION;
    if (ofs)
        flags |= SND_OFFSET;
    if (soundindex > 255)
        flags |= SND_INDEX16;

    // send origin for invisible entities
    // the origin can also be explicitly set
    force_pos = (edict->svflags & SVF_NOCLIENT) || origin;

    // use the entity origin unless it is a bmodel or explicitly specified
    if (!origin) {
        if (edict->solid == SOLID_BSP) {
            VectorAvg(edict->mins, edict->maxs, origin_v);
            VectorAdd(origin_v, edict->s.origin, origin_v);
            origin = origin_v;
        } else {
            origin = edict->s.origin;
        }
    }

    // prepare multicast message
    MSG_WriteByte(svc_sound);
    MSG_WriteByte(flags | SND_POS);
    if (flags & SND_INDEX16)
        MSG_WriteShort(soundindex);
    else
        MSG_WriteByte(soundindex);

    if (flags & SND_VOLUME)
        MSG_WriteByte(vol);
    if (flags & SND_ATTENUATION)
        MSG_WriteByte(att);
    if (flags & SND_OFFSET)
        MSG_WriteByte(ofs);

    MSG_WriteShort(sendchan);
    MSG_WritePos(origin);

    // if the sound doesn't attenuate, send it to everyone
    // (global radio chatter, voiceovers, etc)
    if (attenuation == ATTN_NONE)
        channel |= CHAN_NO_PHS_ADD;

    // multicast if force sending origin
    if (force_pos) {
        if (channel & CHAN_NO_PHS_ADD) {
            if (channel & CHAN_RELIABLE) {
                SV_Multicast(NULL, MULTICAST_ALL_R);
            } else {
                SV_Multicast(NULL, MULTICAST_ALL);
            }
        } else {
            if (channel & CHAN_RELIABLE) {
                SV_Multicast(origin, MULTICAST_PHS_R);
            } else {
                SV_Multicast(origin, MULTICAST_PHS);
            }
        }
        return;
    }

    leaf1 = NULL;
    if (!(channel & CHAN_NO_PHS_ADD)) {
        leaf1 = CM_PointLeaf(&sv.cm, origin);
        BSP_ClusterVis(sv.cm.cache, mask, leaf1->cluster, DVIS_PHS);
    }

    // decide per client if origin needs to be sent
    FOR_EACH_CLIENT(client) {
        // do not send sounds to connecting clients
        if (!CLIENT_ACTIVE(client)) {
            continue;
        }

        // PHS cull this sound
        if (!(channel & CHAN_NO_PHS_ADD)) {
            leaf2 = CM_PointLeaf(&sv.cm, client->edict->s.origin);
            if (!CM_AreasConnected(&sv.cm, leaf1->area, leaf2->area))
                continue;
            if (leaf2->cluster == -1)
                continue;
            if (!Q_IsBitSet(mask, leaf2->cluster))
                continue;
        }

        // reliable sounds will always have position explicitly set,
        // as no one guarantees reliables to be delivered in time
        if (channel & CHAN_RELIABLE) {
            SV_ClientAddMessage(client, MSG_RELIABLE);
            continue;
        }

        // default client doesn't know that bmodels have weird origins
        if (edict->solid == SOLID_BSP && client->protocol == PROTOCOL_VERSION_DEFAULT) {
            SV_ClientAddMessage(client, 0);
            continue;
        }

        if (LIST_EMPTY(&client->msg_free_list)) {
            Com_WPrintf("%s: %s: out of message slots\n",
                        __func__, client->name);
            continue;
        }

        msg = LIST_FIRST(message_packet_t, &client->msg_free_list, entry);

        msg->cursize = 0;
        msg->flags = flags;
        msg->index = soundindex;
        msg->volume = vol;
        msg->attenuation = att;
        msg->timeofs = ofs;
        msg->sendchan = sendchan;
        for (i = 0; i < 3; i++) {
            msg->pos[i] = COORD2SHORT(origin[i]);
        }

        List_Remove(&msg->entry);
        List_Append(&client->msg_unreliable_list, &msg->entry);
        client->msg_unreliable_bytes += MAX_SOUND_PACKET;
    }

    // clear multicast buffer
    SZ_Clear(&msg_write);

    SV_MvdStartSound(ent, channel, flags, soundindex, vol, att, ofs);
}

static void PF_StartSound(edict_t *entity, int channel,
                          int soundindex, float volume,
                          float attenuation, float timeofs)
{
    if (!entity)
        return;
    SV_StartSound(NULL, entity, channel, soundindex, volume, attenuation, timeofs);
}

void PF_Pmove(pmove_t *pm)
{
    if (sv_client) {
        Pmove(pm, &sv_client->pmp);
    } else {
        Pmove(pm, &sv_pmp);
    }
}

static cvar_t *PF_cvar(const char *name, const char *value, int flags)
{
    if (flags & CVAR_EXTENDED_MASK) {
        Com_WPrintf("Game attemped to set extended flags on '%s', masked out.\n", name);
        flags &= ~CVAR_EXTENDED_MASK;
    }

    return Cvar_Get(name, value, flags | CVAR_GAME);
}

#if USE_CLIENT && defined(REFERENCE_B_B2)
static void SV_B2GameExit(const char *string);
#endif
static void PF_AddCommandString(const char *string)
{
#if USE_CLIENT
    if (!strcmp(string, "menu_loadgame\n"))
        string = "pushmenu loadgame\n";
#endif
#if USE_CLIENT && defined(REFERENCE_B_B2)
    if (!strncmp(string, "gamemap ", 8) && Cvar_VariableInteger("b2_lifecycle") == 1)
        SV_B2GameExit(string);
#endif
    Cbuf_AddText(&cmd_buffer, string);
}

static void PF_SetAreaPortalState(int portalnum, qboolean open)
{
    if (!sv.cm.cache) {
        Com_Error(ERR_DROP, "%s: no map loaded", __func__);
    }
    CM_SetAreaPortalState(&sv.cm, portalnum, open);
}

static qboolean PF_AreasConnected(int area1, int area2)
{
    if (!sv.cm.cache) {
        Com_Error(ERR_DROP, "%s: no map loaded", __func__);
    }
    return CM_AreasConnected(&sv.cm, area1, area2);
}

static void *PF_TagMalloc(unsigned size, unsigned tag)
{
    if (tag > UINT16_MAX - TAG_MAX) {
        Com_Error(ERR_DROP, "%s: bad tag", __func__);
    }
    return Z_TagMallocz(size, tag + TAG_MAX);
}

static void PF_FreeTags(unsigned tag)
{
    if (tag > UINT16_MAX - TAG_MAX) {
        Com_Error(ERR_DROP, "%s: bad tag", __func__);
    }
    Z_FreeTags(tag + TAG_MAX);
}

static void PF_DebugGraph(float value, int color)
{
}

static int PF_LoadFile(const char *path, void **buffer, unsigned flags, unsigned tag)
{
    if (tag > UINT16_MAX - TAG_MAX) {
        Com_Error(ERR_DROP, "%s: bad tag", __func__);
    }
    return FS_LoadFileEx(path, buffer, flags, tag + TAG_MAX);
}

static void *PF_TagRealloc(void *ptr, size_t size)
{
    if (!ptr && size) {
        Com_Error(ERR_DROP, "%s: untagged allocation not allowed", __func__);
    }
    return Z_Realloc(ptr, size);
}

//==============================================

static const game_import_t game_import = {
    .multicast = SV_Multicast,
    .unicast = PF_Unicast,
    .bprintf = PF_bprintf,
    .dprintf = PF_dprintf,
    .cprintf = PF_cprintf,
    .centerprintf = PF_centerprintf,
    .error = PF_error,

    .linkentity = PF_LinkEdict,
    .unlinkentity = PF_UnlinkEdict,
    .BoxEdicts = SV_AreaEdicts,
    .trace = SV_Trace,
    .pointcontents = SV_PointContents,
    .setmodel = PF_setmodel,
    .inPVS = PF_inPVS,
    .inPHS = PF_inPHS,
    .Pmove = PF_Pmove,

    .modelindex = PF_ModelIndex,
    .soundindex = PF_SoundIndex,
    .imageindex = PF_ImageIndex,

    .configstring = PF_configstring,
    .sound = PF_StartSound,
    .positioned_sound = SV_StartSound,

    .WriteChar = MSG_WriteChar,
    .WriteByte = MSG_WriteByte,
    .WriteShort = MSG_WriteShort,
    .WriteLong = MSG_WriteLong,
    .WriteFloat = PF_WriteFloat,
    .WriteString = MSG_WriteString,
    .WritePosition = MSG_WritePos,
    .WriteDir = MSG_WriteDir,
    .WriteAngle = MSG_WriteAngle,

    .TagMalloc = PF_TagMalloc,
    .TagFree = Z_Free,
    .FreeTags = PF_FreeTags,

    .cvar = PF_cvar,
    .cvar_set = Cvar_UserSet,
    .cvar_forceset = Cvar_Set,

    .argc = Cmd_Argc,
    .argv = Cmd_Argv,
    .args = Cmd_RawArgs,
    .AddCommandString = PF_AddCommandString,

    .DebugGraph = PF_DebugGraph,
    .SetAreaPortalState = PF_SetAreaPortalState,
    .AreasConnected = PF_AreasConnected,
};

static const game_import_ex_t game_import_ex = {
    .apiversion = GAME_API_VERSION_EX,

    .OpenFile = FS_OpenFile,
    .CloseFile = FS_CloseFile,
    .LoadFile = PF_LoadFile,

    .ReadFile = FS_Read,
    .WriteFile = FS_Write,
    .FlushFile = FS_Flush,
    .TellFile = FS_Tell,
    .SeekFile = FS_Seek,
    .ReadLine = FS_ReadLine,

    .ListFiles = FS_ListFiles,
    .FreeFileList = FS_FreeList,

    .ErrorString = Q_ErrorString,
    .TagRealloc = PF_TagRealloc,
};

static void *game_library;
#include "../../../../baseline/b2-consolidated215/inputs/origin-002/include/refresh/b2_weapon_bridge.h"
static b2w_game_api_t b2w_api;
#if USE_CLIENT && defined(REFERENCE_B_B2)
#include "../../../../baseline/b2-consolidated215/inputs/origin-002/include/b2/reference_b_b2_lifecycle.h"
// Manual lifecycle map changes require the current stock local game and its
// existing save policy. This admits no provider, mode or save-root changes.
bool SV_B2GameMapAllowed(void)
{
    if (!game_library || !ge || ge->apiversion!=GAME_API_VERSION || !svs.initialized ||
        sv.state!=ss_game || !sv_running->integer || dedicated->integer ||
        sv_maxclients->integer!=1 || sys_forcegamelib->string[0] || fs_game->string[0] ||
        Cvar_VariableInteger("coop") || SV_NoSaveGames()) return false;
    edict_t *ent=EDICT_NUM(1);
    return ent->inuse && ent->client;
}

bool SV_B2PauseState(int *frame, player_state_t *player, vec3_t origin)
{
    if (!ge || !svs.initialized || !sv_running->integer || sv_maxclients->integer != 1) return false;
    edict_t *ent=EDICT_NUM(1);
    if (!ent->inuse || !ent->client) return false;
    *frame=sv.framenum; *player=ent->client->ps; VectorCopy(ent->s.origin,origin);
    return true;
}
#endif
#include "shared/b2_combat.h"

#if USE_CLIENT && defined(REFERENCE_B_B2)
static void SV_B2GameExit(const char *string)
{
    bool (*query)(unsigned,const char *) = game_library ? Sys_GetProcAddress(game_library,"B2CombatLevelExit") : NULL;
    if (!query || !ge || !svs.initialized || sv.state != ss_game ||
        dedicated->integer || sv_maxclients->integer != 1 || !query(B2_COMBAT_API,string))
        Com_Error(ERR_DROP,"B2 invalid game exit ownership");
    B2_BeginWorldChange("game-exit");
}

bool SV_B2RestoreWarming(bool begin, unsigned ticks)
{
    if (Cvar_VariableInteger("b2_lifecycle") != 1) return true;
    bool (*query)(unsigned,bool,unsigned) = game_library ? Sys_GetProcAddress(game_library,"B2CombatWarming") : NULL;
    return query && ge && svs.initialized && sv.state == ss_loading &&
        !dedicated->integer && sv_maxclients->integer == 1 && query(B2_COMBAT_API,begin,ticks);
}
#endif

bool SV_B2CombatState(const player_state_t *ps, b2_combat_state_t *out)
{
    if (!game_library || !ge) return false;
    b2_combat_query_t query = Sys_GetProcAddress(game_library, "GetB2CombatState");
    return query && query(B2_COMBAT_API, ps, out);
}

/*
===============
SV_ShutdownGameProgs

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
void SV_ShutdownGameProgs(void)
{
    if (b2w_api.reset) b2w_api.reset();
    memset(&b2w_api,0,sizeof(b2w_api));
    gex = NULL;
    if (ge) {
        ge->Shutdown();
        ge = NULL;
    }
    if (game_library) {
        Sys_FreeLibrary(game_library);
        game_library = NULL;
    }
    Cvar_Set("g_features", "0");

    Z_LeakTest(TAG_FREE);
}

static void *SV_LoadGameLibraryFrom(const char *path)
{
    void *entry;

    entry = Sys_LoadLibrary(path, "GetGameAPI", &game_library);
    if (!entry)
        Com_EPrintf("Failed to load game library: %s\n", Com_GetLastError());
    else
        Com_Printf("Loaded game library from %s\n", path);

    return entry;
}

static void *SV_LoadGameLibrary(const char *libdir, const char *gamedir)
{
    char path[MAX_OSPATH];

    if (Q_concat(path, sizeof(path), libdir,
                 PATH_SEP_STRING, gamedir, PATH_SEP_STRING,
                 "game" CPUSTRING LIBSUFFIX) >= sizeof(path)) {
        Com_EPrintf("Game library path length exceeded\n");
        return NULL;
    }

    if (os_access(path, X_OK)) {
        { if (Cvar_VariableInteger("developer") > 0) Com_Printf("Can't access %s: %s\n", path, strerror(errno)); }
        return NULL;
    }

    return SV_LoadGameLibraryFrom(path);
}

/*
===============
SV_InitGameProgs

Init the game subsystem for a new map
===============
*/
void SV_InitGameProgs(void)
{
    game_import_t   import;
    game_entry_t    entry = NULL;

    // unload anything we have now
    SV_ShutdownGameProgs();

    // for debugging or `proxy' mods
    if (sys_forcegamelib->string[0])
        entry = SV_LoadGameLibraryFrom(sys_forcegamelib->string);

    // try game first
    if (!entry && fs_game->string[0]) {
        if (sys_homedir->string[0])
            entry = SV_LoadGameLibrary(sys_homedir->string, fs_game->string);
        if (!entry)
            entry = SV_LoadGameLibrary(sys_libdir->string, fs_game->string);
    }

    // then try baseq2
    if (!entry) {
        if (sys_homedir->string[0])
            entry = SV_LoadGameLibrary(sys_homedir->string, BASEGAME);
        if (!entry)
            entry = SV_LoadGameLibrary(sys_libdir->string, BASEGAME);
    }

    // all paths failed
    if (!entry)
        Com_Error(ERR_DROP, "Failed to load game library");

    // load a new game dll
    import = game_import;

    ge = entry(&import);
    if (!ge) {
        Com_Error(ERR_DROP, "Game library returned NULL exports");
    }

    if (ge->apiversion != GAME_API_VERSION) {
        Com_Error(ERR_DROP, "Game library is version %d, expected %d",
                  ge->apiversion, GAME_API_VERSION);
    }

    // get extended api if present
    game_entry_ex_t entry_ex = Sys_GetProcAddress(game_library, "GetExtendedGameAPI");
    if (entry_ex)
        gex = entry_ex(&game_import_ex);

    // initialize
    ge->Init();

    if (g_features->integer & GMF_PROTOCOL_EXTENSIONS) {
        Com_Printf("Game supports Q2PRO protocol extensions.\n");
        svs.csr = cs_remap_new;
    }

    // sanitize edict_size
    unsigned min_size = svs.csr.extended ? sizeof(edict_t) : q_offsetof(edict_t, x);
    unsigned max_size = INT_MAX / svs.csr.max_edicts;

    if (ge->edict_size < min_size || ge->edict_size > max_size || ge->edict_size % q_alignof(edict_t)) {
        Com_Error(ERR_DROP, "Game library returned bad size of edict_t");
    }

    // sanitize max_edicts
    if (ge->max_edicts <= sv_maxclients->integer || ge->max_edicts > svs.csr.max_edicts) {
        Com_Error(ERR_DROP, "Game library returned bad number of max_edicts");
    }
}


#if USE_CLIENT && defined(REFERENCE_B_B2)
bool SV_B2WeaponAPI(b2w_game_api_t *out)
{
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (!game_library || !ge || ge->apiversion!=GAME_API_VERSION || !svs.initialized ||
        sv.state!=ss_game || dedicated->integer || sv_maxclients->integer!=1 || sys_forcegamelib->string[0] ||
        (fs_game->string[0] && strcmp(fs_game->string,BASEGAME))) return false;
    edict_t *ent=EDICT_NUM(1); if (!ent->inuse || !ent->client) return false;
    if (!b2w_api.version) {
        b2w_get_api_t query=Sys_GetProcAddress(game_library,"GetB2WeaponAPI");
        b2w_host_t host={sizeof(host),B2W_API,Sys_Milliseconds,B2W_Current,B2W_Context,B2WE_Owner,B2WE_Record,B2WN_Enabled,B2WN_Sample,B2RM_Current,B2RM_Accept,B2TP_Enabled,B2TP_Current,B2TP_Result};
        b2w_game_api_t api={0};
        if (!query || !query(B2W_API,sizeof(api),&host,&api) || api.version!=B2W_API || api.size!=sizeof(api) ||
            !api.inventory || !api.preview || !api.select || !api.reset || !api.teleport_preview) return false;
        b2w_api=api;
    }
    *out=b2w_api; return true;
}

// Presentation-only snapshot of the current authoritative local collision hull.
// This reads the existing edict ABI and changes no collision or private game API.
bool SV_B2TeleportHull(b2tp_hull_t *out)
{
    if(!out)return false;
    memset(out,0,sizeof(*out));
    b2w_game_api_t api;
    if(!B2TP_Enabled() || !B2W_Requested() || !SV_B2WeaponAPI(&api))return false;
    edict_t *ent=EDICT_NUM(1);
    if(ent->solid!=SOLID_BBOX || ent->client->ps.pmove.pm_type!=PM_NORMAL ||
       ent->client->ps.stats[STAT_HEALTH]<=0)return false;
    b2tp_hull_t hull;
    VectorCopy(ent->s.origin,hull.origin);VectorCopy(ent->mins,hull.mins);VectorCopy(ent->maxs,hull.maxs);
    if(!b2tp_hull_valid(&hull))return false;
    *out=hull;return true;
}
#endif

#if USE_CLIENT && defined(REFERENCE_B_B2)
bool SV_B2ItemState(b2wi_inventory_t *out)
{
    b2w_game_api_t api;
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (!B2W_Requested() || !SV_B2WeaponAPI(&api)) return false;
    b2wi_query_t query=Sys_GetProcAddress(game_library,"GetB2ItemState");
    if (!query || !query(B2WI_API,sizeof(*out),out) || !out->epoch) {
        memset(out,0,sizeof(*out));return false;
    }
    out->capture_ms=Sys_Milliseconds();return true;
}
bool SV_B2UseItem(const b2wi_inventory_t *shown, unsigned slot, uint64_t serial, uint32_t owner)
{
    static uint64_t seen;
    static uint32_t seen_owner,seen_epoch;
    b2wi_inventory_t current,after;
    if (!shown || slot>=B2WI_COUNT || !serial || !owner ||
        (uint32_t)(Sys_Milliseconds()-shown->capture_ms)>100 || !SV_B2ItemState(&current) ||
        current.epoch!=shown->epoch) return false;
    if (owner!=seen_owner || current.epoch!=seen_epoch) {
        seen=0;seen_owner=owner;seen_epoch=current.epoch;
    }
    if (serial<=seen) return false;
    seen=serial; // A refused edge is consumed too; there is no retry queue.
    if (!shown->index[slot] || shown->index[slot]!=current.index[slot] ||
        shown->count[slot]!=current.count[slot] || !(shown->available&(1u<<slot)) ||
        !(current.available&(1u<<slot))) return false;
    char saved[MAX_STRING_CHARS],command[96];
    Q_strlcpy(saved,Cmd_RawString(),sizeof(saved));
    Q_snprintf(command,sizeof(command),"use %s",b2wi_names[slot]);
    Cmd_TokenizeString(command,false);
    ge->ClientCommand(EDICT_NUM(1)); // Stock Cmd_Use_f and the stock item use callback.
    Cmd_TokenizeString(saved,false);
    if (!SV_B2ItemState(&after) || after.epoch!=current.epoch) return false;
    return after.count[slot]<current.count[slot] || after.active!=current.active;
}

// One deliberate grid selection; stock GiveItems owns inventory/health writes.
// No public game/save/usercmd ABI extension and no automatic startup grant.
int SV_B2GiveAll(const b2wi_inventory_t *shown,uint64_t serial,uint32_t owner)
{
    static uint64_t seen;
    static uint32_t seen_owner,seen_epoch;
    b2wi_inventory_t current,after;
    int frame,after_frame;player_state_t before,player,expected;vec3_t origin,after_origin;
    if (!shown || !serial || !owner || B2_UISimulationHeld() ||
        (uint32_t)(Sys_Milliseconds()-shown->capture_ms)>100 ||
        !SV_B2ItemState(&current) || current.epoch!=shown->epoch ||
        !SV_B2PauseState(&frame,&before,origin)) return 0;
    if (owner!=seen_owner || current.epoch!=seen_epoch) {
        seen=0;seen_owner=owner;seen_epoch=current.epoch;
    }
    if (serial<=seen) return 0;
    seen=serial; // Consume even a refused selection; never buffer a retry.
    if (memcmp(current.index,shown->index,sizeof(current.index)) ||
        memcmp(current.count,shown->count,sizeof(current.count)) ||
        current.owned!=shown->owned || current.available!=shown->available ||
        current.active!=shown->active) return 0;
    char saved[MAX_STRING_CHARS];
    Q_strlcpy(saved,Cmd_RawString(),sizeof(saved));
    Cmd_TokenizeString("give all",false);
    ge->ClientCommand(EDICT_NUM(1));
    Cmd_TokenizeString(saved,false);
    if (!SV_B2PauseState(&after_frame,&player,after_origin) ||
        after_frame!=frame || memcmp(origin,after_origin,sizeof(origin)) ||
        !SV_B2ItemState(&after) || after.epoch!=current.epoch) return -1;
    expected=before;
    expected.stats[STAT_PICKUP_ICON]=player.stats[STAT_PICKUP_ICON];
    expected.stats[STAT_PICKUP_STRING]=player.stats[STAT_PICKUP_STRING];
    expected.stats[STAT_SELECTED_ITEM]=player.stats[STAT_SELECTED_ITEM];
    return memcmp(&expected,&player,sizeof(player)) ? -1:1;
}
#endif

#include "b2_feedback304.h"
bool SV_B2Pickup304(b2_pickup304_t *out)
{
 b2w_game_api_t api;if(!out)return false;memset(out,0,sizeof(*out));
 if(!B2W_Requested() || !SV_B2WeaponAPI(&api))return false;
 typedef bool (*query_t)(unsigned,b2_pickup304_t*);
 query_t query=Sys_GetProcAddress(game_library,"GetB2Pickup304");
 return query && query(sizeof(*out),out) && out->epoch;
}
