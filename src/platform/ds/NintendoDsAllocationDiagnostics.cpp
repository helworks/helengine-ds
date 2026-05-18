#include "platform/ds/NintendoDsAllocationDiagnostics.hpp"

#include <cstdlib>
#include <new>

namespace helengine::ds {
    namespace {
        struct alignas(std::max_align_t) AllocationHeader {
            std::size_t Size;
            void* BasePointer;
        };

        std::size_t LastRequestedSizeValue = 0;
        std::size_t LastSuccessfulSizeValue = 0;
        std::size_t LastFailedSizeValue = 0;
        std::size_t AllocationRequestCountValue = 0;
        std::size_t CurrentAllocatedSizeValue = 0;
        std::size_t PeakAllocatedSizeValue = 0;
        std::size_t TotalAllocatedSizeValue = 0;
        std::size_t TotalFreedSizeValue = 0;

        /// Releases one tracked allocation size from the current live-byte counter.
        /// <param name="allocationSize">Previously tracked allocation size in bytes.</param>
        void ReleaseTrackedAllocation(std::size_t allocationSize) {
            if (allocationSize == 0) {
                return;
            }

            CurrentAllocatedSizeValue -= allocationSize;
            TotalFreedSizeValue += allocationSize;
        }

        /// Aligns one pointer-sized integer upward to the requested power-of-two alignment.
        /// <param name="value">Pointer-sized integer to align.</param>
        /// <param name="alignment">Requested alignment in bytes.</param>
        /// <returns>Aligned pointer-sized integer.</returns>
        std::size_t AlignUp(std::size_t value, std::size_t alignment) {
            return (value + (alignment - 1)) & ~(alignment - 1);
        }

        /// Allocates one tracked heap block honoring the requested alignment.
        /// <param name="size">Requested user allocation size in bytes.</param>
        /// <param name="alignment">Requested user alignment in bytes.</param>
        /// <param name="useNoThrow">True to return null on failure instead of throwing.</param>
        /// <returns>Aligned user pointer or null when <paramref name="useNoThrow"/> is true and allocation fails.</returns>
        void* AllocateTrackedMemory(std::size_t size, std::size_t alignment, bool useNoThrow) {
            NintendoDsAllocationDiagnostics::RecordRequest(size);
            std::size_t requestedAlignment = alignment < alignof(AllocationHeader)
                ? alignof(AllocationHeader)
                : alignment;
            std::size_t allocationSize = sizeof(AllocationHeader) + size + requestedAlignment - 1;
            void* basePointer = std::malloc(allocationSize);
            if (basePointer == nullptr) {
                NintendoDsAllocationDiagnostics::RecordFailure(size);
                if (useNoThrow) {
                    return nullptr;
                }

                throw std::bad_alloc();
            }

            std::size_t userPointerValue = AlignUp(
                reinterpret_cast<std::size_t>(basePointer) + sizeof(AllocationHeader),
                requestedAlignment);
            void* userPointer = reinterpret_cast<void*>(userPointerValue);
            AllocationHeader* header = static_cast<AllocationHeader*>(userPointer) - 1;
            header->Size = size;
            header->BasePointer = basePointer;
            NintendoDsAllocationDiagnostics::RecordSuccess(size);
            return userPointer;
        }

        /// Releases one tracked allocation and returns the underlying base allocation to the C runtime.
        /// <param name="memory">Previously allocated user pointer.</param>
        void FreeTrackedMemory(void* memory) noexcept {
            if (memory == nullptr) {
                return;
            }

            AllocationHeader* header = static_cast<AllocationHeader*>(memory) - 1;
            std::size_t allocationSize = header->Size;
            void* basePointer = header->BasePointer;
            ReleaseTrackedAllocation(allocationSize);
            std::free(basePointer);
        }
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
        CurrentAllocatedSizeValue += size;
        TotalAllocatedSizeValue += size;
        if (CurrentAllocatedSizeValue > PeakAllocatedSizeValue) {
            PeakAllocatedSizeValue = CurrentAllocatedSizeValue;
        }
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

    /// Gets the number of bytes currently held by live allocations routed through the DS diagnostics-aware allocator hook.
    /// <returns>Current live allocated byte count.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetCurrentAllocatedSize() {
        return CurrentAllocatedSizeValue;
    }

