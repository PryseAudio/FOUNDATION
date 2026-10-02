# FOUNDATION — ARCHITECTURE (approved baseline)

Status: **approved** (Step 2) with the Step 2 lock amendments applied. Decision records: `ADR/`.
Authoritative requirements: `01_NON_NEGOTIABLE.md`, `02_MASTER_SPEC.md`. Where this document and the
master spec disagree, the master spec wins; raise the conflict rather than resolving it silently.

PLAY is the finished visual identity and the default landing page. EDIT is the deep sound-design
environment. Both are views over one engine, one parameter system, one modulation system and one
state system.

---

## 0. Locked decisions

| # | Decision | Record |
|---|---|---|
| 1 | Knob ring visual mapping: 0 % = 7:00, 25 % = 10:00, 50 % = 12:00, 75 % = 2:00, 100 % = 5:00, exact. Deliberate non-linear UI mapping. | ADR-0001 |
| 2 | PLAY artwork coordinate space 1983 × 793 is authoritative for both pages; the window never resizes on page switch. | ADR-0002 |
| 3 | Mandatory PLAY rendering proof in Phase 1 (technical validation only). | ADR-0002 |
| 4 | Oversampling: architecture retained, final strategy decided from Phase 1/3/5 measurements. | ADR-0005 |
| 5 | EDIT is a native JUCE reconstruction: six pages, Simple / Advanced / Expert. | — |
| 6 | SUB = Macro 1, BODY = 2, TEXTURE = 3, MOVEMENT = 4, WIDTH = 5, OUTPUT = 6; Macros 7–8 EDIT-only. No special-case PLAY DSP. | ADR-0004 |
| 7 | Mode groups are real engine parameters; defaults CLEAN / STEREO / SMOOTH; stages kept modular for tuning. | — |
| 8 | 808 is a SUB engine type. | — |
| 9 | 64-slot fixed modulation route pool; no dynamic host parameters. | ADR-0004 |
| 10 | Versioned state; exact recall across plugin versions is a hard requirement. | ADR-0006 |
| 11 | Organ synthesised by default; no copyrighted M1 samples; licensed content later via content IDs. | — |
| 12 | Favourite-filled heart drawn natively over the printed outline; no hover visuals on PLAY (cursor only). | — |
| 13 | No placeholder UI: no editor until Phase 9; host generic parameter view, RenderCLI and tests meanwhile. | — |
| 14 | A/B stored in session state; preset files store the active sound only. | ADR-0006 |

---

## A. Project architecture

### A1. Folders (ADR-0008)

```
CMakeLists.txt / CMakePresets.json / cmake/      build orchestration, identity, JUCE pin, compiler settings
External/JUCE, External/Catch2                    pinned git submodules
01_REFERENCES/        design references — never compiled or shipped
02_DOCUMENTATION/     specs, this document, ADRs
03_SOURCE/            SOUND SOURCES: oscillators, wavetables, unison, SUB/808, organ, sample, noise, FM/AM/ring
04_DSP/Core/          DSP primitives + real-time utilities
04_DSP/Engine/        AudioEngine, VoiceManager, Voice, NoteStack, Glide, MidiRouter, Mixer, FilterSection, ShapeSection
05_MODULATION/        envelopes, LFOs, sources, matrix, RouteTable, macros, audio follower
06_FX/                Performance stages (Character/Response), FX, FxChain, Stereo, Output, LowEndGuard
07_PRESETS/Source     library, preset files, metadata, favourites, randomiser, A/B, migration
07_PRESETS/Factory    factory preset data
08_UI/Common|PLAY|EDIT  theme, typography, look-and-feel, controls; PLAY rendering; EDIT pages
09_TESTS/             unit, DSP, state, realtime, automation, UI, golden, fixtures, benchmarks, tools
10_BUILD/             build/CI/packaging scripts and notes only — never generated output
11_PLUGIN/            processor, editor shell, bus layout, plugin entry
12_PARAMETERS_STATE/  parameter registry, IDs, layout, state model, serialiser, undo
13_ANALYSIS/          analysis taps, spectrum, loudness, meters, stereo, bass bands, harmonics
Assets/               production assets (supplied files are never modified)
```

