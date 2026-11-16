// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 14 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
#include "../../runtime-menu442/source/b2_graphics270.h"
static void B2G_Dependencies(menuFrameWork_t *menu);
// SPDX-License-Identifier: GPL-2.0-or-later
// Included by the selected menu.c; uses its stock controls and commit-on-pop.
static bool B2VR_Menu(const menuFrameWork_t *menu)
{
    return menu && menu->name && !strcmp(menu->name,"b2_vr");
}

static bool B2VR_Styled(const menuFrameWork_t *menu)
{ return B2VR_Menu(menu) || (menu && menu->name && !strcmp(menu->name,"b2_graphics")); }

// Bits local to these menu dependencies; retain pre-existing grey/disabled flags.
#define QMF_B2_LOCK534 BIT(7)
#define QMF_B2_WAS_GREY534 BIT(8)
extern bool Cvar_B2GraphicsLocked534(const char *name);
static bool B2VR_StockLocked534(const menuCommon_t *item)
{
    // Player Setup commits model/skin through its custom Pop, without row cvars.
    // Keep its handedness, aiming and view controls available.
    if (item->type == MTYPE_SPINCONTROL && item->parent && item->parent->name &&
        !strcmp(item->parent->name, "players") && item->name &&
        (!strcmp(item->name, "model") || !strcmp(item->name, "skin")))
        return Cvar_B2GraphicsLocked534("skin");
    cvar_t *var = NULL;
    switch (item->type) {
    case MTYPE_SPINCONTROL: case MTYPE_BITFIELD: case MTYPE_PAIRS:
    case MTYPE_VALUES: case MTYPE_STRINGS: case MTYPE_TOGGLE:
        var = ((const menuSpinControl_t *)item)->cvar; break;
    case MTYPE_SLIDER: var = ((const menuSlider_t *)item)->cvar; break;
    case MTYPE_FIELD: var = ((const menuField_t *)item)->cvar; break;
    default: break;
    }
    return var && Cvar_B2GraphicsLocked534(var->name);
}
static bool B2VR_StockDependencies534(menuFrameWork_t *menu)
{
    bool any = false;
    if (!menu) return false;
    for (int i = 0; i < menu->nitems; ++i) {
        menuCommon_t *item = menu->items[i];
        if (B2VR_StockLocked534(item)) {
            any = true;
            if (!(item->flags & QMF_B2_LOCK534)) {
                if (item->flags & QMF_GRAYED) item->flags |= QMF_B2_WAS_GREY534;
                item->flags |= QMF_B2_LOCK534 | QMF_GRAYED;
            }
            if (uis.mouseTracker == item) uis.mouseTracker = NULL;
        } else if (item->flags & QMF_B2_LOCK534) {
            if (!(item->flags & QMF_B2_WAS_GREY534)) item->flags &= ~QMF_GRAYED;
            item->flags &= ~(QMF_B2_LOCK534 | QMF_B2_WAS_GREY534);
        }
    }
    return any;
}

static void B2VR_Dependencies(menuFrameWork_t *menu)
{
    B2VR_StockDependencies534(menu);
    if (menu && menu->name && !strcmp(menu->name,"b2_graphics")) {B2G_Dependencies(menu);return;}
    if (!B2VR_Menu(menu) || menu->nitems != 13) return;
    const bool smooth = ((menuSpinControl_t *)menu->items[0])->curvalue == 1;
    const bool teleport = ((menuSpinControl_t *)menu->items[4])->curvalue == 1;
    const int slots[] = {1,2,5,6,7};
    const bool enabled[] = {smooth,!smooth,teleport,teleport,teleport};
    for (int i=0;i<5;i++) {
        menuCommon_t *item = menu->items[slots[i]];
        if (enabled[i]) item->flags &= ~QMF_GRAYED;
        else {
            item->flags |= QMF_GRAYED;
            if (uis.mouseTracker == item) uis.mouseTracker = NULL;
            if (item->flags & QMF_HASFOCUS)
                Menu_SetFocus(menu->items[slots[i]>=5 ? 4 : 0]);
        }
    }
}

