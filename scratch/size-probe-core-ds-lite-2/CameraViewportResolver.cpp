#ifdef DrawText
#undef DrawText
#endif
#include "CameraViewportResolver.hpp"
#include "runtime/native_exceptions.hpp"
#include "float4.hpp"
#include "CameraViewportResolver.hpp"
#include "float3.hpp"
#include "runtime/native_string.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/native_exceptions.hpp"

::float4 CameraViewportResolver::ResolveViewport(::float4 viewport, double targetWidth, double targetHeight)
{
    if (targetWidth <= 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000033 = "targetWidth";
auto __ctor_arg_00000034 = "Target width must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_00000033, __ctor_arg_00000034);
})();
    }
    if (targetHeight <= 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000035 = "targetHeight";
auto __ctor_arg_00000036 = "Target height must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_00000035, __ctor_arg_00000036);
})();
    }
double offsetX = viewport.X;
double offsetY = viewport.Y;
double width = viewport.Z;
double height = viewport.W;
    if (width <= 1.0 && height <= 1.0)
    {
offsetX *= targetWidth;
width *= targetWidth;
    if (CameraViewportResolver::UsesStackedDualScreenViewportUnits(viewport, targetWidth, targetHeight))
    {
const double screenHeight = targetHeight * 0.5;
offsetY *= screenHeight;
height *= screenHeight;
    }
else {
offsetY *= targetHeight;
height *= targetHeight;
}
    }
return ::float4(static_cast<float>(offsetX), static_cast<float>(offsetY), static_cast<float>(width), static_cast<float>(height));}

bool CameraViewportResolver::UsesStackedDualScreenViewportUnits(::float4 viewport, double targetWidth, double targetHeight)
{
const double expectedStackedHeight = targetWidth * 1.5;
    if (Math::Abs(targetHeight - expectedStackedHeight) > 0.5)
    {
return false;    }
return viewport.Y >= 0.0f && (viewport.Y + viewport.W) <= 2.0f;}

