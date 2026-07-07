#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float3;
class float4;

#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "float4.hpp"

class SceneEntityPlatformTransformOverrideAsset
{
public:
    virtual ~SceneEntityPlatformTransformOverrideAsset() = default;

    SceneEntityPlatformTransformOverrideAsset();

    std::string PlatformId;

    const std::string& get_PlatformId();
    void set_PlatformId(std::string value);

    bool HasLocalPositionOverride;

    bool get_HasLocalPositionOverride();
    void set_HasLocalPositionOverride(bool value);

    ::float3 LocalPosition;

    ::float3 get_LocalPosition();
    void set_LocalPosition(::float3 value);

    bool HasLocalScaleOverride;

    bool get_HasLocalScaleOverride();
    void set_HasLocalScaleOverride(bool value);

    ::float3 LocalScale;

    ::float3 get_LocalScale();
    void set_LocalScale(::float3 value);

    bool HasLocalOrientationOverride;

    bool get_HasLocalOrientationOverride();
    void set_HasLocalOrientationOverride(bool value);

    ::float4 LocalOrientation;

    ::float4 get_LocalOrientation();
    void set_LocalOrientation(::float4 value);
};
