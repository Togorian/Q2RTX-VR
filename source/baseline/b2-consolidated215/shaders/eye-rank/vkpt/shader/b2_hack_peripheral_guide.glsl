// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Design/direction by Togorian; implementation with Codex assistance.
// Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
// Modified 11 November 2026: provenance/notice clarification only. See PROVENANCE.md.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* Private HF experiment; GPL-2.0-or-later, like asvgf_gradient_reproject.comp.
 * The donor is a CURRENT-frame brightness guide, never previous replay data. */
layout(set = 3, binding = 0) uniform usampler2D b2_guide_hf;
layout(set = 3, binding = 1) uniform usampler2D b2_guide_spec;
layout(set = 3, binding = 2) uniform usampler2D b2_guide_primitive;
layout(set = 3, binding = 3) uniform sampler2D b2_guide_depth;
layout(set = 3, binding = 4) uniform usampler2D b2_guide_geo_normal;
layout(set = 3, binding = 5) uniform usampler2D b2_guide_gradient;
layout(push_constant) uniform B2GuidePush {
    mat4 vp;
    vec4 origin;
    vec4 extent_jitter;
    uvec4 flags;
} b2_guide;

bool b2_guide_focus_mode()
{
    return global_ubo.padding.x == 9.0 && b2_guide.flags.x != 0u;
}

bool b2_guide_finite(float value)
{
    return !isnan(value) && !isinf(value);
}

// This is a conservative same-surface footprint guide, not exact cross-view replay.
float b2_guide_score(ivec2 p, ivec2 pp)
{
    if(b2_guide.flags.y == 0u)
        return -1.0;
    uvec2 primitive = texelFetch(TEX_PT_VISBUF_PRIM_A, p, 0).xy;
    uvec2 previous_primitive = texelFetch(TEX_PT_VISBUF_PRIM_B, pp, 0).xy;
    if(primitive.x != ~0u || any(notEqual(primitive, previous_primitive)) ||
        primitive.y >= uint(global_ubo.num_static_primitives))
        return -1.0;
    vec4 position_material = imageLoad(IMG_PT_SHADING_POSITION, p);
    uint material = floatBitsToUint(position_material.w);
    uint fields = uint(imageLoad(IMG_PT_VIEW_DIRECTION, p).w);
    if((fields & CHECKERBOARD_FLAG_FIELD_MASK) != CHECKERBOARD_FLAG_PRIMARY ||
        (fields & CHECKERBOARD_FLAG_WEAPON) != 0u ||
        (material & MATERIAL_KIND_MASK) != MATERIAL_KIND_REGULAR)
        return -1.0;
    Triangle triangle = load_and_transform_triangle(-1, VERTEX_BUFFER_WORLD, primitive.y);
    if(triangle.alpha != 1.0 ||
        (triangle.material_id & MATERIAL_KIND_MASK) != MATERIAL_KIND_REGULAR)
        return -1.0;
    float focus_depth = texelFetch(TEX_PT_VIEW_DEPTH_A, p, 0).x;
    float previous_depth = texelFetch(TEX_PT_VIEW_DEPTH_B, pp, 0).x;
    if(!(focus_depth > 0.0) || !(previous_depth > 0.0))
        return -1.0;
    vec4 clip = b2_guide.vp * vec4(position_material.xyz, 1.0);
    if(any(isnan(clip)) || any(isinf(clip)) || !(clip.w > 0.0))
        return -1.0;
    vec2 uv = clip.xy / clip.w * 0.5 + 0.5;
    if(any(lessThanEqual(uv, vec2(0))) || any(greaterThanEqual(uv, vec2(1))))
        return -1.0;
    ivec2 extent = ivec2(b2_guide.extent_jitter.xy);
    // Focus jitter is already in its actual world hit; subtract only donor jitter.
    ivec2 donor_pixel = ivec2(floor(uv * vec2(extent) - b2_guide.extent_jitter.zw));
    if(any(lessThan(donor_pixel, ivec2(0))) || any(greaterThanEqual(donor_pixel, extent)))
        return -1.0;
    bool even_field = (donor_pixel.x & 1) == (donor_pixel.y & 1);
    ivec2 q = ivec2(donor_pixel.x / 2 + (even_field ? 0 : extent.x / 2), donor_pixel.y);
    if(any(notEqual(texelFetch(b2_guide_primitive, q, 0).xy, primitive)))
        return -1.0;
    uint marker = texelFetch(b2_guide_gradient, q / GRAD_DWN, 0).x;
    ivec2 offset = ivec2(marker >> (STRATUM_OFFSET_SHIFT * 0),
        marker >> (STRATUM_OFFSET_SHIFT * 1)) & STRATUM_OFFSET_MASK;
    // A donor gradient pixel shades an old patched position but retains current visibility.
    if(marker != 0u && all(equal(offset, q % GRAD_DWN)))
        return -1.0;
    float donor_depth = texelFetch(b2_guide_depth, q, 0).x;
    float projected_depth = length(position_material.xyz - b2_guide.origin.xyz);
    if(!(donor_depth > 0.0) || !b2_guide_finite(donor_depth) ||
        !(projected_depth > 0.0) || !b2_guide_finite(projected_depth) ||
        abs(donor_depth - projected_depth) / projected_depth >= 0.1)
        return -1.0;
    vec3 donor_normal = decode_normal(texelFetch(b2_guide_geo_normal, q, 0).x);
    vec3 focus_normal = decode_normal(texelFetch(TEX_PT_GEO_NORMAL_A, p, 0).x);
    // Reuse existing broad depth/normal thresholds; they are not an exact-point proof.
    if(!(dot(donor_normal, focus_normal) > 0.9))
        return -1.0;
    float score = luminance(unpackRGBE(texelFetch(b2_guide_hf, q, 0).x)) +
        luminance(unpackRGBE(texelFetch(b2_guide_spec, q, 0).x));
    return b2_guide_finite(score) && score >= 0.0 ? score : -1.0;
}
