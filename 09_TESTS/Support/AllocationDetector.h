#pragma once

#include <cstdint>

// =================================================================================================
// FOUNDATION — real-time allocation detector (test/tool executables ONLY)
//
// AllocationDetector.cpp replaces the global operator new/delete family for the executable it is
// linked into. Every heap allocation or deallocation performed while the calling thread is inside a
// foundation::rt::ScopedRealtimeScope (04_DSP/Core/Source/RealtimeGuard.h) is counted.
//
// WHY executables only: replacing global operator new inside a plugin binary can interpose on the
// host's allocator (notably on macOS). It is safe in our own test runner, benchmarks and tools.
// =================================================================================================

namespace foundation::test
{

struct RealtimeHeapCounts
{
    std::int64_t allocations = 0;
    std::int64_t deallocations = 0;
};

/** Totals since program start. */
[[nodiscard]] RealtimeHeapCounts realtimeHeapCounts() noexcept;

/** Measures heap operations performed inside real-time scopes during this object's lifetime. */
class ScopedRealtimeHeapCheck
{
public:
    ScopedRealtimeHeapCheck() noexcept;

    [[nodiscard]] std::int64_t allocations() const noexcept;
    [[nodiscard]] std::int64_t deallocations() const noexcept;

private:
    RealtimeHeapCounts start;
};

} // namespace foundation::test
