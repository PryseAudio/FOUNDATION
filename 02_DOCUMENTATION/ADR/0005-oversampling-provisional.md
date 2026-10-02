# ADR-0005 — Oversampling strategy (PROVISIONAL)

Status: **Provisional** — final decision after Phase 5 measurements

## Current design

- Keep independent oversampling capability at three boundaries: the voice domain, local per-stage
  oversampling for nonlinear stages, and source-level oversampling for FM/audio-rate modulation.
- The initial proposal (whole voice domain at 1×/2×/4×, voices summed before a single decimation)
  is the working hypothesis, not a final decision.
- Oversampling factor is a non-automatable quality setting; latency is constant per configuration
  and reported to the host.

## Measurement plan

Phase 1 implements only the oversampling interface plus 1×/2×/4× for the voice domain.

| Gate | Measures |
|---|---|
| Phase 1 | polyphony × unison at 1×/2×/4× |
| Phase 3 | FM (operator matrix, feedback) |
| Phase 5 | nonlinear stages (distortion, wavefold, clip, saturation; ADAA vs OS) |

Each gate records CPU (`foundation_bench`) and aliasing (DSP test metrics).

## Decision rule

Aliasing quality ranks first, CPU second. Sound quality is not compromised to save CPU. No
oversampling machinery beyond what a measurement needs is built ahead of the measurements.
