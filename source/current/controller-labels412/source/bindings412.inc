// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#include "b2_bindings287.h"
#include "b2_bindings_table287.inc"
bool B2B_SonyFaces412(unsigned hand);
#include "draw412.inc"
static b2b_config_t b2bm_pending,b2bm_custom316;
static void b2bm_label316(int source,char label[64])
{
    if (source<0 || source>=12) { Q_strlcpy(label,"Unbound",64);return; }
    unsigned hand=(unsigned)source/6,part=(unsigned)source%6;
    const char *side=hand?"Right":"Left";
    if(part==B2B_PRIMARY || part==B2B_SECONDARY){
        int profile=B2B_Profile316(hand);
        const char *button=part==B2B_PRIMARY?"Primary":"Secondary";
        if(B2B_SonyFaces412(hand)) Q_snprintf(label,64,"%s %s (%c) Button",side,button,1+(hand?0:2)+(part==B2B_SECONDARY));
        else if(profile<0) Q_snprintf(label,64,"%s %s Button",side,button);
        else {
            const char *letters=(hand || profile==1)?"AB":"XY";
            Q_snprintf(label,64,"%s %s (%c) Button",side,button,letters[part==B2B_SECONDARY]);
        }
    } else Q_strlcpy(label,b2b_inputs[source],64);
}
static menuSpinControl_t *b2bm_haptics316,*b2bm_hand,*b2bm_preset,*b2bm_rows[B2B_COUNT];
static int b2bm_choices[B2B_COUNT][26];
static menuCommon_t *b2bm_copy320;
static bool b2bm_copied320;
static bool b2bm_locked320(const menuSpinControl_t *row)
{
    if(b2bm_pending.preset==2 || !(row->generic.flags & QMF_GRAYED))return false;
    for(unsigned i=0;i<B2B_COUNT;i++)if(row==b2bm_rows[i])return true;
    return false;
}
static menuCommon_t *b2bm_menu_note,*b2bm_film_note,*b2bm_fixed327;
static int b2bm_profiles316[2],b2bm_menu_mask316;
static bool b2bm_sony412[2];
static int b2bm_menu_capability316(void)
{
    return (B2B_MenuAvailable316(0)?1:0) | (B2B_MenuAvailable316(1)?2:0);
}
static void b2bm_choices_refresh(void)
{
    unsigned d=1-b2bm_hand->curvalue;
    for(unsigned h=0;h<2;h++){b2bm_profiles316[h]=B2B_Profile316(h);b2bm_sony412[h]=B2B_SonyFaces412(h);}
    b2bm_menu_mask316=b2bm_menu_capability316();
    static const char *const menu_buttons[]={"Fixed Menu (open/back): no active controller input","Fixed Menu (open/back): left input active","Fixed Menu (open/back): right input active","Fixed Menu (open/back): both inputs active"};
    for(unsigned a=0;a<B2B_COUNT;a++){
        menuSpinControl_t *s=b2bm_rows[a];
        for(int i=0;i<s->numItems;i++)Z_Free(s->itemnames[i]);
        memset(s->itemnames,0,26*sizeof(char*));s->numItems=0;
        if(b2bm_pending.preset==2)s->generic.flags&=~QMF_GRAYED;
        else s->generic.flags|=QMF_GRAYED;
        int current=b2b_source(&b2bm_pending,a,d);
        for(int choice=0;choice<13;choice++){
            int physical=choice==12?25:choice;if(!b2b_type(a,physical))continue;
            int i=s->numItems++;
            // One visible choice per physical component. Preserve the current
            // encoded absolute/relative/legacy mapping until explicitly changed.
            b2bm_choices[a][i]=(physical==current || (physical==25 && current==-1))?b2bm_pending.input[a]:physical;
            char label[64];b2bm_label316(physical,label);
            s->itemnames[i]=UI_CopyString(label);
            if(physical==current || (physical==25 && current==-1))s->curvalue=i;
        }
    }
    if(b2bm_menu_note){
        Z_Free(b2bm_fixed327->name);
        b2bm_fixed327->name=UI_CopyString(menu_buttons[b2bm_menu_mask316]);
        Z_Free(b2bm_menu_note->name);Z_Free(b2bm_film_note->name);
        b2bm_menu_note->name=UI_CopyString("Menus: either stick / either primary");
        char button[64],hint[96];b2bm_label316(d*6+B2B_PRIMARY,button);
        Q_snprintf(hint,sizeof(hint),"Film: %s / trigger hold",button);
        b2bm_film_note->name=UI_CopyString(hint);
    }
}
static void b2bm_refresh(void)
{
    b2bm_preset->curvalue=b2bm_pending.preset;
    if(b2bm_pending.preset==2){
        b2bm_copy320->flags|=QMF_GRAYED;
        if(uis.mouseTracker==b2bm_copy320)uis.mouseTracker=NULL;
        if(b2bm_copy320->flags & QMF_HASFOCUS)Menu_SetFocus(&b2bm_preset->generic);
    } else b2bm_copy320->flags&=~QMF_GRAYED;
    b2bm_choices_refresh();
}
static menuSound_t b2bm_changed(menuCommon_t *item)
{
    if(item==&b2bm_haptics316->generic){
        Cvar_SetInteger(b2bm_haptics316->cvar,b2bm_haptics316->curvalue,FROM_MENU);
        return QMS_NOTHANDLED;
    }
    b2bm_copied320=false;
    if(item==&b2bm_preset->generic){
        int choice=b2bm_preset->curvalue;
        if(choice<2)b2b_preset(&b2bm_pending,choice);
        else b2bm_pending=b2bm_custom316;
    }else for(unsigned a=0;a<B2B_COUNT;a++)if(item==&b2bm_rows[a]->generic){
        if(b2bm_pending.preset!=2){b2bm_refresh();return QMS_BEEP;}
        b2bm_pending.input[a]=b2bm_choices[a][b2bm_rows[a]->curvalue];b2bm_custom316=b2bm_pending;
    }
    b2bm_refresh();Menu_Init(item->parent);
    item->parent->status=b2b_validate(&b2bm_pending)?"Pending changes. Apply to save; Cancel discards.":"Conflict in game or menu (either hand). Choose another input.";
    return QMS_NOTHANDLED;
}
static bool b2bm_push(menuFrameWork_t *menu)
{
    b2bm_haptics316->curvalue=b2bm_haptics316->cvar->integer!=0;
    b2bm_hand->curvalue=1-(int)B2B_Hand();
    B2B_Read(&b2bm_pending);B2B_Custom316(&b2bm_custom316);b2bm_copied320=false;b2bm_refresh();
    menu->status="Choose Custom to edit. Apply saves changes.";
    return true;
}
static void b2bm_pop(menuFrameWork_t *menu){(void)menu;} // Back/escape/cancel never commit.
static menuSound_t b2bm_apply(menuCommon_t *item)
{
    if(!B2B_Apply(&b2bm_pending)){item->parent->status="Conflict: choose distinct game inputs; menu cannot use confirm.";return QMS_BEEP;}
    if(b2bm_pending.preset<2)B2B_SaveCustom316(&b2bm_custom316);
    Cvar_SetInteger(Cvar_Get("hand","0",CVAR_ARCHIVE|CVAR_USERINFO),b2bm_hand->curvalue,FROM_MENU);
    B2B_Hand();
    Cbuf_AddText(&cmd_buffer,"popmenu\n");return QMS_NOTHANDLED;
}
static menuSound_t b2bm_copy_to_custom320(menuCommon_t *item)
{
    if(b2bm_pending.preset==2 || (item->flags&QMF_GRAYED))return QMS_BEEP;
    b2bm_custom316=b2bm_pending;
    b2bm_custom316.preset=2;
    b2bm_copied320=true;
    item->parent->status="Copied. Choose Custom to edit, or Apply to save.";
    return QMS_NOTHANDLED;
}
static menuSpinControl_t *b2bm_spin(menuFrameWork_t *menu,const char *name)
{
    menuSpinControl_t *s=UI_Mallocz(sizeof(*s));s->generic.type=MTYPE_SPINCONTROL;
    s->generic.name=UI_CopyString(name);s->generic.change=b2bm_changed;
    s->itemnames=UI_Mallocz(26*sizeof(char *));Menu_AddItem(menu,s);return s;
}
static void b2bm_action(menuFrameWork_t *menu,const char *name,menuSound_t (*activate)(menuCommon_t *),const char *command)
{
    menuAction_t *a=UI_Mallocz(sizeof(*a));a->generic.type=MTYPE_ACTION;
    a->generic.name=UI_CopyString(name);a->generic.activate=activate;a->generic.uiFlags=UI_CENTER;
    if(command)a->cmd=UI_CopyString(command);Menu_AddItem(menu,a);
}
static void UI_B2BindingsInit(void)
{
    menuFrameWork_t *options=UI_FindMenu("options");if(!options || UI_FindMenu("b2_bindings"))return;
    menuFrameWork_t *m=UI_Mallocz(sizeof(*m));m->name=UI_CopyString("b2_bindings");
    m->title=UI_CopyString("VR Controller Settings");m->push=b2bm_push;m->pop=b2bm_pop;m->free=Menu_Free;
    m->image=options->image;m->color=options->color;m->transparent=options->transparent;m->compact=true;
    b2bm_hand=b2bm_spin(m,"Handedness");
    b2bm_hand->itemnames[0]=UI_CopyString("Right");b2bm_hand->itemnames[1]=UI_CopyString("Left");b2bm_hand->numItems=2;
    b2bm_preset=b2bm_spin(m,"Preset");
    const char *names[]={"Togorian","Quest-style","Custom"};
    for(int i=0;i<3;i++)b2bm_preset->itemnames[i]=UI_CopyString(names[i]);b2bm_preset->numItems=3;
    b2bm_haptics316=b2bm_spin(m,"Haptics");
    b2bm_haptics316->cvar=Cvar_Get("b2_haptics","1",CVAR_ARCHIVE);
    b2bm_haptics316->itemnames[0]=UI_CopyString("Off");
    b2bm_haptics316->itemnames[1]=UI_CopyString("On");
    b2bm_haptics316->numItems=2;
    b2bm_haptics316->generic.status=UI_CopyString("Controller vibration. Applies immediately and saves for every preset.");
    menuSeparator_t *gap=UI_Mallocz(sizeof(*gap));gap->generic.type=MTYPE_SEPARATOR;Menu_AddItem(m,gap);
    for(unsigned a=0;a<B2B_COUNT;a++){
        const char *name=b2b_names[a];
        if(a==B2B_MENU)name="Menu / back shortcut";
        if(a==B2B_EXTRA)name="Additional menu shortcut";
        menuSpinControl_t *s=b2bm_rows[a]=b2bm_spin(m,name);
        for(int code=0;code<26;code++)if(b2b_type(a,code)){
            b2bm_choices[a][s->numItems]=code;s->itemnames[s->numItems++]=UI_CopyString(b2b_inputs[code]);
        }
        s->generic.status=UI_CopyString(a==B2B_RUN?"Tap toggles walk/run. Teleport only: hold this input + forward for a slow vignetted nudge.":a==B2B_CYCLE347?"Fresh presses cycle mission, inventory and closed. Use a secondary button or stick click.":"Only supported components are offered. Conflicts are checked for both hands.");
    }
    b2bm_action(m,"Apply to Custom settings",b2bm_copy_to_custom320,NULL);
    b2bm_copy320=m->items[m->nitems-1];
    b2bm_action(m,"Apply",b2bm_apply,NULL);b2bm_action(m,"Cancel",B2VR_Action,"popmenu");
    b2bm_action(m,"Fixed Menu",NULL,NULL);b2bm_fixed327=m->items[m->nitems-1];
    b2bm_fixed327->flags|=QMF_GRAYED;
    b2bm_action(m,"Menus: either stick / either primary",NULL,NULL);b2bm_menu_note=m->items[m->nitems-1];((menuCommon_t *)m->items[m->nitems-1])->flags|=QMF_GRAYED;
    b2bm_action(m,"Film: Right primary / trigger hold",NULL,NULL);b2bm_film_note=m->items[m->nitems-1];((menuCommon_t *)m->items[m->nitems-1])->flags|=QMF_GRAYED;
    List_Append(&ui_menus,&m->entry);
    b2bm_action(options,"VR Controller Settings",B2VR_Action,"pushmenu b2_bindings");
    menuCommon_t *link=options->items[options->nitems-1];link->uiFlags=UI_CENTER;
    memmove(options->items+2,options->items+1,(options->nitems-2)*sizeof(void *));options->items[1]=link;
}

