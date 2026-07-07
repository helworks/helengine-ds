#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RotationKeyframeAsset;

#include "runtime/array.hpp"

class RotationKeyframeTrackAsset
{
public:
    virtual ~RotationKeyframeTrackAsset() = default;

    RotationKeyframeTrackAsset();

    Array<::RotationKeyframeAsset*>* Keyframes;

    Array<::RotationKeyframeAsset*>* get_Keyframes();
    void set_Keyframes(Array<::RotationKeyframeAsset*>* value);
};
