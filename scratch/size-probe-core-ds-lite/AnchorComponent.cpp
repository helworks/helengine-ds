#ifdef DrawText
#undef DrawText
#endif
#include "AnchorComponent.hpp"

uint8_t AnchorComponent::get_AnchorFlags()
{
return LayoutComponent::get_AnchorFlags();
}

void AnchorComponent::set_AnchorFlags(uint8_t value)
{
LayoutComponent::set_AnchorFlags(value);
}

::float4 AnchorComponent::get_AnchorDistances()
{
return LayoutComponent::get_AnchorDistances();
}

void AnchorComponent::set_AnchorDistances(::float4 value)
{
LayoutComponent::set_AnchorDistances(value);
}

uint8_t AnchorComponent::get_LayoutSpace()
{
return LayoutComponent::get_LayoutSpace();
}

void AnchorComponent::set_LayoutSpace(uint8_t value)
{
LayoutComponent::set_LayoutSpace(value);
}

bool AnchorComponent::get_IsAnchored()
{
return LayoutComponent::get_IsAnchored();
}

::AnchorSpace* AnchorComponent::get_AnchorSpace()
{
return LayoutComponent::get_AnchorSpace();
}

::Entity* AnchorComponent::get_Parent()
{
return Component::get_Parent();
}

void AnchorComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool AnchorComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* AnchorComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool AnchorComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

