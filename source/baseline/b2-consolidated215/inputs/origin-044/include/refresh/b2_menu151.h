// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_MENU151_H
#define B2_MENU151_H
#include <stdbool.h>
bool B2_MenuSession(void);
bool B2_UISurfaceActive(void);
void B2_CinematicBoundary(bool preparing);
bool B2_MenuCommand(const char *command);
void SCR_DrawB2Surface(void);
void R_B2CinematicPicture(int width, int height);
#define B2UI_CINEMATIC 6u
#endif
