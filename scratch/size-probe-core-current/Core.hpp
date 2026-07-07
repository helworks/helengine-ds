#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class CoreInitializationOptions;
class ContentManager;
class ObjectManager;
class IEntityFactory;
class RenderManager3D;
class FontAsset;
class int2;
class RenderManager2D;
class InputSystem;
class StandardPlatformInput;
class PointerInteractionSystem;
class PlatformInfo;
class PhysicsFixedStepScheduler;
class IPhysicsRuntime;
class RuntimeSceneAssetReferenceResolver;
class RuntimeSceneLoadService;
class SceneManager;
class RuntimeDiagnosticsService;
class RuntimeComponentRegistry;
class ITextClipboardService;
class TextBoxShortcutRegistry;
class IRuntimeUpdateStageDiagnosticsProvider;
class IInputBackend;
class IRuntimeComponentDeserializer;
class RuntimeSceneCatalog;

#include "runtime/native_disposable.hpp"
#include "runtime/native_string.hpp"
#include "int2.hpp"
#include "runtime/native_dictionary.hpp"
#include "system/diagnostics/stopwatch.hpp"

class Core : public ::IDisposable
{
public:
    virtual ~Core() = default;

    static ::Core* Instance;

    static ::Core* get_Instance();
    static void set_Instance(::Core* value);

    ::CoreInitializationOptions* InitializationOptions;

    ::CoreInitializationOptions* get_InitializationOptions();
    void set_InitializationOptions(::CoreInitializationOptions* value);

    ::ContentManager* get_ContentManager();

    ::ObjectManager* ObjectManager;

    ::ObjectManager* get_ObjectManager();
    void set_ObjectManager(::ObjectManager* value);

    ::IEntityFactory* EntityFactory;

    ::IEntityFactory* get_EntityFactory();
    void set_EntityFactory(::IEntityFactory* value);

    ::RenderManager3D* RenderManager3D;

    ::RenderManager3D* get_RenderManager3D();
    void set_RenderManager3D(::RenderManager3D* value);

    double LastRenderManager3DDrawMilliseconds;

    double get_LastRenderManager3DDrawMilliseconds();
    void set_LastRenderManager3DDrawMilliseconds(double value);

    int32_t LastRenderManager3DDrawCallCount;

    int32_t get_LastRenderManager3DDrawCallCount();
    void set_LastRenderManager3DDrawCallCount(int32_t value);

    bool UsesPerformanceOverlayMetrics;

    bool get_UsesPerformanceOverlayMetrics();
    void set_UsesPerformanceOverlayMetrics(bool value);

    double PerformanceOverlayTriangleSetupMilliseconds;

    double get_PerformanceOverlayTriangleSetupMilliseconds();
    void set_PerformanceOverlayTriangleSetupMilliseconds(double value);

    double PerformanceOverlayTrianglePrepMilliseconds;

    double get_PerformanceOverlayTrianglePrepMilliseconds();
    void set_PerformanceOverlayTrianglePrepMilliseconds(double value);

    double PerformanceOverlayTriangleEmitMilliseconds;

    double get_PerformanceOverlayTriangleEmitMilliseconds();
    void set_PerformanceOverlayTriangleEmitMilliseconds(double value);

    double PerformanceOverlayPacketEncodeMilliseconds;

    double get_PerformanceOverlayPacketEncodeMilliseconds();
    void set_PerformanceOverlayPacketEncodeMilliseconds(double value);

    double PerformanceOverlaySubmitMilliseconds;

    double get_PerformanceOverlaySubmitMilliseconds();
    void set_PerformanceOverlaySubmitMilliseconds(double value);

    double PerformanceOverlayWaitMilliseconds;

    double get_PerformanceOverlayWaitMilliseconds();
    void set_PerformanceOverlayWaitMilliseconds(double value);

    int32_t PerformanceOverlaySubmittedTriangleCount;

    int32_t get_PerformanceOverlaySubmittedTriangleCount();
    void set_PerformanceOverlaySubmittedTriangleCount(int32_t value);

    int32_t PerformanceOverlayDispatchCount;

    int32_t get_PerformanceOverlayDispatchCount();
    void set_PerformanceOverlayDispatchCount(int32_t value);

    std::string PerformanceOverlayUpdateText;

    const std::string& get_PerformanceOverlayUpdateText();
    void set_PerformanceOverlayUpdateText(std::string value);

