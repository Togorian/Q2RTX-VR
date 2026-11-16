// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_TELEPORT_BRIDGE_H
#define B2_TELEPORT_BRIDGE_H
#include "b2_weapon_bridge.h"
#include "b2_teleport.h"
typedef struct b2tp_request_s {
    b2w_packet_t aim;
    b2w_command_t command;
    float target[3],head[3],range,accepted_origin[3];
    uint32_t number,generation;
    uint8_t accepted_flags,accepted_time;
    bool valid,executed,acknowledged,accepted;
} b2tp_request_t;
void B2TP_Reset(void);
void B2TP_WorldReset(void);
void B2TP_PredictionBegin(uint32_t acknowledged);
void B2TP_SetOwner(bool enabled,const b2w_epochs_t *epochs);
bool B2TP_Enabled(void);
bool B2TP_Expired(uint32_t now);
bool B2TP_Publish(const b2tp_request_t *,uint32_t now);
void B2TP_Finalise(uint32_t number,const b2w_command_t *,uint32_t now);
void B2TP_BeginPacket(uint32_t sequence);
void B2TP_Bind(uint32_t sequence,unsigned ordinal,uint32_t number,const b2w_command_t *,uint32_t now);
void B2TP_Enter(uint32_t sequence,unsigned ordinal,const b2w_command_t *,bool fresh,uint32_t now);
void B2TP_Leave(void);
bool B2TP_Current(b2tp_request_t *);
bool B2TP_Result(uint32_t number,bool accepted,const float origin[3],uint8_t flags,uint8_t time);
bool B2TP_Prediction(uint32_t number,b2tp_request_t *);
void B2_TeleportResult(const b2tp_request_t *);
#endif
