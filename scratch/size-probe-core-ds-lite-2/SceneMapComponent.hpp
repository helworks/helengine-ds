#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class Entity;

#include "UpdateComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_dictionary.hpp"

class SceneMapComponent : public ::UpdateComponent
{
public:
    virtual ~SceneMapComponent() = default;

    static ::SceneMapComponent* Instance;

    static ::SceneMapComponent* get_Instance();
    static void set_Instance(::SceneMapComponent* value);

    std::string InitialSceneId;

    const std::string& get_InitialSceneId();
    void set_InitialSceneId(std::string value);

    Dictionary<std::string, std::string>* get_Mappings();

    void set_Mappings(Dictionary<std::string, std::string>* value);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void Dispose();

    static std::string ResolveSceneId(std::string sceneId);

    SceneMapComponent();

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    static bool StartupSceneWasRequested;

    Dictionary<std::string, std::string>* MappingsValue;

    void ClearSingletonIfOwned();

    void RegisterSingleton();

    void RequestInitialSceneLoad();
};