Build output always lives outside the source tree: `../FOUNDATION_build/<preset>`.

### A2. Module dependency direction

```
foundation_dsp_core (04/Core)
   ▲
foundation_params_state (12)
   ▲
foundation_sources (03) · foundation_modulation (05) · foundation_fx (06) · foundation_analysis (13)
   ▲
foundation_engine (04/Engine)
   ▲
foundation_presets (07)
   ▲
foundation_ui (08) — reads parameters / presets / analysis snapshots, never engine internals
   ▲
foundation_processor + Foundation plugin (11) → VST3 / AU / Standalone
```

Rules: DSP and engine code never include `juce_gui_*`. The UI never touches engine internals. Only
`11_PLUGIN` sees `JucePlugin_*` macros.

**Build mechanism (ADR-0008, implemented in Phase 0):** each module is a CMake *interface source
library* (`cmake/FoundationModule.cmake`): its sources compile inside each final target that links
it (plugin shared code, test runner, tools). `juce_add_plugin` always compiles JUCE into the plugin's
own shared-code target, so a separately compiled JUCE or static module libraries built against
different JUCE configuration would place duplicate or inconsistent JUCE definitions in one binary.

### A3. Spec module map

| Spec module | Location / main classes |
|---|---|
| Audio Engine | `04/Engine/AudioEngine` |
| Voice Engine | `04/Engine/VoiceManager`, `Voice`, `NoteStack`, `Glide` |
| Oscillator Engine | `03/Oscillator/{Oscillator, WavetableBank, WavetableGenerator, UnisonStack, VAShapes}` |
| Organ Engine | `03/Organ/{OrganEngine, Drawbars, Percussion, KeyClick, Leakage}` (synthesised) |
| 808 Engine | `03/Sub/{SubEngine, Engine808}` (SUB type) |
| FM Engine | `03/FM/{FMMatrix, PhaseModulator, RingAM}` |
| Filter Engine | `04/Core/Filters/*` + `04/Engine/FilterSection` |
| Harmonic Engine | `04/Core/Saturation/*` + `04/Engine/ShapeSection` |
| Modulation / Matrix / Macros | `05/*` |
| FX Engine | `06/Fx/*`, `06/FxChain` |
| Stereo Engine | `06/Stereo/{StereoEngine, BandWidth, MonoBelow}` |
| Analysis Engine | `13/*` |
| Preset / State | `07/Source/*` + `12/*` |
| UI PLAY / EDIT | `08/PLAY`, `08/EDIT`, `08/Common` |

---

## B. DSP architecture

```
HOST MIDI ─► MidiRouter ─► sub-block splitter (MIDI event times + 32-sample control ticks)
                               ▼
                         VoiceManager ──► Voice[0 .. voiceLimit)
GLOBAL MOD SOURCES (macros, global LFOs, wheel, channel AT, bend, follower, global random, perf modes)

PER VOICE — internal rate = host rate × OS factor (provisional, ADR-0005):
  ModEval(voice) → SOURCES: OSC1 · OSC2 · OSC3 (unison, FM/PM, AM, ring) · SUB/808 · SAMPLE/ORGAN · NOISE
  MIXER: level · pan · mute/solo · phase invert · width · routing → [F1 bus] [F2 bus] [Direct bus]
  FILTERS: F1, F2 — serial / parallel / split, morph (LP→HP, LP→BP, LP→NOTCH)
  SHAPE: harmonics/distortion → low-end-safe saturation → transient → DC block
  AMP: ENV1 × velocity curve → voice pan
Σ voices → single decimation to host rate
GLOBAL (host rate):
  Character (CLEAN/WARM/DRIVE) → Response (SMOOTH/AGGRESSIVE)
  FX: Drive · Saturation · EQ (5-band + sub filter) · Compressor · Multiband · Transient · Chorus ·
      Microshift · Delay · Reverb (bypass + mix each; wet paths sub-protected)
  STEREO: SUB/LOW/MID/HIGH width · M/S · spread · MONO/STEREO
  OUTPUT: EQ · Compressor · Saturation · Clipper · Gain
  LOW-END GUARD: DC block → subsonic HPF → mono-below LR4 (20–250 Hz) → linked limiter/ceiling
Analysis taps: post-guard (meters, LUFS, spectrum), pre-FX (optional), display tap (PLAY waveform)
```

