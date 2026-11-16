# Third-party notices

This index identifies component notices. Original vendor headers and [NOTICE-UPSTREAM.txt](NOTICE-UPSTREAM.txt) remain applicable. The linked excerpts retain the selected notice wording; presentation-only extraction is identified below. They do not replace broader source-file obligations.

The combined engine retains its inherited GPL version 2 distribution basis. [LICENSE.txt](LICENSE.txt) preserves the inherited preamble credits to NVIDIA, Christoph Schied, Florian Simon, Tobias Zirr, Andrey Nazarov and Id Software. The [complete original upstream licence file](licences/Q2RTX-UPSTREAM-LICENSE.txt) and [exact GPLv2 terms](licences/GPL-2.0.txt) are also supplied. The original demo-data appendix does not authorise bundling game data or change the modified engine's licence.

## Selected engine components

| Component and selected source | Use and notice |
|---|---|
| tinyobjloader-c, `origin-011/upstream/extern/tinyobjloader-c/tinyobj_loader_c.h` under `source/baseline/b2-consolidated215/inputs/` | Included by the selected mesh code; [Syoyo Fujita MIT notice](licences/tinyobjloader-c-MIT.txt). No unverified library release number is assigned. |
| DirectXTK-derived DDS definitions, `origin-025/upstream/src/refresh/vkpt/dds.h` | Included by selected sky/texture code; [Microsoft MIT and origin notice](licences/DirectXTK-dds-MIT.txt), including the DXGI source context. |
| Hershey futural/timesr data, `origin-046/upstream/src/refresh/debug_fonts/` | Included by selected debug-text code; exact [futural](licences/Hershey-futural-notice.txt) and [timesr](licences/Hershey-timesr-notice.txt) notices, WTFPL v2, acknowledgements and the original NTIS-format restriction. |
| stb headers | Selected upstream header implementations retain their individual notices and alternatives; [stb licence](licences/extern-stb-LICENSE). |
| Mesa-derived rotation matrix, `SetupRotationMatrix` in `math.c` | Adapted from Mesa 3D's `_math_matrix_rotate`; [Brian Paul MIT-style notice](licences/Mesa-7.5.1-MIT.txt). See the notice-source qualification below. |
| Floating-point conversion, `floatToHalf` and `halfToFloat` in `conversion.h` | Adapted from Stack Overflow answer 3542975 by Phernost; [Unlicense text](licences/Unlicense.txt). See the author's licence offer and source links below. |
| Original VR functions | Applicable id Software notices remain; see the [full origin account](PROVENANCE.md). The confirmed own-contribution GPLv2-or-later grant does not relicense upstream work. |

