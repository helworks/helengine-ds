#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float2;

#include "runtime/native_equatable.hpp"
#include "float2.hpp"
#include "runtime/native_string.hpp"

class float3
{
public:
    float3();

    float X;

    float Y;

    float Z;

    static ::float3 get_Zero();

    static ::float3 get_One();

    static ::float3 get_UnitX();

    static ::float3 get_UnitY();

    static ::float3 get_UnitZ();

    static ::float3 Abs(::float3 value);

    static void Cross__ref0_ref1_out2(::float3& vector1, ::float3& vector2, ::float3& result);

    static ::float3 Cross(::float3 vector1, ::float3 vector2);

    static void Dot__ref0_ref1_out2(::float3& value1, ::float3& value2, float& result);

    static float Dot(::float3 value1, ::float3 value2);

    bool Equals(::float3 other);

    bool Equals(void* obj);

    int32_t GetHashCode();

    float Length();

    float LengthSquared();

    static ::float3 Lerp(::float3 start, ::float3 end, float amount);

    static ::float3 Max(::float3 left, ::float3 right);

    static ::float3 Min(::float3 left, ::float3 right);

    float3(float x, float y, float z);

    float3(float value);

    float3(::float2 value, float z);

    static ::float3 Normalize(::float3 value);

    static ::float3 SquareRoot(::float3 value);

    std::string ToString();

    friend bool operator!=(::float3 a, ::float3 b);

    friend ::float3 operator*(::float3 a, ::float3 b);

    friend ::float3 operator*(::float3 a, float scalar);

    friend ::float3 operator+(::float3 a, ::float3 b);

    friend ::float3 operator-(::float3 a, ::float3 b);

    friend ::float3 operator-(::float3 value);

    friend ::float3 operator/(::float3 a, ::float3 b);

    friend ::float3 operator/(::float3 a, float scalar);

    friend bool operator==(::float3 a, ::float3 b);
private:
    static ::float3 zero;

    static ::float3 one;

    static ::float3 unitX;

    static ::float3 unitY;

    static ::float3 unitZ;
};
