# Release notes

[VERSION](../VERSION) defines the product version. The packaged `RELEASE-MANIFEST.json` identifies the delivered files; `BUILD-RECORD.json` inside corresponding source records their build inputs and product identities.

## 0.8.4

This release corrects valid blocked-movement fallback and jumping from an overlapping collision boundary, including encounters with enemies near a shore. Holding jump while a map teleporter's movement timer is still active no longer causes a false vertical movement safety stop. The release also removes the short publication-counter limit during a long stay in one level and refreshes visual history when a new authenticated gameplay update follows a long delivery gap. Movement bounds, collision checks, publication freshness and input ownership remain enforced.

An emitted tracked shot now keeps its remaining weapon recovery when firing authority is interrupted. Once the muzzle is clear and aim is valid, fully release the trigger, then press it again. After an interruption, the chaingun, hyperblaster and railgun can take up to about 2 seconds to become ready while their normal wind-down runs.

When the game refuses VR during startup before the session begins, the error message now directs you to `startup-status.txt` to identify the problem before relaunching. During play, internal safety stops explain why the game stopped and advise restarting and loading a save. That explanation takes precedence over headset-loss advice if the runtime subsequently exits. New Game is available after the campaign's ending film or victory picture.

The current implementation provides OpenXR stereo and compatible Quad Views rendering, tracked controllers, room-scale movement, physical crouch, teleport, controller menus and a desktop mirror. Each hand's available actions and tracking are handled independently. Missing or limited controller profiles are shown on the start panel. Focus loss and headset removal use deliberate resume; transient input holds can recover after fresh neutral frames.

Quad Views was a central design goal for this path-traced VR adaptation. It renders a focus view and a peripheral view for each eye, with fixed or eye-tracked foveation. Eye-tracked Quad Views implements dynamic foveated rendering (DFR). Denoising, temporal history and focus-to-periphery transitions were refined to support a more consistent image. Hardware, runtime and settings determine feature availability, performance and image quality; see [requirements](REQUIREMENTS.md).

Clean/reset defaults select controllers, right-handed with Togorian's preferred bindings, smooth turning at 360 degrees/second and teleport-only vignette. Optional 45-degree snap turning and movement/turn vignette remain available. Immutable defaults load before the player's saved preferences, including compatible older partial configuration overlays. Ordinary startup does not reset saved bindings. Artificial walk bob and landing dip are omitted from the VR eye offset while stance and kick are retained.

Movement can continue safely away from an obstructed tracked muzzle, including near the dead Makron. Weapon display/firing remains suppressed while obstructed. Save handling validates complete staged slots, retains backups and reports actual manual/autosave outcomes.

Graphics settings that require unsupported restarts are locked in VR. Vsync remains usable; the mirror is windowed. Renderer history and resource maintenance operate independently of diagnostic capture code. External networking, downloads, demos and recording are disabled while engine-local single-player transitions remain available.

Minimising the desktop window in keyboard-and-mouse mode keeps the paused headset view responding to head movement while tracking remains available. Gameplay stays paused until focus is restored and the player deliberately resumes. Headset tracking and runtime rendering safeguards remain enforced.

The native Windows launcher checks required files, supplies the runtime's startup arguments and records startup diagnostics. Build, shader and packaging recipes are included in corresponding source. See [known limitations](KNOWN-ISSUES.md), [controllers](CONTROLLERS.md), [installation](INSTALLING.md) and [provenance](../PROVENANCE.md).

Togorian came to the project with very limited prior game-programming experience: a few hobbyist Quake II tweaks in 2000. Togorian directed the project and tested successive builds in headsets; Codex implemented and revised original VR code and tools, ChatGPT supported ideas and planning, and Claude provided separate audits. See the [development story](../README.md#made-with-ai) and the role and licensing qualifications in [provenance](../PROVENANCE.md).
