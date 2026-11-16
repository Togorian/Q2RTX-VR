# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
"""Check retired tracked-shot recovery against the selected stock weapon code.

Run from an x64 MSVC developer command prompt:
python -I -B -S tools/collision-fire-regression.py --output C:/scratch/fire-checks

The output directory must be new and outside the source tree. Actual production
functions are extracted for both the fixed case and the pre-fix negative control.
No game, headset, renderer or network is started.
"""
from pathlib import Path
import argparse
import hashlib
import json
import runpy
import subprocess

FIRE = 'source/baseline/b2-consolidated215/inputs/origin-044/include/refresh/b2_weapon_fire.inc'
WEAPON = 'source/baseline/b2-consolidated215/inputs/origin-005/src/game/p_weapon.c'
BASELINE = '4e829ea926d3f72dee481ac5dbf02e7d5e1053f8'
BEFORE = 'tools/collision-fire-before.c.in'
BEFORE_SHA256 = 'cc0efc9e0edc7b6890c6563f12e21a0be30702f7f88de8d4b5796d38db90349c'

PREFIX = r'''
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static void fail(const char *s,int line){fprintf(stderr,"FAIL line %d: %s\n",line,s);exit(86);}
#define assert(x) ((x)?(void)0:fail(#x,__LINE__))
typedef float vec3_t[3];
typedef struct {int value;} epochs_t;
typedef struct {epochs_t epochs;} pose_t;
typedef struct {pose_t pose;uint32_t game_epoch;int weapon;bool attack;} b2w_packet_t;
typedef struct {int binding;} b2we_owner_t;
typedef struct {uint64_t cycle;int state;} b2wg_state_t;
typedef struct {int quantity;} gitem_t;
typedef struct {
  struct {const gitem_t *weapon;int inventory[32];} pers;
  struct {int gunframe;struct {int pm_flags;} pmove;} ps;
  const gitem_t *newweapon;
  int buttons,oldbuttons,latched_buttons,weaponstate,grenade_framenum,weapon_sound;
  int machinegun_shots,ammo_index,anim_priority,anim_end,quad_framenum;
  bool grenade_blew_up;
  vec3_t v_angle,kick_origin,kick_angles;
} gclient_t;
typedef struct edict_s {
  gclient_t *client;int health,deadflag,inuse,viewheight,pain_debounce_framenum;
  struct {int modelindex,frame;vec3_t origin;} s;
} edict_t;
static edict_t entities[2],*g_edicts=entities;
static gclient_t player;
static gitem_t items[12];
static struct {int framenum;} level;
enum {BUTTON_ATTACK=1,WEAPON_READY,WEAPON_ACTIVATING,WEAPON_DROPPING,WEAPON_FIRING,
  B2WG_INTERRUPTED=10,B2WG_HELD,MODELINDEX_PLAYER=255,ANIM_REVERSE=1,ANIM_ATTACK=2,
  PMF_DUCKED=1,FRAME_crpain4=1,FRAME_crpain1=2,FRAME_pain304=3,FRAME_pain301=4,
  FRAME_crattak1=5,FRAME_crattak9=6,FRAME_attack1=7,FRAME_attack8=8,
  CHAN_ITEM=1,CHAN_WEAPON,CHAN_AUTO,CHAN_VOICE,ATTN_NORM=1,ATTN_IDLE,EF_HYPERBLASTER,
  DEFAULT_BULLET_HSPREAD=1,DEFAULT_BULLET_VSPREAD,MOD_MACHINEGUN,MOD_CHAINGUN,
  svc_muzzleflash,MZ_MACHINEGUN,MZ_CHAINGUN1,MZ_SHOTGUN,MULTICAST_PVS,PNOISE_WEAPON,
  DF_INFINITE_AMMO=1,BASE_FRAMERATE=10,DEFAULT_DEATHMATCH_SHOTGUN_COUNT=12,
  DEFAULT_SHOTGUN_COUNT=12,MOD_SHOTGUN};
#define B2W_CONTEXT_REQUESTED 1u
#define B2W_CONTEXT_COMMAND 2u
#define B2W_CONTEXT_NEW_COMMAND 4u
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
static struct {
  edict_t *player;gclient_t *client;const gitem_t *weapon;
  epochs_t epochs,armed_epochs;uint32_t game_epoch,armed_game_epoch;
  int slot;bool requested,rearm,armed,ordinary,owned,emitted,cancel,in_think,packet_valid,emission;
  bool recovering;int recovery_frame;
  bool death,death_ready,death_pending;b2w_packet_t packet;
  b2we_owner_t armed_owner,cycle_owner;b2wg_state_t grenade;vec3_t command_angles;
} b2w_fire;
static struct {bool display_emission,aim_prepared;int aim;} b2wn_previous;
static struct {bool begun;} b2we_pending;
static uint64_t b2wg_cycle;
static unsigned test_context;
static bool test_packet_valid=true,test_owner_valid=true;
static b2w_packet_t test_packet;
static b2we_owner_t test_owner;
static unsigned shot_count,change_count;
static bool is_quad;static int is_silenced;
typedef struct {float value;} cvar_t;
static cvar_t deathmatch_value,dmflags_value;
static cvar_t *deathmatch=&deathmatch_value,*dmflags=&dmflags_value;
static void noop_sound(edict_t *e,int channel,int index,float volume,float attenuation,float delay)
{(void)e;(void)channel;(void)index;(void)volume;(void)attenuation;(void)delay;}
static int noop_index(const char *s){(void)s;return 0;}
static void noop_write(int i){(void)i;}
static void noop_multicast(const float *v,int i){(void)v;(void)i;}
static struct {void (*sound)(edict_t*,int,int,float,float,float);int (*soundindex)(const char*);
  void (*WriteByte)(int);void (*WriteShort)(int);void (*multicast)(const float*,int);
} gi={noop_sound,noop_index,noop_write,noop_write,noop_multicast};
#define VectorCopy(a,b) memcpy((b),(a),sizeof(vec3_t))
#define VectorSet(v,x,y,z) ((v)[0]=(x),(v)[1]=(y),(v)[2]=(z))
#define VectorScale(a,n,b) ((b)[0]=(a)[0]*(n),(b)[1]=(a)[1]*(n),(b)[2]=(a)[2]*(n))
#define VectorAdd(a,b,c) ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
static void AngleVectors(const float *a,float *f,float *r,float *u)
{(void)a;if(f)VectorSet(f,1,0,0);if(r)VectorSet(r,0,1,0);if(u)VectorSet(u,0,0,1);}
static void P_ProjectSource(edict_t *e,const float *p,const float *d,const float *f,const float *r,float *o)
{(void)e;(void)d;(void)f;(void)r;VectorCopy(p,o);}
static float crandom(void){return 0;}
static float random(void){return 0;}
static unsigned Q_rand(void){return 1;}
static bool G_CombatObserver(edict_t *e){(void)e;return false;}
static void G_ObserverTrace(const char *s,edict_t *e){(void)s;(void)e;}
static void G_CombatInventory(edict_t *e,int i,int count,const char *s)
{(void)s;e->client->pers.inventory[i]=count;}
static void PlayerNoise(edict_t *e,const float *p,int n){(void)e;(void)p;(void)n;}
static void ChangeWeapon(edict_t *e){(void)e;change_count++;}
static void NoAmmoWeaponChange(edict_t *e){(void)e;assert(false);}
static unsigned b2w_fire_context(void){return test_context;}
static bool b2wn_state(edict_t *e,bool emission,int *out)
{(void)e;(void)emission;*out=0;return true;}
static bool b2w_fire_packet(edict_t *e,b2w_packet_t *out)
{(void)e;*out=test_packet;return test_packet_valid;}
static bool b2we_owner(edict_t *e,const b2w_packet_t *p,b2we_owner_t *out)
{(void)e;(void)p;*out=test_owner;return test_owner_valid;}
static bool b2we_binding(const b2we_owner_t *a,const b2we_owner_t *b){return a->binding==b->binding;}
// External shot sinks are counted rather than invoking the game world.
static bool B2W_WeaponMuzzle(edict_t *e,float *start,float *forward)
{(void)e;(void)start;(void)forward;return true;}
static void B2W_WeaponEmitted(edict_t *e){(void)e;shot_count++;}
static void B2W_WeaponEmissionEnd(edict_t *e){(void)e;}
#define fire_bullet(...) ((void)0)
#define fire_shotgun(...) ((void)0)
static bool Blaster_Fire(edict_t *e,const float *o,int damage,bool hyper,int effect)
{(void)e;(void)o;(void)damage;(void)hyper;(void)effect;shot_count++;return true;}
static void unexpected_fire(edict_t *e){(void)e;assert(false);}
#define Weapon_Blaster_Fire unexpected_fire
#define weapon_grenadelauncher_fire unexpected_fire
#define Weapon_RocketLauncher_Fire unexpected_fire
#define weapon_supershotgun_fire unexpected_fire
#define weapon_railgun_fire unexpected_fire
#define weapon_flaregun_fire unexpected_fire
#define FRAME_FIRE_FIRST (FRAME_ACTIVATE_LAST+1)
#define FRAME_IDLE_FIRST (FRAME_FIRE_LAST+1)
#define FRAME_DEACTIVATE_FIRST (FRAME_IDLE_LAST+1)
'''

