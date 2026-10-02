# ADR-0002 — Artwork coordinate space and PLAY rendering

Status: **Accepted** (Step 2 lock)

## Context

The PLAY and EDIT references are both 1983 × 793. PLAY is visually locked; the supplied
`UI BUTTONS WHITE - OFF.png` and `UI BUTTONS BLACK - ON.png` are production artwork.
`PLAY_PAGE_REFERENCE.png` is pixel-identical to the ON image (all seven buttons selected).
ON and OFF differ only inside x 266–1714, y 558–597.

## Decision

- One fixed artwork-space coordinate system, 1983 × 793, for both pages. PLAY renders the supplied
  composition in full (backdrop included). The host window never resizes on page switch.
- All PLAY geometry lives in a single `PlayLayout` table in artwork pixels; the editor applies one
  scale transform. User scale 50–100 % (default fits the screen); background pre-scaled and cached.
- Layers: OFF artwork (unmodified) → selected-button crops taken at runtime from the unmodified ON
  artwork → knob rings (ADR-0001) → waveform display (clipped to the recessed circle) → preset text
  (Typography) → invisible hit areas → native preset browser overlay.
- Exact button crop rects (artwork px, all y 558–597): CLEAN 266–403, WARM 416–561,
  DRIVE 575–719, MONO 839–985, STEREO 999–1147, SMOOTH 1305–1489, AGGRESSIVE 1503–1714.
- A mandatory technical PLAY rendering proof runs in Phase 1 (see 04_ARCHITECTURE.md §H). It is not
  the start of PLAY implementation and creates no controls.

## Consequences

- The supplied files remain the single source of truth; no derived artwork files exist.
- 1× artwork softens above 100 % physical scale; @2x variants are picked up automatically if supplied.
