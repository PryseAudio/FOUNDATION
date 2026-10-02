# ADR-0003 — Parameter ID policy

Status: **Accepted**

## Decision

- `ParameterRegistry` (`12_PARAMETERS_STATE`) is the single source of truth for every parameter.
- IDs are dot-separated segments, each lowerCamelCase: `macro1.value`, `perf.stereoMode`,
  `filter1.cutoff`, `mod.slot17.amount`.
- IDs are permanent: never renamed, reused or removed. Deprecated parameters stay registered (hidden).
- A released parameter's range, mapping, default and choice order never change. Hosts store
  automation as normalised 0..1 values, so such a change silently rewrites users' automation.
  Unavoidable changes create a new ID plus a state migration.
- Choice lists are append-only.
- JUCE `ParameterID` version hints are used (AU parameter ordering); new parameters take the next hint.
- Never toggle `JUCE_FORCE_USE_LEGACY_PARAM_IDS`; VST3 parameter IDs are hashed from the string IDs.
- Continuous parameters use `FoundationFloatParameter` (exact plain storage, host-echo guard).

## Enforcement

`09_TESTS/Golden/parameter_ids.golden.json` pins every ID, its order and definition. The test fails
on any removal, reorder, definition change or unregistered addition. The golden file may be updated
deliberately until the first public release; afterwards it is frozen.
