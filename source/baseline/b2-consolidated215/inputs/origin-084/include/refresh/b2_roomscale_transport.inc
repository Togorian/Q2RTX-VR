// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// B2 command-bound local transport. No cvar, packet, save or public ABI fields.
#include "b2_roomscale_bridge.h"
enum { B2RM_COUNT=128,B2RM_ORDINALS=32 };
typedef struct {bool valid,used;uint32_t number;b2w_command_t wire;} b2rm_binding_t;
static struct {
    uint32_t generation,active,pending,prediction_last,last_request_ms;
    uint64_t last_serial;
    b2rm_request_t publication,records[B2RM_COUNT];
    struct {uint32_t sequence;bool valid;b2rm_binding_t bindings[B2RM_ORDINALS];} packets[B2RM_COUNT];
    float base_raw[3],prediction_raw[3];
    bool active_valid,prediction_started;
} b2rm;
static bool b2rm_key(const b2w_command_t *a,const b2w_command_t *b,bool light)
{
    return a && b && a->angles[0]==b->angles[0] && a->angles[1]==b->angles[1] &&
        a->angles[2]==b->angles[2] && a->forward==b->forward && a->side==b->side &&
        a->up==b->up && a->buttons==b->buttons && a->msec==b->msec &&
        a->impulse==b->impulse && (!light || a->light==b->light);
}
void B2RM_Reset(void)
{
    uint32_t epoch=b2rm.generation+1;if(!epoch)epoch=1;
    memset(&b2rm,0,sizeof(b2rm));b2rm.generation=epoch;
}
uint32_t B2RM_LastRequestMs(void){return b2rm.last_request_ms;}
bool B2RM_Pending(uint32_t now)
{
    if(!b2rm.pending)return false;
    b2rm_request_t *r=&b2rm.records[b2rm.pending%B2RM_COUNT];
    if(r->number!=b2rm.pending || !r->valid || r->folded) {b2rm.pending=0;return false;}
    // A timed-out request remains pending until the client explicitly retires
    // the generation; it must not issue another cumulative displacement.
    (void)now;return true;
}
bool B2RM_Publish(uint64_t serial,uint32_t now,const float movement[3],const float basis[4])
{
    if(!serial || serial<=b2rm.last_serial || B2RM_Pending(now) ||
       !b2rs_finite3(movement) || movement[2] || b2rs_dist2(movement,(float[3]){0})>4096)return false;
    for(unsigned i=0;i<4;i++)if(!isfinite(basis[i]))return false;
    float det=basis[0]*basis[3]-basis[1]*basis[2];
    if(fabsf(det)<100.f || fabsf(det)>1000000.f)return false;
    b2rm_request_t *p=&b2rm.publication;memset(p,0,sizeof(*p));
    p->serial=serial;p->captured_ms=now;p->generation=b2rm.generation;p->valid=true;
    memcpy(p->movement,movement,sizeof(p->movement));memcpy(p->raw_basis,basis,sizeof(p->raw_basis));
    return true;
}
void B2RM_Finalise(uint32_t number,const b2w_command_t *c,uint32_t now)
{
    b2rm_request_t *p=&b2rm.publication,*r=&b2rm.records[number%B2RM_COUNT];
    if(!number || !c || !c->msec || !p->valid || p->generation!=b2rm.generation ||
       (uint32_t)(now-p->captured_ms)>100 || B2RM_Pending(now) || (r->valid && !r->folded))return;
    *r=*p;r->number=number;r->command=*c;
    b2rm.last_serial=p->serial;b2rm.pending=number;b2rm.last_request_ms=p->captured_ms;p->valid=false;
}
void B2RM_BeginPacket(uint32_t sequence)
{
    unsigned i=sequence%B2RM_COUNT;memset(&b2rm.packets[i],0,sizeof(b2rm.packets[i]));
    b2rm.packets[i].valid=true;b2rm.packets[i].sequence=sequence;
}
void B2RM_Bind(uint32_t seq,unsigned ordinal,uint32_t number,const b2w_command_t *c,uint32_t now)
{
    if(ordinal>=B2RM_ORDINALS)return;
    unsigned i=seq%B2RM_COUNT;
    if(!b2rm.packets[i].valid || b2rm.packets[i].sequence!=seq)return;
    b2rm_binding_t *b=&b2rm.packets[i].bindings[ordinal];
    b2rm_request_t *r=&b2rm.records[number%B2RM_COUNT];
    if(b->used)return;b->valid=false;
    if(!r->valid || r->number!=number || r->generation!=b2rm.generation ||
       (uint32_t)(now-r->captured_ms)>100 || !b2rm_key(&r->command,c,false))return;
    b->number=number;b->wire=*c;b->valid=true;
}
void B2RM_Enter(uint32_t seq,unsigned ordinal,const b2w_command_t *c,bool fresh,uint32_t now)
{
    b2rm.active_valid=false;
    if(!fresh || ordinal>=B2RM_ORDINALS)return;
    unsigned i=seq%B2RM_COUNT;
    if(!b2rm.packets[i].valid || b2rm.packets[i].sequence!=seq)return;
    b2rm_binding_t *b=&b2rm.packets[i].bindings[ordinal];
    bool used=b->used;b->used=true;
    if(used || !b->valid || !b2rm_key(&b->wire,c,true))return;
    b2rm_request_t *r=&b2rm.records[b->number%B2RM_COUNT];
    if(!r->valid || r->number!=b->number || r->generation!=b2rm.generation ||
       r->executed || (uint32_t)(now-r->captured_ms)>100)return;
    r->executed=true;b2rm.active=r->number;b2rm.active_valid=true;
}
void B2RM_Leave(void){b2rm.active_valid=false;}
bool B2RM_Current(b2rm_request_t *out)
{
    if(!out || !b2rm.active_valid)return false;
    b2rm_request_t *r=&b2rm.records[b2rm.active%B2RM_COUNT];
    if(!r->valid || r->number!=b2rm.active || r->acknowledged)return false;
    *out=*r;return true;
}
static bool b2rm_accepted(const b2rm_request_t *r,const float a[3])
{
    return b2rs_finite3(a) && a[0]*a[0]+a[1]*a[1]<=
        r->movement[0]*r->movement[0]+r->movement[1]*r->movement[1]+2.f &&
        fabsf(a[2])<=18.125f;
}
static void b2rm_raw(const b2rm_request_t *r,const float a[3],float raw[3])
{
    const float *b=r->raw_basis;float det=b[0]*b[3]-b[1]*b[2];
    raw[0]=(a[0]*b[3]-b[1]*a[1])/det;raw[1]=0;
    raw[2]=(b[0]*a[1]-a[0]*b[2])/det;
}
bool B2RM_Accept(uint32_t number,const float a[3])
{
    b2rm_request_t *r=&b2rm.records[number%B2RM_COUNT];
    if(!b2rm.active_valid || b2rm.active!=number || !r->valid || r->number!=number ||
       !r->executed || r->acknowledged || !b2rm_accepted(r,a))return false;
    memcpy(r->accepted,a,sizeof(r->accepted));r->acknowledged=true;return true;
}
void B2RM_PredictionBegin(uint32_t ack)
{
    for(unsigned i=0;i<B2RM_COUNT;i++){
        b2rm_request_t *r=&b2rm.records[i];
        if(r->valid && r->acknowledged && !r->folded && (int32_t)(ack-r->number)>=0){
            float raw[3];b2rm_raw(r,r->accepted,raw);
            for(unsigned j=0;j<3;j++)b2rm.base_raw[j]+=raw[j];
            r->folded=true;
        }
    }
    memcpy(b2rm.prediction_raw,b2rm.base_raw,sizeof(b2rm.base_raw));
    b2rm.prediction_last=ack;b2rm.prediction_started=true;
}
bool B2RM_PredictionRequest(uint32_t number,b2rm_request_t *out)
{
    const b2rm_request_t *r=&b2rm.records[number%B2RM_COUNT];
    if(!out || !b2rm.prediction_started || (int32_t)(number-b2rm.prediction_last)<=0 ||
       !r->valid || r->folded || r->number!=number || r->generation!=b2rm.generation)return false;
    *out=*r;return true;
}
bool B2RM_Predicted(uint32_t number,const float a[3])
{
    b2rm_request_t r;if(!B2RM_PredictionRequest(number,&r) || !b2rm_accepted(&r,a))return false;
    float raw[3];b2rm_raw(&r,a,raw);
    for(unsigned i=0;i<3;i++)b2rm.prediction_raw[i]+=raw[i];
    b2rm.prediction_last=number;return true;
}
void B2RM_Offset(float raw[3]){memcpy(raw,b2rm.prediction_raw,3*sizeof(float));}

void B2RM_AcknowledgedOffset(float raw[3])
{
    memcpy(raw,b2rm.base_raw,3*sizeof(float));
    for(unsigned i=0;i<B2RM_COUNT;i++){
        b2rm_request_t *r=&b2rm.records[i];
        if(r->valid && r->acknowledged && !r->folded){
            float part[3];b2rm_raw(r,r->accepted,part);
            for(unsigned j=0;j<3;j++)raw[j]+=part[j];
        }
    }
}