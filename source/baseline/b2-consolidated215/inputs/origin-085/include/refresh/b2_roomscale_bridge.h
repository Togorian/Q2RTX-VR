// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_ROOMSCALE_BRIDGE_H
#define B2_ROOMSCALE_BRIDGE_H
#include "b2_roomscale.h"
#include "b2_weapon_bridge.h"
typedef struct {float position[3];uint64_t serial;uint32_t captured_ms,generation;bool valid;} b2rs_head_t;
typedef struct b2rm_request_s {
    uint64_t serial;
    uint32_t generation,number,captured_ms;
    b2w_command_t command;
    float movement[3], raw_basis[4], accepted[3];
    bool valid,executed,acknowledged,folded;
} b2rm_request_t;
bool B2XR_ReadHeadInput(b2rs_head_t *);
void B2RM_Reset(void);
bool B2RM_Pending(uint32_t now);
uint32_t B2RM_LastRequestMs(void);
bool B2RM_Publish(uint64_t serial,uint32_t now,const float movement[3],const float basis[4]);
void B2RM_Finalise(uint32_t number,const b2w_command_t *,uint32_t now);
void B2RM_BeginPacket(uint32_t sequence);
void B2RM_Bind(uint32_t sequence,unsigned ordinal,uint32_t number,const b2w_command_t *,uint32_t now);
void B2RM_Enter(uint32_t sequence,unsigned ordinal,const b2w_command_t *,bool fresh,uint32_t now);
void B2RM_Leave(void);
bool B2RM_Current(b2rm_request_t *);
bool B2RM_Accept(uint32_t number,const float accepted[3]);
void B2RM_PredictionBegin(uint32_t ack);
bool B2RM_PredictionRequest(uint32_t number,b2rm_request_t *);
bool B2RM_Predicted(uint32_t number,const float accepted[3]);
void B2RM_Offset(float raw[3]);
void B2RM_AcknowledgedOffset(float raw[3]);
#endif
