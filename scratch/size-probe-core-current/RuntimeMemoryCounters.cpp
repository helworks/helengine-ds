#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeMemoryCounters.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "RuntimeMemoryCounters.hpp"
#include "runtime/native_exceptions.hpp"

RuntimeMemoryCounters::RuntimeMemoryCounters() : ResidentBytes(), PeakResidentBytes(), CommittedBytes(), PeakCommittedBytes(), AvailablePhysicalBytes(), PageFaultCount()
{
}

uint64_t RuntimeMemoryCounters::get_ResidentBytes()
{
return this->ResidentBytes;
}

void RuntimeMemoryCounters::set_ResidentBytes(uint64_t value)
{
this->ResidentBytes = value;
}

uint64_t RuntimeMemoryCounters::get_PeakResidentBytes()
{
return this->PeakResidentBytes;
}

void RuntimeMemoryCounters::set_PeakResidentBytes(uint64_t value)
{
this->PeakResidentBytes = value;
}

uint64_t RuntimeMemoryCounters::get_CommittedBytes()
{
return this->CommittedBytes;
}

void RuntimeMemoryCounters::set_CommittedBytes(uint64_t value)
{
this->CommittedBytes = value;
}

uint64_t RuntimeMemoryCounters::get_PeakCommittedBytes()
{
return this->PeakCommittedBytes;
}

void RuntimeMemoryCounters::set_PeakCommittedBytes(uint64_t value)
{
this->PeakCommittedBytes = value;
}

uint64_t RuntimeMemoryCounters::get_AvailablePhysicalBytes()
{
return this->AvailablePhysicalBytes;
}

void RuntimeMemoryCounters::set_AvailablePhysicalBytes(uint64_t value)
{
this->AvailablePhysicalBytes = value;
}

uint64_t RuntimeMemoryCounters::get_PageFaultCount()
{
return this->PageFaultCount;
}

void RuntimeMemoryCounters::set_PageFaultCount(uint64_t value)
{
this->PageFaultCount = value;
}

void RuntimeMemoryCounters::CopyFromSnapshot(::RuntimeMemoryDiagnosticsSnapshot* snapshot)
{
    if (snapshot == nullptr)
    {
throw new ArgumentNullException("snapshot");
    }
this->set_ResidentBytes(snapshot->ResidentBytes);
this->set_PeakResidentBytes(snapshot->PeakResidentBytes);
this->set_CommittedBytes(snapshot->CommittedBytes);
this->set_PeakCommittedBytes(snapshot->PeakCommittedBytes);
this->set_AvailablePhysicalBytes(snapshot->AvailablePhysicalBytes);
this->set_PageFaultCount(snapshot->PageFaultCount);
}

void RuntimeMemoryCounters::Reset()
{
this->set_ResidentBytes(0u);
this->set_PeakResidentBytes(0u);
this->set_CommittedBytes(0u);
this->set_PeakCommittedBytes(0u);
this->set_AvailablePhysicalBytes(0u);
this->set_PageFaultCount(0u);
}

