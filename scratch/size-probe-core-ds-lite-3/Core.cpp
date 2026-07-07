#ifdef DrawText
#undef DrawText
#endif
#include "Core.hpp"
#include "runtime/native_exceptions.hpp"
#include "InputSystem.hpp"
#include "CoreInitializationOptions.hpp"
#include "StandardPlatformInput.hpp"
#include "ContentManager.hpp"
#include "RuntimeContentManagerConfiguration.hpp"
#include "IEntityFactory.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "FPSComponent.hpp"
#include "DebugComponent.hpp"
#include "SceneManager.hpp"
#include "RenderManager3D.hpp"
#include "RenderManager2D.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "RuntimeExecutionPhaseProbe.hpp"
#include "ObjectManager.hpp"
#include "PointerInteractionSystem.hpp"
#include "IRuntimeSceneTransitionDiagnosticsProvider.hpp"
#include "IRuntimeEntityDisposalDiagnosticsProvider.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "NullTextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "int2.hpp"
#include "RuntimeEntityFactory.hpp"
#include "Core.hpp"
#include "FontAsset.hpp"
#include "PlatformInfo.hpp"
#include "IPhysicsRuntime.hpp"
#include "ITextClipboardService.hpp"
#include "runtime/native_dictionary.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "InputFrameState.hpp"
#include "runtime/native_list.hpp"
#include "InputBinding.hpp"
#include "InputActionState.hpp"
#include "MouseState.hpp"
#include "KeyboardState.hpp"
#include "system/action.hpp"
#include "InputContextId.hpp"
#include "InputActionId.hpp"
#include "InputGamepadState.hpp"
#include "ButtonState.hpp"
#include "InputPointerState.hpp"
#include "InputTextState.hpp"
#include "Keys.hpp"
#include "InputGamepadButton.hpp"
#include "ISceneIdPathResolver.hpp"
#include "IRuntimeDiagnosticsProvider.hpp"
#include "StandardPlatformInputConfiguration.hpp"
#include "StandardPlatformAction.hpp"
#include "ContentProcessorRegistration.hpp"
#include "runtime/native_type.hpp"
#include "IContentProcessor_1.hpp"
#include "runtime/array.hpp"
#include "Entity.hpp"
#include "TextComponent.hpp"
#include "RuntimeMemoryCounters.hpp"
#include "runtime/native_event.hpp"
#include "LoadedSceneRecord.hpp"
#include "PendingSceneOperation.hpp"
#include "RuntimeTexture.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
#include "SceneLoadMode.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "SceneAsset.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "SceneEntityAsset.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "ModelAsset.hpp"
#include "RenderTarget.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "TextureAsset.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
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
#include "PointerCursorKind.hpp"
#include "float4.hpp"
#include "AnimationClipAsset.hpp"
#include "SceneAssetReference.hpp"
#include "RuntimeSceneLoadResult.hpp"
#include "Component.hpp"
#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "TextBoxShortcutBinding.hpp"
#include "system/string_comparer.hpp"
#include "runtime/native_datetime.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_datetime.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/number.hpp"
#include "system/io/path.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "system/string_comparer.hpp"

::Core* Core::Instance;

::Core* Core::get_Instance()
{
return Core::Instance;
}

void Core::set_Instance(::Core* value)
{
Core::Instance = value;
}

::CoreInitializationOptions* Core::get_InitializationOptions()
{
return this->InitializationOptions;
}

void Core::set_InitializationOptions(::CoreInitializationOptions* value)
{
this->InitializationOptions = value;
}

::ContentManager* Core::get_ContentManager()
{
return this->GetContentManager();
}

::ObjectManager* Core::get_ObjectManager()
{
return this->ObjectManager;
}

void Core::set_ObjectManager(::ObjectManager* value)
{
this->ObjectManager = value;
}

::IEntityFactory* Core::get_EntityFactory()
{
return this->EntityFactory;
}

void Core::set_EntityFactory(::IEntityFactory* value)
{
this->EntityFactory = value;
}

::RenderManager3D* Core::get_RenderManager3D()
{
return this->RenderManager3D;
}

void Core::set_RenderManager3D(::RenderManager3D* value)
{
this->RenderManager3D = value;
}

double Core::get_LastRenderManager3DDrawMilliseconds()
{
return this->LastRenderManager3DDrawMilliseconds;
}

void Core::set_LastRenderManager3DDrawMilliseconds(double value)
{
this->LastRenderManager3DDrawMilliseconds = value;
}

int32_t Core::get_LastRenderManager3DDrawCallCount()
{
return this->LastRenderManager3DDrawCallCount;
}

