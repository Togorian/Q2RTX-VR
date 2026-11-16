// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
static unsigned b2b_cue_revision;
// SPDX-License-Identifier: GPL-2.0-or-later
// Included after xr and b2_actions. Optional UI metadata cannot authorise input.
static bool b2_resume_name261(PFN_xrGetInputSourceLocalizedName get_name,
    XrPath source, XrInputSourceLocalizedNameFlags flags, char name[B2_RESUME_LABEL261])
{
    XrInputSourceLocalizedNameGetInfo info={XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO};
    info.sourcePath=source;info.whichComponents=flags;
    uint32_t count=0;
    memset(name,0,B2_RESUME_LABEL261);
    if (get_name(xr.session,&info,B2_RESUME_LABEL261,&count,name)!=XR_SUCCESS ||
        count<2 || count>B2_RESUME_LABEL261 || name[count-1]!=0 ||
        strlen(name)!=count-1) return false;
    // The stock font is single-byte. Never truncate UTF-8 or control text.
    for (uint32_t i=0;i<count-1;i++) if ((unsigned char)name[i]<32 || (unsigned char)name[i]>126 || name[i]=='/' || name[i]=='\\') return false;
    return name[0]!=' ' && name[count-2]!=' ';
}

static int b2_resume_hand261(const char *name)
{
    char lower[B2_RESUME_LABEL261];unsigned i=0;
    for (;name[i] && i<sizeof(lower)-1;i++) lower[i]=(name[i]>='A' && name[i]<='Z')?name[i]+('a'-'A'):name[i];
    lower[i]=0;
    // OpenXR has no per-subaction enumeration filter. The source paths are opaque.
    // Only these unambiguous English USER_PATH names can be associated here.
    // Other localisations/names use the honest keyboard fallback, never a guess.
    if (!strcmp(lower,"left hand")) return 0;
    if (!strcmp(lower,"right hand")) return 1;
    return -1;
}

#include "faces413.inc"

void B2XR_UpdateResumeCue(bool needed, uint64_t serial, uint32_t generation, uint32_t input_generation)
{
    memset(&xr.resume_cue,0,sizeof(xr.resume_cue));
    if (!needed || !serial || !generation || !input_generation || xr.frame_open ||
        !B2XR_InteractionReady() || !b2_actions.attached || !b2_actions.input_enabled ||
        b2_actions.exhausted || serial!=b2_actions.serial || generation!=b2_actions.generation ||
        input_generation!=xr.input_generation) return;
    xr.resume_cue.serial=serial;xr.resume_cue.generation=generation;
    xr.resume_cue.input_generation=input_generation;
}

