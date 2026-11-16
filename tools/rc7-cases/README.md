# Focused RC7 CPU cases

From an x64 MSVC developer command prompt at the source root:

```
python -I -B -S tools/rc7-regression.py --output C:/scratch/rc7-checks
```

Choose an absent output directory outside the source tree. Add `--finding A1`
through `A8`, `R6` or `FINAL` to run one group. Add `--case R6-startup.c`
to select a case within that group. An unknown or empty selection fails. CMake
3.25 or newer must be on PATH for the startup case: it uses the actual product
version recipe and includes the actual launcher argument header. The runner verifies exact selected production
functions/spans against the retained cases before compiling. A changed span must
be reviewed and the fixture regenerated; a stale copied implementation refuses.
The A4 filter case excludes the separately tested A5 startup branch. It does not
claim full key-event, menu or repeated-key execution. A6 tests the actual parsed
command dispatch boundary; parsing and asset replacement are controlled seams.

XR, Vulkan and engine calls are inert controlled seams. A3 alone uses real Win32
file operations and junctions, confined to newly created synthetic output folders.
It never opens player saves. Generated cases, logs and synthetic data are retained
in the chosen output directory; nothing is cleaned or replaced automatically.
No engine product, shader build, game, headset or GPU is started. CPU results do
not qualify headset behaviour. A8 produces only a text README preview.

The fourteen `R6-*.c` cases in `R6.json` regenerate the retained RC6 desktop,
recovery, retirement, settings lock, startup, renderer/XR retirement, interruption,
configuration/bindings, save-slot and haptic checks against the selected source.
The original external effects and fixture declarations remain controlled seams;
source equality alone does not make a fixture a complete engine execution.
The startup case expands the real launcher header and version macros and passes
its arguments through the copied early-command function. Synthetic saved values
are replayed between its two calls; no player's configuration is opened.

`FINAL-settings.c` exercises the actual settings setter and crosshair callback,
including pre-arm values, same-value writes, blocked write sources and non-VR
changes. Its picture registry is a controlled cache and its repeated loc_draw
checks simulate continuing play; it does not load maps. `FINAL-player-selectors.c`
uses the actual dependency adapter and selector handlers with synthetic Player
Setup rows: model and skin stay fixed while handedness, aiming point and view
remain usable. Runtime, display and real level-transition verification still
require the headset smoke test. All committed cases run in the default selection.
