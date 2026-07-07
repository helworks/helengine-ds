#ifdef DrawText
#undef DrawText
#endif
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "PositionKeyframeAsset.hpp"
#include "runtime/array.hpp"

PositionOffsetKeyframeTrackAsset::PositionOffsetKeyframeTrackAsset() : Keyframes(Array<PositionKeyframeAsset*>::Empty())
{
}

Array<::PositionKeyframeAsset*>* PositionOffsetKeyframeTrackAsset::get_Keyframes()
{
return this->Keyframes;
}

void PositionOffsetKeyframeTrackAsset::set_Keyframes(Array<::PositionKeyframeAsset*>* value)
{
this->Keyframes = value;
}

