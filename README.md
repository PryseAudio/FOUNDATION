# FOUNDATION

Premium bass sound-design instrument.

## Structure
- `01_REFERENCES/` — PLAY and EDIT visual references (never compiled or shipped)
- `02_DOCUMENTATION/` — rules, master specification, architecture (`04_ARCHITECTURE.md`), decision records (`ADR/`)
- `03_SOURCE/` — **sound sources**: oscillators, wavetables, unison, SUB/808, organ, sample, noise, FM/AM/ring (not "source code")
- `04_DSP/` — `Core/` DSP primitives and real-time utilities; `Engine/` audio and voice engine
- `05_MODULATION/` — envelopes, LFOs, matrix, macros
- `06_FX/` — performance stages, effects, stereo, output and low-end protection
- `07_PRESETS/` — preset system (`Source/`) and factory preset data (`Factory/`)
- `08_UI/` — PLAY and EDIT UI (`Common/`, `PLAY/`, `EDIT/`)
- `09_TESTS/` — tests, golden data, fixtures, benchmarks, tools (RenderCLI)
- `10_BUILD/` — build/CI scripts and notes only (generated output lives in `../FOUNDATION_build/`)
- `11_PLUGIN/` — plugin shell: processor, editor, plugin entry
- `12_PARAMETERS_STATE/` — parameter registry, host parameter layout, state serialisation
- `13_ANALYSIS/` — spectrum, loudness, meters, stereo and bass-band analysis
- `Assets/` — shared production assets (supplied files are never modified)
- `cmake/`, `External/` — build helpers; pinned JUCE 9.0.3 and Catch2 v3.16.0 submodules

Module folders gain files in the phase that gives them real code. Build instructions: `10_BUILD/README.md`.

PLAY = simple, beautiful, immediate, musical, approachable.
EDIT = deep, technical, powerful, flexible, limitless.

PLAY is already designed and must not be redesigned.
