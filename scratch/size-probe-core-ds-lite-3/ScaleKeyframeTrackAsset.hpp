#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class PositionKeyframeAsset;

#include "runtime/array.hpp"

class ScaleKeyframeTrackAsset
{
public:
    virtual ~ScaleKeyframeTrackAsset() = default;

    ScaleKeyframeTrackAsset();

    Array<::PositionKeyframeAsset*>* Keyframes;

    Array<::PositionKeyframeAsset*>* get_Keyframes();
    void set_Keyframes(Array<::PositionKeyframeAsset*>* value);
};
