#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class EditorAssetBinaryValueKind
{
    TextureAsset = 1,
    ModelAsset = 2,
    ShaderAsset = 3,
    TextAsset = 4,
    MaterialAsset = 5,
    SceneAsset = 6,
    AnimationClipAsset = 8,
    PlatformMaterialAsset = 9
};
