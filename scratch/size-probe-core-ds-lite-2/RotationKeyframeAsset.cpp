#ifdef DrawText
#undef DrawText
#endif
#include "RotationKeyframeAsset.hpp"
#include "runtime/native_string.hpp"
#include "float4.hpp"
#include "AnimationInterpolationMode.hpp"
#include "RotationKeyframeAsset.hpp"
#include "runtime/native_string.hpp"

const std::string& RotationKeyframeAsset::get_FrameId()
{
return this->FrameId;
}

void RotationKeyframeAsset::set_FrameId(std::string value)
{
this->FrameId = value;
}

float RotationKeyframeAsset::get_Time()
{
return this->Time;
}

void RotationKeyframeAsset::set_Time(float value)
{
this->Time = value;
}

::float4 RotationKeyframeAsset::get_Value()
{
return this->Value;
}

void RotationKeyframeAsset::set_Value(::float4 value)
{
this->Value = value;
}

::AnimationInterpolationMode RotationKeyframeAsset::get_InterpolationMode()
{
return this->InterpolationMode;
}

void RotationKeyframeAsset::set_InterpolationMode(::AnimationInterpolationMode value)
{
this->InterpolationMode = value;
}

RotationKeyframeAsset::RotationKeyframeAsset() : FrameId(String::Empty), Time(), Value(), InterpolationMode()
{
}

RotationKeyframeAsset::RotationKeyframeAsset(float time, ::float4 value, ::AnimationInterpolationMode interpolationMode) : FrameId(String::Empty), Time(), Value(), InterpolationMode()
{
this->set_Time(time);
this->set_Value(value);
this->set_InterpolationMode(interpolationMode);
}

