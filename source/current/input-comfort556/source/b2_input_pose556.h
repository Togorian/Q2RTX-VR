// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 12 November 2026: current per-hand pose admission; no cached pose substitution.
#ifndef B2_INPUT_POSE556_H
#define B2_INPUT_POSE556_H
#include <math.h>
static inline bool b2c_aim_usable556(const b2c_snapshot_t *p,unsigned hand,uint32_t now)
{
    if(hand>1 || !p->ready || (uint32_t)(now-p->capture_ms)>B2C_MAX_AGE_MS ||
       (uint32_t)(now-p->head_ms)>B2C_MAX_AGE_MS)return false;
    const b2c_hand_t *h=&p->hands[hand];
    if(!h->aim_active556 || !h->aim_valid || (!h->aim_tracked556 &&
       (uint32_t)(now-h->aim_tracked_ms556)>B2C_POSE_GRACE_MS556))return false;
    float norm=0;
    for(unsigned i=0;i<3;i++)if(!isfinite(h->aim_position[i]))return false;
    for(unsigned i=0;i<4;i++){if(!isfinite(h->aim_orientation[i]))return false;norm+=h->aim_orientation[i]*h->aim_orientation[i];}
    return isfinite(norm) && fabsf(norm-1.f)<=.02f;
}
#endif
