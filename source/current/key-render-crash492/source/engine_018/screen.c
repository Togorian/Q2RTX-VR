/* Modified 14 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
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
/* Modified by Togorian with AI assistance, 14 November 2026, for Q2RTX VR
 * (OpenXR VR support and source preparation). See PROVENANCE.md. */

// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

#include "../../../../baseline/b2-consolidated215/inputs/origin-053/src/client/client.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-053/inc/refresh/images.h"
#ifdef REFERENCE_B_B2
#include "../../../../baseline/b2-consolidated215/inputs/origin-053/menu-pause-console/source-v2/src/client/reference_b_b2.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-001/include/b2/reference_b_b2_lifecycle.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-040/include/refresh/b2_menu151.h"
#include "../../../../baseline/b2-consolidated215/inputs/origin-001/include/refresh/b2_weapon_ui.h"
#include "../../../../baseline/b2-resume-cue261/source/b2_resume_cue261.h"
#include "../../../reference-realignment391/source/b2_realign391.h"
#endif

#include "current/input-comfort556/source/b2_client556.h"

#define STAT_PICS       11
#define STAT_MINUS      (STAT_PICS - 1)  // num frame for '-' stats digit

static struct {
    bool        initialized;        // ready to draw

    qhandle_t   crosshair_pic;
    int         crosshair_width, crosshair_height;
    color_t     crosshair_color;

    qhandle_t   pause_pic;
    int         pause_width, pause_height;

    qhandle_t   loading_pic;
    int         loading_width, loading_height;
    bool        draw_loading;

    qhandle_t   sb_pics[2][STAT_PICS];
    qhandle_t   inven_pic;
    qhandle_t   field_pic;

    qhandle_t   backtile_pic;

    qhandle_t   net_pic;
    qhandle_t   font_pic;

    int         hud_width, hud_height;
    float       hud_scale;
    float       hud_alpha;
} scr;

cvar_t   *scr_viewsize;
static cvar_t   *scr_centertime;
static cvar_t   *scr_showpause;
#if USE_DEBUG
static cvar_t   *scr_showstats;
static cvar_t   *scr_showpmove;
#endif
static cvar_t   *scr_showturtle;
static cvar_t   *scr_showitemname;

static cvar_t   *scr_draw2d;
static cvar_t   *scr_lag_x;
static cvar_t   *scr_lag_y;
static cvar_t   *scr_lag_draw;
static cvar_t   *scr_lag_min;
static cvar_t   *scr_lag_max;
static cvar_t   *scr_alpha;
static cvar_t   *scr_fps;
static cvar_t   *b2_showfps347;
#include "../../../hud-cleanup356/source/b2_fps347.h"

static cvar_t   *scr_demobar;
static cvar_t   *scr_font;
static cvar_t   *scr_scale;

static cvar_t   *scr_crosshair;

static cvar_t   *scr_chathud;
static cvar_t   *scr_chathud_lines;
static cvar_t   *scr_chathud_time;
static cvar_t   *scr_chathud_x;
static cvar_t   *scr_chathud_y;

static cvar_t   *ch_health;
static cvar_t   *ch_red;
static cvar_t   *ch_green;
static cvar_t   *ch_blue;
static cvar_t   *ch_alpha;

static cvar_t   *ch_scale;
static cvar_t   *ch_x;
static cvar_t   *ch_y;

vrect_t     scr_vrect;      // position of render window on screen

const uint32_t colorTable[8] = {
    U32_BLACK, U32_RED, U32_GREEN, U32_YELLOW,
    U32_BLUE, U32_CYAN, U32_MAGENTA, U32_WHITE
};

/*
===============================================================================

UTILS

===============================================================================
*/

#define SCR_DrawString(x, y, flags, string) \
    SCR_DrawStringEx(x, y, flags, MAX_STRING_CHARS, string, scr.font_pic)

/*
==============
SCR_DrawStringEx
==============
*/
int SCR_DrawStringEx(int x, int y, int flags, size_t maxlen,
                     const char *s, qhandle_t font)
{
    size_t len = strlen(s);

    if (len > maxlen) {
        len = maxlen;
    }

    if ((flags & UI_CENTER) == UI_CENTER) {
        x -= len * CHAR_WIDTH / 2;
    } else if (flags & UI_RIGHT) {
        x -= len * CHAR_WIDTH;
    }

    return R_DrawString(x, y, flags, maxlen, s, font);
}


/*
==============
SCR_DrawStringMulti
==============
*/
void SCR_DrawStringMulti(int x, int y, int flags, size_t maxlen,
                         const char *s, qhandle_t font)
{
    char    *p;
    size_t  len;

    while (*s) {
        p = strchr(s, '\n');
        if (!p) {
            SCR_DrawStringEx(x, y, flags, maxlen, s, font);
            break;
        }

        len = p - s;
        if (len > maxlen) {
            len = maxlen;
        }
        SCR_DrawStringEx(x, y, flags, len, s, font);

        y += CHAR_HEIGHT;
        s = p + 1;
    }
}


/*
=================
SCR_FadeAlpha
=================
*/
float SCR_FadeAlpha(unsigned startTime, unsigned visTime, unsigned fadeTime)
{
    float alpha;
    unsigned timeLeft, delta = cls.realtime - startTime;

    if (delta >= visTime) {
        return 0;
    }

    if (fadeTime > visTime) {
        fadeTime = visTime;
    }

    alpha = 1;
    timeLeft = visTime - delta;
    if (timeLeft < fadeTime) {
        alpha = (float)timeLeft / fadeTime;
    }

    return alpha;
}

bool SCR_ParseColor(const char *s, color_t *color)
{
    int i;
    int c[8];

    // parse generic color
    if (*s == '#') {
        s++;
        for (i = 0; s[i]; i++) {
            if (i == 8) {
                return false;
            }
            c[i] = Q_charhex(s[i]);
            if (c[i] == -1) {
                return false;
            }
        }

        switch (i) {
        case 3:
            color->u8[0] = c[0] | (c[0] << 4);
            color->u8[1] = c[1] | (c[1] << 4);
            color->u8[2] = c[2] | (c[2] << 4);
            color->u8[3] = 255;
            break;
        case 6:
            color->u8[0] = c[1] | (c[0] << 4);
            color->u8[1] = c[3] | (c[2] << 4);
            color->u8[2] = c[5] | (c[4] << 4);
            color->u8[3] = 255;
            break;
        case 8:
            color->u8[0] = c[1] | (c[0] << 4);
            color->u8[1] = c[3] | (c[2] << 4);
            color->u8[2] = c[5] | (c[4] << 4);
            color->u8[3] = c[7] | (c[6] << 4);
            break;
        default:
            return false;
        }

        return true;
    }

    // parse name or index
    i = Com_ParseColor(s);
    if (i >= q_countof(colorTable)) {
        return false;
    }

    color->u32 = colorTable[i];
    return true;
}

/*
===============================================================================

BAR GRAPHS

===============================================================================
*/

static void draw_progress_bar(float progress, bool paused, int framenum)
{
    char buffer[16];
    int x, w, h;
    size_t len;

    w = Q_rint(scr.hud_width * progress);
    h = Q_rint(CHAR_HEIGHT / scr.hud_scale);

    scr.hud_height -= h;

    R_DrawFill8(0, scr.hud_height, w, h, 4);
    R_DrawFill8(w, scr.hud_height, scr.hud_width - w, h, 0);

    R_SetScale(scr.hud_scale);

    w = Q_rint(scr.hud_width * scr.hud_scale);
    h = Q_rint(scr.hud_height * scr.hud_scale);

    len = Q_scnprintf(buffer, sizeof(buffer), "%.f%%", progress * 100);
    x = (w - len * CHAR_WIDTH) / 2;
    R_DrawString(x, h, 0, MAX_STRING_CHARS, buffer, scr.font_pic);

    if (scr_demobar->integer > 1) {
        int sec = framenum / 10;
        int min = sec / 60; sec %= 60;

        Q_scnprintf(buffer, sizeof(buffer), "%d:%02d.%d", min, sec, framenum % 10);
        R_DrawString(0, h, 0, MAX_STRING_CHARS, buffer, scr.font_pic);
    }

    if (paused) {
        SCR_DrawString(w, h, UI_RIGHT, "[PAUSED]");
    }

    R_SetScale(1.0f);
}

