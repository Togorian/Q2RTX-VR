// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Read-only view of the accepted interruption/gesture state. Never requests resume.
bool B2_ResumeCue(b2_resume_cue261_t *cue)
{
    if (!cue) return false;
    memset(cue,0,sizeof(*cue));
    if (!B2_UIEnabled() || !b2_client.ui_interrupted || b2_client.stopped ||
        b2_client.headset_waiting || !b2_client.walk_ready || b2_client.world_waiting ||
        b2_client.world_rebinding || b2_client.handoff_position_pending) return false;
    if (!b2_client.ui_recovery_ready || !B2_InputFocused556() || !B2XR_InteractionReady()) {
        Q_strlcpy(cue->line[0],"Paused - waiting for tracking",sizeof(cue->line[0]));
        return true;
    }
    if (!b2_client.ui_manual_resume582) {
        Q_strlcpy(cue->line[0],"Paused - release all controls",sizeof(cue->line[0]));
        Q_strlcpy(cue->line[1],"Waiting for stable tracking",sizeof(cue->line[1]));
        Q_strlcpy(cue->line[2],"Resumes automatically; manual pause/menu stays",sizeof(cue->line[2]));
        return true;
    }
    Q_strlcpy(cue->line[0],"Paused - release all controls",sizeof(cue->line[0]));
    const b2c_snapshot_t *p=&b2_controller_sample;
    int dominant=b2_controller_dominant?b2_controller_dominant->integer:-1;
    char label[B2_RESUME_LABEL261];
    unsigned mapped_hand=dominant>=0 && dominant<=1?(unsigned)B2B_Source(B2B_RESUME,dominant)/6:2;
    bool known=b2_controller_resume && b2_controller_resume->integer==1 && b2_controller_enabled() &&
        b2_controller.requested && (dominant==0 || dominant==1) && b2c_sample_valid(p,Sys_Milliseconds()) &&
        p->serial==b2_controller.serial && p->generation==b2_controller.generation &&
        B2XR_ResumeControlLabel(mapped_hand,p->serial,p->generation,p->xr_generation,Sys_Milliseconds(),label);
    if (known) {
        Q_snprintf(cue->line[1],sizeof(cue->line[1]),"Press and release %s: %s",mapped_hand==0?"left hand":"right hand",label);
        Q_strlcpy(cue->line[2],"to resume, or press Pause on keyboard",sizeof(cue->line[2]));
    } else {
        Q_strlcpy(cue->line[1],"Press and release your bound resume control",sizeof(cue->line[1]));
        Q_strlcpy(cue->line[2],"or press Pause on keyboard to resume",sizeof(cue->line[2]));
    }
    return true;
}
