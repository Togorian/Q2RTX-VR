// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Included by the canonical math owner. All entries are immutable copies.
#include "b2_weapon_presentation.h"
#include "b2_weapon_observation.h"
#include "b2_weapon_display.h"
#include "b2_weapon_visual.h"
enum { B2W_COMMANDS=128, B2W_PACKETS=128, B2W_ORDINALS=32, B2WE_EVENTS=32 };
typedef struct {
    b2w_command_t command;
    b2w_packet_t packet;
    uint32_t number;
    b2wp_source_t presentation;
    bool valid, presentation_valid;
} b2w_final_t;
typedef struct {
    b2w_final_t commands[B2W_ORDINALS];
    bool used[B2W_ORDINALS];
    uint32_t sequence;
    bool valid;
} b2w_bound_t;
static struct {
    bool requested, attack, published, in_command, fresh_command, current_valid, last_valid;
    b2w_epochs_t epochs;
    b2w_packet_t publication, current, last;
    b2wp_source_t presentation;
    b2we_event_t events[B2WE_EVENTS];
    uint64_t event_count;
    b2wp_record_t last_presentation;
    bool presentation_valid, last_presentation_valid;
    b2w_packet_t acknowledgement;
    bool acknowledged, selection_accepted;
    uint32_t acknowledged_ms;
    b2w_final_t final[B2W_COMMANDS];
    b2w_bound_t bound[B2W_PACKETS];
} b2w_transport;

