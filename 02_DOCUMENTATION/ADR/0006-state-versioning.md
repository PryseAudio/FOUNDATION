# ADR-0006 — State versioning and exact recall

Status: **Accepted** (Phase 0 implements the parameter subset)

## Decision

- Exact sound recall across plugin versions is a hard requirement.
- State tree root `FOUNDATION_STATE` carries `schemaVersion`, `engineVersion`, `productVersion`.
  `schemaVersion` is independent of the product version.
- Parameter values are stored in plain units as the shortest decimal string that round-trips the
  float exactly (`std::to_chars`), and restored through `FoundationFloatParameter::setPlainValueExact`.
- Registered parameters missing from a state are reset to their defaults (sound-neutral).
- Entries for unknown IDs (from newer builds) are preserved and written back unchanged.
- States with a newer `schemaVersion` are rejected without changing anything; malformed states
  likewise.
- A migration chain (v1 → v2 → …) runs before apply; each step gets fixture tests.
- `engineVersion` keeps legacy algorithms selectable so old presets sound identical after DSP
  improvements; wavetables carry versioned IDs.
- Session state: GZIP-compressed binary ValueTree, including A/B and UI state. Preset files: JSON
  text with the active sound and metadata (Phase 7).

## Phase 0 verification

State round-trip tests: byte-identical re-save, bit-exact plain values (scrambled values and host
automation values), host-echo immunity, missing-parameter reset, unknown-ID preservation, rejection
of newer/malformed/garbage states without change, and exact float text encoding over 100 000 values.
