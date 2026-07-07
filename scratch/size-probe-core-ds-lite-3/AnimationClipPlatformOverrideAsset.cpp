#ifdef DrawText
#undef DrawText
#endif
#include "AnimationClipPlatformOverrideAsset.hpp"
#include "runtime/native_string.hpp"
#include "AnimationClipPlatformOverrideMode.hpp"
#include "runtime/array.hpp"
#include "PlatformPositionKeyframeTrackAsset.hpp"
#include "PlatformRotationKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"

AnimationClipPlatformOverrideAsset::AnimationClipPlatformOverrideAsset() : PlatformId(String::Empty), Mode(), PositionTracks(Array<PlatformPositionKeyframeTrackAsset*>::Empty()), PositionOffsetTracks(Array<PlatformPositionKeyframeTrackAsset*>::Empty()), ScaleTracks(Array<PlatformPositionKeyframeTrackAsset*>::Empty()), RotationTracks(Array<PlatformRotationKeyframeTrackAsset*>::Empty())
{
}

const std::string& AnimationClipPlatformOverrideAsset::get_PlatformId()
{
return this->PlatformId;
}

void AnimationClipPlatformOverrideAsset::set_PlatformId(std::string value)
{
this->PlatformId = value;
}

::AnimationClipPlatformOverrideMode AnimationClipPlatformOverrideAsset::get_Mode()
{
return this->Mode;
}

void AnimationClipPlatformOverrideAsset::set_Mode(::AnimationClipPlatformOverrideMode value)
{
this->Mode = value;
}

Array<::PlatformPositionKeyframeTrackAsset*>* AnimationClipPlatformOverrideAsset::get_PositionTracks()
{
return this->PositionTracks;
}

void AnimationClipPlatformOverrideAsset::set_PositionTracks(Array<::PlatformPositionKeyframeTrackAsset*>* value)
{
this->PositionTracks = value;
}

Array<::PlatformPositionKeyframeTrackAsset*>* AnimationClipPlatformOverrideAsset::get_PositionOffsetTracks()
{
return this->PositionOffsetTracks;
}

void AnimationClipPlatformOverrideAsset::set_PositionOffsetTracks(Array<::PlatformPositionKeyframeTrackAsset*>* value)
{
this->PositionOffsetTracks = value;
}

Array<::PlatformPositionKeyframeTrackAsset*>* AnimationClipPlatformOverrideAsset::get_ScaleTracks()
{
return this->ScaleTracks;
}

void AnimationClipPlatformOverrideAsset::set_ScaleTracks(Array<::PlatformPositionKeyframeTrackAsset*>* value)
{
this->ScaleTracks = value;
}

Array<::PlatformRotationKeyframeTrackAsset*>* AnimationClipPlatformOverrideAsset::get_RotationTracks()
{
return this->RotationTracks;
}

void AnimationClipPlatformOverrideAsset::set_RotationTracks(Array<::PlatformRotationKeyframeTrackAsset*>* value)
{
this->RotationTracks = value;
}