void Core::set_LastRenderManager3DDrawCallCount(int32_t value)
{
this->LastRenderManager3DDrawCallCount = value;
}

bool Core::get_UsesPerformanceOverlayMetrics()
{
return this->UsesPerformanceOverlayMetrics;
}

void Core::set_UsesPerformanceOverlayMetrics(bool value)
{
this->UsesPerformanceOverlayMetrics = value;
}

double Core::get_PerformanceOverlayTriangleSetupMilliseconds()
{
return this->PerformanceOverlayTriangleSetupMilliseconds;
}

void Core::set_PerformanceOverlayTriangleSetupMilliseconds(double value)
{
this->PerformanceOverlayTriangleSetupMilliseconds = value;
}

double Core::get_PerformanceOverlayTrianglePrepMilliseconds()
{
return this->PerformanceOverlayTrianglePrepMilliseconds;
}

void Core::set_PerformanceOverlayTrianglePrepMilliseconds(double value)
{
this->PerformanceOverlayTrianglePrepMilliseconds = value;
}

double Core::get_PerformanceOverlayTriangleEmitMilliseconds()
{
return this->PerformanceOverlayTriangleEmitMilliseconds;
}

void Core::set_PerformanceOverlayTriangleEmitMilliseconds(double value)
{
this->PerformanceOverlayTriangleEmitMilliseconds = value;
}

double Core::get_PerformanceOverlayPacketEncodeMilliseconds()
{
return this->PerformanceOverlayPacketEncodeMilliseconds;
}

void Core::set_PerformanceOverlayPacketEncodeMilliseconds(double value)
{
this->PerformanceOverlayPacketEncodeMilliseconds = value;
}

double Core::get_PerformanceOverlaySubmitMilliseconds()
{
return this->PerformanceOverlaySubmitMilliseconds;
}

void Core::set_PerformanceOverlaySubmitMilliseconds(double value)
{
this->PerformanceOverlaySubmitMilliseconds = value;
}

double Core::get_PerformanceOverlayWaitMilliseconds()
{
return this->PerformanceOverlayWaitMilliseconds;
}

void Core::set_PerformanceOverlayWaitMilliseconds(double value)
{
this->PerformanceOverlayWaitMilliseconds = value;
}

int32_t Core::get_PerformanceOverlaySubmittedTriangleCount()
{
return this->PerformanceOverlaySubmittedTriangleCount;
}

void Core::set_PerformanceOverlaySubmittedTriangleCount(int32_t value)
{
this->PerformanceOverlaySubmittedTriangleCount = value;
}

int32_t Core::get_PerformanceOverlayDispatchCount()
{
return this->PerformanceOverlayDispatchCount;
}

void Core::set_PerformanceOverlayDispatchCount(int32_t value)
{
this->PerformanceOverlayDispatchCount = value;
}

const std::string& Core::get_PerformanceOverlayUpdateText()
{
return this->PerformanceOverlayUpdateText;
}

void Core::set_PerformanceOverlayUpdateText(std::string value)
{
this->PerformanceOverlayUpdateText = value;
}

const std::string& Core::get_PerformanceOverlayRenderText()
{
return this->PerformanceOverlayRenderText;
}

void Core::set_PerformanceOverlayRenderText(std::string value)
{
this->PerformanceOverlayRenderText = value;
}

const std::string& Core::get_PerformanceOverlayDetailText()
{
return this->PerformanceOverlayDetailText;
}

void Core::set_PerformanceOverlayDetailText(std::string value)
{
this->PerformanceOverlayDetailText = value;
}

const std::string& Core::get_PerformanceOverlayAdditionalText()
{
return this->PerformanceOverlayAdditionalText;
}

void Core::set_PerformanceOverlayAdditionalText(std::string value)
{
this->PerformanceOverlayAdditionalText = value;
}

bool Core::get_UsesPlatformOwnedPerformanceOverlayPresentation()
{
return this->UsesPlatformOwnedPerformanceOverlayPresentation;
}

void Core::set_UsesPlatformOwnedPerformanceOverlayPresentation(bool value)
{
this->UsesPlatformOwnedPerformanceOverlayPresentation = value;
}

::FontAsset* Core::get_ResolvedPerformanceOverlayFont()
{
return this->ResolvedPerformanceOverlayFont;
}

void Core::set_ResolvedPerformanceOverlayFont(::FontAsset* value)
{
this->ResolvedPerformanceOverlayFont = value;
}

float Core::get_ResolvedPerformanceOverlayFontScale()
{
return this->ResolvedPerformanceOverlayFontScale;
}

