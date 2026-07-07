#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RotationKeyframeAsset;

#include "runtime/array.hpp"

class PlatformRotationKeyframeTrackAsset
{
public:
    virtual ~PlatformRotationKeyframeTrackAsset() = default;

    PlatformRotationKeyframeTrackAsset();

    Array<::RotationKeyframeAsset*>* Keyframes;

    Array<::RotationKeyframeAsset*>* get_Keyframes();
    void set_Keyframes(Array<::RotationKeyframeAsset*>* value);
};
