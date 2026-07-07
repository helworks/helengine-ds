#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float3;
class float4;
class float4x4;
class Component;

#include "runtime/native_disposable.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

class Entity : public ::IDisposable
{
public:
    virtual ~Entity() = default;

    virtual ::float3 get_Position();

    virtual void set_Position(::float3 value);

    ::float3 get_LocalPosition();

    void set_LocalPosition(::float3 value);

    ::float3 get_Scale();

    void set_Scale(::float3 value);

    ::float3 get_LocalScale();

    void set_LocalScale(::float3 value);

    ::float4 get_Orientation();

    void set_Orientation(::float4 value);

    ::float4 get_LocalOrientation();

    void set_LocalOrientation(::float4 value);

    ::float4x4 get_LocalTransformMatrix();

    ::float4x4 get_WorldTransformMatrix();

    ::Entity* Parent;

    ::Entity* get_Parent();
    void set_Parent(::Entity* value);

    ::Entity* get_ParentUnsafe();

    uint16_t get_LayerMask();

    void set_LayerMask(uint16_t value);

    List<::Component*>* get_Components();

    void set_Components(List<::Component*>* value);

    List<::Entity*>* get_Children();

    void set_Children(List<::Entity*>* value);

    bool get_Enabled();

    void set_Enabled(bool value);

    bool get_IsHierarchyEnabled();

    bool get_IsInitialized();

    bool get_IsDisposed();

    bool get_Static();

    void set_Static(bool value);

    void AddChild(::Entity* entity);

    void AddComponent(::Component* comp);

    void Dispose();

    Entity();

    void InitChildren();

    void InitComponents();

    void InitializeHierarchy();

    void RemoveChild(::Entity* entity);

    void RemoveComponent(::Component* comp);
protected:
    virtual void ParentEnabledChange(bool newEnabled);

    virtual void ParentStaticChange(bool newEnabled);
private:
    bool isEnabled;

    bool isStatic;

    bool isInitialized;

    bool isDisposing;

    bool isDisposed;

    ::float3 position;

    ::float3 scale;

    ::float4 orientation;

    uint16_t layerMask;

    List<::Component*>* components;

    List<::Entity*>* children;

    static ::float4x4 CreateTransformMatrix(::float3 position, ::float3 scale, ::float4 orientation);

    void RefreshRegistrationsAfterParentChange();

    static void RefreshRegistrationsAfterParentChangeRecursive(::Entity* entity);

    void ReportChildDisposalStage(std::string stage, ::Entity* entity, int32_t componentIndex);

    void ReportChildDisposalStage(std::string stage, ::Entity* entity);

    void ReportDisposalStage(std::string stage, int32_t componentIndex);

    bool ShouldSuppressRegistrationRefreshForDetachment(::Entity* entity);

    void ThrowIfDisposed();
};
