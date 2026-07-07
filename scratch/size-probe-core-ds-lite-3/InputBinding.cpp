#ifdef DrawText
#undef DrawText
#endif
#include "InputBinding.hpp"
#include "InputContextId.hpp"
#include "InputActionId.hpp"
#include "InputControlId.hpp"
#include "InputBinding.hpp"

InputBinding::InputBinding() : ContextId(), ActionId(), Control(), Scale()
{
}

::InputContextId InputBinding::get_ContextId()
{
return this->ContextId;
}

::InputActionId InputBinding::get_ActionId()
{
return this->ActionId;
}

::InputControlId InputBinding::get_Control()
{
return this->Control;
}

float InputBinding::get_Scale()
{
return this->Scale;
}

InputBinding::InputBinding(::InputContextId contextId, ::InputActionId actionId, ::InputControlId control, float scale) : ContextId(), ActionId(), Control(), Scale()
{
this->ContextId = contextId;
this->ActionId = actionId;
this->Control = control;
this->Scale = scale;
}

