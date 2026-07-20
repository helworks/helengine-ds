#include "platform/ds/NintendoDsAllocationDiagnostics.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <new>
#include <unistd.h>

extern "C" {
    extern char __eheap_end[];
}

namespace helengine::ds {
    namespace {
        /// Captures one allocator heap snapshot including the still-expandable DS heap tail.
        /// <returns>Heap usage and capacity values derived from libc plus the linker heap end.</returns>
        NintendoDsAllocationDiagnostics::HeapSnapshot CaptureHeapSnapshot() {
            struct mallinfo heapInfo = mallinfo();
            std::size_t expandableBytes = 0;
            void* currentHeapBreak = sbrk(0);
            if (currentHeapBreak != reinterpret_cast<void*>(-1)) {
                std::uintptr_t currentHeapBreakValue = reinterpret_cast<std::uintptr_t>(currentHeapBreak);
                std::uintptr_t heapEndValue = reinterpret_cast<std::uintptr_t>(__eheap_end);
                if (currentHeapBreakValue < heapEndValue) {
                    expandableBytes = static_cast<std::size_t>(heapEndValue - currentHeapBreakValue);
                }
            }

            return NintendoDsAllocationDiagnostics::HeapSnapshot {
                heapInfo.arena,
                heapInfo.uordblks,
                heapInfo.fordblks,
                expandableBytes,
                heapInfo.arena + expandableBytes,
                heapInfo.fordblks + expandableBytes
            };
        }
    }

    /// Gets one libc heap snapshot for fatal diagnostics.
    /// <returns>Heap usage and remaining-capacity information.</returns>
    NintendoDsAllocationDiagnostics::HeapSnapshot NintendoDsAllocationDiagnostics::GetHeapSnapshot() {
        return CaptureHeapSnapshot();
    }
}

#if HELENGINE_DS_ENABLE_ALLOCATION_DIAGNOSTICS
namespace helengine::ds {
    namespace {
        constexpr std::size_t WatchedAllocationSizeValue = 512;
        constexpr std::size_t TrackedLiveAllocationSizeCapacity = 4097;

        struct alignas(std::max_align_t) AllocationHeader {
            std::size_t Size;
            void* BasePointer;
            void* CallerAddress;
        };

        struct SmallAllocationSiteRecord {
            void* CallerAddress;
            void* ParentCallerAddress;
            std::size_t Size;
            std::size_t RequestCount;
            std::size_t LiveCount;
            std::size_t PeakLiveCount;
        };

        std::size_t LastRequestedSizeValue = 0;
        std::size_t LastSuccessfulSizeValue = 0;
        std::size_t LastFailedSizeValue = 0;
        std::size_t AllocationRequestCountValue = 0;
        std::size_t CurrentAllocatedSizeValue = 0;
        std::size_t PeakAllocatedSizeValue = 0;
        std::size_t TotalAllocatedSizeValue = 0;
        std::size_t TotalFreedSizeValue = 0;
        std::size_t WatchedLiveAllocationCountValue = 0;
        std::size_t WatchedPeakLiveAllocationCountValue = 0;
        std::array<SmallAllocationSiteRecord, NintendoDsAllocationDiagnostics::SmallAllocationSiteCapacity> SmallAllocationSiteRecords {};
        std::array<std::size_t, TrackedLiveAllocationSizeCapacity> LiveAllocationCountBySize {};

        /// Records one live allocation-size bucket when the size is small enough to retain exactly.
        /// <param name="size">Satisfied allocation size in bytes.</param>
        void RecordLiveAllocationSize(std::size_t size) {
            if (size < LiveAllocationCountBySize.size()) {
                LiveAllocationCountBySize[size]++;
            }
        }

        /// Releases one live allocation-size bucket when the size is small enough to retain exactly.
        /// <param name="size">Previously satisfied allocation size in bytes.</param>
        void ReleaseLiveAllocationSize(std::size_t size) {
            if (size < LiveAllocationCountBySize.size() && LiveAllocationCountBySize[size] > 0) {
                LiveAllocationCountBySize[size]--;
            }
        }

