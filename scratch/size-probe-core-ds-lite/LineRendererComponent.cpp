#ifdef DrawText
#undef DrawText
#endif
#include "LineRendererComponent.hpp"

::Entity* LineRendererComponent::get_Parent()
{
return Component::get_Parent();
}

void LineRendererComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool LineRendererComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* LineRendererComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool LineRendererComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

