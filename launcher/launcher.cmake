# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# Design/direction by Togorian; implementation with Codex assistance.
# Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
# Modified 11 November 2026: provenance/notice clarification only. See PROVENANCE.md.
add_executable(Q2RTXVRLauncher WIN32 launcher/launcher.c launcher/launcher.rc)
target_compile_options(Q2RTXVRLauncher PRIVATE "$<$<COMPILE_LANGUAGE:C>:/W4;/O2;/MT;/std:c11>")
target_link_libraries(Q2RTXVRLauncher PRIVATE user32.lib kernel32.lib)
target_link_options(Q2RTXVRLauncher PRIVATE /INCREMENTAL:NO /MANIFEST:EMBED)
set_target_properties(Q2RTXVRLauncher PROPERTIES OUTPUT_NAME "Q2RTX VR"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_include_directories(Q2RTXVRLauncher PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source/baseline/b2-consolidated215/resources/src/windows/res")