void B2W_Reset(void)
{
    B2WP_RetireView();
    bool requested=b2w_transport.requested;
    memset(&b2w_transport,0,sizeof(b2w_transport));
    b2w_transport.requested=requested;
}
void B2W_SetContext(bool requested, const b2w_epochs_t *epochs, bool attack, uint32_t now)
{
    (void)now;
    if (requested!=b2w_transport.requested || !epochs ||
        !B2W_EpochsEqual(epochs,&b2w_transport.epochs)) B2W_Reset();
    b2w_transport.requested=requested;
    b2w_transport.attack=requested && attack;
    if (epochs) b2w_transport.epochs=*epochs;
    if (!requested) B2W_Reset();
}
bool B2W_Requested(void) { return b2w_transport.requested; }
unsigned B2W_Context(void)
{
    return (b2w_transport.requested ? B2W_CONTEXT_REQUESTED : 0) |
        (b2w_transport.in_command ? B2W_CONTEXT_COMMAND : 0) |
        (b2w_transport.in_command && b2w_transport.fresh_command ? B2W_CONTEXT_NEW_COMMAND : 0);
}
static bool b2w_packet_current(const b2w_packet_t *p, uint32_t now)
{
    return b2w_transport.requested && B2W_PoseCurrent(&p->pose,&b2w_transport.epochs,now) &&
        p->game_epoch && p->weapon>=0 && p->weapon<12 &&
        (!p->selection || (p->slot>=0 && p->slot<12 && (uint32_t)(now-p->selection_ms)<=100));
}
bool B2W_Publish(const b2w_packet_t *packet, uint32_t now)
{
    if (!packet || !b2w_packet_current(packet,now) ||
        (b2w_transport.published && (packet->pose.serial<=b2w_transport.publication.pose.serial ||
            packet->pose.pose_time<b2w_transport.publication.pose.pose_time))) {
        B2W_Reset(); return false;
    }
    b2w_transport.publication=*packet; b2w_transport.published=true;
    b2w_transport.presentation_valid=false;
    return true;
}
bool B2W_Ready(uint32_t now)
{
    return b2w_transport.published && b2w_packet_current(&b2w_transport.publication,now);
}
void B2W_Acknowledge(const b2w_packet_t *packet, bool accepted, uint32_t now)
{
    // Only the actual current new command can acknowledge its immutable token.
    // A later duplicate refusal cannot replace the first authoritative result.
    if (!packet || !packet->selection || !b2w_transport.in_command || !b2w_transport.fresh_command ||
        !b2w_transport.current_valid || packet->selection!=b2w_transport.current.selection ||
        packet->pose.serial!=b2w_transport.current.pose.serial ||
        packet->slot!=b2w_transport.current.slot || packet->attack ||
        packet->game_epoch!=b2w_transport.current.game_epoch || packet->weapon!=b2w_transport.current.weapon ||
        packet->selection_ms!=b2w_transport.current.selection_ms ||
        memcmp(&packet->pose,&b2w_transport.current.pose,sizeof(packet->pose)) || !b2w_packet_current(packet,now) ||
        (b2w_transport.acknowledged && packet->selection<=b2w_transport.acknowledgement.selection)) return;
    b2w_transport.acknowledgement=*packet;
    b2w_transport.selection_accepted=accepted;b2w_transport.acknowledged=true;
    b2w_transport.acknowledged_ms=now;
}
// An accepted result is an event, not another pose sample. Keep it briefly
// after command expiry; context retirement still discards it immediately.
bool B2W_SelectionAck(uint64_t token, int *slot, bool *accepted, uint32_t now)
{
    if (!token || !slot || !accepted || !b2w_transport.acknowledged ||
        b2w_transport.acknowledgement.selection!=token ||
        !b2w_transport.requested ||
        !B2W_EpochsEqual(&b2w_transport.acknowledgement.pose.epochs,&b2w_transport.epochs) ||
        (uint32_t)(now-b2w_transport.acknowledged_ms)>1000) return false;
    *slot=b2w_transport.acknowledgement.slot;*accepted=b2w_transport.selection_accepted;return true;
}
static bool b2w_command_valid(const b2w_command_t *c)
{
    // The admitted controller command is represented exactly by old, R1Q2
    // and enhanced encodings. Refuse unsupported inputs, never guess truncation.
    return c && c->forward>=-400 && c->forward<=400 && c->side>=-400 && c->side<=400 &&
        c->up>=-400 && c->up<=400 && !(c->buttons&~131u) && !c->impulse;
}
static bool b2w_command_equal(const b2w_command_t *a, const b2w_command_t *b, bool light)
{
    return a->angles[0]==b->angles[0] && a->angles[1]==b->angles[1] && a->angles[2]==b->angles[2] &&
        a->forward==b->forward && a->side==b->side && a->up==b->up && a->buttons==b->buttons &&
        a->msec==b->msec && a->impulse==b->impulse && (!light || a->light==b->light);
}
void B2W_Finalise(uint32_t number, const b2w_command_t *command, uint32_t now)
{
    b2w_final_t *entry=&b2w_transport.final[number%B2W_COMMANDS];
    memset(entry,0,sizeof(*entry)); entry->number=number;
    if (!b2w_command_valid(command) || !B2W_Ready(now)) return;
    entry->command=*command; entry->packet=b2w_transport.publication;
    entry->presentation=b2w_transport.presentation;
    entry->presentation_valid=b2w_transport.presentation_valid;
    entry->packet.attack=entry->packet.attack && (command->buttons&1)!=0;
    entry->valid=true;
}
void B2W_BeginPacket(uint32_t sequence)
{
    b2w_bound_t *p=&b2w_transport.bound[sequence%B2W_PACKETS];
    memset(p,0,sizeof(*p)); p->sequence=sequence; p->valid=true;
}
void B2W_Bind(uint32_t sequence, unsigned ordinal, uint32_t number,
    const b2w_command_t *wire, uint32_t now)
{
    b2w_bound_t *p=&b2w_transport.bound[sequence%B2W_PACKETS];
    const b2w_final_t *f=&b2w_transport.final[number%B2W_COMMANDS];
    if (!p->valid || p->sequence!=sequence || ordinal>=B2W_ORDINALS) return;
    p->commands[ordinal].valid=false;
    if (p->used[ordinal] || !f->valid || f->number!=number || !b2w_command_valid(wire) ||
        !b2w_command_equal(&f->command,wire,false) || !b2w_packet_current(&f->packet,now)) return;
    p->commands[ordinal]=*f; p->commands[ordinal].command=*wire;
}
bool B2W_Enter(uint32_t sequence, unsigned ordinal, const b2w_command_t *command,
    bool fresh, uint32_t now)
{
    b2w_transport.in_command=true; b2w_transport.current_valid=false;
    b2w_transport.fresh_command=fresh;
    if (!b2w_transport.requested) return false;
    if (!fresh) return true; // Movement recovery remains; no tracked attack/select.
    b2w_transport.last_valid=false;
    b2w_transport.last_presentation_valid=false;
    b2w_transport.event_count=0;
    b2w_bound_t *p=&b2w_transport.bound[sequence%B2W_PACKETS];
    if (!p->valid || p->sequence!=sequence || ordinal>=B2W_ORDINALS) return true;
    b2w_final_t *f=&p->commands[ordinal];
    bool used=p->used[ordinal]; p->used[ordinal]=true;
    if (used || !f->valid || !b2w_command_valid(command) ||
        !b2w_command_equal(&f->command,command,true) || !b2w_packet_current(&f->packet,now)) return true;
    b2w_transport.current=b2w_transport.last=f->packet;
    b2w_transport.current_valid=b2w_transport.last_valid=true;
    if (f->presentation_valid) {
        b2wp_record_t *r=&b2w_transport.last_presentation;
        memset(r,0,sizeof(*r));r->size=sizeof(*r);r->version=B2WP_VERSION;
        r->source=f->presentation;r->packet=f->packet;r->command=f->command;
        r->number=f->number;r->sequence=sequence;r->ordinal=ordinal;
        b2w_transport.last_presentation_valid=true;
    }
    return !f->packet.attack || !b2w_transport.attack;
}
void B2W_Leave(void) { b2w_transport.in_command=false; b2w_transport.fresh_command=false; b2w_transport.current_valid=false; }
bool B2W_CurrentAt(b2w_packet_t *out, uint32_t now)
{
    if (!out) return false;
    memset(out,0,sizeof(*out));
    const b2w_packet_t *p=b2w_transport.in_command ? &b2w_transport.current : &b2w_transport.last;
    bool valid=b2w_transport.in_command ? b2w_transport.current_valid : b2w_transport.last_valid;
    if (!valid || !b2w_packet_current(p,now)) return false;
    *out=*p; out->attack=out->attack && b2w_transport.attack;
    return true;
}


