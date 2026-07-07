#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class PositionKeyframeAsset;

#include "runtime/array.hpp"

class PlatformPositionKeyframeTrackAsset
{
public:
    virtual ~PlatformPositionKeyframeTrackAsset() = default;

    PlatformPositionKeyframeTrackAsset();

    Array<::PositionKeyframeAsset*>* Keyframes;

    Array<::PositionKeyframeAsset*>* get_Keyframes();
    void set_Keyframes(Array<::PositionKeyframeAsset*>* value);
};
