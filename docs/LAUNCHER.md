# Windows launcher

Start **Q2RTX VR.exe**. Its source is `launcher/launcher.c`; product and log strings derive from [VERSION](../VERSION).

The implementation finds its own executable directory, checks package and parent game files, constructs fully quoted Windows arguments and calls `CreateProcessW` with the full engine path and explicit child working directory. It clones the inherited environment, replacing only the seven existing SDL hints. The parent environment and persistent settings are unchanged. It uses no command shell, elevation, installer, updater, runtime selection or migration.

The two logging arguments select `console` followed by the product version from VERSION. The selected engine appends `.log` and uses append mode because `logfile` is 2. The startup report describes the version, passed file checks, launch intent and log location. Errors use a normal Windows message box. The launcher waits for the child, closes its handles and returns its exit code. Additional user arguments are not passed through.

## Actual-code CPU checks

`launcher/checks/checks.c` includes the real production C file. It substitutes only the Windows process/file/environment/UI boundary calls with in-process stand-ins; it does not copy the quoting or launcher logic into a second implementation. The tests cover quote/space/backslash/Unicode arguments, capacity refusal, local paths, exact startup arguments, environment preservation/overrides, missing files, unwritable report, short writes, process/wait/exit failures, handle cleanup and exit propagation. They never start the game, write a file, display a message box or contact a runtime.

From an x64 MSVC developer shell, the standalone checks can be built without the bulk engine sources:

```text
cmake -S launcher/checks -B build-launcher-checks -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_CONFIGURATION:STRING=Release
cmake --build build-launcher-checks
ctest --test-dir build-launcher-checks --output-on-failure
```

These checks use simulated Windows boundaries. They do not establish real process-start behaviour or headset compatibility. Rerun them when changing the launcher; actual product identities belong to the build receipt.
