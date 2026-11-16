// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// B2 implementation of the user-supplied event/tuning specification.
// Private deterministic state; no gameplay, renderer or runtime dependency.
#ifndef B2_HAPTIC_STATE_H
#define B2_HAPTIC_STATE_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
enum { B2H_BLASTER, B2H_SHOTGUN, B2H_SSHOTGUN, B2H_MACHINEGUN,
    B2H_CHAINGUN, B2H_GRENADE_LAUNCHER, B2H_ROCKET, B2H_HYPERBLASTER,
    B2H_RAILGUN, B2H_PROFILE_COUNT };
enum { B2H_MODEL_OTHER, B2H_MODEL_GRENADE, B2H_MODEL_BFG };
typedef struct { uint32_t start, duration; float amplitude; bool fade; } b2h_envelope_t;
typedef struct {
    bool active, weapons, bfg, damage, armour, controller;
    unsigned hand;
} b2h_config_t;
typedef struct {
    bool valid;
    int number, server, player, health, armour, flashes, model, gunframe, selected;
    unsigned world;
} b2h_snapshot_t;
typedef struct {
    b2h_config_t config;
    b2h_snapshot_t previous;
    bool baseline, weapon_baseline, charge;
    b2h_envelope_t health, armour, ordinary[2], fire[2];
} b2h_state_t;
// Diagnostic total survives state cancellation; never affects an envelope.
static uint64_t b2h_created;
static void b2h_reset(b2h_state_t *s) { memset(s,0,sizeof(*s)); }
static void b2h_weapon_reset(b2h_state_t *s) {
    memset(s->ordinary,0,sizeof(s->ordinary)); memset(s->fire,0,sizeof(s->fire));
    s->charge=false; s->weapon_baseline=false;
}
static void b2h_configure(b2h_state_t *s, b2h_config_t c) {
    if (!c.active || c.hand>1) { b2h_reset(s); return; }
    if (!s->config.active || c.hand!=s->config.hand ||
        c.controller!=s->config.controller) b2h_weapon_reset(s);
    if (!c.controller || !c.weapons) {
        memset(s->ordinary,0,sizeof(s->ordinary));
    }
    if (!c.controller || !c.bfg) {
        memset(s->fire,0,sizeof(s->fire)); s->charge=false;
    }
    if (c.weapons!=s->config.weapons || c.bfg!=s->config.bfg) s->weapon_baseline=false;
    if (!c.damage) memset(&s->health,0,sizeof(s->health));
    if (!c.armour) memset(&s->armour,0,sizeof(s->armour));
    s->config=c;
}
static unsigned b2h_hand(int hand, float dominant) {
    if (hand==0) return 1;
    if (hand==1) return 0;
    return isfinite(dominant) && dominant<.5f ? 0u : 1u;
}
static void b2h_start(b2h_envelope_t *e, uint32_t now, uint32_t ms, float amplitude, bool fade) {
    if (!ms || ms>3000 || !isfinite(amplitude) || amplitude<=0 || amplitude>1) {
        memset(e,0,sizeof(*e)); return;
    }
    if (b2h_created!=UINT64_MAX) b2h_created++;
    *e=(b2h_envelope_t){now,ms,amplitude,fade};
}
static float b2h_value(b2h_envelope_t *e, uint32_t now) {
    uint32_t elapsed=now-e->start;
    if (!e->duration || elapsed>=e->duration || !isfinite(e->amplitude) ||
        e->amplitude<0 || e->amplitude>1) { memset(e,0,sizeof(*e)); return 0; }
    return e->amplitude*(e->fade ? 1.f-(float)elapsed/(float)e->duration : 1.f);
}
static bool b2h_profile(unsigned profile, uint32_t *ms, float *amplitude) {
    static const uint32_t durations[B2H_PROFILE_COUNT]={60,110,160,80,120,120,140,90,90};
    static const float amplitudes[B2H_PROFILE_COUNT]={.05f,.15f,.35f,.08f,.05f,.22f,.30f,.06f,.35f};
    if (profile>=B2H_PROFILE_COUNT) return false;
    *ms=durations[profile]; *amplitude=amplitudes[profile]; return true;
}
static void b2h_muzzle(b2h_state_t *s, unsigned profile, uint32_t now) {
    uint32_t ms; float amplitude;
    if (!s->config.active || !s->config.controller || !s->config.weapons ||
        !b2h_profile(profile,&ms,&amplitude)) return;
    b2h_start(&s->ordinary[s->config.hand],now,ms,amplitude,false);
}
static float b2h_capped(float base, int loss, float cap) {
    float value=base+(float)loss/200.f; return value<cap ? value : cap;
}
static void b2h_snapshot(b2h_state_t *s, const b2h_snapshot_t *p, uint32_t now) {
    if (!s->config.active) return;
    if (!p->valid || !p->world || p->number<0 || p->player<0 ||
        p->health<-32768 || p->health>32767 || p->armour<0 || p->armour>32767 ||
        p->selected<0 || p->model<0 || p->model>B2H_MODEL_BFG || p->gunframe<0 || p->gunframe>255) {
        b2h_config_t c=s->config; b2h_reset(s); s->config=c; return;
    }
    bool epoch=!s->baseline || p->server!=s->previous.server ||
        p->world!=s->previous.world || p->player!=s->previous.player;
    if (!epoch && p->number<=s->previous.number) {
        if (p->number<s->previous.number) {
            b2h_weapon_reset(s);
            memset(&s->health,0,sizeof(s->health)); memset(&s->armour,0,sizeof(s->armour));
        }
        return;
    }
    bool respawn=!epoch && s->previous.health<=0 && p->health>0;
    if (epoch || respawn) {
        b2h_config_t c=s->config; b2h_reset(s); s->config=c;
    } else {
        int loss=s->previous.health-p->health;
        // STAT_FLASHES blood bit distinguishes damage from passive overheal decay.
        // Lethal damage can return before damage_blood is accumulated: keep death feedback.
        if (s->config.damage && s->previous.health>0 && loss>0 &&
            (p->health<=0 || (p->flashes&1)))
            b2h_start(&s->health,now,p->health<=0?3000u:250u,
                b2h_capped(.20f,p->health<=0?s->previous.health:loss,.70f),p->health<=0);
        loss=s->previous.armour-p->armour;
        if (s->config.armour && s->previous.health>0 && loss>0 && (p->flashes&2))
            b2h_start(&s->armour,now,100,b2h_capped(.10f,loss,.60f),false);
    }
    bool same=s->weapon_baseline && p->model==s->previous.model && p->selected==s->previous.selected;
    if (!same || p->health<=0) b2h_weapon_reset(s);
    if (s->config.controller && p->health>0) {
        if (s->config.weapons && same && p->model==B2H_MODEL_GRENADE) {
            int old=s->previous.gunframe;
            if (old<6 && p->gunframe>=6 && p->gunframe<=11)
                b2h_start(&s->ordinary[s->config.hand],now,50,.06f,false);
            else if (old>=6 && old<=12 && p->gunframe>=13)
                b2h_start(&s->ordinary[s->config.hand],now,80,.18f,false);
        }
        if (s->config.bfg && p->model==B2H_MODEL_BFG) {
            s->charge=p->gunframe>=9 && p->gunframe<=16;
            if (same && s->previous.gunframe>=9 && s->previous.gunframe<=16 && p->gunframe>=17)
                b2h_start(&s->fire[s->config.hand],now,180,1.f,false);
        } else s->charge=false;
        s->weapon_baseline=true;
    }
    s->previous=*p; s->baseline=true;
}
static void b2h_compose(b2h_state_t *s, uint32_t now, float out[2]) {
    out[0]=out[1]=0;
    if (!s->config.active) return;
    float health=b2h_value(&s->health,now),armour=b2h_value(&s->armour,now);
    for (unsigned hand=0;hand<2;hand++) {
        float amplitude=health>armour?health:armour;
        float ordinary=b2h_value(&s->ordinary[hand],now),fire=b2h_value(&s->fire[hand],now);
        if (ordinary>amplitude) amplitude=ordinary;
        if (fire>amplitude) amplitude=fire;
        if (s->charge && s->config.controller && s->config.bfg && hand==s->config.hand && amplitude<.12f)
            amplitude=.12f;
        out[hand]=isfinite(amplitude) && amplitude>0 && amplitude<=1 ? amplitude : 0;
    }
}
#endif
