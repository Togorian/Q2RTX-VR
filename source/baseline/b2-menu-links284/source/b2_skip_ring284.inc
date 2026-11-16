// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
// Local coverage raster, using the ordinary blended UI fill primitive.
// Four alpha levels merge into horizontal runs. At most 96 rows, with
// four monotone radial edges and two arc transitions: <=20 runs per row.
static void SCR_B2SkipRing(int cx, int cy, int radius, int thickness,
    float progress, float scale)
{
    const float r = min(radius, 44);
    const float step = radius / r;
    const float original_half = thickness / (2.f * step);
    const float half = 2.f * original_half;
    const float centre_radius = r - original_half;
    const int extent = (int)ceilf(r + original_half + .5f);
    const float ring_scale = scale / step;
    const int x0 = (int)floorf(cx / step);
    const int y0 = (int)floorf(cy / step);
    const float fx = cx / step - x0, fy = cy / step - y0;
    R_SetScale(ring_scale);
    for (int y = -extent; y < extent; y++) {
        uint32_t previous = 0;
        int start = -extent;
        for (int x = -extent; x <= extent; x++) {
            uint32_t colour = 0;
            if (x < extent) {
                const float dx = x + .5f - fx, dy = y + .5f - fy;
                float coverage = half + .5f - fabsf(sqrtf(dx*dx + dy*dy) - centre_radius);
                int level = (int)floorf(Q_clipf(coverage, 0.f, 1.f) * 4.f + .5f);
                if (level) {
                    float angle = atan2f(dx, -dy);
                    if (angle < 0) angle += 2.f*M_PI;
                    int shade = angle < progress*(2.f*M_PI) ? 235 : 75;
                    colour = MakeColor(shade, shade, shade, (level*255+2)/4);
                }
            }
            if (colour != previous) {
                if (previous) R_DrawFill32(x0+start,y0+y,x-start,1,previous);
                start = x;
                previous = colour;
            }
        }
    }
    R_SetScale(scale);
}