void Core::set_ResolvedPerformanceOverlayFontScale(float value)
{
this->ResolvedPerformanceOverlayFontScale = value;
}

::int2 Core::get_ResolvedPerformanceOverlayPadding()
{
return this->ResolvedPerformanceOverlayPadding;
}

void Core::set_ResolvedPerformanceOverlayPadding(::int2 value)
{
this->ResolvedPerformanceOverlayPadding = value;
}

const std::string& Core::get_ResolvedPerformanceOverlayUpdateText()
{
return this->ResolvedPerformanceOverlayUpdateText;
}

void Core::set_ResolvedPerformanceOverlayUpdateText(std::string value)
{
this->ResolvedPerformanceOverlayUpdateText = value;
}

const std::string& Core::get_ResolvedPerformanceOverlayRenderText()
{
return this->ResolvedPerformanceOverlayRenderText;
}

void Core::set_ResolvedPerformanceOverlayRenderText(std::string value)
{
this->ResolvedPerformanceOverlayRenderText = value;
}

const std::string& Core::get_ResolvedPerformanceOverlayDetailText()
{
return this->ResolvedPerformanceOverlayDetailText;
}

void Core::set_ResolvedPerformanceOverlayDetailText(std::string value)
{
this->ResolvedPerformanceOverlayDetailText = value;
}

const std::string& Core::get_ResolvedPerformanceOverlayAdditionalText()
{
return this->ResolvedPerformanceOverlayAdditionalText;
}

void Core::set_ResolvedPerformanceOverlayAdditionalText(std::string value)
{
this->ResolvedPerformanceOverlayAdditionalText = value;
}

::RenderManager2D* Core::get_RenderManager2D()
{
return this->RenderManager2D;
}

void Core::set_RenderManager2D(::RenderManager2D* value)
{
this->RenderManager2D = value;
}

::InputSystem* Core::get_Input()
{
return this->Input;
}

void Core::set_Input(::InputSystem* value)
{
this->Input = value;
}

::InputSystem* Core::get_InputSystem()
{
return this->Input;
}

::StandardPlatformInput* Core::get_StandardPlatformInput()
{
return this->StandardPlatformInput;
}

void Core::set_StandardPlatformInput(::StandardPlatformInput* value)
{
this->StandardPlatformInput = value;
}

::PointerInteractionSystem* Core::get_PointerInteractionSystem()
{
return this->PointerInteractionSystem;
}

void Core::set_PointerInteractionSystem(::PointerInteractionSystem* value)
{
this->PointerInteractionSystem = value;
}

double Core::get_FrameDeltaSeconds()
{
return this->FrameDeltaSeconds;
}

void Core::set_FrameDeltaSeconds(double value)
{
this->FrameDeltaSeconds = value;
}

::PlatformInfo* Core::get_PlatformInfo()
{
return this->PlatformInfo;
}

void Core::set_PlatformInfo(::PlatformInfo* value)
{
this->PlatformInfo = value;
}

float Core::get_DeltaTime()
{
return this->DeltaTime;
}

void Core::set_DeltaTime(float value)
{
this->DeltaTime = value;
}

float Core::get_UnscaledDeltaTime()
{
return this->UnscaledDeltaTime;
}

void Core::set_UnscaledDeltaTime(float value)
{
this->UnscaledDeltaTime = value;
}

double Core::get_TotalElapsedSeconds()
{
return this->TotalElapsedSeconds;
}

void Core::set_TotalElapsedSeconds(double value)
{
this->TotalElapsedSeconds = value;
}

::PhysicsFixedStepScheduler* Core::get_PhysicsScheduler()
{
return this->PhysicsSchedulerValue;
}

::IPhysicsRuntime* Core::get_PhysicsRuntime()
{
return this->PhysicsRuntimeValue;
}

::RuntimeSceneAssetReferenceResolver* Core::get_SceneAssetReferenceResolver()
{
return this->SceneAssetReferenceResolver;
}

void Core::set_SceneAssetReferenceResolver(::RuntimeSceneAssetReferenceResolver* value)
{
this->SceneAssetReferenceResolver = value;
}

::RuntimeSceneLoadService* Core::get_SceneLoadService()
{
return this->SceneLoadService;
}

void Core::set_SceneLoadService(::RuntimeSceneLoadService* value)
{
this->SceneLoadService = value;
}

::SceneManager* Core::get_SceneManager()
{
return this->SceneManager;
}

void Core::set_SceneManager(::SceneManager* value)
{
this->SceneManager = value;
}

::RuntimeDiagnosticsService* Core::get_RuntimeDiagnosticsService()
{
return this->RuntimeDiagnosticsService;
}

