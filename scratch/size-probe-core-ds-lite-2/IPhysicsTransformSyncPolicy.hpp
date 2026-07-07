#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class float3;
class float4;

#include "float3.hpp"
#include "float4.hpp"

class IPhysicsTransformSyncPolicy
{
public:
    virtual void ApplyTransform(::Entity* entity, ::float3 position, ::float4 orientation) = 0;
};
