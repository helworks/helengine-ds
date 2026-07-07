#ifdef DrawText
#undef DrawText
#endif
#include "float2.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "system/number.hpp"

float2::float2() : X(), Y()
{
}

float float2::Dot(::float2 value1, ::float2 value2)
{
return value1.X * value2.X + value1.Y * value2.Y;}

bool float2::Equals(::float3 other)
{
return this->X == other.X && this->Y == other.Y;}

bool float2::Equals(void* obj)
{
    if (obj == nullptr)
    {
return false;    }
else {
    if (!(obj != nullptr))
    {
return false;    }
}
::float2 other = (*static_cast<float2*>(obj));
return this->X == other.X && this->Y == other.Y;}

int32_t float2::GetHashCode()
{
{
int32_t hash = Number::GetHashCode(this->X);
hash = (hash * 397) ^ Number::GetHashCode(this->Y);
return hash;}
}

float float2::Length()
{
return static_cast<float>(Math::Sqrt(this->LengthSquared()));}

float float2::LengthSquared()
{
return this->X * this->X + this->Y * this->Y;}

float2::float2(float x, float y) : X(), Y()
{
this->X = x;
this->Y = y;
}

float2::float2(float value) : X(), Y()
{
this->X = value;
this->Y = value;
}

::float2 float2::Normalize(::float2 value)
{
float factor = static_cast<float>(Math::Sqrt((value.X * value.X) + (value.Y * value.Y)));
factor = 1.0f / factor;
return ::float2(value.X * factor, value.Y * factor);}

bool operator!=(::float2 a, ::float2 b)
{
return a.X != b.X || a.Y != b.Y;}

::float2 operator*(::float2 a, ::float2 b)
{
return ::float2(a.X * b.X, a.Y * b.Y);}

::float2 operator*(::float2 a, float scalar)
{
return ::float2(a.X * scalar, a.Y * scalar);}

::float2 operator+(::float2 a, ::float2 b)
{
return ::float2(a.X + b.X, a.Y + b.Y);}

::float2 operator-(::float2 value)
{
return ::float2(-value.X, -value.Y);}

::float2 operator-(::float2 a, ::float2 b)
{
return ::float2(a.X - b.X, a.Y - b.Y);}

::float2 operator/(::float2 a, float scalar)
{
return ::float2(a.X / scalar, a.Y / scalar);}

::float2 operator/(::float2 a, ::float2 b)
{
return ::float2(a.X / b.X, a.Y / b.Y);}

bool operator==(::float2 a, ::float2 b)
{
return a.X == b.X && a.Y == b.Y;}