void Core::set_RuntimeDiagnosticsService(::RuntimeDiagnosticsService* value)
{
this->RuntimeDiagnosticsService = value;
}

::RuntimeComponentRegistry* Core::get_SceneRuntimeComponentRegistry()
{
return this->SceneRuntimeComponentRegistry;
}

void Core::set_SceneRuntimeComponentRegistry(::RuntimeComponentRegistry* value)
{
this->SceneRuntimeComponentRegistry = value;
}

const std::string& Core::get_LastSceneTransitionStage()
{
return this->LastSceneTransitionStage;
}

void Core::set_LastSceneTransitionStage(std::string value)
{
this->LastSceneTransitionStage = value;
}

::ITextClipboardService* Core::get_TextClipboardService()
{
return this->TextClipboardServiceValue;
}

::TextBoxShortcutRegistry* Core::get_TextBoxShortcutRegistry()
{
return this->TextBoxShortcutRegistryValue;
}

void Core::AttachPhysicsRuntime(::IPhysicsRuntime* runtime)
{
    if (runtime == nullptr)
    {
throw new ArgumentNullException("runtime");
    }
this->PhysicsRuntimeValue = runtime;
this->PhysicsSchedulerValue->Reset();
}

void Core::CompleteFrameBoundary()
{
    if (this->SceneManager != nullptr)
    {
this->set_LastSceneTransitionStage("CompleteFrameBoundaryCommitBegin");
this->SceneManager->CommitPendingOperationsAtFrameBoundary();
this->set_LastSceneTransitionStage("CompleteFrameBoundaryCommitEnd");
    }
}

void Core::DetachPhysicsRuntime()
{
this->PhysicsRuntimeValue = nullptr;
this->PhysicsSchedulerValue->Reset();
}

void Core::Dispose()
{
    if (this->RenderManager3D != nullptr)
    {
this->RenderManager3D->Dispose();
    }
    if (this->RenderManager2D != nullptr)
    {
this->RenderManager2D->Dispose();
    }
this->set_RuntimeDiagnosticsService(nullptr);
}

void Core::Draw()
{
this->set_LastSceneTransitionStage("DrawBegin");
    if (this->InitializationOptions->CommitPendingSceneOperationsDuringDraw)
    {
this->set_LastSceneTransitionStage("BeforeCompleteFrameBoundary");
this->CompleteFrameBoundary();
this->set_LastSceneTransitionStage("AfterCompleteFrameBoundary");
    }
this->set_LastSceneTransitionStage("BeforeRenderManager3DDraw");
this->set_LastRenderManager3DDrawMilliseconds(this->MeasureRenderManager3DDrawMilliseconds());
this->set_LastSceneTransitionStage("AfterRenderManager3DDraw");
this->set_LastRenderManager3DDrawCallCount(this->RenderManager3D == nullptr ? 0 : this->RenderManager3D->get_LastDrawCallCount());
FPSComponent::RecordRenderFrame();
DebugComponent::RecordRenderFrame();
this->set_LastSceneTransitionStage("DrawEnd");
}

::ContentManager* Core::GetContentManager(std::string rootDirectory)
{
    if (String::IsNullOrWhiteSpace(rootDirectory))
    {
throw ([&]() {
auto __ctor_arg_000001B1 = "Root directory must be provided.";
auto __ctor_arg_000001B2 = "rootDirectory";
return new ArgumentException(__ctor_arg_000001B1, __ctor_arg_000001B2);
})();
    }
const std::string normalizedRootDirectory = Path::GetFullPath(rootDirectory);
// Lock omitted in TypeScript
::ContentManager* contentManager;
    if (this->ContentManagersByRootPath->TryGetValue(normalizedRootDirectory, contentManager))
    {
return contentManager;    }
contentManager = new ::ContentManager(normalizedRootDirectory);
this->ContentManagersByRootPath->Add(normalizedRootDirectory, contentManager);
return contentManager;}

::ContentManager* Core::GetContentManager()
{
return this->GetContentManager(this->InitializationOptions->ContentRootPath);}

void Core::Initialize(::RenderManager3D* render3D, ::RenderManager2D* render2D, ::IInputBackend* input, ::PlatformInfo* platformInfo)
{
this->Initialize(render3D, render2D, input, platformInfo, this->InitializationOptions);
}

