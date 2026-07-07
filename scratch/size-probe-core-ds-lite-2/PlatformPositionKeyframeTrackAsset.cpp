#ifdef DrawText
#undef DrawText
#endif
#include "PlatformPositionKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "PositionKeyframeAsset.hpp"
#include "runtime/array.hpp"

PlatformPositionKeyframeTrackAsset::PlatformPositionKeyframeTrackAsset() : Keyframes(Array<PositionKeyframeAsset*>::Empty())
{
}

Array<::PositionKeyframeAsset*>* PlatformPositionKeyframeTrackAsset::get_Keyframes()
{
return this->Keyframes;
}

void PlatformPositionKeyframeTrackAsset::set_Keyframes(Array<::PositionKeyframeAsset*>* value)
{
this->Keyframes = value;
}

