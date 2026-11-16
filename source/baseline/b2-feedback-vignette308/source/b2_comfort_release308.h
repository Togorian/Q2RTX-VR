// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_COMFORT_RELEASE308_H
#define B2_COMFORT_RELEASE308_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
typedef struct {
    bool valid,moving,fading;
    uint64_t frame;
    uint32_t now,released,owner[9];
    float strength,value;
} b2_comfort_release308_t;
static void B2Comfort_Reset308(b2_comfort_release308_t *s){memset(s,0,sizeof(*s));}
// Called outside the eye loop, with the existing logical frame and millisecond
// clock. Only ordinary locomotion release gets this tail; teleport is separate.
static float B2Comfort_Release308(b2_comfort_release308_t *s,uint64_t frame,
    uint32_t now,const uint32_t owner[9],bool valid,bool moving,float strength)
{
    if(!valid || !isfinite(strength) || strength<=0){B2Comfort_Reset308(s);return 0;}
    strength=fminf(1.f,strength);
    if(s->valid && (memcmp(s->owner,owner,sizeof(s->owner)) ||
       s->strength!=strength || (uint32_t)(now-s->now)>100 || frame<s->frame))
        B2Comfort_Reset308(s);
    if(s->valid && frame==s->frame)return s->value;
    if(moving){s->moving=true;s->fading=false;s->value=strength;}
    else {
        if(s->moving){s->moving=false;s->fading=true;s->released=now;}
        uint32_t age=now-s->released;
        if(s->fading && age<150){
            float t=(float)age/150.f;
            s->value=strength*(1.f-t*t*(3.f-2.f*t));
        }else {s->fading=false;s->value=0;}
    }
    s->valid=true;s->frame=frame;s->now=now;s->strength=strength;
    memcpy(s->owner,owner,sizeof(s->owner));return s->value;
}
#endif
