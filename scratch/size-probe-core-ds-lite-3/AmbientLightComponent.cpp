#ifdef DrawText
#undef DrawText
#endif
#include "AmbientLightComponent.hpp"
#include "LightType.hpp"

AmbientLightComponent::AmbientLightComponent() : ::LightComponent(LightType::Ambient)
{
}

::LightType AmbientLightComponent::get_LightType()
{
return LightComponent::get_LightType();
}

::float4 AmbientLightComponent::get_Color()
{
return LightComponent::get_Color();
}

void AmbientLightComponent::set_Color(::float4 value)
{
LightComponent::set_Color(value);
}

float AmbientLightComponent::get_Intensity()
{
return LightComponent::get_Intensity();
}

void AmbientLightComponent::set_Intensity(float value)
{
LightComponent::set_Intensity(value);
}

bool AmbientLightComponent::get_ShadowsEnabled()
{
return LightComponent::get_ShadowsEnabled();
}

void AmbientLightComponent::set_ShadowsEnabled(bool value)
{
LightComponent::set_ShadowsEnabled(value);
}

::ShadowMapMode AmbientLightComponent::get_ShadowMapMode()
{
return LightComponent::get_ShadowMapMode();
}

void AmbientLightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
LightComponent::set_ShadowMapMode(value);
}

float AmbientLightComponent::get_ShadowStrength()
{
return LightComponent::get_ShadowStrength();
}

void AmbientLightComponent::set_ShadowStrength(float value)
{
LightComponent::set_ShadowStrength(value);
}

::Entity* AmbientLightComponent::get_Parent()
{
return Component::get_Parent();
}

void AmbientLightComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool AmbientLightComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* AmbientLightComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool AmbientLightComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

