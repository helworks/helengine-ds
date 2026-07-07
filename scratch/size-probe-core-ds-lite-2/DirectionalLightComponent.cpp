#ifdef DrawText
#undef DrawText
#endif
#include "DirectionalLightComponent.hpp"
#include "LightType.hpp"
#include "LightComponent.hpp"
#include "DirectionalLightComponent.hpp"

float DirectionalLightComponent::get_ShadowDistance()
{
return this->ShadowDistance;
}

void DirectionalLightComponent::set_ShadowDistance(float value)
{
this->ShadowDistance = value;
}

DirectionalLightComponent::DirectionalLightComponent() : ::LightComponent(LightType::Directional), ShadowDistance()
{
this->set_ShadowsEnabled(true);
this->set_ShadowDistance(DefaultShadowDistance);
}

::LightType DirectionalLightComponent::get_LightType()
{
return LightComponent::get_LightType();
}

::float4 DirectionalLightComponent::get_Color()
{
return LightComponent::get_Color();
}

void DirectionalLightComponent::set_Color(::float4 value)
{
LightComponent::set_Color(value);
}

float DirectionalLightComponent::get_Intensity()
{
return LightComponent::get_Intensity();
}

void DirectionalLightComponent::set_Intensity(float value)
{
LightComponent::set_Intensity(value);
}

bool DirectionalLightComponent::get_ShadowsEnabled()
{
return LightComponent::get_ShadowsEnabled();
}

void DirectionalLightComponent::set_ShadowsEnabled(bool value)
{
LightComponent::set_ShadowsEnabled(value);
}

::ShadowMapMode DirectionalLightComponent::get_ShadowMapMode()
{
return LightComponent::get_ShadowMapMode();
}

void DirectionalLightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
LightComponent::set_ShadowMapMode(value);
}

float DirectionalLightComponent::get_ShadowStrength()
{
return LightComponent::get_ShadowStrength();
}

void DirectionalLightComponent::set_ShadowStrength(float value)
{
LightComponent::set_ShadowStrength(value);
}

::Entity* DirectionalLightComponent::get_Parent()
{
return Component::get_Parent();
}

void DirectionalLightComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool DirectionalLightComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* DirectionalLightComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool DirectionalLightComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