void Core::Initialize(::RenderManager3D* render3D, ::RenderManager2D* render2D, ::IInputBackend* input, ::PlatformInfo* platformInfo, ::CoreInitializationOptions* options)
{
    if (platformInfo == nullptr)
    {
throw new ArgumentNullException("platformInfo");
    }
this->set_RenderManager3D(render3D);
this->set_RenderManager2D(render2D);
this->Input->SetBackend(input);
this->set_PlatformInfo(platformInfo);
    if (options == nullptr)
    {
throw new ArgumentNullException("options");
    }
options->Normalize();
this->set_InitializationOptions(options);
this->PhysicsSchedulerValue = this->CreatePhysicsScheduler(options);
this->StandardPlatformInput->Configure(options->StandardPlatformInputConfiguration);
this->set_ObjectManager(new ::ObjectManager(options));
this->set_EntityFactory(this->CreateEntityFactory());
    if (this->EntityFactory == nullptr)
    {
throw new InvalidOperationException("Core entity factory creation must return one factory instance.");
    }
::ContentManager *contentManager = this->GetContentManager();
RuntimeContentManagerConfiguration::ConfigureSharedAssetContentManager(contentManager);
this->set_SceneAssetReferenceResolver(new ::RuntimeSceneAssetReferenceResolver(contentManager, this->InitializationOptions->ContentRootPath));
this->set_SceneRuntimeComponentRegistry(RuntimeComponentRegistry::CreateDefault());
this->set_SceneLoadService(new ::RuntimeSceneLoadService(this->SceneAssetReferenceResolver, this->SceneRuntimeComponentRegistry));
this->set_SceneManager(this->CreateSceneManager(contentManager, this->InitializationOptions->SceneCatalog));
this->set_RuntimeDiagnosticsService(new ::RuntimeDiagnosticsService(this->InitializationOptions->RuntimeDiagnosticsProvider, this->SceneManager, this->ObjectManager));
    IRuntimeUpdateStageDiagnosticsProvider* stageDiagnosticsProvider = he_cpp_try_cast<IRuntimeUpdateStageDiagnosticsProvider>(this->InitializationOptions->RuntimeDiagnosticsProvider);
    if (stageDiagnosticsProvider != nullptr)
    {
this->UpdateStageDiagnosticsProviderValue = stageDiagnosticsProvider;
    }
else {
this->UpdateStageDiagnosticsProviderValue = nullptr;
}
}

Core::Core() : Core(new ::CoreInitializationOptions())
{
}

Core::Core(::CoreInitializationOptions* options) : InitializationOptions(), ObjectManager(), EntityFactory(), RenderManager3D(), LastRenderManager3DDrawMilliseconds(0), LastRenderManager3DDrawCallCount(0), UsesPerformanceOverlayMetrics(), PerformanceOverlayTriangleSetupMilliseconds(0), PerformanceOverlayTrianglePrepMilliseconds(0), PerformanceOverlayTriangleEmitMilliseconds(0), PerformanceOverlayPacketEncodeMilliseconds(0), PerformanceOverlaySubmitMilliseconds(0), PerformanceOverlayWaitMilliseconds(0), PerformanceOverlaySubmittedTriangleCount(0), PerformanceOverlayDispatchCount(0), PerformanceOverlayUpdateText(), PerformanceOverlayRenderText(), PerformanceOverlayDetailText(), PerformanceOverlayAdditionalText(), UsesPlatformOwnedPerformanceOverlayPresentation(), ResolvedPerformanceOverlayFont(), ResolvedPerformanceOverlayFontScale(), ResolvedPerformanceOverlayPadding(), ResolvedPerformanceOverlayUpdateText(), ResolvedPerformanceOverlayRenderText(), ResolvedPerformanceOverlayDetailText(), ResolvedPerformanceOverlayAdditionalText(), RenderManager2D(), Input(), StandardPlatformInput(), PointerInteractionSystem(), FrameDeltaSeconds(0), PlatformInfo(), DeltaTime(), UnscaledDeltaTime(), TotalElapsedSeconds(0), SceneAssetReferenceResolver(), SceneLoadService(), SceneManager(), RuntimeDiagnosticsService(), SceneRuntimeComponentRegistry(), LastSceneTransitionStage(), ContentManagersByRootPath(), ContentManagerLock(), PhysicsSchedulerValue(), PhysicsRuntimeValue(), TextClipboardServiceValue(), TextBoxShortcutRegistryValue(), UpdateStopwatchValue(), DrawStopwatchValue(), HasPreviousMeasuredUpdateSeconds(), PreviousMeasuredUpdateSeconds(0), UpdateStageDiagnosticsProviderValue()
{
    if (options == nullptr)
    {
throw new ArgumentNullException("options");
    }
this->ContentManagersByRootPath = new Dictionary<std::string, ::ContentManager*>(StringComparer::get_OrdinalIgnoreCase());
this->ContentManagerLock = new char[1];
Core::set_Instance(this);
this->set_InitializationOptions(options);
this->InitializationOptions->Normalize();
this->PhysicsSchedulerValue = this->CreatePhysicsScheduler(this->InitializationOptions);
this->set_Input(new ::InputSystem());
this->set_StandardPlatformInput(new ::StandardPlatformInput(this->Input));
this->set_PointerInteractionSystem(new ::PointerInteractionSystem(this, this->Input));
this->TextClipboardServiceValue = new ::NullTextClipboardService();
this->TextBoxShortcutRegistryValue = new ::TextBoxShortcutRegistry();
this->UpdateStopwatchValue = System::Diagnostics::Stopwatch::StartNew();
this->DrawStopwatchValue = new Stopwatch();
this->set_ResolvedPerformanceOverlayFontScale(1.0f);
this->set_ResolvedPerformanceOverlayPadding(::int2(static_cast<int32_t>(0), static_cast<int32_t>(0)));
this->set_ResolvedPerformanceOverlayUpdateText(String::Empty);
this->set_ResolvedPerformanceOverlayRenderText(String::Empty);
this->set_ResolvedPerformanceOverlayDetailText(String::Empty);
this->set_ResolvedPerformanceOverlayAdditionalText(String::Empty);
}

