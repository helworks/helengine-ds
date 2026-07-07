#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;
class PositionKeyframeTrackAsset;
class PositionOffsetKeyframeTrackAsset;
class ScaleKeyframeTrackAsset;
class RotationKeyframeTrackAsset;
class AnimationClipPlatformOverrideAsset;

#include "Asset.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

class AnimationClipAsset : public ::Asset
{
public:
    virtual ~AnimationClipAsset() = default;

    AnimationClipAsset();

    float Duration;

    float get_Duration();
    void set_Duration(float value);

    Array<::PositionKeyframeTrackAsset*>* PositionTracks;

    Array<::PositionKeyframeTrackAsset*>* get_PositionTracks();
    void set_PositionTracks(Array<::PositionKeyframeTrackAsset*>* value);

    Array<::PositionOffsetKeyframeTrackAsset*>* PositionOffsetTracks;

    Array<::PositionOffsetKeyframeTrackAsset*>* get_PositionOffsetTracks();
    void set_PositionOffsetTracks(Array<::PositionOffsetKeyframeTrackAsset*>* value);

    Array<::ScaleKeyframeTrackAsset*>* ScaleTracks;

    Array<::ScaleKeyframeTrackAsset*>* get_ScaleTracks();
    void set_ScaleTracks(Array<::ScaleKeyframeTrackAsset*>* value);

    Array<::RotationKeyframeTrackAsset*>* RotationTracks;

    Array<::RotationKeyframeTrackAsset*>* get_RotationTracks();
    void set_RotationTracks(Array<::RotationKeyframeTrackAsset*>* value);

    Array<::AnimationClipPlatformOverrideAsset*>* PlatformOverrides;

    Array<::AnimationClipPlatformOverrideAsset*>* get_PlatformOverrides();
    void set_PlatformOverrides(Array<::AnimationClipPlatformOverrideAsset*>* value);

    const std::string& get_Id();

    void set_Id(std::string value);

    uint64_t get_RuntimeAssetId();

    void set_RuntimeAssetId(uint64_t value);
};
