#ifdef DrawText
#undef DrawText
#endif
#include "PositionKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "PositionKeyframeAsset.hpp"
#include "runtime/array.hpp"

PositionKeyframeTrackAsset::PositionKeyframeTrackAsset() : Keyframes(Array<PositionKeyframeAsset*>::Empty())
{
}

Array<::PositionKeyframeAsset*>* PositionKeyframeTrackAsset::get_Keyframes()
{
return this->Keyframes;
}

void PositionKeyframeTrackAsset::set_Keyframes(Array<::PositionKeyframeAsset*>* value)
{
this->Keyframes = value;
}

