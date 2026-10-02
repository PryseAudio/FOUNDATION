# FOUNDATION — MASTER SPECIFICATION

## Product vision

FOUNDATION is a premium professional bass sound-design instrument. It has two primary experiences:

- PLAY — simple, beautiful, immediate, musical and approachable.
- EDIT — deep, technical, powerful, flexible and limitless.

Both experiences use one underlying DSP engine, parameter system, modulation system and preset/state system.

The product must support professional creation of clean subs, 808s, M1/organ basses, Reese basses, FM basses, hard/crisp basses, modern house basses, distorted basses, plucked/rolling basses, experimental basses, analog/digital basses and harmonic-rich basses.

## Non-negotiables

1. PLAY is already designed and is the default landing page.
2. Do not redesign, reinterpret, replace or simplify the supplied PLAY artwork.
3. EDIT is the advanced sound-design environment.
4. PLAY and EDIT must feel like the same premium product.
5. DSP controls must be real, not cosmetic.
6. Use supplied artwork/assets where specified instead of generic JUCE controls.
7. Keep the project buildable and testable throughout development.
8. Real-time audio code must not allocate, perform file I/O, block, manipulate UI or use unpredictable locks.
9. Presets must recall exact sound state.
10. Host automation must work with stable parameter IDs.
11. Modulation must be scalable and genuinely functional.
12. Protect low-end stability and mono compatibility.

## Technology

Use C++, JUCE, VST3, AU where appropriate, and a standalone development target. Keep DSP, parameters, state, presets, UI, editor, assets, analysis and tests cleanly separated.

## Architecture

FOUNDATION
- Audio Engine
- Voice Engine
- Oscillator Engine
- Organ Engine
- 808 Engine
- FM Engine
- Filter Engine
- Harmonic Engine
- Modulation Engine
- Modulation Matrix
- Macro Engine
- FX Engine
- Stereo Engine
- Analysis Engine
- Preset / State System
- UI
  - PLAY
  - EDIT
    - SOURCE
    - SHAPE
    - MODULATION
    - MATRIX
    - FX
    - OUTPUT

## PLAY

PLAY is the finished landing page and the visual source of truth is the supplied reference artwork. Do not redesign it.

Visual language: warm off-white/ivory, tactile/neumorphic, embossed/engraved FOUNDATION branding, restrained charcoal/black controls, premium physical-instrument feeling.

Main controls: SUB, BODY, TEXTURE, central dynamic waveform display, MOVEMENT, WIDTH, OUTPUT.

Top: branding, preset browser, previous/next preset, favourite, PLAY/EDIT switch, waveform/menu controls.

Bottom exclusive groups:
- CLEAN / WARM / DRIVE
- MONO / STEREO
- SMOOTH / AGGRESSIVE

Only one option per group is selected. Use supplied white button artwork as the base and supplied black button artwork/crops as selected overlays. Do not replace them with generic JUCE buttons.

Knobs: white wheel, dynamically rendered black progress ring, small gap; arc starts around 8 o'clock at 0% and moves clockwise to about 4 o'clock at 100%.

The central circle is a dynamic waveform display, not a knob. There are no side IN/OUT sliders in the current PLAY design.

PLAY macros must connect to the real modulation/macro engine.

## EDIT navigation

1. SOURCE — what is making the sound?
2. SHAPE — what does it sound/look like?
3. MODULATION — how does it move?
4. MATRIX — what controls what?
5. FX — how is it processed?
6. OUTPUT — how does it translate and leave the instrument?

EDIT must look like the same premium instrument as PLAY.

## SOURCE

Provide OSC1, OSC2, OSC3, SUB and SAMPLE/ORGAN.

Oscillator controls: enable, level, pan, octave, semitone, fine tune, phase, phase reset/randomization, waveform, waveform position/morph, unison, voice count, detune, spread, FM, AM and ring modulation.

Waveform/source families:
- Analog: sine, triangle, saw, reverse saw, square, pulse, variable pulse, noise.
- Digital: additive, wavetable, digital saw, digital square, formant, metallic, complex harmonic.
- Bass: clean sine, rounded sine, saturated sine, 808, Reese, hollow Reese, wide Reese, rubber, pluck, sub pulse.
- Organ: organ fundamental, harmonic combinations, M1-style sources, bright/dark/percussive organ.

Wavetable: position, interpolation, morph, phase, warp, symmetry, harmonic emphasis.

