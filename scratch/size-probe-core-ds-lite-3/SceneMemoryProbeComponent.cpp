#ifdef DrawText
#undef DrawText
#endif
#include "SceneMemoryProbeComponent.hpp"
#include "UpdateComponent.hpp"
#include "Core.hpp"
#include "SceneMemoryProbeStep.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneManager.hpp"
#include "runtime/native_string.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "RuntimeMemoryCounters.hpp"
#include "ObjectManager.hpp"
#include "SceneMemoryProbeMeasurement.hpp"
#include "Logger.hpp"
#include "NativeOwnership.hpp"
#include "system/text/string-builder.hpp"
#include "SceneMemoryProbeActionKind.hpp"
#include "SceneMemoryProbeComponent.hpp"
#include "SceneMemoryProbeLogFormatter.hpp"
#include "SceneLoadMode.hpp"
#include "runtime/array.hpp"
#include "Entity.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "FontAsset.hpp"
#include "int2.hpp"
#include "RenderManager2D.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "runtime/native_dictionary.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_list.hpp"
#include "LoadedSceneRecord.hpp"
#include "ISceneIdPathResolver.hpp"
#include "IRuntimeSceneTransitionDiagnosticsProvider.hpp"
#include "IRuntimeEntityDisposalDiagnosticsProvider.hpp"
#include "PendingSceneOperation.hpp"
#include "RuntimeTexture.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "SceneAsset.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "SceneEntityAsset.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "IRuntimeDiagnosticsProvider.hpp"
#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "IUpdateable.hpp"
#include "IDrawable2D.hpp"
#include "IDrawable3D.hpp"
#include "ICamera.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "IInteractable2D.hpp"
#include "PendingUpdateOperation.hpp"
#include "ICameraBoundViewportOwner.hpp"
#include "LogLevel.hpp"
#include "runtime/array.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "system/number.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "system/text/string-builder.hpp"

const std::string& SceneMemoryProbeComponent::get_ProbeName()
{
return this->ProbeName;
}

void SceneMemoryProbeComponent::set_ProbeName(std::string value)
{
this->ProbeName = value;
}

Array<::SceneMemoryProbeStep*>* SceneMemoryProbeComponent::get_Steps()
{
return this->Steps;
}

void SceneMemoryProbeComponent::set_Steps(Array<::SceneMemoryProbeStep*>* value)
{
this->Steps = value;
}

bool SceneMemoryProbeComponent::get_Loop()
{
return this->Loop;
}

void SceneMemoryProbeComponent::set_Loop(bool value)
{
this->Loop = value;
}

bool SceneMemoryProbeComponent::get_StartAutomatically()
{
return this->StartAutomatically;
}

void SceneMemoryProbeComponent::set_StartAutomatically(bool value)
{
this->StartAutomatically = value;
}

double SceneMemoryProbeComponent::get_InitialDelaySeconds()
{
return this->InitialDelaySeconds;
}

void SceneMemoryProbeComponent::set_InitialDelaySeconds(double value)
{
this->InitialDelaySeconds = value;
}

int32_t SceneMemoryProbeComponent::get_CurrentStepIndex()
{
return this->CurrentStepIndexValue;}

int32_t SceneMemoryProbeComponent::get_CurrentCycleIndex()
{
return this->CurrentCycleIndexValue;}

bool SceneMemoryProbeComponent::get_HasStarted()
{
return this->StartedValue;}

bool SceneMemoryProbeComponent::get_IsCompleted()
{
return this->CompletedValue;}

SceneMemoryProbeComponent::SceneMemoryProbeComponent() : ProbeName(), Steps(), Loop(), StartAutomatically(), InitialDelaySeconds(0), MemoryCountersValue(), CurrentStepElapsedSecondsValue(0), InitialDelayElapsedSecondsValue(0), CurrentStepIndexValue(0), CurrentCycleIndexValue(0), StartedValue(), CompletedValue(), CurrentStepActionIssuedValue()
{
this->MemoryCountersValue = new ::RuntimeMemoryCounters();
this->set_Steps(Array<SceneMemoryProbeStep*>::Empty());
this->set_ProbeName(String::Empty);
this->set_StartAutomatically(true);
}

void SceneMemoryProbeComponent::StartProbe()
{
this->ValidateConfiguration();
this->ResetRuntimeState();
this->StartedValue = true;
}

void SceneMemoryProbeComponent::StopProbe()
{
this->StartedValue = false;
this->CompletedValue = true;
this->CurrentStepActionIssuedValue = false;
this->CurrentStepElapsedSecondsValue = 0.0;
this->InitialDelayElapsedSecondsValue = 0.0;
}

