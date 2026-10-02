# ADR-0008 — Project structure and module build mechanism

Status: **Accepted** for the folder structure. **Pending owner sign-off** for the build mechanism
change described below (implemented in Phase 0).

## Folder structure

The numbered organisation is preserved. `03_SOURCE` means **sound sources** (oscillators, organ,
SUB/808, sample, noise, FM), never "source code". New folders: `11_PLUGIN` (plugin shell),
`12_PARAMETERS_STATE` (registry, layout, state), `13_ANALYSIS`. Build tooling lives in `cmake/`,
pinned dependencies in `External/`. `10_BUILD` holds scripts and notes only. Generated output always
goes to `../FOUNDATION_build/<preset>`.

A module folder gains a `CMakeLists.txt` and sources only in the phase that gives it real code; no
placeholder sources exist.

## Build mechanism (deviation from the Step 2 proposal)

The Step 2 proposal described a `foundation_juce` static library compiling JUCE once, plus one
separately compiled static library per module. Inspection of JUCE 9.0.3 showed this is unsafe:
`juce_add_plugin` always links `juce_audio_plugin_client` (and through it the JUCE modules) into the
plugin's own shared-code target. A separately compiled JUCE would be built into the same binary
twice, and static module libraries compiled against different JUCE configuration macros would give
one binary inconsistent definitions of JUCE classes (duplicate symbols or silent ODR violations).

Implemented instead (`cmake/FoundationModule.cmake`):

- Each module is a CMake **interface source library** (`foundation_add_module`). Its sources compile
  inside each final target that links it — plugin shared code, test runner, tools — so every
  translation unit in a binary sees identical JUCE configuration.
- JUCE modules are linked `PRIVATE` into each final target exactly once (JUCE's documented rule).
- The dependency direction between modules is unchanged and enforced: a module can only include
  headers of modules it `DEPENDS` on.
- Cost: FOUNDATION sources compile once per final target (plugin, tests, RenderCLI).

Strict warnings (`/W4 /WX`, external headers silenced) apply to FOUNDATION-owned sources only,
attached as per-source compile options in each consumer target's scope.
