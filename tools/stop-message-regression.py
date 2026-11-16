# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian; implementation with Codex assistance.
"""Exercise actual terminal-message selection without a runtime, window or GPU.

Run from an x64 MSVC developer prompt with --output pointing to a new directory
outside the source tree. Production functions/spans are hashed in EXTRACTIONS.json.
The retained pre-fix request-stop body must fail the initiating-cause case.
The pre-startup-correction condition must fail the local startup refusal case.
"""
from pathlib import Path
import argparse, hashlib, json, re, runpy, subprocess

XR = 'source/current/upper-palace500/source/engine_087/reference_b_b2_xr.c'
HAPTIC = 'source/baseline/b2-menu-defaults320/source/refresh/b2_haptics_xr.inc'
RUNTIME = 'source/current/runtime-recovery555/source/b2_runtime555.h'
SYSTEM = 'source/baseline/b2-teleport-controls301/source/engine_070/system.c'
BEFORE = 'tools/stop-message-before.c.in'

PREFIX = r'''
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define XR_NO_PROTOTYPES
#include <openxr/openxr.h>
#include "source/current/runtime-recovery555/source/b2_runtime555.h"
#define assert(x) do { if (!(x)) { fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#x); exit(86); } } while(0)
static char b2_failure_message555[256],shown[4096];
static bool b2_stopped_notified555;
static int records,boxes,resets,exits,histories;
static XrResult exit_result=XR_SUCCESS,reset_result=XR_SUCCESS;
typedef struct { bool should_render; } frame_t;
static struct { bool loader,instance,running,exit_requested,failed,loss_pending,frame_open;
 XrSession session; frame_t *frame; PFN_xrRequestExitSession xrRequestExitSession; } xr;
typedef struct { bool haptic_rearm582,haptic_focused582,haptic_failed;
 bool haptic_active[2],haptic_available[2]; unsigned haptic_errors,haptic_error_records594,haptic_result_records582; } b2c_native_t;
static void Q_strlcpy(char *dst,const char *src,size_t n){snprintf(dst,n,"%s",src);}
#define Q_snprintf snprintf
#define B2D_Record(...) (++records)
#define Com_Printf(...) ((void)0)
#define Com_EPrintf(...) ((void)0)
#define SDL_MESSAGEBOX_ERROR 1
#define MAXERRORMSG 4096
#define PRODUCT "Q2RTX VR"
#define MB_ICONERROR 1
#define MB_OK 0
static void SDL_ShowSimpleMessageBox(int flags,const char *title,const char *message,void *window){boxes++;Q_strlcpy(shown,message,sizeof(shown));}
#define MessageBoxA(window,text,title,flags) SDL_ShowSimpleMessageBox(flags,title,text,window)
void B2XR_RequestStop(void);
static void b2_xr_note_failure555(XrResult result,const char *reason);
static bool b2_xr_result(XrResult result,const char *operation);
static void B2XR_InvalidateHistory(const char *reason){histories++;}
static void B2H_Reset(void){resets++;if(reset_result!=XR_SUCCESS)b2_xr_note_failure555(reset_result,"cleanup-stop");}
static XrResult XRAPI_PTR request_exit(XrSession session){exits++;return exit_result;}
'''