FM is first-class: carrier/modulator/both, amount, ratio, index, feedback, FM envelope/source, routing matrix, integer and non-integer ratios.

AM/ring modulation: source, destination, amount and mix.

Unison: voice count, detune, spread, phase/random phase, stereo randomization, drift, low-end-safe mode.

Mixer channels OSC1/2/3, SUB, SAMPLE/ORGAN with level, pan, mute, solo, phase invert, width and routing.

## SAMPLE ENGINE

One-shot/loop, start/end, reverse, pitch, envelope, filter, level, pan, phase, key tracking.

## SUB ENGINE

Sine, triangle, saturated sine, 808-style, organ-derived. Level, octave, drive, attack/release, low-pass, phase, mono, harmonic generation.

## ORGAN ENGINE

Drawbars: 16', 8', 5 1/3', 4', 2 2/3', 2', 1 3/5', 1'. Add percussion, key click, leakage, tone, attack, decay, release, harmonic balance and saturation. Provide usable M1-style organ behavior.

## 808 ENGINE

Fundamental, pitch drop, pitch envelope, drop time, initial transient, click, saturation, clipping, harmonic generation, decay, tail and sub harmonics.

## SHAPE

Filter 1/2, harmonics, distortion, saturation, transient shaping, envelopes and noise.

Filters: LP12, LP24, HP12, HP24, BP, notch, comb, formant, character. Two filters with serial/parallel/split routing. Controls cutoff, resonance, drive, key tracking, envelope amount and modulation amount. Filter morph routes such as LP->HP, LP->BP and LP->NOTCH; morph is modulatable.

Harmonic/distortion types: soft clip, hard clip, tube, tape, diode, wavefold, rectify, bit crush, downsample, digital clip, asymmetric saturation. Controls drive, tone, mix, bias, threshold, output.

Dedicated low-end-safe saturation: fundamental, harmonics, upper-bass, clipping, crossover and mix.

Transient: attack, body, sustain, click, snap, knock, noise.

Noise: white, pink, brown, digital, air, texture; level, filter, envelope, pan, width.

## ENVELOPES

At least ENV1 AMP, ENV2 FILTER, ENV3 PITCH, ENV4 MOD. ADSR + curve, velocity, key tracking, loop; drawable multi-stage envelopes with additional points.

## LFOs

At least four. Rate, depth, phase, fade, smooth, sync, bipolar/unipolar, retrigger/free-run. Shapes sine, triangle, saw, reverse saw, square, sample & hold, random, user drawn. Tempo sync 1/1, 1/2, 1/4, 1/8, 1/16, 1/32 and triplets. Drawable points, curves, smoothing, loop, symmetry and phase.

## MODULATION SOURCES

ENV1-4, LFO1-4, velocity, key tracking, mod wheel, aftertouch, pitch bend, random, audio follower and macros.

Audio follower, where format/host allows, can modulate filter, distortion, amplitude, FM, wavetable, width and effects.

## MODULATION MATRIX

Core route: SOURCE -> AMOUNT -> DESTINATION -> CURVE. Support many simultaneous routes and do not hardcode the matrix to a tiny number of slots.

Meaningful modulatable parameters include oscillator pitch/level/pan/waveform position/morph, FM index/amount, phase, filter cutoff/resonance/drive, saturation, distortion, EQ, stereo width, chorus/delay/reverb, transient, sub level and macros.

Modulated parameters should show a clear visual indication in EDIT. Right-click macro assignment where practical.

## MACROS

At least eight macros. Each can control multiple destinations with positive/negative amounts, custom ranges and inversion. PLAY controls must be powered by this system rather than special hardcoded cases.

## FX

Drive, saturation, EQ, compressor, multiband, transient, chorus, microshift, delay, reverb, clipper, limiter. Support parallel processing paths where useful.

EQ: five bands (low, low-mid, mid, high-mid, high), frequency/gain/Q/enable, plus sub filter and low-end mono.

Compressor: threshold, ratio, attack, release, knee, makeup, mix; sidechain architecture where practical.

Multiband: low/mid/high.

Stereo: SUB/LOW/MID/HIGH width, stereo spread, micro-detune, chorus width, M/S, protect fundamental.

Chorus/microshift: rate, depth, detune, width, mix; left/right pitch offsets.

Delay: time, sync, feedback, filter, width, mix.

