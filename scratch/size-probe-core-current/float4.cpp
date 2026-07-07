#ifdef DrawText
#undef DrawText
#endif
#include "float4.hpp"
#include "float4.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "runtime/native_exceptions.hpp"
#include "float2.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/native_exceptions.hpp"

float4::float4() : X(), Y(), Z(), W()
{
}

::float4 float4::get_Identity()
{
return identity;}

::float4 float4::get_Zero()
{
return zero;}

::float4 float4::get_One()
{
return one;}

::float4 float4::Clamp(::float4 value, ::float4 min, ::float4 max)
{
return ::float4(value.X < min.X ? min.X : value.X > max.X ? max.X : value.X, value.Y < min.Y ? min.Y : value.Y > max.Y ? max.Y : value.Y, value.Z < min.Z ? min.Z : value.Z > max.Z ? max.Z : value.Z, value.W < min.W ? min.W : value.W > max.W ? max.W : value.W);}

void float4::Concatenate__ref0_ref1_out2(::float4& value1, ::float4& value2, ::float4& result)
{
const float x1 = value1.X;
const float y1 = value1.Y;
const float z1 = value1.Z;
const float w1 = value1.W;
const float x2 = value2.X;
const float y2 = value2.Y;
const float z2 = value2.Z;
const float w2 = value2.W;
result.X = ((x2 * w1) + (x1 * w2)) + ((y2 * z1) - (z2 * y1));
result.Y = ((y2 * w1) + (y1 * w2)) + ((z2 * x1) - (x2 * z1));
result.Z = ((z2 * w1) + (z1 * w2)) + ((x2 * y1) - (y2 * x1));
result.W = (w2 * w1) - (((x2 * x1) + (y2 * y1)) + (z2 * z1));
}

bool float4::Contains(float x, float y)
{
return ((((this->X <= x) && (x < (this->X + this->Z))) && (this->Y <= y)) && (y < (this->Y + this->W)));}

void float4::CreateFromAxisAngle__ref0_out2(::float3& axis, float angle, ::float4& result)
{
const float half = angle * 0.5f;
const float sin = static_cast<float>(Math::Sin(half));
const float cos = static_cast<float>(Math::Cos(half));
result.X = axis.X * sin;
result.Y = axis.Y * sin;
result.Z = axis.Z * sin;
result.W = cos;
}

void float4::CreateFromAxisAngle__out2(::float3 axis, float angle, ::float4& result)
{
float4::CreateFromAxisAngle__ref0_out2(axis, angle, result);
}

void float4::CreateFromYawPitchRoll__out3(float yaw, float pitch, float roll, ::float4& result)
{
const float halfRoll = roll * 0.5f;
const float halfPitch = pitch * 0.5f;
const float halfYaw = yaw * 0.5f;
const float sinRoll = static_cast<float>(Math::Sin(halfRoll));
const float cosRoll = static_cast<float>(Math::Cos(halfRoll));
const float sinPitch = static_cast<float>(Math::Sin(halfPitch));
const float cosPitch = static_cast<float>(Math::Cos(halfPitch));
const float sinYaw = static_cast<float>(Math::Sin(halfYaw));
const float cosYaw = static_cast<float>(Math::Cos(halfYaw));
result.X = (cosYaw * sinPitch * cosRoll) + (sinYaw * cosPitch * sinRoll);
result.Y = (sinYaw * cosPitch * cosRoll) - (cosYaw * sinPitch * sinRoll);
result.Z = (cosYaw * cosPitch * sinRoll) - (sinYaw * sinPitch * cosRoll);
result.W = (cosYaw * cosPitch * cosRoll) + (sinYaw * sinPitch * sinRoll);
}

float float4::Dot(::float4 left, ::float4 right)
{
return (left.X * right.X) + (left.Y * right.Y) + (left.Z * right.Z) + (left.W * right.W);}

::float4 float4::Inverse(::float4 value)
{
const double lengthSquared = (value.X * value.X) + (value.Y * value.Y) + (value.Z * value.Z) + (value.W * value.W);
    if (lengthSquared <= 0.0)
    {
throw new InvalidOperationException("Cannot invert a zero-length quaternion.");
    }
const double inverseLengthSquared = 1.0 / lengthSquared;
return ::float4(static_cast<float>((-value.X * inverseLengthSquared)), static_cast<float>((-value.Y * inverseLengthSquared)), static_cast<float>((-value.Z * inverseLengthSquared)), static_cast<float>((value.W * inverseLengthSquared)));}

float float4::Length()
{
return static_cast<float>(Math::Sqrt(this->LengthSquared()));}

float float4::LengthSquared()
{
return this->X * this->X + this->Y * this->Y + this->Z * this->Z + this->W * this->W;}

