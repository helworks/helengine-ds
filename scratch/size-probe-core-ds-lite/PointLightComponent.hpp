#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class LightComponent;

#include "LightComponent.hpp"
#include "runtime/native_disposable.hpp"

class PointLightComponent : public ::LightComponent
{
public:
    virtual ~PointLightComponent() = default;

    float Range;

    float get_Range();
    void set_Range(float value);

    PointLightComponent();

    ::LightType get_LightType();

    ::float4 get_Color();

    void set_Color(::float4 value);

    float get_Intensity();

    void set_Intensity(float value);

    bool get_ShadowsEnabled();

    void set_ShadowsEnabled(bool value);

    ::ShadowMapMode get_ShadowMapMode();

    void set_ShadowMapMode(::ShadowMapMode value);

    float get_ShadowStrength();

    void set_ShadowStrength(float value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
};
