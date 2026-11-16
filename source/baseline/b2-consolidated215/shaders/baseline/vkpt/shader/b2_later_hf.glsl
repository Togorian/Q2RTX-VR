// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// R64: append-only wall-probe observations; no production image stores.
// Small direct adaptation of the accepted B2 R60 observation helpers.
#define B2H_WORDS 320u
int b2h_base = -1;
uint b2h_flags = 0u;
uint b2h_n = 0u;
void b2h_u(uint w, uint v) {
    if(b2h_base >= 0 && w < B2H_WORDS)
        readback.b2_hf_later[uint(b2h_base)+w] = v;
}
void b2h_f(uint w, float v) { b2h_u(w,floatBitsToUint(v)); }
void b2h_v(uint w, vec3 v) { for(uint k=0u;k<3u;k++) b2h_f(w+k,v[k]); }
void b2h_or(uint w, uint v) {
    if(b2h_base >= 0 && w < B2H_WORDS)
        readback.b2_hf_later[uint(b2h_base)+w] |= v;
}
void b2h_begin(ivec2 p, uint packed_hf, vec3 colour, vec2 moments) {
    if(spec_iteration < 1 || spec_iteration > 3 || !b2m_armed() ||
       (readback.b1_tone_witness[1]!=0u && readback.b1_tone_witness[1]!=2u) ||
       (readback.b1_tone_witness[3] & (1u<<9u))!=0u ||
       any(notEqual(p,ivec2(readback.b1_tone_witness[13],readback.b1_tone_witness[14])))) return;
    b2h_base = (int(spec_iteration)-1)*int(B2H_WORDS);
    for(uint k=0u;k<B2H_WORDS;k++) b2h_u(k,B2M_NAN);
    b2h_flags=1u|(global_ubo.flt_atrous_hf<=spec_iteration?8u:0u);
    b2h_u(0u,0xB2644846u); b2h_u(1u,1u); b2h_u(2u,uint(spec_iteration));
    b2h_u(3u,b2h_flags); b2h_u(4u,uint(global_ubo.current_frame_idx));
    b2h_u(5u,readback.b1_tone_witness[4]); b2h_u(6u,readback.b1_tone_witness[1]);
    b2h_u(7u,1u); b2h_u(8u,uint(p.x)); b2h_u(9u,uint(p.y));
    b2h_u(10u,1u<<spec_iteration);
    b2h_u(11u,uint(global_ubo.flt_atrous_hf)); b2h_u(12u,uint(global_ubo.flt_atrous_spec));
    b2h_f(13u,global_ubo.flt_atrous_lum_hf); b2h_f(14u,global_ubo.flt_atrous_normal_hf);
    b2h_f(15u,global_ubo.flt_atrous_depth); b2h_f(16u,global_ubo.flt_atrous_normal_spec);
    b2h_f(17u,global_ubo.flt_min_alpha_color_hf); b2h_f(18u,global_ubo.flt_min_alpha_moments_hf);
    b2h_u(19u,readback.b1_tone_witness[5]); b2h_u(20u,packed_hf);
    b2h_v(21u,colour); b2h_f(24u,moments.x); b2h_f(25u,moments.y);
    for(uint k=42u;k<48u;k++) b2h_u(k,0u);
    b2h_u(59u,0xffffffffu);
    // Diagnostic-only correspondence reads, never HF admission predicates.
    uvec2 primitive=texelFetch(TEX_PT_VISBUF_PRIM_A,p,0).xy;
    vec2 bary=texelFetch(TEX_PT_VISBUF_BARY_A,p,0).xy;
    b2h_u(63u,primitive.x); b2h_u(64u,primitive.y);
    b2h_f(65u,bary.x); b2h_f(66u,bary.y);
    b2h_u(67u,texelFetch(TEX_PT_GEO_NORMAL_A,p,0).x); b2h_flags|=256u;
    b2h_u(70u,uint(global_ubo.current_gpu_slice_width)); b2h_u(71u,uint(global_ubo.height));
    // Words72..79 retain NaN until the corresponding neighbour is read.
    for(uint n=0u;n<8u;n++) {
        uint w=80u+n*30u;
        for(uint k=0u;k<4u;k++) b2h_u(w+k,0x80000000u);
        b2h_u(w+4u,0u); b2h_u(w+5u,0xffffffffu);
        b2h_u(w+24u,0xffffffffu); b2h_u(w+25u,0xffffffffu); b2h_u(w+28u,0xffffffffu);
    }
}
void b2h_candidate(int xx,int yy,ivec2 p,ivec2 extent,int left,int right) {
    if(b2h_base<0) return;
    b2h_n=uint((yy+1)*3+xx+1); if(b2h_n>4u)b2h_n--;
    uint w=80u+b2h_n*30u, bit=1u<<b2h_n;
    b2h_u(w,uint(xx)); b2h_u(w+1u,uint(yy)); b2h_u(w+2u,uint(p.x)); b2h_u(w+3u,uint(p.y));
    bool outside=any(lessThan(p,ivec2(0)))||any(greaterThanEqual(p,extent));
    bool field=p.x<left||p.x>=right;
    b2h_u(w+4u,1u|(outside?4u:0u)|(field?8u:0u));
    b2h_or(42u,bit); if(outside)b2h_or(43u,bit); if(field)b2h_or(44u,bit);
}
void b2h_neighbour(ivec2 p,uint packed_hf,vec3 colour,vec2 moments,vec3 normal,
    float depth,float dist_z,float lum,float dist_l,float ndot,float weight,float roughness) {
    if(b2h_base<0)return;
    uint w=80u+b2h_n*30u, bit=1u<<b2h_n;
    b2h_u(w+4u,1u|2u|128u|(weight==0?16u:0u)|(weight>0?32u:0u)|((b2h_flags&32u)!=0u?64u:0u));
    b2h_u(w+5u,packed_hf); b2h_v(w+6u,colour); b2h_f(w+9u,moments.x); b2h_f(w+10u,moments.y);
    b2h_v(w+11u,normal); b2h_f(w+14u,depth); b2h_f(w+15u,dist_z);
    b2h_f(w+18u,lum); b2h_f(w+19u,dist_l); b2h_f(w+21u,ndot); b2h_f(w+23u,weight);
    uvec2 primitive=texelFetch(TEX_PT_VISBUF_PRIM_A,p,0).xy;
    vec2 bary=texelFetch(TEX_PT_VISBUF_BARY_A,p,0).xy;
    b2h_u(w+24u,primitive.x); b2h_u(w+25u,primitive.y); b2h_f(w+26u,bary.x); b2h_f(w+27u,bary.y);
    b2h_u(w+28u,texelFetch(TEX_PT_GEO_NORMAL_A,p,0).x); b2h_f(w+29u,roughness);
    // Diagnostic-only path flags: distinguish opaque primary from secondary/weapon.
    b2h_f(72u+b2h_n,texelFetch(TEX_PT_VIEW_DIRECTION,p,0).a);
    b2h_or(45u,bit); if(weight==0)b2h_or(46u,bit); if(weight>0)b2h_or(47u,bit);
}
