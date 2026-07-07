#ifdef DrawText
#undef DrawText
#endif
#include "GeometryUtils.hpp"
#include "float3.hpp"
#include "float2.hpp"
#include "runtime/native_string.hpp"

bool GeometryUtils::IsPointInsideRect(double x, double y, ::float3 origin, int32_t width, int32_t height)
{
const double left = origin.X;
const double top = origin.Y;
const double right = left + width;
const double bottom = top + height;
return x >= left && x < right && y >= top && y < bottom;}