static void SCR_DrawDemo(void)
{
#if USE_MVD_CLIENT
    float progress;
    bool paused;
    int framenum;
#endif

    if (!scr_demobar->integer) {
        return;
    }

    if (cls.demo.playback) {
        if (cls.demo.file_size) {
            draw_progress_bar(
                cls.demo.file_progress,
                sv_paused->integer &&
                cl_paused->integer &&
                scr_showpause->integer == 2,
                cls.demo.frames_read);
        }
        return;
    }

#if USE_MVD_CLIENT
    if (sv_running->integer != ss_broadcast) {
        return;
    }

    if (!MVD_GetDemoStatus(&progress, &paused, &framenum)) {
        return;
    }

    if (sv_paused->integer && cl_paused->integer && scr_showpause->integer == 2) {
        paused = true;
    }

    draw_progress_bar(progress, paused, framenum);
#endif
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

static char     scr_centerstring[MAX_STRING_CHARS];
static unsigned scr_centertime_start;   // for slow victory printing
static int      scr_center_lines;

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint(const char *str)
{
    const char  *s;

    scr_centertime_start = cls.realtime;
    if (!strcmp(scr_centerstring, str)) {
        return;
    }

    Q_strlcpy(scr_centerstring, str, sizeof(scr_centerstring));

    // count the number of lines for centering
    scr_center_lines = 1;
    s = str;
    while (*s) {
        if (*s == '\n')
            scr_center_lines++;
        s++;
    }

    // echo it to the console
    Com_Printf("%s\n", scr_centerstring);
    Con_ClearNotify_f();
}

static void SCR_DrawCenterString(void)
{
    int y;
    float alpha;

    Cvar_ClampValue(scr_centertime, 0.3f, 10.0f);

    alpha = SCR_FadeAlpha(scr_centertime_start, scr_centertime->value * 1000, 300);
    if (!alpha) {
        return;
    }

    R_SetAlpha(alpha * scr_alpha->value);

    y = scr.hud_height / 4 - scr_center_lines * 8 / 2;

    SCR_DrawStringMulti(scr.hud_width / 2, y, UI_CENTER,
                        MAX_STRING_CHARS, scr_centerstring, scr.font_pic);

    R_SetAlpha(scr_alpha->value);
}

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define LAG_WIDTH   48
#define LAG_HEIGHT  48

#define LAG_WARN_BIT    BIT(30)
#define LAG_CRIT_BIT    BIT(31)

#define LAG_BASE    0xD5
#define LAG_WARN    0xDC
#define LAG_CRIT    0xF2

static struct {
    unsigned samples[LAG_WIDTH];
    unsigned head;
} lag;

void SCR_LagClear(void)
{
    lag.head = 0;
}

void SCR_LagSample(void)
{
    int i = cls.netchan.incoming_acknowledged & CMD_MASK;
    client_history_t *h = &cl.history[i];
    unsigned ping;

    h->rcvd = cls.realtime;
    if (!h->cmdNumber || h->rcvd < h->sent) {
        return;
    }

    ping = h->rcvd - h->sent;
    for (i = 0; i < cls.netchan.dropped; i++) {
        lag.samples[lag.head % LAG_WIDTH] = ping | LAG_CRIT_BIT;
        lag.head++;
    }

    if (cl.frameflags & FF_SUPPRESSED) {
        ping |= LAG_WARN_BIT;
    }
    lag.samples[lag.head % LAG_WIDTH] = ping;
    lag.head++;
}

static void SCR_LagDraw(int x, int y)
{
    int i, j, v, c, v_min, v_max, v_range;

    v_min = Cvar_ClampInteger(scr_lag_min, 0, LAG_HEIGHT * 10);
    v_max = Cvar_ClampInteger(scr_lag_max, 0, LAG_HEIGHT * 10);

    v_range = v_max - v_min;
    if (v_range < 1)
        return;

    for (i = 0; i < LAG_WIDTH; i++) {
        j = lag.head - i - 1;
        if (j < 0) {
            break;
        }

        v = lag.samples[j % LAG_WIDTH];

        if (v & LAG_CRIT_BIT) {
            c = LAG_CRIT;
        } else if (v & LAG_WARN_BIT) {
            c = LAG_WARN;
        } else {
            c = LAG_BASE;
        }

        v &= ~(LAG_WARN_BIT | LAG_CRIT_BIT);
        v = Q_clip((v - v_min) * LAG_HEIGHT / v_range, 0, LAG_HEIGHT);

        R_DrawFill8(x + LAG_WIDTH - i - 1, y + LAG_HEIGHT - v, 1, v, c);
    }
}

static void SCR_DrawNet(void)
{
    int x = scr_lag_x->integer;
    int y = scr_lag_y->integer;

    if (x < 0) {
        x += scr.hud_width - LAG_WIDTH + 1;
    }
    if (y < 0) {
        y += scr.hud_height - LAG_HEIGHT + 1;
    }

    // draw ping graph
    if (scr_lag_draw->integer) {
        if (scr_lag_draw->integer > 1) {
            R_DrawFill8(x, y, LAG_WIDTH, LAG_HEIGHT, 4);
        }
        SCR_LagDraw(x, y);
    }

    // draw phone jack
    if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged >= CMD_BACKUP) {
        if ((cls.realtime >> 8) & 3) {
            R_DrawStretchPic(x, y, LAG_WIDTH, LAG_HEIGHT, scr.net_pic);
        }
    }
}


/*
===============================================================================

DRAW OBJECTS

===============================================================================
*/

typedef struct {
    list_t          entry;
    int             x, y;
    cvar_t          *cvar;
    cmd_macro_t     *macro;
    int             flags;
    color_t         color;
} drawobj_t;

#define FOR_EACH_DRAWOBJ(obj) \
    LIST_FOR_EACH(drawobj_t, obj, &scr_objects, entry)
#define FOR_EACH_DRAWOBJ_SAFE(obj, next) \
    LIST_FOR_EACH_SAFE(drawobj_t, obj, next, &scr_objects, entry)

static LIST_DECL(scr_objects);

static void SCR_Color_g(genctx_t *ctx)
{
    int color;

    for (color = 0; color < COLOR_COUNT; color++)
        Prompt_AddMatch(ctx, colorNames[color]);
}

static void SCR_Draw_c(genctx_t *ctx, int argnum)
{
    if (argnum == 1) {
        Cvar_Variable_g(ctx);
        Cmd_Macro_g(ctx);
    } else if (argnum == 4) {
        SCR_Color_g(ctx);
    }
}

// draw cl_fps -1 80
static void SCR_Draw_f(void)
{
    int x, y;
    const char *s, *c;
    drawobj_t *obj;
    cmd_macro_t *macro;
    cvar_t *cvar;
    color_t color;
    int flags;
    int argc = Cmd_Argc();

    if (argc == 1) {
        if (LIST_EMPTY(&scr_objects)) {
            Com_Printf("No draw strings registered.\n");
            return;
        }
        Com_Printf("Name               X    Y\n"
                   "--------------- ---- ----\n");
        FOR_EACH_DRAWOBJ(obj) {
            s = obj->macro ? obj->macro->name : obj->cvar->name;
            Com_Printf("%-15s %4d %4d\n", s, obj->x, obj->y);
        }
        return;
    }

    if (argc < 4) {
        Com_Printf("Usage: %s <name> <x> <y> [color]\n", Cmd_Argv(0));
        return;
    }

    color.u32 = U32_BLACK;
    flags = UI_IGNORECOLOR;

    s = Cmd_Argv(1);
    x = Q_atoi(Cmd_Argv(2));
    y = Q_atoi(Cmd_Argv(3));

    if (x < 0) {
        flags |= UI_RIGHT;
    }

    if (argc > 4) {
        c = Cmd_Argv(4);
        if (!strcmp(c, "alt")) {
            flags |= UI_ALTCOLOR;
        } else if (strcmp(c, "none")) {
            if (!SCR_ParseColor(c, &color)) {
                Com_Printf("Unknown color '%s'\n", c);
                return;
            }
            flags &= ~UI_IGNORECOLOR;
        }
    }

    cvar = NULL;
    macro = Cmd_FindMacro(s);
    if (!macro) {
        cvar = Cvar_WeakGet(s);
    }

    FOR_EACH_DRAWOBJ(obj) {
        if (obj->macro == macro && obj->cvar == cvar) {
            obj->x = x;
            obj->y = y;
            obj->flags = flags;
            obj->color.u32 = color.u32;
            return;
        }
    }

    obj = Z_Malloc(sizeof(*obj));
    obj->x = x;
    obj->y = y;
    obj->cvar = cvar;
    obj->macro = macro;
    obj->flags = flags;
    obj->color.u32 = color.u32;

    List_Append(&scr_objects, &obj->entry);
}

static void SCR_Draw_g(genctx_t *ctx)
{
    drawobj_t *obj;
    const char *s;

    if (LIST_EMPTY(&scr_objects)) {
        return;
    }

    Prompt_AddMatch(ctx, "all");

    FOR_EACH_DRAWOBJ(obj) {
        s = obj->macro ? obj->macro->name : obj->cvar->name;
        Prompt_AddMatch(ctx, s);
    }
}

static void SCR_UnDraw_c(genctx_t *ctx, int argnum)
{
    if (argnum == 1) {
        SCR_Draw_g(ctx);
    }
}

static void SCR_UnDraw_f(void)
{
    char *s;
    drawobj_t *obj, *next;
    cmd_macro_t *macro;
    cvar_t *cvar;

    if (Cmd_Argc() != 2) {
        Com_Printf("Usage: %s <name>\n", Cmd_Argv(0));
        return;
    }

    if (LIST_EMPTY(&scr_objects)) {
        Com_Printf("No draw strings registered.\n");
        return;
    }

    s = Cmd_Argv(1);
    if (!strcmp(s, "all")) {
        FOR_EACH_DRAWOBJ_SAFE(obj, next) {
            Z_Free(obj);
        }
        List_Init(&scr_objects);
        Com_Printf("Deleted all draw strings.\n");
        return;
    }

    cvar = NULL;
    macro = Cmd_FindMacro(s);
    if (!macro) {
        cvar = Cvar_WeakGet(s);
    }

    FOR_EACH_DRAWOBJ_SAFE(obj, next) {
        if (obj->macro == macro && obj->cvar == cvar) {
            List_Remove(&obj->entry);
            Z_Free(obj);
            return;
        }
    }

    Com_Printf("Draw string '%s' not found.\n", s);
}

static void SCR_DrawObjects(void)
{
    char buffer[MAX_QPATH];
    int x, y;
    drawobj_t *obj;

    FOR_EACH_DRAWOBJ(obj) {
        // Keep the single VR counter clear of retired comparison readouts,
        // including registrations restored by an old startup file.
        if (Cvar_VariableInteger("b2_openxr")) {
            const char *name=obj->macro ? obj->macro->name : obj->cvar->name;
            if ((obj->macro && !strcmp(name,"r_fps")) ||
                !strcmp(name,"b2_hack_hud_mode") ||
                !strcmp(name,"b2_hack_hud_values") ||
                !strcmp(name,"b2_hack_hud_live") ||
                !strcmp(name,"b2_hack_mode")) continue;
        }
        x = obj->x;
        y = obj->y;
        if (x < 0) {
            x += scr.hud_width + 1;
        }
        if (y < 0) {
            y += scr.hud_height - CHAR_HEIGHT + 1;
        }
        if (!(obj->flags & UI_IGNORECOLOR)) {
            R_SetColor(obj->color.u32);
        }
        if (obj->macro) {
            obj->macro->function(buffer, sizeof(buffer));
            SCR_DrawString(x, y, obj->flags, buffer);
        } else {
            SCR_DrawString(x, y, obj->flags, obj->cvar->string);
        }
        if (!(obj->flags & UI_IGNORECOLOR)) {
            R_ClearColor();
            R_SetAlpha(scr_alpha->value);
        }
    }
}

extern int CL_GetFps(void);
extern int CL_GetResolutionScale(void);