void Core::RegisterRuntimeComponentDeserializer(::IRuntimeComponentDeserializer* deserializer)
{
    if (deserializer == nullptr)
    {
throw new ArgumentNullException("deserializer");
    }
    if (this->SceneRuntimeComponentRegistry == nullptr)
    {
throw new InvalidOperationException("Core must be initialized before runtime component deserializers can be registered.");
    }
this->SceneRuntimeComponentRegistry->Register(deserializer);
}

void Core::ReportSceneTransitionStage(std::string stage)
{
this->set_LastSceneTransitionStage(stage);
if (this->UpdateStageDiagnosticsProviderValue != nullptr)
{
this->UpdateStageDiagnosticsProviderValue->ReportUpdateStage(this->LastSceneTransitionStage);
}
}

void Core::ResetPhysicsTimingState()
{
this->PhysicsSchedulerValue->Reset();
}

void Core::SetPerformanceOverlayMetrics(bool usesPerformanceOverlayMetrics, double triangleSetupMilliseconds, double trianglePrepMilliseconds, double triangleEmitMilliseconds, double packetEncodeMilliseconds, double submitMilliseconds, double waitMilliseconds, int32_t submittedTriangleCount, int32_t dispatchCount)
{
this->set_UsesPerformanceOverlayMetrics(usesPerformanceOverlayMetrics);
this->set_PerformanceOverlayTriangleSetupMilliseconds(triangleSetupMilliseconds);
this->set_PerformanceOverlayTrianglePrepMilliseconds(trianglePrepMilliseconds);
this->set_PerformanceOverlayTriangleEmitMilliseconds(triangleEmitMilliseconds);
this->set_PerformanceOverlayPacketEncodeMilliseconds(packetEncodeMilliseconds);
this->set_PerformanceOverlaySubmitMilliseconds(submitMilliseconds);
this->set_PerformanceOverlayWaitMilliseconds(waitMilliseconds);
this->set_PerformanceOverlaySubmittedTriangleCount(submittedTriangleCount);
this->set_PerformanceOverlayDispatchCount(dispatchCount);
this->set_PerformanceOverlayUpdateText(String::Empty);
this->set_PerformanceOverlayRenderText(String::Empty);
this->set_PerformanceOverlayDetailText(String::Empty);
this->set_PerformanceOverlayAdditionalText(String::Empty);
}

void Core::SetPerformanceOverlayTextRows(bool usesPerformanceOverlayMetrics, std::string updateText, std::string renderText, std::string detailText, std::string additionalText)
{
this->set_UsesPerformanceOverlayMetrics(usesPerformanceOverlayMetrics);
this->set_PerformanceOverlayUpdateText(usesPerformanceOverlayMetrics ? updateText : String::Empty);
this->set_PerformanceOverlayRenderText(usesPerformanceOverlayMetrics ? renderText : String::Empty);
this->set_PerformanceOverlayDetailText(usesPerformanceOverlayMetrics ? detailText : String::Empty);
this->set_PerformanceOverlayAdditionalText(usesPerformanceOverlayMetrics ? additionalText : String::Empty);
}

void Core::SetPlatformOwnedPerformanceOverlayPresentation(bool usesPlatformOwnedPresentation)
{
this->set_UsesPlatformOwnedPerformanceOverlayPresentation(usesPlatformOwnedPresentation);
    if (!usesPlatformOwnedPresentation)
    {
this->ClearResolvedPerformanceOverlayPresentation();
    }
}

