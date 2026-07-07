#ifdef DrawText
#undef DrawText
#endif
#include "AnimationClipAsset.hpp"
#include "runtime/array.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "AnimationClipPlatformOverrideAsset.hpp"
#include "runtime/array.hpp"

AnimationClipAsset::AnimationClipAsset() : Duration(), PositionTracks(Array<PositionKeyframeTrackAsset*>::Empty()), PositionOffsetTracks(Array<PositionOffsetKeyframeTrackAsset*>::Empty()), ScaleTracks(Array<ScaleKeyframeTrackAsset*>::Empty()), RotationTracks(Array<RotationKeyframeTrackAsset*>::Empty()), PlatformOverrides(Array<AnimationClipPlatformOverrideAsset*>::Empty())
{
}

float AnimationClipAsset::get_Duration()
{
return this->Duration;
}

void AnimationClipAsset::set_Duration(float value)
{
this->Duration = value;
}

Array<::PositionKeyframeTrackAsset*>* AnimationClipAsset::get_PositionTracks()
{
return this->PositionTracks;
}

void AnimationClipAsset::set_PositionTracks(Array<::PositionKeyframeTrackAsset*>* value)
{
this->PositionTracks = value;
}

Array<::PositionOffsetKeyframeTrackAsset*>* AnimationClipAsset::get_PositionOffsetTracks()
{
return this->PositionOffsetTracks;
}

void AnimationClipAsset::set_PositionOffsetTracks(Array<::PositionOffsetKeyframeTrackAsset*>* value)
{
this->PositionOffsetTracks = value;
}

Array<::ScaleKeyframeTrackAsset*>* AnimationClipAsset::get_ScaleTracks()
{
return this->ScaleTracks;
}

void AnimationClipAsset::set_ScaleTracks(Array<::ScaleKeyframeTrackAsset*>* value)
{
this->ScaleTracks = value;
}

Array<::RotationKeyframeTrackAsset*>* AnimationClipAsset::get_RotationTracks()
{
return this->RotationTracks;
}

void AnimationClipAsset::set_RotationTracks(Array<::RotationKeyframeTrackAsset*>* value)
{
this->RotationTracks = value;
}

Array<::AnimationClipPlatformOverrideAsset*>* AnimationClipAsset::get_PlatformOverrides()
{
return this->PlatformOverrides;
}

void AnimationClipAsset::set_PlatformOverrides(Array<::AnimationClipPlatformOverrideAsset*>* value)
{
this->PlatformOverrides = value;
}

const std::string& AnimationClipAsset::get_Id()
{
return Asset::get_Id();
}

void AnimationClipAsset::set_Id(std::string value)
{
Asset::set_Id(value);
}

uint64_t AnimationClipAsset::get_RuntimeAssetId()
{
return Asset::get_RuntimeAssetId();
}

void AnimationClipAsset::set_RuntimeAssetId(uint64_t value)
{
Asset::set_RuntimeAssetId(value);
}

