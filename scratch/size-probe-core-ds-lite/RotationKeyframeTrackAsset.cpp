#ifdef DrawText
#undef DrawText
#endif
#include "RotationKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "RotationKeyframeAsset.hpp"
#include "runtime/array.hpp"

RotationKeyframeTrackAsset::RotationKeyframeTrackAsset() : Keyframes(Array<RotationKeyframeAsset*>::Empty())
{
}

Array<::RotationKeyframeAsset*>* RotationKeyframeTrackAsset::get_Keyframes()
{
return this->Keyframes;
}

void RotationKeyframeTrackAsset::set_Keyframes(Array<::RotationKeyframeAsset*>* value)
{
this->Keyframes = value;
}

