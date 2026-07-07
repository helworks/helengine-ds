#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float3;

#include "float3.hpp"
#include "runtime/native_string.hpp"

class float4
{
public:
    float4();

    float X;

    float Y;

    float Z;

    float W;

    static ::float4 get_Identity();

    static ::float4 get_Zero();

    static ::float4 get_One();

    static ::float4 Clamp(::float4 value, ::float4 min, ::float4 max);

    static void Concatenate__ref0_ref1_out2(::float4& value1, ::float4& value2, ::float4& result);

    bool Contains(float x, float y);

    static void CreateFromAxisAngle__ref0_out2(::float3& axis, float angle, ::float4& result);

    static void CreateFromAxisAngle__out2(::float3 axis, float angle, ::float4& result);

    static void CreateFromYawPitchRoll__out3(float yaw, float pitch, float roll, ::float4& result);

    static float Dot(::float4 left, ::float4 right);

    static ::float4 Inverse(::float4 value);

    float Length();

    float LengthSquared();

    static ::float4 Lerp(::float4 start, ::float4 end, float amount);

    static ::float4 Max(::float4 left, ::float4 right);

    static ::float4 Min(::float4 left, ::float4 right);

    float4(float x, float y, float z, float w);

    float4(float value);

    float4(::float3 xyz, float w);

    void Normalize();

    static ::float3 RotateVector(::float3 value, ::float4 rotation);

    static ::float4 SquareRoot(::float4 value);

    std::string ToString();

    friend ::float4 operator*(::float4 left, ::float4 right);

    friend ::float4 operator*(::float4 value, float scalar);

    friend ::float4 operator+(::float4 left, ::float4 right);

    friend ::float4 operator-(::float4 value);

    friend ::float4 operator-(::float4 left, ::float4 right);

    friend ::float4 operator/(::float4 left, ::float4 right);

    friend ::float4 operator/(::float4 value, float scalar);
private:
    static ::float4 zero;

    static ::float4 identity;

    static ::float4 one;
};