bool B2WE_OwnerAt(b2we_owner_t *out, uint32_t now)
{
    b2w_packet_t packet;
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (!b2w_transport.last_presentation_valid || !B2W_CurrentAt(&packet,now)) return false;
    const b2wp_record_t *r=&b2w_transport.last_presentation;
    if (memcmp(&packet.pose,&r->packet.pose,sizeof(packet.pose)) ||
        packet.game_epoch!=r->packet.game_epoch || packet.weapon!=r->packet.weapon ||
        (uint32_t)(now-r->source.preview_ms)>100) return false;
    out->size=sizeof(*out);out->version=B2WE_VERSION;
    out->packet=r->packet;out->command=r->command;
    out->number=r->number;out->sequence=r->sequence;out->ordinal=r->ordinal;
    out->handle=r->source.handle;out->model=r->source.model;out->aim=r->source.aim;
    return true;
}
bool B2WE_RecordAt(const b2we_event_t *event, uint32_t now)
{
    b2we_owner_t current;
    if (!event || event->size!=sizeof(*event) || event->version!=B2WE_VERSION ||
        !event->serial || event->frame<0 || event->gunframe<0 ||
        (uint32_t)(now-event->completed_ms)>100 || !B2WE_OwnerAt(&current,now) ||
        memcmp(&current,&event->owner,sizeof(current)) ||
        !finite_values(event->corrected,3) || !finite_values(event->direction,3) ||
        !finite_values(event->body_angles,3) || !finite_values(event->anchor,3)) return false;
    float n=0;for(unsigned i=0;i<3;i++)n+=event->direction[i]*event->direction[i];
    if (fabsf(n-1)>.02f) return false;
    if (b2w_transport.event_count) {
        const b2we_event_t *last=&b2w_transport.events[(b2w_transport.event_count-1)%B2WE_EVENTS];
        if (event->serial<=last->serial || event->frame<last->frame) return false;
    }
    // Bounded event history belongs only to this consumed command. Retiring
    // that owner clears the history; no later publication refreshes it.
    b2w_transport.events[b2w_transport.event_count%B2WE_EVENTS]=*event;
    b2w_transport.event_count++;
    return true;
}
bool B2WE_NextAt(uint64_t after, b2we_event_t *out, uint32_t now)
{
    b2we_owner_t current;
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (!B2WE_OwnerAt(&current,now)) return false;
    uint64_t end=b2w_transport.event_count,begin=end>B2WE_EVENTS?end-B2WE_EVENTS:0;
    for(uint64_t i=begin;i<end;i++) {
        const b2we_event_t *e=&b2w_transport.events[i%B2WE_EVENTS];
        if (e->serial>after && (uint32_t)(now-e->completed_ms)<=100 &&
            !memcmp(&current,&e->owner,sizeof(current))) { *out=*e;return true; }
    }
    return false;
}

