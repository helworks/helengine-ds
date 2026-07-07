#ifdef DrawText
#undef DrawText
#endif
#include "ComponentExecutionPolicy.hpp"
#include "runtime/native_exceptions.hpp"
#include "ComponentExecutionContext.hpp"
#include "ComponentExecutionMode.hpp"
#include "ComponentExecutionPolicy.hpp"
#include "RunInEditorAttribute.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"
#include "UpdateComponent.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_type.hpp"

bool ComponentExecutionPolicy::ShouldRunComponentLifecycle(::Component* component, ::Entity* entity)
{
    if (component == nullptr)
    {
throw new ArgumentNullException("component");
    }
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
    if (ComponentExecutionContext::get_CurrentMode() != ComponentExecutionMode::Editor)
    {
return true;    }
    if (!ComponentExecutionPolicy::HasEditorUpdateExecutionSuppressionMarker(entity))
    {
return true;    }
    if ()
    {
return true;    }
return Attribute::IsDefined(component->GetType(), he_cpp_type_of<RunInEditorAttribute>("RunInEditorAttribute"), true);}

bool ComponentExecutionPolicy::HasEditorUpdateExecutionSuppressionMarker(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return false;    }
for (int32_t index = 0; index < entity->get_Components()->get_Count(); index++) {
::Component *component = (*entity->get_Components()).get_Item(static_cast<int32_t>(index));
    if (component != nullptr && component->get_IsEditorUpdateExecutionSuppressionMarker())
    {
return true;    }
}
return false;}

