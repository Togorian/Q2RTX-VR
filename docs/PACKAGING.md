# Preparing a runtime package

Use `packaging/package.py` with Python 3.10+ standard-library modules. The packager does not build/run products, contact a network or discover player directories. Default output is a local/private package with `privateCandidate=true` in `RELEASE-MANIFEST.json` and an `-audit.zip` outer archive.

After the fresh snapshot/build/receipt sequence in [BUILDING.md](BUILDING.md):

```text
python packaging/package.py package --source . --build <fresh-build> --libraries <libraries> --assets <accepted-runtime-assets> --shaders <fresh-shader-build> --output <new-empty-output>
```

The assets input provides the named accepted OpenXR loader at its runtime-relative path. The 51 runtime shaders come directly from `--shaders <fresh-shader-build>`, never the old asset folder, and must equal both the receipt and current payload pins. Recording also compares all six fresh embedded modules with the generated headers and selected native headers. The packager does not read that folder's saves/config/logs. Arrows come from source `assets/pics/b2/` and clean seeds from `packaging/defaults/`. Unchanged runtime inputs must match `packaging/payload-plan.json`. A changed shader needs a separately reviewed selection update, never automatic adoption.

`packaging/source-files.json` explicitly selects source members, preserving necessary vendor sources/notices/public fixtures, generator/calibration and shader recipes. Update it deliberately for new source. Arbitrary workspace files are not swept in. Private homes/saves/logs, owned data, old CMD launcher, Git metadata, audits/builds/backups are excluded. Five static libraries accompany source for relinking; SDK `vulkan-1.lib` remains external.

Output is one `Q2RTX VR/` folder with an inner corresponding-source ZIP, plus one outer ZIP and checksum. Inner members include `Q2RTX-VR-source/`, five `relinking-libraries/`, SDK-prerequisite note, actual build record and source member manifest. There is no second full source copy/extraction. Archive names and runtime README/log substitutions derive from [VERSION](../VERSION).

The explicit runtime template mapping selects user guides and preserves root provenance/notices, `VERSION`, all licences including CC0, artwork and tool guides. The arrows under `home/baseq2/pics/b2/` are generated with ChatGPT from Togorian prompts; CC0 covers only rights he holds. Depressed was generated as an edit of raised; delivered PNG bytes/metadata are preserved.

`SOURCE-MANIFEST.json` excludes itself. `RELEASE-MANIFEST.json` identifies actual payload members and excludes itself and `SHA256SUMS.txt`; checksums then include the completed manifest. The outer checksum is beside the outer ZIP. There is no self-hash cycle.

Refusals cover nonempty/overlapping outputs, traversal/ambiguous Windows paths, symlinks/reparse points, missing inputs, changed source/library snapshots, incorrect product version resources and mismatched products/assets. Archive members are verified after writing. A failure after output creation leaves it for inspection; use a new output path.

Privacy evidence scans actual product/library/source/package-member bytes in ASCII and both UTF-16LE alignments, including bounded nested archives. Unclassified user/worktree paths refuse packaging; neutral build paths are listed separately. Exact path/file-hash/matched-prefix-hash classifications distinguish examples, CI/platform paths and non-filesystem tokens. Retained upstream author/editor paths remain explicit findings and make `personalDataIncluded=true`; this is not a claim that all personal paths are absent. No project user data is admitted by those exact upstream classifications. This scoped scan is not universal secret detection.

Sorted ZIP members, fixed timestamps/permissions and compression provide deterministic archives for identical inputs/tooling. No bit-identical MSVC product, shader visual equivalence or headset acceptance is claimed. The package identity does not establish product execution or headset acceptance.

Focused fixtures retain results and never compile or execute their PE-shaped bytes:

```text
python packaging/test_package.py --output <fresh-fixture-directory>
```

Add `--release` only to the `package` command when deliberately producing a release identity. It sets `privateCandidate=false` and uses an outer filename without `-audit`; it does not publish or upload anything. `prepare` and `record` reject that flag. Omitting it always retains private identity. Existing output is never relabelled or overwritten.

Every source/library byte must match the build receipt, including documentation. A prose-only source change therefore invalidates an older package receipt. Use a fresh snapshot/build/record sequence; do not edit a receipt to adopt changed source.

The six engineering records remain in corresponding source only: `CONNECTION-CORRECTION.md`, `INPUT-COMFORT-RC5.md`, `RUNTIME-RECOVERY-RC5.md`, `RENDERER-DESIGN.md`, `MODIFICATION-NOTICES.md` and `UPSTREAM-COMPARISON.json` (all under `docs/`). Runtime guides refer to those extracted-source paths without runtime-relative links. New guides must be explicitly selected in both the source list and runtime mapping.

The six shipped configuration files include immutable `default-vr.cfg` and clean startup/reset templates. No player `q2config.cfg` is packaged. See [installation](INSTALLING.md) before updating an existing home.

## Completed build context

Before `record`, write `release-context.json` in the build directory from actual observations, outside the selected source. Schema 1 contains `sourceCommit` (full Git id), `sourceCommitUtc` (commit timestamp), `inputsSha256` (SHA-256 of the packager's canonical encoded input snapshot), and `buildStartedUtc` / `buildCompletedUtc` (actual UTC timestamps ending in Z, not the commit date). Observe a clean final Git snapshot before and after the build; compare its full source selection to `release-inputs.json`. Do not infer a commit or date from a directory name.

`tools` contains observed `compilerVersion`, `toolsetVersion`, `windowsSdkVersion`, `cmakeVersion` and `shaderCompilerVersion`. `commands` records each actual argument vector, working directory, phase (`shaders` or `native`), UTC `startedUtc` / `completedUtc` and successful `exitCode`. `logs` contains relative build-directory paths, byte lengths and SHA-256 values for retained compile/configure/tool-version evidence. Keep the actual compiler executable identity in that evidence. The reader verifies the evidence bytes and input snapshot; observations are not cryptographic compiler attestation. Inert fixtures label their observations as synthetic.

Complete the shader and native commands before generating context and recording. Keep unmodified logs locally; check all context text for private paths before distribution. Do not copy private raw logs into source or a package. Their hashes remain in the distributed receipt so the retained evidence can be identified. `distributedLogs` explicitly selects entries from `logs` as objects with `path` and `purpose`; at least one must have purpose `shader-compile`. Selected original log bytes are included under `build-evidence/` in the corresponding-source archive and pass the same privacy scan and archive byte verification as other members. Keep the original non-private shader compilation log available for a fresh audit; do not replace it with hashes alone. The packager rechecks context/evidence, full source/library equality and product identities before assembly. Never amend a frozen receipt or insert a receipt into its own committed source snapshot.

Record literal actual compile argument vectors and working directories. Do not silently redact them and call them raw commands. A portable recipe, if included, must be separately labelled as a recipe. A normal non-verbose shader build log records the actual completed work without echoing the generator's private interpreter path; keep any verbose/configure logs containing private paths local. Only original non-private log bytes may be selected for distribution. If an actual command/log contains private text, stop and resolve its delivery explicitly rather than sanitising it invisibly.
