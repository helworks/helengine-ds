#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IDrawable2D;
class IClipRegion2D;
class float4;

#include "runtime/native_list.hpp"
#include "float4.hpp"

class ClipRegionStackBuilder2D
{
public:
    virtual ~ClipRegionStackBuilder2D() = default;

    void BuildClipChain(::IDrawable2D* drawable, List<::IClipRegion2D*>* clipChain);

    ::float4 Intersect(::float4 current, ::float4 next);
};
