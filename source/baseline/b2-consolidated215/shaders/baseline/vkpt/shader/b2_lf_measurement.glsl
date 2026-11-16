// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// R60: actual HF operands in the existing mutually exclusive diagnostic tail.
// Adapted from exact R51/R58 B2 observation helpers. No renderer stores here.
#define B2M_MAGIC 0xB2604C46u
#define B2M_BASE 24u
#define B2M_STRIDE 86u
#define B2M_SAMPLES 3u
#define B2M_NAN 0x7fc00000u
bool b2m_armed()
{
    return readback.b1_tone_witness[0] == B2M_MAGIC &&
        (readback.b1_tone_witness[3] & 255u) == 0u &&
        readback.b1_tone_witness[9] == 1u &&
        readback.b1_tone_witness[10] == B2M_SAMPLES &&
        readback.b1_tone_witness[2] == uint(global_ubo.current_frame_idx);
}
int b2m_find(ivec2 p, bool output_pixel)
{
    if(!b2m_armed()) return -1;
    uint origin = output_pixel ? 17u : 11u;
    for(uint i = 0u; i < B2M_SAMPLES; i++) {
        if((readback.b1_tone_witness[3] & (1u << (8u+i))) != 0u) continue;
        if(all(equal(p, ivec2(readback.b1_tone_witness[origin+i*2u],
            readback.b1_tone_witness[origin+i*2u+1u]))))
            return int(B2M_BASE+i*B2M_STRIDE);
    }
    return -1;
}
void b2m_u(int base, uint word, uint value)
{
    if(base < int(B2M_BASE) || base > int(B2M_BASE+2u*B2M_STRIDE) || word >= 140u) return;
    if(word < 86u) readback.b1_tone_witness[uint(base)+word] = value;
    else readback.b2_lighting_witness[(uint(base)-B2M_BASE)/B2M_STRIDE*54u+word-86u] = value;
}
void b2m_f(int base, uint word, float value) { b2m_u(base,word,floatBitsToUint(value)); }
void b2m_v(int base, uint word, vec3 value)
{
    for(uint k=0u;k<3u;k++) b2m_f(base,word+k,value[k]);
}
void b2m_begin(int base, ivec2 p, int flags, float depth, vec3 normal,
    vec4 motion, vec2 previous)
{
    if(base < 0) return;
    // Temporal owns1..85 and138..139; other stages own their separate flags/data.
    for(uint k=18u;k<86u;k++) b2m_u(base,k,B2M_NAN);
    for(uint k=50u;k<62u;k++) b2m_u(base,k,0xffffffffu);
    b2m_u(base,62u,0u);
    for(uint i=0u;i<4u;i++) {
        b2m_u(base,18u+8u*i,0x80000000u);
        b2m_u(base,19u+8u*i,0x80000000u);
        b2m_u(base,25u+8u*i,0u);
    }
    b2m_u(base,1u,uint(flags));
    uvec2 primitive=texelFetch(TEX_PT_VISBUF_PRIM_A,p,0).xy;
    vec2 bary=texelFetch(TEX_PT_VISBUF_BARY_A,p,0).xy;
    b2m_u(base,2u,primitive.x); b2m_u(base,3u,primitive.y);
    b2m_u(base,4u,texelFetch(TEX_PT_GEO_NORMAL_A,p,0).x);
    b2m_f(base,5u,bary.x); b2m_f(base,6u,bary.y); b2m_f(base,7u,depth);
    b2m_v(base,8u,normal);
    b2m_u(base,11u,texelFetch(TEX_PT_NORMAL_A,p,0).x);
    for(uint k=0u;k<4u;k++) b2m_f(base,12u+k,motion[k]);
    b2m_f(base,16u,previous.x); b2m_f(base,17u,previous.y);
    b2m_f(base,138u,global_ubo.flt_temporal_hf);
    b2m_f(base,139u,global_ubo.flt_antilag_hf);
}
// Tap flags: candidate1,in-bounds2,geometry-pass4,production-HF-read8,
// positive-weight16,supplementary-previous-ID-read32. Zero weight is data.
void b2m_tap_candidate(int base, int tap, ivec2 p, float weight)
{
    if(base < 0) return;
    uint w=18u+uint(tap)*8u;
    b2m_u(base,w,uint(p.x)); b2m_u(base,w+1u,uint(p.y));
    b2m_f(base,w+2u,weight); b2m_u(base,w+7u,1u);
}
void b2m_tap_surface(int base, int tap, ivec2 p, float depth_error,
    float normal_dot, float geo_dot)
{
    if(base < 0) return;
    uint w=18u+uint(tap)*8u;
    b2m_u(base,w,uint(p.x)); b2m_u(base,w+1u,uint(p.y));
    b2m_f(base,w+4u,depth_error); b2m_f(base,w+5u,normal_dot);
    b2m_f(base,w+6u,geo_dot);
    // This extra read is a diagnostic witness, never a production predicate.
    uvec2 previous_id=texelFetch(TEX_PT_VISBUF_PRIM_B,p,0).xy;
    b2m_u(base,54u+uint(tap)*2u,previous_id.x);
    b2m_u(base,55u+uint(tap)*2u,previous_id.y);
    b2m_u(base,w+7u,1u|2u|32u);
}
void b2m_tap_loaded(int base, int tap, float weight, uint packed_hf)
{
    if(base < 0) return;
    uint w=18u+uint(tap)*8u;
    b2m_f(base,w+3u,weight); b2m_u(base,50u+uint(tap),packed_hf);
    b2m_u(base,w+7u,1u|2u|4u|8u|32u|(weight>0.0?16u:0u));
}
void b2m_spatial_begin(int base)
{
    if(base < 0) return;
    for(uint k=87u;k<114u;k++) b2m_u(base,k,B2M_NAN);
    for(uint k=106u;k<114u;k++) b2m_u(base,k,0xffffffffu);
    b2m_u(base,93u,0xffffffffu); b2m_u(base,86u,0u);
}