        /// Finds or reserves one fixed small-allocation caller record.
        /// <param name="size">Requested allocation size in bytes.</param>
        /// <param name="callerAddress">Return address captured at the allocation entry point.</param>
        /// <param name="parentCallerAddress">Second-level return address captured above the standard-library allocation wrapper.</param>
        /// <returns>Caller-site record, or null when the fixed table is full.</returns>
        SmallAllocationSiteRecord* FindSmallAllocationSiteRecord(std::size_t size, void* callerAddress, void* parentCallerAddress) {
            if (callerAddress == nullptr) {
                return nullptr;
            }

            SmallAllocationSiteRecord* firstEmptyRecord = nullptr;
            SmallAllocationSiteRecord* firstDeadRecord = nullptr;
            for (SmallAllocationSiteRecord& record : SmallAllocationSiteRecords) {
                if (record.CallerAddress == callerAddress && record.Size == size) {
                    return &record;
                } else if (record.CallerAddress == nullptr && firstEmptyRecord == nullptr) {
                    firstEmptyRecord = &record;
                } else if (record.LiveCount == 0 && firstDeadRecord == nullptr) {
                    firstDeadRecord = &record;
                }
            }

            SmallAllocationSiteRecord* targetRecord = firstEmptyRecord != nullptr ? firstEmptyRecord : firstDeadRecord;
            if (targetRecord != nullptr) {
                *targetRecord = SmallAllocationSiteRecord { callerAddress, parentCallerAddress, size, 0, 0, 0 };
            }
            return targetRecord;
        }

        /// Records one sampled small allocation request for the captured call site.
        /// <param name="size">Requested allocation size in bytes.</param>
        /// <param name="callerAddress">Return address captured at the allocation entry point.</param>
        /// <param name="parentCallerAddress">Second-level return address captured above the standard-library allocation wrapper.</param>
        void RecordSmallAllocationSiteRequest(std::size_t size, void* callerAddress, void* parentCallerAddress) {
            SmallAllocationSiteRecord* record = FindSmallAllocationSiteRecord(size, callerAddress, parentCallerAddress);
            if (record == nullptr) {
                return;
            }

            record->RequestCount++;
        }

        /// Records one sampled small allocation success for the captured call site.
        /// <param name="size">Satisfied allocation size in bytes.</param>
        /// <param name="callerAddress">Return address captured at the allocation entry point.</param>
        /// <param name="parentCallerAddress">Second-level return address captured above the standard-library allocation wrapper.</param>
        void RecordSmallAllocationSiteSuccess(std::size_t size, void* callerAddress, void* parentCallerAddress) {
            SmallAllocationSiteRecord* record = FindSmallAllocationSiteRecord(size, callerAddress, parentCallerAddress);
            if (record == nullptr) {
                return;
            }

            record->LiveCount++;
            if (record->LiveCount > record->PeakLiveCount) {
                record->PeakLiveCount = record->LiveCount;
            }
        }

        /// Releases one sampled small allocation from the captured call site.
        /// <param name="size">Previously satisfied allocation size in bytes.</param>
        /// <param name="callerAddress">Return address captured when the allocation was made.</param>
        void ReleaseSmallAllocationSite(std::size_t size, void* callerAddress) {
            SmallAllocationSiteRecord* record = FindSmallAllocationSiteRecord(size, callerAddress, nullptr);
            if (record == nullptr || record->LiveCount == 0) {
                return;
            }

            record->LiveCount--;
        }

