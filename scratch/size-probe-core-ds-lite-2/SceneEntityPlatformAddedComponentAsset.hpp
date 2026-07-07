#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneComponentAssetRecord;

class SceneEntityPlatformAddedComponentAsset
{
public:
    virtual ~SceneEntityPlatformAddedComponentAsset() = default;

    SceneEntityPlatformAddedComponentAsset();

    ::SceneComponentAssetRecord* Component;

    ::SceneComponentAssetRecord* get_Component();
    void set_Component(::SceneComponentAssetRecord* value);
};
