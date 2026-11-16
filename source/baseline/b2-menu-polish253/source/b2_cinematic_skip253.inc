// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Film-only input state. A new owner/film must observe release before holding.
typedef struct {
    bool configured, armed, holding;
    unsigned hand;
    uint32_t generation, xr_generation, interruption, started, last;
    uint64_t serial;
    float progress;
} b2cs_state_t;

static void b2cs_reset(b2cs_state_t *s) { memset(s,0,sizeof(*s)); }

static bool b2cs_step(b2cs_state_t *s, const b2c_snapshot_t *p,
    uint32_t now, bool eligible, unsigned hand, uint32_t interruption)
{
    if (!eligible || hand>1 || !b2c_sample_valid(p,now)) {
        b2cs_reset(s);return false;
    }
    if (!s->configured || s->hand!=hand || s->generation!=p->generation ||
        s->xr_generation!=p->xr_generation || s->interruption!=interruption ||
        p->serial<s->serial || (uint32_t)(now-s->last)>B2C_MAX_AGE_MS) {
        b2cs_reset(s);s->configured=true;s->hand=hand;
        s->generation=p->generation;s->xr_generation=p->xr_generation;
        s->interruption=interruption;
    }
    if (p->serial==s->serial) return false;
    s->serial=p->serial;s->last=now;
    if (p->hands[hand].trigger<.65f) {
        s->holding=false;s->progress=0;
        if (b2c_neutral(p)) s->armed=true;
        return false;
    }
    if (!s->armed) return false;
    if (!s->holding) { s->holding=true;s->started=now; }
    uint32_t elapsed=now-s->started;
    if (elapsed>=1500) { b2cs_reset(s);return true; }
    s->progress=(float)elapsed/1500.f;
    return false;
}
