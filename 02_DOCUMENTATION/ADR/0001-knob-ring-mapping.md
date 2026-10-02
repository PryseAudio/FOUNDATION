# ADR-0001 — PLAY knob ring angle mapping

Status: **Accepted** (Step 2 lock)

## Context

The PLAY knob progress ring geometry is specified by five visual calibration points. They do not
form a linear 300° sweep (a linear sweep from 7 o'clock would place 25 % at 9:30 and 75 % at 2:30).
The specification is authoritative and must not be simplified.

## Decision

Angles are measured clockwise from 12 o'clock (JUCE's arc convention, `Path::addCentredArc`).

| Value | Clock | Angle | Radians |
|---|---|---|---|
| 0 % | 7:00 | 210° | 7π/6 |
| 25 % | 10:00 | 300° | 5π/3 |
| 50 % | 12:00 | 360° | 2π |
| 75 % | 2:00 | 420° | 7π/3 |
| 100 % | 5:00 | 510° | 17π/6 |

Mapping, with `u = v − 0.5` for value `v ∈ [0, 1]`:

    θ(v) = 360° + 220°·u + 320°·u³

- Exact at all five points: u = ±0.25 → ±(55 + 5) = ±60°; u = ±0.5 → ±(110 + 40) = ±150°.
- Smooth, strictly increasing (dθ/du = 220 + 960u² > 0) and symmetric about 12 o'clock.
- The ring is drawn from 210° to θ(v); at 0 % nothing is drawn.
- **Visual only.** The parameter value stays a plain linear 0–1 value for host, DSP and automation.
- Alternative kept available as a one-function swap: piecewise-linear interpolation between the five
  points (also exact at the points, but with visible speed changes at 25 % and 75 %).

## Verification

- Unit test (added with the ring renderer in Phase 1) asserts the five points to 1e-9°.
- Phase 1 PLAY rendering proof checks ring end angles within ±0.5° and centring on the printed track.
- The printed track in the artwork measures ≈ 218°–142° (±3°); the proof renders the ring as
  specified so its ends can be reviewed against the printed track. The specification is not changed.
