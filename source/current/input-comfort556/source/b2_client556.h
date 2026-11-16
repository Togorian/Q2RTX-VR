// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// Modified 12 November 2026: input-mode focus and actual save outcome services.
#ifndef B2_CLIENT556_H
#define B2_CLIENT556_H
#include <stdbool.h>
bool B2_ControllerFocusMode556(void);
bool B2_InputFocused556(void);
void B2_SaveBegin556(bool automatic);
void B2_SaveOutcome556(int outcome);
void B2_SaveFailed556(void);
#endif
