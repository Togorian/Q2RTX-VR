// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 14 November 2026: historical RC6 fixture regenerated from the selected
// Q2RTX VR/Quake II RTX source spans listed in R6.json; external effects are seams.
/*
Copyright (C) 1997-2001 Id Software, Inc.

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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#define REFERENCE_B_B2 1
#define APPLICATION "Q2RTX"
#define MAX_OSPATH 1024
#define MAX_QPATH 64
#define FS_COUNT_READ ((void)0)
#define FS_COUNT_STRCMP ((void)0)
#define FS_DPrintf(...) ((void)0)
#define FS_pathcmp _stricmp
#define Q_stricmp _stricmp
#define Q_ERR(x) (-(x))
#define Q_ERR_INVALID_PATH -100
#define Q_ERR_INVALID_FORMAT -101
#define Q_ERR_INFINITE_LOOP -102
#define Q_ERR_STRING_TRUNCATED -103
#define Q_ERR_SUCCESS 0
#define PATH_NOT_CHECKED -1
#define PATH_INVALID 0
#define FS_PATH_MASK 3
#define FS_PATH_ANY 0
#define FS_PATH_GAME 1
#define FS_PATH_BASE 2
#define FS_TYPE_MASK 12
#define FS_TYPE_REAL 4
#define FS_TYPE_PAK 8
#define TAG_FILESYSTEM 0
#define COM_CONFIG_CFG "q2config.cfg"
#define COM_DEFAULT_CFG "default.cfg"
#define COM_Q2RTX_CFG "q2rtx.cfg"
#define COM_AUTOEXEC_CFG "autoexec.cfg"
#define COM_POSTEXEC_CFG "postexec.cfg"
#define CMD_BUFFER_SIZE 8192
#define ALIAS_LOOP_COUNT 16
#define Q_assert assert
#define Com_Printf(...) ((void)0)
#define Com_WPrintf(...) ((void)0)
typedef struct {unsigned mode;const char *content;} file_t;
typedef struct PACKFILE {struct PACKFILE *hash_next;size_t namelen;int nameofs;} packfile_t;
typedef struct {packfile_t **file_hash;unsigned hash_size;char *names;} pack_t;
typedef struct SEARCH {struct SEARCH *next;unsigned mode;pack_t *pack;char *filename;} searchpath_t;
static searchpath_t *fs_searchpaths;
static char fs_gamedir[]="home/baseq2";
static struct {int integer;char *string;} autoexec={1,""},game={0,""},*fs_autoexec=&autoexec,*fs_game=&game;
static int vr=1,parent_reads,home_present;
static char saved[2048]="bind F1 help\nseta value player\n";
static int Cvar_VariableInteger(const char *s){return vr;}
static unsigned FS_HashPath(const char *s,int n){return 0;}
static int FS_ValidatePath(const char *s){return 1;}
static int Q_concat(char *b,size_t n,const char *a,const char *c,const char *d){return snprintf(b,n,"%s%s%s",a,c,d);}
static const char *Q_ErrorString(int r){return "fixture";}
static int64_t open_from_pack(file_t *f,pack_t *p,packfile_t *e){assert(0);return -1;}
static int64_t open_from_disk(file_t *f,const char *p){
 const char *v=NULL;
 if(!strcmp(p,"parent/baseq2/q2config.cfg")){parent_reads++;v="bind HOME parent\nseta value parent\n";}
 if(!strcmp(p,"home/baseq2/q2config.cfg")&&home_present)v=saved;
 if(!strcmp(p,"home/baseq2/default-vr.cfg"))v="bind HOME recenter\nbind END handoff\nseta value default\n";
 if(!strcmp(p,"parent/baseq2/default-vr.cfg")){assert(!vr);v="seta value wrong-default\n";}
 if(!v)return Q_ERR(ENOENT);f->content=v;return strlen(v);
}
static int64_t open_file_read(file_t *file, const char *normalized, size_t namelen)
{
    char            fullpath[MAX_OSPATH];
    searchpath_t    *search;
    pack_t          *pak;
    unsigned        hash;
    packfile_t      *entry;
    int64_t         ret;
    int             valid;

    FS_COUNT_READ;

    if (!namelen)
        return Q_ERR_INVALID_PATH;

    hash = FS_HashPath(normalized, 0);
#ifdef REFERENCE_B_B2
    // Parent data directories carry the same FS_PATH_GAME flags as home.
    // VR preferences/defaults belong only to the actual writable game folder.
    bool vr_home_config = Cvar_VariableInteger("b2_openxr") &&
        (!FS_pathcmp(normalized, COM_CONFIG_CFG) || !FS_pathcmp(normalized, "default-vr.cfg"));
#endif

    valid = PATH_NOT_CHECKED;

// search through the path, one element at a time
    for (search = fs_searchpaths; search; search = search->next) {
#ifdef REFERENCE_B_B2
        if (vr_home_config && (search->pack || FS_pathcmp(search->filename, fs_gamedir))) continue;
#endif
        if (file->mode & FS_PATH_MASK) {
            if ((file->mode & search->mode & FS_PATH_MASK) == 0) {
                continue;
            }
        }

        // is the element a pak file?
        if (search->pack) {
            if ((file->mode & FS_TYPE_MASK) == FS_TYPE_REAL) {
                continue;
            }
            // don't bother searching in paks if length exceedes MAX_QPATH
            if (namelen >= MAX_QPATH) {
                continue;
            }
            pak = search->pack;
            // look through all the pak file elements
            entry = pak->file_hash[hash & (pak->hash_size - 1)];
            for (; entry; entry = entry->hash_next) {
                if (entry->namelen != namelen) {
                    continue;
                }
                FS_COUNT_STRCMP;
                if (!FS_pathcmp(pak->names + entry->nameofs, normalized)) {
                    // found it!
                    return open_from_pack(file, pak, entry);
                }
            }
        } else {
            if ((file->mode & FS_TYPE_MASK) == FS_TYPE_PAK) {
                continue;
            }
            // don't error out immediately if the path is found to be invalid,
            // just stop looking for it in directory tree but continue to search
            // for it in packs, to give broken maps or mods a chance to work
            if (valid == PATH_NOT_CHECKED) {
                valid = FS_ValidatePath(normalized);
            }
            if (valid == PATH_INVALID) {
                continue;
            }
            // check a file in the directory tree
            if (Q_concat(fullpath, sizeof(fullpath), search->filename,
                         "/", normalized) >= sizeof(fullpath)) {
                ret = Q_ERR(ENAMETOOLONG);
                goto fail;
            }

            ret = open_from_disk(file, fullpath);
            if (ret != Q_ERR(ENOENT))
                return ret;

#ifndef _WIN32
            if (valid == PATH_MIXED_CASE) {
                // convert to lower case and retry
                FS_COUNT_STRLWR;
                Q_strlwr(fullpath + strlen(search->filename) + 1);
                ret = open_from_disk(file, fullpath);
                if (ret != Q_ERR(ENOENT))
                    return ret;
            }
#endif
        }
    }

    // return error if path was checked and found to be invalid
    ret = valid ? Q_ERR(ENOENT) : Q_ERR_INVALID_PATH;

fail:
    FS_DPrintf("%s: %s: %s\n", __func__, normalized, Q_ErrorString(ret));
    return ret;
}


typedef struct {char text[8192];size_t cursize,maxsize;int aliasCount;} cmdbuf_t;
static cmdbuf_t cmd_buffer={.maxsize=8192};
static int FS_LoadFileEx(const char *p,void **out,unsigned flags,int tag){file_t f={flags};int n=(int)open_file_read(&f,p,strlen(p));*out=NULL;if(n<0)return n;*out=malloc(n+1);memcpy(*out,f.content,n+1);return n;}
static void FS_FreeFile(void *p){free(p);}
static int COM_Compress(char *s){return strlen(s);}
void Cbuf_InsertText(cmdbuf_t *buf, const char *text)
{
    size_t l = strlen(text);

// add the entire text of the file
    if (!l) {
        return;
    }
    Q_assert(buf->cursize <= buf->maxsize);
    if (l + 1 > buf->maxsize - buf->cursize) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    memmove(buf->text + l + 1, buf->text, buf->cursize);
    memcpy(buf->text, text, l);
    buf->text[l] = '\n';
    buf->cursize += l + 1;
}
int Cmd_ExecuteFile(const char *path, unsigned flags)
{
    char *f;
    int len, ret;
    cmdbuf_t *buf;
    bool vr_bindings_reset = false;
#ifdef REFERENCE_B_B2
    vr_bindings_reset = Cvar_VariableInteger("b2_openxr") && !Q_stricmp(path, COM_CONFIG_CFG);
#endif

    len = FS_LoadFileEx(path, (void **)&f, flags, TAG_FILESYSTEM);
    if (!f) {
        return len;
    }

    // Generated engine configs contain a complete binding set. The old shipped
    // seed and custom scripts are overlays and must retain default movement keys.
    if (vr_bindings_reset) {
        static const char marker[] = "// generated by " APPLICATION ", do not modify";
        size_t n = sizeof(marker) - 1;
        vr_bindings_reset = len > n && !memcmp(f, marker, n) &&
            (f[n] == '\n' || (len > n + 1 && f[n] == '\r' && f[n+1] == '\n'));
    }

    // check for binary file
    if (memchr(f, 0, len)) {
        ret = Q_ERR_INVALID_FORMAT;
        goto finish;
    }

    // sanity check file size after stripping off comments
    len = COM_Compress(f);
    if (len > CMD_BUFFER_SIZE) {
        ret = Q_ERR(EFBIG);
        goto finish;
    }

    // FIXME: always insert into main command buffer,
    // no matter where command came from?
    buf = &cmd_buffer;

    // check for exec loop
    if (buf->aliasCount >= ALIAS_LOOP_COUNT) {
        ret = Q_ERR_INFINITE_LOOP;
        goto finish;
    }

    // check for overflow
    if (buf->cursize + len + 1 + (vr_bindings_reset ? sizeof("unbindall") : 0) > buf->maxsize) {
        ret = Q_ERR_STRING_TRUNCATED;
        goto finish;
    }

    // everything ok, execute it
    Com_Printf("Execing %s\n", path);

    buf->aliasCount++;
    Cbuf_InsertText(buf, f);
    // Generated saved bindings are complete: omitted keys remain unbound.
    // Prepend only after a real, valid file was loaded; missing first-run config
    // leaves the immutable default bindings intact. Applies to older saves too.
    if (vr_bindings_reset) Cbuf_InsertText(buf, "unbindall");

    ret = Q_ERR_SUCCESS;

finish:
    FS_FreeFile(f);
    return ret;
}
static char home_binding[128],end_binding[128],value[128];
static void Cbuf_Execute(cmdbuf_t *buf){char *s=buf->text;s[buf->cursize]=0;char *line=strtok(s,"\n");while(line){
 if(!strcmp(line,"unbindall")){*home_binding=*end_binding=0;}
 else if(!strncmp(line,"bind HOME ",10))strcpy(home_binding,line+10);
 else if(!strncmp(line,"bind END ",9))strcpy(end_binding,line+9);
 else if(!strncmp(line,"seta value ",11))strcpy(value,line+11);
 line=strtok(NULL,"\n");}buf->cursize=buf->aliasCount=0;}
void Com_AddConfigFile(const char *name, unsigned flags)
{
    int ret;

    ret = Cmd_ExecuteFile(name, flags);
    if (ret == Q_ERR_SUCCESS) {
        Cbuf_Execute(&cmd_buffer);
    } else if (ret != Q_ERR(ENOENT)) {
        Com_WPrintf("Couldn't exec %s: %s\n", name, Q_ErrorString(ret));
    }
}
void FS_AddConfigFiles(bool init)
{
    int flag = init ? FS_PATH_ANY : FS_PATH_GAME;

    if (!init && !fs_autoexec->integer)
        return;

    // default.cfg and q2rtx.cfg may come from packfile, but config.cfg and autoexec.cfg
    // must be real files within the game directory.
    Com_AddConfigFile(COM_DEFAULT_CFG, flag);
    Com_AddConfigFile(COM_Q2RTX_CFG, FS_PATH_ANY);
#ifdef REFERENCE_B_B2
    if (Cvar_VariableInteger("b2_openxr"))
        Com_AddConfigFile("default-vr.cfg", FS_TYPE_REAL);
#endif
    Com_AddConfigFile(COM_CONFIG_CFG, FS_TYPE_REAL | flag);

    // autoexec.cfg is executed twice, first from basedir and then from
    // gamedir. This ensures user settings configured in baseq2/autoexec.cfg
    // override those in default.cfg and config.cfg.
    if (*fs_game->string)
        Com_AddConfigFile(COM_AUTOEXEC_CFG, FS_TYPE_REAL | FS_PATH_BASE);
    Com_AddConfigFile(COM_AUTOEXEC_CFG, FS_TYPE_REAL | FS_PATH_GAME);

    Com_AddConfigFile(COM_POSTEXEC_CFG, FS_TYPE_REAL);
}
typedef int qhandle_t;
static char *keybindings[256];
static const char *Key_KeynumToString(int i){return i==1?"HOME":i==2?"END":"F1";}
static void FS_FPrintf(qhandle_t f,const char *format,...){va_list a;va_start(a,format);vsnprintf(saved+strlen(saved),sizeof(saved)-strlen(saved),format,a);va_end(a);}
void Key_WriteBindings(qhandle_t f)
{
    int     i;

    for (i = 0; i < 256; i++) {
        if (keybindings[i] && keybindings[i][0]) {
            FS_FPrintf(f, "bind %s \"%s\"\n", Key_KeynumToString(i),
                       keybindings[i]);
        }
    }
}
int main(void){searchpath_t parent={NULL,3,NULL,"parent/baseq2"},home={&parent,3,NULL,"home/baseq2"};fs_searchpaths=&home;
FS_AddConfigFiles(true);assert(!parent_reads&&!strcmp(value,"default")&&!strcmp(home_binding,"recenter"));
home_present=1;FS_AddConfigFiles(true);assert(!parent_reads&&!strcmp(value,"player")&&!strcmp(home_binding,"recenter")&&!strcmp(end_binding,"handoff"));
// A partial legacy overlay retains defaults; a generated saved set is complete.
// The real binding writer omits empty keys, so the generated marker preserves unbinding.
keybindings[1]="";keybindings[2]="custom-handoff";strcpy(saved,"// generated by Q2RTX, do not modify\n");Key_WriteBindings(0);strcat(saved,"seta value custom\n");
assert(!strstr(saved,"HOME"));FS_AddConfigFiles(true);assert(!*home_binding&&strstr(end_binding,"custom-handoff")&&!strcmp(value,"custom"));
// Missing player config must not enqueue a binding reset. Oversize refuses
// before either insertion, including the added unbindall prefix.
home_present=0;cmd_buffer.cursize=0;assert(Cmd_ExecuteFile(COM_CONFIG_CFG,FS_TYPE_REAL)==Q_ERR(ENOENT));assert(!cmd_buffer.cursize);
home_present=1;cmd_buffer.maxsize=strlen(saved)+1;assert(Cmd_ExecuteFile(COM_CONFIG_CFG,FS_TYPE_REAL)==Q_ERR_STRING_TRUNCATED);assert(!cmd_buffer.cursize);cmd_buffer.maxsize=8192;
vr=0;home_present=0;*home_binding=0;FS_AddConfigFiles(true);assert(parent_reads==1&&!strcmp(value,"parent")&&!strcmp(home_binding,"parent"));
puts("actual FS search/config chain + command insertion + binding writer: clean/existing/unbound/parent/flat/buffer cases pass");return 0;}