// Split only at the existing space: numeric glyphs gain colour, labels retain white.
static void SCR_DrawFPSValue351(int x,int y,char *text,unsigned color)
{
    char *label=strchr(text,' ');
    if (!label) return;
    *label=0;
    R_SetColor(color);
    SCR_DrawString(x,y,0,text);
    *label=' ';
    R_SetColor(~0u);
    SCR_DrawString(x+(int)(label-text)*CHAR_WIDTH,y,0,label);
}

static void SCR_DrawFPS(void)
{
    if (Cvar_VariableInteger("b2_openxr")) {
        if (b2_showfps347->integer) {
            char fps[32],ms[32];B2FPS_Text347(fps,sizeof(fps),ms,sizeof(ms));
            int x=80,y=48;
            unsigned color=B2FPS_Color351();
            SCR_DrawFPSValue351(x,y,fps,color);
            x+=((int)strlen(fps)+1)*CHAR_WIDTH;
            // The existing bitmap font has no Unicode middle-dot mapping.
            // Raise its period within this same character cell and font scale.
            SCR_DrawString(x,y-3,0,".");
            SCR_DrawFPSValue351(x+2*CHAR_WIDTH,y,ms,color);
            R_ClearColor();R_SetAlpha(scr_alpha->value);
        }
        return;
    }
	if (scr_fps->integer == 0)
		return;

	int fps = R_FPS;
	int scale = CL_GetResolutionScale();

	char buffer[MAX_QPATH];
	if (scr_fps->integer == 2 && cls.ref_type == REF_TYPE_VKPT)
		Q_snprintf(buffer, MAX_QPATH, "%d FPS at %3d%%", fps, scale);
	else
		Q_snprintf(buffer, MAX_QPATH, "%d FPS", fps);

	int x = scr.hud_width - 2;
	int y = 1;

	R_SetColor(~0u);
	SCR_DrawString(x, y, UI_RIGHT, buffer);
}

/*
===============================================================================

CHAT HUD

===============================================================================
*/

#define MAX_CHAT_TEXT       150
#define MAX_CHAT_LINES      32
#define CHAT_LINE_MASK      (MAX_CHAT_LINES - 1)

typedef struct {
    char        text[MAX_CHAT_TEXT];
    unsigned    time;
} chatline_t;

static chatline_t   scr_chatlines[MAX_CHAT_LINES];
static unsigned     scr_chathead;

void SCR_ClearChatHUD_f(void)
{
    memset(scr_chatlines, 0, sizeof(scr_chatlines));
    scr_chathead = 0;
}

void SCR_AddToChatHUD(const char *text)
{
    chatline_t *line;
    char *p;

    line = &scr_chatlines[scr_chathead++ & CHAT_LINE_MASK];
    Q_strlcpy(line->text, text, sizeof(line->text));
    line->time = cls.realtime;

    p = strrchr(line->text, '\n');
    if (p)
        *p = 0;
}

static void SCR_DrawChatHUD(void)
{
    int x, y, i, lines, flags, step;
    float alpha;
    chatline_t *line;

    if (scr_chathud->integer == 0)
        return;

    x = scr_chathud_x->integer;
    y = scr_chathud_y->integer;

    if (scr_chathud->integer == 2)
        flags = UI_ALTCOLOR;
    else
        flags = 0;

    if (x < 0) {
        x += scr.hud_width + 1;
        flags |= UI_RIGHT;
    } else {
        flags |= UI_LEFT;
    }

    if (y < 0) {
        y += scr.hud_height - CHAR_HEIGHT + 1;
        step = -CHAR_HEIGHT;
    } else {
        step = CHAR_HEIGHT;
    }

    lines = scr_chathud_lines->integer;
    if (lines > scr_chathead)
        lines = scr_chathead;

    for (i = 0; i < lines; i++) {
        line = &scr_chatlines[(scr_chathead - i - 1) & CHAT_LINE_MASK];

        if (scr_chathud_time->integer) {
            alpha = SCR_FadeAlpha(line->time, scr_chathud_time->integer, 1000);
            if (!alpha)
                break;

            R_SetAlpha(alpha * scr_alpha->value);
            SCR_DrawString(x, y, flags, line->text);
            R_SetAlpha(scr_alpha->value);
        } else {
            SCR_DrawString(x, y, flags, line->text);
        }

        y += step;
    }
}

/*
===============================================================================

DEBUG STUFF

===============================================================================
*/