// Engine presentation state is staged once with the entity snapshot. Render
// roles reuse it without polling controllers, changing time or tracing again.
typedef struct {
    bool requested, valid;
    uint32_t handle;
    const void *model;
    int frame, oldframe;
    float backlerp, matrix[16], base_matrix[16], rebase[16];
    b2wp_record_t record;
    bool display_mode, display_used, display_decided;
    int64_t display_time;
    uint32_t display_capture_ms;
} b2wp_view_t;
static b2wp_view_t b2wp_view, b2wp_committed;
static bool b2wp_committed_valid;
static bool b2wp_observing;
static unsigned b2wp_observed_reads;
void B2WP_ObserveWindow(bool enabled)
{
    if (!enabled && !b2wp_observing) return;
    b2wp_observing=enabled;b2wp_observed_reads=0;
}
void B2WP_ObserveSnapshot(b2wp_observation_t *out)
{
    // Copy only; do not call Presentation/FrameReady or renew any pose/guard.
    const b2wp_record_t *r=&b2wp_view.record;
    memset(out,0,sizeof(*out));
    out->requested=b2wp_view.requested;out->valid=b2wp_view.valid;
    out->record_present=r->size==sizeof(*r) && r->version==B2WP_VERSION;
    out->matrix_reads=b2wp_observed_reads;out->slot=r->source.model.slot;
    out->number=r->number;out->sequence=r->sequence;out->ordinal=r->ordinal;
    out->serial=r->source.aim.serial;out->pose_time=r->source.aim.pose_time;
    out->capture_ms=r->source.aim.capture_ms;out->head_ms=r->source.aim.head_ms;
    out->frame=b2wp_view.frame;out->oldframe=b2wp_view.oldframe;
    out->backlerp=b2wp_view.backlerp;out->emission=r->emission_pose;out->idle=r->idle_pose;
    out->display_mode=b2wp_view.display_mode;out->display_used=b2wp_view.display_used;
    out->display_time=b2wp_view.display_used ? b2wp_view.display_time : r->source.aim.pose_time;
    out->display_capture_ms=b2wp_view.display_used ? b2wp_view.display_capture_ms : r->source.aim.capture_ms;
    memcpy(out->matrix,b2wp_view.matrix,sizeof(out->matrix));
    memcpy(out->recoil,r->recoil,sizeof(out->recoil));
}
static bool b2wp_key_equal(const b2wm_key_t *a, const b2wm_key_t *b)
{
    return a->slot==b->slot && a->calibration==b->calibration &&
        a->generation==b->generation && !memcmp(a->sha256,b->sha256,sizeof(a->sha256));
}
bool B2WP_TryPreview(b2w_packet_t *packet, b2wp_source_t *source,
    uint32_t handle, const void *model, const b2w_game_api_t *game, uint32_t now, bool *blocked)
{
    if (blocked) *blocked=false;
    b2wm_info_t info;
    if (!packet || !source || !handle || !game || game->version!=B2W_API ||
        game->size!=sizeof(*game) || !(game->capabilities&B2W_CAP_PREVIEW) || !game->preview ||
        !b2w_packet_current(packet,now) ||
        !B2WM_Query(model,B2WM_VERSION,sizeof(info),&info) ||
        info.key.slot!=(unsigned)packet->weapon || (info.requires_body_partition && !game->grenade)) return false;
    b2wp_source_t value={0};
    value.size=sizeof(value);value.version=B2WP_VERSION;value.handle=handle;
    value.model=info.key;value.aim=packet->pose;value.preview_ms=now;
    if (!B2WM_Desired(model,&info.key,&value.aim,value.desired)) return false;
    b2w_packet_t candidate=*packet;
    memcpy(candidate.pose.origin,value.desired,sizeof(value.desired));
    if (!game->preview(&candidate,value.preview)) {
        // The independently valid model/pose could not obtain a safe game muzzle.
        // No packet is published or changed; only this classified result may
        // retain otherwise guarded locomotion. The renderer still has no weapon.
        if (blocked) { *blocked=true;*source=value; }
        return false;
    }
    if (!finite_values(value.preview,3)) return false;
    *packet=candidate;*source=value;return true;
}
bool B2WP_Preview(b2w_packet_t *packet, b2wp_source_t *source,
    uint32_t handle, const void *model, const b2w_game_api_t *game, uint32_t now)
{
    return B2WP_TryPreview(packet,source,handle,model,game,now,NULL);
}
bool B2WP_Publish(const b2w_packet_t *packet, const b2wp_source_t *source, uint32_t now)
{
    if (!packet || !source || source->size!=sizeof(*source) || source->version!=B2WP_VERSION ||
        !source->handle || !source->model.generation || source->model.calibration!=B2WM_CALIBRATION ||
        source->model.slot!=(unsigned)packet->weapon ||
        !B2W_PoseCurrent(&source->aim,&packet->pose.epochs,now) ||
        (uint32_t)(now-source->preview_ms)>100 ||
        !finite_values(source->desired,3) || !finite_values(source->preview,3) ||
        !finite_values(source->body_origin,3) || !finite_values(source->body_angles,3)) return false;
    b2w_pose_t expected=source->aim;
    memcpy(expected.origin,source->desired,sizeof(source->desired));
    if (memcmp(&expected,&packet->pose,sizeof(expected)) || !B2W_Publish(packet,now)) return false;
    b2w_transport.presentation=*source;b2w_transport.presentation_valid=true;return true;
}
bool B2WP_CurrentAt(b2wp_record_t *out, uint32_t now)
{
    b2w_packet_t packet;
    if (!out) return false;
    memset(out,0,sizeof(*out));
    // Presentation belongs to an actually consumed new command. Publication
    // alone, a movement backup or a newer controller sample cannot replace it.
    if (!b2w_transport.last_presentation_valid || !B2W_CurrentAt(&packet,now)) return false;
    const b2wp_record_t *r=&b2w_transport.last_presentation;
    if (memcmp(&packet.pose,&r->packet.pose,sizeof(packet.pose)) ||
        packet.game_epoch!=r->packet.game_epoch || packet.weapon!=r->packet.weapon ||
        packet.selection!=r->packet.selection || (uint32_t)(now-r->source.preview_ms)>100) return false;
    *out=*r;
    if (b2w_transport.event_count) {
        const b2we_event_t *e=&b2w_transport.events[(b2w_transport.event_count-1)%B2WE_EVENTS];
        b2we_owner_t owner;
        if (!B2WE_OwnerAt(&owner,now) || memcmp(&owner,&e->owner,sizeof(owner)) ||
            (uint32_t)(now-e->completed_ms)>100) return false;
        memcpy(out->source.preview,e->anchor,sizeof(e->anchor));
        out->emission_serial=e->serial;out->emission_frame=e->frame;out->emission_gunframe=e->gunframe;
    }
    return true;
}
void B2WP_BeginView(bool requested)
{
    memset(&b2wp_view,0,sizeof(b2wp_view));b2wp_view.requested=requested;
}
void B2WP_RetireView(void) { b2wp_view.valid=false; }
// Stock Weapon_Generic activation endpoints, in the calibrated slot order.
// The hand grenade activates directly at ready16 and has no draw sequence.
static bool b2wp_draw_matrix(const void *model, const b2wm_info_t *info,
    int frame, int oldframe, float backlerp, const b2wp_record_t *record, float matrix[16])
{
    static const int last[12]={4,7,6,3,4,-1,5,4,5,3,8,8};
    if (info->key.slot>=12 || frame<0 || oldframe<0 ||
        !isfinite(backlerp) || backlerp<0 || backlerp>1) return false;
    int end=last[info->key.slot];
    if (end>=0 && (frame<=end || frame==info->ready) &&
        (oldframe<=end || oldframe==info->ready) && (frame<=end || oldframe<=end)) {
        // Keep the ready calibration fixed while the genuine stock vertices
        // draw relative to the controller. The blend to ready converges to the
        // accepted held transform without another clock or firing-pose change.
        frame=oldframe=info->ready;backlerp=0;
    }
    return B2WM_Matrix(model,&info->key,frame,oldframe,backlerp,&record->source.aim,
        record->source.preview,matrix);
}
bool B2WP_Stage(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, const b2wp_record_t *record, uint32_t now)
{
    b2wp_record_t current;
    b2wm_info_t info;
    b2wp_view.valid=false;
    if (!b2wp_view.requested || !record || record->size!=sizeof(*record) ||
        record->version!=B2WP_VERSION ||
        !B2W_Presentation(&current) ||
        memcmp(record,&current,sizeof(current)) || handle!=record->source.handle ||
        !B2WM_Query(model,B2WM_VERSION,sizeof(info),&info) ||
        !b2wp_key_equal(&info.key,&record->source.model)) return false;
    b2wp_record_t posed=*record;
    if (!B2WM_Recoil(&record->source.aim,record->recoil,&posed.source.aim)) return false;
    if (info.requires_body_partition) {
        b2wm_part_t parts[3];
        unsigned state=record->grenade.state;
        if (state<B2WG_READY || state>B2WG_DETONATED ||
            !B2WM_GrenadeParts(model,&info.key,frame,oldframe,backlerp,&posed.source.aim,
                record->source.preview,state==B2WG_READY||state==B2WG_HELD,parts)) return false;
        memcpy(b2wp_view.matrix,parts[0].matrix,sizeof(b2wp_view.matrix));
    } else if (!b2wp_draw_matrix(model,&info,frame,oldframe,backlerp,
        &posed,b2wp_view.matrix)) return false;
    b2wp_view.handle=handle;b2wp_view.model=model;b2wp_view.frame=frame;
    b2wp_view.oldframe=oldframe;b2wp_view.backlerp=backlerp;b2wp_view.record=*record;
    memcpy(b2wp_view.base_matrix,b2wp_view.matrix,sizeof(b2wp_view.matrix));
    memset(b2wp_view.rebase,0,sizeof(b2wp_view.rebase));
    for (unsigned i=0;i<4;i++) b2wp_view.rebase[i*5]=1;
    b2wp_view.valid=true;return true;
}

