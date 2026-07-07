#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneEntityPlatformAddedComponentAsset;

#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

class SceneEntityPlatformComponentOverrideAsset
{
public:
    virtual ~SceneEntityPlatformComponentOverrideAsset() = default;

    SceneEntityPlatformComponentOverrideAsset();

    std::string PlatformId;

    const std::string& get_PlatformId();
    void set_PlatformId(std::string value);

    Array<std::string>* RemovedComponentKeys;

    Array<std::string>* get_RemovedComponentKeys();
    void set_RemovedComponentKeys(Array<std::string>* value);

    Array<::SceneEntityPlatformAddedComponentAsset*>* AddedComponents;

    Array<::SceneEntityPlatformAddedComponentAsset*>* get_AddedComponents();
    void set_AddedComponents(Array<::SceneEntityPlatformAddedComponentAsset*>* value);
};
