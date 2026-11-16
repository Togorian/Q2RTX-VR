# Shader regeneration and comparison

The selected recipe covers **51 shipped variants**, **six embedded arrays in four headers** and a **107-file shader input tree**, with an external wall-return relative include dependency. Lighting overrides retain their producing `path_tracer_rgen.h`; crosshair commands use their isolated source/include closure. The actual selection and options are in `shader-build/` and the source recipes.

Run the shader-only project from extracted corresponding source:

```text
glslangValidator --version
cmake -S shader-build -B <fresh-shader-build> -G Ninja -DGLSLANG_VALIDATOR=<exact-compiler-path>
cmake --build <fresh-shader-build> --target q2rtx_shaders q2rtx_embedded_shaders --parallel 1
```

Record the actual compiler identity/version before compilation. Preserve ordered include paths and recipe options. Crosshair uses `REFERENCE_VR_HISTORY` with diagnostic B1 omitted. The primary-guide pair requires `VKPT_SHADER` with `REFERENCE_B_B0P` and `REFERENCE_B_B1` absent. A compiler directory/version label alone does not establish output identity.

Outputs are `shaders/` (51), `embedded/` (six) and `generated/` (four headers) under the chosen build directory. Use a fresh output directory and preserve the selected reference blobs. Native builds use a separate `product-version/` include directory; generating shaders does not silently replace accepted embedded arrays.

Compare every output size/hash with its reference and decode all six embedded arrays. Header formatting/comments are separate from decoded module identity. Retain and investigate differences using disassembly, interfaces and operations where needed. Structural validation or removing debug names does not establish semantic or visual equivalence. Any changed runtime blob must also match an explicitly updated payload pin.

Corresponding source includes GLSL, recipes and embedded headers. The dated `recipes/shader-input-provenance545.json` retains historical input hashes and observations; it is not a current build receipt. Detailed implementation evidence is in source-only `docs/RENDERER-DESIGN.md`. Current product/package identities belong to `BUILD-RECORD.json` and `RELEASE-MANIFEST.json`; offline shader comparison does not establish GPU or headset results.

The [weapon-table generator](../tools/weapon_table/README.md) is separate. Its user-owned model inputs are not read by shader generation.

For a complete release build, freeze the source commit first. Preserve the actual shader configure/compile logs and compiler version/executable hash. Include the original non-private compile log in the corresponding-source archive; private configure/verbose evidence stays local and hash-identified. Before starting a clean native build, run:

```text
python packaging/package.py verify-shaders --source . --build <fresh-native-build> --libraries <libraries> --shaders <fresh-shader-build>
```

This reads fresh outputs, checks selected GLSL pins, compares all 51 runtime modules with selected reference hashes, and decodes each of the six arrays in both generated and selected native headers. The native compiler consumes the selected headers directly; exact decoded equality establishes that their modules equal the fresh outputs. Header formatting and comment differences are recorded separately. The `q2rtx_release_record` target does not compile shaders; simply enabling the shader targets does not supply this sequence.

Any mismatch stops before native compilation. Investigate the actual difference and review an explicit source/header/payload selection correction, if justified. Commit that correction before another complete clean build; never invent future hashes, silently adopt outputs, amend an old receipt or call an incremental relink clean. The C-side `REFERENCE_VR_HISTORY` selection in `path_tracer.c` is covered by the clean native compilation, not by GLSL regeneration.

Use the same fresh shader root for recording and packaging. The packager reads its 51 runtime modules directly and binds all 57 module identities, generated header hashes and selected native header hashes in `BUILD-RECORD.json`. Retain actual compile evidence outside selected source; hashes in the receipt identify local evidence, while the original non-private shader compile log is separately included under `build-evidence/` without private raw tool paths.
