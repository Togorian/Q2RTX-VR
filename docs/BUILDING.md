# Building and relinking

[VERSION](../VERSION) defines product and resource versions. Use Windows x64, an x64 MSVC developer shell, CMake 3.25+, Ninja and Python 3.10+. Supply compatible static libraries and the external Vulkan SDK import library as described in [DEPENDENCIES.md](DEPENDENCIES.md).

Build records apply only to their exact source/library snapshot. The corresponding-source archive's `BUILD-RECORD.json` and runtime `RELEASE-MANIFEST.json` identify a delivered package. Dated validation JSON under `recipes/` is historical evidence, not proof of a new build.

Ordinary development:

```text
cmake -S . -B <fresh-build> -G Ninja -DCMAKE_BUILD_TYPE=Release -DQ2RTX_DEPENDENCIES=<libraries> -DQ2RTX_VERIFY_REFERENCE_INPUTS=OFF
cmake --build <fresh-build> --parallel 1
```

For a release build, first commit the complete source/version/docs/reader selection. Regenerate and compare shaders using [SHADERS.md](SHADERS.md) before native compilation, then snapshot explicit inputs before configuring/building, then complete all products before recording the completed run:

```text
python packaging/package.py prepare --source . --build <fresh-build> --libraries <libraries>
cmake -S . -B <fresh-build> -G Ninja -DCMAKE_BUILD_TYPE=Release -DQ2RTX_DEPENDENCIES=<libraries> -DQ2RTX_VERIFY_REFERENCE_INPUTS=OFF -DQ2RTX_RELEASE_RECORD=ON
cmake --build <fresh-build> --target q2rtx_release_products --parallel 1
# Save actual completed-run observations as <fresh-build>/release-context.json.
python packaging/package.py record --source . --build <fresh-build> --libraries <libraries> --shaders <fresh-shader-build>
```

Prepare creates only `release-inputs.json`. Build all three products from an absent output tree, retain the completed command logs, and create the external `release-context.json` described in [PACKAGING.md](PACKAGING.md). Only then record actual product hashes/version resources in `release-build.json`. The optional receipt target still depends on all three products but now requires this completed context; it cannot create that context while its own outer build command is still running. Use a fresh release build path for each changed snapshot; records are not silently replaced. These records bind observed files and the normal build dependency sequence, not cryptographic compiler attestation or byte-identical reproduction.

Outputs remain `bin/q2rtx.exe`, `bin/baseq2/gamex86_64.dll` and `bin/Q2RTX VR.exe`: 180 engine/game C files (130 engine and 50 game) and two resources, plus launcher C/resource/link. Product/resource/console/crash/launcher/log identifiers derive from VERSION. The generated header lives in `product-version/`, separate from shader-generated headers. Recorded upstream/diagnostic bytes remain in `recipes/upstream-origin.h`, not presented as a new upstream revision.

Reference checking is optional/off by default. Compatible changes remain buildable with `Q2RTX_VERIFY_REFERENCE_INPUTS=OFF`; missing inputs/compiler/link failures still stop. Use real neutral source/build paths for distributable products. No path alias or binary string patching is used. Keep editable OpenAL Soft/fmt, notices and the full compatible relink route.

See [launcher checks](LAUNCHER.md), [packaging](PACKAGING.md) and [shader comparison](SHADERS.md).

VERSION accepts `major.minor.patch` or `major.minor.patch-rc.number`, with each resource component in 0..65535 and a positive RC number. Plain versions use resource component four = 0 and clear the prerelease flag. The current value is read from VERSION. Display names are Q2RTX VR; the compatible wire userinfo prefix remains lowercase `q2rtx`. The original placeholder icon can be regenerated with `python tools/make_icon.py <new-output.ico>`; the generator refuses to overwrite an existing file.

Assign a new VERSION to any later material corrective build intended for distribution. Keep earlier products and their original receipts labelled with their actual version; do not relabel them retroactively.

Focused CPU regression tests are retained under `tools/`: `collision-movement-regression.py`, `collision-ground-regression.py`, `collision-publication-regression.py`, `collision-fire-regression.py`, `publication-gap-regression.py`, `ending-newgame-regression.py` and `stop-message-regression.py`. Run each from the x64 MSVC developer shell with `python -I -B -S tools/<script> --output <new-directory-outside-source>`. Each requires a separate absent output directory and retains compiler/run logs and extracted-source hashes. Pre-correction controls demonstrate the original refusals or recovery errors. These fixtures use selected production code with controlled external seams; they do not replace real map, graphics, headset or save-operation testing.
