// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* SPDX-License-Identifier: GPL-2.0-or-later
 * Base1 wall preparation: small adaptation of the earlier step-geometry.h diagnostic.
 * Original dot/unit/rotate/view checks and finish-view conventions retained.
 * No renderer/history/runtime calls. The live projection derives the path. */
#ifndef B2_BASE1_WALL_GEOMETRY_H
#define B2_BASE1_WALL_GEOMETRY_H
#include <math.h>
#include <string.h>
#include <stdbool.h>
#define B2AW_ID "base1-ej-combined-21"
/* Original accepted host FP32 landmark, never snapped to the BSP plane. */
static const float b2aw_target[3]={-1663.96826171875f,1580.714599609375f,4.105451583862305f};
/* This helper accepts a live camera only AFTER the separately bound base1
 * body/scene setup has been qualified. It does not position the player, select
 * a save, reconstruct historical XR poses or authorise a sequence. */
static bool b2aw_finite(const float *p,unsigned n){for(unsigned i=0;i<n;i++)if(!isfinite(p[i]))return false;return true;}
static float b2aw_dot(const float *a,const float *b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}
static bool b2aw_unit(float *a){float n=sqrtf(b2aw_dot(a,a));if(!isfinite(n)||n<1e-6f)return false;for(unsigned i=0;i<3;i++)a[i]/=n;return true;}
static void b2aw_cross(const float *a,const float *b,float *c){c[0]=a[1]*b[2]-a[2]*b[1];c[1]=a[2]*b[0]-a[0]*b[2];c[2]=a[0]*b[1]-a[1]*b[0];}
static void b2aw_rotate(const float *v,const float *axis,float c,float s,float *r){
 float d=b2aw_dot(v,axis),cross[3];b2aw_cross(axis,v,cross);
 for(unsigned i=0;i<3;i++)r[i]=v[i]*c+cross[i]*s+axis[i]*d*(1.f-c);
}
static bool b2aw_view_valid(const b2_view_t *v){
 if(!b2aw_finite((const float*)v,sizeof(*v)/sizeof(float)))return false;
 if(v->V[15]!=1.f||v->invV[15]!=1.f||v->P[11]!=1.f||v->P[0]<=0.f||v->P[5]>=0.f)return false;
 for(unsigned k=0;k<3;k++){
  const float *b=&v->invV[k*4];
  if(fabsf(b2aw_dot(b,b)-1.f)>.00002f)return false;
  for(unsigned j=0;j<k;j++)if(fabsf(b2aw_dot(b,&v->invV[j*4]))>.00002f)return false;
  for(unsigned i=0;i<3;i++)if(v->V[i*4+k]!=b[i])return false;
  if(fabsf(v->V[12+k]+b2aw_dot(b,v->origin))>.002f)return false;
 }return true;
}
static void b2aw_finish_view(b2_view_t *v){
 for(unsigned k=0;k<3;k++){
  v->V[12+k]=-b2aw_dot(v->invV+4*k,v->origin);
  for(unsigned i=0;i<3;i++)v->V[4*i+k]=v->invV[4*k+i];
  v->invV[12+k]=v->origin[k];
 }
 float *f=v->invV+8,*r=v->invV,*u=v->invV+4,n=sqrtf(f[0]*f[0]+f[1]*f[1]);
 v->angles[0]=atan2f(-f[2],n)*57.29577951308232f;
 v->angles[1]=atan2f(f[1],f[0])*57.29577951308232f;
 v->angles[2]=atan2f(-r[2],u[2])*57.29577951308232f;
}
static bool b2aw_rotated(const b2_view_t *base,const float axis[3],float angle,b2_view_t *v){
 if(!isfinite(angle)||fabsf(angle)>45.f)return false;
 *v=*base;float radians=angle*.017453292519943295f;
 for(unsigned k=0;k<3;k++)b2aw_rotate(base->invV+4*k,axis,cosf(radians),sinf(radians),v->invV+4*k);
 b2aw_finish_view(v);return b2aw_view_valid(v);
}
static bool b2aw_edges(const b2_view_t *v,float e[4]){
 float ray[3];for(unsigned i=0;i<3;i++)ray[i]=b2aw_target[i]-v->origin[i];
 float x=b2aw_dot(v->invV,ray),y=b2aw_dot(v->invV+4,ray),z=b2aw_dot(v->invV+8,ray);
 if(!isfinite(z)||z<=.01f)return false;
 e[0]=((v->P[0]*x/z+v->P[8])*.5f+.5f)*1948.f;e[1]=1948.f-e[0];
 e[2]=((v->P[5]*y/z+v->P[9])*.5f+.5f)*1563.f;e[3]=1563.f-e[2];
 return b2aw_finite(e,4);
}
static bool b2aw_angle_for_edge(const b2_view_t *base,const float axis[3],float edge,float *angle){
 float lo=-45.f,hi=45.f,e[4],left,right;b2_view_t v;
 if(!b2aw_rotated(base,axis,lo,&v)||!b2aw_edges(&v,e))return false;left=e[3]-edge;
 if(!b2aw_rotated(base,axis,hi,&v)||!b2aw_edges(&v,e))return false;right=e[3]-edge;
 if(left*right>=0)return false;
 for(unsigned i=0;i<32;i++){
  float mid=(lo+hi)*.5f;if(!b2aw_rotated(base,axis,mid,&v)||!b2aw_edges(&v,e))return false;
  if((e[3]-edge)*left>0){lo=mid;left=e[3]-edge;}else hi=mid;
 }*angle=(lo+hi)*.5f;
 return b2aw_rotated(base,axis,*angle,&v)&&b2aw_edges(&v,e)&&fabsf(e[3]-edge)<.01f;
}
/* Construct an orthonormal world camera from the qualified current eye positions and
 * target, preserving each live P/invP. The diagnostic's inter-eye pitch axis is retained.
 * Bottom edge80 remains the interior aim. The old -8px solution determines
 * outward sign only; the new excursion is exactly3 degrees. Both focus views
 * must start >=64px inside and finish >=8px outside; all old path guards stay.
 * This follows the retained R61 upward-pitch/bottom-exit acquisition guide.
 * Jitter (<=1px) and actual geometry remain observations for qualification. */
