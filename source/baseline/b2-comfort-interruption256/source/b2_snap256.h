// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_SNAP256_H
#define B2_SNAP256_H
#include <math.h>
static const float b2_snap_stops256[5]={15,30,45,90,180};
// Nearest stop, ties to the lower stop; nonfinite external input uses45.
static int b2_snap_index256(float value)
{
    if (!isfinite(value)) return 2;
    int best=0;
    for(int i=1;i<5;i++)
        if(fabsf(value-b2_snap_stops256[i])<fabsf(value-b2_snap_stops256[best])) best=i;
    return best;
}
static float b2_snap_value256(float value) {return b2_snap_stops256[b2_snap_index256(value)];}
#endif
