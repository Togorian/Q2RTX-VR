// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_WALL_QUERY_H
#define B2_WALL_QUERY_H
#include <float.h>
#include <math.h>
#include <stdint.h>
// Presentation-only first obstruction along one accepted body-to-camera segment.
// No collision geometry, entity state, movement or physical camera is changed.
typedef struct {
    double enter;
    double depth;
    bool valid;
} b2_wall_query_t;
static inline bool B2WQ_VectorFinite(const vec3_t v)
{ return isfinite(v[0]) && isfinite(v[1]) && isfinite(v[2]); }
void CM_B2WallObstruction(b2_wall_query_t *query,const vec3_t start,const vec3_t end,
    mnode_t *headnode,int mask,const vec3_t origin,const vec3_t angles);
float CL_B2WallDepth(const vec3_t start,const vec3_t end);
#endif
