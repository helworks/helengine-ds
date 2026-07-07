#ifdef DrawText
#undef DrawText
#endif
#include "AnimationClipEvaluator.hpp"
#include "runtime/native_exceptions.hpp"
#include "PositionKeyframeAsset.hpp"
#include "float3.hpp"
#include "RotationKeyframeAsset.hpp"
#include "float4.hpp"
#include "AnimationClipEvaluator.hpp"
#include "AnimationInterpolationMode.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "float2.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

::float3 AnimationClipEvaluator::EvaluatePositionTrack(::PositionKeyframeTrackAsset* track, float time)
{
    if (track == nullptr)
    {
throw new ArgumentNullException("track");
    }
else {
    if (track->Keyframes == nullptr || track->Keyframes->get_Length() == 0)
    {
throw new InvalidOperationException("Animation position tracks must contain at least one keyframe.");
    }
}
::PositionKeyframeAsset *firstKeyframe = (*track->Keyframes)[0];
    if (time <= firstKeyframe->Time)
    {
return firstKeyframe->Value;    }
::PositionKeyframeAsset *lastKeyframe = (*track->Keyframes)[track->Keyframes->get_Length() - 1];
    if (time >= lastKeyframe->Time)
    {
return lastKeyframe->Value;    }
for (int32_t i = 0; i < track->Keyframes->get_Length() - 1; i++) {
::PositionKeyframeAsset *startKeyframe = (*track->Keyframes)[i];
::PositionKeyframeAsset *endKeyframe = (*track->Keyframes)[i + 1];
    if (time <= endKeyframe->Time)
    {
return AnimationClipEvaluator::InterpolatePositionKeyframes(startKeyframe, endKeyframe, time);    }
}
return lastKeyframe->Value;}

::float3 AnimationClipEvaluator::EvaluatePositionTrack(::PositionOffsetKeyframeTrackAsset* track, float time)
{
    if (track == nullptr)
    {
throw new ArgumentNullException("track");
    }
return AnimationClipEvaluator::EvaluatePositionKeyframes(track->Keyframes, time, "Animation additive-position tracks must contain at least one keyframe.");}

::float3 AnimationClipEvaluator::EvaluatePositionTrack(::ScaleKeyframeTrackAsset* track, float time)
{
    if (track == nullptr)
    {
throw new ArgumentNullException("track");
    }
return AnimationClipEvaluator::EvaluatePositionKeyframes(track->Keyframes, time, "Animation scale tracks must contain at least one keyframe.");}

::float4 AnimationClipEvaluator::EvaluateRotationTrack(::RotationKeyframeTrackAsset* track, float time)
{
    if (track == nullptr)
    {
throw new ArgumentNullException("track");
    }
else {
    if (track->Keyframes == nullptr || track->Keyframes->get_Length() == 0)
    {
throw new InvalidOperationException("Animation rotation tracks must contain at least one keyframe.");
    }
}
::RotationKeyframeAsset *firstKeyframe = (*track->Keyframes)[0];
    if (time <= firstKeyframe->Time)
    {
return firstKeyframe->Value;    }
::RotationKeyframeAsset *lastKeyframe = (*track->Keyframes)[track->Keyframes->get_Length() - 1];
    if (time >= lastKeyframe->Time)
    {
return lastKeyframe->Value;    }
for (int32_t i = 0; i < track->Keyframes->get_Length() - 1; i++) {
::RotationKeyframeAsset *startKeyframe = (*track->Keyframes)[i];
::RotationKeyframeAsset *endKeyframe = (*track->Keyframes)[i + 1];
    if (time <= endKeyframe->Time)
    {
return AnimationClipEvaluator::InterpolateRotationKeyframes(startKeyframe, endKeyframe, time);    }
}
return lastKeyframe->Value;}

::float3 AnimationClipEvaluator::EvaluatePositionKeyframes(Array<::PositionKeyframeAsset*>* keyframes, float time, std::string emptyTrackMessage)
{
    if (keyframes == nullptr || keyframes->get_Length() == 0)
    {
throw new InvalidOperationException(emptyTrackMessage);
    }
::PositionKeyframeAsset *firstKeyframe = (*keyframes)[0];
    if (time <= firstKeyframe->Time)
    {
return firstKeyframe->Value;    }
::PositionKeyframeAsset *lastKeyframe = (*keyframes)[keyframes->get_Length() - 1];
    if (time >= lastKeyframe->Time)
    {
return lastKeyframe->Value;    }
for (int32_t i = 0; i < keyframes->get_Length() - 1; i++) {
::PositionKeyframeAsset *startKeyframe = (*keyframes)[i];
::PositionKeyframeAsset *endKeyframe = (*keyframes)[i + 1];
    if (time <= endKeyframe->Time)
    {
return AnimationClipEvaluator::InterpolatePositionKeyframes(startKeyframe, endKeyframe, time);    }
}
return lastKeyframe->Value;}

::float3 AnimationClipEvaluator::InterpolatePositionKeyframes(::PositionKeyframeAsset* startKeyframe, ::PositionKeyframeAsset* endKeyframe, float time)
{
    if (endKeyframe->Time <= startKeyframe->Time)
    {
return endKeyframe->Value;    }
else {
    if (endKeyframe->InterpolationMode == AnimationInterpolationMode::Step)
    {
return startKeyframe->Value;    }
}
const float amount = (time - startKeyframe->Time) / (endKeyframe->Time - startKeyframe->Time);
return float3::Lerp(startKeyframe->Value, endKeyframe->Value, amount);}

::float4 AnimationClipEvaluator::InterpolateRotationKeyframes(::RotationKeyframeAsset* startKeyframe, ::RotationKeyframeAsset* endKeyframe, float time)
{
    if (endKeyframe->Time <= startKeyframe->Time)
    {
return endKeyframe->Value;    }
else {
    if (endKeyframe->InterpolationMode == AnimationInterpolationMode::Step)
    {
return startKeyframe->Value;    }
}
const float amount = (time - startKeyframe->Time) / (endKeyframe->Time - startKeyframe->Time);
return float4::Lerp(startKeyframe->Value, endKeyframe->Value, amount);}

