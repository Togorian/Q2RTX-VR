# Known limitations

- Loading a level can pause headset updates while the map and graphics resources load. Wait for loading to finish.
- Near the dead Makron, the tracked weapon can disappear when its muzzle is obstructed. Walk clear of the body. Movement remains available when its independent safety checks pass; firing remains blocked while the muzzle is obstructed.
- If tracked firing stops after touching an enemy or losing valid aim, move clear and let aiming recover. Fully release the trigger, then press it again. After an interruption, the chaingun, hyperblaster and railgun can take up to about 2 seconds to become ready while their normal wind-down runs.
- A brief stale-input hold can recover once fresh neutral frames return. Release controls and let tracking settle. Focus loss or headset removal requires deliberate resume after returning; an ended session requires restarting the application. Unsaved progress can be lost after session or device failure.
- Controller actions depend on each hand's active runtime profile and tracking. The simple profile is a limited fallback; use keyboard/mouse where required controls are missing. Runtime-reserved system buttons remain reserved.
- The desktop mirror is windowed. Fullscreen is unsupported, and settings requiring unsupported renderer restarts are locked during VR. Vsync remains available. If the mirror fails while VR continues, restart the application to restore the mirror.
- Keep backups of important saves. If saving is interrupted by a power cut or system crash, follow the included save-recovery instructions; recovery cannot be guaranteed. See [save recovery](INSTALLING.md#saves-and-recovery); do not merge slots or remove unfamiliar stage directories.
- The developer commands `mat`, `gun_model` and `skins` are refused during VR. The location-marker display (`loc_draw`) is disabled.
- Local single-player is supported. Remote play, downloads, demos and recording are disabled. See [single-player boundaries](SINGLE-PLAYER.md).
- Teleporting with jump held can omit the teleport sound and particles.
- Hand-editing configuration files can leave the headset blank. Close the game and restore the shipped configuration files, preserving your personal additions separately.
- If the headset connection drops at an unlucky moment, the game can close with an error box instead of the usual session-ended message. Unsaved progress is lost either way.
- A pause labelled “Resumes automatically” resumes by itself only with both controllers tracked. Press the **Pause key on the keyboard** or the resume button to continue.
- Switching a controller off or on pauses the game; press Pause or the resume button to continue.
- Meta Horizon Link (Quest 3) is not supported yet. The game runs, but the controllers give no input. Use Virtual Desktop, or keyboard and mouse.

Keep each earlier complete installation with the saves it created. Updates must preserve the player's home, settings and saves. See [installation](INSTALLING.md), [requirements](REQUIREMENTS.md) and [controllers](CONTROLLERS.md).

See [troubleshooting](TROUBLESHOOTING.md) for common problems and recovery.
