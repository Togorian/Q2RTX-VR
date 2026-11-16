// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#include "b2_fps347.h"
#include <math.h>
// Published only at the existing CL_MeasureStats cadence. Both values describe
// the same game-frame sample; no eye count, GPU timing, wait or readback.
static double b2fps_rate347,b2fps_ms347;
static void b2fps_measure347(double frames,double elapsed_ms)
{
    b2fps_rate347=b2fps_ms347=0;
    if(!isfinite(frames) || !isfinite(elapsed_ms) || frames<=0 || elapsed_ms<=0)return;
    double rate=1000.0*frames/elapsed_ms,ms=elapsed_ms/frames;
    if(!isfinite(rate) || !isfinite(ms) || rate>2147483647.0 || ms>4294967295.0)return;
    b2fps_rate347=rate;b2fps_ms347=ms;
}
void B2FPS_Text347(char *fps,size_t fps_size,char *ms,size_t ms_size)
{
    Q_snprintf(fps,fps_size,"%.0f FPS",b2fps_rate347);
    Q_snprintf(ms,ms_size,"%.1f ms/frame",b2fps_ms347);
}

// Compare the original normalised rate, before display rounding or reciprocal
// rounding at a boundary. Both numbers use this same sample and colour.
unsigned B2FPS_Color351(void)
{
    if (!(b2fps_rate347 > 0) || !(b2fps_ms347 > 0)) return ~0u;
    if (b2fps_rate347 >= 90.0) return MakeColor(0,255,0,255);
    if (b2fps_rate347 >= 45.0) return MakeColor(255,255,0,255);
    if (b2fps_rate347 >= 30.0) return MakeColor(255,165,0,255);
    return MakeColor(255,0,0,255);
}