void SceneMemoryProbeComponent::Update()
{
UpdateComponent::Update();
::Core *core = Core::Instance;
    if (core == nullptr)
    {
return;    }
    if (!this->StartedValue)
    {
this->StartProbeIfNeeded(core);
return;    }
    if (this->CompletedValue || this->ResolveStepCount() == 0)
    {
return;    }
::SceneMemoryProbeStep *currentStep = this->ResolveCurrentStep();
    if (currentStep->ActionKind == SceneMemoryProbeActionKind::Wait)
    {
this->AdvanceWaitStep(core, currentStep);
return;    }
this->AdvanceSceneActionStep(core, currentStep);
}

uint8_t SceneMemoryProbeComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void SceneMemoryProbeComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* SceneMemoryProbeComponent::get_Parent()
{
return Component::get_Parent();
}

void SceneMemoryProbeComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool SceneMemoryProbeComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* SceneMemoryProbeComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool SceneMemoryProbeComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void SceneMemoryProbeComponent::AdvanceSceneActionStep(::Core* core, ::SceneMemoryProbeStep* step)
{
    if (!this->CurrentStepActionIssuedValue)
    {
this->ExecuteSceneAction(core, step);
this->CurrentStepActionIssuedValue = true;
return;    }
this->EmitMeasurement(core, step);
this->AdvanceToNextStep();
}

void SceneMemoryProbeComponent::AdvanceToNextStep()
{
this->CurrentStepActionIssuedValue = false;
this->CurrentStepElapsedSecondsValue = 0.0;
this->CurrentStepIndexValue++;
    if (this->CurrentStepIndexValue < this->ResolveStepCount())
    {
return;    }
    if (!this->Loop || this->ResolveStepCount() == 0)
    {
this->CompletedValue = true;
return;    }
this->CurrentCycleIndexValue++;
this->CurrentStepIndexValue = 0;
}

void SceneMemoryProbeComponent::AdvanceWaitStep(::Core* core, ::SceneMemoryProbeStep* step)
{
this->CurrentStepElapsedSecondsValue += core->DeltaTime;
    if (this->CurrentStepElapsedSecondsValue < step->DurationSeconds)
    {
return;    }
this->EmitMeasurement(core, step);
this->AdvanceToNextStep();
}

std::string SceneMemoryProbeComponent::BuildLoadedSceneIds(::SceneManager* sceneManager)
{
    if (sceneManager == nullptr || sceneManager->get_LoadedScenes()->get_Count() == 0)
    {
return String::Empty;    }
StringBuilder *builder = new StringBuilder();
auto __localDeleteGuard_000001CD = he_cpp_make_scope_exit([&]() {
delete builder;
});
for (int32_t index = 0; index < sceneManager->get_LoadedScenes()->get_Count(); index++) {
    if (index > 0)
    {
builder->Append(static_cast<char>(','));
    }
builder->Append((*sceneManager->get_LoadedScenes()).get_Item(static_cast<int32_t>(index))->SceneId);
}
return builder->ToString();}

void SceneMemoryProbeComponent::EmitMeasurement(::Core* core, ::SceneMemoryProbeStep* step)
{
return;::RuntimeDiagnosticsService *diagnosticsService = core->RuntimeDiagnosticsService;
    if (diagnosticsService != nullptr)
    {
diagnosticsService->CaptureMemoryCounters(this->MemoryCountersValue);
    }
else {
this->MemoryCountersValue->Reset();
}
::SceneManager *sceneManager = core->SceneManager;
::ObjectManager *objectManager = core->ObjectManager;
::SceneMemoryProbeMeasurement *measurement = ([&]() {
auto __object_000001CE = new ::SceneMemoryProbeMeasurement();
__object_000001CE->set_ProbeName(this->ProbeName);
__object_000001CE->set_CycleIndex(this->CurrentCycleIndexValue);
__object_000001CE->set_StepIndex(this->CurrentStepIndexValue);
__object_000001CE->set_Label(step->Label);
__object_000001CE->set_ActionKind(step->ActionKind);
__object_000001CE->set_ResidentBytes(this->MemoryCountersValue->ResidentBytes);
__object_000001CE->set_CommittedBytes(this->MemoryCountersValue->CommittedBytes);
__object_000001CE->set_LoadedSceneIds(SceneMemoryProbeComponent::BuildLoadedSceneIds(sceneManager));
__object_000001CE->set_Drawables2DCount(objectManager == nullptr ? 0 : objectManager->Drawables2D->get_Count());
__object_000001CE->set_Drawables3DCount(objectManager == nullptr ? 0 : objectManager->Drawables3D->get_Count());
__object_000001CE->set_DrawCallCount(core->LastRenderManager3DDrawCallCount);
__object_000001CE->set_ActiveOwnedTextureCount(sceneManager == nullptr ? 0 : sceneManager->get_ActiveOwnedTextureReferenceCount());
__object_000001CE->set_ActiveOwnedFontCount(sceneManager == nullptr ? 0 : sceneManager->get_ActiveOwnedFontReferenceCount());
__object_000001CE->set_ActiveOwnedModelCount(sceneManager == nullptr ? 0 : sceneManager->get_ActiveOwnedModelReferenceCount());
__object_000001CE->set_ActiveOwnedMaterialCount(sceneManager == nullptr ? 0 : sceneManager->get_ActiveOwnedMaterialReferenceCount());
return __object_000001CE;
})();
Logger::WriteLine(SceneMemoryProbeLogFormatter::Format(measurement));
delete measurement;
}

