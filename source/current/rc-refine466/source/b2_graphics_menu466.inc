// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
static void B2G_Label(menuFrameWork_t *menu,int index,const char *text)
{
    menuCommon_t *item=menu->items[index];
    if(item->name && !strcmp(item->name,text))return;
    Z_Free(item->name);item->name=UI_CopyString(text);
}
// One non-selectable blank line after scale, Pending, and Applied groups.
static bool B2G_ReadoutGap466(const menuFrameWork_t *menu,int index)
{
    return menu && menu->name && !strcmp(menu->name,"b2_graphics") &&
        menu->nitems==14 && (index==0 || index==1 || index==4);
}
static void B2G_Output462(char *text,size_t size,const char *phase,const char *category,
    const b2g_info_t *info,unsigned eye,bool pending)
{
    const char *fixed=pending?" (fixed)":"";
    if(info->output_width[eye]==info->output_width[eye+1] &&
       info->output_height[eye]==info->output_height[eye+1])
        Q_snprintf(text,size,"%s %s: %ux%u%s",phase,category,
            info->output_width[eye],info->output_height[eye],fixed);
    else
        Q_snprintf(text,size,"%s %s: %ux%u / %ux%u (unequal)%s",phase,category,
            info->output_width[eye],info->output_height[eye],
            info->output_width[eye+1],info->output_height[eye+1],fixed);
}
static void B2G_Dependencies(menuFrameWork_t *menu)
{
    if(menu->nitems!=14)return;
    b2g_info_t info;char text[128];B2G_Info(&info);
    uint32_t applied_w,applied_h,w,h;
    bool ready=info.applied && B2G_Pending442(info.percent,&applied_w,&applied_h);
    float value=((menuSlider_t*)menu->items[0])->curvalue;
    bool pending=ready && isfinite(value) && value>=10 && value<=100 &&
        value==(int)value && B2G_Pending442((int)value,&w,&h);
    if(pending)Q_snprintf(text,sizeof(text),"Pending render: %ux%u each",w,h);
    else Q_snprintf(text,sizeof(text),ready?"Pending after restart: unavailable/invalid":"Pending after restart: session not ready");
    B2G_Label(menu,1,text);
    if(ready)Q_snprintf(text,sizeof(text),"Applied render: %ux%u each (%d%%)",info.width,info.height,info.percent);
    else Q_snprintf(text,sizeof(text),"Applied: session not ready");
    B2G_Label(menu,2,text);
    // Output rectangles are current runtime targets, never scaled by this slider.
    // Quad order is Focus then Peripheral; stereo has no such two categories.
    const char *label="Applied";int first=3;
    if(info.views==4) {
        for(unsigned row=0;row<2;row++) {
            const char *category=row?"Peripheral":"Focus";
            if(ready)B2G_Output462(text,sizeof(text),label,category,&info,row?0:2,false);
            else Q_snprintf(text,sizeof(text),"%s %s: unavailable",label,category);
            B2G_Label(menu,first+row,text);
        }
    } else {
        if(ready && info.views==2)B2G_Output462(text,sizeof(text),label,"Stereo output",&info,0,false);
        else Q_snprintf(text,sizeof(text),"%s Stereo output: unavailable",label);
        B2G_Label(menu,first,text);
        Q_snprintf(text,sizeof(text),"%s Focus / Peripheral: not used",label);
        B2G_Label(menu,first+1,text);
    }
}
static bool B2G_Push(menuFrameWork_t *menu)
{
    cvar_t *scale=Cvar_Get("b2_render_percent","100",CVAR_ARCHIVE);int percent;
    if(B2G_Percent364(scale->value,scale->integer,&percent) && scale->integer!=percent)
        Cvar_SetByVar(scale,"100",FROM_CODE);
    menuCommon_t *focus=Menu_ItemAtCursor(menu);
    menu->status=focus?focus->status:NULL;
    // Stock push loads controls first, then invokes B2G_Dependencies.
    return Menu_Push(menu);
}
static void B2G_BloomLive312(menuSlider_t *s);
static menuSound_t B2G_Reset320(menuCommon_t *item)
{
    menuFrameWork_t *menu=item->parent;
    B2VR_ResetSlider320(menu,0,100);
    ((menuSpinControl_t*)menu->items[5])->curvalue=0; // Show FPS: No.
    ((menuSpinControl_t*)menu->items[6])->curvalue=0; // Off in the FPS pairs table.
    B2VR_ResetSlider320(menu,7,.037f);
    B2VR_ResetSlider320(menu,8,.002f);
    B2G_BloomLive312(menu->items[7]);
    B2G_BloomLive312(menu->items[8]);
    ((menuSpinControl_t*)menu->items[11])->curvalue=1; // Stabilised.
    B2G_Dependencies(menu);
    Menu_SetFocus(item);
    menu->status="Graphics defaults restored. Resolution applies at next launch.";
    return QMS_IN;
}
static void UI_B2GInit(void)
{
    menuFrameWork_t *video=UI_FindMenu("video"),*comfort=UI_FindMenu("options");
    if(!video || !comfort || UI_FindMenu("b2_graphics"))return;
    menuFrameWork_t *menu=UI_Mallocz(sizeof(*menu));
    menu->name=UI_CopyString("b2_graphics");menu->title=UI_CopyString("VR Graphics Settings");
    menu->push=B2G_Push;menu->pop=Menu_Pop;menu->free=Menu_Free;
    menu->image=comfort->image;menu->color=comfort->color;menu->transparent=comfort->transparent;menu->compact=true;
    B2VR_Slider(menu,"Pending scale %","b2_render_percent","100",10,100,5,"%.0f%%");
    for(int i=0;i<4;i++) {
        menuAction_t *row=UI_Mallocz(sizeof(*row));row->generic.type=MTYPE_ACTION;
        row->generic.name=UI_CopyString("Session information");
        row->generic.flags=QMF_GRAYED;row->generic.uiFlags=UI_CENTER;Menu_AddItem(menu,row);
    }
    B2VR_Spin(menu,"Show FPS","b2_showfps","0","No","Yes");
    ((menuCommon_t*)menu->items[5])->status=UI_CopyString("Game frames per second and average milliseconds per frame.");
    B2VR_Spin(menu,"Frame-rate limit","r_maxfps","0","Off","30");
    menuSpinControl_t *fps=menu->items[6];
    for(int i=0;i<2;i++)Z_Free(fps->itemnames[i]);Z_Free(fps->itemnames);
    fps->generic.type=MTYPE_PAIRS;fps->numItems=8;
    fps->itemnames=UI_Mallocz(9*sizeof(char*));fps->itemvalues=UI_Mallocz(9*sizeof(char*));
    const char *values[]={"0","30","45","60","75","90","105","120"};
    for(int i=0;i<8;i++){fps->itemnames[i]=UI_CopyString(i?values[i]:"Off");fps->itemvalues[i]=UI_CopyString(values[i]);}
    ((menuCommon_t*)menu->items[0])->status=UI_CopyString("10-100% of largest runtime view envelope; applies at restart");
    ((menuCommon_t*)menu->items[6])->status=UI_CopyString("Limits game rendering, not headset refresh. Off removes this cap.");
    B2VR_Slider(menu,"Bloom radius","bloom_sigma","0.037", .001f,.050f,.001f,"%.3f");
    B2VR_Slider(menu,"Bloom intensity","bloom_intensity","0.002",0,.010f,.0005f,"%.4f");
    ((menuCommon_t*)menu->items[7])->status=UI_CopyString("Glow spread as a fraction of view height; default 0.037. Applies live.");
    ((menuCommon_t*)menu->items[8])->status=UI_CopyString("Glow strength; default 0.002, zero is off. Applies live, including menus and water.");
    menuSeparator_t *gap=UI_Mallocz(sizeof(*gap));gap->generic.type=MTYPE_SEPARATOR;Menu_AddItem(menu,gap);
    menuSeparator_t *heading=UI_Mallocz(sizeof(*heading));
    heading->generic.type=MTYPE_SEPARATOR;heading->generic.name=UI_CopyString("Quad Views");
    heading->generic.uiFlags=UI_CENTER;Menu_AddItem(menu,heading);
    B2VR_Spin(menu,"Processing","b2_quad_style293","1","Standard","Stabilised");
    ((menuCommon_t*)menu->items[11])->status=UI_CopyString("Stabilised can reduce dark focus edges; lighting may look less natural.");
    B2VR_AddReset320(menu,B2G_Reset320);
    ((menuCommon_t*)menu->items[12])->status=UI_CopyString("Restore the default VR graphics settings.");
    menuAction_t *back=UI_Mallocz(sizeof(*back));back->generic.type=MTYPE_ACTION;
    back->generic.name=UI_CopyString("Back");back->generic.activate=B2VR_Action;back->cmd=UI_CopyString("popmenu");Menu_AddItem(menu,back);
    List_Append(&ui_menus,&menu->entry);
    menuAction_t *link=UI_Mallocz(sizeof(*link));link->generic.type=MTYPE_ACTION;
    link->generic.name=UI_CopyString("VR Graphics Settings");link->generic.uiFlags=UI_CENTER;
    link->generic.activate=B2VR_Action;link->cmd=UI_CopyString("pushmenu b2_graphics");Menu_AddItem(video,link);
    memmove(video->items+1,video->items,(video->nitems-1)*sizeof(void*));video->items[0]=link;
    // Set the initial choice once; normal navigation/re-entry keeps its focus.
    Menu_SetFocus(&link->generic);
}

