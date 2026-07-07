#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class RuntimeContentProcessorIds
{
public:
    virtual ~RuntimeContentProcessorIds() = default;

    inline static const std::string MaterialAsset = "runtime.material-asset";

    inline static const std::string ModelAsset = "runtime.model-asset";

    inline static const std::string TextureAsset = "runtime.texture-asset";

    inline static const std::string TextAsset = "runtime.text-asset";

    inline static const std::string SceneAsset = "runtime.scene-asset";

    inline static const std::string FontAsset = "runtime.font-asset";

    inline static const std::string AnimationClipAsset = "runtime.animation-clip-asset";
};