    std::string PerformanceOverlayRenderText;

    const std::string& get_PerformanceOverlayRenderText();
    void set_PerformanceOverlayRenderText(std::string value);

    std::string PerformanceOverlayDetailText;

    const std::string& get_PerformanceOverlayDetailText();
    void set_PerformanceOverlayDetailText(std::string value);

    std::string PerformanceOverlayAdditionalText;

    const std::string& get_PerformanceOverlayAdditionalText();
    void set_PerformanceOverlayAdditionalText(std::string value);

    bool UsesPlatformOwnedPerformanceOverlayPresentation;

    bool get_UsesPlatformOwnedPerformanceOverlayPresentation();
    void set_UsesPlatformOwnedPerformanceOverlayPresentation(bool value);

    ::FontAsset* ResolvedPerformanceOverlayFont;

    ::FontAsset* get_ResolvedPerformanceOverlayFont();
    void set_ResolvedPerformanceOverlayFont(::FontAsset* value);

    float ResolvedPerformanceOverlayFontScale;

    float get_ResolvedPerformanceOverlayFontScale();
    void set_ResolvedPerformanceOverlayFontScale(float value);

    ::int2 ResolvedPerformanceOverlayPadding;

    ::int2 get_ResolvedPerformanceOverlayPadding();
    void set_ResolvedPerformanceOverlayPadding(::int2 value);

    std::string ResolvedPerformanceOverlayUpdateText;

    const std::string& get_ResolvedPerformanceOverlayUpdateText();
    void set_ResolvedPerformanceOverlayUpdateText(std::string value);

    std::string ResolvedPerformanceOverlayRenderText;

    const std::string& get_ResolvedPerformanceOverlayRenderText();
    void set_ResolvedPerformanceOverlayRenderText(std::string value);

    std::string ResolvedPerformanceOverlayDetailText;

    const std::string& get_ResolvedPerformanceOverlayDetailText();
    void set_ResolvedPerformanceOverlayDetailText(std::string value);

    std::string ResolvedPerformanceOverlayAdditionalText;

    const std::string& get_ResolvedPerformanceOverlayAdditionalText();
    void set_ResolvedPerformanceOverlayAdditionalText(std::string value);

    ::RenderManager2D* RenderManager2D;

    ::RenderManager2D* get_RenderManager2D();
    void set_RenderManager2D(::RenderManager2D* value);

    ::InputSystem* Input;

    ::InputSystem* get_Input();
    void set_Input(::InputSystem* value);

    ::InputSystem* get_InputSystem();

    ::StandardPlatformInput* StandardPlatformInput;

    ::StandardPlatformInput* get_StandardPlatformInput();
    void set_StandardPlatformInput(::StandardPlatformInput* value);

    ::PointerInteractionSystem* PointerInteractionSystem;

    ::PointerInteractionSystem* get_PointerInteractionSystem();
    void set_PointerInteractionSystem(::PointerInteractionSystem* value);

    double FrameDeltaSeconds;

    double get_FrameDeltaSeconds();
    void set_FrameDeltaSeconds(double value);

    ::PlatformInfo* PlatformInfo;

    ::PlatformInfo* get_PlatformInfo();
    void set_PlatformInfo(::PlatformInfo* value);

    float DeltaTime;

    float get_DeltaTime();
    void set_DeltaTime(float value);

    float UnscaledDeltaTime;

    float get_UnscaledDeltaTime();
    void set_UnscaledDeltaTime(float value);

    double TotalElapsedSeconds;

    double get_TotalElapsedSeconds();
    void set_TotalElapsedSeconds(double value);

    ::PhysicsFixedStepScheduler* get_PhysicsScheduler();

    ::IPhysicsRuntime* get_PhysicsRuntime();

    ::RuntimeSceneAssetReferenceResolver* SceneAssetReferenceResolver;

    ::RuntimeSceneAssetReferenceResolver* get_SceneAssetReferenceResolver();
    void set_SceneAssetReferenceResolver(::RuntimeSceneAssetReferenceResolver* value);

    ::RuntimeSceneLoadService* SceneLoadService;

    ::RuntimeSceneLoadService* get_SceneLoadService();
    void set_SceneLoadService(::RuntimeSceneLoadService* value);

    ::SceneManager* SceneManager;

    ::SceneManager* get_SceneManager();
    void set_SceneManager(::SceneManager* value);

