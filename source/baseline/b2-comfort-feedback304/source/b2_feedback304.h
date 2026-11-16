// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_FEEDBACK304_H
#define B2_FEEDBACK304_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {uint64_t sequence;uint32_t epoch;} b2_pickup304_t;
bool SV_B2Pickup304(b2_pickup304_t *out);
#endif
