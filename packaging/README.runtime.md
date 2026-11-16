# Q2RTX VR @Q2RTX_TAG@

Q2RTX VR brings Quake II RTX's single-player campaign to virtual reality through OpenXR. Designed and directed by Togorian with AI coding assistance. Q2RTX VR is an unofficial, fan-made modification by Togorian. It is not affiliated with, sponsored by or endorsed by NVIDIA, id Software, Bethesda Softworks or ZeniMax, or by any headset or runtime provider. All third-party names and trademarks remain the property of their respective owners. Quake is a trademark of its owner. You need the full Quake II game data installed with Quake II RTX to play. The free three-level demo alone is not supported.

Place the complete **Q2RTX VR** folder directly inside your owned **Quake II RTX** installation and start **Q2RTX VR.exe**. Keep original game data in the parent. When updating, preserve the previous complete folder for rollback and retain your existing `home`, preferences and saves. For an update, keep your existing `home` and replace every file the new package ships under `home/baseq2`: `default-vr.cfg`, the other five supplied `.cfg` files, `shader_vkpt/` and `pics/`. Preserve `q2config.cfg`, the save folder, logs and player-authored startup files. Merge any personal additions to `autoexec.cfg` into its updated template rather than discarding them. The package never supplies a player `q2config.cfg`. See [installation and recovery](docs/INSTALLING.md).

**VR comfort and safety:** Clear obstacles from your play space and keep people and pets out of reach. Follow your headset manufacturer's safety guidance and use its boundary system where available. Take regular breaks and stop playing if you feel unwell or uncomfortable.

Defaults are controllers, right-handed with Togorian's preferred bindings, smooth turning at 360 degrees/second, optional 45-degree snap turning, teleport-only vignette and 100% application render scale. Existing saved preferences take precedence; Reset to Defaults restores comfort and graphics settings; it does not reset controller bindings. External networking and downloads are disabled. Select your normal active OpenXR runtime before launch; the application does not install/select runtimes or change drivers.

## Quad Views and foveated rendering

Quad Views was a central design goal of Q2RTX VR. Quake II RTX's path-traced lighting makes it a demanding foundation for high-resolution virtual reality—and a compelling application for eye-tracked foveated rendering.

On compatible hardware and OpenXR runtimes, Quad Views combines higher-resolution focus views with lower-resolution peripheral views. With eye tracking, those detailed regions follow the player's gaze, reducing rendering workload while concentrating image quality where they look. This can provide a valuable balance between clarity and performance in a renderer where rendering every part of both eyes at high resolution can be expensive.

Making that combination work well involved more than enabling four views. Development included improvements to denoising, temporal history and focus-to-periphery transitions, helping the different views form a more consistent image. Fixed-foveation and standard stereo rendering are also supported; performance and image quality depend on hardware and settings.

Quad Views renders four views: one focus view and one peripheral view for each eye. It can use fixed or eye-tracked foveation. Eye-tracked Quad Views implements dynamic foveated rendering (DFR).

Eye tracking and Quad Views require compatible hardware and runtime support; the supported headset routes below do not imply that every route provides either feature.

## Made with AI

Q2RTX VR is also an exploration of what human direction and AI-assisted development can achieve together. Togorian came to the project with very limited prior game-programming experience: a few hobbyist Quake II tweaks in 2000. He shaped the project's goals, made its gameplay and comfort decisions, and tested successive builds in real headsets. OpenAI Codex wrote and revised the original VR code, build scripts and tools under that direction; ChatGPT helped develop ideas and plan features. Claude provided separate audits of the code, documentation and licensing questions.

The work developed through an ongoing cycle of ideas, implementation, review and hands-on testing. Controller behaviour, rendering artefacts, comfort settings and recovery from headset interruptions were refined through repeated feedback from actual play. AI assistance was a substantial part of that engineering process, alongside human judgement about how the game should feel and behave.

This project builds on the substantial open-source foundations of Quake II, Q2PRO, Q2VKPT and NVIDIA's Quake II RTX. It aims to demonstrate how those foundations, modern VR techniques and sustained collaboration with AI tools can help an individual creator undertake an ambitious adaptation—and share the result for others to play, study and build upon.

The audits performed using Claude Code do not constitute legal advice, certification or endorsement by Anthropic. See [provenance](PROVENANCE.md), [third-party notices](THIRD-PARTY-NOTICES.md), [licensing](LICENSE.txt) and [notice roles](NOTICE.md). The combined engine retains its inherited GPL version 2 distribution basis. Rights Togorian holds in his own contributions are offered under GPL version 2 or later; this does not relicense upstream work or game data.

## Player guides

Read [requirements](docs/REQUIREMENTS.md), [controllers](docs/CONTROLLERS.md), [limitations](docs/KNOWN-ISSUES.md) and [release notes](docs/RELEASE-NOTES.md). Startup output is `startup-status.txt`; game output appends to `home/baseq2/logs/@Q2RTX_LOG_FILE@`.

Arrows under `home/baseq2/pics/b2/` were generated with ChatGPT from Togorian prompts. CC0 covers rights he holds. Depressed was generated as an edit of raised; PNG bytes/metadata remain unchanged. See the [artwork record](assets/pics/b2/README.md) and [CC0 terms](licences/CC0-1.0.txt).

[Provenance](PROVENANCE.md), [third-party notices](THIRD-PARTY-NOTICES.md), [licensing](LICENSE.txt) and [notices](NOTICE.md) accompany the [corresponding source/relink material](corresponding-source-@Q2RTX_TAG@.zip). Actual members/hashes and scoped privacy evidence are in `RELEASE-MANIFEST.json` and `SHA256SUMS.txt`. Build instructions apply inside extracted source; see [package layout](docs/PACKAGING.md).

See [troubleshooting](docs/TROUBLESHOOTING.md) for common problems and recovery.

## Supported headset routes

| Headset | OpenXR runtime route |
|---|---|
| Pimax Dream Air | Pimax OpenXR |
| Sony PS VR2 | SteamVR |
| Meta Quest 3 | Virtual Desktop VDXR |

For Pimax Dream Air, install and run Pimax EVO on the PC and use Pimax OpenXR. For Sony PS VR2, install the PlayStation VR2 PC app and SteamVR, and use SteamVR as the OpenXR runtime. For Meta Quest 3, use Virtual Desktop on the headset and Virtual Desktop Streamer on the PC, with VDXR selected for OpenXR.

Controller profiles and available features still depend on the runtime; this list does not establish compatibility with other routes.
