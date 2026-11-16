// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_RESUME_CUE261_H
#define B2_RESUME_CUE261_H
#include <stdbool.h>
#include <stdint.h>
enum { B2_RESUME_LABEL261 = 48, B2_RESUME_LINE261 = 96 };
typedef struct { char line[3][B2_RESUME_LINE261]; } b2_resume_cue261_t;
// Metadata request only, after the existing input read; never synchronises actions.
void B2XR_UpdateResumeCue(bool needed, uint64_t serial, uint32_t generation, uint32_t input_generation);
bool B2XR_ResumeControlLabel(unsigned hand, uint64_t serial, uint32_t generation,
    uint32_t input_generation, uint32_t now, char label[B2_RESUME_LABEL261]);
bool B2_ResumeCue(b2_resume_cue261_t *cue);
#endif
