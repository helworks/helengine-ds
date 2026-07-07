#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityAsset.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/array.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityAsset.hpp"
#include "runtime/array.hpp"

int32_t SceneEntityAsset::get_LiveInstanceCount()
{
return LiveInstanceCountValue;
}

uint32_t SceneEntityAsset::get_Id()
{
return this->Id;
}

void SceneEntityAsset::set_Id(uint32_t value)
{
this->Id = value;
}

const std::string& SceneEntityAsset::get_Name()
{
return this->Name;
}

void SceneEntityAsset::set_Name(std::string value)
{
this->Name = value;
}

bool SceneEntityAsset::get_IsStatic()
{
return this->IsStatic;
}

void SceneEntityAsset::set_IsStatic(bool value)
{
this->IsStatic = value;
}

bool SceneEntityAsset::get_Enabled()
{
return this->Enabled;
}

void SceneEntityAsset::set_Enabled(bool value)
{
this->Enabled = value;
}

uint16_t SceneEntityAsset::get_LayerMask()
{
return this->LayerMask;
}

void SceneEntityAsset::set_LayerMask(uint16_t value)
{
this->LayerMask = value;
}

::float3 SceneEntityAsset::get_LocalPosition()
{
return this->LocalPosition;
}

void SceneEntityAsset::set_LocalPosition(::float3 value)
{
this->LocalPosition = value;
}

::float3 SceneEntityAsset::get_LocalScale()
{
return this->LocalScale;
}

void SceneEntityAsset::set_LocalScale(::float3 value)
{
this->LocalScale = value;
}

::float4 SceneEntityAsset::get_LocalOrientation()
{
return this->LocalOrientation;
}

void SceneEntityAsset::set_LocalOrientation(::float4 value)
{
this->LocalOrientation = value;
}

Array<::SceneComponentAssetRecord*>* SceneEntityAsset::get_Components()
{
return this->Components;
}

void SceneEntityAsset::set_Components(Array<::SceneComponentAssetRecord*>* value)
{
this->Components = value;
}

Array<::SceneEntityPlatformExistenceOverrideAsset*>* SceneEntityAsset::get_PlatformExistenceOverrides()
{
return this->PlatformExistenceOverrides;
}

void SceneEntityAsset::set_PlatformExistenceOverrides(Array<::SceneEntityPlatformExistenceOverrideAsset*>* value)
{
this->PlatformExistenceOverrides = value;
}

Array<::SceneEntityPlatformTransformOverrideAsset*>* SceneEntityAsset::get_PlatformTransformOverrides()
{
return this->PlatformTransformOverrides;
}

void SceneEntityAsset::set_PlatformTransformOverrides(Array<::SceneEntityPlatformTransformOverrideAsset*>* value)
{
this->PlatformTransformOverrides = value;
}

Array<::SceneEntityPlatformComponentOverrideAsset*>* SceneEntityAsset::get_PlatformComponentOverrides()
{
return this->PlatformComponentOverrides;
}

void SceneEntityAsset::set_PlatformComponentOverrides(Array<::SceneEntityPlatformComponentOverrideAsset*>* value)
{
this->PlatformComponentOverrides = value;
}

Array<::SceneEntityAsset*>* SceneEntityAsset::get_Children()
{
return this->Children;
}

void SceneEntityAsset::set_Children(Array<::SceneEntityAsset*>* value)
{
this->Children = value;
}

void SceneEntityAsset::MarkReleasedForDiagnostics()
{
LiveInstanceCountValue--;
}

SceneEntityAsset::SceneEntityAsset() : Id(0), Name(), IsStatic(), Enabled(true), LayerMask(), LocalPosition(), LocalScale(), LocalOrientation(), Components(Array<SceneComponentAssetRecord*>::Empty()), PlatformExistenceOverrides(Array<SceneEntityPlatformExistenceOverrideAsset*>::Empty()), PlatformTransformOverrides(Array<SceneEntityPlatformTransformOverrideAsset*>::Empty()), PlatformComponentOverrides(Array<SceneEntityPlatformComponentOverrideAsset*>::Empty()), Children(Array<SceneEntityAsset*>::Empty())
{
LiveInstanceCountValue++;
}

int32_t SceneEntityAsset::LiveInstanceCountValue = 0;

