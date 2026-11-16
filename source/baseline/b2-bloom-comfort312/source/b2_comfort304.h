// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_TELEPORT_COMFORT_H
#define B2_TELEPORT_COMFORT_H
#include <math.h>
enum { B2TP_MASK_GRID=48 };
// One full physical-eye angular mask. Reproject that same field into each view;
// the foveal image never defines a second mask centre or radius.
static bool B2TP_ComfortQuad(const float fov[4],const float physical_inv[16],
    const float view[16],const float projection[16],unsigned row,unsigned col,
    float strength,float radius,unsigned physical,float clip[4][4],float *alpha)
{
    if(physical>1 || row>=B2TP_MASK_GRID || col>=B2TP_MASK_GRID || !isfinite(strength) || strength<=0)return false;
    float l=tanf(fov[0]),r=tanf(fov[1]),u=tanf(fov[2]),d=tanf(fov[3]);
    if(!isfinite(l)||!isfinite(r)||!isfinite(u)||!isfinite(d)||r<=l||u<=d)return false;
    float cx=((float)col+.5f)/B2TP_MASK_GRID*2.f-1.f;
    float cy=((float)row+.5f)/B2TP_MASK_GRID*2.f-1.f;
    if(!isfinite(radius))radius=.25f;
    radius=fminf(.75f,fmaxf(.1f,radius));
    // Physical0 is left: its +camera-right side is nasal; physical1 mirrors it.
    // Extend only the nasal half with a C1 smooth ramp over 0..0.5 half-view.
    // Keep the 0.18 midline extension; broaden its smooth vertical taper.
    // leaves the complete temporal half and original soft transition intact.
    float nasal=physical==0?cx:-cx;
    float t=fminf(1.f,fmaxf(0.f,nasal/.5f));
    float relief=.18f*t*t*(3.f-2.f*t)*expf(-2.f*cy*cy);
    float edge=fminf(1.f,fmaxf(0.f,(sqrtf(cx*cx+cy*cy)-radius-relief)/.25f));
    *alpha=fminf(1.f,strength)*edge*edge*(3.f-2.f*edge);
    if(*alpha<=0)return false;
    const unsigned corners[4][2]={{0,1},{0,0},{1,1},{1,0}};
    for(unsigned c=0;c<4;c++){
        float x=l+(r-l)*(float)(col+corners[c][0])/B2TP_MASK_GRID;
        float y=u+(d-u)*(float)(row+corners[c][1])/B2TP_MASK_GRID;
        float ray[3],camera[3];
        for(unsigned k=0;k<3;k++)ray[k]=physical_inv[k]*x+physical_inv[4+k]*y+physical_inv[8+k];
        for(unsigned k=0;k<3;k++)camera[k]=view[k]*ray[0]+view[4+k]*ray[1]+view[8+k]*ray[2];
        for(unsigned k=0;k<4;k++){
            clip[c][k]=projection[k]*camera[0]+projection[4+k]*camera[1]+projection[8+k]*camera[2];
            if(!isfinite(clip[c][k]))return false;
        }
        if(clip[c][3]<=.0001f)return false;
        clip[c][2]=0;
    }
    return true;
}
#endif
