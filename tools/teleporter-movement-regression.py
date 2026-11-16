# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
"""Exercise teleport jump release with the selected Pmove, filter and guard.
Run from an x64 MSVC developer shell with --output ABSENT_DIRECTORY outside source.
No game, graphics, headset, saved game or runtime is opened. Outputs are retained.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import time

ap = argparse.ArgumentParser(description=__doc__)
ap.add_argument('--output', type=Path, required=True)
a = ap.parse_args()
source = Path(__file__).resolve().parents[1]
output = a.output.resolve()
assert not output.exists() and not output.is_relative_to(source) and not source.is_relative_to(output)
output.mkdir(parents=True)
guard_path = 'source/current/rc-cleanup457/source/game_003/g_combat.c'
pmove_path = 'source/baseline/b2-consolidated215/inputs/origin-006/src/common/pmove.c'
math_path = 'source/baseline/b2-consolidated215/inputs/origin-006/src/shared/shared.c'
proof = []


def record(path, name, text):
    proof.append(dict(source=path, selection=name, sha256=hashlib.sha256(text.encode()).hexdigest()))
    return text


def take(path, name):
    text = (source / path).read_text(encoding='utf-8')
    match = re.search(r'^(?:static )?(?:bool|void|int|vec_t) ' + name + r'\([^;]*?\)\s*\{', text, re.M)
    assert match, name
    end, depth = match.end(), 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return record(path, name, text[match.start():end]) + '\n'


guard_text = (source / guard_path).read_text(encoding='utf-8')
begin = guard_text.index('typedef struct {\n    vec3_t origin, velocity, mins, maxs;')
end = guard_text.index('} obs_trace;', begin) + len('} obs_trace;')
state = record(guard_path, 'observer_state_t and obs_trace', guard_text[begin:end])
validator = take(guard_path, 'observer_check_walk_pmove')
corrected = '(was_held && held && obs_trace.command.upmove < 10 && !landing_delay && !water_time && !ledge && !teleport_after)'
assert validator.count(corrected) == 1
before_validator = record(guard_path, 'pre-fix comparison: remove only the teleport_after exemption',
    validator.replace(corrected, corrected.replace(' && !teleport_after', '')))
helper = take(guard_path, 'observer_stock_snap_fallback')
callbacks = ''.join(take(guard_path, name) for name in (
    'G_ObserverPmove', 'G_ObserverPointContents', 'G_ObserverCollision'))
command_filter = take(guard_path, 'G_CombatFilterCommand')
math_code = ''.join(take(math_path, name) for name in ('AngleVectors', 'VectorNormalize'))
pmove_code = record(pmove_path, 'complete selected translation unit', (source / pmove_path).read_text(encoding='utf-8'))

# Reuse the existing inert observer seams; the water/snap-specific fixture and
# main below that prefix are replaced by a dry floor and teleport cases.
template_path = 'tools/collision-movement-case.c.in'
template = (source / template_path).read_text(encoding='utf-8')
prefix = template[:template.index('static trace_t q_gameabi world_trace(')]
record(template_path, 'inert observer harness prefix before dry-floor adaptations', prefix)
for old, new in (
    ('static struct { bool enabled; unsigned relocations; } combat;',
     'static struct { bool enabled, input_hold; unsigned relocations; } combat;'),
    ('contents_calls++; return CONTENTS_WATER;', 'contents_calls++; return 0;'),
    ('    obs_trace.walk_faulted = true;\n    refusal_phase = phase;',
     '    if (obs_trace.walk_faulted) return;\n    obs_trace.walk_faulted = true;\n    refusal_phase = phase;'),
):
    assert prefix.count(old) == 1
    prefix = prefix.replace(old, new)
for marker, text in [('/* OBSERVER_STATE */', state), ('/* PMOVE */', math_code + pmove_code),
                     ('/* SNAP_HELPER */', helper), ('/* CALLBACKS */', callbacks)]:
    assert prefix.count(marker) == 1
    prefix = prefix.replace(marker, text)
assert prefix.count('/* VALIDATOR */') == 1

cases = r'''
static bool G_CombatObserver(edict_t *ent) { return true; }
/* COMMAND_FILTER */
static edict_t player, floor_entity;
static pmove_t movement;

static trace_t q_gameabi dry_floor(const vec3_t start, const vec3_t mins,
                                  const vec3_t maxs, const vec3_t end)
{
    trace_t trace = {0};
    trace.fraction = 1.f;
    VectorCopy(end, trace.endpos);
    float start_bottom = start[2] + mins[2], end_bottom = end[2] + mins[2];
    if (start_bottom >= 0.f && end_bottom < 0.f) {
        trace.fraction = start_bottom / (start_bottom - end_bottom);
        for (unsigned i = 0; i < 3; ++i)
            trace.endpos[i] = start[i] + trace.fraction * (end[i] - start[i]);
        trace.plane.normal[2] = 1.f;
        trace.ent = &floor_entity;
        trace.contents = CONTENTS_SOLID;
    }
    G_ObserverCollision(start, end, &trace);
    return trace;
}

static void capture(unsigned duration, bool stock, bool held, unsigned msec, int upmove)
{
    pmoveParams_t parameters;
    memset(&obs_trace, 0, sizeof obs_trace);
    memset(&combat, 0, sizeof combat);
    memset(&player, 0, sizeof player);
    memset(&movement, 0, sizeof movement);
    combat.enabled = true;
    combat.relocations = stock && duration ? 1 : 0;
    combat.input_hold = stock;
    b2tp_moved = !stock && duration;
    obs_trace.walk_guard = obs_trace.vertical = obs_trace.command_active = true;
    refusal_mask = 0;
    refusal_phase = NULL;
    contents_calls = 0;
    movement.s.pm_type = PM_NORMAL;
    movement.s.origin[2] = 24 * 8;
    movement.s.gravity = 800;
    // Controlled post-relocation state: both writers preserve JUMP_HELD.
    // Stock has a 20-tick pause and input drain; VR has a two-tick pause.
    movement.s.pm_flags = (stock && duration ? 0 : PMF_ON_GROUND) | (held ? PMF_JUMP_HELD : 0) |
        (duration ? PMF_TIME_TELEPORT : 0);
    movement.s.pm_time = duration;
    movement.cmd.msec = msec;
    movement.cmd.upmove = upmove;
    movement.trace = dry_floor;
    movement.pointcontents = G_ObserverPointContents;
    G_CombatFilterCommand(&player, &movement.cmd);
    assert(movement.cmd.upmove == (stock ? 0 : upmove));
    assert(combat.input_hold == (stock && upmove != 0));
    obs_trace.command = movement.cmd;
    PmoveInit(&parameters);
    G_ObserverPmove(&player, &movement, true);
    Pmove(&movement, &parameters);
    assert(contents_calls == 2);
}

static void validate(bool expected_fault, unsigned expected_mask, const char *phase)
{
    obs_trace.walk_faulted = false;
    refusal_mask = 0;
    refusal_phase = NULL;
    G_ObserverPmove(&player, &movement, false);
    assert(obs_trace.walk_faulted == expected_fault);
    assert(refusal_mask == expected_mask);
    assert(expected_fault ? (refusal_phase && !strcmp(refusal_phase, phase)) : refusal_phase == NULL);
}

static void surviving_pause(void)
{
    assert(movement.s.pm_flags & PMF_TIME_TELEPORT);
    assert(movement.s.pm_flags & PMF_JUMP_HELD);
    assert(movement.cmd.upmove == 0 && movement.s.pm_time > 0);
#ifdef EXPECT_TELEPORT_REFUSAL
    validate(true, BIT(1), "unsupported_pmove_result");
#else
    validate(false, 0, NULL);
#endif
}

static void expired_pause(void)
{
    assert(!(movement.s.pm_flags & (PMF_TIME_TELEPORT | PMF_JUMP_HELD)));
    assert(!movement.s.pm_time);
    validate(false, 0, NULL);
    // A pause present before integration cannot exempt a stale latch after
    // its expiry: this distinguishes teleport_after from teleport_time.
    movement.s.pm_flags |= PMF_JUMP_HELD;
    validate(true, BIT(1), "unsupported_pmove_result");
}

int main(void)
{
    const unsigned command_ms[] = {0, 1, 7, 8, 11, 16, 159};
    for (unsigned i = 0; i < sizeof command_ms / sizeof command_ms[0]; ++i) {
        capture(20, true, true, command_ms[i], 200);
        assert(movement.s.pm_time == 20 - max(1u, command_ms[i] >> 3));
        surviving_pause();
    }
    capture(20, true, true, 11, 0); surviving_pause();
    capture(2, true, true, 8, 200); assert(movement.s.pm_time == 1); surviving_pause();
    capture(20, true, false, 11, 0); validate(false, 0, NULL);
    capture(1, true, true, 8, 200); expired_pause();
    capture(20, true, true, 160, 200); expired_pause();
    puts("PASS: stock held/released jump, short commands, last active tick and expiry expectations");

    // The VR writer preserves the latch without the stock command drain.
    // Releasing jump on the next 11 ms command leaves one timer tick.
    capture(2, false, true, 11, 0); assert(movement.s.pm_time == 1); surviving_pause();
    capture(2, false, true, 11, 200); validate(false, 0, NULL);
    capture(2, false, true, 16, 0); expired_pause();
    puts("PASS: VR next-command release uses the same surviving-pause condition");

    capture(0, false, true, 11, 0); validate(false, 0, NULL);
    assert(!(movement.s.pm_flags & PMF_JUMP_HELD));
    movement.s.pm_flags |= PMF_JUMP_HELD;
    validate(true, BIT(1), "unsupported_pmove_result");
    capture(0, false, false, 11, 200);
    assert(movement.s.pm_flags & PMF_JUMP_HELD);
    validate(false, 0, NULL);

    capture(20, true, false, 11, 0); movement.s.pm_time++;
    validate(true, 0, "unexplained_teleport_pmove_timer");
    capture(20, true, false, 11, 0); movement.s.pm_flags &= ~PMF_TIME_TELEPORT;
    validate(true, 0, "unexplained_teleport_pmove_timer");
    capture(20, true, false, 11, 0); combat.relocations = 0;
    validate(true, BIT(7), "unexplained_teleport_pmove_timer");
    puts("PASS: normal jump/release, stale latch, invalid timer and unauthorised pause controls");
#ifdef EXPECT_TELEPORT_REFUSAL
    puts("PRE-CORRECTION: surviving stock and VR pauses refused solely by movement bit 1");
#else
    puts("CURRENT: surviving stock and VR pauses accepted; invalid controls still refused");
#endif
    return 0;
}
'''
assert cases.count('/* COMMAND_FILTER */') == 1
cases = cases.replace('/* COMMAND_FILTER */', command_filter)
for stem, selected, define in (('before', before_validator, '#define EXPECT_TELEPORT_REFUSAL 1\n'),
                               ('current', validator, '')):
    (output / (stem + '.c')).write_text(define + prefix.replace('/* VALIDATOR */', selected) + cases, encoding='utf-8')
(output / 'EXTRACTIONS.json').write_text(json.dumps(proof, indent=2) + '\n', encoding='utf-8')


def run(name, command):
    start = time.monotonic()
    child = subprocess.Popen([str(x) for x in command], cwd=output, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        raw = child.communicate(timeout=90)[0]
    except subprocess.TimeoutExpired:
        subprocess.run(['taskkill', '/PID', str(child.pid), '/T', '/F'], capture_output=True, timeout=20)
        raw = child.communicate(timeout=10)[0]
        (output / (name + '-timeout.txt')).write_bytes(raw)
        raise
    (output / (name + '.txt')).write_bytes(raw)
    (output / (name + '.json')).write_text(json.dumps(dict(argv=[str(x) for x in command], pid=child.pid,
        exit=child.returncode, seconds=time.monotonic() - start, terminal=True), indent=2) + '\n', encoding='utf-8')
    print(raw.decode(errors='replace'), flush=True)
    assert child.returncode == 0, name


include = source / 'source/baseline/b2-consolidated215/inputs/origin-046/upstream/inc'
for stem in ('before', 'current'):
    run(stem + '-compile', ['cl.exe', '/nologo', '/W3', '/O1', '/MT', '/std:c11', '/D_CRT_SECURE_NO_WARNINGS',
        '/I' + str(include), stem + '.c', '/Fe' + stem + '.exe', '/Fo' + stem + '.obj', '/link', '/INCREMENTAL:NO'])
    run(stem + '-run', [output / (stem + '.exe')])
(output / 'COMPLETE.json').write_text(json.dumps(dict(status='TELEPORTER_MOVEMENT_REGRESSION_PASSED',
    scope='Actual selected Pmove, mathematics, input filter, capture callbacks and movement validator; post-relocation state and dry-floor callbacks are controlled seams. The comparison removes only the teleport_after exemption. No teleporter writer, end-to-end headset or BSP collision claim.',
    gameOrHeadsetExecution=False), indent=2) + '\n', encoding='utf-8')