TESTS = r'''
static void init(void)
{
 memset(&xr,0,sizeof(xr));xr.loader=xr.instance=xr.running=true;xr.xrRequestExitSession=request_exit;
 b2_failure_message555[0]=shown[0]=0;b2_stopped_notified555=false;
 records=boxes=resets=exits=histories=0;exit_result=reset_result=XR_SUCCESS;
}
static void internal(void)
{
 assert(B2XR_FailureMessage555());
 assert(!strcmp(B2XR_FailureMessage555(),B2_InternalStopMessage642()));
 assert(!strstr(B2XR_FailureMessage555(),"headset connection"));
}
int main(void)
{
 // Local refusal before a running session must direct the player to startup details.
 init();xr.running=false;b2_xr_note_failure555(XR_SUCCESS,"render-setting-invalid");
 assert(!strcmp(B2XR_FailureMessage555(),"VR could not start. Check startup-status.txt for details, then relaunch after correcting the reported problem."));
 init();b2_xr_note_failure555(XR_SUCCESS,"render-setting-invalid");internal();
 // Genuine runtime loss retains its diagnosis even before the running flag is set.
 init();xr.running=false;b2_xr_note_failure555(XR_ERROR_SESSION_LOST,"create-session");
 assert(strstr(B2XR_FailureMessage555(),"headset connection"));
 // A client/renderer guard requests exit before the runtime reports EXITING.
 init();B2XR_RequestStop();assert(xr.exit_requested&&exits==1&&resets==1);internal();
 assert(records==0&&boxes==0);b2_xr_note_failure555(XR_SUCCESS,"session-exit-or-loss");internal();
 B2XR_NotifyStopped555();assert(boxes==1&&!strcmp(shown,B2_InternalStopMessage642()));
 B2XR_NotifyStopped555();assert(boxes==1);B2XR_RequestStop();assert(exits==1&&resets==2);
 // Normal quit uses that same request but never calls NotifyStopped: no box/log.
 init();B2XR_RequestStop();assert(records==0&&boxes==0);
 init();xr.loader=xr.instance=false;B2XR_RequestStop();assert(resets==1&&!exits&&!B2XR_FailureMessage555());
 // A later exit/cleanup error must not overwrite an initiating internal stop.
 init();exit_result=XR_ERROR_SESSION_LOST;reset_result=XR_ERROR_INSTANCE_LOST;B2XR_RequestStop();internal();
 // Runtime loss is actionable only when it was the initiating cause.
 const XrResult losses[]={XR_ERROR_SESSION_LOST,XR_ERROR_INSTANCE_LOST,XR_SESSION_LOSS_PENDING};
 for(int i=0;i<3;i++){
  init();b2_xr_note_failure555(losses[i],"wait-frame");B2XR_RequestStop();
  assert(strstr(B2XR_FailureMessage555(),"headset connection"));assert(records==1);
  // The actual optional-output loss route records the result before cleanup.
  init();b2c_native_t a={0};a.haptic_active[0]=a.haptic_available[0]=true;
  assert(!b2ha_result582(&a,losses[i],16,"stop"));assert(xr.exit_requested&&exits==1);
  assert(strstr(B2XR_FailureMessage555(),"headset connection"));assert(!a.haptic_active[0]&&!a.haptic_available[0]);
 }
 init();b2_xr_note_failure555(XR_SUCCESS,"instance-loss-pending");B2XR_RequestStop();assert(strstr(B2XR_FailureMessage555(),"headset connection"));
 init();b2_xr_note_failure555(XR_SUCCESS,"session-exit-or-loss");B2XR_RequestStop();assert(strstr(B2XR_FailureMessage555(),"headset connection"));
 // Local guards are not reported as headset or runtime faults.
 const char *guards[]={"frame-begin-reentry","eye-copy-ownership","eye-finish-contract","end-without-begin","incomplete-rendered-pair","reference-recovery-generation","quad-same-eye-pose-mismatch"};
 for(int i=0;i<sizeof(guards)/sizeof(guards[0]);i++){init();b2_xr_note_failure555(XR_SUCCESS,guards[i]);B2XR_RequestStop();internal();}
 init();b2_xr_note_failure555(XR_SUCCESS,"runtime-events-lost-state-unconfirmed");assert(strstr(B2XR_FailureMessage555(),"stopped itself")&&!strstr(B2XR_FailureMessage555(),"headset connection"));
 // Existing startup/runtime diagnosis remains precise.
 init();b2_xr_note_failure555(XR_ERROR_RUNTIME_UNAVAILABLE,"create-instance");assert(strstr(B2XR_FailureMessage555(),"No active OpenXR runtime"));
 init();b2_xr_note_failure555(XR_ERROR_FORM_FACTOR_UNAVAILABLE,"get-system");assert(strstr(B2XR_FailureMessage555(),"No VR headset"));
 init();b2_xr_note_failure555(XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED,"create-session");assert(strstr(B2XR_FailureMessage555(),"does not support"));
 init();b2_xr_note_failure555(XR_SUCCESS,"local-loader-unavailable");assert(strstr(B2XR_FailureMessage555(),"packaged OpenXR loader"));
 init();b2_xr_note_failure555(XR_ERROR_RUNTIME_FAILURE,"end-frame");assert(strstr(B2XR_FailureMessage555(),"runtime error")&&!strstr(B2XR_FailureMessage555(),"headset connection"));
 init();xr.frame_open=true;B2XR_NotifyStopped555();assert(!boxes);xr.frame_open=false;B2XR_NotifyStopped555();assert(boxes==1&&!strcmp(shown,B2_InternalStopMessage642()));
 const char *fatals[]={"B2_FAIL: world resource boundary rejected","VR_FAIL: technical command reset/begin failed (VK_ERROR_DEVICE_LOST)","B2 invalid ownership","B2_HUD: bad state","VR settings rejection encountered unfinished eye ownership","VR rejected-frame retirement failed; restart required.","VR graphics could not safely continue. Restart Q2RTX VR and load a saved game.","Desktop graphics could not safely retire an unfinished frame. Restart Q2RTX VR.","VR callback setting was not registered","VR graphics could not start: invalid initial frame. See startup-status.txt."};
 for(int i=0;i<sizeof(fatals)/sizeof(fatals[0]);i++){fatal_display(fatals[i]);assert(strstr(shown,B2_InternalStopMessage642())&&strstr(shown,fatals[i])&&strstr(shown,"Details:"));}
 const char *ordinary[]={"The graphics device stopped responding. Restart after checking the driver.","Not enough video memory for VR.","No ray tracing capable GPU found.","VR stopped after a runtime error.",B2_InternalStopMessage642()};
 fatal_display("instanced-position producer and consumer ranges differ");assert(strstr(shown,B2_InternalStopMessage642())&&strstr(shown,"Details:"));
 fatal_display("private peripheral guide: invalid descriptor slot");assert(strstr(shown,B2_InternalStopMessage642())&&strstr(shown,"Details:"));
 for(int i=0;i<sizeof(ordinary)/sizeof(ordinary[0]);i++){fatal_display(ordinary[i]);assert(!strcmp(shown,ordinary[i]));}
 puts("PASS: startup/local/runtime refusal text, internal/runtime first cause, owned exit, haptic loss, quiet quit, repeat stop, frame closure and fatal recovery detail");
 return 0;
}
'''