void SceneMemoryProbeComponent::ExecuteSceneAction(::Core* core, ::SceneMemoryProbeStep* step)
{
::SceneManager *sceneManager = (core->SceneManager != nullptr ? core->SceneManager : throw new InvalidOperationException("Scene memory probes require one initialized scene manager."));
    if (String::IsNullOrWhiteSpace(step->SceneId))
    {
throw new InvalidOperationException(std::string("Scene memory probe step '") + std::to_string(this->CurrentStepIndexValue) + std::string("' requires one scene id for action '") + std::to_string(static_cast<int32_t>(step->ActionKind)) + std::string("'."));
    }
    if (step->ActionKind == SceneMemoryProbeActionKind::LoadSceneSingle)
    {
sceneManager->LoadScene(step->SceneId, static_cast<SceneLoadMode>(SceneLoadMode::Single));
return;    }
    if (step->ActionKind == SceneMemoryProbeActionKind::LoadSceneAdditive)
    {
sceneManager->LoadScene(step->SceneId, static_cast<SceneLoadMode>(SceneLoadMode::Additive));
return;    }
    if (step->ActionKind == SceneMemoryProbeActionKind::UnloadScene)
    {
sceneManager->UnloadScene(step->SceneId);
return;    }
throw new InvalidOperationException(std::string("Scene memory probe does not support scene action '") + std::to_string(static_cast<int32_t>(step->ActionKind)) + std::string("' as one scene-manager operation."));
}

void SceneMemoryProbeComponent::ResetRuntimeState()
{
this->CurrentStepElapsedSecondsValue = 0.0;
this->InitialDelayElapsedSecondsValue = 0.0;
this->CurrentStepIndexValue = 0;
this->CurrentCycleIndexValue = 0;
this->CurrentStepActionIssuedValue = false;
this->CompletedValue = false;
}

::SceneMemoryProbeStep* SceneMemoryProbeComponent::ResolveCurrentStep()
{
    if (this->CurrentStepIndexValue < 0 || this->CurrentStepIndexValue >= this->ResolveStepCount())
    {
throw new InvalidOperationException("Scene memory probe attempted to resolve one step outside the authored step range.");
    }
::SceneMemoryProbeStep *step = (*this->Steps)[this->CurrentStepIndexValue];
    if (step == nullptr)
    {
throw new InvalidOperationException(std::string("Scene memory probe step '") + std::to_string(this->CurrentStepIndexValue) + std::string("' must not be null."));
    }
return step;}

int32_t SceneMemoryProbeComponent::ResolveStepCount()
{
return this->Steps == nullptr ? 0 : this->Steps->get_Length();}

void SceneMemoryProbeComponent::StartProbeIfNeeded(::Core* core)
{
    if (core == nullptr)
    {
throw new ArgumentNullException("core");
    }
    if (!this->StartAutomatically)
    {
return;    }
this->ValidateConfiguration();
    if (this->ResolveStepCount() == 0)
    {
this->StartedValue = true;
this->CompletedValue = true;
return;    }
    if (this->InitialDelaySeconds <= 0.0)
    {
this->StartedValue = true;
return;    }
this->InitialDelayElapsedSecondsValue += core->DeltaTime;
    if (this->InitialDelayElapsedSecondsValue >= this->InitialDelaySeconds)
    {
this->StartedValue = true;
    }
}

void SceneMemoryProbeComponent::ValidateConfiguration()
{
    if (Number::IsNaN(this->InitialDelaySeconds) || Number::IsInfinity(this->InitialDelaySeconds) || this->InitialDelaySeconds < 0.0)
    {
throw new InvalidOperationException("Scene memory probe initial delay must be one finite non-negative value.");
    }
for (int32_t index = 0; index < this->ResolveStepCount(); index++) {
::SceneMemoryProbeStep *step = ((*this->Steps)[index] != nullptr ? (*this->Steps)[index] : throw new InvalidOperationException(std::string("Scene memory probe step '") + std::to_string(index) + std::string("' must not be null.")));
    if (Number::IsNaN(step->DurationSeconds) || Number::IsInfinity(step->DurationSeconds) || step->DurationSeconds < 0.0)
    {
throw new InvalidOperationException(std::string("Scene memory probe step '") + std::to_string(index) + std::string("' must define one finite non-negative duration."));
    }
}
}

