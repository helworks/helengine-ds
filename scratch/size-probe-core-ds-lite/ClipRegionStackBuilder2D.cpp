#ifdef DrawText
#undef DrawText
#endif
#include "ClipRegionStackBuilder2D.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"
#include "system/math.hpp"
#include "float4.hpp"
#include "IDrawable2D.hpp"
#include "IClipRegion2D.hpp"
#include "float3.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "runtime/native_string.hpp"
#include "system/math.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

void ClipRegionStackBuilder2D::BuildClipChain(::IDrawable2D* drawable, List<::IClipRegion2D*>* clipChain)
{
    if (clipChain == nullptr)
    {
throw new ArgumentNullException("clipChain");
    }
clipChain->Clear();
::Entity *current = drawable != nullptr ? drawable->get_Parent() : nullptr;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < current->get_Components()->get_Count(); componentIndex++) {
    IClipRegion2D* clipRegion = he_cpp_try_cast<IClipRegion2D>((*current->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (clipRegion != nullptr)
    {
clipChain->Insert(static_cast<int32_t>(0), clipRegion);
    }
}
    }
current = current->Parent;
}
}

::float4 ClipRegionStackBuilder2D::Intersect(::float4 current, ::float4 next)
{
const float left = Math::Max(current.X, next.X);
const float top = Math::Max(current.Y, next.Y);
const float right = Math::Min(current.X + current.Z, next.X + next.Z);
const float bottom = Math::Min(current.Y + current.W, next.Y + next.W);
const float width = Math::Max(0.0f, right - left);
const float height = Math::Max(0.0f, bottom - top);
return ::float4(left, top, width, height);}