static void SCR_DrawTurtle(void)
{
    int x, y;

    if (scr_showturtle->integer <= 0)
        return;

    if (!cl.frameflags)
        return;

    x = CHAR_WIDTH;
    y = scr.hud_height - 11 * CHAR_HEIGHT;

#define DF(f) \
    if (cl.frameflags & FF_##f) { \
        SCR_DrawString(x, y, UI_ALTCOLOR, #f); \
        y += CHAR_HEIGHT; \
    }

    if (scr_showturtle->integer > 1) {
        DF(SUPPRESSED)
    }
    DF(CLIENTPRED)
    if (scr_showturtle->integer > 1) {
        DF(CLIENTDROP)
        DF(SERVERDROP)
    }
    DF(BADFRAME)
    DF(OLDFRAME)
    DF(OLDENT)
    DF(NODELTA)

#undef DF
}

#if USE_DEBUG

static void SCR_DrawDebugStats(void)
{
    char buffer[MAX_QPATH];
    int i, j;
    int x, y;

    j = scr_showstats->integer;
    if (j <= 0)
        return;

    if (j > MAX_STATS)
        j = MAX_STATS;

    x = CHAR_WIDTH;
    y = (scr.hud_height - j * CHAR_HEIGHT) / 2;
    for (i = 0; i < j; i++) {
        Q_snprintf(buffer, sizeof(buffer), "%2d: %d", i, cl.frame.ps.stats[i]);
        if (cl.oldframe.ps.stats[i] != cl.frame.ps.stats[i]) {
            R_SetColor(U32_RED);
        }
        R_DrawString(x, y, 0, MAX_STRING_CHARS, buffer, scr.font_pic);
        R_ClearColor();
        y += CHAR_HEIGHT;
    }
}

static void SCR_DrawDebugPmove(void)
{
    static const char * const types[] = {
        "NORMAL", "SPECTATOR", "DEAD", "GIB", "FREEZE"
    };
    static const char * const flags[] = {
        "DUCKED", "JUMP_HELD", "ON_GROUND",
        "TIME_WATERJUMP", "TIME_LAND", "TIME_TELEPORT",
        "NO_PREDICTION", "TELEPORT_BIT"
    };
    unsigned i, j;
    int x, y;

    if (!scr_showpmove->integer)
        return;

    x = CHAR_WIDTH;
    y = (scr.hud_height - 2 * CHAR_HEIGHT) / 2;

    i = cl.frame.ps.pmove.pm_type;
    if (i > PM_FREEZE)
        i = PM_FREEZE;

    R_DrawString(x, y, 0, MAX_STRING_CHARS, types[i], scr.font_pic);
    y += CHAR_HEIGHT;

    j = cl.frame.ps.pmove.pm_flags;
    for (i = 0; i < 8; i++) {
        if (j & (1 << i)) {
            x = R_DrawString(x, y, 0, MAX_STRING_CHARS, flags[i], scr.font_pic);
            x += CHAR_WIDTH;
        }
    }
}

#endif

//============================================================================

// Sets scr_vrect, the coordinates of the rendered window
void SCR_CalcVrect(void)
{
    scr_vrect.width = scr.hud_width;
    scr_vrect.height = scr.hud_height;
    scr_vrect.x = 0;
    scr_vrect.y = 0;
}

/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
// VR has one process-selected eye extent; use the existing restart setting.
static bool SCR_B2Size492(int direction)
{
#ifdef REFERENCE_B_B2
    if (Cvar_VariableInteger("b2_openxr")) {
        cvar_t *scale=Cvar_Get("b2_render_percent","100",CVAR_ARCHIVE);
        if (!isfinite(scale->value) || scale->value!=scale->integer ||
            scale->integer<10 || scale->integer>100 || scale->integer%5) {
            Com_Printf("VR resolution setting invalid; use VR Graphics Settings.\n");
            return true;
        }
        int next=Q_clip(scale->integer+direction*5,10,100);
        Cvar_SetInteger(scale,next,FROM_CONSOLE);
        Com_Printf("VR resolution: %d%% pending next launch (current eyes unchanged).\n",next);
        return true;
    }
#endif
    return false;
}

static void SCR_SizeUp_f(void)
{
    if (SCR_B2Size492(1)) return;
	int delta = (scr_viewsize->integer < 100) ? 5 : 10;
    Cvar_SetInteger(scr_viewsize, scr_viewsize->integer + delta, FROM_CONSOLE);
}

/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
static void SCR_SizeDown_f(void)
{
    if (SCR_B2Size492(-1)) return;
	int delta = (scr_viewsize->integer <= 100) ? 5 : 10;
	Cvar_SetInteger(scr_viewsize, scr_viewsize->integer - delta, FROM_CONSOLE);
}

/*
=================
SCR_Sky_f

Set a specific sky and rotation speed. If empty sky name is provided, falls
back to server defaults.
=================
*/
static void SCR_Sky_f(void)
{
    char    *name;
    float   rotate;
    vec3_t  axis;
    int     argc = Cmd_Argc();

    if (argc < 2) {
        Com_Printf("Usage: sky <basename> [rotate] [axis x y z]\n");
        return;
    }

    if (cls.state != ca_active) {
        Com_Printf("No map loaded.\n");
        return;
    }

    name = Cmd_Argv(1);
    if (!*name) {
        CL_SetSky();
        return;
    }

    if (argc > 2)
        rotate = Q_atof(Cmd_Argv(2));
    else
        rotate = 0;

    if (argc == 6) {
        axis[0] = Q_atof(Cmd_Argv(3));
        axis[1] = Q_atof(Cmd_Argv(4));
        axis[2] = Q_atof(Cmd_Argv(5));
    } else
        VectorSet(axis, 0, 0, 1);

    R_SetSky(name, rotate, 1, axis);
}

/*
================
SCR_TimeRefresh_f
================
*/
static void SCR_TimeRefresh_f(void)
{
    int     i;
    unsigned    start, stop;
    float       time;

    if (cls.state != ca_active) {
        Com_Printf("No map loaded.\n");
        return;
    }

    start = Sys_Milliseconds();

    if (Cmd_Argc() == 2) {
        // run without page flipping
        R_BeginFrame();
        for (i = 0; i < 128; i++) {
            cl.refdef.viewangles[1] = i / 128.0f * 360.0f;
            R_RenderFrame(&cl.refdef);
        }
        R_EndFrame();
    } else {
        for (i = 0; i < 128; i++) {
            cl.refdef.viewangles[1] = i / 128.0f * 360.0f;

            R_BeginFrame();
            R_RenderFrame(&cl.refdef);
            R_EndFrame();
        }
    }

    stop = Sys_Milliseconds();
    time = (stop - start) * 0.001f;
    Com_Printf("%f seconds (%f fps)\n", time, 128.0f / time);
}


//============================================================================

static void scr_crosshair_changed(cvar_t *self)
{
    char buffer[16];
    int w, h;
    float scale;

    if (scr_crosshair->integer > 0) {
        Q_snprintf(buffer, sizeof(buffer), "ch%i", scr_crosshair->integer);
        scr.crosshair_pic = R_RegisterPic(buffer);
        R_GetPicSize(&w, &h, scr.crosshair_pic);

        // prescale
        scale = Cvar_ClampValue(ch_scale, 0.1f, 9.0f);
        scr.crosshair_width = w * scale;
        scr.crosshair_height = h * scale;
        if (scr.crosshair_width < 1)
            scr.crosshair_width = 1;
        if (scr.crosshair_height < 1)
            scr.crosshair_height = 1;

        if (ch_health->integer) {
            SCR_SetCrosshairColor();
        } else {
            scr.crosshair_color.u8[0] = Cvar_ClampValue(ch_red, 0, 1) * 255;
            scr.crosshair_color.u8[1] = Cvar_ClampValue(ch_green, 0, 1) * 255;
            scr.crosshair_color.u8[2] = Cvar_ClampValue(ch_blue, 0, 1) * 255;
        }
        scr.crosshair_color.u8[3] = Cvar_ClampValue(ch_alpha, 0, 1) * 255;
    } else {
        scr.crosshair_pic = 0;
    }
}

void SCR_SetCrosshairColor(void)
{
    int health;

    if (!ch_health->integer) {
        return;
    }

    health = cl.frame.ps.stats[STAT_HEALTH];
    if (health <= 0) {
        VectorSet(scr.crosshair_color.u8, 0, 0, 0);
        return;
    }

    // red
    scr.crosshair_color.u8[0] = 255;

    // green
    if (health >= 66) {
        scr.crosshair_color.u8[1] = 255;
    } else if (health < 33) {
        scr.crosshair_color.u8[1] = 0;
    } else {
        scr.crosshair_color.u8[1] = (255 * (health - 33)) / 33;
    }

    // blue
    if (health >= 99) {
        scr.crosshair_color.u8[2] = 255;
    } else if (health < 66) {
        scr.crosshair_color.u8[2] = 0;
    } else {
        scr.crosshair_color.u8[2] = (255 * (health - 66)) / 33;
    }
}

void SCR_ModeChanged(void)
{
    IN_Activate();
    Con_CheckResize();
    UI_ModeChanged();
    cls.disable_screen = 0;
    if (scr.initialized)
        scr.hud_scale = R_ClampScale(scr_scale);

    scr.hud_alpha = 1.f;
}

/*
==================
SCR_RegisterMedia
==================
*/
void SCR_RegisterMedia(void)
{
    int     i;

    for (i = 0; i < STAT_MINUS; i++)
        scr.sb_pics[0][i] = R_RegisterPic(va("num_%d", i));
    scr.sb_pics[0][i] = R_RegisterPic("num_minus");

    for (i = 0; i < STAT_MINUS; i++)
        scr.sb_pics[1][i] = R_RegisterPic(va("anum_%d", i));
    scr.sb_pics[1][i] = R_RegisterPic("anum_minus");

    scr.inven_pic = R_RegisterPic("inventory");
    scr.field_pic = R_RegisterPic("field_3");

    scr.backtile_pic = R_RegisterImage("backtile", IT_PIC, IF_PERMANENT | IF_REPEAT);

    scr.pause_pic = R_RegisterPic("pause");
    R_GetPicSize(&scr.pause_width, &scr.pause_height, scr.pause_pic);

    scr.loading_pic = R_RegisterPic("loading");
    R_GetPicSize(&scr.loading_width, &scr.loading_height, scr.loading_pic);

    scr.net_pic = R_RegisterPic("net");
    scr.font_pic = R_RegisterFont(scr_font->string);

    scr_crosshair_changed(scr_crosshair);
}

static void scr_font_changed(cvar_t *self)
{
    scr.font_pic = R_RegisterFont(self->string);
}

static void scr_scale_changed(cvar_t *self)
{
    scr.hud_scale = R_ClampScale(self);
}

static const cmdreg_t scr_cmds[] = {
    { "timerefresh", SCR_TimeRefresh_f },
    { "sizeup", SCR_SizeUp_f },
    { "sizedown", SCR_SizeDown_f },
    { "sky", SCR_Sky_f },
    { "draw", SCR_Draw_f, SCR_Draw_c },
    { "undraw", SCR_UnDraw_f, SCR_UnDraw_c },
    { "clearchathud", SCR_ClearChatHUD_f },
    { NULL }
};

/*
==================
SCR_Init
==================
*/
void SCR_Init(void)
{
    scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);
    scr_showpause = Cvar_Get("scr_showpause", "1", 0);
    scr_centertime = Cvar_Get("scr_centertime", "2.5", 0);
    scr_demobar = Cvar_Get("scr_demobar", "1", 0);
    scr_font = Cvar_Get("scr_font", "conchars", 0);
    scr_font->changed = scr_font_changed;
    scr_scale = Cvar_Get("scr_scale", "0", 0);
    scr_scale->changed = scr_scale_changed;
    scr_crosshair = Cvar_Get("crosshair", "0", CVAR_ARCHIVE);
    scr_crosshair->changed = scr_crosshair_changed;

    scr_chathud = Cvar_Get("scr_chathud", "0", 0);
    scr_chathud_lines = Cvar_Get("scr_chathud_lines", "4", 0);
    scr_chathud_time = Cvar_Get("scr_chathud_time", "0", 0);
    scr_chathud_time->changed = cl_timeout_changed;
    scr_chathud_time->changed(scr_chathud_time);
    scr_chathud_x = Cvar_Get("scr_chathud_x", "8", 0);
    scr_chathud_y = Cvar_Get("scr_chathud_y", "-64", 0);

    ch_health = Cvar_Get("ch_health", "0", 0);
    ch_health->changed = scr_crosshair_changed;
    ch_red = Cvar_Get("ch_red", "1", 0);
    ch_red->changed = scr_crosshair_changed;
    ch_green = Cvar_Get("ch_green", "1", 0);
    ch_green->changed = scr_crosshair_changed;
    ch_blue = Cvar_Get("ch_blue", "1", 0);
    ch_blue->changed = scr_crosshair_changed;
    ch_alpha = Cvar_Get("ch_alpha", "1", 0);
    ch_alpha->changed = scr_crosshair_changed;

    ch_scale = Cvar_Get("ch_scale", "1", 0);
    ch_scale->changed = scr_crosshair_changed;
    ch_x = Cvar_Get("ch_x", "0", 0);
    ch_y = Cvar_Get("ch_y", "0", 0);

    scr_draw2d = Cvar_Get("scr_draw2d", "2", 0);
    scr_showturtle = Cvar_Get("scr_showturtle", "1", 0);
    scr_showitemname = Cvar_Get("scr_showitemname", "1", CVAR_ARCHIVE);
    scr_lag_x = Cvar_Get("scr_lag_x", "-1", 0);
    scr_lag_y = Cvar_Get("scr_lag_y", "-1", 0);
    scr_lag_draw = Cvar_Get("scr_lag_draw", "0", 0);
    scr_lag_min = Cvar_Get("scr_lag_min", "0", 0);
    scr_lag_max = Cvar_Get("scr_lag_max", "200", 0);
	scr_alpha = Cvar_Get("scr_alpha", "1", 0);
	scr_fps = Cvar_Get("scr_fps", "0", CVAR_ARCHIVE);
    b2_showfps347 = Cvar_Get("b2_showfps", "0", CVAR_ARCHIVE);
#ifdef USE_DEBUG
    scr_showstats = Cvar_Get("scr_showstats", "0", 0);
    scr_showpmove = Cvar_Get("scr_showpmove", "0", 0);
#endif

    Cmd_Register(scr_cmds);

    scr_scale_changed(scr_scale);

    scr.initialized = true;
}

void SCR_Shutdown(void)
{
    Cmd_Deregister(scr_cmds);
    scr.initialized = false;
}

//=============================================================================

/*
================
SCR_BeginLoadingPlaque
================
*/
void SCR_BeginLoadingPlaque(void)
{
    if (!cls.state) {
        return;
    }

    S_StopAllSounds();
    OGG_Stop();

    if (cls.disable_screen) {
        return;
    }

#if USE_DEBUG
    if (developer->integer) {
        return;
    }
#endif

    // if at console or menu, don't bring up the plaque
    if (cls.key_dest & (KEY_CONSOLE | KEY_MENU)) {
        return;
    }

    scr.draw_loading = true;
    SCR_UpdateScreen();

    cls.disable_screen = Sys_Milliseconds();
}

/*
================
SCR_EndLoadingPlaque
================
*/
void SCR_EndLoadingPlaque(void)
{
    if (!cls.state) {
        return;
    }
    cls.disable_screen = 0;
    Con_ClearNotify_f();
}

// Clear any parts of the tiled background that were drawn on last frame
static void SCR_TileClear(void)
{
}

/*
===============================================================================

STAT PROGRAMS

===============================================================================
*/

#define ICON_WIDTH  24
#define ICON_HEIGHT 24
#define DIGIT_WIDTH 16
#define ICON_SPACE  8

#define HUD_DrawString(x, y, string) \
    R_DrawString(x, y, 0, MAX_STRING_CHARS, string, scr.font_pic)

