#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeMemoryDiagnosticsSnapshot;

class RuntimeMemoryCounters
{
public:
    virtual ~RuntimeMemoryCounters() = default;

    RuntimeMemoryCounters();

    uint64_t ResidentBytes;

    uint64_t get_ResidentBytes();
    void set_ResidentBytes(uint64_t value);

    uint64_t PeakResidentBytes;

    uint64_t get_PeakResidentBytes();
    void set_PeakResidentBytes(uint64_t value);

    uint64_t CommittedBytes;

    uint64_t get_CommittedBytes();
    void set_CommittedBytes(uint64_t value);

    uint64_t PeakCommittedBytes;

    uint64_t get_PeakCommittedBytes();
    void set_PeakCommittedBytes(uint64_t value);

    uint64_t AvailablePhysicalBytes;

    uint64_t get_AvailablePhysicalBytes();
    void set_AvailablePhysicalBytes(uint64_t value);

    uint64_t PageFaultCount;

    uint64_t get_PageFaultCount();
    void set_PageFaultCount(uint64_t value);

    void CopyFromSnapshot(::RuntimeMemoryDiagnosticsSnapshot* snapshot);

    void Reset();
};
