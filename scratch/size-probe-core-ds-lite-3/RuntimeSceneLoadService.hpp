#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeSceneAssetReferenceResolver;
class RuntimeComponentRegistry;
class Entity;
class SceneAsset;
class RuntimeSceneLoadResult;
class Component;
class SceneComponentAssetRecord;
class SceneEntityAsset;

#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"

class RuntimeSceneLoadService
{
public:
    virtual ~RuntimeSceneLoadService() = default;

    std::string LastTraceStage;

    const std::string& get_LastTraceStage();
    void set_LastTraceStage(std::string value);

    int32_t LastTraceRootEntityIndex;

    int32_t get_LastTraceRootEntityIndex();
    void set_LastTraceRootEntityIndex(int32_t value);

    int32_t LastTraceEntityDepth;

    int32_t get_LastTraceEntityDepth();
    void set_LastTraceEntityDepth(int32_t value);

    std::string LastTraceComponentTypeId;

    const std::string& get_LastTraceComponentTypeId();
    void set_LastTraceComponentTypeId(std::string value);

    const std::string& get_LastTextLoadStage();

    const std::string& get_LastTextFontRelativePath();

    const std::string& get_LastTextureLoadStage();

    const std::string& get_LastTextureRelativePath();

    const std::string& get_LastFontDeserializeStage();

    List<::Entity*>* Load(::SceneAsset* sceneAsset);

    ::RuntimeSceneLoadResult* LoadTracked(::SceneAsset* sceneAsset);

    RuntimeSceneLoadService(::RuntimeSceneAssetReferenceResolver* referenceResolver);

    RuntimeSceneLoadService(::RuntimeSceneAssetReferenceResolver* referenceResolver, ::RuntimeComponentRegistry* componentRegistry);
private:
    ::RuntimeSceneAssetReferenceResolver* ReferenceResolver;

    ::RuntimeComponentRegistry* ComponentRegistry;

    ::Component* LoadComponent(::SceneComponentAssetRecord* record, int32_t rootEntityIndex, int32_t entityDepth);

    ::Entity* LoadEntity(::SceneEntityAsset* entityAsset, int32_t rootEntityIndex, int32_t entityDepth);

    void RecordTraceState(std::string stage, int32_t rootEntityIndex, int32_t entityDepth, std::string componentTypeId);
};
