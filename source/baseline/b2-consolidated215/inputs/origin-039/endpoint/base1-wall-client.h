// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
/* SPDX-License-Identifier: GPL-2.0-or-later
 * GPL diagnostic adaptation: client pause/head-view integration.
 * Coherent world hold and sampled client state are project diagnostic stages;
 * the GPL-2.0-or-later notice above is retained. See root PROVENANCE.md.
 * Included after existing client activation/pause/head-view implementations.
 * Coherent world hold owns only its pause reason, never renderer history.
 */
#include "base1-wall-geometry.h"
#include "base1-wall-sequence.h"
static uint32_t b2aw_init_ms;
static cvar_t *b2_wall_recipe;
static struct {
 bool started, armed, finished, held, pending, has_previous, controls_set;
 uint32_t start_ms, sequence_ms, frame_ms, world_hash, serial;
 unsigned generation, epoch, last_logical, iteration, phase, block, applied_block;
 int client_frame, servercount, areabytes;
 float axis[3], endpoint[2], requested, input_angles[3];
 b2_view_t base[4], previous[4];
 player_state_t player;refdef_t scene;
} b2aw;
static bool b2aw_enabled(void){return b2_wall_recipe&&!strcmp(b2_wall_recipe->string,"1");}
static bool b2aw_owned_hold(void){return b2aw.held;}
static uint32_t b2aw_hash(uint32_t h,const void *ptr,size_t n){const byte *p=ptr;for(size_t i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static uint32_t b2aw_world_hash(void){
 const refdef_t *s=&b2_client.scene;uint32_t h=b2aw_hash(2166136261u,s,sizeof(*s));
 h=b2aw_hash(h,s->entities,sizeof(*s->entities)*(size_t)s->num_entities);
 h=b2aw_hash(h,s->particles,sizeof(*s->particles)*(size_t)s->num_particles);
 h=b2aw_hash(h,s->dlights,sizeof(*s->dlights)*(size_t)s->num_dlights);
 h=b2aw_hash(h,s->lightstyles,sizeof(*s->lightstyles)*MAX_LIGHTSTYLES);
 return b2aw_hash(h,s->areabits,(size_t)b2aw.areabytes);
}
static bool b2aw_input_clear(void){
 const unsigned keys[]={'w','s','a','d','c','r',K_SPACE,K_SHIFT,K_MOUSE1,K_MOUSE2,K_LEFTARROW,K_RIGHTARROW,K_HOME};
 for(unsigned i=0;i<sizeof(keys)/sizeof(keys[0]);i++)if(Key_IsDown(keys[i]))return false;
 return true;
}
/* Called by the existing terminal-stop path. Do not apply_pause here: terminal
 * stop owns the final pause, including manual/menu/interruption intent. */
static void b2aw_abort(void){
 if(!b2aw.started||b2aw.finished)return;
 Com_Printf("B2_WALL_END recipe=%s success=0 elapsed_ms=%u held=%u no_auto_resume=1\n",B2AW_ID,(uint32_t)(Sys_Milliseconds()-b2aw.start_ms),b2aw.held);
 b2aw.finished=true;b2aw.held=b2aw.pending=false;
 Cbuf_AddText(&cmd_buffer,"quit\n");
}
static void b2aw_fail(void){b2aw_abort();b2_stop_at(__func__,__LINE__);}
/* Existing Cvars only; no renderer/history/RNG reset. Check, never silently
 * repair, moments or an unexpected external colour change. */
static bool b2aw_hf_controls(unsigned block){
 cvar_t *colour=Cvar_FindVar("flt_min_alpha_color_hf"),*moments=Cvar_FindVar("flt_min_alpha_moments_hf");
 cvar_t *mode=Cvar_FindVar("b2_hack_mode");
 return block<B2AW_BLOCKS&&colour&&moments&&mode&&colour->value==b2aw_colour(block)&&moments->value==.01f&&mode->value==(float)b2aw_mode(block);
}
/* Called at the existing coherent boundary, after geometry validation and
 * before B2_HistoryBegin/any eye. An attempted change is not a committed frame. */
static bool b2aw_apply_controls(const b2_frame_t *frame){
 unsigned previous=b2aw.controls_set?b2aw.applied_block:0u;
 if(!b2aw_hf_controls(previous))return false;
 if(!b2aw.controls_set||b2aw.block!=previous){
  if(b2aw.phase!=0u||(!b2aw.controls_set&&b2aw.block!=0u)||
     (b2aw.controls_set&&b2aw.block!=previous+1u))return false;
  if(b2aw.controls_set)Cvar_Set("b2_hack_mode",b2aw_mode(b2aw.block)==5u?"5":"10");
  if(!b2aw_hf_controls(b2aw.block))return false;
  Com_Printf("B2_WALL_CONTROL recipe=%s block=%u previous_block=%u pair=%u generation=%u time_ms=%u colour_hf=%.9g moments_hf=%.9g hack_mode=%u changed=%u before_history_begin=1 committed=0\n",
   B2AW_ID,b2aw.block,previous,frame->logical_index,frame->history_generation,b2aw.frame_ms,b2aw_colour(b2aw.block),.01f,b2aw_mode(b2aw.block),b2aw.controls_set&&b2aw_mode(previous)!=b2aw_mode(b2aw.block));
  b2aw.applied_block=b2aw.block;b2aw.controls_set=true;
 }return true;
}
/* Explicit human arm AFTER normal navigation; no navigation or exact stance
 * replay is automated. The measured movement then needs no manual pitch. */
static void b2aw_start_f(void){
 if(!b2aw_enabled()||Cmd_Argc()!=1||b2aw.started||b2_client.stopped||
    (uint32_t)(Sys_Milliseconds()-b2aw_init_ms)>=B2AW_STARTUP_MS||
    strcmp(cl.mapname,"base1")||!b2_client.frozen||!b2_client.walk_ready||
    !b2_client.origin_latched||!b2_client.history.valid||B2_UIPaused()||
    cls.active!=ACT_ACTIVATED||cls.key_dest!=KEY_GAME||!B2XR_InteractionReady()||
    !b2aw_input_clear()||Cvar_VariableInteger("b2_lf_measure")!=0||!b2aw_hf_controls(0u)){
  Com_Printf("B2_WALL_REFUSED arm_preconditions\n");return;
 }
 b2aw.started=b2aw.armed=true;b2aw.start_ms=Sys_Milliseconds();
 Com_Printf("B2_WALL_START recipe=%s max_work_ms=%u startup_ms=%u sequence_ms=%u repeats=%u target_probe=1 candidate=10755 native_authority=external human_ready=0\n",B2AW_ID,B2AW_WORK_MS,B2AW_STARTUP_MS,B2AW_SEQUENCE_MS,B2AW_REPEATS);
}
static bool b2aw_world_valid(void){
 return b2aw.held&&b2aw_enabled()&&Cvar_VariableInteger("b2_lf_measure")==1&&B2_UIEnabled()&&b2_client.advancing&&!b2_client.stopped&&
  !b2_client.world_waiting&&!b2_client.world_rebinding&&!b2_client.ui_manual_pause&&!b2_client.ui_interrupted&&
  cls.key_dest==KEY_GAME&&cls.active==ACT_ACTIVATED&&b2_client.ui_paused&&cl_paused->integer==1&&
  Cvar_VariableInteger("sv_paused")==1&&b2_ui_validate_hold()&&
  cl.servercount==b2aw.servercount&&cl.frame.number==b2aw.client_frame&&cl.frame.areabytes==b2aw.areabytes&&
  b2_client.scene_serial==b2aw.serial&&!memcmp(&cl.frame.ps,&b2aw.player,sizeof(b2aw.player))&&
  !memcmp(&b2_client.scene,&b2aw.scene,sizeof(b2aw.scene))&&b2aw_world_hash()==b2aw.world_hash;
}
/* Outside any XR/history transaction, after actual events/input processing.
 * No broad input injection or new runtime route. Existing freeze activation
 * and CL_B2PreparePause publish the held scene using their normal guards. */
static void b2aw_tick(void){
 if(!b2aw_enabled()||b2aw.finished)return;
 if(!b2aw.started){
  if(Cvar_VariableInteger("b2_lf_measure")!=0||(uint32_t)(Sys_Milliseconds()-b2aw_init_ms)>=B2AW_STARTUP_MS){b2aw.started=true;b2aw.start_ms=Sys_Milliseconds();b2aw_fail();}
  return;
 }
 uint32_t elapsed=(uint32_t)(Sys_Milliseconds()-b2aw.start_ms);
 if(!b2aw_enabled()||elapsed>B2AW_SEQUENCE_MS+1000u||b2_client.stopped){b2aw_fail();return;}
 if(b2aw.held){
  if(!b2aw_world_valid()||!b2aw_input_clear()||!B2XR_InteractionReady()){b2aw_fail();return;}
  if((uint32_t)(Sys_Milliseconds()-b2aw.sequence_ms)>=B2AW_SEQUENCE_MS){
   if(b2_client.history.frame_open||b2aw.pending||!b2aw.has_previous){b2aw_fail();return;}
   /* The last rendered quad already committed. Keep the coherent pause until
    * terminal shutdown; never resume simulation just to close a capture. */
   Com_Printf("B2_WALL_END recipe=%s success=1 last_logical=%u elapsed_ms=%u held=1 terminal=1\n",B2AW_ID,b2aw.last_logical,elapsed);
   b2aw.finished=true;b2aw.held=false;b2_stop_at(__func__,__LINE__);Cbuf_AddText(&cmd_buffer,"quit\n");
  }return;
 }
 if(elapsed>=1000u||!b2aw.armed||strcmp(cl.mapname,"base1")||
    Cvar_VariableInteger("b2_dynamic_foveation")||Cvar_VariableInteger("b2_controller_input")||
    Cvar_VariableInteger("b2_roomscale")||Cvar_VariableInteger("b2_world_reticle")||
    Cvar_VariableInteger("b2_wall_return")||Cvar_VariableInteger("b2_teleport")||Cvar_VariableInteger("b2_tracked_weapon")||
    Cvar_VariableInteger("b2_lf_measure")!=0){b2aw_fail();return;}
 for(unsigned k=0;k<3;k++)if(cl.frame.ps.pmove.velocity[k]){b2aw_fail();return;}
 if(cls.active!=ACT_ACTIVATED||cls.key_dest!=KEY_GAME||!B2XR_InteractionReady()||!b2aw_input_clear()||b2_client.ui_interrupted||b2_client.ui_manual_pause){b2aw_fail();return;}
 if(!b2_client.frozen||!b2_client.walk_ready||!b2_client.origin_latched||!b2_client.history.valid)return;
 if(!B2_UIEnabled()||!b2_client.advancing||b2_client.history.frame_open||B2_UIPaused()||b2_client.world_waiting||b2_client.world_rebinding){b2aw_fail();return;}
 b2aw.held=true;b2_ui_apply_pause();
 if(b2_client.stopped||!B2_UIPaused()||cl.frame.areabytes<0||cl.frame.areabytes>MAX_MAP_AREA_BYTES){b2aw_fail();return;}
 b2aw.areabytes=cl.frame.areabytes;b2aw.scene=b2_client.scene;b2aw.serial=b2_client.scene_serial;
 b2aw.client_frame=cl.frame.number;b2aw.servercount=cl.servercount;b2aw.player=cl.frame.ps;
 b2aw.world_hash=b2aw_world_hash();b2aw.sequence_ms=Sys_Milliseconds();
 memcpy(b2aw.input_angles,b2_client.input_angles,sizeof(b2aw.input_angles));
 /* Enable the unchanged measurement writer exactly once at acquisition. All
  * rendered pairs thereafter retain full cadence, three probes and R64 ABI. */
 Cvar_Set("b2_lf_measure","1");
 if(Cvar_VariableInteger("b2_lf_measure")!=1){b2aw_fail();return;}
 Com_Printf("B2_WALL_SETUP map=base1 player_eighth=%d,%d,%d scene_xyz=%.9g,%.9g,%.9g scene=%u world_hash=%08x exact_old_stance=0 train_equivalence=0\n",
  cl.frame.ps.pmove.origin[0],cl.frame.ps.pmove.origin[1],cl.frame.ps.pmove.origin[2],
  b2_client.scene.vieworg[0],b2_client.scene.vieworg[1],b2_client.scene.vieworg[2],b2aw.serial,b2aw.world_hash);
 Com_Printf("B2_WALL_CONTEXT_BEGIN\n");
 {cvar_t *v=Cvar_FindVar("pt_num_bounce_rays");if(!v){b2aw_fail();return;}Com_Printf("\"pt_num_bounce_rays\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_enable");if(!v){b2aw_fail();return;}Com_Printf("\"flt_enable\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_fsr_enable");if(!v){b2aw_fail();return;}Com_Printf("\"flt_fsr_enable\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("pt_accumulation_rendering");if(!v){b2aw_fail();return;}Com_Printf("\"pt_accumulation_rendering\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("drs_enable");if(!v){b2aw_fail();return;}Com_Printf("\"drs_enable\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("viewsize");if(!v){b2aw_fail();return;}Com_Printf("\"viewsize\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("r_maxfps");if(!v){b2aw_fail();return;}Com_Printf("\"r_maxfps\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("cl_async");if(!v){b2aw_fail();return;}Com_Printf("\"cl_async\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("timedemo");if(!v){b2aw_fail();return;}Com_Printf("\"timedemo\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_taa");if(!v){b2aw_fail();return;}Com_Printf("\"flt_taa\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_temporal_hf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_temporal_hf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_temporal_lf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_temporal_lf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_temporal_spec");if(!v){b2aw_fail();return;}Com_Printf("\"flt_temporal_spec\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_antilag_hf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_antilag_hf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_antilag_lf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_antilag_lf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_antilag_spec");if(!v){b2aw_fail();return;}Com_Printf("\"flt_antilag_spec\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_min_alpha_color_lf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_min_alpha_color_lf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_min_alpha_color_spec");if(!v){b2aw_fail();return;}Com_Printf("\"flt_min_alpha_color_spec\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_min_alpha_color_hf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_min_alpha_color_hf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_min_alpha_moments_hf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_min_alpha_moments_hf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_lf_measure");if(!v){b2aw_fail();return;}Com_Printf("\"b2_lf_measure\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_atrous_hf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_atrous_hf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_atrous_lf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_atrous_lf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_atrous_spec");if(!v){b2aw_fail();return;}Com_Printf("\"flt_atrous_spec\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_atrous_deflicker_lf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_atrous_deflicker_lf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_show_gradients");if(!v){b2aw_fail();return;}Com_Printf("\"flt_show_gradients\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("bloom_enable");if(!v){b2aw_fail();return;}Com_Printf("\"bloom_enable\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("bloom_debug");if(!v){b2aw_fail();return;}Com_Printf("\"bloom_debug\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_bloom_scale");if(!v){b2aw_fail();return;}Com_Printf("\"b2_bloom_scale\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_scene_gamma");if(!v){b2aw_fail();return;}Com_Printf("\"b2_scene_gamma\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("tm_exposure_bias");if(!v){b2aw_fail();return;}Com_Printf("\"tm_exposure_bias\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("fixedtime");if(!v){b2aw_fail();return;}Com_Printf("\"fixedtime\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("timescale");if(!v){b2aw_fail();return;}Com_Printf("\"timescale\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_wall_recipe");if(!v){b2aw_fail();return;}Com_Printf("\"b2_wall_recipe\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_dynamic_foveation");if(!v){b2aw_fail();return;}Com_Printf("\"b2_dynamic_foveation\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_controller_input");if(!v){b2aw_fail();return;}Com_Printf("\"b2_controller_input\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_roomscale");if(!v){b2aw_fail();return;}Com_Printf("\"b2_roomscale\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_world_reticle");if(!v){b2aw_fail();return;}Com_Printf("\"b2_world_reticle\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_wall_return");if(!v){b2aw_fail();return;}Com_Printf("\"b2_wall_return\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_teleport");if(!v){b2aw_fail();return;}Com_Printf("\"b2_teleport\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_tracked_weapon");if(!v){b2aw_fail();return;}Com_Printf("\"b2_tracked_weapon\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_atrous_lum_hf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_atrous_lum_hf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_atrous_normal_hf");if(!v){b2aw_fail();return;}Com_Printf("\"flt_atrous_normal_hf\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_atrous_depth");if(!v){b2aw_fail();return;}Com_Printf("\"flt_atrous_depth\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("flt_atrous_normal_spec");if(!v){b2aw_fail();return;}Com_Printf("\"flt_atrous_normal_spec\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("cl_maxfps");if(!v){b2aw_fail();return;}Com_Printf("\"cl_maxfps\" is \"%s\"\n",v->string);}
 {cvar_t *v=Cvar_FindVar("b2_hack_mode");if(!v){b2aw_fail();return;}Com_Printf("\"b2_hack_mode\" is \"%s\"\n",v->string);}
 Com_Printf("B2_WALL_CONTEXT_END\n");

 if(!b2aw_world_valid())b2aw_fail();
}
static uint32_t b2aw_bits(float f){uint32_t u;memcpy(&u,&f,4);return u;}
static void b2aw_log_view(const b2_frame_t *frame,unsigned eye){
 /* Exact float words, one bounded line per view. Raw views0/2 independently
  * join these actual applied matrices; views1/3 are retained in this context. */
 char v[129],p[129];for(unsigned k=0;k<16;k++){
  Q_snprintf(v+8*k,9,"%08x",b2aw_bits(frame->views[eye].V[k]));
  Q_snprintf(p+8*k,9,"%08x",b2aw_bits(frame->views[eye].P[k]));
 }
 const float *f=frame->views[eye].invV+8,*bf=b2aw.base[eye].invV+8;float cross[3];b2aw_cross(bf,f,cross);
 float achieved=atan2f(b2aw_dot(cross,b2aw.axis),b2aw_dot(bf,f))*57.29577951308232f;
 Com_Printf("B2_WALL_VIEW pair=%u generation=%u eye=%u achieved=%.9g V=%s P=%s\n",frame->logical_index,frame->history_generation,eye,achieved,v,p);
}
/* Before B2_HistoryBegin, after genuine views and head have been constructed. */
static bool b2aw_prepare(b2_frame_t *frame,b2_view_t *head){
 if(!b2aw.held)return !b2aw.pending;
 if(b2aw.pending||!b2aw_world_valid()||!b2aw_input_clear()||!B2XR_InteractionReady()||
  frame->view_count!=4||!frame->tracking_valid||!frame->should_render||b2_client.recenter_pending||
  b2_client.handoff_position_pending||b2_client.history.frame_open||strcmp(cl.mapname,"base1")||
  Cvar_VariableInteger("b2_dynamic_foveation")||memcmp(b2aw.input_angles,b2_client.input_angles,sizeof(b2aw.input_angles)))return false;
 if(!b2aw.has_previous){
  if(!b2aw_make_path(frame->views,b2aw.base,b2aw.axis,b2aw.endpoint))return false;
  b2aw.generation=frame->history_generation;b2aw.epoch=b2_client.world_epoch;
  Com_Printf("B2_WALL_PATH recipe=%s inside_deg=%.9g outside_deg=%.9g leg_ms=%u hold_ms=%u initial_ms=%u axis=%.9g,%.9g,%.9g current_projection=1 target_registration=pending\n",B2AW_ID,b2aw.endpoint[0],b2aw.endpoint[1],B2AW_LEG_MS,B2AW_HOLD_MS,B2AW_INITIAL_MS,b2aw.axis[0],b2aw.axis[1],b2aw.axis[2]);
 }else if(frame->logical_index!=b2aw.last_logical+1||frame->history_generation!=b2aw.generation||b2_client.world_epoch!=b2aw.epoch)return false;
 b2aw.frame_ms=(uint32_t)(Sys_Milliseconds()-b2aw.sequence_ms);float fraction;
 if(!b2aw_schedule(b2aw.frame_ms,&b2aw.block,&b2aw.iteration,&b2aw.phase,&fraction))return false;
 b2aw.requested=b2aw.endpoint[0]+(b2aw.endpoint[1]-b2aw.endpoint[0])*fraction;
 for(unsigned eye=0;eye<4;eye++){
  /* Preserve actual projection changes as terminal refusal, never overwrite
   * the fresh runtime projection with stale accepted values. */
  if(memcmp(frame->views[eye].P,b2aw.base[eye].P,sizeof(b2aw.base[eye].P))||
     memcmp(frame->views[eye].invP,b2aw.base[eye].invP,sizeof(b2aw.base[eye].invP)))return false;
  if(!b2aw_rotated(b2aw.base+eye,b2aw.axis,b2aw.requested,frame->views+eye))return false;
  float e[4];if(!b2aw_edges(frame->views+eye,e)||e[0]<64||e[1]<64||e[2]<64||(eye<2&&e[3]<64))return false;
 }
 *head=frame->views[0];for(unsigned k=0;k<3;k++)head->origin[k]=.5f*(frame->views[0].origin[k]+frame->views[1].origin[k]);b2aw_finish_view(head);
 if(!b2aw_view_valid(head)||!b2aw_apply_controls(frame))return false;b2aw.pending=true;return true;
}
/* B2_HistoryBegin is the only history opener; its previous camera must be the
 * actual prior committed camera, not an A/B approximation during a sweep. */
static bool b2aw_history(const b2_frame_t *frame){
 if(!b2aw.held)return true;
 if(!b2aw.pending||!frame->history_valid||frame->history_generation!=b2aw.generation)return false;
 if(b2aw.has_previous)for(unsigned eye=0;eye<4;eye++)
  if(memcmp(frame->previous[eye].V,b2aw.previous[eye].V,sizeof(b2aw.previous[eye].V))||
     memcmp(frame->previous[eye].P,b2aw.previous[eye].P,sizeof(b2aw.previous[eye].P)))return false;
 return true;
}
/* Called only after actual XR end, renderer finish and HistoryFinish. Context
 * cannot turn an attempted frame into an accepted or fabricated observation. */
static bool b2aw_commit(const b2_frame_t *frame,bool committed){
 if(!b2aw.held)return true;
 if(!committed||!b2aw.pending||b2_client.history.frame_open||!b2aw_world_valid()||
    !b2aw.controls_set||b2aw.applied_block!=b2aw.block||!b2aw_hf_controls(b2aw.block))return false;
 Com_Printf("B2_WALL_FRAME pair=%u generation=%u iteration=%u phase=%u time_ms=%u requested=%.9g predicted_ns=%lld duration_ns=%lld tracking=%u render=%u history=%u held=1 scene=%u world_hash=%08x committed=1\n",frame->logical_index,frame->history_generation,b2aw.iteration,b2aw.phase,b2aw.frame_ms,b2aw.requested,(long long)frame->predicted_time_ns,(long long)frame->duration_ns,frame->tracking_valid,frame->should_render,frame->history_valid,b2aw.serial,b2aw.world_hash);
 Com_Printf("B2_WALL_CONTROL_FRAME recipe=%s pair=%u generation=%u block=%u time_ms=%u colour_hf=%.9g moments_hf=%.9g hack_mode=%u committed=1\n",
  B2AW_ID,frame->logical_index,frame->history_generation,b2aw.block,b2aw.frame_ms,b2aw_colour(b2aw.block),.01f,b2aw_mode(b2aw.block));
 for(unsigned eye=0;eye<4;eye++)b2aw_log_view(frame,eye);
 memcpy(b2aw.previous,frame->views,sizeof(b2aw.previous));b2aw.last_logical=frame->logical_index;
 b2aw.has_previous=true;b2aw.pending=false;return true;
}
