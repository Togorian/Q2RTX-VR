// Modified 14 November 2026: selected Quake II RTX/Q2RTX VR production code
// adapted into a CPU fixture; copied paths and spans are listed in its manifest.
// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Retained CPU fixture: selected production spans are verified by rc7-regression.py.
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#define Q_stricmp _stricmp
enum{KEY_GAME=1,KEY_CONSOLE=2,KEY_MESSAGE=4,K_PAUSE=130,K_ESCAPE=27,K_ENTER=13,K_ALT=131,K_F1=140,K_F12=151,K_END=152};
static struct{int key_dest;}cls={KEY_GAME};static struct{bool ui_interrupted,stopped;}b2_client;
static bool enabled=true,realign,held;static const char*bindings[256];
static void b2_recenter_up_f(void){}static void b2_recenter_down_f(void){}
static bool B2_UIEnabled(void){return enabled;}static bool B2_RealignActive391(void){return realign;}
static void b2_realign_key391(bool b){}static const char*Key_GetBindingForKey(unsigned k){return bindings[k];}
static bool UI_B2KeyboardInput530(void){return false;}static int Key_IsDown(unsigned k){return k==K_ALT?0:1;}
static void b2_request_interruption_resume(bool b){}static void B2_UITogglePause(void){}
static bool B2_UISimulationHeld(void){return held;}
bool B2_UIKey(unsigned key, bool down)
{
    // Track the physical press, even if its binding or destination changes.
    // Release must also work after focus loss, realignment or VR shutdown.
    static bool recenter_keys530[256];
    if (!down && recenter_keys530[key]) {
        recenter_keys530[key]=false;
        for (unsigned i=0;i<256;i++) if (recenter_keys530[i]) return true;
        b2_recenter_up_f();
        return true;
    }
    if (down && recenter_keys530[key]) return true;
    if (!B2_UIEnabled()) return false;
    if(B2_RealignActive391() && key==K_PAUSE){b2_realign_key391(down);return true;}
    if(B2_RealignActive391())return true;
    const char *binding=Key_GetBindingForKey(key);
    if (binding && !Q_stricmp(binding,"+b2_recenter") &&
        !(cls.key_dest&(KEY_CONSOLE|KEY_MESSAGE)) && !UI_B2KeyboardInput530() &&
        key!=K_ESCAPE && key!='`' && key!='~' && key!=K_PAUSE &&
        !(key==K_ENTER && Key_IsDown(K_ALT))) {
        // Consume the ordinary +command route, which would skip a cinematic.
        // The existing request function still owns every focus/pose/state gate.
        if (down && Key_IsDown(key)==1) {
            recenter_keys530[key]=true;
            b2_recenter_down_f();
        }
        return true;
    }
    if (key == K_PAUSE && b2_client.ui_interrupted) {
        if (down && Key_IsDown(key) == 1 && !b2_client.stopped) {
            b2_request_interruption_resume(false);
        }
        return true;
    }
    if (key == K_PAUSE) {
        if (down && Key_IsDown(key) == 1) B2_UITogglePause();
        return true;
    }
    if (key == K_ENTER && Key_IsDown(K_ALT)) {
        if (down && Key_IsDown(key) == 1) Com_Printf("B2UI: display changes are unavailable in this session.\n");
        return true;
    }
    // Stock F-key actions and user non-button rebindings use ordinary dispatch.
    // Held gameplay button bindings still remain behind the simulation hold.
    if (key >= K_F1 && key <= K_F12) {
        const char *binding=Key_GetBindingForKey(key);
        return B2_UISimulationHeld() && binding && binding[0]=='+';
    }
    if (B2_UISimulationHeld() && cls.key_dest == KEY_GAME &&
        key != K_ESCAPE && key != '`' && key != '~' &&
        !(binding && !Q_stricmp(binding,"b2_controller_handoff"))) return true;
    return false;
}
int main(void){for(int hold=0;hold<3;hold++){held=hold!=0;b2_client.ui_interrupted=hold==1;
bindings[K_END]="b2_controller_handoff";bindings['h']="b2_controller_handoff";
for(int down=0;down<2;down++){assert(!B2_UIKey(K_END,down));assert(!B2_UIKey('h',down));}
bindings['h']="B2_CONTROLLER_HANDOFF";assert(!B2_UIKey('h',true));
const char*bad[]={"+attack","b2_controller_handoff; +attack","echo b2_controller_handoff","b2_controller_handoff_extra"};
for(int n=0;n<4;n++){bindings['h']=bindings[K_END]=bad[n];assert(B2_UIKey('h',true)==held);assert(B2_UIKey(K_END,true)==held);}
}realign=true;bindings['h']="b2_controller_handoff";assert(B2_UIKey('h',true));
puts("actual UIKey: End/rebound H/case normal and held/manual-pause dispatch, unrelated End and chained commands suppressed PASS");}
