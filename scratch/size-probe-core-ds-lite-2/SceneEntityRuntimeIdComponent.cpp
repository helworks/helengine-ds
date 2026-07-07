#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityRuntimeIdComponent.hpp"

SceneEntityRuntimeIdComponent::SceneEntityRuntimeIdComponent() : SceneEntityId(0)
{
}

uint32_t SceneEntityRuntimeIdComponent::get_SceneEntityId()
{
return this->SceneEntityId;
}

void SceneEntityRuntimeIdComponent::set_SceneEntityId(uint32_t value)
{
this->SceneEntityId = value;
}

::Entity* SceneEntityRuntimeIdComponent::get_Parent()
{
return Component::get_Parent();
}

void SceneEntityRuntimeIdComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool SceneEntityRuntimeIdComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* SceneEntityRuntimeIdComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool SceneEntityRuntimeIdComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

