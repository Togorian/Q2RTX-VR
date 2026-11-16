// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Private post-stock-Pmove phase. The stock observer has already checked the
// unmodified stock result; room movement uses a separately consumed authority.
#include "b2_roomscale_pmove.inc"
void B2W_RoomscaleMove(edict_t *ent,pmove_t *pm)
{
    b2rm_request_t request;
    if(!ent || ent!=b2w_live_player() || !b2w_host.room_current || !b2w_host.room_accepted ||
       !b2w_host.room_current(&request))return;
    float accepted[3]={0};b2rs_apply_pmove(pm,&request,accepted);
    if(!b2w_host.room_accepted(request.number,accepted))
        observer_walk_fault("roomscale_acknowledgement_rejected",ent);
}
