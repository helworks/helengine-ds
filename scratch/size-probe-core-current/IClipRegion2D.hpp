#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class float4;

#include "float4.hpp"

class IClipRegion2D
{
public:
    virtual ::Entity* get_Parent() = 0;

    virtual ::float4 GetClipRect() = 0;
};
