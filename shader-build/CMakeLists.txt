# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# Project shader-only build glue; rights held by Togorian offered GPLv2-or-later.
cmake_minimum_required(VERSION 3.25)
project(Q2RTXVRShaderReview LANGUAGES NONE)
set(Q2RTX_REBUILD_SHADERS ON CACHE BOOL "Expose shader review targets" FORCE)
include("${CMAKE_CURRENT_LIST_DIR}/../recipes/shaders.cmake")