static menuSound_t B2VR_Changed(menuCommon_t *item)
{
    B2VR_Dependencies(item->parent);
    return QMS_NOTHANDLED;
}

static void B2VR_Spin(menuFrameWork_t *menu,const char *label,const char *cvar,
    const char *initial,const char *zero,const char *one)
{
    menuSpinControl_t *s = UI_Mallocz(sizeof(*s));
    s->generic.type = MTYPE_SPINCONTROL;
    s->generic.name = UI_CopyString(label);
    s->generic.change = B2VR_Changed;
    s->cvar = Cvar_Get(cvar,initial,CVAR_ARCHIVE);
    s->numItems = 2;
    s->itemnames = UI_Mallocz(3*sizeof(char *));
    s->itemnames[0] = UI_CopyString(zero);
    s->itemnames[1] = UI_CopyString(one);
    Menu_AddItem(menu,s);
}

static void B2VR_Slider(menuFrameWork_t *menu,const char *label,const char *cvar,
    const char *initial,float low,float high,float step,const char *format)
{
    menuSlider_t *s = UI_Mallocz(sizeof(*s));
    s->generic.type = MTYPE_SLIDER;
    s->generic.name = UI_CopyString(label);
    s->cvar = Cvar_Get(cvar,initial,CVAR_ARCHIVE);
    s->minvalue=low; s->maxvalue=high; s->step=step;
    s->format=UI_CopyString(format);
    Menu_AddItem(menu,s);
}

static void B2VR_ResetSlider320(menuFrameWork_t *menu,int index,float value)
{
    menuSlider_t *slider=menu->items[index];
    slider->curvalue=value;
    slider->modified=true;
}

static void B2VR_AddReset320(menuFrameWork_t *menu,menuSound_t (*reset)(menuCommon_t *))
{
    menuAction_t *row=UI_Mallocz(sizeof(*row));
    row->generic.type=MTYPE_ACTION;
    row->generic.name=UI_CopyString("Reset to Defaults");
    row->generic.activate=reset;
    Menu_AddItem(menu,row);
}

static menuSound_t B2VR_ResetComfort320(menuCommon_t *item)
{
    menuFrameWork_t *menu=item->parent;
    ((menuSpinControl_t*)menu->items[0])->curvalue=1; // Smooth turning at 360 degrees/sec; snap remains selectable.
    B2VR_ResetSlider320(menu,1,360);
    B2VR_ResetSlider320(menu,2,45);
    ((menuSpinControl_t*)menu->items[3])->curvalue=0; // Smooth movement.
    for(int i=4;i<=6;i++)((menuSpinControl_t*)menu->items[i])->curvalue=1;
    ((menuSpinControl_t*)menu->items[7])->curvalue=0; // Telefragging.
    ((menuSpinControl_t*)menu->items[8])->curvalue=1; // Teleport only; other vignette modes remain selectable.
    B2VR_ResetSlider320(menu,9,.90f);
    B2VR_ResetSlider320(menu,10,.25f);
    B2VR_Dependencies(menu);
    Menu_SetFocus(item);
    menu->status="Comfort defaults restored. Close this panel to apply.";
    return QMS_IN;
}

static menuSound_t B2VR_Action(menuCommon_t *item)
{
    Cbuf_AddText(&cmd_buffer,((menuAction_t *)item)->cmd);
    Cbuf_AddText(&cmd_buffer,"\n");
    return QMS_NOTHANDLED;
}

#include "b2_graphics_menu466.inc"
#include "../../controller-labels412/source/bindings412.inc"

