// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian; implementation with Codex assistance.
// Modified 12 November 2026: bounded runtime failure and lifecycle interfaces.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
static inline const char *B2_InternalStopMessage642(void)
{
    return "Q2RTX VR stopped itself because it could not safely continue. Close the game window, restart Q2RTX VR and load a saved game. Existing saves are retained; unsaved progress is not recovered.";
}
// Internal invariant failures keep their technical detail in the log and in
// the fatal dialog, with a plain recovery instruction above that detail.
// A boundary failure may include a driver result; its exact detail is retained.
// Ordinary runtime/driver messages keep their existing explanation.
static inline bool B2_InternalFatal642(const char *message)
{
    return message && strncmp(message,B2_InternalStopMessage642(),strlen(B2_InternalStopMessage642())) &&
        (strstr(message,"B2_FAIL:") || strstr(message,"VR_FAIL:") ||
        !strncmp(message,"B2 ",3) || !strncmp(message,"B2_",3) ||
        !strncmp(message,"VR settings rejection",21) ||
        !strncmp(message,"VR rejected",11) ||
        !strncmp(message,"instanced-position ",19) ||
        !strcmp(message,"private peripheral guide: invalid descriptor slot") ||
        strstr(message,"could not safely continue") ||
        strstr(message,"could not safely retire an unfinished frame") ||
        !strcmp(message,"VR callback setting was not registered") ||
        !strcmp(message,"VR graphics could not start: invalid initial frame. See startup-status.txt."));
}
uint32_t B2XR_ActionGeneration594(void);
const char *B2XR_FailureMessage555(void);
bool B2XR_AbortFrame555(void);
void B2XR_NotifyStopped555(void);
bool R_B2DropRetire555(unsigned *next_epoch);
void B2_DropToMenu555(void);
bool B2_DropMenu555(void);
static inline const char *B2XR_StartupMessage555(const char *fallback)
{
    const char *reason=B2XR_FailureMessage555();
    return reason ? reason : fallback;
}

bool B2XR_Focused582(void);

const char *B2XR_ControllerNotice582(unsigned hand);
