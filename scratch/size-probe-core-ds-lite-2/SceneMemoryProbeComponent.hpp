#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class SceneMemoryProbeStep;
class RuntimeMemoryCounters;
class Core;
class SceneManager;

#include "UpdateComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"

class SceneMemoryProbeComponent : public ::UpdateComponent
{
public:
    virtual ~SceneMemoryProbeComponent() = default;

    std::string ProbeName;

    const std::string& get_ProbeName();
    void set_ProbeName(std::string value);

    Array<::SceneMemoryProbeStep*>* Steps;

    Array<::SceneMemoryProbeStep*>* get_Steps();
    void set_Steps(Array<::SceneMemoryProbeStep*>* value);

    bool Loop;

    bool get_Loop();
    void set_Loop(bool value);

    bool StartAutomatically;

    bool get_StartAutomatically();
    void set_StartAutomatically(bool value);

    double InitialDelaySeconds;

    double get_InitialDelaySeconds();
    void set_InitialDelaySeconds(double value);

    int32_t get_CurrentStepIndex();

    int32_t get_CurrentCycleIndex();

    bool get_HasStarted();

    bool get_IsCompleted();

    SceneMemoryProbeComponent();

    void StartProbe();

    void StopProbe();

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    ::RuntimeMemoryCounters* MemoryCountersValue;

    double CurrentStepElapsedSecondsValue;

    double InitialDelayElapsedSecondsValue;

    int32_t CurrentStepIndexValue;

    int32_t CurrentCycleIndexValue;

    bool StartedValue;

    bool CompletedValue;

    bool CurrentStepActionIssuedValue;

    void AdvanceSceneActionStep(::Core* core, ::SceneMemoryProbeStep* step);

    void AdvanceToNextStep();

    void AdvanceWaitStep(::Core* core, ::SceneMemoryProbeStep* step);

    static std::string BuildLoadedSceneIds(::SceneManager* sceneManager);

    void EmitMeasurement(::Core* core, ::SceneMemoryProbeStep* step);

    void ExecuteSceneAction(::Core* core, ::SceneMemoryProbeStep* step);

    void ResetRuntimeState();

    ::SceneMemoryProbeStep* ResolveCurrentStep();

    int32_t ResolveStepCount();

    void StartProbeIfNeeded(::Core* core);

    void ValidateConfiguration();
};
