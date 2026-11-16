// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#ifdef REFERENCE_B_B2
struct refdef_s;
void B2_Init(void);
void B2_Shutdown(void);
void B2_BeforeRendererShutdown(void);
bool B2_HasFrozenScene(void);
bool B2_RenderStereo(void);
bool B2_ObserveScene(const struct refdef_s *fd);
#endif