        /// Releases one tracked allocation size from the current live-byte counter.
        /// <param name="allocationSize">Previously tracked allocation size in bytes.</param>
        /// <param name="callerAddress">Return address captured when the allocation was made.</param>
        void ReleaseTrackedAllocation(std::size_t allocationSize, void* callerAddress) {
            if (allocationSize == 0) {
                return;
            }

            CurrentAllocatedSizeValue -= allocationSize;
            TotalFreedSizeValue += allocationSize;
            if (allocationSize == WatchedAllocationSizeValue && WatchedLiveAllocationCountValue > 0) {
                WatchedLiveAllocationCountValue--;
            }
            ReleaseLiveAllocationSize(allocationSize);
            ReleaseSmallAllocationSite(allocationSize, callerAddress);
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
        /// <param name="callerAddress">Return address captured at the global allocation entry point.</param>
        /// <param name="parentCallerAddress">Second-level return address captured above the standard-library allocation wrapper.</param>
        /// <returns>Aligned user pointer or null when <paramref name="useNoThrow"/> is true and allocation fails.</returns>
        void* AllocateTrackedMemory(std::size_t size, std::size_t alignment, bool useNoThrow, void* callerAddress, void* parentCallerAddress) {
            NintendoDsAllocationDiagnostics::RecordRequest(size);
            RecordSmallAllocationSiteRequest(size, callerAddress, parentCallerAddress);
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
            header->CallerAddress = callerAddress;
            NintendoDsAllocationDiagnostics::RecordSuccess(size);
            RecordLiveAllocationSize(size);
            RecordSmallAllocationSiteSuccess(size, callerAddress, parentCallerAddress);
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
            void* callerAddress = header->CallerAddress;
            ReleaseTrackedAllocation(allocationSize, callerAddress);
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
        if (size == WatchedAllocationSizeValue) {
            WatchedLiveAllocationCountValue++;
            if (WatchedLiveAllocationCountValue > WatchedPeakLiveAllocationCountValue) {
                WatchedPeakLiveAllocationCountValue = WatchedLiveAllocationCountValue;
            }
        }
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

    /// Gets the watched allocation size that receives dedicated live-count tracking in fatal diagnostics.
    /// <returns>Watched allocation size in bytes.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetWatchedAllocationSize() {
        return WatchedAllocationSizeValue;
    }

    /// Gets the current live allocation count for the watched allocation size.
    /// <returns>Current live allocation count for the watched allocation size.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetWatchedLiveAllocationCount() {
        return WatchedLiveAllocationCountValue;
    }

    /// Gets the highest live allocation count observed for the watched allocation size.
    /// <returns>Peak live allocation count for the watched allocation size.</returns>
    std::size_t NintendoDsAllocationDiagnostics::GetWatchedPeakLiveAllocationCount() {
        return WatchedPeakLiveAllocationCountValue;
    }

    /// Gets one fixed small-allocation call-site snapshot.
    /// <param name="index">Snapshot index in the fixed caller-site table.</param>
    /// <returns>Captured caller-site data, or an empty record when the index is out of range.</returns>
    NintendoDsAllocationDiagnostics::SmallAllocationSiteSnapshot NintendoDsAllocationDiagnostics::GetSmallAllocationSiteSnapshot(std::size_t index) {
        if (index >= SmallAllocationSiteCapacity) {
            return SmallAllocationSiteSnapshot { nullptr, nullptr, 0, 0, 0, 0, 0 };
        }

        const SmallAllocationSiteRecord& record = SmallAllocationSiteRecords[index];
        return SmallAllocationSiteSnapshot {
            record.CallerAddress,
            record.ParentCallerAddress,
            record.Size,
            record.RequestCount,
            record.LiveCount,
            record.PeakLiveCount,
            record.LiveCount * record.Size
        };
    }

    /// Gets one live allocation site ranked by current retained byte count.
    /// <param name="rank">Zero-based rank by live byte count.</param>
    /// <returns>Captured caller-site data, or an empty record when no site exists at that rank.</returns>
    NintendoDsAllocationDiagnostics::SmallAllocationSiteSnapshot NintendoDsAllocationDiagnostics::GetTopLiveAllocationSiteSnapshot(std::size_t rank) {
        std::size_t selectedRank = 0;
        for (std::size_t ignoredRank = 0; ignoredRank <= rank; ignoredRank++) {
            std::size_t bestIndex = SmallAllocationSiteCapacity;
            std::size_t bestLiveBytes = 0;
            for (std::size_t index = 0; index < SmallAllocationSiteCapacity; index++) {
                const SmallAllocationSiteRecord& record = SmallAllocationSiteRecords[index];
                std::size_t liveBytes = record.LiveCount * record.Size;
                if (liveBytes == 0 || liveBytes < bestLiveBytes) {
                    continue;
                }

                if (liveBytes == bestLiveBytes && bestIndex != SmallAllocationSiteCapacity && index <= bestIndex) {
                    continue;
                }

                std::size_t higherRankCount = 0;
                for (std::size_t comparisonIndex = 0; comparisonIndex < SmallAllocationSiteCapacity; comparisonIndex++) {
                    const SmallAllocationSiteRecord& comparisonRecord = SmallAllocationSiteRecords[comparisonIndex];
                    std::size_t comparisonLiveBytes = comparisonRecord.LiveCount * comparisonRecord.Size;
                    if (comparisonLiveBytes > liveBytes || (comparisonLiveBytes == liveBytes && comparisonIndex < index)) {
                        higherRankCount++;
                    }
                }

                if (higherRankCount == selectedRank) {
                    bestIndex = index;
                    bestLiveBytes = liveBytes;
                }
            }

            if (ignoredRank == rank) {
                if (bestIndex == SmallAllocationSiteCapacity) {
                    return SmallAllocationSiteSnapshot { nullptr, nullptr, 0, 0, 0, 0, 0 };
                }

                return GetSmallAllocationSiteSnapshot(bestIndex);
            }

            selectedRank++;
        }

        return SmallAllocationSiteSnapshot { nullptr, nullptr, 0, 0, 0, 0, 0 };
    }

