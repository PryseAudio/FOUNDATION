# ADR-0004 — Modulation route pool and macros

Status: **Accepted** (implementation: Phase 4)

## Decision

- A fixed pool of **64** route slots. Host parameters per slot: `mod.slotNN.amount` (−1…+1) and
  `mod.slotNN.enabled`. No host parameter is ever created dynamically.
- Source, destination, curve, polarity, source range, invert and via are **state**, stored as stable
  string IDs — never as choice indices, which would shift whenever a destination is added and break
  presets and automation.
- Unused slots are inactive and cost nothing. The pool can grow later only by appending IDs.
- Route edits build an immutable `RouteTable` on the message thread, published to the audio thread
  by atomic pointer swap with deferred reclamation.
- 8 macros (`macro1.value` … `macro8.value`) are modulation sources; their assignments are ordinary
  routes. PLAY: SUB = M1, BODY = M2, TEXTURE = M3, MOVEMENT = M4, WIDTH = M5, OUTPUT = M6.
  M7–M8 are EDIT-only. There is no special-case PLAY DSP.
- Macros can be destinations only from global, non-macro sources (evaluated first) — no cycles.
