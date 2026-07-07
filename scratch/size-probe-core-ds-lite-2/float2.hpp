#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float3;

#include "float3.hpp"

class float2
{
public:
    float2();

    float X;

    float Y;

    static float Dot(::float2 value1, ::float2 value2);

    bool Equals(::float3 other);

    bool Equals(void* obj);

    int32_t GetHashCode();

    float Length();

    float LengthSquared();

    float2(float x, float y);

    float2(float value);

    static ::float2 Normalize(::float2 value);

    friend bool operator!=(::float2 a, ::float2 b);

    friend ::float2 operator*(::float2 a, ::float2 b);

    friend ::float2 operator*(::float2 a, float scalar);

    friend ::float2 operator+(::float2 a, ::float2 b);

    friend ::float2 operator-(::float2 value);

    friend ::float2 operator-(::float2 a, ::float2 b);

    friend ::float2 operator/(::float2 a, float scalar);

    friend ::float2 operator/(::float2 a, ::float2 b);

    friend bool operator==(::float2 a, ::float2 b);
};
