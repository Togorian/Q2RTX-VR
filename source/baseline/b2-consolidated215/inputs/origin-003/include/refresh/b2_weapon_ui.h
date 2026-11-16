// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Private cached flat-HUD state. Rendering never polls native input or game code.
#ifndef B2_WEAPON_UI_H
#define B2_WEAPON_UI_H
#include "b2_weapon_bridge.h"
// Separate private inventory query: stock public game/save/usercmd ABI unchanged.
#define B2WI_API 0x42324901u
#define B2WI_COUNT 7
static const char *const b2wi_names[B2WI_COUNT]={"Quad Damage","Invulnerability","Silencer",
    "Rebreather","Environment Suit","Power Screen","Power Shield"};
static const char *const b2wi_icons[B2WI_COUNT]={"p_quad","p_invulnerability","p_silencer",
    "p_rebreather","p_envirosuit","i_powerscreen","i_powershield"};
typedef struct {
    uint32_t epoch, capture_ms;
    int index[B2WI_COUNT], count[B2WI_COUNT];
    uint16_t owned, available, active;
} b2wi_inventory_t;
typedef bool (*b2wi_query_t)(unsigned version, unsigned size, b2wi_inventory_t *out);
bool SV_B2ItemState(b2wi_inventory_t *out);
bool SV_B2UseItem(const b2wi_inventory_t *shown, unsigned slot, uint64_t serial, uint32_t owner);
int SV_B2GiveAll(const b2wi_inventory_t *shown, uint64_t serial, uint32_t owner);
enum { B2W_UI_NONE, B2W_UI_REARM, B2W_UI_WAITING, B2W_UI_ACCEPTED, B2W_UI_REFUSED, B2W_UI_ITEM_USED, B2W_UI_ITEM_REFUSED };
typedef struct {
    b2w_epochs_t epochs;
    b2w_inventory_t inventory;
    b2wi_inventory_t items;
    unsigned page, slide_from;
    uint32_t capture_ms, status_ms, slide_ms;
    float x, y;
    int candidate, stable, slot, status;
    bool open, cursor, rearm, anchored, sliding, arrow_pressed;
    b2w_pose_t plane;
    float body_origin[3];
} b2w_hud_t;
bool B2W_HUDState(b2w_hud_t *out, uint32_t now);
bool B2W_HideBodyReticle(void);
void R_B2SelectorBegin(const b2w_pose_t *plane);
void R_B2SelectorEnd(void);
void R_B2PageArrow(int x, int y, int w, int h, bool left, int pic);
void R_B2PageClip(bool begin, int x, int y, int w, int h);
#endif
