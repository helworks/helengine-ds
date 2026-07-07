#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "runtime/native_list.hpp"
#include "NativeOwnership.hpp"
#include "RuntimeDiagnosticsMetric.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "runtime/array.hpp"
#include "runtime/native_list.hpp"

RuntimeMemoryDiagnosticsSnapshot::RuntimeMemoryDiagnosticsSnapshot() : ResidentBytes(), PeakResidentBytes(), CommittedBytes(), PeakCommittedBytes(), AvailablePhysicalBytes(), PageFaultCount(), TrackedSceneIds(new List<std::string>()), DetailMetrics(new List<::RuntimeDiagnosticsMetric*>())
{
}

uint64_t RuntimeMemoryDiagnosticsSnapshot::get_ResidentBytes()
{
return this->ResidentBytes;
}

void RuntimeMemoryDiagnosticsSnapshot::set_ResidentBytes(uint64_t value)
{
this->ResidentBytes = value;
}

uint64_t RuntimeMemoryDiagnosticsSnapshot::get_PeakResidentBytes()
{
return this->PeakResidentBytes;
}

void RuntimeMemoryDiagnosticsSnapshot::set_PeakResidentBytes(uint64_t value)
{
this->PeakResidentBytes = value;
}

uint64_t RuntimeMemoryDiagnosticsSnapshot::get_CommittedBytes()
{
return this->CommittedBytes;
}

void RuntimeMemoryDiagnosticsSnapshot::set_CommittedBytes(uint64_t value)
{
this->CommittedBytes = value;
}

uint64_t RuntimeMemoryDiagnosticsSnapshot::get_PeakCommittedBytes()
{
return this->PeakCommittedBytes;
}

void RuntimeMemoryDiagnosticsSnapshot::set_PeakCommittedBytes(uint64_t value)
{
this->PeakCommittedBytes = value;
}

uint64_t RuntimeMemoryDiagnosticsSnapshot::get_AvailablePhysicalBytes()
{
return this->AvailablePhysicalBytes;
}

void RuntimeMemoryDiagnosticsSnapshot::set_AvailablePhysicalBytes(uint64_t value)
{
this->AvailablePhysicalBytes = value;
}

uint64_t RuntimeMemoryDiagnosticsSnapshot::get_PageFaultCount()
{
return this->PageFaultCount;
}

void RuntimeMemoryDiagnosticsSnapshot::set_PageFaultCount(uint64_t value)
{
this->PageFaultCount = value;
}

List<std::string>* RuntimeMemoryDiagnosticsSnapshot::get_TrackedSceneIds()
{
return this->TrackedSceneIds;
}

void RuntimeMemoryDiagnosticsSnapshot::set_TrackedSceneIds(List<std::string>* value)
{
this->TrackedSceneIds = value;
}

List<::RuntimeDiagnosticsMetric*>* RuntimeMemoryDiagnosticsSnapshot::get_DetailMetrics()
{
return this->DetailMetrics;
}

void RuntimeMemoryDiagnosticsSnapshot::set_DetailMetrics(List<::RuntimeDiagnosticsMetric*>* value)
{
this->DetailMetrics = value;
}

void RuntimeMemoryDiagnosticsSnapshot::Dispose()
{
    if (this->DetailMetrics != nullptr)
    {
for (int32_t index = 0; index < this->DetailMetrics->get_Count(); index++) {
delete (*this->DetailMetrics).get_Item(static_cast<int32_t>(index));
}
this->DetailMetrics->Clear();
delete this->DetailMetrics;
this->set_DetailMetrics(nullptr);
    }
    if (this->TrackedSceneIds != nullptr)
    {
this->TrackedSceneIds->Clear();
delete this->TrackedSceneIds;
this->set_TrackedSceneIds(nullptr);
    }
}

