// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Private optional in-process API; no network, public game or save ABI fields.
#ifndef B2_WEAPON_BRIDGE_H
#define B2_WEAPON_BRIDGE_H
#include "b2_weapon.h"
#include "b2_weapon_model.h"
#define B2W_API 0x42325709u
struct b2rm_request_s;
struct b2tp_request_s;
struct b2tp_preview_s;
#define B2W_CONTEXT_REQUESTED 1u
#define B2W_CONTEXT_NEW_COMMAND 2u
#define B2W_CONTEXT_COMMAND 4u
#define B2W_CAP_INVENTORY 1u
#define B2W_CAP_PREVIEW 2u
#define B2W_CAP_SELECTION 4u
#define B2W_CAP_FIRE 8u
typedef struct {
    int16_t angles[3], forward, side, up;
    uint8_t buttons, msec, impulse, light;
} b2w_command_t;
typedef struct {
    b2w_pose_t pose;
    uint64_t selection;
    uint32_t selection_ms, game_epoch;
    int slot, weapon;
    bool attack;
} b2w_packet_t;
typedef struct {
    uint32_t epoch;
    uint16_t owned, available;
    int current, pending;
    bool ready;
} b2w_inventory_t;

// One completed stock fire call, owned by an immutable consumed command.
// Kept in the optional private API; never in public game or save structures.
#define B2WE_VERSION 0x42324502u
typedef struct {
    unsigned size, version;
    b2w_packet_t packet;
    b2w_command_t command;
    uint32_t number, sequence, ordinal, handle;
    b2wm_key_t model;
    b2w_pose_t aim;
} b2we_owner_t;
typedef struct {
    unsigned size, version;
    b2we_owner_t owner;
    uint64_t serial;
    uint32_t completed_ms;
    int frame, gunframe;
    float corrected[3], direction[3], body_angles[3], anchor[3];
} b2we_event_t;
bool B2WE_OwnerAt(b2we_owner_t *out, uint32_t now);
bool B2WE_Owner(b2we_owner_t *out);
bool B2WE_RecordAt(const b2we_event_t *event, uint32_t now);
bool B2WE_Record(const b2we_event_t *event);
bool B2WE_NextAt(uint64_t after, b2we_event_t *out, uint32_t now);

// Simulation-owned state. No public/save/packet fields; model sampling is pure.
#define B2WN_VERSION 0x42324e03u
typedef struct {
    unsigned size, version;
    uint32_t epoch, captured_ms;
    int slot, frame, gunframe, gunindex, hand;
    int model_frame; // Aiming geometry; gunframe retains actual event identity.
    bool emission, idle;
    float recoil[3]; // Weapon/damage kick before desktop movement sway.
    float body_origin[3], body_angles[3], origin[3], angles[3];
} b2wn_state_t;
typedef struct {
    b2w_pose_t pose;
    float anchor[3];
} b2wn_pose_t;
bool B2WN_Enabled(void);
bool B2WN_Sample(const b2wn_state_t *state, const b2we_owner_t *owner, b2wn_pose_t *out);
typedef struct {
    unsigned size, version;
    uint32_t (*milliseconds)(void);
    bool (*current)(b2w_packet_t *out);
    unsigned (*context)(void);
    bool (*emission_owner)(b2we_owner_t *out);
    bool (*emitted)(const b2we_event_t *event);
    bool (*nominal_enabled)(void);
    bool (*nominal)(const b2wn_state_t *, const b2we_owner_t *, b2wn_pose_t *);
    bool (*room_current)(struct b2rm_request_s *);
    bool (*room_accepted)(uint32_t number,const float accepted[3]);
    bool (*teleport_enabled)(void);
    bool (*teleport_current)(struct b2tp_request_s *);
    bool (*teleport_result)(uint32_t,bool,const float[3],uint8_t,uint8_t);
} b2w_host_t;
// Authoritative private held-grenade lifecycle, never inferred from animation.
enum { B2WG_INVALID, B2WG_READY, B2WG_HELD, B2WG_RELEASED, B2WG_DETONATED, B2WG_INTERRUPTED };
typedef struct {
    uint64_t cycle, emission_serial;
    unsigned state;
    int frame, gunframe;
} b2wg_state_t;

typedef struct {
    unsigned size, version, capabilities;
    bool (*inventory)(b2w_inventory_t *out);
    bool (*preview)(const b2w_packet_t *packet, float corrected[3]);
    bool (*select)(uint64_t token, int slot);
    void (*reset)(void);
    bool (*grenade)(const b2we_owner_t *owner, b2wg_state_t *out);
    bool (*motion)(b2wn_state_t *out);
    bool (*teleport_preview)(const b2w_packet_t *,float,struct b2tp_preview_s *);
} b2w_game_api_t;
typedef bool (*b2w_get_api_t)(unsigned version, unsigned size,
    const b2w_host_t *host, b2w_game_api_t *out);

void B2W_Reset(void);
void B2W_SetContext(bool requested, const b2w_epochs_t *epochs, bool attack, uint32_t now);
bool B2W_Requested(void);
bool B2W_Publish(const b2w_packet_t *packet, uint32_t now);
bool B2W_Ready(uint32_t now);
void B2W_Finalise(uint32_t number, const b2w_command_t *command, uint32_t now);
void B2W_BeginPacket(uint32_t sequence);
void B2W_Bind(uint32_t sequence, unsigned ordinal, uint32_t number,
    const b2w_command_t *wire, uint32_t now);
bool B2W_Enter(uint32_t sequence, unsigned ordinal, const b2w_command_t *command,
    bool fresh, uint32_t now);
void B2W_Leave(void);
bool B2W_CurrentAt(b2w_packet_t *out, uint32_t now);
unsigned B2W_Context(void);
void B2W_PreServerFrame(void);
bool B2W_Current(b2w_packet_t *out);
void B2W_Acknowledge(const b2w_packet_t *packet, bool accepted, uint32_t now);
bool B2W_SelectionAck(uint64_t token, int *slot, bool *accepted, uint32_t now);
bool SV_B2WeaponAPI(b2w_game_api_t *out);
#endif
