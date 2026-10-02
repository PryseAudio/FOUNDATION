#include "AllocationDetector.h"

#include "RealtimeGuard.h"

#include <atomic>
#include <cstdlib>
#include <new>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

namespace
{

std::atomic<std::int64_t> realtimeAllocations { 0 };
std::atomic<std::int64_t> realtimeDeallocations { 0 };

void noteAllocation() noexcept
{
    if (foundation::rt::isInRealtimeScope())
        realtimeAllocations.fetch_add(1, std::memory_order_relaxed);
}

void noteDeallocation(void* pointer) noexcept
{
    if (pointer != nullptr && foundation::rt::isInRealtimeScope())
        realtimeDeallocations.fetch_add(1, std::memory_order_relaxed);
}

void* allocate(std::size_t size) noexcept
{
    noteAllocation();
    return std::malloc(size == 0 ? 1 : size);
}

void* allocateAligned(std::size_t size, std::align_val_t alignment) noexcept
{
    noteAllocation();
    const auto align = static_cast<std::size_t>(alignment);
    const auto bytes = size == 0 ? align : size;
#if defined(_MSC_VER)
    return _aligned_malloc(bytes, align);
#else
    void* pointer = nullptr;
    return posix_memalign(&pointer, align < sizeof(void*) ? sizeof(void*) : align, bytes) == 0 ? pointer : nullptr;
#endif
}

void release(void* pointer) noexcept
{
    noteDeallocation(pointer);
    std::free(pointer);
}

void releaseAligned(void* pointer) noexcept
{
    noteDeallocation(pointer);
#if defined(_MSC_VER)
    _aligned_free(pointer);
#else
    std::free(pointer);
#endif
}

void* allocateOrThrow(std::size_t size)
{
    if (auto* pointer = allocate(size))
        return pointer;
    throw std::bad_alloc();
}

void* allocateAlignedOrThrow(std::size_t size, std::align_val_t alignment)
{
    if (auto* pointer = allocateAligned(size, alignment))
        return pointer;
    throw std::bad_alloc();
}

} // namespace

namespace foundation::test
{

RealtimeHeapCounts realtimeHeapCounts() noexcept
{
    return { realtimeAllocations.load(std::memory_order_relaxed),
             realtimeDeallocations.load(std::memory_order_relaxed) };
}

ScopedRealtimeHeapCheck::ScopedRealtimeHeapCheck() noexcept
    : start(realtimeHeapCounts())
{
}

std::int64_t ScopedRealtimeHeapCheck::allocations() const noexcept
{
    return realtimeHeapCounts().allocations - start.allocations;
}

std::int64_t ScopedRealtimeHeapCheck::deallocations() const noexcept
{
    return realtimeHeapCounts().deallocations - start.deallocations;
}

} // namespace foundation::test

//==================================================================================================
// Replaceable global allocation functions
//==================================================================================================

void* operator new(std::size_t size) { return allocateOrThrow(size); }
void* operator new[](std::size_t size) { return allocateOrThrow(size); }
void* operator new(std::size_t size, const std::nothrow_t&) noexcept { return allocate(size); }
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept { return allocate(size); }
void* operator new(std::size_t size, std::align_val_t alignment) { return allocateAlignedOrThrow(size, alignment); }
void* operator new[](std::size_t size, std::align_val_t alignment) { return allocateAlignedOrThrow(size, alignment); }

void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    return allocateAligned(size, alignment);
}

void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    return allocateAligned(size, alignment);
}

void operator delete(void* pointer) noexcept { release(pointer); }
void operator delete[](void* pointer) noexcept { release(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { release(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { release(pointer); }
void operator delete(void* pointer, const std::nothrow_t&) noexcept { release(pointer); }
void operator delete[](void* pointer, const std::nothrow_t&) noexcept { release(pointer); }
void operator delete(void* pointer, std::align_val_t) noexcept { releaseAligned(pointer); }
void operator delete[](void* pointer, std::align_val_t) noexcept { releaseAligned(pointer); }
void operator delete(void* pointer, std::size_t, std::align_val_t) noexcept { releaseAligned(pointer); }
void operator delete[](void* pointer, std::size_t, std::align_val_t) noexcept { releaseAligned(pointer); }
void operator delete(void* pointer, std::align_val_t, const std::nothrow_t&) noexcept { releaseAligned(pointer); }
void operator delete[](void* pointer, std::align_val_t, const std::nothrow_t&) noexcept { releaseAligned(pointer); }
