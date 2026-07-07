#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float3;
class float4;
class SceneComponentAssetRecord;
class SceneEntityPlatformExistenceOverrideAsset;
class SceneEntityPlatformTransformOverrideAsset;
class SceneEntityPlatformComponentOverrideAsset;

#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

class SceneEntityAsset
{
public:
    virtual ~SceneEntityAsset() = default;

    static int32_t get_LiveInstanceCount();

    uint32_t Id;

    uint32_t get_Id();
    void set_Id(uint32_t value);

    std::string Name;

    const std::string& get_Name();
    void set_Name(std::string value);

    bool IsStatic;

    bool get_IsStatic();
    void set_IsStatic(bool value);

    bool Enabled;

    bool get_Enabled();
    void set_Enabled(bool value);

    uint16_t LayerMask;

    uint16_t get_LayerMask();
    void set_LayerMask(uint16_t value);

    ::float3 LocalPosition;

    ::float3 get_LocalPosition();
    void set_LocalPosition(::float3 value);

    ::float3 LocalScale;

    ::float3 get_LocalScale();
    void set_LocalScale(::float3 value);

    ::float4 LocalOrientation;

    ::float4 get_LocalOrientation();
    void set_LocalOrientation(::float4 value);

    Array<::SceneComponentAssetRecord*>* Components;

    Array<::SceneComponentAssetRecord*>* get_Components();
    void set_Components(Array<::SceneComponentAssetRecord*>* value);

    Array<::SceneEntityPlatformExistenceOverrideAsset*>* PlatformExistenceOverrides;

    Array<::SceneEntityPlatformExistenceOverrideAsset*>* get_PlatformExistenceOverrides();
    void set_PlatformExistenceOverrides(Array<::SceneEntityPlatformExistenceOverrideAsset*>* value);

    Array<::SceneEntityPlatformTransformOverrideAsset*>* PlatformTransformOverrides;

    Array<::SceneEntityPlatformTransformOverrideAsset*>* get_PlatformTransformOverrides();
    void set_PlatformTransformOverrides(Array<::SceneEntityPlatformTransformOverrideAsset*>* value);

    Array<::SceneEntityPlatformComponentOverrideAsset*>* PlatformComponentOverrides;

    Array<::SceneEntityPlatformComponentOverrideAsset*>* get_PlatformComponentOverrides();
    void set_PlatformComponentOverrides(Array<::SceneEntityPlatformComponentOverrideAsset*>* value);

    Array<::SceneEntityAsset*>* Children;

    Array<::SceneEntityAsset*>* get_Children();
    void set_Children(Array<::SceneEntityAsset*>* value);

    void MarkReleasedForDiagnostics();

    SceneEntityAsset();
private:
    static int32_t LiveInstanceCountValue;
};
