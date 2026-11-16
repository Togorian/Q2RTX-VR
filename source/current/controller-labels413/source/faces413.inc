// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Optional presentation metadata. Never grants input authority or changes bindings.
static struct {
    XrSession session;
    XrPath profile[2];
    uint32_t generation,input_generation;
    bool checked,sony[2];
} b2_faces412;

static bool b2_sony_name412(const char *name)
{
    char lower[B2_RESUME_LABEL261];unsigned i=0;
    for(;name[i] && i<sizeof(lower)-1;i++)
        lower[i]=(name[i]>='A' && name[i]<='Z')?name[i]+('a'-'A'):name[i];
    lower[i]=0;
    // Exact, explicit controller names only. Index/Touch contracts, headset
    // names, generic "Sony" and unrelated substrings are not identification.
    // Unrecognised/localised/emulated names retain the existing labels.
    return !strcmp(lower,"playstation vr2 sense controller") ||
        !strcmp(lower,"ps vr2 sense controller") ||
        !strcmp(lower,"psvr2 sense controller") ||
        !strcmp(lower,"playstation vr2 sense controllers") ||
        !strcmp(lower,"ps vr2 sense controllers") ||
        !strcmp(lower,"psvr2 sense controllers");
}

static bool b2_faces_ready412(void)
{
    return B2XR_InteractionReady() && b2_actions.attached &&
        b2_actions.input_enabled && !b2_actions.exhausted &&
        b2_actions.generation && xr.input_generation;
}
static bool b2_faces_current412(void)
{
    return b2_faces412.checked && b2_faces412.session==xr.session &&
        b2_faces412.generation==b2_actions.generation &&
        b2_faces412.input_generation==xr.input_generation &&
        b2_faces412.profile[0]==b2_actions.current[0] &&
        b2_faces412.profile[1]==b2_actions.current[1];
}

static void b2_xr_refresh_faces412(void)
{
    if(!b2_faces_ready412()) {memset(&b2_faces412,0,sizeof(b2_faces412));return;}
    // Called after the final existing action sync, never by menu rendering.
    // Re-query only at init, an observed profile/input generation change, or
    // re-entry after loss of readiness. A failed query is not retried per frame.
    if(!xr.frame_open || b2_faces_current412())return;
    memset(&b2_faces412,0,sizeof(b2_faces412));
    b2_faces412.checked=true;b2_faces412.session=xr.session;
    b2_faces412.generation=b2_actions.generation;
    b2_faces412.input_generation=xr.input_generation;
    memcpy(b2_faces412.profile,b2_actions.current,sizeof(b2_faces412.profile));
    PFN_xrEnumerateBoundSourcesForAction enumerate=NULL;
    PFN_xrGetInputSourceLocalizedName get_name=NULL;
    if(!xr.get_proc ||
        xr.get_proc(xr.instance,"xrEnumerateBoundSourcesForAction",(PFN_xrVoidFunction*)&enumerate)!=XR_SUCCESS ||
        xr.get_proc(xr.instance,"xrGetInputSourceLocalizedName",(PFN_xrVoidFunction*)&get_name)!=XR_SUCCESS ||
        !enumerate || !get_name)return;
    bool sony[2]={true,true};
    const unsigned actions[2]={B2CA_PRIMARY,B2CA_SECONDARY};
    for(unsigned a=0;a<2;a++){
        XrBoundSourcesForActionEnumerateInfo info={XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO};
        info.action=b2_actions.actions[actions[a]];
        XrPath sources[8];uint32_t count=0;unsigned matches[2]={0};
        if(enumerate(xr.session,&info,8,&count,sources)!=XR_SUCCESS || !count || count>8)return;
        for(uint32_t i=0;i<count;i++){
            char user[B2_RESUME_LABEL261],name[B2_RESUME_LABEL261];
            // Sources are opaque: no conversion to paths, prefix inference,
            // persistence or assumption that enumeration is ordered by hand.
            if(!b2_resume_name261(get_name,sources[i],XR_INPUT_SOURCE_LOCALIZED_NAME_USER_PATH_BIT,user))return;
            int hand=b2_resume_hand261(user);if(hand<0)return;
            matches[hand]++;
            if(!b2_resume_name261(get_name,sources[i],XR_INPUT_SOURCE_LOCALIZED_NAME_INTERACTION_PROFILE_BIT,name) ||
                !b2_sony_name412(name))sony[hand]=false;
        }
        for(unsigned h=0;h<2;h++)if(matches[h]!=1)sony[h]=false;
    }
    memcpy(b2_faces412.sony,sony,sizeof(sony));
}

// Private engine087 -> engine022 presentation API; no shared structure change.
bool B2B_SonyFaces412(unsigned hand)
{
    return hand<2 && b2_faces_ready412() && b2_faces_current412() &&
        B2B_Profile316(hand)>=0 && b2_faces412.sony[hand];
}
