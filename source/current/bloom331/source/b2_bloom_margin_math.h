// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <string.h>

/* This is the retained aggregate allocation ceiling, including Vulkan alignment.
 * One packed HDR texel (two uint32 words) is eight bytes in all four buffers. */
#define BM_BYTE_CAP (UINT64_C(128) * 1024 * 1024)
#define BM_RADIUS 400u
typedef struct { uint32_t w,h,px,py,ew,eh; uint64_t bytes; } bm_layout_t;
static int bm_layout(uint32_t width,uint32_t height,int nx,int ny,bm_layout_t *out)
{
    if (!out || width<4 || height<4 || (uint64_t)width*height >= BM_BYTE_CAP/8 ||
        nx<4 || ny<4 || nx>BM_RADIUS || ny>BM_RADIUS) return 0;
    out->w=width/4; out->h=height/4;
    /* The paired bilinear taps reach the largest even index <= num_samples.
     * An extra cell covers interpolation and the partial 4x4 boundary. */
    out->px=(uint32_t)(nx/2)*2+1; out->py=(uint32_t)(ny/2)*2+1;
    out->ew=out->w+2*out->px; out->eh=out->h+2*out->py;
    out->bytes=(uint64_t)out->ew*out->eh*8;
    /* The source pixel bound above also bounds ew/eh and cell*4 in the
     * signed shader ABI. Apply the scratch limit before publishing the plan. */
    return out->bytes<=BM_BYTE_CAP;
}
/* Worst-case paired kernels are reserved once per resource lifetime. */
static const char *bm_plan(uint32_t width,uint32_t height,uint64_t range,
    uint32_t groups_x,uint32_t groups_y,uint64_t *source,uint64_t *scratch)
{
    bm_layout_t worst;
    if(!bm_layout(width,height,BM_RADIUS,BM_RADIUS,&worst)) return "extent";
    uint64_t src=((uint64_t)width*height+1)*8;
    if(src>range || worst.bytes>range) return "descriptor-range";
    if(src+worst.bytes>BM_BYTE_CAP/2) return "four-buffer-budget";
    uint32_t max_x=width>worst.ew?width:worst.ew;
    uint32_t max_y=height>worst.eh?height:worst.eh;
    if((max_x+15)/16>groups_x || (max_y+15)/16>groups_y) return "dispatch";
    *source=src;
    *scratch=worst.bytes;
    return NULL;
}
static int bm_projection(const float *p)
{
    for(int i=0;i<16;i++) if(!isfinite(p[i])) return 0;
    return p[0]>0 && p[5]<0 && p[11]==1 && p[15]==0 &&
        p[1]==0 && p[2]==0 && p[3]==0 && p[4]==0 && p[6]==0 && p[7]==0 && p[12]==0 && p[13]==0;
}
static int bm_rotation(const float *v)
{
    for(int i=0;i<16;i++) if(!isfinite(v[i])) return 0;
    for(int a=0;a<3;a++) for(int b=0;b<3;b++) {
        double dot=0; for(int k=0;k<3;k++) dot+=(double)v[a*4+k]*v[b*4+k];
        /* Three products/sums of a binary32 quaternion-derived rotation. */
        if(fabs(dot-(a==b?1.:0.))>32.*FLT_EPSILON) return 0;
    }
    return v[3]==0 && v[7]==0 && v[11]==0 && v[15]==1;
}
/* Rows of a direction homography: focus NDC -> peripheral clip x,y,w.
 * The caller must first prove identical optical origins. */
static int bm_homography(const float *fp,const float *finv,const float *pp,const float *pv,float rows[12])
{
    if(!bm_projection(fp)||!bm_projection(pp)||!bm_rotation(finv)||!bm_rotation(pv)) return 0;
    double r[3][3],d[3][3]={{1./fp[0],0,-(double)fp[8]/fp[0]},
        {0,1./fp[5],-(double)fp[9]/fp[5]},{0,0,1}};
    for(int a=0;a<3;a++) for(int b=0;b<3;b++) {
        r[a][b]=0; for(int k=0;k<3;k++) r[a][b]+=(double)pv[k*4+a]*finv[b*4+k];
    }
    memset(rows,0,12*sizeof(float));
    for(int c=0;c<3;c++) {
        double v[3]={0}; for(int a=0;a<3;a++) for(int k=0;k<3;k++) v[a]+=r[a][k]*d[k][c];
        rows[c]=(float)(pp[0]*v[0]+pp[8]*v[2]);
        rows[4+c]=(float)(pp[5]*v[1]+pp[9]*v[2]); rows[8+c]=(float)v[2];
    }
    for(int i=0;i<12;i++) if(!isfinite(rows[i])) return 0;
    return 1;
}
static int bm_support_valid(const float h[12],const bm_layout_t *l,uint32_t w,uint32_t ht)
{
    for(int a=0;a<2;a++) for(int b=0;b<2;b++) {
        double x=2.*((a?(double)(l->w+l->px)*4-.5:-(double)l->px*4+.5)/w)-1;
        double y=2.*((b?(double)(l->h+l->py)*4-.5:-(double)l->py*4+.5)/ht)-1;
        double z=h[8]*x+h[9]*y+h[10];
        if(!isfinite(z)||z<=0) return 0;
        for(int r=0;r<2;r++) {
            double p=(h[r*4]*x+h[r*4+1]*y+h[r*4+2])/z;
            if(!isfinite(p)||fabs(p)>1.e6) return 0;
        }
    }
    return 1;
}
typedef struct {
    uint32_t valid,eye,logical,generation,render,resources,width,height;
    int64_t predicted;
    float origin[3],position[3],P[16],V[16],sigma,intensity,exposure;
    uint64_t source_image;
} bm_binding_t;
static int bm_match(const bm_binding_t *b,uint32_t eye,uint32_t logical,uint32_t generation,
    uint32_t render,uint32_t resources,uint32_t width,uint32_t height,int64_t predicted,
    const float origin[3],const float position[3],float sigma,float intensity,float exposure,uint64_t image)
{
    if(!b->valid||b->eye!=eye||b->logical!=logical||b->generation!=generation||
        (uint32_t)(render-b->render)!=2||b->resources!=resources||b->width!=width||b->height!=height||
        b->predicted!=predicted||b->source_image!=image||b->sigma!=sigma||b->intensity!=intensity||
        b->exposure!=exposure||!isfinite(exposure)||!isfinite(sigma)||!isfinite(intensity)) return 0;
    for(int i=0;i<3;i++) if(!isfinite(origin[i])||!isfinite(position[i])||
        b->origin[i]!=origin[i]||b->position[i]!=position[i]) return 0;
    return 1;
}
