// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Two fixed footer destinations; ordinary action controls and menu stack.
extern bool UI_B2LinksFocused(void);
extern bool Sys_B2QueueCredit301(unsigned destination);
static menuSound_t B2L_Ask(menuCommon_t *item);
static menuFrameWork_t *b2l_dialog;
static unsigned b2l_destination;
static bool b2l_edge,b2l_blocked,b2l_used,b2l_held[256];
static bool B2L_Link(const menuCommon_t *item)
{ return item && item->activate==B2L_Ask; }
static const char *B2L_Address(unsigned id)
{ return id==0?"https://github.com/Togorian/Q2RTX-VR":id==1?"https://ko-fi.com/togorian":NULL; }
void UI_B2LinksFocusLost(void)
{
    b2l_edge=false;b2l_blocked=true;
    if(b2l_dialog && uis.activeMenu==b2l_dialog)Menu_SetFocus(b2l_dialog->items[1]);
}
bool UI_B2LinkKey(int key,bool down)
{
    b2l_edge=false;
    if(key!=K_ENTER && key!=K_KP_ENTER && key!=K_MOUSE1 && key!=K_MOUSE3)return false;
    if(!down){b2l_held[key]=false;b2l_blocked=false;return false;}
    bool repeated=b2l_held[key] || Key_IsDown(key)>1;b2l_held[key]=true;
    bool relevant=uis.activeMenu && (uis.activeMenu==b2l_dialog || B2L_Link(Menu_ItemAtCursor(uis.activeMenu)));
    if(!relevant)return false;
    if(!UI_B2LinksFocused() || b2l_blocked || repeated)return true;
    b2l_edge=true;return false;
}
void UI_B2LinkEndKey(void){b2l_edge=false;}
static menuSound_t B2L_Choice(menuCommon_t *item)
{
    if(!b2l_edge || !UI_B2LinksFocused() || uis.activeMenu!=b2l_dialog)return QMS_SILENT;
    b2l_edge=false;
    if(item->id==0){UI_PopMenu();return QMS_OUT;}
    if(b2l_used || !B2L_Address(b2l_destination))return QMS_SILENT;
    b2l_used=true; // Consume this confirmation before queuing; the OS is never called here.
    bool ok=Sys_B2QueueCredit301(b2l_destination);
    b2l_dialog->status=ok?"Queued for normal game exit. Back returns to menu.":"Could not queue. Address above; Back returns to menu.";
    ((menuCommon_t *)b2l_dialog->items[2])->flags|=QMF_GRAYED;
    Menu_SetFocus(b2l_dialog->items[1]);
    return ok?QMS_IN:QMS_BEEP;
}
static menuSound_t B2L_Ask(menuCommon_t *item)
{
    if(!b2l_edge || !UI_B2LinksFocused() || !B2L_Address(item->id))return QMS_SILENT;
    b2l_edge=false;
    b2l_dialog=UI_FindMenu("b2_credit_confirm");
    if(!b2l_dialog){
        b2l_dialog=UI_Mallocz(sizeof(*b2l_dialog));
        b2l_dialog->name=UI_CopyString("b2_credit_confirm");
        b2l_dialog->title=UI_CopyString("Open in desktop browser AFTER QUIT?");
        b2l_dialog->compact=true;b2l_dialog->push=Menu_Push;b2l_dialog->pop=Menu_Pop;b2l_dialog->free=Menu_Free;
        menuStatic_t *address=UI_Mallocz(sizeof(*address));address->generic.type=MTYPE_STATIC;address->generic.uiFlags=UI_CENTER;
        address->generic.name=UI_CopyString("");Menu_AddItem(b2l_dialog,address);
        for(int i=0;i<2;i++){
            menuAction_t *a=UI_Mallocz(sizeof(*a));a->generic.type=MTYPE_ACTION;a->generic.id=i;
            a->generic.name=UI_CopyString(i?"Queue until quit":"Cancel");a->generic.uiFlags=UI_CENTER;a->generic.activate=B2L_Choice;Menu_AddItem(b2l_dialog,a);
        }
        List_Append(&ui_menus,&b2l_dialog->entry);
    }
    b2l_destination=item->id;b2l_used=false;
    menuCommon_t *address=b2l_dialog->items[0];Z_Free(address->name);address->name=UI_CopyString(B2L_Address(item->id));
    b2l_dialog->image=item->parent->image;b2l_dialog->color=item->parent->color;b2l_dialog->transparent=item->parent->transparent;
    b2l_dialog->status="Opens after you quit the game normally. Cancel queues nothing.";
    ((menuCommon_t *)b2l_dialog->items[2])->flags&=~QMF_GRAYED;
    Menu_SetFocus(b2l_dialog->items[1]);UI_PushMenu(b2l_dialog);Menu_SetFocus(b2l_dialog->items[1]);
    return QMS_IN;
}
static void B2L_Attach(menuFrameWork_t *menu)
{
    if(!menu->footer || !menu->name || (strcmp(menu->name,"main") && strcmp(menu->name,"game")))return;
    for(int i=0;i<menu->nitems;i++)if(B2L_Link(menu->items[i]))return;
    for(int i=0;i<2;i++){
        menuAction_t *a=UI_Mallocz(sizeof(*a));a->generic.type=MTYPE_ACTION;a->generic.id=i;
        a->generic.name=UI_CopyString(i?"ko-fi.com/togorian":"github.com/Togorian/Q2RTX-VR");
        a->generic.uiFlags=UI_CENTER;a->generic.activate=B2L_Ask;Menu_AddItem(menu,a);
    }
}
static float B2L_Position(menuCommon_t *item)
{
    const vrect_t *r=&item->parent->footer_rc;float fit=r->width/300.f;
    float left=r->x+r->width*(132.5f/600.f)-4.f*fit,right=r->x+r->width*(428.f/600.f)+3.f*fit;
    float prior=min(1.f*fit,min((left-4.f)/(9.5f*CHAR_WIDTH),(uis.width-4.f-right)/(9.5f*CHAR_WIDTH)));
    float scale=min(.875f*prior,(uis.width-4.f-right)/(14.f*CHAR_WIDTH));
    int y=r->y+r->height+(int)ceilf(3.f*fit+CHAR_HEIGHT*prior);
    item->x=(int)(right/scale);item->y=(int)((y+item->id*CHAR_HEIGHT*prior)/scale);
    item->rect=(vrect_t){(int)(right-strlen(item->name)*CHAR_WIDTH*scale/2),(int)(item->y*scale),(int)ceilf(strlen(item->name)*CHAR_WIDTH*scale),(int)ceilf(CHAR_HEIGHT*scale)};
    return scale;
}
static void B2L_Draw(menuFrameWork_t *menu)
{
    for(int i=0;i<menu->nitems;i++){
        menuCommon_t *a=menu->items[i];if(!B2L_Link(a))continue;
        float scale=B2L_Position(a);int flags=UI_CENTER;
        R_SetScale(uis.scale/scale);
        if(a->flags&QMF_HASFOCUS)flags|=UI_ALTCOLOR;else R_SetColor(MakeColor(0,255,0,255));
        // Same centred-action glyph, cadence, colour and spacing as Action_Draw.
        // The active URL scale also applies to its selection cue.
        if((a->flags&QMF_HASFOCUS) && ((uis.realtime>>8)&1))
            UI_DrawChar(a->x-strlen(a->name)*CHAR_WIDTH/2-CHAR_WIDTH,a->y,flags,13);
        UI_DrawString(a->x,a->y,flags,a->name);R_ClearColor();R_SetScale(uis.scale);
    }
}

void UI_B2LinksReset(void){b2l_dialog=NULL;b2l_edge=false;b2l_used=true;}
