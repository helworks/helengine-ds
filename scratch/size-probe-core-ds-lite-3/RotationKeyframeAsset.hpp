#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4;

#include "runtime/native_string.hpp"
#include "float4.hpp"
#include "AnimationInterpolationMode.hpp"

class RotationKeyframeAsset
{
public:
    virtual ~RotationKeyframeAsset() = default;

    std::string FrameId;

    const std::string& get_FrameId();
    void set_FrameId(std::string value);

    float Time;

    float get_Time();
    void set_Time(float value);

    ::float4 Value;

    ::float4 get_Value();
    void set_Value(::float4 value);

    ::AnimationInterpolationMode InterpolationMode;

    ::AnimationInterpolationMode get_InterpolationMode();
    void set_InterpolationMode(::AnimationInterpolationMode value);

    RotationKeyframeAsset();

    RotationKeyframeAsset(float time, ::float4 value, ::AnimationInterpolationMode interpolationMode);
};
