// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_HF_SAMPLES241_H
#define B2_HF_SAMPLES241_H
#include <stdbool.h>
bool B2_HFSamplesPlaying(void);
void R_B2HFSamplesArm(unsigned label);
void R_B2HFSamplesInterrupt(bool terminal);
void R_B2HFSamplesFrameFinished(bool committed);
#endif
