#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRuntimeDiagnosticsProvider;
class SceneManager;
class ObjectManager;
class RuntimeMemoryCounters;
class RuntimeMemoryDiagnosticsSnapshot;

#include "runtime/native_string.hpp"

class RuntimeDiagnosticsService
{
public:
    virtual ~RuntimeDiagnosticsService() = default;

    void CaptureMemoryCounters(::RuntimeMemoryCounters* counters);

    ::RuntimeMemoryDiagnosticsSnapshot* CaptureSnapshot();

    RuntimeDiagnosticsService(::IRuntimeDiagnosticsProvider* runtimeDiagnosticsProvider, ::SceneManager* runtimeSceneManager, ::ObjectManager* runtimeObjectManager);
private:
    inline static const int32_t ReferenceSlotSizeInBytes = 8;

    ::IRuntimeDiagnosticsProvider* RuntimeDiagnosticsProvider;

    ::SceneManager* RuntimeSceneManager;

    ::ObjectManager* RuntimeObjectManager;

    void AppendCameraQueueMetrics(::RuntimeMemoryDiagnosticsSnapshot* snapshot);

    void AppendEngineCollectionMetrics(::RuntimeMemoryDiagnosticsSnapshot* snapshot);

    void AppendEntityHierarchyMetrics(::RuntimeMemoryDiagnosticsSnapshot* snapshot);

    static void AppendMetric(::RuntimeMemoryDiagnosticsSnapshot* snapshot, std::string name, uint64_t value);

    static uint64_t EstimateReferenceListBytes(int32_t capacity);
};