void Core::SetResolvedPerformanceOverlayPresentation(::FontAsset* font, float fontScale, ::int2 padding, std::string updateText, std::string renderText, std::string detailText, std::string additionalText)
{
    if (!this->UsesPlatformOwnedPerformanceOverlayPresentation)
    {
this->ClearResolvedPerformanceOverlayPresentation();
return;    }
this->set_ResolvedPerformanceOverlayFont(font);
this->set_ResolvedPerformanceOverlayFontScale(fontScale);
this->set_ResolvedPerformanceOverlayPadding(padding);
this->set_ResolvedPerformanceOverlayUpdateText(updateText);
this->set_ResolvedPerformanceOverlayRenderText(renderText);
this->set_ResolvedPerformanceOverlayDetailText(detailText);
this->set_ResolvedPerformanceOverlayAdditionalText(additionalText);
}

void Core::SetTextClipboardService(::ITextClipboardService* clipboardService)
{
    if (clipboardService == nullptr)
    {
throw new ArgumentNullException("clipboardService");
    }
this->TextClipboardServiceValue = clipboardService;
}

void Core::Update()
{
const double currentMeasuredUpdateSeconds = this->GetCurrentMeasuredUpdateSeconds();
const double elapsedSeconds = this->ResolveMeasuredElapsedSeconds(currentMeasuredUpdateSeconds);
this->AdvanceUpdate(elapsedSeconds, currentMeasuredUpdateSeconds);
}

void Core::Update(double elapsedSeconds)
{
this->ValidateElapsedSeconds(elapsedSeconds);
const double currentMeasuredUpdateSeconds = this->GetCurrentMeasuredUpdateSeconds();
this->AdvanceUpdate(elapsedSeconds, currentMeasuredUpdateSeconds);
}

::IEntityFactory* Core::CreateEntityFactory()
{
return new ::RuntimeEntityFactory();}

double Core::GetCurrentMeasuredUpdateSeconds()
{
return this->UpdateStopwatchValue->get_Elapsed().get_TotalMilliseconds() / 1000.0;}

double Core::MeasureRenderManager3DDrawMilliseconds()
{
this->DrawStopwatchValue->Restart();
this->RenderManager3D->Draw();
this->DrawStopwatchValue->Stop();
return this->DrawStopwatchValue->get_Elapsed().get_TotalMilliseconds();}

void Core::AdvanceUpdate(double elapsedSeconds, double currentMeasuredUpdateSeconds)
{
const float elapsedSecondsFloat = static_cast<float>(elapsedSeconds);
this->set_FrameDeltaSeconds(elapsedSeconds);
this->set_UnscaledDeltaTime(elapsedSecondsFloat);
this->set_DeltaTime(elapsedSecondsFloat);
this->TotalElapsedSeconds += elapsedSeconds;
this->PreviousMeasuredUpdateSeconds = currentMeasuredUpdateSeconds;
this->HasPreviousMeasuredUpdateSeconds = true;
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::BeforeInputEarlyUpdatePhaseId));
const bool shouldRecordUpdateStages = this->UpdateStageDiagnosticsProviderValue != nullptr;
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("BeforeInputEarlyUpdate");
    }
this->Input->EarlyUpdate();
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::AfterInputEarlyUpdatePhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("AfterInputEarlyUpdate");
this->RecordUpdateStage("BeforeFpsRecordUpdateFrame");
    }
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::BeforeFpsRecordUpdateFramePhaseId));
FPSComponent::RecordUpdateFrame();
DebugComponent::RecordUpdateFrame();
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::AfterFpsRecordUpdateFramePhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("AfterFpsRecordUpdateFrame");
    }
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::BeforeObjectManagerUpdatePhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("BeforeObjectManagerUpdate");
    }
this->ObjectManager->Update();
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::AfterObjectManagerUpdatePhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("AfterObjectManagerUpdate");
    }
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::BeforeUpdatePhysicsPhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("BeforeUpdatePhysics");
    }
this->UpdatePhysics(elapsedSeconds);
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::AfterUpdatePhysicsPhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("AfterUpdatePhysics");
    }
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::BeforeInputUpdatePhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("BeforeInputUpdate");
    }
this->Input->Update();
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::AfterInputUpdatePhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("AfterInputUpdate");
this->RecordUpdateStage("BeforePointerInteractionSystemUpdate");
    }
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::BeforePointerInteractionSystemUpdatePhaseId));
this->PointerInteractionSystem->Update();
RuntimeExecutionPhaseProbe::SetCurrentPhaseId(static_cast<int32_t>(RuntimeExecutionPhaseProbe::AfterPointerInteractionSystemUpdatePhaseId));
    if (shouldRecordUpdateStages)
    {
this->RecordUpdateStage("AfterPointerInteractionSystemUpdate");
    }
}