// Re-express a consumed pose in the current admitted body frame. This never
// modifies the immutable source, command, packet, emission event or animation.
static void b2wp_transform_matrix(const float transform[16], const float source[16], float out[16])
{
    float result[16]={0};
    for (unsigned col=0;col<4;col++) for (unsigned row=0;row<4;row++)
        for (unsigned k=0;k<4;k++) result[col*4+row]+=transform[k*4+row]*source[col*4+k];
    memcpy(out,result,sizeof(result));
}
void B2WP_DisplayBegin(bool requested)
{
    // The natural scene/crosshair retains the immutable command base matrix.
    // Reset even for a held scene so an earlier B matrix cannot leak into A.
    if (b2wp_view.display_used && b2wp_view.valid)
        b2wp_transform_matrix(b2wp_view.rebase,b2wp_view.base_matrix,b2wp_view.matrix);
    b2wp_view.display_mode=requested;
    b2wp_view.display_used=b2wp_view.display_decided=false;
    b2wp_view.display_time=0;b2wp_view.display_capture_ms=0;
}
bool B2WP_DisplayApply(const b2wp_record_t *owner, const b2w_pose_t *aim,
    const float preview[3], int64_t eye_time, uint32_t now)
{
    if (!b2wp_view.display_mode || b2wp_view.display_decided) return false;
    b2wp_view.display_decided=true;
    b2wm_info_t info;
    if (!owner || !aim || !preview || !B2WV_Held(owner) ||
        !B2WP_FrameReady(now) || !b2wp_view.valid ||
        memcmp(owner,&b2wp_view.record,sizeof(*owner)) ||
        eye_time<=0 || aim->pose_time!=eye_time ||
        aim->hand!=owner->source.aim.hand ||
        !B2W_PoseCurrent(aim,&owner->source.aim.epochs,now) ||
        !finite_values(preview,3) ||
        !B2WM_Query(b2wp_view.model,B2WM_VERSION,sizeof(info),&info) ||
        info.requires_body_partition || !b2wp_key_equal(&info.key,&owner->source.model)) return false;
    b2wp_record_t posed=*owner;float matrix[16];
    if (!B2WM_Recoil(aim,owner->recoil,&posed.source.aim)) return false;
    memcpy(posed.source.preview,preview,sizeof(posed.source.preview));
    if (!b2wp_draw_matrix(b2wp_view.model,&info,b2wp_view.frame,
        b2wp_view.oldframe,b2wp_view.backlerp,&posed,matrix)) return false;
    // Aim/preview are already in this frame's body space: do not rebase twice.
    memcpy(b2wp_view.matrix,matrix,sizeof(matrix));
    b2wp_view.display_time=aim->pose_time;b2wp_view.display_capture_ms=aim->capture_ms;
    b2wp_view.display_used=true;return true;
}

