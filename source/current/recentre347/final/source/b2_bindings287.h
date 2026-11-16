// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_BINDINGS287_H
#define B2_BINDINGS287_H
#ifndef B2_CONTROLLER_H
#include "../../../../baseline/b2-consolidated215/inputs/origin-044/include/refresh/b2_controller.h"
#endif
enum { B2B_MOVE,B2B_TURN,B2B_FIRE,B2B_JUMP,B2B_CROUCH,B2B_TELEPORT,
 B2B_USE,B2B_SELECT,B2B_RECENTER,B2B_MENU,B2B_EXTRA,B2B_RESUME,B2B_RUN,B2B_ITEMS,B2B_INVENTORY,B2B_HELP,B2B_CYCLE347,B2B_COUNT };
enum { B2B_STICK,B2B_TRIGGER,B2B_GRIP,B2B_PRIMARY,B2B_SECONDARY,B2B_CLICK,B2B_PARTS };
// Source: physical left/right or dominant/off hand, then existing component.
typedef struct { int preset; int input[B2B_COUNT]; } b2b_config_t;
unsigned B2B_Hand(void);
bool B2B_TeleportOnly(void);
enum { B2B_OPEN_INVENTORY316=16, B2B_OPEN_HELP316=32, B2B_CYCLE338=64 };
void B2B_SaveCustom316(const b2b_config_t *in);
void B2B_Custom316(b2b_config_t *out);
bool B2B_Quest316(void);
bool B2B_Cycle338(void);
bool B2B_MenuAvailable316(unsigned hand);
int B2B_Profile316(unsigned hand);
void B2B_Read(b2b_config_t *out);
bool B2B_Apply(const b2b_config_t *in);
int B2B_Source(unsigned action,unsigned dominant);
float B2B_Value(const b2c_snapshot_t *p,unsigned action,unsigned dominant,unsigned axis);
void B2B_Clear(b2c_snapshot_t *p,unsigned action,unsigned dominant);
unsigned B2B_Revision(void);
bool B2B_Barrier(const b2c_snapshot_t *p,unsigned dominant);
void B2B_Invalidate(void);
bool B2B_Label(unsigned source,char *label,unsigned capacity);
#endif
