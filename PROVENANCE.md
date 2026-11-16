# How Q2RTX VR was made

Togorian designed, directed and tested Q2RTX VR with AI coding assistance. OpenAI Codex wrote and revised original project code, build scripts and tools under his direction. ChatGPT helped plan features and generated the arrow images from his descriptions. Togorian used Claude, an AI model made by Anthropic, to audit the code, documentation and licensing questions independently of the tools that wrote it. The audits performed using Claude Code do not constitute legal advice, certification or endorsement by Anthropic.

Togorian came to the project with very limited prior game-programming experience: a few hobbyist Quake II tweaks in 2000. Human direction, AI-assisted implementation, separate review and repeated headset testing shaped the project. Togorian made the gameplay and comfort decisions, using feedback from actual play to refine controller behaviour, rendering artefacts and recovery from headset interruptions. See the [development story](README.md#made-with-ai).

## What it is built on

The engine derives from NVIDIA Quake II RTX 1.8.1, id Software's Quake II, Q2PRO by Andrey Nazarov and Q2VKPT by Christoph Schied, Florian Simon and Tobias Zirr. Applicable upstream notices remain. OpenXR provides the headset interface. Vulkan provides graphics interfaces. Other components and exact notice files are indexed in [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md). Players supply their own Quake II RTX installation and game data.

## Original VR work

The VR integration, controller deadzone, room-scale movement, physical crouch, teleport, comfort features, menus and launcher are original project work directed by Togorian and implemented with Codex. No other human authored those original VR additions. Room-scale stepping follows id Software's 18-unit step and 0.7 slope limit, with its snap-position fallback; duck compensation follows the 24-unit duck drop; teleport uses id Software's telefrag logic.

Applicable id Software notices remain on room-scale movement and teleport code, which follow its movement and telefrag logic. Physical crouch originated separately.

## Copyright and licence

Q2RTX VR as a whole is distributed under GNU GPL version 2. Copyright (C) 2026 Togorian applies to whatever rights he holds in his original contributions, offered under GPL version 2 or any later version. Inherited later-version permissions remain intact. This statement grants no rights over other parties' work or game data and makes no exclusive copyright claim over AI output. See [LICENSE.txt](LICENSE.txt).

Current Vulkan C headers are the MIT-offered vulkan-sdk-1.4.357.0 selection, commit e3b1eec08173d6b825cd3ac88c885a63b621504a. Eligible OpenXR 1.1.62 headers/loader use the offered MIT alternative; origin and verification limits remain documented in [the election](licences/openxr/LICENCE-ELECTION.md).

This SDL2 selection disables OpenGL ES and EGL while retaining desktop OpenGL/WGL and Vulkan. The Apache-only EGL and OpenGL ES headers remain in the vendored source with their licence notices; they are not compiled into the selected programs. The dependency selection and target include evidence are recorded in `recipes/sdl-build-validation.json`; the delivered `BUILD-RECORD.json` identifies the actual product build.

## Images

The raised and depressed stone-and-metal arrows were generated with ChatGPT from Togorian's descriptions on 8 October 2026; the depressed image was generated as an edit of the raised image. Their delivered PNG bytes and C2PA Content Credentials remain unchanged. Togorian makes no copyright claim to them. Any rights he holds are dedicated under CC0 1.0, including its fallback terms. See [the artwork record](assets/pics/b2/README.md) and [CC0 text](licences/CC0-1.0.txt).

## Changes to upstream files

A recorded path-by-path comparison with the exact official NVIDIA Q2RTX 1.8.1 archive is in [UPSTREAM-COMPARISON.md](docs/UPSTREAM-COMPARISON.md) and its machine-readable companion in corresponding source. The recorded classifications describe that historical comparison, not every subsequent modification. It distinguishes changed engine files, identical files, separately versioned dependencies and files with no counterpart. It records ancestry limitations rather than labelling every basename match as proof. Change notices retain original headers and terms.

The [targeted counterpart corrections](docs/UPSTREAM-COMPARISON.md#targeted-counterpart-corrections--13-november-2026) list all nine identified renamed shader derivatives and distinguish the curl header from the unrelated engine header. This completes that specific correction set; the surrounding comparison and its hashes retain their stated historical scope.
