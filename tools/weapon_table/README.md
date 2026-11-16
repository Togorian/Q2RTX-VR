# Weapon reference table

`generate.py` is the portable adaptation of the original `table-01.py` generator. It uses the original `model_geometry.py` parser, unchanged, and the editable subset of the recorded calibration06 selection in `calibration.json`. `origins.json` identifies the historical inputs and selected output. The Python files retain their GPL-2.0-or-later notices. The historical calibration JSON has no separate file-level licence notice; its authored selections and rationale are preserved with their provenance.

The engine normally compiles the checked-in `b2_weapon_model_data.inc`; Python and game model files are not required for an ordinary build. Regeneration is an optional developer operation. It reads the twelve specific user-owned model files listed in `calibration.json`, verifies their exact lengths and SHA-256 identities, then preserves the original reference groups, vertex IDs, ready frames, scales and basis modes. A different asset revision fails explicitly. These hashes identify compatible inputs; they do not grant redistribution rights. Model/texture files are not supplied or relicensed by this tool.

Use Python 3.9 or later, without `-O`. Supply an existing directory whose `models/weapons/...` entries match the calibration. Asset acquisition/extraction is outside this tool. Choose an output file that does not already exist, with an existing parent directory:

```text
python -B tools/weapon_table/generate.py --assets-dir "<owned-model-directory>" --output "<new-header-path>"
```

`--calibration` optionally selects an edited calibration JSON. The default is the adjacent file. The tool reads only the named inputs and writes only the explicit new output. It does not search installations, download/extract archives, start the game or overwrite checked-in source. Invalid identities, formats, reference/triangle bounds and ready frames produce errors.

The table contains authored calibration/reference groups, exact asset identities and mesh/vertex/triangle counts. It contains no copied triangle-index arrays and is not a replacement model or a new calibration algorithm. The consumer matches the runtime model name, full raw-byte SHA-256, size and frame/mesh counts before binding the chosen groups. It compares loaded indices directly with the triangle indices in that exact owned raw file, with checked spans and index bounds. It also accounts for the loader's MD3 winding and MD2 seam expansion. The groups supply the existing muzzle/grip/basis conventions; the retained rationale distinguishes measured arithmetic from proposed controller alignment. No physical grip or headset validation is claimed.

`asset.entry` is the actual MD2/MD3 file to read. `modelReference` is the separate name used by the existing runtime: slots 0-10 retain their `.md2` compatibility names, even where the original file is MD3. Only the already-selected flare reference uses `.md3`. Do not replace all runtime names with the input filenames. The output carries the 10 November 2026 modification notice for removing copied topology arrays; its calibration and count fields retain their prior values.

The slim calibration retains the consumed fields plus mesh names, selection descriptions, anchor meaning, notes and classification rationale. It omits local paths, archive administration, incident-triangle lists and calculated ready/all-frame geometry. The unchanged parser decodes triangles for validation and counts, but their index arrays are no longer emitted. Editing selections is explicit source work; there is no automatic recalibration.

The assembled selected compiler input is `source/baseline/b2-consolidated215/inputs/origin-003/include/refresh/b2_weapon_model_data.inc`. Compare your generated header before explicitly adopting changes there and rebuilding its consumers. The preparation comparison is recorded in `recipes/weapon-table-plan.json`; it does not qualify a future edited table or complete the rest of the source package.

Modified 10 November 2026: the generator accepts an explicit byte reader internally for in-memory verification; the documented CLI still reads only the named user-owned paths. No model geometry is distributed. Removing the former copied triangle arrays does not grant rights to the underlying models.

The CLI checks each explicit input file size before reading it, then verifies the full size/hash identity. The internal in-memory reader and generated calibration/count bytes are unchanged. No archive discovery or extraction is performed.

The `generate.py` tool is in `tools/weapon_table/` inside the corresponding-source archive. Extract that source archive before following these instructions; the runtime folder contains this guide only.
