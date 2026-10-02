# FOUNDATION — build

Generated output never lives in the source tree. Every preset builds into
`../FOUNDATION_build/<preset>` (for this machine: `C:\Dev\FOUNDATION_build\<preset>`).

## Requirements

| Platform | Toolchain |
|---|---|
| Windows | Visual Studio 2026 (v18) with "Desktop development with C++", CMake ≥ 3.25 |
| macOS | Xcode (universal arm64 + x86_64; AU), CMake ≥ 3.25 |

Dependencies are pinned git submodules (ADR-0007): JUCE 9.0.3, Catch2 v3.16.0.

```
git submodule update --init --recursive
```

## Build and test

```
cmake --preset windows-vs2026
cmake --build --preset windows-debug --parallel
ctest --preset windows-debug
```

Configurations: `windows-debug`, `windows-relwithdebinfo`, `windows-release`
(`macos-*` and `ci-*` equivalents exist).

Or use the scripts:

| Script | Purpose |
|---|---|
| `scripts/build.ps1` / `scripts/build.sh` | configure + build (+ test) one configuration |
| `scripts/run_pluginval.ps1` / `scripts/run_pluginval.sh` | validate the built VST3 (and AU on macOS) with pluginval |
| `scripts/ci_local.ps1` | the CI pipeline locally: configure, build Debug + Release, test, pluginval if available |

## Outputs (Windows, per configuration `<cfg>`)

| Artefact | Location under `FOUNDATION_build/windows-vs2026/` |
|---|---|
| VST3 | `11_PLUGIN/Foundation_artefacts/<cfg>/VST3/FOUNDATION.vst3` |
| Standalone | `11_PLUGIN/Foundation_artefacts/<cfg>/Standalone/FOUNDATION.exe` |
| Test runner | `09_TESTS/foundation_tests_artefacts/<cfg>/foundation_tests.exe` |
| RenderCLI | `09_TESTS/Tools/RenderCLI/foundation_render_cli_artefacts/<cfg>/foundation_render_cli.exe` |

`FOUNDATION_COPY_PLUGIN` is OFF by default (copying into the system VST3 folder needs admin rights
on Windows).

## pluginval

pluginval (Tracktion, GPLv3) is a validation tool only; it is never linked or distributed. The scripts
look for it in `FOUNDATION_PLUGINVAL`, then `PATH`, then `10_BUILD/tools/` (git-ignored).

## Plugin identity

All identity values in `cmake/FoundationIdentity.cmake` are placeholders. Replace them before any
build leaves the development machine.