def function(text, name):
    match = re.search(r'^(?:static )?(?:inline )?(?:const )?(?:char\s*\*|void|bool)\s*'+re.escape(name)+r'\([^;]*?\)\s*\{', text, re.M)
    if not match: raise ValueError('Missing function: '+name)
    end, depth = match.end(), 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[match.start():end]

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);args=ap.parse_args()
    root=Path(__file__).resolve().parents[1];out=args.output.resolve()
    if out.exists() or out.is_relative_to(root) or root.is_relative_to(out):raise ValueError('Use a new, disjoint output directory')
    run=runpy.run_path(str(root/'tools/rc7-regression.py'))['run']
    text={p:(root/p).read_text(encoding='utf-8') for p in [XR,HAPTIC,RUNTIME,SYSTEM,BEFORE]}
    names=['b2_xr_failure_text555','b2_xr_note_failure555','B2XR_FailureMessage555','B2XR_NotifyStopped555','b2_xr_result','B2XR_RequestStop']
    display=text[SYSTEM][text[SYSTEM].index('    char recovery[MAXERRORMSG];'):]
    display=display[:display.index('    exit(1);')]
    assert 'MessageBoxA(NULL, display, PRODUCT " Fatal Error", MB_ICONERROR | MB_OK);' in display
    out.mkdir(parents=True);results=[]
    for label,previous in [('current',False),('pre-fix-negative',True),('pre-startup-fix-negative',False)]:
        work=out/label;work.mkdir();parts=[PREFIX];records=[]
        def add(path,name,span):
            parts.append(span);records.append(dict(source=path,span=name,sha256=hashlib.sha256(span.encode()).hexdigest()))
        records.append(dict(source=RUNTIME,span='whole-header',sha256=hashlib.sha256(text[RUNTIME].encode()).hexdigest()))
        for name in names:
            path=BEFORE if previous and name=='B2XR_RequestStop' else XR
            span=function(text[path],name)
            if label=='pre-startup-fix-negative' and name=='b2_xr_failure_text555':
                condition='if (result == XR_SUCCESS && xr.running)'
                assert span.count(condition)==1
                span=span.replace(condition,'if (result == XR_SUCCESS)')
                path+=' (only startup condition reverted)'
            add(path,name,span)
        add(HAPTIC,'b2ha_result582',function(text[HAPTIC],'b2ha_result582'))
        parts.append('static void fatal_display(const char *text)\n{\n'+display+'}\n')
        records.append(dict(source=SYSTEM,span='Sys_Error dialog span',sha256=hashlib.sha256(display.encode()).hexdigest()))
        parts.append(TESTS);(work/'case.c').write_text('\n\n'.join(parts),encoding='utf-8')
        (work/'EXTRACTIONS.json').write_text(json.dumps(records,indent=2)+'\n')
        run(['cl.exe','/nologo','/W3','/O1','/MT','/I'+str(root),'/I'+str(root/'source/baseline/b2-consolidated215/inputs/origin-008/vendor/openxr/include'),'case.c','/Fetest.exe','/Fotest.obj','/link','/INCREMENTAL:NO'],work,work/'compile.txt')
        if previous or label=='pre-startup-fix-negative':
            p=subprocess.run([str(work/'test.exe')],cwd=work,capture_output=True,timeout=30)
            (work/'run.txt').write_bytes(p.stdout+p.stderr)
            expected=b'startup-status.txt' if label=='pre-startup-fix-negative' else b'B2XR_FailureMessage555()'
            assert p.returncode==86 and expected in p.stderr,('Pre-fix must fail its regression case',label,p.returncode)
            results.append(dict(case=label,exit=p.returncode,expectedFailure=True))
        else:
            run([work/'test.exe'],work,work/'run.txt',30);results.append(dict(case=label,exit=0))
        print(label,'PASS',flush=True)
    (out/'COMPLETE.json').write_text(json.dumps(dict(status='PASSED',cases=results,headsetVerification=False),indent=2)+'\n')

if __name__=='__main__':main()
