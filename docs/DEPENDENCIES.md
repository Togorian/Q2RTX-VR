# Dependency sources and builds

Editable curl, OpenAL Soft/fmt, SDL2, zlib and MIT-offered Vulkan C headers are under `dependencies/src/`; compatible options are in `dependencies/options/`. Corresponding source includes five static relinking libraries beside `Q2RTX-VR-source/`. Supply the Windows x64 Vulkan SDK import library separately. Historical validation records describe their recorded bytes; use the current build receipt for a new product.

| Engine link input | Source or external prerequisite | Licence material |
|---|---|---|
| libcurl.lib | Preserved curl 8.7.0-DEV source | curl COPYING |
| OpenAL32.lib | Preserved OpenAL Soft 1.24.1, including bundled fmt 11.0.2 | OpenAL COPYING and fmt licence |
| SDL2-static.lib / SDL2main.lib | Preserved SDL2 2.32.10 | SDL2 LICENSE.txt |
| zlibstatic.lib | Preserved zlib 1.3.1 | zlib LICENSE |
| vulkan-1.lib | Separately supplied x64 import library from installed Vulkan SDK 1.4.304.1 (66,154 bytes; exact identity in build-plan.json) | SDK import provenance is separate from MIT-offered Vulkan C headers |

The Vulkan import library is separate from Vulkan-Headers. The installed driver/loader is an external runtime dependency. Microsoft compiler/Windows SDK headers and system import libraries are external prerequisites and are not copied into this source tree.

## Compatible static library builds

Use the preserved sources, an x64 MSVC developer shell, CMake and Ninja. The reference options under `dependencies/options/` select the established Windows static-runtime configuration. The engine may use compatible rebuilt libraries with reference hash verification left off. Byte-for-byte reproduction is not required for normal development.

Run these commands from the repository root after source assembly:

```text
cmake -S dependencies/src/zlib -B build-deps/zlib -G Ninja -C dependencies/options/zlib.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
cmake --build build-deps/zlib --target zlibstatic
cmake -S dependencies/src/SDL2 -B build-deps/SDL2 -G Ninja -C dependencies/options/SDL2.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
cmake --build build-deps/SDL2 --target SDL2-static SDL2main
cmake -S dependencies/src/openal-soft -B build-deps/openal-soft -G Ninja -C dependencies/options/openal-soft.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
cmake --build build-deps/openal-soft --target OpenAL
cmake -S dependencies/src/curl -B build-deps/curl -G Ninja -C dependencies/options/curl.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
cmake --build build-deps/curl --target libcurl_static
```

Select Release for try-compile as shown above, including C and C++ compiler/ABI probes. Preserve the selected generated configuration headers and shared MIT-offered Vulkan include selection.

Collect the five rebuilt static libraries `zlibstatic.lib`, `SDL2-static.lib`, `SDL2main.lib`, `OpenAL32.lib`, `libcurl.lib` and supply your SDK's x64 `vulkan-1.lib` in the directory passed as `Q2RTX_DEPENDENCIES`. CMake may place a component's library in its `lib/` subdirectory. Keep the matching generated configuration headers with each dependency build. The selected engine already retains its matching reference headers; upgrading an API/configuration requires corresponding header/source changes, rather than silently swapping unrelated versions.

The curl recipe intentionally preserves the selected HTTP-only configuration and disabled optional TLS/SSH/compression backends; the actual configure reports HTTP, IPFS and IPNS with no TLS backend. It does not provide newly enabled HTTPS support. OpenAL's source includes fmt and its licence, so that subdirectory must not be dropped. Static OpenAL source and the full editable engine build route must remain available for rebuilding/relinking modified versions.

## OpenXR loader