TESTS = r'''
typedef void (*weapon_fn)(edict_t*);
static weapon_fn weapons[12]={Weapon_Blaster,Weapon_Shotgun,Weapon_SuperShotgun,
  Weapon_Machinegun,Weapon_Chaingun,NULL,Weapon_GrenadeLauncher,Weapon_RocketLauncher,
  Weapon_HyperBlaster,Weapon_Railgun,NULL,Weapon_FlareGun};
static edict_t *prepare(int slot,int frame)
{
  memset(&b2w_fire,0,sizeof(b2w_fire));memset(&b2wn_previous,0,sizeof(b2wn_previous));
  memset(entities,0,sizeof(entities));memset(&player,0,sizeof(player));
  edict_t *ent=entities+1;ent->client=&player;ent->health=100;ent->inuse=1;
  ent->viewheight=22;ent->s.modelindex=MODELINDEX_PLAYER;
  player.pers.weapon=items+slot;player.ammo_index=1;player.pers.inventory[1]=1000;
  player.weaponstate=WEAPON_FIRING;player.ps.gunframe=frame;
  player.buttons=player.latched_buttons=BUTTON_ATTACK;
  b2w_fire.player=ent;b2w_fire.client=&player;b2w_fire.weapon=player.pers.weapon;
  b2w_fire.slot=slot;b2w_fire.requested=b2w_fire.owned=b2w_fire.emitted=b2w_fire.armed=true;
  b2w_fire.epochs.value=b2w_fire.armed_epochs.value=42;
  b2w_fire.game_epoch=b2w_fire.armed_game_epoch=1;
  b2w_fire.armed_owner.binding=b2w_fire.cycle_owner.binding=1;
  test_packet=(b2w_packet_t){{{42}},1,slot,true};test_owner.binding=1;
  test_packet_valid=test_owner_valid=true;
  test_context=B2W_CONTEXT_REQUESTED|B2W_CONTEXT_COMMAND|B2W_CONTEXT_NEW_COMMAND;
  level.framenum=100;shot_count=change_count=0;return ent;
}
static void cadence(int slot,int frame,bool keyboard)
{
  edict_t *ent=prepare(slot,frame);
  edict_t reference=*ent;gclient_t client=player;reference.client=&client;
  client.buttons=client.latched_buttons=0;
  b2w_fire_cancel(ent);
  assert(b2w_fire.rearm && !b2w_fire.owned);
  assert(player.weaponstate==WEAPON_FIRING && player.ps.gunframe==frame);
  if(keyboard)test_context=0;
  // Neither repeated calls nor fresh trigger edges advance recovery in this tick.
  for(int repeat=0;repeat<4;repeat++){
    player.buttons=player.latched_buttons=BUTTON_ATTACK;
    assert(!B2W_WeaponBegin(ent));assert(player.ps.gunframe==frame);
    assert(!player.buttons&&!player.latched_buttons);
  }
  unsigned ticks=0;
  while(client.weaponstate==WEAPON_FIRING){
    assert(++ticks<=40);level.framenum++;
    weapons[slot](&reference); // Actual selected callback + Weapon_Generic.
    unsigned reference_shots=shot_count;
    player.buttons=player.latched_buttons=BUTTON_ATTACK;
    assert(!B2W_WeaponBegin(ent));
    assert(player.ps.gunframe==client.ps.gunframe);
    assert(player.weaponstate==client.weaponstate);
    assert(shot_count==reference_shots); // Recovery never calls an emission path.
    if(b2w_fire.recovering){assert(!B2W_WeaponBegin(ent));assert(player.ps.gunframe==client.ps.gunframe);}
  }
  assert(!b2w_fire.recovering&&player.weaponstate==WEAPON_READY);
  // Back in controller mode: a held trigger alone still cannot rearm.
  test_context=B2W_CONTEXT_REQUESTED|B2W_CONTEXT_COMMAND|B2W_CONTEXT_NEW_COMMAND;
  player.buttons=BUTTON_ATTACK;test_packet.attack=true;
  B2W_WeaponCommand(ent);assert(b2w_fire.rearm&&!B2W_WeaponStart(ent));
  // Invalid/stale input cannot manufacture the neutral acknowledgement either.
  player.buttons=player.latched_buttons=0;test_packet.attack=false;test_packet_valid=false;
  B2W_WeaponCommand(ent);assert(b2w_fire.rearm&&!B2W_WeaponStart(ent));
  test_packet_valid=true;B2W_WeaponCommand(ent);assert(!b2w_fire.rearm);
  player.buttons=BUTTON_ATTACK;test_packet.attack=true;
  B2W_WeaponCommand(ent);assert(B2W_WeaponBegin(ent));assert(B2W_WeaponStart(ent));
}
static void boundaries(void)
{
  edict_t *ent=prepare(2,8);
  // An invalid active packet reaches the production cancellation path itself.
  test_packet_valid=false;assert(!B2W_WeaponBegin(ent));
  assert(b2w_fire.recovering&&player.ps.gunframe==8&&b2w_fire.rearm);
  test_packet_valid=true;test_context=B2W_CONTEXT_REQUESTED|B2W_CONTEXT_COMMAND;
  level.framenum++;assert(!B2W_WeaponBegin(ent));assert(player.ps.gunframe==8);
  player.buttons=player.latched_buttons=0;test_packet.attack=false;
  B2W_WeaponCommand(ent);assert(b2w_fire.rearm); // Duplicate command is not neutral.
  test_context|=B2W_CONTEXT_NEW_COMMAND;B2W_WeaponCommand(ent);assert(!b2w_fire.rearm);
  assert(!B2W_WeaponBegin(ent));assert(player.ps.gunframe==9); // Genuine recovery tick only.

  ent=prepare(2,8);test_owner.binding=2;assert(!B2W_WeaponBegin(ent));
  assert(b2w_fire.recovering&&b2w_fire.rearm&&player.ps.gunframe==8);
  ent=prepare(2,8);test_packet.game_epoch=2;assert(!B2W_WeaponBegin(ent));
  assert(b2w_fire.recovering&&b2w_fire.rearm&&player.ps.gunframe==8);
  // A mode round-trip cannot misclassify retired recovery as an ordinary cycle.
  ent=prepare(2,8);b2w_fire_cancel(ent);test_context=B2W_CONTEXT_COMMAND|B2W_CONTEXT_NEW_COMMAND;
  player.buttons=player.latched_buttons=0;B2W_WeaponCommand(ent);
  assert(!b2w_fire.requested&&!b2w_fire.rearm&&B2W_WeaponTracked(ent));
  test_context=B2W_CONTEXT_COMMAND;level.framenum++;
  assert(!B2W_WeaponBegin(ent)&&player.ps.gunframe==8);
  test_context=B2W_CONTEXT_REQUESTED|B2W_CONTEXT_COMMAND|B2W_CONTEXT_NEW_COMMAND;
  B2W_WeaponCommand(ent);assert(!b2w_fire.ordinary&&b2w_fire.recovering);
  // A partial discharge retires only after its callback has charged ammunition.
  ent=prepare(2,8);player.pers.inventory[1]=998;b2w_fire.cancel=true;
  B2W_WeaponEnd(ent);assert(b2w_fire.recovering&&player.pers.inventory[1]==998&&!shot_count);

  ent=prepare(2,8);b2w_fire_cancel(ent);ent->health=0;ent->deadflag=1;
  assert(B2W_WeaponBegin(ent));assert(!b2w_fire.recovering&&!b2w_fire.owned);
  ent=prepare(2,8);b2w_fire_cancel(ent);B2W_WeaponChanged(ent);assert(!b2w_fire.recovering);
  ent=prepare(2,8);b2w_fire_cancel(ent);b2w_fire_reset();assert(!b2w_fire.recovering);
  ent=prepare(2,8);b2w_fire_cancel(ent);player.pers.weapon=items+1;
  assert(!b2w_fire_recovery(ent)&&!b2w_fire.recovering);
  // A queued weapon switch still waits for recovery; it cannot shorten cadence.
  ent=prepare(2,8);b2w_fire_cancel(ent);player.newweapon=items+1;
  level.framenum++;assert(!B2W_WeaponBegin(ent)&&player.ps.gunframe==9&&!change_count);
  // Pre-emission cancellation remains immediate and emits nothing later.
  ent=prepare(2,7);b2w_fire.emitted=false;b2w_fire_cancel(ent);
  assert(!b2w_fire.recovering&&player.weaponstate==WEAPON_READY&&b2w_fire.rearm);
  // Existing delayed-shot handling is unchanged for emitted hand grenades/BFG.
  for(int slot=5;slot<=10;slot+=5){
    ent=prepare(slot,slot==5?15:18);int frame=player.ps.gunframe;
    b2w_fire_cancel(ent);
    assert(b2w_fire.owned&&b2w_fire.rearm&&!b2w_fire.recovering);
    assert(player.weaponstate==WEAPON_FIRING&&player.ps.gunframe==frame);
  }
  ent=prepare(5,11);b2w_fire.emitted=false;player.grenade_framenum=150;
  b2w_fire_cancel(ent);
  assert(!b2w_fire.recovering&&b2w_fire.grenade.state==B2WG_INTERRUPTED&&player.grenade_framenum==0);
  // Ordinary keyboard/mouse with no retired tracked shot keeps the stock route.
  ent=prepare(2,8);memset(&b2w_fire,0,sizeof(b2w_fire));test_context=0;
  assert(B2W_WeaponBegin(ent)&&!b2w_fire.recovering);
  Weapon_SuperShotgun(ent);assert(player.ps.gunframe==9);B2W_WeaponEnd(ent);
}
int main(void)
{
  // Every post-emission frame and recovery tail of all ten non-delayed slots.
  const int first[12]={6,9,8,4,6,0,7,6,6,5,0,10};
  for(int slot=0;slot<12;slot++)if(weapons[slot])
    for(int frame=first[slot];frame<=b2w_fire_idle[slot];frame++){
      cadence(slot,frame,false);cadence(slot,frame,true);
    }
  boundaries();
  puts("PASS: actual stock recovery for ten slots; cancellation, rearm, duplicate/stale input, mode switch, death, lifecycle reset, weapon change, grenade/BFG and KBM boundaries");
  return 0;
}
'''


