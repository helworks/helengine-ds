#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneAssetReference;

#include "runtime/native_string.hpp"

class EngineSceneAssetReferenceFactory
{
public:
    virtual ~EngineSceneAssetReferenceFactory() = default;

    inline static const std::string ProviderIdValue = "engine";

    inline static const std::string CubeRelativePath = "Engine/Models/Cube";

    inline static const std::string PlaneRelativePath = "Engine/Models/Plane";

    inline static const std::string SphereRelativePath = "Engine/Models/Sphere";

    inline static const std::string StandardMaterialRelativePath = "Engine/Materials/Standard";

    inline static const std::string StandardMaterialAssetId = "engine:material:standard";

    static ::SceneAssetReference* CreateCubeModel();

    static ::SceneAssetReference* CreatePlaneModel();

    static ::SceneAssetReference* CreateSphereModel();

    static ::SceneAssetReference* CreateStandardMaterial();
private:
    static ::SceneAssetReference* CreateGenerated(std::string relativePath, std::string assetId);
};