// Copy only the final admitted transform. No counter, timestamp or state mutation.
bool B2WP_CopyVisual(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, int64_t eye_time, uint32_t now, b2wv_visual_t *out)
{
    if (!out) return false;
    memset(out,0,sizeof(*out));
    b2wp_record_t current;b2wm_info_t info;
    if (eye_time<=0 || !b2wp_view.requested || !b2wp_view.valid ||
        !B2W_Presentation(&current) || memcmp(&current,&b2wp_view.record,sizeof(current)) ||
        (uint32_t)(now-current.source.preview_ms)>100 ||
        handle!=b2wp_view.handle || model!=b2wp_view.model ||
        frame!=b2wp_view.frame || oldframe!=b2wp_view.oldframe ||
        backlerp!=b2wp_view.backlerp || !finite_values(b2wp_view.matrix,16) ||
        !B2WM_Query(model,B2WM_VERSION,sizeof(info),&info) ||
        !b2wp_key_equal(&info.key,&current.source.model)) return false;
    if (b2wp_view.display_used && (!b2wp_view.display_mode ||
        b2wp_view.display_time!=eye_time ||
        (uint32_t)(now-b2wp_view.display_capture_ms)>100)) return false;
    out->owner=current;out->handle=handle;out->model=model;
    out->frame=frame;out->oldframe=oldframe;out->backlerp=backlerp;
    out->eye_time=eye_time;out->display_used=b2wp_view.display_used;
    memcpy(out->matrix,b2wp_view.matrix,sizeof(out->matrix));
    return true;
}

