#include "platform/ds/NintendoDsAllocationDiagnostics.hpp"

#include <cstdlib>
#include <new>

namespace helengine::ds {
    namespace {
        std::size_t LastRequestedSizeValue = 0;
        std::size_t LastSuccessfulSizeValue = 0;
        std::size_t LastFailedSizeValue = 0;
        std::size_t AllocationRequestCountValue = 0;
    }

    /// Records one allocation request before the allocator attempts to satisfy it.
    /// <param name="size">Requested allocation size in bytes.</param>
    void NintendoDsAllocationDiagnostics::RecordRequest(std::size_t size) {
        LastRequestedSizeValue = size;
        AllocationRequestCountValue++;
    }

    /// Records one allocation success after the allocator returned memory.
    /// <param name="size">Satisfied allocation size in bytes.</param>
    void NintendoDsAllocationDiagnostics::RecordSuccess(std::size_t size) {
        LastSuccessfulSizeValue = size;
    }

    /// Records one allocation failure before throwing <c>std::bad_alloc</c>.
    /// <param name="size">Failed allocation size in bytes.</param>
    void NintendoDsAllocationDiagnostics::RecordFailure(std::size_t size) {
        LastFailedSizeValue = size;
    }

    /// Gets the last requested allocation size observed by the diagnostics hook.
    /// <returns>Last requested allocation size in bytes.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetLastRequestedSize() {
        return LastRequestedSizeValue;
    }

    /// Gets the last successful allocation size observed by the diagnostics hook.
    /// <returns>Last successful allocation size in bytes.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetLastSuccessfulSize() {
        return LastSuccessfulSizeValue;
    }

    /// Gets the last failed allocation size observed by the diagnostics hook.
    /// <returns>Last failed allocation size in bytes.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetLastFailedSize() {
        return LastFailedSizeValue;
    }

    /// Gets the running allocation-request count observed by the diagnostics hook.
    /// <returns>Total allocation request count seen so far.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetAllocationRequestCount() {
        return AllocationRequestCountValue;
    }
}

/// Routes global scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <returns>Allocated memory block.</returns>
void* operator new(std::size_t size) {
    helengine::ds::NintendoDsAllocationDiagnostics::RecordRequest(size);
    void* memory = std::malloc(size);
    if (memory == nullptr) {
        helengine::ds::NintendoDsAllocationDiagnostics::RecordFailure(size);
        throw std::bad_alloc();
    }

    helengine::ds::NintendoDsAllocationDiagnostics::RecordSuccess(size);
    return memory;
}

/// Routes global array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <returns>Allocated memory block.</returns>
void* operator new[](std::size_t size) {
    helengine::ds::NintendoDsAllocationDiagnostics::RecordRequest(size);
    void* memory = std::malloc(size);
    if (memory == nullptr) {
        helengine::ds::NintendoDsAllocationDiagnostics::RecordFailure(size);
        throw std::bad_alloc();
    }

    helengine::ds::NintendoDsAllocationDiagnostics::RecordSuccess(size);
    return memory;
}

/// Releases one scalar allocation created by the diagnostics-aware allocator hook.
/// <param name="memory">Previously allocated memory block.</param>
void operator delete(void* memory) noexcept {
    std::free(memory);
}

/// Releases one array allocation created by the diagnostics-aware allocator hook.
/// <param name="memory">Previously allocated memory block.</param>
void operator delete[](void* memory) noexcept {
    std::free(memory);
}

/// Releases one scalar allocation when the compiler emits a sized delete call.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="size">Unused compile-time size information.</param>
void operator delete(void* memory, std::size_t size) noexcept {
    (void)size;
    std::free(memory);
}

/// Releases one array allocation when the compiler emits a sized delete call.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="size">Unused compile-time size information.</param>
void operator delete[](void* memory, std::size_t size) noexcept {
    (void)size;
    std::free(memory);
}