// Cached runtime metadata is revalidated on every draw, never captured as a
// persistent item string. This getter performs no native action query.
static void B2BM_Status(menuFrameWork_t *menu)
{
    if(!menu->name || strcmp(menu->name,"b2_bindings"))return;
    if(b2bm_sony412[0]!=B2B_SonyFaces412(0) || b2bm_sony412[1]!=B2B_SonyFaces412(1) || b2bm_menu_mask316!=b2bm_menu_capability316() || b2bm_profiles316[0]!=B2B_Profile316(0) || b2bm_profiles316[1]!=B2B_Profile316(1))
        b2bm_choices_refresh();
    static char status[160];
    menu->status=b2b_validate(&b2bm_pending)?"Pending edits: Apply saves; Cancel discards. Only Custom can edit action rows.":"Conflict in game or menu (either hand). Choose another input.";
    menuCommon_t *focus=Menu_ItemAtCursor(menu);
    if(focus==&b2bm_haptics316->generic){menu->status=focus->status;return;}
    if(b2bm_copied320){menu->status="Copied. Choose Custom to edit, or Apply to save.";return;}
    if(!b2b_validate(&b2bm_pending))return;
    unsigned d=1-b2bm_hand->curvalue;
    for(unsigned a=0;a<B2B_COUNT;a++)if(focus==&b2bm_rows[a]->generic){
        int source=b2b_source(&b2bm_pending,a,d);
        int cycle=b2b_source(&b2bm_pending,B2B_CYCLE347,d);
        if(source>=0 && source==cycle && a==B2B_MENU){
            menu->status="Shared with cycle: use hardware Menu during play; this input is Back inside menus.";return;
        }
        if(source>=0 && source==cycle && a==B2B_EXTRA){
            menu->status="Shared with cycle: screen cycle during play; menu shortcut during films.";return;
        }
        if(a==B2B_CYCLE347){menu->status=focus->status;return;}
        char name[96];
        if(B2B_Label((unsigned)b2b_source(&b2bm_pending,a,d),name,sizeof(name))){
            Q_snprintf(status,sizeof(status),"Runtime input: %s. Apply saves pending edits.",name);menu->status=status;
        }
    }
}