bool B2WP_ReadSceneMatrix(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, const float origin[3], const float angles[3], uint32_t now, float matrix[16])
{
    b2wp_record_t current;b2wm_info_t info;
    if (!matrix || !origin || !angles || !b2wp_view.requested || !b2wp_view.valid ||
        !B2W_Presentation(&current) || memcmp(&current,&b2wp_view.record,sizeof(current)) ||
        (uint32_t)(now-current.source.preview_ms)>100 ||
        handle!=b2wp_view.handle || model!=b2wp_view.model || frame!=b2wp_view.frame ||
        oldframe!=b2wp_view.oldframe || backlerp!=b2wp_view.backlerp ||
        !B2WM_Query(model,B2WM_VERSION,sizeof(info),&info) ||
        !b2wp_key_equal(&info.key,&current.source.model)) return false;
    const b2wp_source_t *s=&current.source;
    const float zero[3]={0},q[4]={0,0,0,1},fov[4]={-.5f,.5f,.5f,-.5f};
    b2_view_t before,after;
    if (!B2_BuildView(zero,q,fov,s->body_origin,s->body_angles,1,&before) ||
        !B2_BuildView(zero,q,fov,origin,angles,1,&after)) return false;
    float transform[16]={0};transform[15]=1;
    for (unsigned row=0;row<3;row++) {
        for (unsigned col=0;col<3;col++) for (unsigned k=0;k<3;k++)
            transform[col*4+row]+=after.invV[k*4+row]*before.invV[k*4+col];
        transform[12+row]=origin[row];
        for (unsigned col=0;col<3;col++) transform[12+row]-=transform[col*4+row]*s->body_origin[col];
    }
    // Never read the prior scene's rebase or mutate staged draw/firing state.
    b2wp_transform_matrix(transform,b2wp_view.base_matrix,matrix);
    return true;
}

bool B2WP_RebaseView(const float origin[3], const float angles[3], uint32_t now, float muzzle[3])
{
    if (!muzzle || !B2WP_FrameReady(now) || !b2wp_view.requested || !b2wp_view.valid) return false;
    const b2wp_source_t *s=&b2wp_view.record.source;
    const float zero[3]={0},q[4]={0,0,0,1},fov[4]={-.5f,.5f,.5f,-.5f};
    b2_view_t before,after;
    if (!B2_BuildView(zero,q,fov,s->body_origin,s->body_angles,1,&before) ||
        !B2_BuildView(zero,q,fov,origin,angles,1,&after)) { b2wp_view.valid=false;return false; }
    float transform[16]={0};transform[15]=1;
    for (unsigned row=0;row<3;row++) {
        for (unsigned col=0;col<3;col++) for (unsigned k=0;k<3;k++)
            transform[col*4+row]+=after.invV[k*4+row]*before.invV[k*4+col];
        transform[12+row]=origin[row];
        for (unsigned col=0;col<3;col++) transform[12+row]-=transform[col*4+row]*s->body_origin[col];
        muzzle[row]=transform[12+row];
        for (unsigned col=0;col<3;col++) muzzle[row]+=transform[col*4+row]*s->preview[col];
    }
    // Start from the immutable staged matrix each time; duplicate roles cannot
    // accumulate movement or rotation. Grenade partitions share this transform.
    b2wp_transform_matrix(transform,b2wp_view.base_matrix,b2wp_view.matrix);
    memcpy(b2wp_view.rebase,transform,sizeof(transform));
    return true;
}

