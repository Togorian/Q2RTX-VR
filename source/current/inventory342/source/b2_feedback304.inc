// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#include "b2_feedback304.h"
static b2h_envelope_t b2_ui_pulse304[2],b2_pickup_pulse304[2];
static uint32_t b2_ui_sample304,b2_ui_generation304,b2_ui_interruption304,b2_ui_last304;
static bool b2_ui_ready304,b2_ui_rate304,b2_pickup_baseline304;
static b2_pickup304_t b2_pickup_seen304;
static bool b2_ui_pending308[2],b2_pickup_pending308[2];
static void b2_feedback_reset304(void)
{
 memset(b2_ui_pulse304,0,sizeof(b2_ui_pulse304));memset(b2_pickup_pulse304,0,sizeof(b2_pickup_pulse304));
 b2_ui_ready304=b2_ui_rate304=b2_pickup_baseline304=false;
 memset(b2_ui_pending308,0,sizeof(b2_ui_pending308));memset(b2_pickup_pending308,0,sizeof(b2_pickup_pending308));
}
static bool b2_ui_eligible304(uint32_t now)
{
 return b2_ui_ready304 && now-b2_ui_sample304<=100 && b2_haptics_master && b2_haptics_master->integer &&
  b2_controller_input && b2_controller_input->integer && B2_InputFocused556() &&
  ((cls.key_dest&KEY_MENU) || b2_inventory_feedback342()) && !(cls.key_dest&KEY_CONSOLE) && B2XR_Requested() &&
  B2XR_InteractionReady() && !B2XR_StopRequested() && b2_ui_generation304==B2XR_InputGeneration() &&
  b2_ui_interruption304==B2XR_InterruptionEpoch();
}
static void b2_ui_cancel304(void)
{
 memset(b2_ui_pending308,0,sizeof(b2_ui_pending308));
 b2_ui_ready304=false;memset(b2_ui_pulse304,0,sizeof(b2_ui_pulse304));
}
static void b2_ui_sample_set304(uint32_t now)
{
 b2_ui_sample304=now;b2_ui_generation304=B2XR_InputGeneration();
 b2_ui_interruption304=B2XR_InterruptionEpoch();b2_ui_ready304=true;
}
void B2H_UI304(unsigned event,unsigned hand)
{
 uint32_t now=Sys_Milliseconds();
 if(hand>1 || event>2 || !b2_ui_eligible304(now))return;
 if(b2_ui_rate304 && now-b2_ui_last304<100)return;
 b2_ui_last304=now;b2_ui_rate304=true;
 static const float amplitude[]={.03f,.10f,.08f};
 static const unsigned duration[]={10,25,20};
 b2h_start(&b2_ui_pulse304[hand],now,duration[event],amplitude[event],false);
 b2_ui_pending308[hand]=true;
}
static void b2_pickup_poll304(uint32_t now,bool active,unsigned hand)
{
 b2_pickup304_t value={0};bool valid=SV_B2Pickup304(&value);
 if(!active || hand>1 || !valid){memset(b2_pickup_pending308,0,sizeof(b2_pickup_pending308));b2_pickup_baseline304=false;memset(b2_pickup_pulse304,0,sizeof(b2_pickup_pulse304));return;}
 if(b2_pickup_baseline304 && value.epoch==b2_pickup_seen304.epoch && value.sequence>b2_pickup_seen304.sequence)
  for(unsigned h=0;h<2;h++){b2h_start(&b2_pickup_pulse304[h],now,20,.08f,false);b2_pickup_pending308[h]=true;}
 if(b2_pickup_baseline304 && value.epoch!=b2_pickup_seen304.epoch){
  memset(b2_pickup_pending308,0,sizeof(b2_pickup_pending308));memset(b2_pickup_pulse304,0,sizeof(b2_pickup_pulse304));}
 b2_pickup_seen304=value;b2_pickup_baseline304=true;
}
// A short event may precede the next display update by longer than its duration.
// Admit it once at that update, at most 100ms old, never across invalid eligibility.
static void b2_feedback_admit308(b2h_envelope_t *e,bool *pending,uint32_t now,bool valid)
{
 if(!valid){memset(e,0,sizeof(*e));*pending=false;return;}
 if(*pending){if((uint32_t)(now-e->start)<=100)e->start=now;else memset(e,0,sizeof(*e));*pending=false;}
}
static void b2_feedback_compose304(uint32_t now,float out[2])
{
 bool ui=b2_ui_eligible304(now);
 bool game=b2_haptics.config.active && b2_haptics.config.controller;
 if(!ui)memset(b2_ui_pulse304,0,sizeof(b2_ui_pulse304));
 if(!game)memset(b2_pickup_pulse304,0,sizeof(b2_pickup_pulse304));
 for(unsigned h=0;h<2;h++){b2_feedback_admit308(&b2_ui_pulse304[h],&b2_ui_pending308[h],now,ui);b2_feedback_admit308(&b2_pickup_pulse304[h],&b2_pickup_pending308[h],now,game);}
 for(unsigned h=0;h<2;h++)out[h]=fmaxf(out[h],fmaxf(b2h_value(&b2_ui_pulse304[h],now),b2h_value(&b2_pickup_pulse304[h],now)));
}

// Retain legacy 50ms combat/death transport when it wins the maximum.
static void b2_feedback_duration308(uint32_t now,const float out[2],uint32_t duration[2])
{
 float combat[2];b2h_compose(&b2_haptics,now,combat);
 for(unsigned h=0;h<2;h++){
  duration[h]=50;
  if(out[h]<=0 || (combat[h]>0 && combat[h]>=out[h]))continue;
  duration[h]=0;
  b2h_envelope_t *events[2]={&b2_ui_pulse304[h],&b2_pickup_pulse304[h]};
  for(unsigned i=0;i<2;i++){
   b2h_envelope_t *e=events[i];uint32_t age=now-e->start;
   if(e->duration && age<e->duration && e->amplitude>=out[h]){
    uint32_t left=e->duration-age;if(left>duration[h])duration[h]=left;
   }
  }
 }
}
