// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#include "b2_bindings287.h"
#include "b2_bindings_table287.inc"
static void b2b_reset_client(void);
static b2b_config_t b2b_live;
static cvar_t *b2b_saved,*b2b_custom316;
static bool b2b_initialised,b2b_release;
static unsigned b2b_revision=1,b2b_hand=2;
static uint64_t b2b_serial;
static char b2b_last[256];
static bool b2b_decode(const char *s,b2b_config_t *out)
{
    if(!strcmp(s,"0") || !strcmp(s,"1")){b2b_preset(out,s[0]-'0');return true;}
    if(strncmp(s,"2,",2))return false;
    b2b_config_t c={.preset=2};s+=2;
    for(unsigned a=B2B_ITEMS;a<B2B_COUNT;a++)c.input[a]=25;
    for(unsigned a=0;a<B2B_COUNT;a++){
        unsigned v=0,n=0;while(*s>='0' && *s<='9'){if(++n>2)return false;v=v*10+*s++-'0';}
        if(!n || v>25)return false;c.input[a]=(int)v;
        if(a==B2B_RESUME && !*s){
            // Legacy12-action custom value: preserve every old assignment.
            c.input[B2B_RUN]=23;
            if(!b2b_validate(&c))c.input[B2B_RUN]=25;
            if(!b2b_validate(&c))return false;*out=c;return true;
        }
        if((a==B2B_RUN || a==B2B_HELP) && !*s){if(!b2b_validate(&c))return false;*out=c;return true;}
        if(a+1<B2B_COUNT){if(*s++!=',')return false;}else if(*s)return false;
    }
    if(!b2b_validate(&c))return false;*out=c;return true;
}
static void b2b_encode(const b2b_config_t *c,char text[256])
{
    if(c->preset<2){snprintf(text,256,"%d",c->preset);return;}
    unsigned used=(unsigned)snprintf(text,256,"2");
    for(unsigned a=0;a<B2B_COUNT;a++)used+=(unsigned)snprintf(text+used,256-used,",%d",c->input[a]);
}
void B2B_Invalidate(void){b2b_release=true;b2b_serial=0;if(++b2b_revision==0)b2b_revision=1;}
unsigned B2B_Hand(void)
{
    cvar_t *hand=Cvar_Get("hand","0",CVAR_ARCHIVE|CVAR_USERINFO);
    int side=(hand->value==0.f || hand->value==1.f)?hand->integer:0;
    if(!Cvar_VariableInteger("b2_openxr"))return 1-side;
    if(hand->value!=side)Cvar_SetInteger(hand,side,FROM_CODE);
    cvar_t *mirror=Cvar_Get("b2_controller_dominant_hand","1",CVAR_NOSET|CVAR_NOARCHIVE);
    Cvar_SetInteger(mirror,1-side,FROM_CODE);
    return 1-side; // stock0=right/1=left; tracked0=left/1=right
}
bool B2B_TeleportOnly(void)
{return Cvar_Get("b2_teleport_only293","0",CVAR_ARCHIVE)->integer==1;}
static void b2b_poll(void)
{
    static int last_hand=-1,last_move=-1;
    int hand=(int)B2B_Hand(),move=B2B_TeleportOnly();
    if((last_hand>=0 && last_hand!=hand) || (last_move>=0 && last_move!=move)){
        B2B_Invalidate();b2b_reset_client();
    }
    last_hand=hand;last_move=move;
    if(!b2b_custom316)b2b_custom316=Cvar_Get("b2_custom_bindings316","",CVAR_ARCHIVE);
    if(!b2b_saved)b2b_saved=Cvar_Get("b2_bindings287","0",CVAR_ARCHIVE);
    if(b2b_initialised && !strcmp(b2b_last,b2b_saved->string))return;
    b2b_config_t next;if(!b2b_decode(b2b_saved->string,&next))b2b_preset(&next,0);
    Q_strlcpy(b2b_last,b2b_saved->string,sizeof(b2b_last));
    bool changed=b2b_initialised && memcmp(&next,&b2b_live,sizeof(next));
    b2b_live=next;b2b_initialised=true;
    if(changed){B2B_Invalidate();b2b_reset_client();}
}
bool B2B_Quest316(void){if(!b2b_initialised)b2b_poll();return b2b_live.preset==1;}
// Availability follows the explicit action, including saved Custom bindings.
bool B2B_Cycle338(void)
{
    if(!b2b_initialised)b2b_poll();
    return b2b_source(&b2b_live,B2B_CYCLE347,B2B_Hand())>=0;
}
void B2B_Read(b2b_config_t *out){b2b_poll();*out=b2b_live;}
void B2B_SaveCustom316(const b2b_config_t *in)
{
    if(in->preset!=2 || !b2b_validate(in))return;
    char text[256];b2b_encode(in,text);
    Cvar_SetByVar(b2b_custom316,text,FROM_MENU);
}

void B2B_Custom316(b2b_config_t *out)
{
    b2b_poll();
    if (b2b_live.preset==2) { *out=b2b_live;return; }
    if (b2b_decode(b2b_custom316->string,out) && out->preset==2) return;
    *out=b2b_live;
    out->preset=2;
    out->input[B2B_CYCLE347]=25; // New Custom action starts unbound; keep all old actions.
}
bool B2B_Apply(const b2b_config_t *in)
{
    b2b_config_t c=*in;if(c.preset==0 || c.preset==1)b2b_preset(&c,c.preset);
    if(!b2b_validate(&c))return false;
    char text[256];b2b_encode(&c,text);b2b_poll();
    if (c.preset==2) {
        Cvar_SetByVar(b2b_custom316,text,FROM_MENU);
    } else if (b2b_live.preset==2) {
        char old_custom[256];
        b2b_encode(&b2b_live,old_custom);
        Cvar_SetByVar(b2b_custom316,old_custom,FROM_MENU);
    }
    Cvar_SetByVar(b2b_saved,text,FROM_MENU);b2b_poll();
    B2B_Invalidate();b2b_reset_client();return true;
}
int B2B_Source(unsigned action,unsigned dominant)
{if(!b2b_initialised)b2b_poll();return b2b_source(&b2b_live,action,dominant);}
float B2B_Value(const b2c_snapshot_t *p,unsigned action,unsigned dominant,unsigned axis)
{return b2b_value(p,B2B_Source(action,dominant),axis);}
void B2B_Clear(b2c_snapshot_t *p,unsigned action,unsigned dominant)
{b2b_clear(p,B2B_Source(action,dominant));}
unsigned B2B_Revision(void){return b2b_revision;}
bool B2B_Barrier(const b2c_snapshot_t *p,unsigned dominant)
{
    b2b_poll();
    if(b2b_hand!=dominant){bool first=b2b_hand==2;b2b_hand=dominant;if(!first){B2B_Invalidate();b2b_reset_client();}}
    if(!b2b_release)return false;
    if(dominant>1 || !b2c_sample_valid(p,Sys_Milliseconds()) || !p->xr_generation)return true;
    if(p->serial!=b2b_serial){b2b_serial=p->serial;if(b2c_neutral(p))b2b_release=false;}
    return true; // The releasing sample never activates an action.
}
