// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
static void SCR_DrawB2ResumeCue(void)
{
    b2_resume_cue261_t cue;
    if (!B2_ResumeCue(&cue) || r_config.width<=0 || r_config.height<=0) return;
    // Hide only the ready heading; preserve the tracking-wait cue.
    int first=!strcmp(cue.line[0],"Paused - release all controls");
    size_t longest=0;int lines=0;
    for (int i=first;i<3;i++) if (cue.line[i][0]) {size_t n=strlen(cue.line[i]);if(n>longest)longest=n;lines=i-first+1;}
    if (!lines) return;
    // Nominal20px stock text, shrinking to fit small surfaces without clipping.
    float scale=8.f/20.f;
    float fit=(float)(longest+4)*CHAR_WIDTH/r_config.width;
    if (fit>scale) scale=fit;
    fit=(float)(lines+4)*CHAR_HEIGHT/r_config.height;
    if (fit>scale) scale=fit;
    int x=(int)(r_config.width*scale/2),y=(int)(r_config.height*scale/2)-lines*CHAR_HEIGHT/2;
    R_B2UIRole(B2UI_MENU);R_SetScale(scale);R_ClearColor();R_SetAlphaScale(1.f);
    for (int i=0;i<lines;i++) SCR_DrawString(x,y+i*CHAR_HEIGHT,UI_CENTER,cue.line[first+i]);
    R_ClearColor();R_SetScale(1.f);R_SetAlphaScale(1.f);R_B2UIRole(B2UI_FLAT);
}
