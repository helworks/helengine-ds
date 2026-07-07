#ifdef DrawText
#undef DrawText
#endif
#include "ScaleKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "PositionKeyframeAsset.hpp"
#include "runtime/array.hpp"

ScaleKeyframeTrackAsset::ScaleKeyframeTrackAsset() : Keyframes(Array<PositionKeyframeAsset*>::Empty())
{
}

Array<::PositionKeyframeAsset*>* ScaleKeyframeTrackAsset::get_Keyframes()
{
return this->Keyframes;
}

void ScaleKeyframeTrackAsset::set_Keyframes(Array<::PositionKeyframeAsset*>* value)
{
this->Keyframes = value;
}