Key decisions: band-limited mipmapped wavetables generated deterministically in code (versioned IDs);
FM as phase modulation (3×3 + feedback); composite bass families (Reese etc.) as documented
algorithms; mixer Direct bus keeps SUB out of filters; layered low-end protection guaranteeing mono
low end at the output; control-rate modulation every 32 host samples with per-sample interpolation
for amp/pitch/cutoff; sample-accurate MIDI via sub-block splitting; crossfaded mode switches between
pre-allocated algorithm instances.

**Oversampling (ADR-0005, provisional):** the architecture keeps independent oversampling capability
at three boundaries — voice domain, local per-stage for nonlinear stages, source-level for FM.
Phase 1 implements only the interface plus 1×/2×/4× for the voice domain, enough to measure.
Benchmarks (CPU + aliasing) at Phase 1 (polyphony × unison), Phase 3 (FM), Phase 5 (nonlinear).
Aliasing quality ranks first, CPU second. Final strategy recorded after Phase 5.

## C. Voice architecture

- `VoicePool` allocates `kMaxVoiceCapacity` (single constant) + 2 fade slots in `prepareToPlay`;
  user *Voices* setting is a runtime limit. Unison (≤ 16 per oscillator) lives inside a voice.
- Modes Poly / Mono / Legato; mono priority Last / Lowest / Highest (fixed 128-entry note stack);
  retrigger Always / Legato-only; envelope reset From-zero / From-current (default).
- Stealing: releasing → quietest → oldest; stolen voice fades 2–5 ms in a fade slot.
- Glide Off / Time / Rate / Legato-only, in log-pitch; poly glide from the last note.
- Pitch bend separate up/down ranges, smoothed, also a bipolar source. Velocity with curve;
  channel AT global source, poly AT per voice. MPE out of scope but not precluded.
- ENV1 AMP, ENV2 FILTER, ENV3 PITCH, ENV4 MOD: one multi-stage engine; ADSR knobs are parameters
  driving the canonical segments, extra drawn points are state.
- LFOs Voice (retrigger) or Global (free / host-synced).
- Oscillator phase Free / Reset / Random; unison phase spread + random; SUB/808 default Reset 0°;
  random draws from a seeded per-instance PRNG.

## D. Parameter architecture (ADR-0003)

- `ParameterRegistry` (12/) is the single source of truth (ID, version hint, group, range, mapping,
  default, unit, format, flags). Host layout, state format, golden test — and later mod destinations,
  UI bindings, randomiser rules and docs — derive from it.
- IDs: dot-separated lowerCamelCase segments; permanent (never renamed, reused or removed).
- Never change a released parameter's range, mapping, default or choice order; choice lists
  append-only. Pinned by `09_TESTS/Golden/parameter_ids.golden.json`.
- Host groups follow pages: Performance (Macros, Modes) / Source / Shape / Modulation / Matrix / FX /
  Output / Global.
- Normalisation via exact mapping functions (log for frequency/time, linear-in-dB gains, linear
  semitones/pan, discrete choices). Structural settings (OS quality, voice capacity, UI scale/page,
  detail level) are non-automatable state.
- Continuous parameters use `FoundationFloatParameter`: exact plain-value storage, echo guard, atomic
  plain-value pointer for the audio thread.

## E. Modulation architecture (ADR-0004)

- Sources (stable string IDs): voice — ENV1–4, voice LFOs, velocity, key, poly AT, note random;
  global — global LFOs, mod wheel, channel AT, pitch bend, global random, audio follower, Macro1–8,
  perf modes.
- Destinations: registry parameters flagged modulatable (voice or global scope). Macros accept only
  global non-macro sources (pass 1) — no cycles.
- Route: enabled, source, amount (−1…+1), destination, curve (type + bend), polarity, source range,
  invert, optional via (Expert). Offsets summed in the destination's normalised space, clamped.