    /// Gets the highest live allocated byte count observed by the DS diagnostics-aware allocator hook.
    /// <returns>Peak live allocated byte count.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetPeakAllocatedSize() {
        return PeakAllocatedSizeValue;
    }

    /// Gets the cumulative number of bytes successfully allocated through the DS diagnostics-aware allocator hook.
    /// <returns>Total allocated byte count.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetTotalAllocatedSize() {
        return TotalAllocatedSizeValue;
    }

    /// Gets the cumulative number of bytes released through the DS diagnostics-aware allocator hook.
    /// <returns>Total freed byte count.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetTotalFreedSize() {
        return TotalFreedSizeValue;
    }
}

/// Routes global scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <returns>Allocated memory block.</returns>
void* operator new(std::size_t size) {
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), false);
}

/// Routes global array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <returns>Allocated memory block.</returns>
void* operator new[](std::size_t size) {
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), false);
}

/// Routes aligned scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="alignment">Requested allocation alignment.</param>
/// <returns>Allocated memory block.</returns>
void* operator new(std::size_t size, std::align_val_t alignment) {
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), false);
}

/// Routes aligned array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="alignment">Requested allocation alignment.</param>
/// <returns>Allocated memory block.</returns>
void* operator new[](std::size_t size, std::align_val_t alignment) {
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), false);
}

/// Routes nothrow scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="tag">Unused nothrow tag.</param>
/// <returns>Allocated memory block or null on failure.</returns>
void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), true);
}

/// Routes nothrow array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="tag">Unused nothrow tag.</param>
/// <returns>Allocated memory block or null on failure.</returns>
void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), true);
}

/// Routes aligned nothrow scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="alignment">Requested allocation alignment.</param>
/// <param name="tag">Unused nothrow tag.</param>
/// <returns>Allocated memory block or null on failure.</returns>
void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), true);
}

/// Routes aligned nothrow array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="alignment">Requested allocation alignment.</param>
/// <param name="tag">Unused nothrow tag.</param>
/// <returns>Allocated memory block or null on failure.</returns>
void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), true);
}

/// Releases one scalar allocation created by the diagnostics-aware allocator hook.
/// <param name="memory">Previously allocated memory block.</param>
void operator delete(void* memory) noexcept {
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one array allocation created by the diagnostics-aware allocator hook.
/// <param name="memory">Previously allocated memory block.</param>
void operator delete[](void* memory) noexcept {
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one scalar allocation when the compiler emits a sized delete call.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="size">Unused compile-time size information.</param>
void operator delete(void* memory, std::size_t size) noexcept {
    (void)size;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one array allocation when the compiler emits a sized delete call.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="size">Unused compile-time size information.</param>
void operator delete[](void* memory, std::size_t size) noexcept {
    (void)size;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one aligned scalar allocation created by the diagnostics-aware allocator hook.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="alignment">Unused alignment tag.</param>
void operator delete(void* memory, std::align_val_t alignment) noexcept {
    (void)alignment;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one aligned array allocation created by the diagnostics-aware allocator hook.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="alignment">Unused alignment tag.</param>
void operator delete[](void* memory, std::align_val_t alignment) noexcept {
    (void)alignment;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one aligned scalar allocation when the compiler emits a sized aligned delete call.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="size">Unused compile-time size information.</param>
/// <param name="alignment">Unused alignment tag.</param>
void operator delete(void* memory, std::size_t size, std::align_val_t alignment) noexcept {
    (void)size;
    (void)alignment;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one aligned array allocation when the compiler emits a sized aligned delete call.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="size">Unused compile-time size information.</param>
/// <param name="alignment">Unused alignment tag.</param>
void operator delete[](void* memory, std::size_t size, std::align_val_t alignment) noexcept {
    (void)size;
    (void)alignment;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one scalar allocation when the compiler emits the nothrow delete form.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="tag">Unused nothrow tag.</param>
void operator delete(void* memory, const std::nothrow_t& tag) noexcept {
    (void)tag;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one array allocation when the compiler emits the nothrow delete form.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="tag">Unused nothrow tag.</param>
void operator delete[](void* memory, const std::nothrow_t& tag) noexcept {
    (void)tag;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one aligned scalar allocation when the compiler emits the aligned nothrow delete form.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="alignment">Unused alignment tag.</param>
/// <param name="tag">Unused nothrow tag.</param>
void operator delete(void* memory, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)alignment;
    (void)tag;
    helengine::ds::FreeTrackedMemory(memory);
}

/// Releases one aligned array allocation when the compiler emits the aligned nothrow delete form.
/// <param name="memory">Previously allocated memory block.</param>
/// <param name="alignment">Unused alignment tag.</param>
/// <param name="tag">Unused nothrow tag.</param>
void operator delete[](void* memory, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)alignment;
    (void)tag;
    helengine::ds::FreeTrackedMemory(memory);
}