// The script has already created the ordinary keyboard Controls list.
// Identify it by its stock Fire keybind, not by translated menu labels.
static void B2VR_KeyboardBinding530(menuFrameWork_t *menu,const char *label,const char *command)
{
    int insert=0;
    for (int i=0;i<menu->nitems;i++) {
        menuKeybind_t *existing=menu->items[i];
        if (existing->generic.type!=MTYPE_KEYBIND) continue;
        if (!Q_stricmp(existing->cmd,command)) return;
        insert=i+1;
    }
    menuKeybind_t *row=UI_Mallocz(sizeof(*row));
    row->generic.type=MTYPE_KEYBIND;
    row->generic.name=UI_CopyString(label);
    row->generic.uiFlags=UI_CENTER;
    row->generic.status=UI_CopyString("Press Enter to change, Backspace to clear");
    row->cmd=UI_CopyString(command);
    row->altstatus=UI_CopyString("Press the desired key, Escape to cancel");
    Menu_AddItem(menu,row);
    memset(&row->generic.condition,0,sizeof(row->generic.condition));
    // Keep any existing Back/reset actions at the end, in their own order.
    for (int i=menu->nitems-1;i>insert;i--) menu->items[i]=menu->items[i-1];
    menu->items[insert]=row;
}

static void B2VR_KeyboardBindings530(void)
{
    menuFrameWork_t *menu;
    LIST_FOR_EACH(menuFrameWork_t,menu,&ui_menus,entry) {
        for (int i=0;i<menu->nitems;i++) {
            menuKeybind_t *row=menu->items[i];
            if (row->generic.type!=MTYPE_KEYBIND || Q_stricmp(row->cmd,"+attack")) continue;
            B2VR_KeyboardBinding530(menu,"Recentre view","+b2_recenter");
            B2VR_KeyboardBinding530(menu,"Switch controllers / keyboard and mouse","b2_controller_handoff");
            break;
        }
    }
}

bool UI_B2KeyboardInput530(void)
{
    menuFrameWork_t *menu=uis.activeMenu;
    if (!menu) return false;
    if (menu->keywait) return true;
    menuCommon_t *item=Menu_ItemAtCursor(menu);
    return item && item->type==MTYPE_FIELD;
}

void UI_B2VRInit(void)
{
    B2VR_KeyboardBindings530();
    UI_B2GInit();
    menuFrameWork_t *options = UI_FindMenu("options");
    if (!options || UI_FindMenu("b2_vr")) return;
    menuFrameWork_t *menu = UI_Mallocz(sizeof(*menu));
    menu->name=UI_CopyString("b2_vr"); menu->title=UI_CopyString("VR Comfort Settings");
    menu->push=Menu_Push; menu->pop=Menu_Pop; menu->free=Menu_Free;
    menu->image=options->image; menu->color=options->color;
    menu->transparent=options->transparent;
    menu->compact=true; // Same stock style as HDR/resolution submenus; only this panel.
    B2VR_Spin(menu,"Turning","b2_controller_turn_mode","1","Snap","Smooth");
    B2VR_Slider(menu,"Smooth deg/sec","b2_controller_smooth_speed","360",45,540,45,"%.0f");
    B2VR_Slider(menu,"Snap degrees","b2_controller_snap_angle","45",15,180,15,"%.0f");
    B2VR_Spin(menu,"Movement stick","b2_teleport_only293","0","Smooth","Teleport only");
    B2VR_Spin(menu,"Teleport","b2_teleport","1","Off","On");
    B2VR_Spin(menu,"Teleport sound","b2_teleport_sound301","1","Off","On");
    B2VR_Spin(menu,"Teleport particles","b2_teleport_particles301","1","Off","On");
    B2VR_Spin(menu,"Telefragging","b2_telefrag","0","Off","On");
    B2VR_Spin(menu,"Vignette applies to","b2_comfort_vignette","1","Off","Teleport only");
    menuSpinControl_t *mode=menu->items[8];
    for(int i=0;i<2;i++)Z_Free(mode->itemnames[i]);
    Z_Free(mode->itemnames);mode->itemnames=UI_Mallocz(5*sizeof(char*));mode->numItems=4;
    const char *names[]={"Off","Teleport only","Movement / turning","Both"};
    for(int i=0;i<4;i++)mode->itemnames[i]=UI_CopyString(names[i]);
    B2VR_Slider(menu,"Vignette strength","b2_comfort_vignette_strength","0.9",0,1,.05,"%.2f");
    B2VR_Slider(menu,"Vignette radius","b2_comfort_radius304","0.25",.1,.75,.05,"%.2f");
    ((menuCommon_t*)menu->items[3])->status=UI_CopyString("Teleport only: aim/release; hold Walk/Run + forward for a slow nudge");
    ((menuCommon_t*)menu->items[9])->status=UI_CopyString("Border darkness: 0 is clear, 1 is black at the outer edge");
    ((menuCommon_t*)menu->items[10])->status=UI_CopyString("Clear radius as a fraction of the peripheral half-view; smaller is stronger");
    B2VR_AddReset320(menu,B2VR_ResetComfort320);
    menuAction_t *back=UI_Mallocz(sizeof(*back));
    back->generic.type=MTYPE_ACTION; back->generic.name=UI_CopyString("Back");
    back->generic.activate=B2VR_Action;
    back->cmd=UI_CopyString("popmenu"); Menu_AddItem(menu,back);
    List_Append(&ui_menus,&menu->entry);
    menuAction_t *link=UI_Mallocz(sizeof(*link));
    link->generic.type=MTYPE_ACTION; link->generic.name=UI_CopyString("VR Comfort Settings");
    link->generic.uiFlags=UI_CENTER;
    link->generic.activate=B2VR_Action;
    link->cmd=UI_CopyString("pushmenu b2_vr"); Menu_AddItem(options,link);
    // Place VR Comfort Settings first, leaving all existing Options entries in their own order.
    memmove(options->items+1,options->items,(options->nitems-1)*sizeof(void *));
    options->items[0]=link;
    UI_B2BindingsInit();
}