::float4 float4::Lerp(::float4 start, ::float4 end, float amount)
{
const double normalizedAmount = amount;
const double dot = (start.X * end.X) + (start.Y * end.Y) + (start.Z * end.Z) + (start.W * end.W);
::float4 adjustedEnd = end;
    if (dot < 0.0)
    {
adjustedEnd = ::float4(-end.X, -end.Y, -end.Z, -end.W);
    }
::float4 result = ::float4(static_cast<float>((start.X + ((adjustedEnd.X - start.X) * normalizedAmount))), static_cast<float>((start.Y + ((adjustedEnd.Y - start.Y) * normalizedAmount))), static_cast<float>((start.Z + ((adjustedEnd.Z - start.Z) * normalizedAmount))), static_cast<float>((start.W + ((adjustedEnd.W - start.W) * normalizedAmount))));
result.Normalize();
return result;}

::float4 float4::Max(::float4 left, ::float4 right)
{
return ::float4(left.X > right.X ? left.X : right.X, left.Y > right.Y ? left.Y : right.Y, left.Z > right.Z ? left.Z : right.Z, left.W > right.W ? left.W : right.W);}

::float4 float4::Min(::float4 left, ::float4 right)
{
return ::float4(left.X < right.X ? left.X : right.X, left.Y < right.Y ? left.Y : right.Y, left.Z < right.Z ? left.Z : right.Z, left.W < right.W ? left.W : right.W);}

float4::float4(float x, float y, float z, float w) : X(), Y(), Z(), W()
{
this->X = x;
this->Y = y;
this->Z = z;
this->W = w;
}

float4::float4(float value) : X(), Y(), Z(), W()
{
this->X = value;
this->Y = value;
this->Z = value;
this->W = value;
}

float4::float4(::float3 xyz, float w) : X(), Y(), Z(), W()
{
this->X = xyz.X;
this->Y = xyz.Y;
this->Z = xyz.Z;
this->W = w;
}

void float4::Normalize()
{
const float num = 1.0f / static_cast<float>(Math::Sqrt((this->X * this->X) + (this->Y * this->Y) + (this->Z * this->Z) + (this->W * this->W)));
this->X *= num;
this->Y *= num;
this->Z *= num;
this->W *= num;
}

::float3 float4::RotateVector(::float3 value, ::float4 rotation)
{
const double qx = rotation.X;
const double qy = rotation.Y;
const double qz = rotation.Z;
const double qw = rotation.W;
const double vx = value.X;
const double vy = value.Y;
const double vz = value.Z;
const double tx = 2.0 * (qy * vz - qz * vy);
const double ty = 2.0 * (qz * vx - qx * vz);
const double tz = 2.0 * (qx * vy - qy * vx);
const double cx = (qy * tz) - (qz * ty);
const double cy = (qz * tx) - (qx * tz);
const double cz = (qx * ty) - (qy * tx);
const double rx = vx + (tx * qw) + cx;
const double ry = vy + (ty * qw) + cy;
const double rz = vz + (tz * qw) + cz;
return ::float3(static_cast<float>(rx), static_cast<float>(ry), static_cast<float>(rz));}

::float4 float4::SquareRoot(::float4 value)
{
return ([&]() {
auto __ctor_arg_0000018F = static_cast<float>(Math::Sqrt(value.X));
auto __ctor_arg_00000190 = static_cast<float>(Math::Sqrt(value.Y));
auto __ctor_arg_00000191 = static_cast<float>(Math::Sqrt(value.Z));
auto __ctor_arg_00000192 = static_cast<float>(Math::Sqrt(value.W));
return ::float4(__ctor_arg_0000018F, __ctor_arg_00000190, __ctor_arg_00000191, __ctor_arg_00000192);
})();}

std::string float4::ToString()
{
return std::to_string(this->X) + std::string(", ") + std::to_string(this->Y) + std::string(", ") + std::to_string(this->Z) + std::string(", ") + std::to_string(this->W);}

::float4 float4::zero = ::float4(0.0f, 0.0f, 0.0f, 0.0f);

::float4 float4::identity = ::float4(0.0f, 0.0f, 0.0f, 1.0f);

::float4 float4::one = ::float4(1.0f, 1.0f, 1.0f, 1.0f);

::float4 operator*(::float4 left, ::float4 right)
{
return ::float4(left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W);}

::float4 operator*(::float4 value, float scalar)
{
return ::float4(value.X * scalar, value.Y * scalar, value.Z * scalar, value.W * scalar);}

::float4 operator+(::float4 left, ::float4 right)
{
return ::float4(left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W);}

::float4 operator-(::float4 value)
{
return ::float4(-value.X, -value.Y, -value.Z, -value.W);}

::float4 operator-(::float4 left, ::float4 right)
{
return ::float4(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);}

::float4 operator/(::float4 left, ::float4 right)
{
return ::float4(left.X / right.X, left.Y / right.Y, left.Z / right.Z, left.W / right.W);}

::float4 operator/(::float4 value, float scalar)
{
return ::float4(value.X / scalar, value.Y / scalar, value.Z / scalar, value.W / scalar);}

