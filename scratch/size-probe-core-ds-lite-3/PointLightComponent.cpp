#ifdef DrawText
#undef DrawText
#endif
#include "PointLightComponent.hpp"
#include "LightType.hpp"
#include "PointLightComponent.hpp"

float PointLightComponent::get_Range()
{
return this->Range;
}

void PointLightComponent::set_Range(float value)
{
this->Range = value;
}

PointLightComponent::PointLightComponent() : ::LightComponent(LightType::Point), Range()
{
this->set_Range(10.0f);
}

::LightType PointLightComponent::get_LightType()
{
return LightComponent::get_LightType();
}

::float4 PointLightComponent::get_Color()
{
return LightComponent::get_Color();
}

void PointLightComponent::set_Color(::float4 value)
{
LightComponent::set_Color(value);
}

float PointLightComponent::get_Intensity()
{
return LightComponent::get_Intensity();
}

void PointLightComponent::set_Intensity(float value)
{
LightComponent::set_Intensity(value);
}

bool PointLightComponent::get_ShadowsEnabled()
{
return LightComponent::get_ShadowsEnabled();
}

void PointLightComponent::set_ShadowsEnabled(bool value)
{
LightComponent::set_ShadowsEnabled(value);
}

::ShadowMapMode PointLightComponent::get_ShadowMapMode()
{
return LightComponent::get_ShadowMapMode();
}

void PointLightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
LightComponent::set_ShadowMapMode(value);
}

float PointLightComponent::get_ShadowStrength()
{
return LightComponent::get_ShadowStrength();
}

void PointLightComponent::set_ShadowStrength(float value)
{
LightComponent::set_ShadowStrength(value);
}

::Entity* PointLightComponent::get_Parent()
{
return Component::get_Parent();
}

void PointLightComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool PointLightComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* PointLightComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool PointLightComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

