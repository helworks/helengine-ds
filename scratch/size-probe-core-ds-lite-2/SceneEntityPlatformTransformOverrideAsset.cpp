#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/native_string.hpp"

SceneEntityPlatformTransformOverrideAsset::SceneEntityPlatformTransformOverrideAsset() : PlatformId(String::Empty), HasLocalPositionOverride(), LocalPosition(), HasLocalScaleOverride(), LocalScale(), HasLocalOrientationOverride(), LocalOrientation()
{
}

const std::string& SceneEntityPlatformTransformOverrideAsset::get_PlatformId()
{
return this->PlatformId;
}

void SceneEntityPlatformTransformOverrideAsset::set_PlatformId(std::string value)
{
this->PlatformId = value;
}

bool SceneEntityPlatformTransformOverrideAsset::get_HasLocalPositionOverride()
{
return this->HasLocalPositionOverride;
}

void SceneEntityPlatformTransformOverrideAsset::set_HasLocalPositionOverride(bool value)
{
this->HasLocalPositionOverride = value;
}

::float3 SceneEntityPlatformTransformOverrideAsset::get_LocalPosition()
{
return this->LocalPosition;
}

void SceneEntityPlatformTransformOverrideAsset::set_LocalPosition(::float3 value)
{
this->LocalPosition = value;
}

bool SceneEntityPlatformTransformOverrideAsset::get_HasLocalScaleOverride()
{
return this->HasLocalScaleOverride;
}

void SceneEntityPlatformTransformOverrideAsset::set_HasLocalScaleOverride(bool value)
{
this->HasLocalScaleOverride = value;
}

::float3 SceneEntityPlatformTransformOverrideAsset::get_LocalScale()
{
return this->LocalScale;
}

void SceneEntityPlatformTransformOverrideAsset::set_LocalScale(::float3 value)
{
this->LocalScale = value;
}

bool SceneEntityPlatformTransformOverrideAsset::get_HasLocalOrientationOverride()
{
return this->HasLocalOrientationOverride;
}

void SceneEntityPlatformTransformOverrideAsset::set_HasLocalOrientationOverride(bool value)
{
this->HasLocalOrientationOverride = value;
}

::float4 SceneEntityPlatformTransformOverrideAsset::get_LocalOrientation()
{
return this->LocalOrientation;
}

void SceneEntityPlatformTransformOverrideAsset::set_LocalOrientation(::float4 value)
{
this->LocalOrientation = value;
}

