#ifdef DrawText
#undef DrawText
#endif
#include "StandardPlatformActionBinding.hpp"
#include "StandardPlatformAction.hpp"
#include "InputControlId.hpp"
#include "StandardPlatformActionBinding.hpp"

::StandardPlatformAction StandardPlatformActionBinding::get_Action()
{
return this->Action;
}

::InputControlId StandardPlatformActionBinding::get_Control()
{
return this->Control;
}

StandardPlatformActionBinding::StandardPlatformActionBinding(::StandardPlatformAction action, ::InputControlId control) : Action(), Control()
{
this->Action = action;
this->Control = control;
}

