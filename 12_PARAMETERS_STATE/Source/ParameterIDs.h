#pragma once

#include <array>
#include <string_view>

// =================================================================================================
// FOUNDATION — permanent parameter IDs (ADR-0003)
//
// RULES — these IDs are a contract with every host session and preset ever saved:
//   1. Never rename an ID. Never reuse an ID. Never remove an ID (deprecated IDs stay registered).
//   2. Never change a released parameter's range, mapping, default or choice order. Hosts store
//      automation as normalised 0..1 values, so such a change silently rewrites users' automation.
//      If a change is unavoidable, create a new ID and migrate (StateSerializer).
//   3. Choice lists are append-only.
//   4. Format: dot-separated segments, each lowerCamelCase: "<section>[.<sub>].<name>".
//
// 09_TESTS/Golden/parameter_ids.golden.json pins every ID and its definition. Until the first
// public release the golden file may be updated deliberately; after release it is frozen.
// =================================================================================================

namespace foundation::params::ids
{

// Macros — PLAY knobs SUB/BODY/TEXTURE/MOVEMENT/WIDTH/OUTPUT are Macros 1–6; Macros 7–8 are EDIT-only.
inline constexpr std::string_view macro1 = "macro1.value";
inline constexpr std::string_view macro2 = "macro2.value";
inline constexpr std::string_view macro3 = "macro3.value";
inline constexpr std::string_view macro4 = "macro4.value";
inline constexpr std::string_view macro5 = "macro5.value";
inline constexpr std::string_view macro6 = "macro6.value";
inline constexpr std::string_view macro7 = "macro7.value";
inline constexpr std::string_view macro8 = "macro8.value";

inline constexpr int numMacros = 8;

inline constexpr std::array<std::string_view, numMacros> macros {
    macro1, macro2, macro3, macro4, macro5, macro6, macro7, macro8
};

// PLAY mode groups — real engine parameters.
inline constexpr std::string_view perfCharacter = "perf.character";   // CLEAN / WARM / DRIVE
inline constexpr std::string_view perfStereoMode = "perf.stereoMode"; // MONO / STEREO
inline constexpr std::string_view perfResponse = "perf.response";     // SMOOTH / AGGRESSIVE

// Output
inline constexpr std::string_view outputGain = "output.gain";

} // namespace foundation::params::ids

namespace foundation::params
{

// Choice indices. Append-only: existing values must never be reordered or removed.
enum class Character : int
{
    clean = 0,
    warm = 1,
    drive = 2
};

enum class StereoMode : int
{
    mono = 0,
    stereo = 1
};

enum class Response : int
{
    smooth = 0,
    aggressive = 1
};

} // namespace foundation::params
