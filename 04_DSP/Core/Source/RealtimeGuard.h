#pragma once

// =================================================================================================
// FOUNDATION — real-time scope marker
//
// Real-time audio code must not allocate, perform file I/O, block, take unpredictable locks or touch
// the UI (01_NON_NEGOTIABLE.md #8). Every entry point that runs on the audio thread opens a
// ScopedRealtimeScope for its whole duration. Debug tooling (the allocation detector compiled into
// the test runner, see 09_TESTS/Support/AllocationDetector.*) uses isInRealtimeScope() to flag any
// heap allocation that happens while a scope is open on the current thread.
//
// The marker is a single thread_local counter: it costs one increment and one decrement per block,
// never allocates, and is safe to leave enabled in release builds.
// =================================================================================================

namespace foundation::rt
{

namespace detail
{
inline thread_local int realtimeScopeDepth = 0;
} // namespace detail

/** True while the calling thread is inside at least one ScopedRealtimeScope. */
[[nodiscard]] inline bool isInRealtimeScope() noexcept
{
    return detail::realtimeScopeDepth > 0;
}

/** Marks the calling thread as executing real-time audio code for the lifetime of this object. */
class ScopedRealtimeScope
{
public:
    ScopedRealtimeScope() noexcept { ++detail::realtimeScopeDepth; }
    ~ScopedRealtimeScope() noexcept { --detail::realtimeScopeDepth; }

    ScopedRealtimeScope(const ScopedRealtimeScope&) = delete;
    ScopedRealtimeScope& operator=(const ScopedRealtimeScope&) = delete;
    ScopedRealtimeScope(ScopedRealtimeScope&&) = delete;
    ScopedRealtimeScope& operator=(ScopedRealtimeScope&&) = delete;
};

} // namespace foundation::rt
