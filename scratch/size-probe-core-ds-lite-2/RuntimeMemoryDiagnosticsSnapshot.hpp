#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeDiagnosticsMetric;

#include "runtime/native_disposable.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"

class RuntimeMemoryDiagnosticsSnapshot : public ::IDisposable
{
public:
    virtual ~RuntimeMemoryDiagnosticsSnapshot() = default;

    RuntimeMemoryDiagnosticsSnapshot();

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

    List<std::string>* TrackedSceneIds;

    List<std::string>* get_TrackedSceneIds();
    void set_TrackedSceneIds(List<std::string>* value);

    List<::RuntimeDiagnosticsMetric*>* DetailMetrics;

    List<::RuntimeDiagnosticsMetric*>* get_DetailMetrics();
    void set_DetailMetrics(List<::RuntimeDiagnosticsMetric*>* value);

    void Dispose();
};
