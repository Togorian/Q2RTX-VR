// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifndef B2D_PATH
#define B2D_PATH "startup-status.txt"
#endif
// The launcher creates/resets this report in the engine working directory.
// No directories, config reads, dumps, timers or background logging. Failure to
// record never changes admission or cleanup. Reserve4KiB for terminal errors.
static inline void B2D_Record(int terminal,const char *fmt,...)
{
    FILE *f=fopen(B2D_PATH,"r+b");if(!f)return;
    if(fseek(f,0,SEEK_END)){fclose(f);return;}
    long used=ftell(f);long cap=terminal?16384:12288;
    if(used<0 || used>=cap){fclose(f);return;}
    char line[512];va_list args;va_start(args,fmt);
    int n=vsnprintf(line,sizeof(line)-1,fmt,args);va_end(args);
    if(n<0){fclose(f);return;}
    size_t count=(size_t)n;if(count>sizeof(line)-2)count=sizeof(line)-2;
    line[count++]='\n';if(count>(size_t)(cap-used))count=(size_t)(cap-used);
    fwrite(line,1,count,f);fclose(f);
}
