#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeSceneCatalog;
class ISceneIdPathResolver;
class IRuntimeDiagnosticsProvider;
class StandardPlatformInputConfiguration;

#include "runtime/native_string.hpp"

class CoreInitializationOptions
{
public:
    virtual ~CoreInitializationOptions() = default;

    CoreInitializationOptions();

    std::string ContentRootPath;

    const std::string& get_ContentRootPath();
    void set_ContentRootPath(std::string value);

    uint8_t UpdateOrderLayers;

    uint8_t get_UpdateOrderLayers();
    void set_UpdateOrderLayers(uint8_t value);

    uint8_t RenderOrderLayers3D;

    uint8_t get_RenderOrderLayers3D();
    void set_RenderOrderLayers3D(uint8_t value);

    int32_t UpdateListInitialCapacity;

    int32_t get_UpdateListInitialCapacity();
    void set_UpdateListInitialCapacity(int32_t value);

    int32_t RenderList2DInitialCapacity;

    int32_t get_RenderList2DInitialCapacity();
    void set_RenderList2DInitialCapacity(int32_t value);

    int32_t RenderList3DInitialCapacity;

    int32_t get_RenderList3DInitialCapacity();
    void set_RenderList3DInitialCapacity(int32_t value);

    double DefaultUpdateDeltaSeconds;

    double get_DefaultUpdateDeltaSeconds();
    void set_DefaultUpdateDeltaSeconds(double value);

    double PhysicsFixedStepSeconds;

    double get_PhysicsFixedStepSeconds();
    void set_PhysicsFixedStepSeconds(double value);

    int32_t PhysicsMaxStepsPerUpdate;

    int32_t get_PhysicsMaxStepsPerUpdate();
    void set_PhysicsMaxStepsPerUpdate(int32_t value);

    ::RuntimeSceneCatalog* SceneCatalog;

    ::RuntimeSceneCatalog* get_SceneCatalog();
    void set_SceneCatalog(::RuntimeSceneCatalog* value);

    ::ISceneIdPathResolver* ScenePathResolver;

    ::ISceneIdPathResolver* get_ScenePathResolver();
    void set_ScenePathResolver(::ISceneIdPathResolver* value);

    ::IRuntimeDiagnosticsProvider* RuntimeDiagnosticsProvider;

    ::IRuntimeDiagnosticsProvider* get_RuntimeDiagnosticsProvider();
    void set_RuntimeDiagnosticsProvider(::IRuntimeDiagnosticsProvider* value);

    ::StandardPlatformInputConfiguration* StandardPlatformInputConfiguration;

    ::StandardPlatformInputConfiguration* get_StandardPlatformInputConfiguration();
    void set_StandardPlatformInputConfiguration(::StandardPlatformInputConfiguration* value);

    bool CommitPendingSceneOperationsDuringDraw;

    bool get_CommitPendingSceneOperationsDuringDraw();
    void set_CommitPendingSceneOperationsDuringDraw(bool value);

    void Normalize();
};
