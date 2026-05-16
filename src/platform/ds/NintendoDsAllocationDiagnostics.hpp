#pragma once

#include <cstddef>

namespace helengine::ds {
    /// Tracks allocator request sizes so DS fatal logs can report the exact failing heap request.
    class NintendoDsAllocationDiagnostics {
    public:
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
    };
}
