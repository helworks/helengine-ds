#pragma once

#include <cstddef>

namespace helengine::ds {
    /// Tracks allocator request sizes so DS fatal logs can report the exact failing heap request.
    class NintendoDsAllocationDiagnostics {
    public:
        /// Stores the maximum number of small allocation caller sites retained without heap allocation.
        static constexpr std::size_t SmallAllocationSiteCapacity = 64;

        /// Captures one sampled small-allocation call site for fatal diagnostics.
        struct SmallAllocationSiteSnapshot {
            /// Raw return address captured from the global allocation entry point.
            void* CallerAddress;

            /// Second-level return address captured above the standard-library allocation wrapper when available.
            void* ParentCallerAddress;

            /// Allocation size associated with the sampled call site.
            std::size_t Size;

            /// Number of allocation requests observed at this call site.
            std::size_t RequestCount;

            /// Number of live allocations retained at this call site.
            std::size_t LiveCount;

            /// Highest live allocation count observed for this call site.
            std::size_t PeakLiveCount;

            /// Current live bytes retained by this allocation site.
            std::size_t LiveBytes;
        };

        /// Captures one live allocation-size bucket without requiring caller-site retention.
        struct LiveAllocationSizeSnapshot {
            /// Allocation size represented by this bucket.
            std::size_t Size;

            /// Number of live allocations currently held for this size.
            std::size_t LiveCount;

            /// Total live bytes currently held for this size.
            std::size_t LiveBytes;
        };

        /// Records one allocation request before the allocator attempts to satisfy it.
        /// <param name="size">Requested allocation size in bytes.</param>
        static void RecordRequest(std::size_t size);

        /// Records one allocation success after the allocator returned memory.
        /// <param name="size">Satisfied allocation size in bytes.</param>
        static void RecordSuccess(std::size_t size);

        /// Records one allocation failure before throwing <c>std::bad_alloc</c>.
        /// <param name="size">Failed allocation size in bytes.</param>
        static void RecordFailure(std::size_t size);

        /// Gets the last requested allocation size observed by the diagnostics hook.
        /// <returns>Last requested allocation size in bytes.</returns>
        static std::size_t GetLastRequestedSize();

        /// Gets the last successful allocation size observed by the diagnostics hook.
        /// <returns>Last successful allocation size in bytes.</returns>
        static std::size_t GetLastSuccessfulSize();

        /// Gets the last failed allocation size observed by the diagnostics hook.
        /// <returns>Last failed allocation size in bytes.</returns>
        static std::size_t GetLastFailedSize();

        /// Gets the running allocation-request count observed by the diagnostics hook.
        /// <returns>Total allocation request count seen so far.</returns>
        static std::size_t GetAllocationRequestCount();

        /// Gets the number of bytes currently held by live allocations routed through the DS diagnostics-aware allocator hook.
        /// <returns>Current live allocated byte count.</returns>
        static std::size_t GetCurrentAllocatedSize();

        /// Gets the highest live allocated byte count observed by the DS diagnostics-aware allocator hook.
        /// <returns>Peak live allocated byte count.</returns>
        static std::size_t GetPeakAllocatedSize();

        /// Gets the cumulative number of bytes successfully allocated through the DS diagnostics-aware allocator hook.
        /// <returns>Total allocated byte count.</returns>
        static std::size_t GetTotalAllocatedSize();

        /// Gets the cumulative number of bytes released through the DS diagnostics-aware allocator hook.
        /// <returns>Total freed byte count.</returns>
        static std::size_t GetTotalFreedSize();

        /// Gets the watched allocation size that receives dedicated live-count tracking in fatal diagnostics.
        /// <returns>Watched allocation size in bytes.</returns>
        static std::size_t GetWatchedAllocationSize();

        /// Gets the current live allocation count for the watched allocation size.
        /// <returns>Current live allocation count for the watched allocation size.</returns>
        static std::size_t GetWatchedLiveAllocationCount();

        /// Gets the highest live allocation count observed for the watched allocation size.
        /// <returns>Peak live allocation count for the watched allocation size.</returns>
        static std::size_t GetWatchedPeakLiveAllocationCount();

        /// Gets one fixed small-allocation call-site snapshot.
        /// <param name="index">Snapshot index in the fixed caller-site table.</param>
        /// <returns>Captured caller-site data, or an empty record when the index is out of range.</returns>
        static SmallAllocationSiteSnapshot GetSmallAllocationSiteSnapshot(std::size_t index);

        /// Gets one live allocation site ranked by current retained byte count.
        /// <param name="rank">Zero-based rank by live byte count.</param>
        /// <returns>Captured caller-site data, or an empty record when no site exists at that rank.</returns>
        static SmallAllocationSiteSnapshot GetTopLiveAllocationSiteSnapshot(std::size_t rank);

        /// Gets one allocation size bucket ranked by current retained byte count.
        /// <param name="rank">Zero-based rank by live byte count.</param>
        /// <returns>Captured allocation-size bucket data, or an empty record when no bucket exists at that rank.</returns>
        static LiveAllocationSizeSnapshot GetTopLiveAllocationSizeSnapshot(std::size_t rank);
    };
}
