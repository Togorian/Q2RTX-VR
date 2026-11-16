// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef B2_WALL_RETURN_H
#define B2_WALL_RETURN_H
// Shared production arithmetic: compiled as both C and GLSL, with no vector ABI.
#ifdef B2RC_GLSL
#define B2RC_FN
#define B2RC_MIN min
#define B2RC_MAX max
#define B2RC_SQRT sqrt
#define B2RC_ACOS acos
#define B2RC_EXP exp
#define B2RC_POW pow
#else
#include <stdbool.h>
#include <math.h>
#define B2RC_FN static inline
#define B2RC_MIN fminf
#define B2RC_MAX fmaxf
#define B2RC_SQRT sqrtf
#define B2RC_ACOS acosf
#define B2RC_EXP expf
#define B2RC_POW powf
#endif

B2RC_FN bool B2RC_Finite(float x)
{
#ifdef B2RC_GLSL
    return !isnan(x) && !isinf(x);
#else
    return isfinite(x);
#endif
}
B2RC_FN float B2RC_Clamp(float x)
{ return B2RC_MIN(1.0f,B2RC_MAX(0.0f,x)); }
B2RC_FN float B2RC_DepthResponse(float depthQU)
{
    if(!B2RC_Finite(depthQU) || depthQU<=0.0f)return 0.0f;
    if(depthQU>=40.0f)return 1.0f;
    return B2RC_Clamp((1.0f-B2RC_EXP(-depthQU/20.0f))/(1.0f-B2RC_EXP(-2.0f)));
}
// Independent spherical half-angles: clear about return, opaque about opposite.
#define B2RC_CLEAR_HALF_DEGREES 27.5f
#define B2RC_OPAQUE_HALF_DEGREES 22.5f
#define B2RC_CLEAR_COSINE 0.8870108331782217f
#define B2RC_OPAQUE_COSINE -0.9238795325112867f
B2RC_FN float B2RC_Alpha(float cosine,float depthQU)
{
    if(!B2RC_Finite(cosine))return 0.0f;
    float d=B2RC_DepthResponse(depthQU);
    if(d<=0.0f || cosine>=B2RC_CLEAR_COSINE)return 0.0f;
    if(cosine<=B2RC_OPAQUE_COSINE)return d;
    float theta=B2RC_ACOS(B2RC_MIN(1.0f,B2RC_MAX(-1.0f,cosine)))*57.29577951308232f;
    float u=B2RC_Clamp((theta-B2RC_CLEAR_HALF_DEGREES)/(180.0f-B2RC_CLEAR_HALF_DEGREES-B2RC_OPAQUE_HALF_DEGREES));
    float v=u/(0.3f+0.7f*u);
    return B2RC_Clamp(v*v*(3.0f-2.0f*v)*B2RC_POW(d,2.25f-1.25f*u));
}
// Pixel centres use the actual output extent and accepted projection, including
// its Y sign and off-centre terms. Rotation columns are padded to four floats.
B2RC_FN float B2RC_PixelAlpha(float x,float y,float width,float height,
    const float projection[4],const float rotation[12],const float direction[3],float depthQU)
{
    if(!B2RC_Finite(x)||!B2RC_Finite(y)||!B2RC_Finite(width)||!B2RC_Finite(height)||
       width<=0.0f||height<=0.0f||x<0.0f||y<0.0f||x>=width||y>=height)return 0.0f;
    for(int i=0;i<4;i++)if(!B2RC_Finite(projection[i]))return 0.0f;
    for(int i=0;i<12;i++)if(!B2RC_Finite(rotation[i]))return 0.0f;
    for(int i=0;i<3;i++)if(!B2RC_Finite(direction[i]))return 0.0f;
    if(projection[0]==0.0f||projection[1]==0.0f)return 0.0f;
    float cx=(2.0f*(x+0.5f)/width-1.0f-projection[2])/projection[0];
    float cy=(2.0f*(y+0.5f)/height-1.0f-projection[3])/projection[1];
    float qx=rotation[0]*cx+rotation[4]*cy+rotation[8];
    float qy=rotation[1]*cx+rotation[5]*cy+rotation[9];
    float qz=rotation[2]*cx+rotation[6]*cy+rotation[10];
    float q2=qx*qx+qy*qy+qz*qz;
    float r2=direction[0]*direction[0]+direction[1]*direction[1]+direction[2]*direction[2];
    if(!B2RC_Finite(q2)||!B2RC_Finite(r2)||q2<1e-12f||r2<1e-12f)return 0.0f;
    float cosine=(qx*direction[0]+qy*direction[1]+qz*direction[2])/
        (B2RC_SQRT(q2)*B2RC_SQRT(r2));
    return B2RC_Alpha(cosine,depthQU);
}
B2RC_FN float B2RC_Encode(float linear)
{
    if(!B2RC_Finite(linear))return 0.0f;
    float x=B2RC_Clamp(linear);
    return x<=0.0031308f?12.92f*x:1.055f*B2RC_POW(x,1.0f/2.4f)-0.055f;
}
B2RC_FN float B2RC_Decode(float display)
{
    float x=B2RC_Clamp(display);
    return x<=0.04045f?x/12.92f:B2RC_POW((x+0.055f)/1.055f,2.4f);
}
B2RC_FN float B2RC_Channel(float linear,float cyan,float alpha)
{
    if(!B2RC_Finite(alpha)||alpha<=0.0f)return linear;
    if(alpha>=1.0f)return cyan;
    return B2RC_Decode((1.0f-alpha)*B2RC_Encode(linear)+alpha*cyan);
}
#ifndef B2RC_GLSL
B2RC_FN bool B2RC_Direction(const float anchor[3],const float head[3],float direction[3])
{
    float n=0.0f;
    for(int i=0;i<3;i++){
        direction[i]=0.0f;
        if(!isfinite(anchor[i])||!isfinite(head[i]))return false;
        float d=anchor[i]-head[i];n+=d*d;
    }
    if(!isfinite(n)||n<1e-12f)return false;
    n=sqrtf(n);
    for(int i=0;i<3;i++)direction[i]=(anchor[i]-head[i])/n;
    return true;
}
// CM_ClipBoxToBrush backs the ordinary point entry up by DIST_EPSILON.
// World endpos and rotated normal are valid for transformed entity traces;
// their plane.dist is model-local, so it is deliberately not used here.
B2RC_FN float B2RC_Penetration(float fraction,bool startsolid,bool allsolid,
    const float end[3],const float head[3],const float normal[3])
{
    if(!isfinite(fraction)||fraction<=0.0f||fraction>=1.0f||startsolid||allsolid)return 0.0f;
    float n=0.0f,d=0.0f;
    for(int i=0;i<3;i++){
        if(!isfinite(end[i])||!isfinite(head[i])||!isfinite(normal[i]))return 0.0f;
        n+=normal[i]*normal[i];d+=(end[i]-head[i])*normal[i];
    }
    if(!isfinite(n)||!isfinite(d)||n<1e-12f)return 0.0f;
    float depth=(d-0.03125f)/sqrtf(n);
    return isfinite(depth)?fmaxf(0.0f,depth):0.0f;
}
#endif
#endif
