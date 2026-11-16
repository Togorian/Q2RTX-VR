// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#ifdef REFERENCE_B_B0U

void B0U_Init(void);
void B0U_Shutdown(void);
bool B0U_Requested(void);
bool B0U_HasFrozenScene(void);
bool B0U_PrepareFrame(refdef_t *fd);
bool B0U_BeforeBeginFrame(void);
void B0U_AfterEndFrame(void);

#endif
