# Installing Q2RTX VR

Install the complete runtime folder; replacing only an executable can leave incompatible shaders, defaults or libraries. A source tree is not a runtime installation. See [requirements](REQUIREMENTS.md).

Use the complete runtime folder, with **Q2RTX VR.exe** at its top level.

1. Close the game and preserve any previous complete Q2RTX VR folder as a rollback copy.
2. Put the new **Q2RTX VR** folder directly inside your owned **Quake II RTX** installation. Avoid double-nesting. Its parent must contain `baseq2/pak0.pak`, `pak1.pak`, `pak2.pak`, `q2rtx_media.pkz` and `blue_noise.pkz`. Keep films, music and other game data in that original installation.
3. For a fresh installation, use the package’s clean `home`; do not merge unrelated settings or save sets. For an update, keep your existing `home` and replace every file the new package ships under `home/baseq2`: `default-vr.cfg`, the other five supplied `.cfg` files, `shader_vkpt/` and `pics/`. Preserve `q2config.cfg`, the save folder, logs and player-authored startup files. Merge any personal additions to `autoexec.cfg` into its updated template rather than discarding them. The package never supplies a player `q2config.cfg`.
4. Select your normal active headset/OpenXR runtime and start **Q2RTX VR.exe**. The same entry point serves the supported runtime routes.

**VR comfort and safety:** Clear obstacles from your play space and keep people and pets out of reach. Follow your headset manufacturer's safety guidance and use its boundary system where available. Take regular breaks and stop playing if you feel unwell or uncomfortable.

The folder needs ordinary write access for preferences, saves and `startup-status.txt`. It reads owned assets from its parent and keeps a separate VR `home`. It does not select/install runtimes, change drivers, elevate privileges or migrate the flat installation's settings or saves.

Current clean/reset defaults are controllers, right-handed with Togorian's preferred bindings, **smooth turning at 360 degrees/second**, selectable 45-degree snap turning, automatic view/foveation requests, **teleport-only vignette** and 100% application render scale. The external runtime controls its own resolution. Graphics options marked for next launch require a full restart. Saved preferences remain the player's own after first launch. The package contains `default-vr.cfg`, not `q2config.cfg`. Defaults load first, then only the VR home's saved preferences; the parent flat-game q2config is not inherited. Generated saved configs retain deliberate unbindings. Partial templates and custom configuration scripts overlay the defaults. The game creates or updates its own `q2config.cfg` on normal configuration saving. Reset to Defaults is the explicit way to choose the comfort defaults.

Home recentres; End switches controllers / keyboard and mouse. These actions can be rebound in Options → Action Key Bindings. Startup does not reset keys. Use the supplied optional reset configuration only when intentionally restoring the documented bindings.

The runtime's `home/baseq2/pics/b2/` contains the two menu arrows generated with ChatGPT from Togorian's prompts, covered by CC0 1.0 Universal for rights he holds. The depressed image was generated as an edit of the raised image; delivered PNG bytes and embedded metadata are preserved. See the [artwork record](../assets/pics/b2/README.md) and [complete CC0 text](../licences/CC0-1.0.txt).

## Saves and recovery

This save format has no old-save importer or backward exporter. Preserve earlier saves with the complete build that created them. To roll back, use that earlier folder without merging homes.

The Windows save transaction prepares and validates a complete staged slot before replacing the destination. It moves the prior slot to a unique backup, then moves the stage into place. A failed move triggers rollback where possible; otherwise the complete stage and prior backup remain for inspection. Keep backups of important saves. If saving is interrupted by a power cut or system crash, follow the included save-recovery instructions; recovery cannot be guaranteed.

If interrupted, close the game and copy the entire VR home somewhere safe before recovery. Use the exact paths reported in the log: the backup ends in `.old` and the related stage name contains `.b2-stage-`. Keep all copies. To restore the prior slot, move any current destination aside under a new name, then restore the matching complete backup to the original slot name. Do not merge files from different slots, delete unfamiliar stage directories or apply a generic cleanup command. If the correct complete backup is unclear, leave the copies intact for inspection.

Startup diagnostics are in `startup-status.txt`; game output appends to the versioned log under `home/baseq2/logs/`, named in `startup-status.txt`. No existing logs, personal settings or saved games belong in a distributed package. Keep fresh diagnostics if a launch fails. See [known limitations](KNOWN-ISSUES.md).
