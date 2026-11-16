// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
#ifndef B2_BINDINGS_TABLE287
#define B2_BINDINGS_TABLE287
// SPDX-License-Identifier: GPL-2.0-or-later
static const char *const b2b_names[B2B_COUNT]={"Move","Turn","Fire / respawn","Jump","Crouch","Teleport hold/release","Use","Weapon / item selector","Recenter","Menu / back","Additional menu","Interruption resume","Walk / run (tap)","Open selector on Items","Stock inventory","Mission helper","Mission / inventory cycle"};
static const char *const b2b_inputs[26]={"Left stick","Left trigger","Left grip","Left primary","Left secondary","Left stick click","Right stick","Right trigger","Right grip","Right primary","Right secondary","Right stick click","Dominant stick","Dominant trigger","Dominant grip","Dominant primary","Dominant secondary","Dominant stick click","Off-hand stick","Off-hand trigger","Off-hand grip","Off-hand primary","Off-hand secondary","Off-hand stick click","Togorian use (X / B)","Unbound"};
static int b2b_resolve(int source,unsigned dominant)
{ if(source==25)return -1;if(source==24)return dominant?3:10;int hand=source/B2B_PARTS;return ((hand==2)?dominant:(hand==3)?1-dominant:hand)*B2B_PARTS+source%B2B_PARTS; }
static void b2b_preset(b2b_config_t *c,int preset)
{
    static const int base[B2B_COUNT]={18,12,13,9,20,19,24,14,17,16,4,22,23,25,25,25,4};
    c->preset=preset;memcpy(c->input,base,sizeof(base));
    if (preset == 1) {
        c->input[B2B_CROUCH] = 15;
        c->input[B2B_JUMP] = 16;
        c->input[B2B_ITEMS] = 20;
        c->input[B2B_INVENTORY] = 21;
        c->input[B2B_HELP] = 22;
        c->input[B2B_CYCLE347] = 25;
        c->input[B2B_USE] = c->input[B2B_MENU] = c->input[B2B_EXTRA] = 25;
    }
}
static int b2b_source(const b2b_config_t *c,unsigned action,unsigned dominant)
{
    return b2b_resolve(c->input[action],dominant);
}
static bool b2b_type(unsigned action,int source)
{
    if(source==25)return action>=B2B_RUN || action==B2B_USE || action==B2B_MENU || action==B2B_EXTRA;
    if(source==24)return action==B2B_USE;
    if(source<0 || source>=24)return false;
    int part=source%B2B_PARTS;
    if(action==B2B_MOVE || action==B2B_TURN)return part==B2B_STICK;
    if(action==B2B_FIRE || action==B2B_TELEPORT)return part==B2B_TRIGGER;
    if(action==B2B_SELECT || action==B2B_ITEMS)return part==B2B_GRIP;
    if(action==B2B_CROUCH && part==B2B_GRIP)return true;
    // Primary is fixed inventory confirmation; only choices that can complete
    // all three cycle presses are offered for this new action.
    if(action==B2B_CYCLE347)return part==B2B_SECONDARY || part==B2B_CLICK;
    return part==B2B_PRIMARY || part==B2B_SECONDARY || part==B2B_CLICK;
}
static bool b2b_validate(const b2b_config_t *c)
{
    if(c->preset<0 || c->preset>2)return false;
    for(unsigned a=0;a<B2B_COUNT;a++)if(!b2b_type(a,c->input[a]))return false;
    for(unsigned d=0;d<2;d++){
        for(unsigned a=0;a<B2B_COUNT;a++)for(unsigned b=0;b<a;b++){
            if(a==B2B_RESUME || b==B2B_RESUME || c->input[a]==25 || c->input[b]==25)continue;
            if(a==B2B_EXTRA && b==B2B_MENU)continue; // Same deliberate menu action.
            // A shared cycle input owns play/stock overlays. Its menu binding
            // remains Back in ordinary menus and its extra shortcut serves films.
            // The same contextual mapping is valid when copied into Custom.
            if(a==B2B_CYCLE347 && (b==B2B_MENU || b==B2B_EXTRA))continue;
            if(b2b_source(c,a,d)==b2b_source(c,b,d))return false;
        }
        // Menu/extra also operate beside fixed dominant-primary confirmation/skip.
        for(unsigned a=B2B_MENU;a<=B2B_EXTRA;a++)
            if(b2b_source(c,a,d)==(int)d*6+B2B_PRIMARY)return false;
    }
    return true;
}
static float b2b_value(const b2c_snapshot_t *p,int source,unsigned axis)
{
    if(source<0 || source>=12)return 0;
    const b2c_hand_t *h=&p->hands[source/6];
    switch(source%6){case B2B_STICK:return h->stick[axis&1];case B2B_TRIGGER:return h->trigger;
    case B2B_GRIP:return h->squeeze;case B2B_PRIMARY:return h->primary;
    case B2B_SECONDARY:return h->secondary;default:return h->click;}
}
static void b2b_clear(b2c_snapshot_t *p,int source)
{
    if(source<0 || source>=12)return;
    b2c_hand_t *h=&p->hands[source/6];
    switch(source%6){case B2B_STICK:h->stick[0]=h->stick[1]=0;break;case B2B_TRIGGER:h->trigger=0;break;
    case B2B_GRIP:h->squeeze=0;break;case B2B_PRIMARY:h->primary=false;break;
    case B2B_SECONDARY:h->secondary=false;break;default:h->click=false;}
}
// One atomic archived value; code24 retains the contextual Togorian Use exception.

#endif
