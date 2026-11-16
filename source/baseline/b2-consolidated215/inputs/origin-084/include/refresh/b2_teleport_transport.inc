// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Adapted from the accepted B2RM local command transport, separate ownership.
#include "b2_teleport_bridge.h"
enum { B2TP_RECORDS=128,B2TP_ORDINALS=32 };
static struct {
    bool enabled,active_valid;
    uint32_t generation,active,pending;
    b2w_epochs_t epochs;
    uint64_t last_serial;
    b2tp_request_t publication,records[B2TP_RECORDS];
    struct {bool valid;uint32_t sequence;
        struct {bool valid,used;uint32_t number;b2w_command_t wire;} binding[B2TP_ORDINALS];
    } packets[B2TP_RECORDS];
} b2tp_transport;
// Accepted movement survives an input cancellation until a server snapshot includes it.
static b2tp_request_t b2tp_completed[B2TP_RECORDS];
void B2TP_WorldReset(void){B2TP_Reset();memset(b2tp_completed,0,sizeof(b2tp_completed));}
void B2TP_PredictionBegin(uint32_t ack)
{
    for(unsigned i=0;i<B2TP_RECORDS;i++)if(b2tp_completed[i].valid &&
        (int32_t)(ack-b2tp_completed[i].number)>=0)b2tp_completed[i].valid=false;
}
void B2TP_Reset(void)
{
    uint32_t next=b2tp_transport.generation+1;if(!next)next=1;
    memset(&b2tp_transport,0,sizeof(b2tp_transport));b2tp_transport.generation=next;
}
void B2TP_SetOwner(bool enabled,const b2w_epochs_t *e)
{
    if(!enabled || !e){if(b2tp_transport.enabled)B2TP_Reset();return;}
    if(!b2tp_transport.enabled || memcmp(e,&b2tp_transport.epochs,sizeof(*e)))B2TP_Reset();
    b2tp_transport.enabled=true;b2tp_transport.epochs=*e;
}
bool B2TP_Enabled(void){return b2tp_transport.enabled;}
bool B2TP_Expired(uint32_t now)
{
    if(b2tp_transport.pending){
        const b2tp_request_t *r=&b2tp_transport.records[b2tp_transport.pending%B2TP_RECORDS];
        return (uint32_t)(now-r->aim.pose.capture_ms)>100;
    }
    return b2tp_transport.publication.valid &&
        (uint32_t)(now-b2tp_transport.publication.aim.pose.capture_ms)>100;
}
bool B2TP_Publish(const b2tp_request_t *p,uint32_t now)
{
    if(!p || !B2TP_Enabled() || b2tp_transport.pending ||
       !B2W_PoseCurrent(&p->aim.pose,&b2tp_transport.epochs,now) ||
       p->aim.pose.serial<=b2tp_transport.last_serial || !b2tp_finite3(p->target) ||
       !b2tp_finite3(p->head) || !isfinite(p->range))return false;
    b2tp_transport.publication=*p;b2tp_transport.publication.generation=b2tp_transport.generation;
    b2tp_transport.publication.valid=true;b2tp_transport.last_serial=p->aim.pose.serial;
    return true;
}
void B2TP_Finalise(uint32_t number,const b2w_command_t *c,uint32_t now)
{
    b2tp_request_t *p=&b2tp_transport.publication;
    if(!B2TP_Enabled() || !p->valid || !number || !c || !c->msec || b2tp_transport.pending ||
       !B2W_PoseCurrent(&p->aim.pose,&b2tp_transport.epochs,now))return;
    b2tp_request_t *r=&b2tp_transport.records[number%B2TP_RECORDS];
    *r=*p;r->number=number;r->command=*c;p->valid=false;b2tp_transport.pending=number;
}
void B2TP_BeginPacket(uint32_t sequence)
{
    unsigned i=sequence%B2TP_RECORDS;
    memset(&b2tp_transport.packets[i],0,sizeof(b2tp_transport.packets[i]));
    b2tp_transport.packets[i].valid=true;b2tp_transport.packets[i].sequence=sequence;
}
void B2TP_Bind(uint32_t seq,unsigned ordinal,uint32_t number,const b2w_command_t *c,uint32_t now)
{
    if(!B2TP_Enabled() || ordinal>=B2TP_ORDINALS)return;
    unsigned i=seq%B2TP_RECORDS;
    if(!b2tp_transport.packets[i].valid || b2tp_transport.packets[i].sequence!=seq ||
       b2tp_transport.packets[i].binding[ordinal].used)return;
    b2tp_request_t *r=&b2tp_transport.records[number%B2TP_RECORDS];
    if(!r->valid || r->number!=number || r->generation!=b2tp_transport.generation ||
       r->executed || !B2W_PoseCurrent(&r->aim.pose,&b2tp_transport.epochs,now) ||
       !b2rm_key(&r->command,c,false))return;
    b2tp_transport.packets[i].binding[ordinal].valid=true;
    b2tp_transport.packets[i].binding[ordinal].number=number;
    b2tp_transport.packets[i].binding[ordinal].wire=*c;
}
void B2TP_Enter(uint32_t seq,unsigned ordinal,const b2w_command_t *c,bool fresh,uint32_t now)
{
    b2tp_transport.active_valid=false;
    if(!B2TP_Enabled() || !fresh || ordinal>=B2TP_ORDINALS)return;
    unsigned i=seq%B2TP_RECORDS;
    if(!b2tp_transport.packets[i].valid || b2tp_transport.packets[i].sequence!=seq)return;
    bool used=b2tp_transport.packets[i].binding[ordinal].used;
    b2tp_transport.packets[i].binding[ordinal].used=true;
    if(used || !b2tp_transport.packets[i].binding[ordinal].valid ||
       !b2rm_key(&b2tp_transport.packets[i].binding[ordinal].wire,c,true))return;
    uint32_t number=b2tp_transport.packets[i].binding[ordinal].number;
    b2tp_request_t *r=&b2tp_transport.records[number%B2TP_RECORDS];
    if(!r->valid || r->number!=number || r->executed || r->generation!=b2tp_transport.generation ||
       !B2W_PoseCurrent(&r->aim.pose,&b2tp_transport.epochs,now))return;
    r->executed=true;b2tp_transport.active=number;b2tp_transport.active_valid=true;
}
bool B2TP_Current(b2tp_request_t *out)
{
    if(!out || !B2TP_Enabled() || !b2tp_transport.active_valid)return false;
    b2tp_request_t *r=&b2tp_transport.records[b2tp_transport.active%B2TP_RECORDS];
    if(!r->valid || !r->executed || r->acknowledged || r->number!=b2tp_transport.active)return false;
    *out=*r;return true;
}
bool B2TP_Result(uint32_t number,bool accepted,const float origin[3],uint8_t flags,uint8_t time)
{
    b2tp_request_t r;
    if(!B2TP_Current(&r) || r.number!=number || (accepted && !b2tp_finite3(origin)))return false;
    b2tp_request_t *p=&b2tp_transport.records[number%B2TP_RECORDS];
    p->acknowledged=true;p->accepted=accepted;
    if(accepted){memcpy(p->accepted_origin,origin,sizeof(p->accepted_origin));p->accepted_flags=flags;p->accepted_time=time;}
    b2tp_transport.pending=0;
    if(accepted)b2tp_completed[number%B2TP_RECORDS]=*p;
    B2_TeleportResult(p);return true;
}
void B2TP_Leave(void)
{
    b2tp_request_t r;
    if(B2TP_Current(&r))B2TP_Result(r.number,false,NULL,0,0);
    b2tp_transport.active_valid=false;
}
bool B2TP_Prediction(uint32_t number,b2tp_request_t *out)
{
    if(!out)return false;
    b2tp_request_t *r=&b2tp_completed[number%B2TP_RECORDS];
    if(!r->valid || r->number!=number || !r->acknowledged || !r->accepted)return false;
    *out=*r;return true;
}
