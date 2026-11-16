// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// The same stock scrollbar hit regions; thumb dragging and wheel events stay stock.
static int B2UI_ScrollZone304(menuList_t *l)
{
 if(!(l->mlFlags&MLF_SCROLLBAR) || l->numItems<=l->maxItems)return 0;
 int x=l->generic.rect.x+l->generic.rect.width-MLIST_SCROLLBAR_WIDTH;
 int y=l->generic.rect.y+MLIST_SPACING,h=l->generic.height;
 if(l->mlFlags&MLF_HEADER){y+=MLIST_SPACING;h-=MLIST_SPACING;}
 int bar=h-2*MLIST_SPACING;
 int page=Q_rint(bar*(float)l->maxItems/l->numItems);
 int pre=Q_rint(bar*(float)l->prestep/l->numItems);
 vrect_t r={x,y,MLIST_SCROLLBAR_WIDTH,pre};if(UI_CursorInRect(&r))return -2;
 r.y=y+pre+page;r.height=bar-pre-page;if(UI_CursorInRect(&r))return 2;
 r.y=y-MLIST_SPACING;r.height=MLIST_SPACING;if(UI_CursorInRect(&r))return -1;
 r.y=l->generic.rect.y+l->generic.height-MLIST_SPACING;if(UI_CursorInRect(&r))return 1;
 return 0;
}
static void B2UI_ScrollStart304(menuList_t *l,int zone)
{
 if(!b2vr_stepping && Key_IsDown(K_MOUSE1)==1){
  b2vr_repeat.menu=l->generic.parent;b2vr_repeat.item=&l->generic;
  b2vr_repeat.owner=3;b2vr_repeat.key=zone;
  b2vr_repeat.started=b2vr_repeat.stepped=Sys_Milliseconds();
 }
}
static void B2UI_ScrollTick304(uint32_t now)
{
 if(b2vr_repeat.owner!=3)return;
 menuList_t *l=(menuList_t*)b2vr_repeat.item;
 if(!Key_IsDown(K_MOUSE1) || uis.activeMenu!=b2vr_repeat.menu || uis.mouseTracker ||
    !l || Menu_ItemAtCursor(uis.activeMenu)!=&l->generic ||
    !UI_IsItemSelectable(&l->generic) || B2UI_ScrollZone304(l)!=b2vr_repeat.key){UI_B2VRRepeatCancel();return;}
 uint32_t held=now-b2vr_repeat.started;unsigned interval=held>=2000?45:held>=1000?80:150;
 if(held<400 || now-b2vr_repeat.stepped<interval)return;
 b2vr_repeat.stepped=now;
 int old=l->prestep,zone=b2vr_repeat.key;
 l->prestep+=(zone==2 || zone==-2)?zone/2*l->maxItems:zone;
 MenuList_ValidatePrestep(l);if(old!=l->prestep)UI_StartSound(QMS_MOVE);
}