Reverb: pre-delay, size, decay, damping, width, low-cut, high-cut, mix; protect the sub.

## OUTPUT / ANALYSIS

Output processing: EQ, compressor, saturation, clipper, limiter.

Analyzer: spectrum, peak, RMS, LUFS, stereo width.

Bass bands:
- SUB 20–60 Hz
- BODY 60–150 Hz
- CHARACTER 150–500 Hz
- PRESENCE 500 Hz+

Also show fundamental, harmonics and stereo width. Adjustable mono-below crossover, e.g. 20–250 Hz.

## LOW-END SAFETY

DC blocking, subsonic filtering, soft clipping, limiter protection, output ceiling, and oversampling where appropriate, especially nonlinear processing, wavefolding, clipping and FM. Prevent excessive aliasing, unstable sub and destructive stereo phase in the fundamental.

## VOICE

Mono/poly, legato, glide/portamento, voice stealing, pitch bend, aftertouch, velocity, retrigger, envelope reset and oscillator phase modes. Mono note priority last/lowest/highest. Portamento off/time/rate/legato-only. Velocity can modulate amplitude, filter, FM, distortion, harmonics, transient and macros.

## PRESETS / WORKFLOW

Load, save, save as, favourite, tags, categories, search, previous/next. Categories: SUB, 808, ORGAN, REESE, HOUSE, HARD, FM, ANALOG, DIGITAL, PLUCK, TEXTURE, EXPERIMENTAL.

Exact state recall is mandatory.

Intelligent randomization for source, shape, modulation, FX and everything, with locks for sections.

A/B with A->B and B->A. Undo/redo. Signal-flow visualization. Modulated parameters visibly indicate modulation.

## UI COMPLEXITY

Use collapsible/expandable/contextual/advanced/expert panels. Simple -> Advanced -> Expert reveals complexity without changing the underlying engine.

## SUGGESTED UI COMPONENTS

FoundationEditor, FoundationHeader, PresetBrowser, PageNavigation, SourcePage, OscillatorPanel, SubPanel, OrganPanel, SamplePanel, MixerPanel, FMMatrixPanel, ShapePage, FilterPanel, HarmonicsPanel, DistortionPanel, EnvelopePanel, TransientPanel, ModulationPage, LFOPanel, AudioFollowerPanel, MatrixPage, ModulationMatrix, MacroPanel, FXPage, DrivePanel, EQPanel, CompressorPanel, ChorusPanel, DelayPanel, ReverbPanel, OutputPage, Analyzer, StereoPanel, LimiterPanel.

## VISUAL LANGUAGE

Warm off-white, ivory, soft gray, charcoal, black, subtle shadows, embossed typography, tactile controls, restrained contrast. Avoid neon/cyberpunk/gaming/glowing-blue/generic AI-synth aesthetics. EDIT must look like the same product as PLAY.

## DEVELOPMENT PHASES

0 Architecture
1 Minimum audio engine
2 Advanced source
3 Advanced synthesis
4 Modulation
5 Shaping
6 FX
7 Professional workflow
8 Analysis
9 PLAY implementation
10 EDIT implementation
11 Integration
12 Testing
13 Polish

Keep the project buildable/testable after every phase.

## TESTING

Test oscillator pitch, filters, envelopes, LFO timing, FM, modulation, preset save/load, parameter recall, mono/poly, portamento, host automation, sample rates, block sizes, rapid parameter changes, rapid preset changes, plugin reload, denormal protection, audio-thread allocation and output protection.

## ACCEPTANCE CRITERIA

Not acceptable if PLAY is redesigned, PLAY differs materially from the supplied reference, EDIT looks like a different product, generic controls replace specified artwork, FM is cosmetic, modulation is hardcoded, presets fail recall, automation fails, audio thread allocates, low end is unstable, stereo destroys the sub, aliasing is excessive, automation clicks, presets cannot reproduce their sound, or the engine cannot create genuinely diverse basses.

Success means a professional sound designer can deliberately create clean sub, 808, M1-style organ, modern house bass, Reese, wide Reese, FM, metallic, hard, distorted, plucked, rolling, experimental, analog, digital and harmonic-rich basses and sounds between them.

## FINAL PRINCIPLE

PLAY = simple, beautiful, immediate, musical, approachable.
EDIT = deep, technical, powerful, flexible, limitless.
DSP ENGINE = the machine underneath both.

PLAY is the finished visual identity.
EDIT is where the beast lives.