The Mesa notice is copied from `Mesa-7.5.1/src/mesa/math/m_matrix.c` in the [official Mesa 7.5.1 source archive](https://archive.mesa3d.org/older-versions/7.x/7.5.1/MesaLib-7.5.1.tar.gz), which contains `_math_matrix_rotate`. The copyright, permission and disclaimer wording is retained verbatim, with C comment prefixes removed and LF line endings. Version 7.5.1 identifies the verified notice source; the exact historical Mesa version NVIDIA adapted is unknown.

For the conversion functions, see [Phernost's answer 3542975](https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion#3542975) and its [historical revision](https://stackoverflow.com/revisions/3542975/4). The author explicitly offered the code under the Unlicense in 2016, and the current answer carries that licence. The shipped text is copied unchanged from the [official Unlicense text](https://unlicense.org/UNLICENSE), linked by [unlicense.org](https://unlicense.org/). This records the attribution and licence offer without claiming which answer revision NVIDIA originally acquired.

Selected source headers additionally retain credits including Frank Richter and Jonathan “Paril” Barkley in debug support, r1ch.net in HTTP/Windows support, Andrew Tridgell in MD4 code, and Matthias Bentrup, Thilo Schulz and Zack Middleton in IQM support. stb retains its contributor list, including Nicolas Schulz. These are examples, not a substitute for every individual header or an attribution of their work to Togorian.

## Libraries and associated material

| Component | Selected version/use | Notice material |
|---|---|---|
| OpenAL Soft | 1.24.1, selected for static linking; editable source and compatible relink route retained | [LGPL COPYING](licences/openal-soft-COPYING), [BSD-3-Clause](licences/openal-BSD-3Clause.txt) |
| OpenAL pffft | Selected `common/pffft.cpp` is included in the selected library build | [Exact pffft licence](licences/openal-pffft.txt) |
| OpenAL bs2b | Selected `core/bs2b.cpp` is included in the selected library build | [Boris Mikhaylov MIT notice](licences/openal-bs2b-MIT.txt) |
| MIT KEMAR data | OpenAL default HRTF material is embedded in the recorded selection; associated `utils/MIT_KEMAR.def` is retained | [MIT Media Lab 1994 notice and citation](licences/openal-MIT-KEMAR-citation.txt); credit Bill Gardner and Keith Martin. This is a data/citation notice, not the MIT software licence. |
| fmt | 11.0.2, bundled with selected OpenAL source | [fmt licence](licences/fmt-11.0.2-LICENSE.txt) |
| SDL2 | 2.32.10, static SDL2/SDL2main | [SDL licence](licences/SDL2-LICENSE.txt) |
| SDL yuv2rgb | Selected conversion code included in the selected library build | [Exact BSD notice](licences/SDL-yuv2rgb-BSD.txt) |
| SDL HIDAPI | Selected HIDAPI code included in the selected library build | [SDL and Alan Ott notices](licences/SDL-HIDAPI-notice.txt) |
| SDL fdlibm | Selected `e_atan2.c` selected with the fallback enabled | [Sun notice](licences/SDL-fdlibm-Sun-notice.txt) |
| curl | Preserved 8.7.0-DEV source, static library | [curl COPYING](licences/curl-COPYING) |
| curl inet_ntop / inet_pton | Translation units compiled, but system-function availability disables these fallback bodies in the recorded Windows configuration; retained source notices | [inet_ntop ISC](licences/curl-inet_ntop-ISC.txt), [inet_pton ISC](licences/curl-inet_pton-ISC.txt) |
| curl krb5 | Translation unit compiled, but FTP is disabled and GSSAPI is not enabled; retained source-only feature | [Exact BSD notice](licences/curl-krb5-BSD.txt) |
| zlib | 1.3.1, static library | [zlib licence](licences/zlib-LICENSE) |
| Vulkan C headers | `vulkan-sdk-1.4.357.0`, commit `e3b1eec08173d6b825cd3ac88c885a63b621504a` | [Upstream licence information](licences/Vulkan-Headers-LICENSE.md), [MIT](licences/extern-Vulkan-Headers-LICENSES-MIT.txt) elected where offered, [Apache](licences/extern-Vulkan-Headers-LICENSES-Apache-2.0.txt) retained. SDK import library/driver are separate external prerequisites. |
| OpenXR | Named source and accepted Windows x64 loader release 1.1.62 | [Election and origin limits](licences/openxr/LICENCE-ELECTION.md), [COPYING](licences/openxr/COPYING.adoc), [binary-package licence](licences/openxr/binary-package-LICENSE.txt), [MIT](licences/openxr/MIT.txt), [Apache](licences/openxr/upstream-Apache-LICENSE.txt), [JsonCpp](licences/openxr/jsoncpp-LICENSE.txt). Generator/generated-source notices remain in the same directory. |

This SDL2 selection disables OpenGL ES and EGL while retaining desktop OpenGL/WGL and Vulkan. The Apache-only EGL and OpenGL ES headers remain in the vendored source with their licence notices; they are not compiled into the selected programs. The dependency selection and target include evidence are recorded in `recipes/sdl-build-validation.json`; the delivered `BUILD-RECORD.json` identifies the actual product build.

Selected translation units do not imply every optional function is active. Selected vendor files retain their original notices. Only the unbuilt material listed in docs/DEPENDENCIES.md is omitted from the export; original source notices remain applicable. This does not simplify or replace applicable licences.

See [dependency/relink instructions](docs/DEPENDENCIES.md) and [source/build instructions](docs/BUILDING.md). The engine's external game data is not included or relicensed. 

## Images

The two menu arrow PNGs were generated with ChatGPT (OpenAI image generation) from Togorian's prompts on 8 October 2026. The depressed image was generated as an edit of the raised image. Both delivered files and all embedded metadata remain unchanged.

On 11 November 2026, Togorian dedicated any copyright and related rights he holds in these two images to the public domain under CC0 1.0 Universal, to the extent legally possible, with the CC0 fallback licence. This applies only to his rights in these two PNGs; it does not relicense other parties' work, code or game data. No exclusive copyright or human authorship is claimed.

[Canonical CC0 terms](https://creativecommons.org/publicdomain/zero/1.0/legalcode.en) · [Complete licence text](licences/CC0-1.0.txt) · [Artwork record, filenames and exact identities](assets/pics/b2/README.md). In the runtime layout these images are under `home/baseq2/pics/b2/`.

The charcoal square with two cyan eye panels is an original placeholder generated by the supplied `tools/make_icon.py`, with no third-party image or AI image generation. The generator carries the project contribution licence. The arrow images above are separate, unchanged assets.

The icon generator `tools/make_icon.py` is provided inside the corresponding-source archive, not the runtime folder. Extract the source archive to use it.
