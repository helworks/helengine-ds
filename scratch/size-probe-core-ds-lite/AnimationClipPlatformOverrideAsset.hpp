#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class PlatformPositionKeyframeTrackAsset;
class PlatformRotationKeyframeTrackAsset;

#include "runtime/native_string.hpp"
#include "AnimationClipPlatformOverrideMode.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

class AnimationClipPlatformOverrideAsset
{
public:
    virtual ~AnimationClipPlatformOverrideAsset() = default;

    AnimationClipPlatformOverrideAsset();

    std::string PlatformId;

    const std::string& get_PlatformId();
    void set_PlatformId(std::string value);

    ::AnimationClipPlatformOverrideMode Mode;

    ::AnimationClipPlatformOverrideMode get_Mode();
    void set_Mode(::AnimationClipPlatformOverrideMode value);

    Array<::PlatformPositionKeyframeTrackAsset*>* PositionTracks;

    Array<::PlatformPositionKeyframeTrackAsset*>* get_PositionTracks();
    void set_PositionTracks(Array<::PlatformPositionKeyframeTrackAsset*>* value);

    Array<::PlatformPositionKeyframeTrackAsset*>* PositionOffsetTracks;

    Array<::PlatformPositionKeyframeTrackAsset*>* get_PositionOffsetTracks();
    void set_PositionOffsetTracks(Array<::PlatformPositionKeyframeTrackAsset*>* value);

    Array<::PlatformPositionKeyframeTrackAsset*>* ScaleTracks;

    Array<::PlatformPositionKeyframeTrackAsset*>* get_ScaleTracks();
    void set_ScaleTracks(Array<::PlatformPositionKeyframeTrackAsset*>* value);

    Array<::PlatformRotationKeyframeTrackAsset*>* RotationTracks;

    Array<::PlatformRotationKeyframeTrackAsset*>* get_RotationTracks();
    void set_RotationTracks(Array<::PlatformRotationKeyframeTrackAsset*>* value);
};
