#ifdef DrawText
#undef DrawText
#endif
#include "PositionKeyframeAsset.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "AnimationInterpolationMode.hpp"
#include "PositionKeyframeAsset.hpp"
#include "runtime/native_string.hpp"

const std::string& PositionKeyframeAsset::get_FrameId()
{
return this->FrameId;
}

void PositionKeyframeAsset::set_FrameId(std::string value)
{
this->FrameId = value;
}

float PositionKeyframeAsset::get_Time()
{
return this->Time;
}

void PositionKeyframeAsset::set_Time(float value)
{
this->Time = value;
}

::float3 PositionKeyframeAsset::get_Value()
{
return this->Value;
}

void PositionKeyframeAsset::set_Value(::float3 value)
{
this->Value = value;
}

::AnimationInterpolationMode PositionKeyframeAsset::get_InterpolationMode()
{
return this->InterpolationMode;
}

void PositionKeyframeAsset::set_InterpolationMode(::AnimationInterpolationMode value)
{
this->InterpolationMode = value;
}

PositionKeyframeAsset::PositionKeyframeAsset() : FrameId(String::Empty), Time(), Value(), InterpolationMode()
{
}

PositionKeyframeAsset::PositionKeyframeAsset(float time, ::float3 value, ::AnimationInterpolationMode interpolationMode) : FrameId(String::Empty), Time(), Value(), InterpolationMode()
{
this->set_Time(time);
this->set_Value(value);
this->set_InterpolationMode(interpolationMode);
}