static bool B2VR_Credits(const menuFrameWork_t *menu)
{
    return menu->footer && menu->name &&
        (!strcmp(menu->name,"main") || !strcmp(menu->name,"game"));
}

static void B2VR_DrawCredits(menuFrameWork_t *menu)
{
    if (!B2VR_Credits(menu)) return;
    // Exact selected 600x64 footer: left logo x=10..255, right x=298..558.
    // One pixel larger than253; only text contracts horizontally on narrow windows.
    const float fit=menu->footer_rc.width/300.f;
    const vrect_t *r=&menu->footer_rc;
    const float left=r->x+r->width*(132.5f/600.f)-4.f*fit; // Four pixels keep the enlarged columns separate.
    const float right=r->x+r->width*(428.f/600.f)+3.f*fit; //253 +35px minus4 new8px advances
    const float text_scale=min(1.f*fit,min((left-4.f)/(9.5f*CHAR_WIDTH),
        (uis.width-4.f-right)/(9.5f*CHAR_WIDTH)));
    int y=r->y+r->height+(int)ceilf(3.f*fit+CHAR_HEIGHT*text_scale);
    R_SetScale(uis.scale/text_scale);
    const int left_centre=(int)(left/text_scale)-CHAR_WIDTH/2-2*CHAR_WIDTH;
    UI_DrawString(left_centre,
        (int)(y/text_scale),UI_CENTER,"VR port by");
    UI_DrawString(left_centre,
        (int)(y/text_scale)+CHAR_HEIGHT,UI_CENTER,"Togorian");
    R_SetScale(uis.scale);
    B2L_Draw(menu);
}

static bool B2VR_SnapSlider(const menuSlider_t *s)
{
    return B2VR_Menu(s->generic.parent) && s->cvar && !strcmp(s->cvar->name,"b2_controller_snap_angle");
}
static float B2VR_SliderPosition(const menuSlider_t *s)
{
    return B2VR_SnapSlider(s)?b2_snap_index256(s->curvalue)/4.f:
        Q_clipf((s->curvalue-s->minvalue)/(s->maxvalue-s->minvalue),0,1);
}
