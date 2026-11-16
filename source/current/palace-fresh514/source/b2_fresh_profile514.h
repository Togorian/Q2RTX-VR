// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

static bool B2_FreshRequested(void)
{
    return Cvar_VariableInteger("b2_openxr") == 1 &&
           Cvar_VariableInteger("b2_lifecycle") == 1 &&
           Cvar_VariableInteger("g_protocol_extensions") == 1;
}

static bool B2_FreshRemap(const cs_remap_t *r)
{
    return r && r->extended == cs_remap_new.extended &&
        r->max_edicts == cs_remap_new.max_edicts &&
        r->max_models == cs_remap_new.max_models &&
        r->max_sounds == cs_remap_new.max_sounds &&
        r->max_images == cs_remap_new.max_images &&
        r->airaccel == cs_remap_new.airaccel &&
        r->maxclients == cs_remap_new.maxclients &&
        r->mapchecksum == cs_remap_new.mapchecksum &&
        r->models == cs_remap_new.models && r->sounds == cs_remap_new.sounds &&
        r->images == cs_remap_new.images && r->lights == cs_remap_new.lights &&
        r->items == cs_remap_new.items && r->playerskins == cs_remap_new.playerskins &&
        r->general == cs_remap_new.general && r->end == cs_remap_new.end;
}

static bool B2_FreshPeer(const cs_remap_t *r, int protocol, int minor, bool loopback)
{
    return loopback && B2_FreshRemap(r) && protocol == PROTOCOL_VERSION_Q2PRO &&
           minor >= PROTOCOL_VERSION_Q2PRO_EXTENDED_LIMITS;
}

static bool B2_FreshClient(const cs_remap_t *r, int protocol, int minor,
                           unsigned es_flags, unsigned ps_flags, bool loopback)
{
    return B2_FreshPeer(r, protocol, minor, loopback) &&
           (es_flags & MSG_ES_EXTENSIONS) && (ps_flags & MSG_PS_EXTENSIONS);
}
