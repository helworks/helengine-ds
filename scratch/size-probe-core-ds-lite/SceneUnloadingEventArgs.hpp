#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;

#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"

class SceneUnloadingEventArgs
{
public:
    virtual ~SceneUnloadingEventArgs() = default;

    std::string SceneId;

    const std::string& get_SceneId();

    std::string CookedRelativePath;

    const std::string& get_CookedRelativePath();

    List<::Entity*>* RootEntities;

    List<::Entity*>* get_RootEntities();

    SceneUnloadingEventArgs(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities);
};
