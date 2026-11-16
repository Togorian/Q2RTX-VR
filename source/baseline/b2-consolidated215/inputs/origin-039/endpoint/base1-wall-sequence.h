// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
/* E/J/J/E blocks retain natural history. Settling is recorded, never a
 * convergence or independent-replication claim. */
#define B2AW_BLOCKS 4u
#define B2AW_BLOCK_REPEATS 2u
#define B2AW_REPEATS (B2AW_BLOCKS*B2AW_BLOCK_REPEATS)
#define B2AW_INITIAL_MS 30000u
#define B2AW_LEG_MS 500u
#define B2AW_HOLD_MS 750u
#define B2AW_RETURN_HOLD_MS 4250u
#define B2AW_CYCLE_MS (2u*B2AW_LEG_MS+B2AW_HOLD_MS+B2AW_RETURN_HOLD_MS)
#define B2AW_BLOCK_MS (B2AW_INITIAL_MS+B2AW_BLOCK_REPEATS*B2AW_CYCLE_MS)
#define B2AW_SEQUENCE_MS (B2AW_BLOCKS*B2AW_BLOCK_MS)
#define B2AW_STARTUP_MS 174000u
#define B2AW_WORK_MS (B2AW_STARTUP_MS+1000u+B2AW_SEQUENCE_MS)
static float b2aw_colour(unsigned block){return .02f;}
static unsigned b2aw_mode(unsigned block){return block==0u||block==3u?5u:10u;}
/* Absolute time; missing frames/lead-in samples are never manufactured.
 * Phase0 lead-in,1 outward,2 outside hold,3 return,4 starting-view hold. */
static bool b2aw_schedule(unsigned elapsed,unsigned *block,unsigned *iteration,unsigned *phase,float *fraction){
 if(elapsed>B2AW_SEQUENCE_MS)return false;
 if(elapsed==B2AW_SEQUENCE_MS){*block=B2AW_BLOCKS-1u;*iteration=B2AW_REPEATS;*phase=4;*fraction=0;return true;}
 *block=elapsed/B2AW_BLOCK_MS;unsigned t=elapsed%B2AW_BLOCK_MS;
 *iteration=*block*B2AW_BLOCK_REPEATS;
 if(t<B2AW_INITIAL_MS){*phase=0;*fraction=0;return true;}
 t-=B2AW_INITIAL_MS;*iteration+=t/B2AW_CYCLE_MS+1u;t%=B2AW_CYCLE_MS;
 if(t<B2AW_LEG_MS){*phase=1;*fraction=(float)t/B2AW_LEG_MS;}
 else if((t-=B2AW_LEG_MS)<B2AW_HOLD_MS){*phase=2;*fraction=1;}
 else if((t-=B2AW_HOLD_MS)<B2AW_LEG_MS){*phase=3;*fraction=1.f-(float)t/B2AW_LEG_MS;}
 else{*phase=4;*fraction=0;}return true;
}
