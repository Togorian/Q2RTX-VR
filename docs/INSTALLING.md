# Installing Q2RTX VR

Q2RTX VR runs from its own folder inside your existing **Quake II RTX** installation. It uses the game data already installed there and keeps its VR settings and saves separate.

Installing for the first time? Follow the steps below. If you already have Q2RTX VR installed, use [Updating Q2RTX VR](#updating-q2rtx-vr).

## Before you start

You need:

- A Windows x64 PC capable of running Quake II RTX, including a Vulkan ray-tracing-capable GPU.
- A compatible headset with its OpenXR runtime installed and working. See [requirements and supported headset routes](REQUIREMENTS.md).
- **The full original Quake II game data installed with Quake II RTX. The free three-level demo alone is not supported.** Game data is not included with this mod.

If you have only installed the demo, run the Quake II RTX setup program again. For a Steam installation, look in the `setup` folder inside your Quake II RTX installation.

On Steam, you can also use **Properties → Installed Files → Verify integrity of game files**, then launch the normal (non-VR) Quake II RTX to bring up the setup choice. This route has been tested during development. If the setup choice does not appear, run the setup program directly.

Select **Quake II Full Game**, and point it to your installed copy of the **original Quake II**, not to the Quake II RTX folder. NVIDIA's [installation guide and FAQ](https://store.steampowered.com/news/posts/?appgroupname=Quake+II+RTX&appids=1089130&enddate=1560965154&feed=steam_community_announcements) also describes copying the files manually.

Check that Quake II RTX runs normally with your full-game data before adding the VR mod.

## First installation

1. **Download the playable release.** Open the [latest release](https://github.com/Togorian/Q2RTX-VR/releases/latest) and download the versioned runtime ZIP and its matching `.sha256` file from **Assets**. For example, version 0.8.4 uses `Q2RTX-VR-v0.8.4.zip`. Follow the checksum instructions on that release page to verify the download. The source-code archives are for building the software, not playing it.

2. **Close Quake II RTX and extract the complete `Q2RTX VR` folder into its installation folder.** Place it alongside the existing `baseq2` folder. The launcher should be at:

   ```text
   Quake II RTX\Q2RTX VR\Q2RTX VR.exe
   ```

   Avoid an extra folder between `Quake II RTX` and `Q2RTX VR`, or a second `Q2RTX VR` folder inside the first. Keep the entire supplied folder together, including its `home` folder; moving only the executable will not install the mod correctly.

   Your existing `Quake II RTX\baseq2` folder must contain all five files: `pak0.pak`, `pak1.pak`, `pak2.pak`, `q2rtx_media.pkz` and `blue_noise.pkz`. Leave these files, films, music and other game data in the original installation. Use the new package's clean `home` folder without importing other settings or saves.

3. **Connect your headset and start its OpenXR software.** Select the runtime for your headset before launching the mod. For **Pimax Dream Air**, start **Pimax EVO** and use **Pimax OpenXR**. Other tested routes are listed in [requirements](REQUIREMENTS.md). Q2RTX VR does not install or select a runtime for you.

4. **Run `Q2RTX VR.exe` from inside the `Q2RTX VR` folder.** The same launcher serves all supported headset routes. You must be able to save files in this folder so the game can store preferences, saves and diagnostics.

## Your first session

**VR comfort and safety:** Clear obstacles from your play space and keep people and pets out of reach. Follow your headset manufacturer's safety guidance and use its boundary system where available. Take regular breaks and stop playing if you feel unwell or uncomfortable.

The default controls use motion controllers, right-handed, with Togorian's preferred bindings. Useful keyboard shortcuts are:

| Key | Action |
| --- | --- |
| **Home** | Recentre the view. |
| **End** | Switch between controllers and keyboard/mouse after play has started. |

You can rebind these in **Options → Action Key Bindings**. If you are playing without controllers, start a game from the menu using the keyboard, then press **End**. See the [controller guide](CONTROLLERS.md) for the full mappings and [troubleshooting](TROUBLESHOOTING.md) if input is unavailable.

The initial comfort and graphics settings are:

| Setting | Default |
| --- | --- |
| Turning | Smooth turning at **360 degrees/second**; 45-degree snap turning is available. |
| Vignette | **Teleport only**. |
| Application render scale | **100%**. |
| View mode and foveation | Automatic requests, subject to headset and runtime support. |

Quad Views and eye tracking depend on compatible hardware and runtime support. Automatic requests do not guarantee that either feature is active. Your OpenXR runtime also controls its own rendering resolution. Graphics settings marked for the next launch require a **full game restart**.

## Updating Q2RTX VR

**Keep a complete copy of your current VR folder before updating.** This preserves the build, settings and saves you may need for rollback.

1. Close the game. Copy the entire existing `Q2RTX VR` folder somewhere safe.
2. Download and verify the new runtime release, then extract it to a separate temporary location.
3. Update the runtime files in your installed `Q2RTX VR` folder with those from the new package. Keep your existing `home` folder, and update its supplied files as described below.
4. In `home/baseq2`, replace the package-supplied configuration files (`default-vr.cfg` and the other five supplied `.cfg` files), and update the supplied files in `shader_vkpt/` and `pics/`. If you have customised `autoexec.cfg`, merge your additions **from the backup** into the **new template** rather than replacing it with your old version.
5. Preserve your `q2config.cfg`, save folder, logs and player-authored startup files. The release package does not supply a player `q2config.cfg`.

Replacing only the executable can leave incompatible shaders, defaults or libraries. Copying an old `home` folder over a new installation can also undo required updates to the supplied files.

Keep earlier saves with the complete build that created them. This save format has no old-save importer or backward exporter. To roll back, close the game, move the newer VR folder somewhere safe, and restore the complete older folder to `Quake II RTX\Q2RTX VR`. Keep the two versions separate; do not merge their `home` folders. Launch the restored copy from inside the Quake II RTX installation, not directly from a backup stored elsewhere.

## Settings and save locations

VR settings and saves live under `Q2RTX VR\home`. The mod does not migrate settings or saves from the original flat-game installation. It does not change drivers or request administrator privileges.

Defaults load first, followed by preferences saved in the VR `home`. The game creates or updates its own `home/baseq2/q2config.cfg` when saving configuration. Your saved preferences take precedence on later launches.

**Reset to Defaults** restores the documented comfort and graphics defaults; it does not reset controller bindings. Startup also leaves your saved bindings intact. Only use the supplied optional binding-reset configuration if you intend to restore the documented bindings.

<details>
<summary>Custom configuration files</summary>

The package supplies `default-vr.cfg`, not a player `q2config.cfg`. The parent flat-game `q2config.cfg` is not inherited. Generated saved configurations retain deliberate unbindings. Partial templates and custom configuration scripts overlay the defaults.

</details>

## If the game will not start

Read `startup-status.txt`, which is generated beside `Q2RTX VR.exe`. It identifies the versioned game log under `home/baseq2/logs/`. Keep the fresh diagnostics when reporting a problem.

See [troubleshooting](TROUBLESHOOTING.md) and [known limitations](KNOWN-ISSUES.md) for runtime, controller, performance and session-recovery guidance.

## Recovering an interrupted save

Keep backups of important saves. If a power cut or system crash interrupts saving, recovery cannot be guaranteed.

1. Close the game and copy the **entire VR `home` folder** somewhere safe before attempting recovery.
2. Find the exact paths reported in the log. The backup ends in `.old`; the related staged-save name contains `.b2-stage-`. Keep all copies.
3. To restore the previous slot, move any current destination aside under a new name, then restore the **matching complete backup** to the original slot name.

Do not merge files from different save slots, delete unfamiliar stage directories or run a generic cleanup command. If you cannot identify the correct complete backup, leave all copies intact for inspection.

<details>
<summary>How save recovery works</summary>

On Windows, the game prepares and validates a complete staged save slot before replacing the destination. It moves the previous slot to a unique backup, then moves the staged slot into place. If a move fails, it attempts rollback. If rollback is not possible, the complete stage and previous backup remain for inspection.

</details>

## Artwork and redistribution notes

The two menu arrows in `home/baseq2/pics/b2/` were generated with ChatGPT from Togorian's prompts. Rights he holds in them are offered under CC0 1.0 Universal. See the [artwork record](../assets/pics/b2/README.md) and [complete CC0 text](../licences/CC0-1.0.txt) for provenance and terms.

If preparing a package for distribution, do not include personal settings, saved games or existing logs.
