# Project modifications

Q2RTX VR adapts the upstream engine for OpenXR stereo rendering, tracked input and local single-player play. The VR integration includes room-scale movement, physical crouch, teleport, per-hand action availability, controller menus, comfort settings and deliberate interruption recovery. A blocked muzzle suppresses weapon display/firing while independently qualified movement can leave the obstruction.

The renderer maintains stereo history, allocation and completion independently of diagnostic capture owners. Runtime shader selection and embedded modules are described in [SHADERS.md](SHADERS.md). Weapon geometry is validated against user-owned models; the editable calibration and generator are supplied with the [weapon-table tool](../tools/weapon_table/README.md).

Save handling validates complete staged slots, preserves previous slots and reports completed writes or retained-backup failures. Immutable VR defaults load before player preferences; updates preserve settings and saves. The Windows launcher supplies checked startup arguments and diagnostics. Remote networking, downloads, demos and recording are disabled while local play and level transitions remain available.

The source selects MIT-offered Vulkan C headers and adapts SDL's shared-header include selection. Original notices and inherited terms remain. See [provenance](../PROVENANCE.md), [third-party notices](../THIRD-PARTY-NOTICES.md), [dependencies](DEPENDENCIES.md) and [licensing roles](../NOTICE.md).

Detailed engineering notes and dated evidence are retained inside the extracted corresponding source: `docs/MODIFICATION-NOTICES.md`, `docs/RENDERER-DESIGN.md`, the input/runtime recovery notes and `recipes/shader-input-provenance545.json`. Their historical observations apply to the bytes and dates they identify. Notice-insertion dates are not implementation dates. The delivered package's `BUILD-RECORD.json` and `RELEASE-MANIFEST.json` identify its actual build and payload.