bool B2WP_ViewRequested(void) { return b2wp_view.requested; }
bool B2WP_FrameReady(uint32_t now)
{
    b2wp_record_t current;
    if (!b2wp_view.requested) return true;
    if (!b2wp_view.valid ||
        !B2W_Presentation(&current) ||
        memcmp(&current,&b2wp_view.record,sizeof(current))) {
        b2wp_view.valid=false;return false;
    }
    return true;
}
bool B2WP_ViewMatrix(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, float matrix[16])
{
    b2wm_info_t info;
    if (!matrix || !b2wp_view.requested || !b2wp_view.valid || handle!=b2wp_view.handle ||
        model!=b2wp_view.model || frame!=b2wp_view.frame || oldframe!=b2wp_view.oldframe ||
        backlerp!=b2wp_view.backlerp || !B2WM_Query(model,B2WM_VERSION,sizeof(info),&info) ||
        !b2wp_key_equal(&info.key,&b2wp_view.record.source.model)) return false;
    memcpy(matrix,b2wp_view.matrix,sizeof(b2wp_view.matrix));
    if (b2wp_observing && b2wp_observed_reads<UINT32_MAX) ++b2wp_observed_reads;
    return true;
}
static void b2wp_scale_grenade_draw(float matrix[16],const float anchor[3])
{
    for (unsigned row=0;row<3;row++) {
        for (unsigned col=0;col<3;col++) matrix[col*4+row]*=.80f;
        matrix[12+row]=anchor[row]+.80f*(matrix[12+row]-anchor[row]);
    }
}
bool B2WP_ViewPart(uint32_t handle, const void *model, int frame, int oldframe,
    float backlerp, unsigned id, b2wm_part_t *out)
{
    float matrix[16];b2wm_part_t parts[3];
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (id>=3 || b2wp_view.record.source.model.slot!=5 ||
        !B2WP_ViewMatrix(handle,model,frame,oldframe,backlerp,matrix)) return false;
    b2w_pose_t posed;
    if (!B2WM_Recoil(&b2wp_view.record.source.aim,b2wp_view.record.recoil,&posed)) return false;
    unsigned state=b2wp_view.record.grenade.state;
    if (state<B2WG_READY || state>B2WG_DETONATED ||
        !B2WM_GrenadeParts(model,&b2wp_view.record.source.model,frame,oldframe,backlerp,
            &posed,b2wp_view.record.source.preview,
            state==B2WG_READY||state==B2WG_HELD,parts)) return false;
    *out=parts[id];b2wp_transform_matrix(b2wp_view.rebase,parts[id].matrix,out->matrix);
    // All three visual partitions share the corrected hand anchor. Scale only
    // returned draw matrices, never staged pose, muzzle, collision or emission.
    float anchor[3];
    for (unsigned row=0;row<3;row++) {
        anchor[row]=b2wp_view.rebase[12+row];
        for (unsigned col=0;col<3;col++)
            anchor[row]+=b2wp_view.rebase[col*4+row]*b2wp_view.record.source.preview[col];
    }
    b2wp_scale_grenade_draw(out->matrix,anchor);
    return true;
}
bool B2WP_LeftHanded(void) { return b2wp_view.requested && b2wp_view.valid && b2wp_view.record.source.aim.hand==0; }
bool B2WP_HistoryMatches(void)
{
    const b2wp_view_t *a=&b2wp_view,*b=&b2wp_committed;
    if (!b2wp_committed_valid) return !a->requested;
    if (a->requested!=b->requested || a->valid!=b->valid) return false;
    if (!B2WV_Continuity(a->display_mode,a->display_used,b->display_mode,b->display_used)) return false;
    if (!a->requested || !a->valid) return true;
    return a->record.grenade.cycle==b->record.grenade.cycle &&
        a->record.grenade.state==b->record.grenade.state &&
        a->handle==b->handle && a->model==b->model &&
        b2wp_key_equal(&a->record.source.model,&b->record.source.model) &&
        a->record.packet.game_epoch==b->record.packet.game_epoch &&
        a->record.source.aim.hand==b->record.source.aim.hand &&
        B2W_EpochsEqual(&a->record.source.aim.epochs,&b->record.source.aim.epochs);
}
void B2WP_Commit(bool submitted)
{
    if (submitted) { b2wp_committed=b2wp_view;b2wp_committed_valid=true; }
}
