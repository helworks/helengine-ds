#ifdef DrawText
#undef DrawText
#endif
#include "SpotLightComponent.hpp"
#include "LightType.hpp"
#include "SpotLightComponent.hpp"

float SpotLightComponent::get_Range()
{
return this->Range;
}

void SpotLightComponent::set_Range(float value)
{
this->Range = value;
}

float SpotLightComponent::get_InnerConeAngleDegrees()
{
return this->InnerConeAngleDegrees;
}

void SpotLightComponent::set_InnerConeAngleDegrees(float value)
{
this->InnerConeAngleDegrees = value;
}

float SpotLightComponent::get_OuterConeAngleDegrees()
{
return this->OuterConeAngleDegrees;
}

void SpotLightComponent::set_OuterConeAngleDegrees(float value)
{
this->OuterConeAngleDegrees = value;
}

SpotLightComponent::SpotLightComponent() : ::LightComponent(LightType::Spot), Range(), InnerConeAngleDegrees(), OuterConeAngleDegrees()
{
this->set_Range(10.0f);
this->set_InnerConeAngleDegrees(25.0f);
this->set_OuterConeAngleDegrees(45.0f);
}

::LightType SpotLightComponent::get_LightType()
{
return LightComponent::get_LightType();
}

::float4 SpotLightComponent::get_Color()
{
return LightComponent::get_Color();
}

void SpotLightComponent::set_Color(::float4 value)
{
LightComponent::set_Color(value);
}

float SpotLightComponent::get_Intensity()
{
return LightComponent::get_Intensity();
}

void SpotLightComponent::set_Intensity(float value)
{
LightComponent::set_Intensity(value);
}

bool SpotLightComponent::get_ShadowsEnabled()
{
return LightComponent::get_ShadowsEnabled();
}

void SpotLightComponent::set_ShadowsEnabled(bool value)
{
LightComponent::set_ShadowsEnabled(value);
}

::ShadowMapMode SpotLightComponent::get_ShadowMapMode()
{
return LightComponent::get_ShadowMapMode();
}

void SpotLightComponent::set_ShadowMapMode(::ShadowMapMode value)
{
LightComponent::set_ShadowMapMode(value);
}

float SpotLightComponent::get_ShadowStrength()
{
return LightComponent::get_ShadowStrength();
}

void SpotLightComponent::set_ShadowStrength(float value)
{
LightComponent::set_ShadowStrength(value);
}

::Entity* SpotLightComponent::get_Parent()
{
return Component::get_Parent();
}

void SpotLightComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool SpotLightComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* SpotLightComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool SpotLightComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

