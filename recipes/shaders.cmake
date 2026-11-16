# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# Design/direction by Togorian; implementation with Codex assistance.
# Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
# Modified 11 November 2026: provenance/notice clarification only. See PROVENANCE.md.
if(NOT DEFINED Q2RTX_SHADER_SOURCE_ROOT)
  get_filename_component(Q2RTX_SHADER_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()
option(Q2RTX_REBUILD_SHADERS "Expose shader rebuild targets using glslangValidator" OFF)
if(NOT Q2RTX_REBUILD_SHADERS)
  return()
endif()
find_program(GLSLANG_VALIDATOR glslangValidator HINTS "$ENV{VULKAN_SDK}/Bin" REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(shader_outputs)
file(GLOB_RECURSE q2rtx_shader_inputs CONFIGURE_DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/*")
list(APPEND q2rtx_shader_inputs "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/inputs/origin-003/include/refresh/b2_wall_return.h")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/animate_materials.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/animate_materials.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/animate_materials.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/animate_materials.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/animate_materials.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/asvgf_atrous.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/asvgf_atrous.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/asvgf_atrous.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/asvgf_atrous.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/asvgf_atrous.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_atrous.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_atrous.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_gradient_atrous.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_gradient_atrous.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_atrous.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_img.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_img.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_gradient_img.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_gradient_img.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_img.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_reproject.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/eye-rank/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_reproject.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/eye-rank/vkpt/shader/asvgf_gradient_reproject.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/eye-rank/vkpt/shader/asvgf_gradient_reproject.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/asvgf_gradient_reproject.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/asvgf_lf.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/asvgf_lf.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_lf.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_lf.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/asvgf_lf.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/asvgf_taau.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/asvgf_taau.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_taau.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_taau.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/asvgf_taau.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/asvgf_temporal.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/asvgf_temporal.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_temporal.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/asvgf_temporal.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/asvgf_temporal.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/bloom_blur.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/bloom_blur.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_blur.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_blur.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/bloom_blur.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/bloom_composite.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/bloom_composite.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_composite.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_composite.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/bloom_composite.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/bloom_downscale.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/bloom_downscale.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_downscale.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_downscale.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/bloom_downscale.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/bloom_margin.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/bloom_margin.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_margin.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_margin.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/bloom_margin.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/checkerboard_interleave.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/checkerboard_interleave.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/checkerboard_interleave.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/checkerboard_interleave.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/checkerboard_interleave.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/compositing.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/compositing.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/compositing.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/compositing.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/compositing.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/debug_line.frag.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/debug_line.frag.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/debug_line.frag"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/debug_line.frag" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/debug_line.frag.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/debug_line.vert.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/debug_line.vert.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/debug_line.vert"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/debug_line.vert" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/debug_line.vert.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/direct_lighting.rgen.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/direct_lighting.rgen.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/direct_lighting.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/direct_lighting.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/direct_lighting.rgen.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/direct_lighting.rgen.query.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-S" "comp" "-DKHR_RAY_QUERY" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/direct_lighting.rgen.query.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/direct_lighting.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/direct_lighting.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/direct_lighting.rgen.query.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/final_blit.frag.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/final_blit.frag.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/final_blit.frag"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/final_blit.frag" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/final_blit.frag.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/final_blit.vert.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/final_blit.vert.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/final_blit.vert"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/final_blit.vert" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/final_blit.vert.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/fsr_easu_fp16.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/fsr_easu_fp16.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/fsr_easu_fp16.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/fsr_easu_fp16.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/fsr_easu_fp16.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/fsr_easu_fp32.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/fsr_easu_fp32.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/fsr_easu_fp32.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/fsr_easu_fp32.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/fsr_easu_fp32.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/fsr_rcas_fp16.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/fsr_rcas_fp16.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/fsr_rcas_fp16.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/fsr_rcas_fp16.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/fsr_rcas_fp16.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/fsr_rcas_fp32.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/fsr_rcas_fp32.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/fsr_rcas_fp32.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/fsr_rcas_fp32.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/fsr_rcas_fp32.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/god_rays.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/god_rays.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/god_rays.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/god_rays.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/god_rays.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/god_rays_filter.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/god_rays_filter.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/god_rays_filter.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/god_rays_filter.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/god_rays_filter.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/indirect_lighting.rgen.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/indirect_lighting.rgen.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/indirect_lighting.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/indirect_lighting.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/indirect_lighting.rgen.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/indirect_lighting.rgen.query.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-S" "comp" "-DKHR_RAY_QUERY" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/indirect_lighting.rgen.query.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/indirect_lighting.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/combined/vkpt/shader/indirect_lighting.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/indirect_lighting.rgen.query.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/instance_geometry.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/instance_geometry.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/instance_geometry.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/instance_geometry.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/instance_geometry.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/normalize_normal_map.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/normalize_normal_map.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/normalize_normal_map.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/normalize_normal_map.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/normalize_normal_map.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/path_tracer.rchit.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/path_tracer.rchit.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer.rchit"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer.rchit" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/path_tracer.rchit.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/path_tracer.rmiss.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/path_tracer.rmiss.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer.rmiss"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer.rmiss" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/path_tracer.rmiss.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/path_tracer_beam.rahit.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/path_tracer_beam.rahit.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_beam.rahit"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_beam.rahit" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/path_tracer_beam.rahit.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/path_tracer_beam.rint.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/path_tracer_beam.rint.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_beam.rint"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_beam.rint" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/path_tracer_beam.rint.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/path_tracer_explosion.rahit.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/path_tracer_explosion.rahit.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_explosion.rahit"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_explosion.rahit" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/path_tracer_explosion.rahit.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/path_tracer_masked.rahit.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/path_tracer_masked.rahit.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_masked.rahit"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_masked.rahit" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/path_tracer_masked.rahit.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/path_tracer_particle.rahit.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/path_tracer_particle.rahit.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_particle.rahit"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_particle.rahit" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/path_tracer_particle.rahit.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/path_tracer_sprite.rahit.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/path_tracer_sprite.rahit.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_sprite.rahit"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/path_tracer_sprite.rahit" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/path_tracer_sprite.rahit.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/physical_sky.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/physical_sky.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/physical_sky.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/physical_sky.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/physical_sky.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/physical_sky_space.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/physical_sky_space.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/physical_sky_space.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/physical_sky_space.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/physical_sky_space.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/primary_rays.rgen.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/primary_rays.rgen.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/primary_rays.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/primary_rays.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/primary_rays.rgen.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/primary_rays.rgen.query.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-S" "comp" "-DKHR_RAY_QUERY" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/primary_rays.rgen.query.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/primary_rays.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/primary_rays.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/primary_rays.rgen.query.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/reflect_refract.rgen.pipeline.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/reflect_refract.rgen.pipeline.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/reflect_refract.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/reflect_refract.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/reflect_refract.rgen.pipeline.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/reflect_refract.rgen.query.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-S" "comp" "-DKHR_RAY_QUERY" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/reflect_refract.rgen.query.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/reflect_refract.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/reflect_refract.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/reflect_refract.rgen.query.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/shadow_map.vert.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/shadow_map.vert.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/shadow_map.vert"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/shadow_map.vert" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/shadow_map.vert.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/sky_buffer_resolve.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/sky_buffer_resolve.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/sky_buffer_resolve.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/sky_buffer_resolve.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/sky_buffer_resolve.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/stretch_pic.frag.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/stretch_pic.frag.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/stretch_pic.frag"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/stretch_pic.frag" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/stretch_pic.frag.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/stretch_pic.vert.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/stretch_pic.vert.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/stretch_pic.vert"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/stretch_pic.vert" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/stretch_pic.vert.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/tone_mapping_apply.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/tone_mapping_apply.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/tone_mapping_apply.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/tone_mapping_apply.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/tone_mapping_apply.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/tone_mapping_curve.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/tone_mapping_curve.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/tone_mapping_curve.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/tone_mapping_curve.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/tone_mapping_curve.comp.spv")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/shaders/tone_mapping_histogram.comp.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "--quiet" "-DVKPT_SHADER" "-V" "-DREFERENCE_VR_HISTORY=1" "--spirv-val" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/shaders/tone_mapping_histogram.comp.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/tone_mapping_histogram.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/tone_mapping_histogram.comp" ${q2rtx_shader_inputs}
  VERBATIM)
list(APPEND shader_outputs "${CMAKE_BINARY_DIR}/shaders/tone_mapping_histogram.comp.spv")
add_custom_target(q2rtx_shaders DEPENDS ${shader_outputs})
set(embedded_headers)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/embedded/bloom_margin_spv.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/embedded"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "-V" "--spirv-val" "-DVKPT_SHADER" "-DREFERENCE_VR_HISTORY=1" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/embedded/bloom_margin_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_margin.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/bloom_margin.comp" ${q2rtx_shader_inputs}
  VERBATIM)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/generated/bloom_margin_spv.h"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated"
  COMMAND "${Python3_EXECUTABLE}" "${Q2RTX_SHADER_SOURCE_ROOT}/tools/embed_spirv.py" --output "${CMAKE_BINARY_DIR}/generated/bloom_margin_spv.h" "--array" "bloom_margin_spv" "${CMAKE_BINARY_DIR}/embedded/bloom_margin_spv.spv"
  DEPENDS "${CMAKE_BINARY_DIR}/embedded/bloom_margin_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/tools/embed_spirv.py"
  VERBATIM)
list(APPEND embedded_headers "${CMAKE_BINARY_DIR}/generated/bloom_margin_spv.h")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/embedded/b2_wall_return_spv.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/embedded"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "-V" "--spirv-val" "-DVKPT_SHADER" "-DREFERENCE_VR_HISTORY=1" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/inputs/origin-003/src" "-o" "${CMAKE_BINARY_DIR}/embedded/b2_wall_return_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/b2_wall_return.comp"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/b2_wall_return.comp" ${q2rtx_shader_inputs}
  VERBATIM)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/generated/b2_wall_return_spv.h"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated"
  COMMAND "${Python3_EXECUTABLE}" "${Q2RTX_SHADER_SOURCE_ROOT}/tools/embed_spirv.py" --output "${CMAKE_BINARY_DIR}/generated/b2_wall_return_spv.h" "--array" "b2_wall_return_spv" "${CMAKE_BINARY_DIR}/embedded/b2_wall_return_spv.spv"
  DEPENDS "${CMAKE_BINARY_DIR}/embedded/b2_wall_return_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/tools/embed_spirv.py"
  VERBATIM)
list(APPEND embedded_headers "${CMAKE_BINARY_DIR}/generated/b2_wall_return_spv.h")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/embedded/b2_primary_guides_hit_spv.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/embedded"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "-V" "--spirv-val" "-DVKPT_SHADER" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/embedded/b2_primary_guides_hit_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/b2_primary_guides_reflection.rahit"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/b2_primary_guides_reflection.rahit" ${q2rtx_shader_inputs}
  VERBATIM)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/embedded/b2_primary_guides_query_spv.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/embedded"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "-V" "--spirv-val" "-DVKPT_SHADER" "-S" "comp" "-DKHR_RAY_QUERY" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/embedded/b2_primary_guides_query_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/b2_primary_guides_reflection.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/shader/b2_primary_guides_reflection.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/generated/b2_primary_guides_reflection_spv.h"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated"
  COMMAND "${Python3_EXECUTABLE}" "${Q2RTX_SHADER_SOURCE_ROOT}/tools/embed_spirv.py" --output "${CMAKE_BINARY_DIR}/generated/b2_primary_guides_reflection_spv.h" "--array" "b2_primary_guides_hit_spv" "${CMAKE_BINARY_DIR}/embedded/b2_primary_guides_hit_spv.spv" "--array" "b2_primary_guides_query_spv" "${CMAKE_BINARY_DIR}/embedded/b2_primary_guides_query_spv.spv"
  DEPENDS "${CMAKE_BINARY_DIR}/embedded/b2_primary_guides_hit_spv.spv" "${CMAKE_BINARY_DIR}/embedded/b2_primary_guides_query_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/tools/embed_spirv.py"
  VERBATIM)
list(APPEND embedded_headers "${CMAKE_BINARY_DIR}/generated/b2_primary_guides_reflection_spv.h")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/embedded/b2_crosshair_hit_spv.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/embedded"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "-V" "--spirv-val" "-DVKPT_SHADER" "-DREFERENCE_VR_HISTORY=1" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/crosshair87/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/embedded/b2_crosshair_hit_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/crosshair87/vkpt/shader/b2_crosshair_reflection.rahit"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/crosshair87/vkpt/shader/b2_crosshair_reflection.rahit" ${q2rtx_shader_inputs}
  VERBATIM)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/embedded/b2_crosshair_query_spv.spv"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/embedded"
  COMMAND "${GLSLANG_VALIDATOR}" "--target-env" "vulkan1.2" "-V" "--spirv-val" "-DVKPT_SHADER" "-DREFERENCE_VR_HISTORY=1" "-S" "comp" "-DKHR_RAY_QUERY" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/crosshair87/vkpt/shader" "-I${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/baseline/vkpt/fsr" "-o" "${CMAKE_BINARY_DIR}/embedded/b2_crosshair_query_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/crosshair87/vkpt/shader/b2_crosshair_reflection.rgen"
  DEPENDS "${Q2RTX_SHADER_SOURCE_ROOT}/source/baseline/b2-consolidated215/shaders/crosshair87/vkpt/shader/b2_crosshair_reflection.rgen" ${q2rtx_shader_inputs}
  VERBATIM)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/generated/b2_crosshair_reflection_spv.h"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated"
  COMMAND "${Python3_EXECUTABLE}" "${Q2RTX_SHADER_SOURCE_ROOT}/tools/embed_spirv.py" --output "${CMAKE_BINARY_DIR}/generated/b2_crosshair_reflection_spv.h" "--array" "b2_crosshair_hit_spv" "${CMAKE_BINARY_DIR}/embedded/b2_crosshair_hit_spv.spv" "--array" "b2_crosshair_query_spv" "${CMAKE_BINARY_DIR}/embedded/b2_crosshair_query_spv.spv"
  DEPENDS "${CMAKE_BINARY_DIR}/embedded/b2_crosshair_hit_spv.spv" "${CMAKE_BINARY_DIR}/embedded/b2_crosshair_query_spv.spv" "${Q2RTX_SHADER_SOURCE_ROOT}/tools/embed_spirv.py"
  VERBATIM)
list(APPEND embedded_headers "${CMAKE_BINARY_DIR}/generated/b2_crosshair_reflection_spv.h")
add_custom_target(q2rtx_embedded_shaders DEPENDS ${embedded_headers})