The selected Windows x64 loader is from [Khronos OpenXR 1.1.62](https://github.com/KhronosGroup/OpenXR-SDK-Source/releases/tag/release-1.1.62). The [official Windows archive](https://github.com/KhronosGroup/OpenXR-SDK-Source/releases/download/release-1.1.62/openxr_loader_windows-1.1.62.zip) is 30,975,472 bytes. Its SHA-256, published on the [release asset digest page](https://github.com/KhronosGroup/OpenXR-SDK-Source/releases/expanded_assets/release-1.1.62), is `800ec772e2f9448a26ab9f579f4914d984346dd9d0d7c007841abe21d2c8ff2f`.

The retained archive matches that published digest. Its `x64/bin/openxr_loader.dll` member is byte-identical to the selected runtime asset: 2,669,568 bytes, version 1.1.62.0, locally computed SHA-256 `c03b44a09072dc276aabe5b390f642054fac83b38c1c575fcfce56f0efc37ea4`. The DLL checksum is a local member comparison, not a separately published DLL digest. Packaging checks the selected asset again.

The corresponding official `OpenXR-SDK-Source-release-1.1.62.tar.gz` is 4,834,887 bytes, SHA-256 `977073d7f4c0d1af8ab975f57e4b6ffd1c4e9209be66075812b890576e0e1e5f`, also matching the published asset digest. These comparisons establish the retained archive/member correspondence; no signature verification or reproducible loader build is claimed. The existing signature-verification limitation (`NO_PUBKEY`) remains. The preserved `OpenXR-SDK-Source-release-1.1.62.tar.gz` contains loader source, registry, generators and vendored JsonCpp. Extract that named release to `dependencies/src/OpenXR-SDK-Source` when assembling sources; the unversioned SDK-main archive is excluded from the selected clean source export.

Use Python 3 for generation. The release archive includes its Jinja2 Python material used for reflection headers; retain `external/python` and the registry/scripts. No specification build or specification-only Python packages are needed for the loader target.

```text
cmake -S dependencies/src/OpenXR-SDK-Source -B build-deps/openxr -G Ninja -C dependencies/options/openxr.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
cmake --build build-deps/openxr --target openxr_loader
```

A rebuilt `openxr_loader.dll` is an alternative compatible build requiring normal product/runtime testing, not an assertion that it matches the accepted DLL bytes. The retained source/loader provenance and verification limits are described in `licences/openxr/LICENCE-ELECTION.md`.

Existing notices retain the offered MIT election for eligible dual-licensed OpenXR/JsonCpp material, all original copyright/dual-licence text and Apache-only file notices. This does not relicense Apache-only files. Component notices and source/relinking material still belong in the assembled package. See [NOTICE.md](../NOTICE.md) and [SHADERS.md](SHADERS.md). The weapon-table generator/parser and editable calibration subset are supplied separately. Its required model files remain user-owned external inputs, not dependency code or included game media.

## Vulkan C-header selection

The source selects the 34 C headers from Khronos Vulkan-Headers tag `vulkan-sdk-1.4.357.0`, commit `e3b1eec08173d6b825cd3ac88c885a63b621504a`. Each selected header offers `Apache-2.0 OR MIT`; this project elects MIT while preserving the original copyright and dual-licence text. `dependencies/src/Vulkan-Headers/` includes the upstream README and licence files. Old Apache-only C headers and unused C++ wrappers/generator material are excluded from the new selection; they have not been relicensed. This changes neither the requested Vulkan API version/features nor the installed SDK, driver or runtime.

The accepted OpenXR 1.1.62 DLL was matched to its retained official binary archive. Its earlier detached-signature attempt lacked a public key; no signature pass or byte-rebuild claim is made. Eligible dual-licensed source uses its offered MIT option, with Apache-only notices retained for other source material. These observations do not establish a new build or headset result.

SDL2 is an explicitly adapted source version: its internal Vulkan reader uses `<vulkan/vulkan.h>`, and its CMake target selects `../Vulkan-Headers/include` before other directories and fails if that sibling selection is missing. The old `SDL2/src/video/khronos/vulkan/` tree and all legacy origin-folder Vulkan header trees are excluded from the selected export. This keeps the Windows Vulkan backend enabled.

The source export excludes SDL test/sample.wav, the unused iOS demo bitmap font and unused hidapi Windows DDK example makefile. They are not inputs to the selected Windows CMake library targets with SDL tests disabled. No other vendor fixture is removed on that basis. OpenAL `utils/SCUT_KEMAR.def` is a dataset description, not the noncommercial dataset itself; no such dataset is supplied. The unused SCUT_KEMAR description is omitted from the export. Applicable OpenAL notices, MIT_KEMAR description, embedded HRTF and fmt sources remain.

This SDL2 selection disables OpenGL ES and EGL while retaining desktop OpenGL/WGL and Vulkan. The Apache-only EGL and OpenGL ES headers remain in the vendored source with their licence notices; they are not compiled into the selected programs. The dependency selection and target include evidence are recorded in `recipes/sdl-build-validation.json`; the delivered `BUILD-RECORD.json` identifies the actual product build.

## Selected unbuilt material

The source export omits 2,179 files under `dependencies/src/curl/tests/`, 137 under `dependencies/src/curl/docs/examples/`, and `dependencies/src/openal-soft/utils/SCUT_KEMAR.def`. Their preserved originals are not changed or deleted. curl's library recipe has BUILD_TESTING=OFF and BUILD_CURL_EXE=OFF; `tests` is conditional, and the `docs/examples` subdirectory call is commented out. Other curl docs, CMake/configure inputs, lib sources, headers and COPYING remain selected. OpenAL utilities/examples/tests are disabled; the SCUT description is not a compiled or configure input to OpenAL. The listed SDL sample/font/DDK files are absent from the selected tree. This is a scoped source export selection.

Compatible static libraries can be reused when their configuration and selected interfaces remain unchanged. Enabling omitted examples/tests requires obtaining their upstream material separately and reviewing its terms; the supplied commands target the retained library configurations.
