#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class float4;
class Entity;

#include "Component.hpp"
#include "runtime/native_disposable.hpp"
#include "LightType.hpp"
#include "float4.hpp"
#include "ShadowMapMode.hpp"

class LightComponent : public ::Component
{
public:
    virtual ~LightComponent() = default;

    ::LightType LightType;

    ::LightType get_LightType();

    ::float4 Color;

    ::float4 get_Color();
    void set_Color(::float4 value);

    float Intensity;

    float get_Intensity();
    void set_Intensity(float value);

    bool ShadowsEnabled;

    bool get_ShadowsEnabled();
    void set_ShadowsEnabled(bool value);

    ::ShadowMapMode ShadowMapMode;

    ::ShadowMapMode get_ShadowMapMode();
    void set_ShadowMapMode(::ShadowMapMode value);

    float ShadowStrength;

    float get_ShadowStrength();
    void set_ShadowStrength(float value);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void ParentEnabledChange(bool newEnabled);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
protected:
    LightComponent(::LightType lightType);
private:
    void RegisterWithObjectManager();

    void RemoveFromObjectManager();
};