- 64 fixed slots. Host parameters per slot: `mod.slotNN.amount`, `mod.slotNN.enabled`.
  Source/destination/curve/range are state stored as string IDs.
- Message thread builds immutable `RouteTable` snapshots; atomic swap; deferred reclamation.
- Modulation display snapshot read by EDIT at ~30 Hz. Right-click assignment on every modulatable control.

## F. Macro architecture

- 8 macros (`macroN.value`), each a modulation source; assignments are ordinary routes.
- PLAY: SUB = M1, BODY = M2, TEXTURE = M3, MOVEMENT = M4, WIDTH = M5, OUTPUT = M6; M7–M8 EDIT-only.
- Factory convention: M6 routes to `output.gain`; EDIT warns if PLAY OUTPUT has no routes.
- Mode groups `perf.character` (Clean/Warm/Drive), `perf.stereoMode` (Mono/Stereo),
  `perf.response` (Smooth/Aggressive): real parameters driving modular DSP stages, also exposed as
  smoothed modulation sources. Defaults CLEAN / STEREO / SMOOTH. Voicing tuned during sound design.

## G. State / preset architecture (ADR-0006)

- Tree: schemaVersion · engineVersion · productVersion; params (plain values); routes; macros;
  env/LFO shapes; source references; rng seed; metadata; session-only A/B and UI.
- Plain values stored as the shortest exactly-round-tripping decimal (bit-exact recall).
- Presets: JSON text (Phase 7). Session: GZIP binary ValueTree.
- `engineVersion` keeps legacy algorithms selectable; wavetables versioned; migration chain per schema.
- Unknown IDs preserved; newer schema rejected without change; missing parameters reset to defaults.
- Loading: build on message thread → ramp down → atomic swap → reset → ramp up.
- A/B snapshots; undo/redo (UndoManager, gestures grouped, automation not recorded, ~200 steps);
  randomiser on message thread with section locks, registry rules, safety pass; deterministic seeds.

## H. PLAY UI architecture (ADR-0002, ADR-0001)

- Coordinates in artwork pixels (1983 × 793) in a single `PlayLayout` table; one scale transform.
- Layers: (1) `UI BUTTONS WHITE - OFF.png` full frame, unmodified; (2) selected-button crops taken at
  runtime from unmodified `UI BUTTONS BLACK - ON.png` (exact rects below); (3) knob rings per
  ADR-0001; (4) waveform display clipped to the recessed circle, pitch-locked window, idle preview;
  (5) preset name via Typography; (6) invisible hit areas; (7) native preset-browser overlay.
- Button rects (artwork px, all y 558–597): CLEAN 266–403 · WARM 416–561 · DRIVE 575–719 ·
  MONO 839–985 · STEREO 999–1147 · SMOOTH 1305–1489 · AGGRESSIVE 1503–1714. All ON/OFF pixel
  differences lie inside x 266–1714, y 558–597.
- Knob centres ≈ (292,396) (505,396) (719,396) (1261,396) (1476,396) (1690,396), track radius ≈ 66;
  display centre ≈ (990,415), outer ≈ 115, inner ≈ 101 — finalised by script in Phase 1/9.
- PLAY/EDIT share the APVTS; new instances open in PLAY; window fixed at 1983:793; user scale
  50–100 % with cached pre-scaled background.

### Phase 1 PLAY rendering proof (mandatory)

Rendering-only (no hit areas, interaction, host controls, or plugin UI), using the real rendering
layers in `08_UI/PLAY/Rendering/`, rendered offscreen. Pass criteria: OFF renders with 0 px
difference; all 7 ON crops over OFF equal ON with 0 px difference; all 12 combinations correct;
rings within ±0.5° at the five calibration points and centred on the printed track; waveform
changes 0 px outside the circle mask; preset text changes 0 px outside its rect (provisional,
labelled font); overlay alignment at 50/70/100/125/150/200 %; artwork↔screen round-trip ≤ 0.5 px;
PNG contact sheet for sign-off.

## I. EDIT UI architecture

