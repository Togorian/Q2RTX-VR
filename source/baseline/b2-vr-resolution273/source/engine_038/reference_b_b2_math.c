// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Standalone CPU code: no renderer, OpenXR loader or engine initialization.
#include "refresh/reference_b_b2.h"
#include "refresh/b2_weapon.h"
#include <math.h>
#include <string.h>

#define B2_PI 3.14159265358979323846f

static bool finite_values(const float *v, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        if (!isfinite(v[i]))
            return false;
    return true;
}

bool B2W_EpochsEqual(const b2w_epochs_t *a, const b2w_epochs_t *b)
{
    return a && b && a->action && a->xr && a->world && a->owner && a->calibration &&
        a->action==b->action && a->xr==b->xr && a->world==b->world &&
        a->recentre==b->recentre && a->owner==b->owner && a->calibration==b->calibration;
}

static float b2w_dot(const float *a, const float *b)
{
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

bool B2W_PoseCurrent(const b2w_pose_t *p, const b2w_epochs_t *epochs, uint32_t now)
{
    if (!p || !p->serial || p->pose_time<=0 || p->hand>1 ||
        !B2W_EpochsEqual(&p->epochs,epochs) || (uint32_t)(now-p->capture_ms)>100 ||
        (uint32_t)(now-p->head_ms)>100 || !finite_values(p->origin,3) ||
        !finite_values(p->forward,3) || !finite_values(p->right,3) || !finite_values(p->up,3)) return false;
    if (fabsf(b2w_dot(p->forward,p->forward)-1)>.001f ||
        fabsf(b2w_dot(p->right,p->right)-1)>.001f || fabsf(b2w_dot(p->up,p->up)-1)>.001f ||
        fabsf(b2w_dot(p->forward,p->right))>.001f || fabsf(b2w_dot(p->forward,p->up))>.001f ||
        fabsf(b2w_dot(p->right,p->up))>.001f) return false;
    // Quake's physical right basis is forward cross up (not model-left).
    float right[3]={p->forward[1]*p->up[2]-p->forward[2]*p->up[1],
        p->forward[2]*p->up[0]-p->forward[0]*p->up[2],p->forward[0]*p->up[1]-p->forward[1]*p->up[0]};
    return b2w_dot(right,p->right)>.999f;
}

// Rebase one proposed smooth-turn publication about its tracked head centre.
// retire_raw is acknowledged minus predicted room travel. Retiring prediction
// moves only this provisional presentation anchor; authoritative pmove is untouched.
bool B2W_TurnSpace(const b2w_space_t *old,const float head[3],
    const float angles[3],const float retire_raw[3],b2w_space_t *out)
{
    if(!old || !head || !angles || !retire_raw || !out || !old->committed ||
       !finite_values(head,3) || !finite_values(retire_raw,3) ||
       !finite_values(old->reference_origin,3) || !isfinite(old->reference_yaw))return false;
    const float q[4]={0,0,0,1},fov[4]={-.5f,.5f,.5f,-.5f},zero[3]={0};
    float c=cosf(old->reference_yaw),s=sinf(old->reference_yaw);
    float retired[3]={c*retire_raw[0]-s*retire_raw[2],retire_raw[1],
        s*retire_raw[0]+c*retire_raw[2]};
    b2_view_t previous,next;
    if(!B2_BuildView(retired,q,fov,old->anchor_origin,old->anchor_angles,old->units,&previous) ||
       !B2_BuildView(zero,q,fov,zero,angles,1.f,&next))return false;
    float x=head[0]-old->reference_origin[0]-retire_raw[0];
    float y=head[1]-old->reference_origin[1]-retire_raw[1];
    float z=head[2]-old->reference_origin[2]-retire_raw[2];
    float p[3]={c*x-s*z,y,s*x+c*z},world[3],relative[3]={0};
    for(unsigned i=0;i<3;i++)
        world[i]=previous.invV[i]*p[0]+previous.invV[4+i]*p[1]-previous.invV[8+i]*p[2];
    // Full orthonormal inverse: no horizontal determinant or gaze singularity.
    for(unsigned i=0;i<3;i++){
        relative[0]+=next.invV[i]*world[i];
        relative[1]+=next.invV[4+i]*world[i];
        relative[2]-=next.invV[8+i]*world[i];
    }
    b2w_space_t result=*old;
    result.reference_origin[0]=head[0]-(c*relative[0]+s*relative[2]);
    result.reference_origin[1]=head[1]-relative[1];
    result.reference_origin[2]=head[2]-(-s*relative[0]+c*relative[2]);
    memcpy(result.anchor_origin,previous.origin,sizeof(result.anchor_origin));
    memcpy(result.anchor_angles,angles,sizeof(result.anchor_angles));
    if(!finite_values(result.reference_origin,3))return false;
    *out=result;return true;
}

bool B2W_BuildAim(const b2c_snapshot_t *sample, unsigned hand,
    const b2w_space_t *space, uint32_t now, b2w_pose_t *out)
{
    if (out) memset(out,0,sizeof(*out));
    if (!sample || !space || !out || hand>1 || !space->committed || !sample->ready ||
        !b2c_aim_usable556(sample,hand,now) ||
        sample->generation!=space->epochs.action || sample->xr_generation!=space->epochs.xr ||
        !finite_values(space->reference_origin,3) || !isfinite(space->reference_yaw)) return false;
    const float *position=sample->hands[hand].aim_position, *q=sample->hands[hand].aim_orientation;
    const float yaw=space->reference_yaw;
    float c=cosf(yaw),s=sinf(yaw),h=sinf(-yaw*.5f),w=cosf(-yaw*.5f);
    float x=position[0]-space->reference_origin[0],z=position[2]-space->reference_origin[2];
    float p[3]={c*x-s*z,position[1]-space->reference_origin[1],s*x+c*z};
    // Same committed inverse-yaw multiplication as the accepted head path.
    float relative[4]={w*q[0]+h*q[2],w*q[1]+h*q[3],w*q[2]-h*q[0],w*q[3]-h*q[1]};
    const float fov[4]={-.5f,.5f,.5f,-.5f}; b2_view_t view;
    if (!B2_BuildView(p,relative,fov,space->anchor_origin,space->anchor_angles,space->units,&view)) return false;
    b2w_pose_t result={0}; result.epochs=space->epochs;result.serial=sample->serial;
    result.pose_time=sample->pose_time;result.capture_ms=sample->capture_ms;result.head_ms=sample->head_ms;result.hand=hand;
    for(unsigned i=0;i<3;i++) { result.origin[i]=view.origin[i];result.forward[i]=view.invV[8+i];result.right[i]=view.invV[i];result.up[i]=view.invV[4+i]; }
    if (!B2W_PoseCurrent(&result,&space->epochs,now)) return false;
    *out=result;return true;
}

// Basis-space pointing uses the project selector design. Including the
// hand origin makes the cursor agree with B2's existing finite flat HUD plane.
bool B2W_HUDRay(const b2c_snapshot_t *sample, unsigned hand,
    float aspect, float *x, float *y)
{
    if (!sample || !x || !y || hand>1 || !sample->ready || !sample->hands[hand].aim_valid ||
        !isfinite(aspect) || aspect<=0 || aspect>4) return false;
    const float fov[4]={-.5f,.5f,.5f,-.5f},zero[3]={0};b2_view_t head,aim;
    if (!B2_BuildView(sample->head_position,sample->head_orientation,fov,zero,zero,1,&head) ||
        !B2_BuildView(sample->hands[hand].aim_position,sample->hands[hand].aim_orientation,fov,zero,zero,1,&aim)) return false;
    float centre[3],delta[3],hit[3];
    for(unsigned i=0;i<3;i++) { centre[i]=head.origin[i]+1.5f*head.invV[8+i];delta[i]=centre[i]-aim.origin[i]; }
    float depth=b2w_dot(aim.invV+8,head.invV+8);
    if (!isfinite(depth) || depth<=.0872f) return false;
    float distance=b2w_dot(delta,head.invV+8)/depth;
    if (!isfinite(distance) || distance<=0) return false;
    for(unsigned i=0;i<3;i++) hit[i]=aim.origin[i]+distance*aim.invV[8+i]-centre[i];
    float px=b2w_dot(hit,head.invV)/(1.5f*.70020754f);
    float py=-b2w_dot(hit,head.invV+4)/(1.5f*.70020754f*aspect);
    if (!isfinite(px) || !isfinite(py) || fabsf(px)>1 || fabsf(py)>1) return false;
    *x=px;*y=py;return true;
}


bool B2W_PlaneRay(const b2w_pose_t *plane, const b2w_pose_t *aim,
    float aspect, float *x, float *y)
{
    if (!plane || !aim || !x || !y || !isfinite(aspect) || aspect<=0 || aspect>4 ||
        !finite_values(plane->origin,3) || !finite_values(plane->forward,3) ||
        !finite_values(plane->right,3) || !finite_values(plane->up,3) ||
        !finite_values(aim->origin,3) || !finite_values(aim->forward,3)) return false;
    // Input is in metres, as is the unchanged physical HUD plane.
    float centre[3],delta[3],hit[3];
    for (unsigned i=0;i<3;i++) { centre[i]=plane->origin[i]+1.5f*plane->forward[i];delta[i]=centre[i]-aim->origin[i]; }
    float depth=b2w_dot(aim->forward,plane->forward);
    if (!isfinite(depth) || depth<=.0872f) return false;
    float distance=b2w_dot(delta,plane->forward)/depth;
    if (!isfinite(distance) || distance<=0) return false;
    for (unsigned i=0;i<3;i++) hit[i]=aim->origin[i]+distance*aim->forward[i]-centre[i];
    float px=b2w_dot(hit,plane->right)/(1.5f*.70020754f);
    float py=-b2w_dot(hit,plane->up)/(1.5f*.70020754f*aspect);
    if (!isfinite(px) || !isfinite(py) || fabsf(px)>1 || fabsf(py)>1) return false;
    *x=px;*y=py;return true;
}

bool B2W_GridCell(unsigned slot, float aspect, float rect[4])
{
    if (!rect || slot>=12 || !isfinite(aspect) || aspect<=0 || aspect>4) return false;
    // Reduce the accepted 70% selector a further 10%, about the same centre.
    // Drawing and hit testing share these square physical cell dimensions.
    rect[0]=-.4536f+.2268f*(slot%4);
    rect[1]=(-.3402f+.2268f*(slot/4))/aspect;
    rect[2]=.2268f;rect[3]=.2268f/aspect;
    return true;
}

int B2W_GridSlot(float x, float y, float aspect)
{
    float rect[4];
    if (!isfinite(x) || !isfinite(y) || !B2W_GridCell(0,aspect,rect)) return -1;
    float gx=(x-rect[0])/rect[2],gy=(y-rect[1])/rect[3];
    if (gx<0 || gx>=4 || gy<0 || gy>=3) return -1;
    unsigned col=(unsigned)gx,row=(unsigned)gy;
    float fx=gx-col,fy=gy-row;
    if (fx<.05f || fx>.95f || fy<.05f || fy>.95f) return -1;
    return (int)(row*4+col);
}

// Arrow bounds share the grid's physical aspect correction. The pointed
// slot 12 is navigation only and can never become a weapon/item index.
bool B2W_PageArrow(unsigned page, float aspect, float rect[4])
{
    if (!rect || page>1 || !isfinite(aspect) || aspect<=0 || aspect>4) return false;
    rect[0]=page?-.72f:.60f;rect[1]=-.28f/aspect;
    rect[2]=.12f;rect[3]=.56f/aspect;return true;
}
bool B2W_PageHit(unsigned page, float x, float y, float aspect)
{
    float r[4];
    if (!isfinite(x) || !isfinite(y) || !B2W_PageArrow(page,aspect,r)) return false;
    float u=(x-r[0])/r[2],v=(y-r[1])/r[3];
    if (u<0 || u>1 || v<0 || v>1) return false;
    return (page?1-u:u)<=1-fabsf(2*v-1);
}

float B2W_PageProgress(uint32_t start, uint32_t now)
{
    uint32_t age=now-start;
    if (age>=180) return 1.f;
    float t=(float)age/180.f;
    return t*t*(3.f-2.f*t);
}

void B2W_CancelSelector(b2w_selector_t *state)
{
    state->open=false;state->rearm=true;state->candidate=state->selected=-1;
    state->candidate_ms=state->candidate_samples=0;state->page=0;
    state->hovering=state->nav_latched=state->sliding=false;
    state->hover_ms=state->hover_samples=state->slide_ms=state->slide_from=0;
}

b2w_selection_t B2W_Select(b2w_selector_t *state, const b2c_snapshot_t *sample,
    const b2w_epochs_t *epochs, uint32_t now, unsigned hand, bool eligible,
    bool weapon_ready, uint16_t available, int pointed_slot)
{
    b2w_selection_t result={.selected=-1,.used=-1};
    bool valid=state && sample && epochs && hand<2 && sample->ready && sample->serial && sample->pose_time>0 &&
        sample->generation==epochs->action && sample->xr_generation==epochs->xr &&
        (uint32_t)(now-sample->capture_ms)<=100 && (uint32_t)(now-sample->head_ms)<=100 &&
        b2c_aim_usable556(sample,hand,now) && sample->hands[hand].squeeze_active &&
        isfinite(sample->hands[hand].squeeze) && sample->hands[hand].squeeze>=0 && sample->hands[hand].squeeze<=1;
    if (valid) valid=finite_values(sample->head_position,3) && finite_values(sample->head_orientation,4) &&
        fabsf(b2w_dot(sample->head_orientation,sample->head_orientation)+
            sample->head_orientation[3]*sample->head_orientation[3]-1)<.02f;
    if (!state) return result;
    bool was_open=state->open;
    // Only the selector's aim/grip owns its geometry. The other hand's
    // unavailable pose must not cancel this independently usable controller.
    if(valid)for(unsigned i=0;i<2;i++){
        const b2c_hand_t *h=&sample->hands[i];
        valid=valid && isfinite(h->trigger) && h->trigger>=0 && h->trigger<=1 &&
            finite_values(h->stick,2) && fabsf(h->stick[0])<=1 && fabsf(h->stick[1])<=1;
    }
    if (!valid || !eligible || !B2W_EpochsEqual(epochs,epochs)) {
        B2W_CancelSelector(state);result.suppress=true;result.cancelled=was_open;return result;
    }
    if (!state->configured || state->hand!=hand || !B2W_EpochsEqual(&state->epochs,epochs)) {
        B2W_CancelSelector(state);state->configured=true;state->hand=hand;state->epochs=*epochs;
        state->serial=sample->serial;state->sample_ms=now;result.suppress=true;result.cancelled=was_open;return result;
    }
    if (sample->serial<=state->serial) {
        if (sample->serial<state->serial) { B2W_CancelSelector(state);result.cancelled=was_open; }
        result.suppress=was_open || state->rearm;return result;
    }
    state->serial=sample->serial;
    uint32_t gap=now-state->sample_ms;state->sample_ms=now;
    if (gap>100) { B2W_CancelSelector(state);result.cancelled=was_open;result.suppress=true;return result; }
    const b2c_hand_t *d=&sample->hands[hand];
    if (state->rearm) {
        bool neutral=d->squeeze<.35f;
        for(unsigned i=0;i<2;i++) {
            const b2c_hand_t *h=&sample->hands[i];
            // Selector rearm owns the preferred grip/turn controls. The
            // offhand movement/crouch may continue; b2c_neutral still owns
            // full release after handoff, interruption or tracking loss.
            neutral=neutral && h->trigger<.45f &&
                (i!=hand || (h->squeeze<.35f && fabsf(h->stick[0])<.25f && fabsf(h->stick[1])<.25f)) &&
                !h->primary && !h->secondary && !h->menu && !h->click;
        }
        if (neutral) state->rearm=false;
        result.suppress=true;return result;
    }
    if (!state->open) {
        if (d->squeeze<=.65f) return result;
        if (!weapon_ready || !isfinite(d->trigger) || d->trigger>=.45f) { B2W_CancelSelector(state);result.suppress=true;return result; }
        state->open=true;state->candidate=state->selected=-1;result.opened=true;
    }
    result.suppress=true;
    // Motion consumes admitted samples but can never select, use or fire.
    // Release closes even on the final slide frame; a new dwell is required.
    if (state->sliding) {
        state->candidate=state->selected=-1;state->candidate_samples=0;
        if (d->squeeze<.35f) {
            result.closed=true;B2W_CancelSelector(state);return result;
        }
        if ((uint32_t)(now-state->slide_ms)>=180) state->sliding=false;
        return result;
    }
    if (pointed_slot!=12) {
        state->hovering=false;state->hover_samples=0;state->nav_latched=false;
    } else if (d->squeeze>=.35f && !state->nav_latched) {
        if (!state->hovering) {
            state->hovering=true;state->hover_ms=now;state->hover_samples=1;
        } else if (state->hover_samples<2) state->hover_samples++;
        uint32_t age=now-state->hover_ms;
        if (age>=60 && age<=1000 && state->hover_samples>=2) {
            state->slide_from=state->page;state->page^=1;
            state->slide_ms=now;state->sliding=true;state->nav_latched=true;
            state->hovering=false;state->hover_samples=0;
            state->candidate=state->selected=-1;
            state->candidate_ms=state->candidate_samples=0;
            result.page_changed=true;return result;
        }
    }
    if (pointed_slot<0 || pointed_slot>=12 || !(available&(1u<<pointed_slot))) {
        state->candidate=state->selected=-1;state->candidate_samples=0;
    } else if (state->candidate!=pointed_slot) {
        state->candidate=pointed_slot;state->selected=-1;state->candidate_ms=now;state->candidate_samples=1;
    } else {
        if (state->candidate_samples<2) state->candidate_samples++;
        uint32_t age=now-state->candidate_ms;
        if (age<=1000 && age>=60 && state->candidate_samples>=2) state->selected=pointed_slot;
        else if (age>1000 && state->selected<0) { state->candidate_ms=now;state->candidate_samples=1; }
    }
    // Both pages commit only on an admitted physical grip release. The
    // release frame above rechecks hover/availability; invalid input/epochs
    // returned through cancellation before reaching this edge.
    if (d->squeeze<.35f) {
        if (state->page) result.used=state->selected;
        else result.selected=state->selected;
        result.closed=true;B2W_CancelSelector(state);
    }
    return result;
}

bool B2_BuildView(const float position[3], const float q[4],
    const float fov[4], const float anchor_origin[3],
    const float anchor_angles[3], float units, b2_view_t *view)
{
    b2_view_t result = { 0 };
    float anchor_right[3], anchor_up[3], anchor_forward[3];
    float right[3], up[3], forward[3], rotation[3][3];
    float norm, x, y, z, w, sp, cp, sy, cy, sr, cr;
    float left, right_tan, top, bottom, width, height;
    const float near_plane = 1.f, far_plane = 4096.f;

    if (!position || !q || !fov || !anchor_origin || !anchor_angles || !view ||
        !finite_values(position, 3) || !finite_values(q, 4) ||
        !finite_values(fov, 4) || !finite_values(anchor_origin, 3) ||
        !finite_values(anchor_angles, 3) || !isfinite(units) || units <= 0.f)
        return false;
    norm = q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3];
    if (!isfinite(norm) || fabsf(norm - 1.f) > 0.02f)
        return false;
    for (unsigned i = 0; i < 4; ++i)
        if (fabsf(fov[i]) >= B2_PI * 0.5f)
            return false;
    if (fov[0] >= fov[1] || fov[3] >= fov[2])
        return false;
    left = tanf(fov[0]); right_tan = tanf(fov[1]);
    top = tanf(fov[2]); bottom = tanf(fov[3]);
    width = right_tan - left; height = top - bottom;
    if (!isfinite(width) || !isfinite(height) || width < 1e-5f || height < 1e-5f)
        return false;

    sp = sinf(anchor_angles[0] * B2_PI / 180.f);
    cp = cosf(anchor_angles[0] * B2_PI / 180.f);
    sy = sinf(anchor_angles[1] * B2_PI / 180.f);
    cy = cosf(anchor_angles[1] * B2_PI / 180.f);
    sr = sinf(anchor_angles[2] * B2_PI / 180.f);
    cr = cosf(anchor_angles[2] * B2_PI / 180.f);
    anchor_forward[0] = cp*cy; anchor_forward[1] = cp*sy; anchor_forward[2] = -sp;
    anchor_right[0] = -sr*sp*cy + cr*sy;
    anchor_right[1] = -sr*sp*sy - cr*cy;
    anchor_right[2] = -sr*cp;
    anchor_up[0] = cr*sp*cy + sr*sy;
    anchor_up[1] = cr*sp*sy - sr*cy;
    anchor_up[2] = cr*cp;

    norm = 1.f / sqrtf(norm);
    x = q[0]*norm; y = q[1]*norm; z = q[2]*norm; w = q[3]*norm;
    rotation[0][0] = 1.f - 2.f*(y*y + z*z);
    rotation[0][1] = 2.f*(x*y - z*w);
    rotation[0][2] = 2.f*(x*z + y*w);
    rotation[1][0] = 2.f*(x*y + z*w);
    rotation[1][1] = 1.f - 2.f*(x*x + z*z);
    rotation[1][2] = 2.f*(y*z - x*w);
    rotation[2][0] = 2.f*(x*z - y*w);
    rotation[2][1] = 2.f*(y*z + x*w);
    rotation[2][2] = 1.f - 2.f*(x*x + y*y);
    for (unsigned i = 0; i < 3; ++i) {
        result.origin[i] = anchor_origin[i] + units *
            (anchor_right[i]*position[0] + anchor_up[i]*position[1] -
             anchor_forward[i]*position[2]);
        right[i] = anchor_right[i]*rotation[0][0] + anchor_up[i]*rotation[1][0] -
            anchor_forward[i]*rotation[2][0];
        up[i] = anchor_right[i]*rotation[0][1] + anchor_up[i]*rotation[1][1] -
            anchor_forward[i]*rotation[2][1];
        forward[i] = -anchor_right[i]*rotation[0][2] - anchor_up[i]*rotation[1][2] +
            anchor_forward[i]*rotation[2][2];
        result.V[i*4] = right[i]; result.V[i*4+1] = up[i];
        result.V[i*4+2] = forward[i];
        result.invV[i] = right[i]; result.invV[4+i] = up[i];
        result.invV[8+i] = forward[i]; result.invV[12+i] = result.origin[i];
    }
    for (unsigned i = 0; i < 3; ++i) {
        result.V[12] -= right[i]*result.origin[i];
        result.V[13] -= up[i]*result.origin[i];
        result.V[14] -= forward[i]*result.origin[i];
    }
    result.V[15] = result.invV[15] = 1.f;
    norm = sqrtf(forward[0]*forward[0] + forward[1]*forward[1]);
    result.angles[0] = atan2f(-forward[2], norm) * 180.f / B2_PI;
    result.angles[1] = (norm > 1e-6f ? atan2f(forward[1], forward[0]) :
        atan2f(right[0], -right[1])) * 180.f / B2_PI;
    result.angles[2] = norm > 1e-6f ? atan2f(-right[2], up[2]) * 180.f / B2_PI : 0.f;
    result.fov_xy[0] = (fov[1] - fov[0]) * 180.f / B2_PI;
    result.fov_xy[1] = (fov[2] - fov[3]) * 180.f / B2_PI;

    // VKPT uses +Z forward and a downward image Y axis. Keep its depth
    // convention, while representing the actual asymmetric eye frustum.
    result.P[0] = 2.f / width; result.P[8] = -(right_tan + left) / width;
    result.P[5] = -2.f / height; result.P[9] = (top + bottom) / height;
    result.P[10] = (far_plane + near_plane) / (far_plane - near_plane);
    result.P[14] = 2.f * far_plane * near_plane / (far_plane - near_plane);
    result.P[11] = 1.f;
    result.invP[0] = 1.f / result.P[0]; result.invP[5] = 1.f / result.P[5];
    result.invP[12] = -result.P[8] / result.P[0];
    result.invP[13] = -result.P[9] / result.P[5];
    result.invP[11] = 1.f / result.P[14]; result.invP[14] = 1.f;
    result.invP[15] = -result.P[10] / result.P[14];
    if (!finite_values(result.origin, 3) || !finite_values(result.V, 16) ||
        !finite_values(result.invV, 16) || !finite_values(result.P, 16) ||
        !finite_values(result.invP, 16))
        return false;
    *view = result;
    return true;
}

