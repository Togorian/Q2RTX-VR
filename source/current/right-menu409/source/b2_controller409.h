// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_CONTROLLER_H
#define B2_CONTROLLER_H
#include <stdbool.h>
#include <stdint.h>

enum { B2C_BLOCKED, B2C_GAME, B2C_MENU, B2C_CONSOLE, B2C_INTERMISSION, B2C_INACTIVE, B2C_CINEMATIC, B2C_DEAD };
enum { B2C_OPEN_MENU=1, B2C_CONFIRM=2, B2C_BACK=4, B2C_RECENTRE=8 };
enum { B2C_UP=1, B2C_DOWN, B2C_LEFT, B2C_RIGHT };
enum { B2C_MAX_AGE_MS=100, B2C_POSE_GRACE_MS556=200 };
// Modified 12 November 2026: action availability/rearm is separate from pose confidence.
enum { B2C_STICK556=1, B2C_TRIGGER556=2, B2C_GRIP556=4, B2C_PRIMARY556=8,
    B2C_SECONDARY556=16, B2C_CLICK556=32, B2C_MENU556=64 };
typedef struct {
    bool aim_valid, grip_valid, controls_active;
    float aim_position[3], aim_orientation[4];
    float grip_position[3], grip_orientation[4];
    float stick[2], trigger, squeeze;
    bool squeeze_active;
    bool primary, secondary, menu, click;
    unsigned active556, ready556, blocked556;
    bool aim_active556, grip_active556, aim_tracked556, grip_tracked556;
    uint32_t aim_tracked_ms556, grip_tracked_ms556;
} b2c_hand_t;
typedef struct {
    uint64_t serial;
    uint32_t generation, capture_ms, head_ms, xr_generation;
    float head_position[3], head_orientation[4];
    int64_t pose_time;
    bool ready;
    b2c_hand_t hands[2];
} b2c_snapshot_t;
#include "current/input-comfort556/source/b2_input_pose556.h"
typedef struct {
    short forward, side, up;
    bool attack;
    float yaw;
    unsigned events, navigation;
} b2c_intent_t;
// Right dedicated Menu is a short-release gesture; left Menu is unchanged.
typedef struct {
    uint32_t press_ms, sample_ms, xr;
    bool clock_valid, down, pending;
} b2cm_release409_t;

typedef struct {
    b2cm_release409_t menu_release409;
    bool requested, wait_neutral, valid, snap_ready, menu_ready, attack, crouch;
    bool previous_primary, previous_secondary, previous_menu, previous_click;
    bool fire_ready556;
    uint64_t serial;
    uint32_t generation;
    unsigned destination;
    unsigned turn_mode, dominant;
    bool configured, turn_clock_valid;
    uint32_t turn_ms;
    b2c_intent_t intent;
} b2c_state_t;

enum { B2C_TURN_SNAP, B2C_TURN_SMOOTH };

// Last read decision only: no polling, acceptance or gameplay ABI.
typedef struct {
    const char *reason;
    uint32_t now_ms, head_ms;
    int hand;
} b2c_diagnostic_t;
void B2XR_ControllerDiagnostic(b2c_diagnostic_t *out);
void B2_ControllerTick(int msec);
bool B2XR_ReadControllerInput(b2c_snapshot_t *snapshot);
bool B2XR_QuitOnly(void);
bool B2XR_ReadQuitInput(b2c_snapshot_t *snapshot);
#endif