def fixture(fire, weapon, function, old_cancel=None):
    names = ['b2w_fire_local', 'b2w_fire_same_cycle', 'b2w_fire_armed',
             'b2w_fire_delayed', 'b2w_fire_recovery', 'b2w_fire_cancel',
             'b2w_fire_reset', 'b2w_fire_transition', 'B2W_WeaponTracked',
             'b2w_fire_death', 'B2W_WeaponCommand', 'B2W_WeaponBegin',
             'B2W_WeaponEnd', 'B2W_WeaponStart', 'B2W_WeaponChanged']
    idle = next(line for line in fire.splitlines() if line.startswith('static const int b2w_fire_idle['))
    pieces = [PREFIX, idle]
    records = [dict(source=FIRE, span='b2w_fire_idle', sha256=hashlib.sha256(idle.encode()).hexdigest())]
    def append(source, name, code):
        pieces.append(code)
        records.append(dict(source=source, function=name, sha256=hashlib.sha256(code.encode()).hexdigest()))
    for name in names:
        previous = name == 'b2w_fire_cancel' and old_cancel
        append(BEFORE if previous else FIRE, name, previous or function(fire, name))
    append(WEAPON, 'Weapon_Generic', function(weapon, 'Weapon_Generic'))
    for name in ['Machinegun_Fire', 'Chaingun_Fire', 'Weapon_HyperBlaster_Fire', 'weapon_shotgun_fire',
                 'Weapon_Blaster', 'Weapon_Shotgun', 'Weapon_SuperShotgun', 'Weapon_Machinegun',
                 'Weapon_Chaingun', 'Weapon_GrenadeLauncher', 'Weapon_RocketLauncher',
                 'Weapon_HyperBlaster', 'Weapon_Railgun', 'Weapon_FlareGun']:
        append(WEAPON, name, function(weapon, name))
    pieces.append(TESTS)
    return '\n\n'.join(pieces) + '\n', records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    out = args.output.resolve()
    if out.exists() or out.is_relative_to(root) or root.is_relative_to(out):
        raise ValueError('Use a new, disjoint output directory')
    helper = runpy.run_path(str(root/'tools/rc7-regression.py'))
    function, run = helper['function'], helper['run']
    fire = (root/FIRE).read_text(encoding='utf-8')
    weapon = (root/WEAPON).read_text(encoding='utf-8')
    before_bytes = (root/BEFORE).read_bytes()
    assert hashlib.sha256(before_bytes).hexdigest() == BEFORE_SHA256
    before = before_bytes.decode('utf-8')
    old_cancel = function(before, 'b2w_fire_cancel')
    assert old_cancel != function(fire, 'b2w_fire_cancel')
    out.mkdir(parents=True)
    results = []
    for label, previous in [('current', None), ('pre-fix-negative', old_cancel)]:
        work = out/label
        work.mkdir()
        code, records = fixture(fire, weapon, function, previous)
        (work/'case.c').write_text(code, encoding='utf-8')
        (work/'EXTRACTIONS.json').write_text(json.dumps(records, indent=2)+'\n')
        run(['cl.exe', '/nologo', '/W3', '/O1', '/MT', 'case.c', '/Fetest.exe', '/Fotest.obj',
             '/link', '/INCREMENTAL:NO'], work, work/'compile.txt')
        if previous:
            result = subprocess.run([str(work/'test.exe')], cwd=work, capture_output=True, timeout=30)
            (work/'run.txt').write_bytes(result.stdout+result.stderr)
            assert result.returncode == 86, ('Pre-fix cadence case must fail', result.returncode)
            assert b'player.weaponstate==WEAPON_FIRING' in result.stderr
            results.append(dict(case=label, exit=result.returncode, expectedFailure=True))
        else:
            run([work/'test.exe'], work, work/'run.txt', 30)
            results.append(dict(case=label, exit=0))
        print(label, 'PASS', flush=True)
    (out/'COMPLETE.json').write_text(json.dumps(dict(status='PASSED', cases=results,
        baseline=BASELINE, sources=[FIRE, WEAPON], headsetVerification=False), indent=2)+'\n')


if __name__ == '__main__':
    main()