    ::RuntimeDiagnosticsService* RuntimeDiagnosticsService;

    ::RuntimeDiagnosticsService* get_RuntimeDiagnosticsService();
    void set_RuntimeDiagnosticsService(::RuntimeDiagnosticsService* value);

    ::RuntimeComponentRegistry* SceneRuntimeComponentRegistry;

    ::RuntimeComponentRegistry* get_SceneRuntimeComponentRegistry();
    void set_SceneRuntimeComponentRegistry(::RuntimeComponentRegistry* value);

    std::string LastSceneTransitionStage;

    const std::string& get_LastSceneTransitionStage();
    void set_LastSceneTransitionStage(std::string value);

    ::ITextClipboardService* get_TextClipboardService();

    ::TextBoxShortcutRegistry* get_TextBoxShortcutRegistry();

    void AttachPhysicsRuntime(::IPhysicsRuntime* runtime);

    virtual void CompleteFrameBoundary();

    void DetachPhysicsRuntime();

    void Dispose();

    virtual void Draw();

    ::ContentManager* GetContentManager(std::string rootDirectory);

    ::ContentManager* GetContentManager();

    virtual void Initialize(::RenderManager3D* render3D, ::RenderManager2D* render2D, ::IInputBackend* input, ::PlatformInfo* platformInfo);

    virtual void Initialize(::RenderManager3D* render3D, ::RenderManager2D* render2D, ::IInputBackend* input, ::PlatformInfo* platformInfo, ::CoreInitializationOptions* options);

    Core();

    Core(::CoreInitializationOptions* options);

    void RegisterRuntimeComponentDeserializer(::IRuntimeComponentDeserializer* deserializer);

    void ReportSceneTransitionStage(std::string stage);

    void ResetPhysicsTimingState();

    void SetPerformanceOverlayMetrics(bool usesPerformanceOverlayMetrics, double triangleSetupMilliseconds, double trianglePrepMilliseconds, double triangleEmitMilliseconds, double packetEncodeMilliseconds, double submitMilliseconds, double waitMilliseconds, int32_t submittedTriangleCount, int32_t dispatchCount);

    void SetPerformanceOverlayTextRows(bool usesPerformanceOverlayMetrics, std::string updateText, std::string renderText, std::string detailText, std::string additionalText);

    void SetPlatformOwnedPerformanceOverlayPresentation(bool usesPlatformOwnedPresentation);

    void SetResolvedPerformanceOverlayPresentation(::FontAsset* font, float fontScale, ::int2 padding, std::string updateText, std::string renderText, std::string detailText, std::string additionalText);

    void SetTextClipboardService(::ITextClipboardService* clipboardService);

    virtual void Update();

    virtual void Update(double elapsedSeconds);
protected:
    virtual ::IEntityFactory* CreateEntityFactory();

    virtual double GetCurrentMeasuredUpdateSeconds();

    virtual double MeasureRenderManager3DDrawMilliseconds();
private:
    Dictionary<std::string, ::ContentManager*>* ContentManagersByRootPath;

    void* ContentManagerLock;

    ::PhysicsFixedStepScheduler* PhysicsSchedulerValue;

    ::IPhysicsRuntime* PhysicsRuntimeValue;

    ::ITextClipboardService* TextClipboardServiceValue;

    ::TextBoxShortcutRegistry* TextBoxShortcutRegistryValue;

    Stopwatch* UpdateStopwatchValue;

    Stopwatch* DrawStopwatchValue;

    bool HasPreviousMeasuredUpdateSeconds;

    double PreviousMeasuredUpdateSeconds;

    ::IRuntimeUpdateStageDiagnosticsProvider* UpdateStageDiagnosticsProviderValue;

    void AdvanceUpdate(double elapsedSeconds, double currentMeasuredUpdateSeconds);

    void ClearResolvedPerformanceOverlayPresentation();

    ::PhysicsFixedStepScheduler* CreatePhysicsScheduler(::CoreInitializationOptions* options);

    ::SceneManager* CreateSceneManager(::ContentManager* contentManager, ::RuntimeSceneCatalog* sceneCatalog);

    void RecordUpdateStage(std::string stage);

    double ResolveMeasuredElapsedSeconds(double currentMeasuredUpdateSeconds);

    void UpdatePhysics(double elapsedSeconds);

    void ValidateElapsedSeconds(double elapsedSeconds);
};
