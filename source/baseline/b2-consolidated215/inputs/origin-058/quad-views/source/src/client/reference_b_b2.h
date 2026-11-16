// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#ifdef REFERENCE_B_B2
struct refdef_s;
void B2_Init(void);
void SCR_DrawB2HUD(void);
void SCR_DrawB2Aim(float x, float y);
struct b2_view_s;
void R_B2HUD(unsigned eye, const struct b2_view_s *head);
bool B2_AudioListener(void);
void B2_Shutdown(void);
void B2_BeforeRendererShutdown(void);
bool B2_HasFrozenScene(void);
bool B2_RenderStereo(void);
bool B2_ObserveScene(const struct refdef_s *fd);
bool B2_AdvancingWorld(void);
void B2_FilterInput(void);
bool B2_CombatSnapshot(void);
bool B2_KeyboardMode(void);
void B2_InputFocusLost(void);
bool B2_TurningMode(void);
void B2_UpdateLook(int msec, float mouse_yaw, float mouse_pitch);
void CL_B2DiscardMouse(void);
void CL_B2ReleaseButtons(void);
bool B2_UIEnabled(void);
bool B2_UIPaused(void);
bool B2_UISimulationHeld(void);
bool CL_B2PreparePause(void);
bool B2_KeepFramePump(void);
void B2_ProcessInterruptions(void);
void B2_UIFocusChanged(void);
bool B2_UIKey(unsigned key, bool down);
bool B2_UIConsole(const char *text);
void B2_UITogglePause(void);
void B2_UIResume(void);
bool SCR_B2PrepareUI(void);
bool UI_B2Prepare(void);
bool SV_B2PauseState(int *frame, player_state_t *player, vec3_t origin);
bool V_BuildB2WorldScene(void);
#endif

void B2_BeginWorldChange(const char *reason);
void B2_LifecycleDisconnect(bool terminal);
bool B2_LifecycleEnabled(void);