void Core::ClearResolvedPerformanceOverlayPresentation()
{
this->set_ResolvedPerformanceOverlayFont(nullptr);
this->set_ResolvedPerformanceOverlayFontScale(1.0f);
this->set_ResolvedPerformanceOverlayPadding(::int2(static_cast<int32_t>(0), static_cast<int32_t>(0)));
this->set_ResolvedPerformanceOverlayUpdateText(String::Empty);
this->set_ResolvedPerformanceOverlayRenderText(String::Empty);
this->set_ResolvedPerformanceOverlayDetailText(String::Empty);
this->set_ResolvedPerformanceOverlayAdditionalText(String::Empty);
}

::PhysicsFixedStepScheduler* Core::CreatePhysicsScheduler(::CoreInitializationOptions* options)
{
    if (options == nullptr)
    {
throw new ArgumentNullException("options");
    }
return new ::PhysicsFixedStepScheduler(options->PhysicsFixedStepSeconds);}

::SceneManager* Core::CreateSceneManager(::ContentManager* contentManager, ::RuntimeSceneCatalog* sceneCatalog)
{
    if (contentManager == nullptr)
    {
throw new ArgumentNullException("contentManager");
    }
    if (sceneCatalog == nullptr && this->InitializationOptions->ScenePathResolver == nullptr)
    {
return nullptr;    }
::IRuntimeSceneTransitionDiagnosticsProvider *sceneTransitionDiagnosticsProvider = nullptr;
    IRuntimeSceneTransitionDiagnosticsProvider* transitionDiagnosticsProvider = he_cpp_try_cast<IRuntimeSceneTransitionDiagnosticsProvider>(this->InitializationOptions->RuntimeDiagnosticsProvider);
    if (transitionDiagnosticsProvider != nullptr)
    {
sceneTransitionDiagnosticsProvider = transitionDiagnosticsProvider;
    }
::IRuntimeEntityDisposalDiagnosticsProvider *entityDisposalDiagnosticsProvider = nullptr;
    IRuntimeEntityDisposalDiagnosticsProvider* disposalDiagnosticsProvider = he_cpp_try_cast<IRuntimeEntityDisposalDiagnosticsProvider>(this->InitializationOptions->RuntimeDiagnosticsProvider);
    if (disposalDiagnosticsProvider != nullptr)
    {
entityDisposalDiagnosticsProvider = disposalDiagnosticsProvider;
    }
return new ::SceneManager(sceneCatalog, contentManager, this->SceneLoadService, this->ObjectManager, this->InitializationOptions->ScenePathResolver, sceneTransitionDiagnosticsProvider, entityDisposalDiagnosticsProvider);}

void Core::RecordUpdateStage(std::string stage)
{
this->ReportSceneTransitionStage(stage);
}

double Core::ResolveMeasuredElapsedSeconds(double currentMeasuredUpdateSeconds)
{
    if (!this->HasPreviousMeasuredUpdateSeconds)
    {
return 0.0;    }
return currentMeasuredUpdateSeconds - this->PreviousMeasuredUpdateSeconds;}

void Core::UpdatePhysics(double elapsedSeconds)
{
    if (this->PhysicsRuntimeValue == nullptr)
    {
return;    }
this->PhysicsSchedulerValue->AddElapsedSeconds(elapsedSeconds);
int32_t consumedStepCount = 0;
while (consumedStepCount < this->InitializationOptions->PhysicsMaxStepsPerUpdate && this->PhysicsSchedulerValue->TryConsumeStep()) {
this->PhysicsRuntimeValue->Step(this->PhysicsSchedulerValue->StepSeconds);
consumedStepCount++;
}
}

void Core::ValidateElapsedSeconds(double elapsedSeconds)
{
    if (Number::IsNaN(elapsedSeconds) || Number::IsInfinity(elapsedSeconds))
    {
throw ([&]() {
auto __ctor_arg_000001B3 = "elapsedSeconds";
auto __ctor_arg_000001B4 = "Elapsed frame time must be finite.";
return new ArgumentOutOfRangeException(__ctor_arg_000001B3, __ctor_arg_000001B4);
})();
    }
    if (elapsedSeconds < 0.0)
    {
throw ([&]() {
auto __ctor_arg_000001B5 = "elapsedSeconds";
auto __ctor_arg_000001B6 = "Elapsed frame time cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000001B5, __ctor_arg_000001B6);
})();
    }
}

