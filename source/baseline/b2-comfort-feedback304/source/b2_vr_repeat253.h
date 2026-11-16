// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_VR_REPEAT253_H
#define B2_VR_REPEAT253_H
void UI_B2VRRepeatCancel(void);
bool UI_B2VRRepeatKey(int key, bool down, uint32_t now);
void UI_B2VRRepeatTick(uint32_t now, bool focused);
bool UI_B2VRRepeatController(int dir, bool pressed, uint32_t now);
#endif

void UI_B2ControllerInput304(int hand);

bool UI_B2BindingCapture304(void);