void B2_HistoryReset(b2_history_t *history)
{
    if (history)
        memset(history, 0, sizeof(*history));
}

bool B2_HistoryBegin(b2_history_t *history, b2_frame_t *frame)
{
    if (history && history->frame_open) {
        history->failed = true;
        return false;
    }
    if (!history || !frame || !frame->should_render ||
        !frame->tracking_valid || !B2_ValidViewCount(frame->view_count) || frame->history_generation == 0 ||
        frame->predicted_time_ns <= 0 || frame->duration_ns <= 0 ||
        history->logical_index == UINT32_MAX)
        return false;
    if (history->generation != frame->history_generation) {
        history->valid = false;
        history->generation = frame->history_generation;
    }
    if (history->view_count != frame->view_count) history->valid = false;
    for (unsigned eye = 0; eye < frame->view_count; ++eye) {
        if (!frame->output_width[eye] || !frame->output_height[eye] ||
            frame->output_width[eye] > INT32_MAX || frame->output_height[eye] > INT32_MAX)
            return false;
        if (history->output_width[eye] != frame->output_width[eye] ||
            history->output_height[eye] != frame->output_height[eye] ||
            // Validated Quad Views reproject through their own committed
            // previous projections, independently of gaze-request eligibility.
            // Generation/count/extent changes still reset, as do projection
            // changes in stereo.
            (frame->view_count != 4 &&
             memcmp(history->committed[eye].P, frame->views[eye].P, sizeof(frame->views[eye].P))))
            history->valid = false;
        history->output_width[eye] = frame->output_width[eye];
        history->output_height[eye] = frame->output_height[eye];
    }
    history->view_count = frame->view_count;
    frame->logical_index = history->logical_index;
    frame->history_valid = history->valid;
    for (unsigned eye = 0; eye < frame->view_count; ++eye) {
        frame->previous[eye] = history->valid ? history->committed[eye] : frame->views[eye];
        history->pending[eye] = frame->views[eye];
    }
    history->accepted_mask = 0;
    history->failed = false;
    history->frame_open = true;
    return true;
}

bool B2_HistoryAcceptEye(b2_history_t *history, uint32_t eye)
{
    if (!history)
        return false;
    if (!history->frame_open || history->failed || eye >= history->view_count ||
        history->accepted_mask != ((1u << eye) - 1u)) {
        history->failed = true;
        return false;
    }
    history->accepted_mask |= 1u << eye;
    return true;
}

bool B2_HistoryFinish(b2_history_t *history, bool submitted_pair)
{
    bool commit;
    if (!history)
        return false;
    commit = history->frame_open && !history->failed &&
        history->accepted_mask == B2_ViewMask(history->view_count) && submitted_pair;
    if (commit) {
        memcpy(history->committed, history->pending, sizeof(history->committed));
        history->logical_index++;
    }
    history->valid = commit;
    history->frame_open = false;
    history->accepted_mask = 0;
    history->failed = false;
    return commit;
}

#include "refresh/b2_weapon_transport.inc"

#include "refresh/b2_roomscale_transport.inc"
#include "refresh/b2_teleport_transport.inc"
