# Requirements and compatibility

Use Windows x64, a Vulkan ray-tracing-capable system able to run Quake II RTX, an active compatible OpenXR runtime/headset and your complete, unmodified full Quake II game data installed with Quake II RTX. The free three-level demo alone is not supported.

Put the runtime folder directly inside the owned installation. The parent `baseq2` must contain `pak0.pak`, `pak1.pak`, `pak2.pak`, `q2rtx_media.pkz` and `blue_noise.pkz`. The VR folder needs ordinary write access for preferences, saves and diagnostics. See [installation and recovery](INSTALLING.md).

Select the normal active OpenXR runtime before launch. The application does not install/select runtimes or change drivers. Controllers are the default; End switches controllers/keyboard and mouse, and Home recentres, subject to saved bindings.

Controller profiles, available buttons, view modes and foveation depend on the active runtime. The [controller guide](CONTROLLERS.md) documents application mappings. Runtime-reserved system buttons remain reserved. See [known limitations](KNOWN-ISSUES.md).

Quad Views renders four views: one focus view and one peripheral view for each eye. It can use fixed or eye-tracked foveation. Eye-tracked Quad Views implements dynamic foveated rendering (DFR). Eye tracking requires compatible hardware and runtime support. Standard stereo and fixed-foveation modes are also supported. The routes below do not imply that every headset/runtime combination provides Quad Views or eye tracking; performance and image quality depend on hardware and settings.

## Supported headset routes

| Headset | OpenXR runtime route |
|---|---|
| Pimax Dream Air | Pimax OpenXR |
| Sony PS VR2 | SteamVR |
| Meta Quest 3 | Virtual Desktop VDXR |

For Pimax Dream Air, install and run Pimax EVO on the PC and use Pimax OpenXR. For Sony PS VR2, install the PlayStation VR2 PC app and SteamVR, and use SteamVR as OpenXR runtime. For Meta Quest 3, use Virtual Desktop on the headset and Virtual Desktop Streamer on the PC, with VDXR selected for OpenXR. Controller profiles and available features still depend on the runtime; this list does not establish compatibility with other routes.


See [troubleshooting](TROUBLESHOOTING.md) for common problems and recovery.

The inherited engine enforces driver floors of NVIDIA 460.82 and AMD 21.1.1. These are minimum engine checks, not a recommendation for those old driver versions or a guarantee of VR compatibility; your headset and runtime can require newer drivers.
