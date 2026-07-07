#ifdef DrawText
#undef DrawText
#endif
#include "PlatformRotationKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "RotationKeyframeAsset.hpp"
#include "runtime/array.hpp"

PlatformRotationKeyframeTrackAsset::PlatformRotationKeyframeTrackAsset() : Keyframes(Array<RotationKeyframeAsset*>::Empty())
{
}

Array<::RotationKeyframeAsset*>* PlatformRotationKeyframeTrackAsset::get_Keyframes()
{
return this->Keyframes;
}

void PlatformRotationKeyframeTrackAsset::set_Keyframes(Array<::RotationKeyframeAsset*>* value)
{
this->Keyframes = value;
}

