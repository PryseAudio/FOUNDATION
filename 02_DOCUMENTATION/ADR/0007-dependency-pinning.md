# ADR-0007 — Dependency pinning

Status: **Accepted**

| Dependency | Version | Commit | Location | Licence |
|---|---|---|---|---|
| JUCE | 9.0.3 | `be29c81492b6151c8ea8d14c840e1311963b3a83` | submodule `External/JUCE` | JUCE 9 commercial EULA (approved) |
| Catch2 | v3.16.0 | `317ac1ed4c0bb6e6b91eafc817e05c488feffcb3` | submodule `External/Catch2` | BSL-1.0 (tests only, not shipped) |
| VST3 SDK | bundled in JUCE | — | inside JUCE | MIT (Steinberg, 2025) |
| pluginval | v1.0.4 | release binary | CI/local tool, not linked | GPLv3 (tool only, not distributed) |

## Decision

- Submodules (not configure-time download): reproducible, offline-capable, auditable.
- Both submodules are marked `shallow = true` in `.gitmodules`.
- `cmake/FoundationJUCE.cmake` and `cmake/FoundationTesting.cmake` verify the exact versions at
  configure time and fail on mismatch. `FOUNDATION_JUCE_PATH` may override the JUCE location for
  local experiments, but the version check still applies.
- Upgrading JUCE or Catch2 is a deliberate change, gated by the full test suite and (from Phase 1)
  golden renders.
- JUCE 9 provides `juce_audio_processors_headless` (no GUI dependency). DSP modules depend only on
  headless/DSP modules. `juce::AudioProcessorValueTreeState` lives in `juce_audio_processors` (which
  depends on `juce_gui_extra`), so `12_PARAMETERS_STATE` requires the full module.