#define HUD_DrawAltString(x, y, string) \
    R_DrawString(x, y, UI_XORCOLOR, MAX_STRING_CHARS, string, scr.font_pic)

#define HUD_DrawCenterString(x, y, string) \
    SCR_DrawStringMulti(x, y, UI_CENTER, MAX_STRING_CHARS, string, scr.font_pic)

#define HUD_DrawAltCenterString(x, y, string) \
    SCR_DrawStringMulti(x, y, UI_CENTER | UI_XORCOLOR, MAX_STRING_CHARS, string, scr.font_pic)

#define HUD_DrawRightString(x, y, string) \
    SCR_DrawStringEx(x, y, UI_RIGHT, MAX_STRING_CHARS, string, scr.font_pic)

#define HUD_DrawAltRightString(x, y, string) \
    SCR_DrawStringEx(x, y, UI_RIGHT | UI_XORCOLOR, MAX_STRING_CHARS, string, scr.font_pic)

static void HUD_DrawNumber(int x, int y, int color, int width, int value)
{
    char    num[16], *ptr;
    int     l;
    int     frame;

    if (width < 1)
        return;

    // draw number string
    if (width > 5)
        width = 5;

    color &= 1;

    l = Q_scnprintf(num, sizeof(num), "%i", value);
    if (l > width)
        l = width;
    x += 2 + DIGIT_WIDTH * (width - l);

    ptr = num;
    while (*ptr && l) {
        if (*ptr == '-')
            frame = STAT_MINUS;
        else
            frame = *ptr - '0';

        R_DrawPic(x, y, scr.sb_pics[color][frame]);
        x += DIGIT_WIDTH;
        ptr++;
        l--;
    }
}

#define DISPLAY_ITEMS   17

static void SCR_DrawInventory(void)
{
    int     i;
    int     num, selected_num, item;
    int     index[MAX_ITEMS];
    char    string[MAX_STRING_CHARS];
    int     x, y;
    const char  *bind;
    int     selected;
    int     top;

    if (!(cl.frame.ps.stats[STAT_LAYOUTS] & LAYOUTS_INVENTORY))
        return;

    selected = cl.frame.ps.stats[STAT_SELECTED_ITEM];

    num = 0;
    selected_num = 0;
    for (i = 0; i < MAX_ITEMS; i++) {
        if (i == selected) {
            selected_num = num;
        }
        if (cl.inventory[i]) {
            index[num++] = i;
        }
    }

    // determine scroll point
    top = selected_num - DISPLAY_ITEMS / 2;
    if (top > num - DISPLAY_ITEMS) {
        top = num - DISPLAY_ITEMS;
    }
    if (top < 0) {
        top = 0;
    }

    x = (scr.hud_width - 256) / 2;
    y = (scr.hud_height - 240) / 2;

    R_DrawPic(x, y + 8, scr.inven_pic);
    y += 24;
    x += 24;

    HUD_DrawString(x, y, "hotkey ### item");
    y += CHAR_HEIGHT;

    HUD_DrawString(x, y, "------ --- ----");
    y += CHAR_HEIGHT;

    for (i = top; i < num && i < top + DISPLAY_ITEMS; i++) {
        item = index[i];
        // search for a binding
        Q_concat(string, sizeof(string), "use ", cl.configstrings[cl.csr.items + item]);
        bind = Key_GetBinding(string);

        Q_snprintf(string, sizeof(string), "%6s %3i %s",
                   bind, cl.inventory[item], cl.configstrings[cl.csr.items + item]);

        if (item != selected) {
            HUD_DrawAltString(x, y, string);
        } else {    // draw a blinky cursor by the selected item
            HUD_DrawString(x, y, string);
            if ((cls.realtime >> 8) & 1) {
                R_DrawChar(x - CHAR_WIDTH, y, 0, 15, scr.font_pic);
            }
        }

        y += CHAR_HEIGHT;
    }
}

static void SCR_DrawSelectedItemName(int x, int y, int item)
{
    static int display_item = -1;
    static int display_start_time = 0;

    float duration = 0.f;
    if (display_item != item)
    {
        display_start_time = Sys_Milliseconds();
        display_item = item;
    }
    else
    {
        duration = (float)(Sys_Milliseconds() - display_start_time) * 0.001f;
    }

    float alpha;
    if (scr_showitemname->integer < 2)
        alpha = max(0.f, min(1.f, 5.f - 4.f * duration)); // show and hide
    else
        alpha = 1; // always show

    if (alpha > 0.f)
    {
        R_SetAlpha(alpha * scr_alpha->value);

        int index = cl.csr.items + item;
        HUD_DrawString(x, y, cl.configstrings[index]);

        R_SetAlpha(scr_alpha->value);
    }
}

static void SCR_SkipToEndif(const char **s)
{
    int i, skip = 1;
    char *token;

    while (*s) {
        token = COM_Parse(s);
        if (!strcmp(token, "xl") || !strcmp(token, "xr") || !strcmp(token, "xv") ||
            !strcmp(token, "yt") || !strcmp(token, "yb") || !strcmp(token, "yv") ||
            !strcmp(token, "pic") || !strcmp(token, "picn") || !strcmp(token, "color") ||
            strstr(token, "string")) {
            COM_Parse(s);
            continue;
        }

        if (!strcmp(token, "client")) {
            for (i = 0; i < 6; i++)
                COM_Parse(s);
            continue;
        }

        if (!strcmp(token, "ctf")) {
            for (i = 0; i < 5; i++)
                COM_Parse(s);
            continue;
        }

        if (!strcmp(token, "num") || !strcmp(token, "health_bars")) {
            COM_Parse(s);
            COM_Parse(s);
            continue;
        }

        if (!strcmp(token, "hnum")) continue;
        if (!strcmp(token, "anum")) continue;
        if (!strcmp(token, "rnum")) continue;

        if (!strcmp(token, "if")) {
            COM_Parse(s);
            skip++;
            continue;
        }

        if (!strcmp(token, "endif")) {
            if (--skip > 0)
                continue;
            return;
        }
    }
}

static void SCR_DrawHealthBar(int x, int y, int value)
{
    if (!value)
        return;

    int bar_width = scr.hud_width / 3;
    float percent = (value - 1) / 254.0f;
    int w = bar_width * percent + 0.5f;
    int h = CHAR_HEIGHT / 2;

    x -= bar_width / 2;
    R_DrawFill8(x, y, w, h, 240);
    R_DrawFill8(x + w, y, bar_width - w, h, 4);
}

