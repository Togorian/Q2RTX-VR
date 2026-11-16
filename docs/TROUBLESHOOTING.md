# Troubleshooting

## Headset or runtime not found

Make sure the headset is connected and its normal OpenXR software is running. Select the matching route in [requirements](REQUIREMENTS.md) before starting **Q2RTX VR.exe**. The application does not install or select a runtime for you. Read `startup-status.txt` and the versioned game log under `home/baseq2/logs/` for the reported failure.

## Interrupted or ended session

After losing focus or removing the headset, return to the application, let tracking settle, release held controls and press the **Pause key on the keyboard** or use the displayed resume action. See the [controller guide](CONTROLLERS.md) for bindings. A short stale-input hold can recover automatically after fresh neutral frames; this differs from deliberate resume after focus/headset loss. If the OpenXR session has ended or the graphics device has failed, close and restart the application. Unsaved progress may be lost.

## Missing controller actions

The start panel identifies a missing supported profile or limited simple profile separately for each hand. Check controller connection and tracking in the active runtime. A missing hand does not disable the other hand's available actions. The simple profile provides only limited confirm/menu controls; it cannot supply full locomotion or weapon controls. Use keyboard/mouse when required controls or the resume binding are absent. **Pause** resumes from a hold and starts play from the **Preparing play** panel. **End** switches controllers / keyboard and mouse only after play has started. With no controllers, start a game from the menu using the keyboard, then press End. That input-mode choice is remembered at the next launch; saved key bindings can differ. Runtime-reserved system buttons are unavailable to the game.

## Low video memory or slow rendering

At low frame rates or under heavy rendering load, VR controller inputs can become unreliable and the weapon can disappear. This is an observed association, not a confirmed cause. Controls may recover when the workload falls, but recovery is not guaranteed. This symptom is separate from missing controller bindings and the dead-Makron muzzle obstruction described elsewhere on this page.

If possible, switch to keyboard/mouse: **End** does this by default, although saved bindings can differ. Save normally if possible, lower **Pending scale %** in **VR Graphics Settings**, then restart to apply it. If controls are unusable, close and relaunch the application, lower render scale from the main menu before starting or loading the campaign, then fully restart again before loading the campaign so the new scale takes effect. Closing can lose unsaved progress.

Lower render scale reduces application rendering resolution and memory demand; the default is 100%. Alternatively, reduce the external runtime's render quality/resolution or Quad Views rendering resolution through your own runtime software. Close other graphics-heavy applications if needed. An allocation/device failure may require restarting the game; reducing resolution does not guarantee recovery of an ended session.

## Locked settings, fullscreen and mirror size

Settings requiring unsupported renderer restarts are locked during VR. Vsync remains safe to change. Fullscreen is unsupported: VR uses a windowed desktop mirror. The mirror must have at least **320 × 240 pixels of client area and drawable area**. This is a mirror-window requirement, not a minimum resolution for the whole monitor or headset. Borders and display scaling can affect the usable area. Enlarge the window if it is too small. If VR continues after a mirror failure, restart the application to restore the mirror.

## Level loading pauses the headset

Level loading can pause headset updates while the map and graphics resources load. Wait for it to finish before continuing.

## Weapon disappears near the dead Makron

The tracked muzzle can be obstructed by the body, which hides the weapon and blocks firing. Walk clear of the body; movement remains available when its independent safety checks pass. Do not force firing through the obstruction.

## Tracked firing stops

If firing stops after touching an enemy or losing valid aim, move clear and let aiming recover. Fully release the trigger, then press it again. After an interruption, the chaingun, hyperblaster and railgun can take up to about 2 seconds to become ready while their normal wind-down runs.

## Keeping settings and saves during updates

Close the game and preserve the previous complete folder for rollback. For an update, keep your existing `home` and replace every file the new package ships under `home/baseq2`: `default-vr.cfg`, the other five supplied `.cfg` files, `shader_vkpt/` and `pics/`. Preserve `q2config.cfg`, the save folder, logs and player-authored startup files. Merge any personal additions to `autoexec.cfg` into its updated template rather than discarding them. The package never supplies a player `q2config.cfg`. Follow [installation and save recovery](INSTALLING.md), and keep earlier saves with the build that created them.

## Appearance settings during VR

Crosshair type, fonts, console background and player appearance are fixed during VR. Player Setup remains available for handedness, aiming point and view controls; its model and skin selectors are greyed. The normal white 3D aiming mark is independent of the stock crosshair type.

Your player model and skin can appear in ray-traced reflections. To change them, close the game and edit the existing line in `home/baseq2/q2config.cfg`, for example `seta skin "male/grunt"`. The value is `model/skin`; use an appearance present in your game data. The next startup applies that saved choice. Keep a backup and do not edit the file while the game is running. In-game appearance editing is deferred with multiplayer.