static bool B2G_VideoGap(const menuFrameWork_t *menu)
{
    if(!menu || !menu->name || strcmp(menu->name,"video") || !menu->nitems)return false;
    const menuCommon_t *first=menu->items[0];
    return first && first->name && !strcmp(first->name,"VR Graphics Settings") && !(first->flags&QMF_HIDDEN);
}

static const char *B2G_Footer327(menuFrameWork_t *menu)
{
    if (menu->name && !strcmp(menu->name,"b2_graphics") && menu->nitems==14) {
        menuCommon_t *focus=Menu_ItemAtCursor(menu);
        if (focus==menu->items[11])
            return ((menuSpinControl_t*)focus)->curvalue ? focus->status :
                "Standard uses the original Quad Views processing.";
        if (focus!=menu->items[12])
            return focus ? focus->status : NULL;
    }
    return menu->status;
}

// Only these sliders commit immediately; the existing controls keep their behaviour.
static bool B2G_BloomSlider316(const menuSlider_t *s)
{
    return s->generic.parent && s->generic.parent->name &&
        !strcmp(s->generic.parent->name,"b2_graphics") && s->cvar &&
        (!strcmp(s->cvar->name,"bloom_sigma") || !strcmp(s->cvar->name,"bloom_intensity"));
}

static void B2G_BloomLive312(menuSlider_t *s)
{
    if (!B2G_BloomSlider316(s)) return;
    char text[16];
    snprintf(text,sizeof(text),s->format,s->curvalue);
    Cvar_SetByVar(s->cvar,text,FROM_MENU);
}