static void b2_xr_refresh_resume_cue261(void)
{
    b2_xr_refresh_faces412();
    memset(xr.resume_cue.label,0,sizeof(xr.resume_cue.label));
    // The existing haptics preparation also calls xrSyncActions. Refresh once
    // after it, before the eye loop. Metadata calls are legal inside a frame;
    // the draw getter makes no native calls and never syncs/reads action state.
    if (!xr.frame_open || !xr.resume_cue.serial || !B2XR_InteractionReady() ||
        !b2_actions.attached || !b2_actions.input_enabled || b2_actions.exhausted ||
        xr.resume_cue.serial!=b2_actions.serial || xr.resume_cue.generation!=b2_actions.generation ||
        xr.resume_cue.input_generation!=xr.input_generation || !xr.get_proc) return;
    PFN_xrEnumerateBoundSourcesForAction enumerate=NULL;
    PFN_xrGetInputSourceLocalizedName get_name=NULL;
    if (xr.get_proc(xr.instance,"xrEnumerateBoundSourcesForAction",(PFN_xrVoidFunction*)&enumerate)!=XR_SUCCESS ||
        xr.get_proc(xr.instance,"xrGetInputSourceLocalizedName",(PFN_xrVoidFunction*)&get_name)!=XR_SUCCESS ||
        !enumerate || !get_name) return;
    XrBoundSourcesForActionEnumerateInfo info={XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO};
    const unsigned actions[6]={B2CA_STICK,B2CA_TRIGGER,B2CA_SQUEEZE,B2CA_PRIMARY,B2CA_SECONDARY,B2CA_CLICK};
    cvar_t *dominant=Cvar_FindVar("b2_controller_dominant_hand");
    if(!dominant || dominant->integer<0 || dominant->integer>1)return;
    info.action=b2_actions.actions[actions[B2B_Source(B2B_RESUME,dominant->integer)%6]];
    b2b_cue_revision=B2B_Revision();
    XrPath sources[8];uint32_t count=0;
    if (enumerate(xr.session,&info,8,&count,sources)!=XR_SUCCESS || !count || count>8) return;
    char labels[2][B2_RESUME_LABEL261]={{0}};unsigned matches[2]={0};
    for (uint32_t i=0;i<count;i++) {
        // No path-to-string call, persistence, prefix inference or default binding.
        char user[B2_RESUME_LABEL261],component[B2_RESUME_LABEL261];
        if (!b2_resume_name261(get_name,sources[i],XR_INPUT_SOURCE_LOCALIZED_NAME_USER_PATH_BIT,user)) return;
        int hand=b2_resume_hand261(user);
        if (hand<0) return;
        matches[hand]++;
        if (!b2_resume_name261(get_name,sources[i],XR_INPUT_SOURCE_LOCALIZED_NAME_COMPONENT_BIT,component)) return;
        memcpy(labels[hand],component,sizeof(component));
    }
    for (unsigned h=0;h<2;h++) if (matches[h]==1) memcpy(xr.resume_cue.label[h],labels[h],sizeof(labels[h]));
    xr.resume_cue.time=Sys_Milliseconds();
}

bool B2XR_ResumeControlLabel(unsigned hand, uint64_t serial, uint32_t generation,
    uint32_t input_generation, uint32_t now, char label[B2_RESUME_LABEL261])
{
    if (!label) return false;
    label[0]=0;
    if (b2b_cue_revision!=B2B_Revision() || hand>1 || !serial || !generation || !input_generation || !B2XR_InteractionReady() ||
        !b2_actions.attached || !b2_actions.input_enabled || b2_actions.exhausted ||
        serial!=b2_actions.serial || generation!=b2_actions.generation || input_generation!=xr.input_generation ||
        serial!=xr.resume_cue.serial || generation!=xr.resume_cue.generation ||
        input_generation!=xr.resume_cue.input_generation || (uint32_t)(now-xr.resume_cue.time)>B2C_MAX_AGE_MS ||
        !xr.resume_cue.label[hand][0]) return false;
    memcpy(label,xr.resume_cue.label[hand],B2_RESUME_LABEL261);
    return true;
}

// Cached active profile from the last admitted action sync.
bool B2B_MenuAvailable316(unsigned hand)
{
    return hand<2 && b2_actions.menu_active316[hand] && B2XR_InteractionReady();
}
int B2B_Profile316(unsigned hand)
{
    if (hand>1 || !b2_actions.attached || !b2_actions.input_enabled ||
        !B2XR_InteractionReady()) return -1;
    for (int i=0;i<3;i++)
        if (b2_actions.suggested[i] && b2_actions.current[hand]==b2_actions.profiles[i]) return i;
    return -1;
}

bool B2B_Label(unsigned source,char *label,unsigned capacity)
{
    cvar_t *d=Cvar_FindVar("b2_controller_dominant_hand");char name[B2_RESUME_LABEL261];
    if(!d || d->integer<0 || d->integer>1 || source!=(unsigned)B2B_Source(B2B_RESUME,d->integer))return false;
    if(!B2XR_ResumeControlLabel(source/6,xr.resume_cue.serial,xr.resume_cue.generation,xr.resume_cue.input_generation,Sys_Milliseconds(),name))return false;
    Q_strlcpy(label,name,capacity);return true;
}
