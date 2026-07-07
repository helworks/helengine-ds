#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float3;

#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "AnimationInterpolationMode.hpp"

class PositionKeyframeAsset
{
public:
    virtual ~PositionKeyframeAsset() = default;

    std::string FrameId;

    const std::string& get_FrameId();
    void set_FrameId(std::string value);

    float Time;

    float get_Time();
    void set_Time(float value);

    ::float3 Value;

    ::float3 get_Value();
    void set_Value(::float3 value);

    ::AnimationInterpolationMode InterpolationMode;

    ::AnimationInterpolationMode get_InterpolationMode();
    void set_InterpolationMode(::AnimationInterpolationMode value);

    PositionKeyframeAsset();

    PositionKeyframeAsset(float time, ::float3 value, ::AnimationInterpolationMode interpolationMode);
};