static bool b2aw_make_path(const b2_view_t live[4],b2_view_t base[4],float axis[3],float endpoint[2]){
 for(unsigned eye=0;eye<4;eye++)if(!b2aw_view_valid(live+eye))return false;
 /* Current quad views must identify the same per-eye camera centres. */
 if(memcmp(live[0].origin,live[2].origin,sizeof(live[0].origin))||
    memcmp(live[1].origin,live[3].origin,sizeof(live[1].origin)))return false;
 float f[3],up[3];for(unsigned i=0;i<3;i++){
  axis[i]=live[1].origin[i]-live[0].origin[i];
  f[i]=b2aw_target[i]-.5f*(live[0].origin[i]+live[1].origin[i]);
 }
 float sep=sqrtf(b2aw_dot(axis,axis));if(sep<.1f||sep>10.f||!b2aw_unit(axis))return false;
 float along=b2aw_dot(f,axis);for(unsigned i=0;i<3;i++)f[i]-=along*axis[i];
 if(!b2aw_unit(f))return false;b2aw_cross(axis,f,up);if(!b2aw_unit(up))return false;
 for(unsigned eye=0;eye<4;eye++){
  if(!b2aw_view_valid(live+eye))return false;base[eye]=live[eye];
  memset(base[eye].V,0,sizeof(base[eye].V));memset(base[eye].invV,0,sizeof(base[eye].invV));
  base[eye].V[15]=base[eye].invV[15]=1.f;
  for(unsigned k=0;k<3;k++){base[eye].invV[k]=axis[k];base[eye].invV[4+k]=up[k];base[eye].invV[8+k]=f[k];base[eye].origin[k]=live[eye].origin[k];}
  b2aw_finish_view(base+eye);if(!b2aw_view_valid(base+eye))return false;
 }
 if(!b2aw_angle_for_edge(base+2,axis,80.f,endpoint)||!b2aw_angle_for_edge(base+2,axis,-8.f,endpoint+1))return false;
 float direction=endpoint[1]-endpoint[0];if(!isfinite(direction)||fabsf(direction)<.01f)return false;
 endpoint[1]=endpoint[0]+(direction>0.f?3.f:-3.f);
 if(fabsf(endpoint[1]-endpoint[0])<.01f||fabsf(endpoint[1]-endpoint[0])>10.f)return false;
 for(unsigned eye=2;eye<4;eye++){
  b2_view_t v;float e[4];
  if(!b2aw_rotated(base+eye,axis,endpoint[0],&v)||!b2aw_edges(&v,e)||e[3]<64.f)return false;
  if(!b2aw_rotated(base+eye,axis,endpoint[1],&v)||!b2aw_edges(&v,e)||e[3]>-8.f)return false;
 }
 /* All path samples retain the peripheral target. Both endpoints and every
  * applied frame are additionally checked, not inferred from this grid. */
 for(unsigned n=0;n<=64;n++)for(unsigned eye=0;eye<4;eye++){
  b2_view_t v;float e[4],a=endpoint[0]+(endpoint[1]-endpoint[0])*(float)n/64.f;
  if(!b2aw_rotated(base+eye,axis,a,&v)||!b2aw_edges(&v,e)||e[0]<64||e[1]<64||e[2]<64||(eye<2&&e[3]<64))return false;
 }return true;
}
#endif
