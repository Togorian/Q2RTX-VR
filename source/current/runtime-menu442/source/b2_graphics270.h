// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
typedef struct { uint32_t width,height,output_width[4],output_height[4],views; int runtime,percent; bool applied; } b2g_info_t;
_Static_assert(sizeof(b2g_info_t)==56,"graphics scalar ABI");
uint32_t B2G_Width(void);
uint32_t B2G_Height(void);
void B2G_Info(b2g_info_t *out);
// Preview only: current session allocation envelope and checked device limits.
bool B2G_Pending442(int percent,uint32_t *width,uint32_t *height);
static inline bool B2G_Mul(uint64_t a,uint64_t b,uint64_t *out)
{ if(!out || (b && a>UINT64_MAX/b))return false;*out=a*b;return true; }
static inline bool B2G_Add(uint64_t a,uint64_t b,uint64_t *out)
{ if(!out || a>UINT64_MAX-b)return false;*out=a+b;return true; }
// Signed pixel indices/coordinates are used by the selected C and GLSL paths.
// Keep their rounded dispatch coordinates representable too. This is an
// arithmetic contract, not a project resolution maximum or a device claim.
static inline bool B2G_Internal(uint32_t w,uint32_t h)
{ return w && h && !(w&1) && w<=INT32_MAX-31u && h<=INT32_MAX-31u && (uint64_t)w*h<=INT32_MAX; }
// Migrate only valid historical scale settings; invalid/nonfinite inputs refuse.
static inline bool B2G_Percent364(float value,int integer,int *out)
{
    if(!out || !isfinite(value) || value!=integer || integer<10 || integer>200 || integer%5)return false;
    *out=integer>100?100:integer;return true;
}
static inline bool B2G_Extent(int runtime,int percent,uint32_t w,uint32_t h,
    uint32_t limit_w,uint32_t limit_h,uint32_t *out_w,uint32_t *out_h)
{
    if(!out_w || !out_h || (runtime!=0 && runtime!=1) || percent<10 || percent>100 || percent%5 ||
       !w || !h || w>INT32_MAX || h>INT32_MAX)return false;
    uint64_t x=runtime?(((uint64_t)w*percent+99)/100+1)&~UINT64_C(1):1948;
    uint64_t y=runtime?((uint64_t)h*percent+99)/100:1563;
    if(x>limit_w || y>limit_h || x>UINT32_MAX || y>UINT32_MAX || !B2G_Internal((uint32_t)x,(uint32_t)y))return false;
    *out_w=(uint32_t)x;*out_h=(uint32_t)y;return true;
}
// Exact33-image per-extra-view inventory; checked even for hostile input.
static inline uint64_t B2G_TemporalBytes(uint32_t w,uint32_t h)
{
    uint64_t a,b,total;
    if(!w || !h || (w&1) || !B2G_Mul((uint64_t)w*h,160,&a) ||
       !B2G_Mul(((uint64_t)w+2)/3,((uint64_t)h+2)/3,&b) ||
       !B2G_Mul(b,8,&b) || !B2G_Add(a,b,&total))return 0;
    return total;
}
static inline uint64_t B2G_TemporalBound(uint32_t w,uint32_t h)
{
    const uint64_t divisor=489865040,multiplier=629145600;
    uint64_t raw=B2G_TemporalBytes(w,h),whole,tail,result;
    if(!raw || !B2G_Mul(raw/divisor,multiplier,&whole))return 0;
    // Remainder is strictly below divisor; its product plus divisor-1 fits64.
    tail=((raw%divisor)*multiplier+divisor-1)/divisor;
    return B2G_Add(whole,tail,&result)?result:0;
}