    /// Gets one allocation size bucket ranked by current retained byte count.
    /// <param name="rank">Zero-based rank by live byte count.</param>
    /// <returns>Captured allocation-size bucket data, or an empty record when no bucket exists at that rank.</returns>
    NintendoDsAllocationDiagnostics::LiveAllocationSizeSnapshot NintendoDsAllocationDiagnostics::GetTopLiveAllocationSizeSnapshot(std::size_t rank) {
        for (std::size_t size = 1; size < LiveAllocationCountBySize.size(); size++) {
            std::size_t liveCount = LiveAllocationCountBySize[size];
            if (liveCount == 0) {
                continue;
            }

            std::size_t liveBytes = liveCount * size;
            std::size_t higherRankCount = 0;
            for (std::size_t comparisonSize = 1; comparisonSize < LiveAllocationCountBySize.size(); comparisonSize++) {
                std::size_t comparisonLiveCount = LiveAllocationCountBySize[comparisonSize];
                std::size_t comparisonLiveBytes = comparisonLiveCount * comparisonSize;
                if (comparisonLiveBytes > liveBytes || (comparisonLiveBytes == liveBytes && comparisonSize < size)) {
                    higherRankCount++;
                }
            }

            if (higherRankCount == rank) {
                return LiveAllocationSizeSnapshot { size, liveCount, liveBytes };
            }
        }

        return LiveAllocationSizeSnapshot { 0, 0, 0 };
    }
}

/// Routes global scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <returns>Allocated memory block.</returns>
void* operator new(std::size_t size) {
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), false, __builtin_return_address(0), __builtin_return_address(1));
}

/// Routes global array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <returns>Allocated memory block.</returns>
void* operator new[](std::size_t size) {
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), false, __builtin_return_address(0), __builtin_return_address(1));
}

/// Routes aligned scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="alignment">Requested allocation alignment.</param>
/// <returns>Allocated memory block.</returns>
void* operator new(std::size_t size, std::align_val_t alignment) {
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), false, __builtin_return_address(0), __builtin_return_address(1));
}

/// Routes aligned array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="alignment">Requested allocation alignment.</param>
/// <returns>Allocated memory block.</returns>
void* operator new[](std::size_t size, std::align_val_t alignment) {
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), false, __builtin_return_address(0), __builtin_return_address(1));
}

/// Routes nothrow scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="tag">Unused nothrow tag.</param>
/// <returns>Allocated memory block or null on failure.</returns>
void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), true, __builtin_return_address(0), __builtin_return_address(1));
}

/// Routes nothrow array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="tag">Unused nothrow tag.</param>
/// <returns>Allocated memory block or null on failure.</returns>
void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), true, __builtin_return_address(0), __builtin_return_address(1));
}

/// Routes aligned nothrow scalar allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="alignment">Requested allocation alignment.</param>
/// <param name="tag">Unused nothrow tag.</param>
/// <returns>Allocated memory block or null on failure.</returns>
void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), true, __builtin_return_address(0), __builtin_return_address(1));
}

/// Routes aligned nothrow array allocations through the DS diagnostics-aware allocator hook.
/// <param name="size">Requested allocation size in bytes.</param>
/// <param name="alignment">Requested allocation alignment.</param>
/// <param name="tag">Unused nothrow tag.</param>
/// <returns>Allocated memory block or null on failure.</returns>
void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), true, __builtin_return_address(0), __builtin_return_address(1));
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
#else
namespace helengine::ds {
    namespace {
        struct alignas(std::max_align_t) AllocationHeader {
            std::size_t Size;
            void* BasePointer;
        };

        std::size_t AlignUp(std::size_t value, std::size_t alignment) {
            return (value + (alignment - 1)) & ~(alignment - 1);
        }

