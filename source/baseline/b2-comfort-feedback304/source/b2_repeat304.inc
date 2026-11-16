// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// One scheduler for real menu/list/slider callers, never action/text repeats.
static struct { menuFrameWork_t *menu; menuCommon_t *item; int owner,key;
    uint32_t started,stepped; } b2vr_repeat;
static bool b2vr_stepping;
static void B2UI_ScrollTick304(uint32_t now);
static int b2_ui_hand304=-1;
void B2H_UI304(unsigned event,unsigned hand);
void UI_B2ControllerInput304(int hand){b2_ui_hand304=hand;}
void UI_B2VRRepeatCancel(void){memset(&b2vr_repeat,0,sizeof(b2vr_repeat));}
static int B2VR_DirectionKey304(int key)
{
 switch(key){
 case K_KP_UPARROW:case 'k':return K_UPARROW;
 case K_KP_DOWNARROW:case 'j':return K_DOWNARROW;
 case K_KP_LEFTARROW:case 'h':return K_LEFTARROW;
 case K_KP_RIGHTARROW:case 'l':return K_RIGHTARROW;
 case K_KP_PGUP:return K_PGUP;case K_KP_PGDN:return K_PGDN;
 default:return key;
 }
}
static bool B2VR_RepeatAllowed(int key)
{
    key=B2VR_DirectionKey304(key);
    menuFrameWork_t *m=uis.activeMenu;
    if(!m || m->keywait || !(Key_GetDest()&KEY_MENU) || (Key_GetDest()&KEY_CONSOLE))return false;
    menuCommon_t *i=Menu_ItemAtCursor(m);
    if(!i || !UI_IsItemSelectable(i) || i->type==MTYPE_FIELD || i->type==MTYPE_KEYBIND)return false;
    return key==K_UPARROW || key==K_DOWNARROW || key==K_LEFTARROW || key==K_RIGHTARROW ||
        (i->type==MTYPE_LIST && (key==K_PGUP || key==K_PGDN));
}
static void B2VR_RepeatStep(int owner,int key,bool pressed,uint32_t now)
{
    if(!B2VR_RepeatAllowed(key)){if(b2vr_repeat.owner==owner)UI_B2VRRepeatCancel();return;}
    menuFrameWork_t *m=uis.activeMenu;menuCommon_t *item=Menu_ItemAtCursor(m);
    if(pressed){b2vr_repeat.menu=m;b2vr_repeat.item=item;b2vr_repeat.owner=owner;
        b2vr_repeat.key=key;b2vr_repeat.started=b2vr_repeat.stepped=now;}
    else {
        if(b2vr_repeat.owner!=owner || b2vr_repeat.menu!=m || b2vr_repeat.key!=key)return;
        if(b2vr_repeat.item!=item){UI_B2VRRepeatCancel();return;}
        uint32_t held=now-b2vr_repeat.started;unsigned interval=held>=2000?45:held>=1000?80:150;
        if(held<400 || now-b2vr_repeat.stepped<interval)return;
        b2vr_repeat.stepped=now;
    }
    b2vr_stepping=true;UI_StartSound(Menu_Keydown(m,key));b2vr_stepping=false;
    if(b2vr_repeat.menu==m && uis.activeMenu==m)b2vr_repeat.item=Menu_ItemAtCursor(m);
}
bool UI_B2VRRepeatKey(int key,bool down,uint32_t now)
{
    if(!down){if((b2vr_repeat.owner==1 && key==b2vr_repeat.key) || (b2vr_repeat.owner==3 && key==K_MOUSE1))UI_B2VRRepeatCancel();return false;}
    if(!Key_IsDown(key))return false;
    if(!B2VR_RepeatAllowed(key)){UI_B2VRRepeatCancel();return false;}
    if(Key_IsDown(key)>1)return true;
    int direction=B2VR_DirectionKey304(key);
    int opposite=direction==K_LEFTARROW?K_RIGHTARROW:direction==K_RIGHTARROW?K_LEFTARROW:
        direction==K_UPARROW?K_DOWNARROW:direction==K_DOWNARROW?K_UPARROW:0;
    const int directions[]={K_LEFTARROW,K_RIGHTARROW,K_UPARROW,K_DOWNARROW,K_KP_LEFTARROW,K_KP_RIGHTARROW,K_KP_UPARROW,K_KP_DOWNARROW,'h','l','k','j'};
    for(unsigned i=0;i<sizeof(directions)/sizeof(*directions);i++)
        if(opposite && B2VR_DirectionKey304(directions[i])==opposite && Key_IsDown(directions[i])){UI_B2VRRepeatCancel();return true;}
    B2VR_RepeatStep(1,key,true,now);return true;
}
void UI_B2VRRepeatTick(uint32_t now,bool focused)
{
    if(!focused){UI_B2VRRepeatCancel();return;}
    B2UI_ScrollTick304(now);
    if(b2vr_repeat.owner!=1)return;
    if(!Key_IsDown(b2vr_repeat.key)){UI_B2VRRepeatCancel();return;}
    B2VR_RepeatStep(1,b2vr_repeat.key,false,now);
}
bool UI_B2VRRepeatController(int dir,bool pressed,uint32_t now)
{
    int key=dir==1?K_RIGHTARROW:dir==-1?K_LEFTARROW:dir==2?K_UPARROW:dir==-2?K_DOWNARROW:0;
    if(!key){if(b2vr_repeat.owner==2)UI_B2VRRepeatCancel();return false;}
    bool allowed=B2VR_RepeatAllowed(key);B2VR_RepeatStep(2,key,pressed,now);return allowed;
}
static float B2UI_Value304(menuCommon_t *i)
{
    if(!i)return 0;
    if(i->type==MTYPE_SLIDER)return ((menuSlider_t*)i)->curvalue;
    if(i->type==MTYPE_LIST)return (float)((menuList_t*)i)->curvalue;
    if(i->type==MTYPE_SPINCONTROL || i->type==MTYPE_PAIRS || i->type==MTYPE_VALUES ||
       i->type==MTYPE_BITFIELD || i->type==MTYPE_TOGGLE || i->type==MTYPE_STRINGS)
        return (float)((menuSpinControl_t*)i)->curvalue;
    return 0;
}
