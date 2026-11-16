// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
// Included after the selected g_local.h; no shared structure or save ABI change.
void trigger_push_touch(edict_t *self,edict_t *other,cplane_t *plane,csurface_t *surf);
bool G_CombatTriggerPush452(edict_t *ent,edict_t *source);