        void* AllocateTrackedMemory(std::size_t size, std::size_t alignment, bool useNoThrow, void* callerAddress, void* parentCallerAddress) {
            (void)callerAddress;
            (void)parentCallerAddress;
            std::size_t requestedAlignment = alignment < alignof(AllocationHeader)
                ? alignof(AllocationHeader)
                : alignment;
            std::size_t allocationSize = sizeof(AllocationHeader) + size + requestedAlignment - 1;
            void* basePointer = std::malloc(allocationSize);
            if (basePointer == nullptr) {
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
            return userPointer;
        }

        void FreeTrackedMemory(void* memory) noexcept {
            if (memory == nullptr) {
                return;
            }

            AllocationHeader* header = static_cast<AllocationHeader*>(memory) - 1;
            std::free(header->BasePointer);
        }
    }

    void NintendoDsAllocationDiagnostics::RecordRequest(std::size_t size) {
        (void)size;
    }

    void NintendoDsAllocationDiagnostics::RecordSuccess(std::size_t size) {
        (void)size;
    }

    void NintendoDsAllocationDiagnostics::RecordFailure(std::size_t size) {
        (void)size;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetLastRequestedSize() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetLastSuccessfulSize() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetLastFailedSize() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetAllocationRequestCount() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetCurrentAllocatedSize() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetPeakAllocatedSize() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetTotalAllocatedSize() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetTotalFreedSize() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetWatchedAllocationSize() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetWatchedLiveAllocationCount() {
        return 0;
    }

    std::size_t NintendoDsAllocationDiagnostics::GetWatchedPeakLiveAllocationCount() {
        return 0;
    }

    NintendoDsAllocationDiagnostics::SmallAllocationSiteSnapshot NintendoDsAllocationDiagnostics::GetSmallAllocationSiteSnapshot(std::size_t index) {
        (void)index;
        return SmallAllocationSiteSnapshot {};
    }

    NintendoDsAllocationDiagnostics::SmallAllocationSiteSnapshot NintendoDsAllocationDiagnostics::GetTopLiveAllocationSiteSnapshot(std::size_t rank) {
        (void)rank;
        return SmallAllocationSiteSnapshot {};
    }

    NintendoDsAllocationDiagnostics::LiveAllocationSizeSnapshot NintendoDsAllocationDiagnostics::GetTopLiveAllocationSizeSnapshot(std::size_t rank) {
        (void)rank;
        return LiveAllocationSizeSnapshot {};
    }
}

void* operator new(std::size_t size) {
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), false, nullptr, nullptr);
}

void* operator new[](std::size_t size) {
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), false, nullptr, nullptr);
}

void* operator new(std::size_t size, std::align_val_t alignment) {
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), false, nullptr, nullptr);
}

void* operator new[](std::size_t size, std::align_val_t alignment) {
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), false, nullptr, nullptr);
}

void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), true, nullptr, nullptr);
}

void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, alignof(std::max_align_t), true, nullptr, nullptr);
}

void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), true, nullptr, nullptr);
}

void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)tag;
    return helengine::ds::AllocateTrackedMemory(size, static_cast<std::size_t>(alignment), true, nullptr, nullptr);
}

void operator delete(void* memory) noexcept {
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete[](void* memory) noexcept {
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete(void* memory, std::size_t size) noexcept {
    (void)size;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete[](void* memory, std::size_t size) noexcept {
    (void)size;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete(void* memory, std::align_val_t alignment) noexcept {
    (void)alignment;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete[](void* memory, std::align_val_t alignment) noexcept {
    (void)alignment;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete(void* memory, std::size_t size, std::align_val_t alignment) noexcept {
    (void)size;
    (void)alignment;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete[](void* memory, std::size_t size, std::align_val_t alignment) noexcept {
    (void)size;
    (void)alignment;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete(void* memory, const std::nothrow_t& tag) noexcept {
    (void)tag;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete[](void* memory, const std::nothrow_t& tag) noexcept {
    (void)tag;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete(void* memory, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)alignment;
    (void)tag;
    helengine::ds::FreeTrackedMemory(memory);
}

void operator delete[](void* memory, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
    (void)alignment;
    (void)tag;
    helengine::ds::FreeTrackedMemory(memory);
}
#endif