static void SCR_ExecuteLayoutString(const char *s)
{
    char    buffer[MAX_QPATH];
    int     x, y;
    int     value;
    char    *token;
    int     width;
    int     index;
    clientinfo_t    *ci;

    if (!s[0])
        return;

    x = 0;
    y = 0;

    while (s) {
        token = COM_Parse(&s);
        if (token[2] == 0) {
            if (token[0] == 'x') {
                if (token[1] == 'l') {
                    token = COM_Parse(&s);
                    x = Q_atoi(token);
                    continue;
                }

                if (token[1] == 'r') {
                    token = COM_Parse(&s);
                    x = scr.hud_width + Q_atoi(token);
                    continue;
                }

                if (token[1] == 'v') {
                    token = COM_Parse(&s);
                    x = scr.hud_width / 2 - 160 + Q_atoi(token);
                    continue;
                }
            }

            if (token[0] == 'y') {
                if (token[1] == 't') {
                    token = COM_Parse(&s);
                    y = Q_atoi(token);
                    continue;
                }

                if (token[1] == 'b') {
                    token = COM_Parse(&s);
                    y = scr.hud_height + Q_atoi(token);
                    continue;
                }

                if (token[1] == 'v') {
                    token = COM_Parse(&s);
                    y = scr.hud_height / 2 - 120 + Q_atoi(token);
                    continue;
                }
            }
        }

        if (!strcmp(token, "pic")) {
            // draw a pic from a stat number
            token = COM_Parse(&s);
            value = Q_atoi(token);
            if (value < 0 || value >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            index = cl.frame.ps.stats[value];
            if (index < 0 || index >= cl.csr.max_images) {
                Com_Error(ERR_DROP, "%s: invalid pic index", __func__);
            }
            token = cl.configstrings[cl.csr.images + index];
            if (token[0]) {
                qhandle_t pic = cl.image_precache[index];
                if (pic != 0) {
                    // hack for action mod scope scaling
                    if (x == scr.hud_width  / 2 - 160 &&
                        y == scr.hud_height / 2 - 120 &&
                        Com_WildCmp("scope?x", token))
                    {
                        int w = 320 * ch_scale->value;
                        int h = 240 * ch_scale->value;
                        R_DrawStretchPic((scr.hud_width  - w) / 2 + ch_x->integer,
                                        (scr.hud_height - h) / 2 + ch_y->integer,
                                        w, h, pic);
                    } else {
                        R_DrawPic(x, y, pic);
                    }
                }
            }

            if (value == STAT_SELECTED_ICON && scr_showitemname->integer)
            {
                SCR_DrawSelectedItemName(x + 32, y + 8, cl.frame.ps.stats[STAT_SELECTED_ITEM]);
            }
            continue;
        }

        if (!strcmp(token, "client")) {
            // draw a deathmatch client block
            int     score, ping, time;

            token = COM_Parse(&s);
            x = scr.hud_width / 2 - 160 + Q_atoi(token);
            token = COM_Parse(&s);
            y = scr.hud_height / 2 - 120 + Q_atoi(token);

            token = COM_Parse(&s);
            value = Q_atoi(token);
            if (value < 0 || value >= MAX_CLIENTS) {
                Com_Error(ERR_DROP, "%s: invalid client index", __func__);
            }
            ci = &cl.clientinfo[value];

            token = COM_Parse(&s);
            score = Q_atoi(token);

            token = COM_Parse(&s);
            ping = Q_atoi(token);

            token = COM_Parse(&s);
            time = Q_atoi(token);

            HUD_DrawAltString(x + 32, y, ci->name);
            HUD_DrawString(x + 32, y + CHAR_HEIGHT, "Score: ");
            Q_snprintf(buffer, sizeof(buffer), "%i", score);
            HUD_DrawAltString(x + 32 + 7 * CHAR_WIDTH, y + CHAR_HEIGHT, buffer);
            Q_snprintf(buffer, sizeof(buffer), "Ping:  %i", ping);
            HUD_DrawString(x + 32, y + 2 * CHAR_HEIGHT, buffer);
            Q_snprintf(buffer, sizeof(buffer), "Time:  %i", time);
            HUD_DrawString(x + 32, y + 3 * CHAR_HEIGHT, buffer);

            if (!ci->icon) {
                ci = &cl.baseclientinfo;
            }
            R_DrawPic(x, y, ci->icon);
            continue;
        }

        if (!strcmp(token, "ctf")) {
            // draw a ctf client block
            int     score, ping;

            token = COM_Parse(&s);
            x = scr.hud_width / 2 - 160 + Q_atoi(token);
            token = COM_Parse(&s);
            y = scr.hud_height / 2 - 120 + Q_atoi(token);

            token = COM_Parse(&s);
            value = Q_atoi(token);
            if (value < 0 || value >= MAX_CLIENTS) {
                Com_Error(ERR_DROP, "%s: invalid client index", __func__);
            }
            ci = &cl.clientinfo[value];

            token = COM_Parse(&s);
            score = Q_atoi(token);

            token = COM_Parse(&s);
            ping = Q_atoi(token);
            if (ping > 999)
                ping = 999;

            Q_snprintf(buffer, sizeof(buffer), "%3d %3d %-12.12s",
                       score, ping, ci->name);
            if (value == cl.frame.clientNum) {
                HUD_DrawAltString(x, y, buffer);
            } else {
                HUD_DrawString(x, y, buffer);
            }
            continue;
        }

        if (!strcmp(token, "picn")) {
            // draw a pic from a name
            token = COM_Parse(&s);
            R_DrawPic(x, y, R_RegisterPic2(token));
            continue;
        }

        if (!strcmp(token, "num")) {
            // draw a number
            token = COM_Parse(&s);
            width = Q_atoi(token);
            token = COM_Parse(&s);
            value = Q_atoi(token);
            if (value < 0 || value >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            value = cl.frame.ps.stats[value];
            HUD_DrawNumber(x, y, 0, width, value);
            continue;
        }

        if (!strcmp(token, "hnum")) {
            // health number
            int     color;

            width = 3;
            value = cl.frame.ps.stats[STAT_HEALTH];
            if (value > 25)
                color = 0;  // green
            else if (value > 0)
                color = ((cl.frame.number / CL_FRAMEDIV) >> 2) & 1;     // flash
            else
                color = 1;

            if (cl.frame.ps.stats[STAT_FLASHES] & 1)
                R_DrawPic(x, y, scr.field_pic);

            HUD_DrawNumber(x, y, color, width, value);
            continue;
        }

        if (!strcmp(token, "anum")) {
            // ammo number
            int     color;

            width = 3;
            value = cl.frame.ps.stats[STAT_AMMO];
            if (value > 5)
                color = 0;  // green
            else if (value >= 0)
                color = ((cl.frame.number / CL_FRAMEDIV) >> 2) & 1;     // flash
            else
                continue;   // negative number = don't show

            if (cl.frame.ps.stats[STAT_FLASHES] & 4)
                R_DrawPic(x, y, scr.field_pic);

            HUD_DrawNumber(x, y, color, width, value);
            continue;
        }

        if (!strcmp(token, "rnum")) {
            // armor number
            int     color;

            width = 3;
            value = cl.frame.ps.stats[STAT_ARMOR];
            if (value < 1)
                continue;

            color = 0;  // green

            if (cl.frame.ps.stats[STAT_FLASHES] & 2)
                R_DrawPic(x, y, scr.field_pic);

            HUD_DrawNumber(x, y, color, width, value);
            continue;
        }

        if (!strncmp(token, "stat_", 5)) {
            char *cmd = token + 5;
            token = COM_Parse(&s);
            index = Q_atoi(token);
            if (index < 0 || index >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            index = cl.frame.ps.stats[index];
            if (index < 0 || index >= cl.csr.end) {
                Com_Error(ERR_DROP, "%s: invalid string index", __func__);
            }
            token = cl.configstrings[index];
            if (!strcmp(cmd, "string"))
                HUD_DrawString(x, y, token);
            else if (!strcmp(cmd, "string2"))
                HUD_DrawAltString(x, y, token);
            else if (!strcmp(cmd, "cstring"))
                HUD_DrawCenterString(x + 320 / 2, y, token);
            else if (!strcmp(cmd, "cstring2"))
                HUD_DrawAltCenterString(x + 320 / 2, y, token);
            else if (!strcmp(cmd, "rstring"))
                HUD_DrawRightString(x, y, token);
            else if (!strcmp(cmd, "rstring2"))
                HUD_DrawAltRightString(x, y, token);
            continue;
        }

        if (!strcmp(token, "cstring")) {
            token = COM_Parse(&s);
            HUD_DrawCenterString(x + 320 / 2, y, token);
            continue;
        }

        if (!strcmp(token, "cstring2")) {
            token = COM_Parse(&s);
            HUD_DrawAltCenterString(x + 320 / 2, y, token);
            continue;
        }

        if (!strcmp(token, "string")) {
            token = COM_Parse(&s);
            HUD_DrawString(x, y, token);
            continue;
        }

        if (!strcmp(token, "string2")) {
            token = COM_Parse(&s);
            HUD_DrawAltString(x, y, token);
            continue;
        }

        if (!strcmp(token, "rstring")) {
            token = COM_Parse(&s);
            HUD_DrawRightString(x, y, token);
            continue;
        }

        if (!strcmp(token, "rstring2")) {
            token = COM_Parse(&s);
            HUD_DrawAltRightString(x, y, token);
            continue;
        }

        if (!strcmp(token, "if")) {
            token = COM_Parse(&s);
            value = Q_atoi(token);
            if (value < 0 || value >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            value = cl.frame.ps.stats[value];
            if (!value) {   // skip to endif
                if (cl.csr.extended) {
                    SCR_SkipToEndif(&s);
                } else while (strcmp(token, "endif")) {
                    token = COM_Parse(&s);
                    if (!s) {
                        break;
                    }
                }
            }
            continue;
        }

        // Q2PRO extension
        if (!strcmp(token, "color")) {
            color_t     color;

            token = COM_Parse(&s);
            if (SCR_ParseColor(token, &color)) {
                color.u8[3] *= scr_alpha->value;
                R_SetColor(color.u32);
            }
            continue;
        }

        if (!strcmp(token, "health_bars")) {
            token = COM_Parse(&s);
            value = Q_atoi(token);
            if (value < 0 || value >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            value = cl.frame.ps.stats[value];

            token = COM_Parse(&s);
            index = Q_atoi(token);
            if (index < 0 || index >= cl.csr.end) {
                Com_Error(ERR_DROP, "%s: invalid string index", __func__);
            }

            HUD_DrawCenterString(x + 320 / 2, y, cl.configstrings[index]);
            SCR_DrawHealthBar(x + 320 / 2, y + CHAR_HEIGHT + 4, value & 0xff);
            SCR_DrawHealthBar(x + 320 / 2, y + CHAR_HEIGHT + 12, (value >> 8) & 0xff);
            continue;
        }
    }

    R_ClearColor();
    R_SetAlpha(scr_alpha->value);
}

//=============================================================================

static void SCR_DrawPause(void)
{
    int x, y;

    if (!sv_paused->integer)
        return;
    if (!cl_paused->integer)
        return;
    if (scr_showpause->integer != 1)
        return;

    x = (scr.hud_width - scr.pause_width) / 2;
    y = (scr.hud_height - scr.pause_height) / 2;

    R_DrawPic(x, y, scr.pause_pic);
}

static void SCR_DrawLoading(void)
{
    int x, y;

    if (!scr.draw_loading)
        return;

    scr.draw_loading = false;

    R_SetScale(scr.hud_scale);

    x = (r_config.width * scr.hud_scale - scr.loading_width) / 2;
    y = (r_config.height * scr.hud_scale - scr.loading_height) / 2;

    R_DrawPic(x, y, scr.loading_pic);

    R_SetScale(1.0f);
}

static void SCR_DrawCrosshair(void)
{
    int x, y;

    if (!scr_crosshair->integer)
        return;
    if (cl.frame.ps.stats[STAT_LAYOUTS] & (LAYOUTS_HIDE_HUD | LAYOUTS_HIDE_CROSSHAIR))
        return;

    x = (scr.hud_width - scr.crosshair_width) / 2;
    y = (scr.hud_height - scr.crosshair_height) / 2;

    R_SetColor(scr.crosshair_color.u32);

    R_DrawStretchPic(x + ch_x->integer,
                     y + ch_y->integer,
                     scr.crosshair_width,
                     scr.crosshair_height,
                     scr.crosshair_pic);
}

// The status bar is a small layout program that is based on the stats array
static void SCR_DrawStats(void)
{
    if (scr_draw2d->integer <= 1)
        return;
    if (cl.frame.ps.stats[STAT_LAYOUTS] & LAYOUTS_HIDE_HUD)
        return;

    SCR_ExecuteLayoutString(cl.configstrings[CS_STATUSBAR]);
}

static void SCR_DrawLayout(void)
{
    if (scr_draw2d->integer == 3 && !Key_IsDown(K_F1))
        return;     // turn off for GTV

    if (cls.demo.playback && Key_IsDown(K_F1))
        goto draw;

    if (!(cl.frame.ps.stats[STAT_LAYOUTS] & LAYOUTS_LAYOUT))
        return;

draw:
    SCR_ExecuteLayoutString(cl.layout);
}

static void SCR_Draw2D(void)
{
	if (scr_draw2d->integer <= 0)
		return;     // turn off for screenshots

	if (cls.key_dest & KEY_MENU)
		return;

	R_SetAlphaScale(scr.hud_alpha);

    R_SetScale(scr.hud_scale);

    scr.hud_height = Q_rint(scr.hud_height * scr.hud_scale);
    scr.hud_width = Q_rint(scr.hud_width * scr.hud_scale);

    // crosshair has its own color and alpha
#ifdef REFERENCE_B_B2
    if (!cl.refdef.b2_active)
#endif
        SCR_DrawCrosshair();

    // the rest of 2D elements share common alpha
    R_ClearColor();
    R_SetAlpha(Cvar_ClampValue(scr_alpha, 0, 1));

#ifdef REFERENCE_B_B2
    R_B2UIRole(B2UI_STATUS);
#endif
    SCR_DrawStats();
#ifdef REFERENCE_B_B2
    R_B2UIRole(B2UI_HELP);
#endif
    SCR_DrawLayout();

    SCR_DrawInventory();
#ifdef REFERENCE_B_B2
    R_B2UIRole(B2UI_FLAT);
#endif

    SCR_DrawCenterString();

    SCR_DrawNet();

    SCR_DrawObjects();

	SCR_DrawFPS();

    SCR_DrawChatHUD();

    SCR_DrawTurtle();

    SCR_DrawPause();

    // debug stats have no alpha
    R_ClearColor();

#if USE_DEBUG
    SCR_DrawDebugStats();
    SCR_DrawDebugPmove();
#endif

    R_SetScale(1.0f);
	R_SetAlphaScale(1.0f);
}

#ifdef REFERENCE_B_B2
static const char *b2w_icons[12]={"w_blaster","w_shotgun","w_sshotgun","w_machinegun",
    "w_chaingun","a_grenades","w_glauncher","w_rlauncher","w_hyperblaster","w_railgun","w_bfg","w_flareg"};
static const char *b2w_labels[12]={"Blaster","Shotgun","Super Shotgun","Machinegun",
    "Chaingun","Grenades","Grenade Launcher","Rocket Launcher","HyperBlaster","Railgun","BFG10K","Flare Gun"};
static qhandle_t b2w_pics[12],b2wi_pics[B2WI_COUNT],b2w_arrow_pics[2];
bool SCR_B2PrepareUI(void)
{
    Con_RegisterMedia();
    scr.pause_pic=R_RegisterPic("pause");
    R_GetPicSize(&scr.pause_width,&scr.pause_height,scr.pause_pic);
    const char *sounds[]={"misc/menu1.wav","misc/menu2.wav","misc/menu3.wav","misc/talk1.wav"};
    for(unsigned i=0;i<q_countof(sounds);i++) if(!S_RegisterSound(sounds[i])) return false;
    for(unsigned i=0;i<12;i++) b2w_pics[i]=R_RegisterPic(b2w_icons[i]);
    for(unsigned i=0;i<B2WI_COUNT;i++) b2wi_pics[i]=R_RegisterPic(b2wi_icons[i]);
    const char *arrows[]={"b2/arrow-stone-metal-raised-original.png","b2/arrow-stone-metal-depressed-original.png"};
    for (unsigned i=0;i<2;i++) {
        int w=0,h=0;b2w_arrow_pics[i]=R_RegisterPic(arrows[i]);
        R_GetPicSize(&w,&h,b2w_arrow_pics[i]);
        if (!b2w_arrow_pics[i] || w!=1254 || h!=1254) return false;
    }
    return UI_B2Prepare() && scr.pause_pic;
}

void SCR_DrawB2Aim(float x, float y)
{
    if (B2W_HideBodyReticle()) return;
    if (!Cvar_Get("b2_hud","1",CVAR_NOARCHIVE)->integer || !scr_crosshair->integer ||
        (cl.frame.ps.stats[STAT_LAYOUTS] & (LAYOUTS_HIDE_HUD|LAYOUTS_HIDE_CROSSHAIR))) return;
    R_SetScale(1.f);
    R_SetColor(scr.crosshair_color.u32);
    float w=scr.crosshair_width*2.f, h=scr.crosshair_height*2.f;
    R_DrawStretchPic(x*r_config.width-w*.5f+ch_x->integer,
        y*r_config.height-h*.5f+ch_y->integer,w,h,scr.crosshair_pic);
    R_ClearColor();
}

// Keep the existing font size; wrap a cell label/state without truncation.
// Prefer word boundaries, with a balanced split for a single long item name.
static void SCR_B2SelectorText(int x, int y, int width, const char *text)
{
    size_t columns=width>CHAR_WIDTH?(size_t)(width/CHAR_WIDTH):1;
    while (*text) {
        size_t length=strlen(text), take=length;
        if (take>columns) {
            take=columns;
            while (take && text[take]!=' ') take--;
            if (!take) take=length<=columns*2?(length+1)/2:columns;
        }
        SCR_DrawStringEx(x,y,UI_CENTER,take,text,scr.font_pic);
        text+=take;while (*text==' ') text++;
        y+=CHAR_HEIGHT;
    }
}

static void SCR_DrawB2Selector(void)
{
    b2w_hud_t hud;
    if (!B2W_HUDState(&hud,Sys_Milliseconds())) return;
    // Same logical plane/ray coordinates as B2W_HUDRay. This draw list is
    // captured once at eye zero and replayed unchanged for stereo/Quad Views.
    R_ClearColor();R_SetAlphaScale(1.f);R_SetScale(.5f);
    const float width=r_config.width*.5f,height=r_config.height*.5f;
    R_B2SelectorBegin(hud.open?&hud.plane:NULL);
    if (hud.open) {
        // Only grids move; glyph and icon scales remain fixed.
        float progress=B2W_PageProgress(hud.slide_ms,Sys_Milliseconds());
        unsigned target_page=hud.page;
        if (hud.sliding) {
            float cell[4];B2W_GridCell(0,height/width,cell);
            R_B2PageClip(true,(int)((cell[0]+1)*.5f*width),(int)((cell[1]+1)*.5f*height),
                (int)(4*cell[2]*.5f*width),(int)(3*cell[3]*.5f*height));
            hud.candidate=hud.stable=-1;
        }
        for (unsigned pass=0;pass<(hud.sliding?2u:1u);pass++) {
            hud.page=hud.sliding?(pass?target_page:hud.slide_from):target_page;
            float direction=target_page?1.f:-1.f;
            float offset=hud.sliding?direction*(pass?1.f-progress:-progress)*.9072f:0;
        for (int slot=0;slot<(hud.page?B2WI_COUNT+1:12);slot++) {
            bool give_all=hud.page && slot==B2WI_COUNT;
            if (hud.page && !((hud.items.owned|hud.items.active)&(1u<<slot))) continue;
            float rect[4];
            if (!B2W_GridCell(slot,height/width,rect)) continue;
            int x=(int)((rect[0]+offset+1.f)*.5f*width),y=(int)((rect[1]+1.f)*.5f*height);
            int w=(int)(rect[2]*.5f*width),h=(int)(rect[3]*.5f*height);
            bool owned=((hud.page?hud.items.owned:hud.inventory.owned)&(1u<<slot))!=0;
            bool available=((hud.page?hud.items.available:hud.inventory.available)&(1u<<slot))!=0;
            qhandle_t pic=give_all?0:hud.page?b2wi_pics[slot]:b2w_pics[slot];
            int colour=slot==hud.stable?112:slot==hud.candidate?104:(!hud.page && slot==hud.inventory.current)?24:8;
            if (!hud.page && colour==8) {
                color_t grey={.u32=d_8to24table[8]};
                for (unsigned channel=0;channel<3;channel++) grey.u8[channel]=(grey.u8[channel]*4)/5;
                R_DrawFill32(x+2,y+2,w-4,h-4,grey.u32);
            } else R_DrawFill8(x+2,y+2,w-4,h-4,colour);
            R_SetAlpha(owned?(available?1.f:.45f):.2f);
            if ((owned || (hud.page && (hud.items.active&(1u<<slot)))) && pic) {
                int pic_width,pic_height;R_GetPicSize(&pic_width,&pic_height,pic);
                if (pic_width>0 && pic_height>0) {
                    // Retain the accepted icon size while reducing cell spacing.
                    int icon_width=(int)((int)(.4f*.5f*width)*.28f);
                    int icon_height=(int)((float)icon_width*pic_height/pic_width);
                    R_DrawStretchPic(x+(w-icon_width)/2,y+(int)(h*.06f),icon_width,icon_height,pic);
                }
            }
            SCR_B2SelectorText(x+w/2,y+(int)(h*.56f),w-4,give_all?"Give All":hud.page?b2wi_names[slot]:b2w_labels[slot]);
            const char *state=!owned?"Not owned":!available?"No ammo":slot==hud.inventory.pending?"Pending":slot==hud.inventory.current?"Current":slot==hud.stable?"Release to select":"";
            char item_state[64];
            const char *item_status=NULL;
            if (give_all) state=slot==hud.stable?"Release to Give All":"Opens paused console";
            else if (hud.page) {
                Q_snprintf(item_state,sizeof(item_state),"x%d",hud.items.count[slot]);
                item_status=(hud.items.active&(1u<<slot))?"Active":!available?(owned?"No cells":"In use"):slot==hud.stable?"Release to use":"";
                state=item_state;
            }
            // Keep item status one font line below its quantity at every scale.
            int text_drop=(!give_all && (hud.page || !strcmp(state,"Current") || !strcmp(state,"Release to select")))?4:0;
            int state_y=y+(int)(h*.72f)+text_drop;
            SCR_B2SelectorText(x+w/2,state_y,w-4,state);
            if (item_status) SCR_B2SelectorText(x+w/2,state_y+CHAR_HEIGHT,w-4,item_status);
            R_ClearColor();
        }
        if (hud.page && !(hud.items.owned|hud.items.active))
            SCR_DrawString((int)(width*.5f*(1+offset)),height/2,UI_CENTER,"No usable items");
        }
        hud.page=target_page;
        if (hud.sliding) R_B2PageClip(false,0,0,0,0);
        unsigned arrow_page=hud.sliding?hud.slide_from:hud.page;
        float arrow[4];
        if (B2W_PageArrow(arrow_page,height/width,arrow)) {
            int x=(int)((arrow[0]+1)*.5f*width),y=(int)((arrow[1]+1)*.5f*height);
            int w=(int)(arrow[2]*.5f*width),h=(int)(arrow[3]*.5f*height);
            R_B2PageArrow(x,y,w,h,arrow_page!=0,b2w_arrow_pics[hud.arrow_pressed?1:0]);
            SCR_DrawString(x+w/2,y+h+8,UI_CENTER,arrow_page?"Weapons":"Items");
        }
        if (hud.cursor && !hud.sliding) {
            int x=(int)((hud.x+1)*.5f*width),y=(int)((hud.y+1)*.5f*height);
            R_DrawFill8(x-4,y-1,9,3,15);R_DrawFill8(x-1,y-4,3,9,15);
        }
    }
    const char *status=hud.status==B2W_UI_WAITING?"Selecting...":hud.status==B2W_UI_ACCEPTED?"Selection accepted":
        hud.status==B2W_UI_REFUSED?"Selection unavailable":hud.status==B2W_UI_ITEM_USED?"Item used":
        hud.status==B2W_UI_ITEM_REFUSED?"Item unavailable":hud.rearm?"Release controls to continue":hud.open?(hud.page?"Release on item; hover arrow for weapons":"Release on weapon; hover arrow for items"):"";
    SCR_DrawString(width/2,(int)(height*.94f),UI_CENTER,status);
    R_B2SelectorEnd();
    R_ClearColor();R_SetScale(1.f);R_SetAlphaScale(1.f);
}

extern bool B2_HeadsetWaiting(void);
#include "../../../runtime-recovery555/source/b2_runtime555.h"
 #include "../../../../baseline/b2-menu-links284/source/b2_skip_ring284.inc"
void SCR_DrawB2CinematicSkip(int width, int height)
{
    float progress=B2_CinematicSkipProgress();
    if (cls.key_dest!=KEY_GAME || width<96 || height<96 || progress<=0) return;
    // Coordinates in film pixels; the renderer maps this role into the same
    // accepted film rectangle/curved anchor without changing the film itself.
    int radius=max(22,width/24),thickness=max(2,radius/8);
    // R_SetScale divides drawing coordinates. Scale every primitive, including
    // the font, to 56.25% of the original cue about the accepted film-pixel centre.
    // This is another 25% reduction; centre rounding is less than .28125px.
    const float scale=16.f/9.f;
    int x=(int)((width-radius-12)*scale+.5f);
    int y=(int)((height-radius-12)*scale+.5f);
    R_B2UIRole(B2UI_CINEMATIC_OVERLAY);R_SetScale(scale);R_ClearColor();
    SCR_B2SkipRing(x,y,radius,thickness,progress,scale);
    SCR_DrawString(x,y-CHAR_HEIGHT/2,UI_CENTER,"skip");
    R_SetScale(1.f);R_B2UIRole(B2UI_FLAT);
}

void SCR_DrawB2Realignment391(void)
{
    char control[192];B2_RealignPrompt391(control,sizeof(control));
    R_B2SelectorBegin(NULL);R_B2UIBegin();
    R_B2UIRole(B2UI_MENU);R_SetScale(.5f);R_ClearColor();R_SetAlphaScale(1.f);
    int x=r_config.width/4,y=r_config.height/4;
    SCR_DrawString(x,y-48,UI_CENTER,"Tracking view reset - game paused");
    SCR_DrawString(x,y-24,UI_CENTER,"Sit or stand in your normal playing posture");
    SCR_DrawString(x,y-8,UI_CENTER,"Face forward comfortably; your game position is kept");
    SCR_DrawString(x,y+16,UI_CENTER,control);
    SCR_DrawString(x,y+40,UI_CENTER,"Then release controls and confirm again to resume");
    SCR_DrawString(x,y+64,UI_CENTER,"If crouched: crouch then release to stand after resuming");
    SCR_DrawString(x,y+88,UI_CENTER,"Close the game window to quit");
    R_ClearColor();R_SetScale(1.f);R_SetAlphaScale(1.f);R_B2UIRole(B2UI_FLAT);R_B2UIEnd();
}

// Completion feedback, not command-dispatch acknowledgement. It remains
// visible in the ordinary VR menu after a save writer reports ERR_DROP.
static struct { bool pending,automatic,visible; int outcome; uint32_t completed; } b2_save_notice556;
void B2_SaveBegin556(bool automatic)
{
    b2_save_notice556.pending=Cvar_VariableInteger("b2_openxr")==1;
    b2_save_notice556.automatic=automatic;
    b2_save_notice556.visible=false;
}
void B2_SaveOutcome556(int outcome)
{
    if(!b2_save_notice556.pending)return;
    b2_save_notice556.pending=false;
    if(outcome<0)return; // A transition archive completed, but no slot was published.
    b2_save_notice556.outcome=outcome;b2_save_notice556.completed=Sys_Milliseconds();
    b2_save_notice556.visible=true;
}
void B2_SaveFailed556(void)
{
    B2_SaveOutcome556(1);
}
static const char *B2_SaveText556(uint32_t now)
{
    if(!b2_save_notice556.visible || (uint32_t)(now-b2_save_notice556.completed)>=
       (b2_save_notice556.outcome?5000u:2500u))return NULL;
    if(b2_save_notice556.outcome==2)return "Save failed. Backup retained; see console for recovery.";
    if(b2_save_notice556.outcome)return b2_save_notice556.automatic?
        "Autosave failed. See console for details.":"Save failed. See console for details.";
    return b2_save_notice556.automatic?"Autosave completed.":"Game saved.";
}
static void SCR_DrawSaveOutcome556(void)
{
    const char *text=B2_SaveText556(Sys_Milliseconds());if(!text)return;
    R_B2UIRole(B2UI_MENU);R_SetScale(.5f);R_ClearColor();R_SetAlphaScale(1.f);
    SCR_DrawString(r_config.width/4,r_config.height/2-28,UI_CENTER,text);
    R_ClearColor();R_SetScale(1.f);R_B2UIRole(B2UI_FLAT);
}

void SCR_DrawB2Surface(void)
{
    R_B2SelectorBegin(NULL);R_B2UIBegin();
    R_ClearColor();R_SetScale(1.f);R_SetAlphaScale(1.f);
    if (cls.state==ca_cinematic) SCR_DrawCinematic();
    R_B2UIRole(B2UI_MENU);UI_Draw(cls.realtime);
    SCR_DrawSaveOutcome556();
    R_B2UIRole(B2UI_FLAT);Con_DrawConsole();
    R_ClearColor();R_SetScale(1.f);R_SetAlphaScale(1.f);R_B2UIEnd();
}

// Existing captured menu plane; replayed for each tracked headset view.
#include "../../../../baseline/b2-ui-refine267/source/b2_resume_draw267.inc"

void SCR_DrawB2HUD(void)
{
    R_B2SelectorBegin(NULL);
    cvar_t *hud=Cvar_Get("b2_hud", "1", CVAR_NOARCHIVE);
    if (!hud->integer && !B2_UIEnabled() && !B2W_HideBodyReticle()) return;
    R_B2UIBegin();
    scr.hud_height=r_config.height;
    scr.hud_width=r_config.width;
    float scale=scr.hud_scale; scr.hud_scale=.25f;
    if(hud->integer) SCR_Draw2D(); // Captured once; renderer replays identical draw data.
    scr.hud_scale=scale;
    SCR_DrawB2Selector();
    if (B2_HeadsetWaiting() && cls.key_dest==KEY_GAME) {
        R_B2UIRole(B2UI_MENU); R_SetScale(.5f); R_ClearColor(); R_SetAlphaScale(1.f);
        SCR_DrawString(r_config.width/4,r_config.height/4-16,UI_CENTER,"Preparing play");
        SCR_DrawString(r_config.width/4,r_config.height/4,UI_CENTER,
            "Release controls to continue");
        bool limited=false;
        for (unsigned hand=0;hand<2;hand++) {
            const char *notice=B2XR_ControllerNotice582(hand);
            if (notice) {
                SCR_DrawString(r_config.width/4,r_config.height/4+24+16*hand,UI_CENTER,notice);
                limited=true;
            }
        }
        if (limited) SCR_DrawString(r_config.width/4,r_config.height/4+64,UI_CENTER,
            "Keyboard and mouse remain available");
    }
    if(B2_UIEnabled()) {
        R_B2UIRole(B2UI_MENU);
        UI_Draw(cls.realtime);
        R_B2UIRole(B2UI_FLAT);
        Con_DrawConsole();
        R_ClearColor(); R_SetScale(1.f); R_SetAlphaScale(1.f);
    }
    SCR_DrawB2ResumeCue();
    SCR_DrawSaveOutcome556();
    R_B2UIEnd();
}
#endif

static void SCR_DrawActive(void)
{
    // if full screen menu is up, do nothing at all
    if (!UI_IsTransparent())
        return;

    // draw black background if not active
    if (cls.state < ca_active) {
        R_DrawFill8(0, 0, r_config.width, r_config.height, 0);
        return;
    }

    if (cls.state == ca_cinematic) {
        SCR_DrawCinematic();
        return;
    }

    // start with full screen HUD
    scr.hud_height = r_config.height;
    scr.hud_width = r_config.width;

    SCR_DrawDemo();

    SCR_CalcVrect();

    // clear any dirty part of the background
    SCR_TileClear();

    // draw 3D game view
    V_RenderView();

    // draw all 2D elements
    SCR_Draw2D();
}

//=======================================================

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void SCR_UpdateScreen(void)
{
    static int recursive;

    if (!scr.initialized) {
        return;             // not initialized yet
    }

    // if the screen is disabled (loading plaque is up), do nothing at all
    if (cls.disable_screen) {
        unsigned delta = Sys_Milliseconds() - cls.disable_screen;

        if (delta < 120 * 1000) {
            return;
        }

        cls.disable_screen = 0;
        Com_Printf("Loading plaque timed out.\n");
    }

    if (recursive > 1) {
        Com_Error(ERR_FATAL, "%s: recursively called", __func__);
    }

    recursive++;

#ifdef REFERENCE_B_B2
    if (B2_RenderStereo()) {
        recursive--;
        return;
    }
#endif


    R_BeginFrame();

    // do 3D refresh drawing
    SCR_DrawActive();

    // draw main menu
    UI_Draw(cls.realtime);

    // draw console
    Con_DrawConsole();

    // draw loading plaque
    SCR_DrawLoading();

    R_EndFrame();


    recursive--;
}

qhandle_t SCR_GetFont(void)
{
	return scr.font_pic;
}

void SCR_SetHudAlpha(float alpha)
{
	scr.hud_alpha = alpha;
}
