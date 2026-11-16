// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_REALIGN391_H
#define B2_REALIGN391_H
bool B2_RealignActive391(void);
bool B2_RealignAllowed391(void);
void B2_RealignPrompt391(char *text, unsigned capacity);
bool B2XR_RealignPending391(void);
bool B2XR_CompleteRealign391(uint32_t generation);
void SCR_DrawB2Realignment391(void);
bool B2_ReferenceBasisAllowed397(void);
void B2_ReferenceReset397(bool stable, bool known, uint32_t previous_epoch);
bool B2XR_ReferenceApplied397(void);
#endif