Theme tokens sampled from PLAY; `Typography` roles with a `FontProvider` (final font pending; OS sans
fallback, provisional, not embedded); `FoundationLookAndFeel`; vector `FoundationKnob` calibrated
against the PLAY knob. Header reuses the PLAY logo crop. Six pages × Simple / Advanced / Expert;
hiding never resets; collapsed panels flag non-default/modulated hidden controls.

| Page | Simple | Advanced | Expert |
|---|---|---|---|
| SOURCE | source cards, mixer levels | unison, WT, FM, AM/ring, SUB/808, drawbars, sample, voice | symmetry, emphasis, interpolation, unison detail, mixer routing, full FM, organ detail |
| SHAPE | filters, distortion, ENV1–3 ADSR | routing/morph, saturation, transient, noise, env curves | drawable envelopes, bias/threshold, noise detail, order |
| MODULATION | LFO1–4, ENV4 | fade/phase/smooth/polarity/retrigger/scope, performance curves | drawn LFOs, audio follower, random |
| MATRIX | macros, active routes | 64-slot list, filtering | via, polarity, signal-flow view |
| FX | chain strip (Drive, EQ, Comp, Chorus, Delay, Reverb) | + Saturation, Transient, Microshift, Clipper | Multiband, parallel/sends, order, per-FX OS |
| OUTPUT | gain, mono-below, width, ceiling, analyzer | stereo engine, output chain, perf mode depths | OS quality, voices, tuning, DC/subsonic, LUFS, vectorscope |

## J. Analysis architecture

SPSC taps → analysis thread (editor open only). Multi-resolution FFT (16k below 500 Hz); sample +
true peak; RMS 300 ms; BS.1770-4 LUFS (M/S/I, gating, rate-correct K-weighting); correlation and
per-band width; LR4 bass bands (SUB 20–60, BODY 60–150, CHARACTER 150–500, PRESENCE 500+);
fundamental/harmonics from known note pitch. Lock-free snapshots to UI.

## K. Real-time safety

No allocation (all sizing in prepare; fixed-capacity containers; allocation detector in tests;
RTSan on macOS CI), no locks (atomics, SPSC, snapshot swaps), no I/O (background loading), no UI
calls (flags + polling), denormal protection, crossfaded mode changes, transactional state loads,
bounded work, no exceptions on the audio path, latency-matched bypass.

## L. Testing architecture

Catch2 via CTest, linking modules directly. Unit (registry, ID golden, maths, voices, migration),
DSP offline renders, determinism matrix, state, golden renders, realtime, automation, plugin/host
(pluginval strictness 10, VST3 validator, auval, DAW matrix), UI (PLAY proof, ring geometry,
EDIT snapshots), performance. RenderCLI renders presets/MIDI to WAV.

## M. Build architecture

JUCE 9.0.3 and Catch2 v3.16.0 as pinned submodules with configure-time version checks. Targets:
module interface libraries; `Foundation` (VST3, Standalone, AU on Apple); `foundation_tests`;
`foundation_render_cli`. C++20, static MSVC runtime, no fast-math, strict warnings on our sources.
Presets `windows-vs2026`, `macos-xcode`, `ci-windows`, `ci-macos`; Debug / RelWithDebInfo / Release.

## N. Risks

Knob/track overshoot review (N1); 1× art on HiDPI (N2); host resize behaviour (N3); aliasing vs CPU
(N4); low-end mono integrity (N5); parameter count in hosts (N6); cross-version recall (N7);
click-free atomic preset switching (N8); sidechain support for instruments (N9); synthesised M1-style
organ quality (N10); late font (N11); scope (N12); float differences across platforms (N14);
missing art (N15); licensing (N16).

## O. Implementation order

0 Repo/build/test foundation · 1 Minimum engine + mandatory PLAY rendering proof + OS baseline
benchmark · 2 Advanced source · 3 Advanced synthesis (+ FM benchmark) · 4 Modulation · 5 Shaping
(+ nonlinear benchmark, ADR-0005 final) · 6 FX · 7 Workflow · 8 Analysis · 9 PLAY · 10 EDIT ·
11 Integration · 12 Testing · 13 Polish. Every phase exits buildable, tests green, pluginval passing,
zero allocation-detector hits.
