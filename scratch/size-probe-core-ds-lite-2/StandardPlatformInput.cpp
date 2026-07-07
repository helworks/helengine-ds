#ifdef DrawText
#undef DrawText
#endif
#include "StandardPlatformInput.hpp"
#include "runtime/native_exceptions.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformActionIds.hpp"
#include "StandardPlatformActionBinding.hpp"
#include "InputBinding.hpp"
#include "StandardPlatformInputConfiguration.hpp"
#include "StandardPlatformAction.hpp"
#include "IInputBackend.hpp"
#include "InputFrameState.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "InputActionState.hpp"
#include "MouseState.hpp"
#include "KeyboardState.hpp"
#include "int2.hpp"
#include "system/action.hpp"
#include "InputContextId.hpp"
#include "InputActionId.hpp"
#include "InputGamepadState.hpp"
#include "ButtonState.hpp"
#include "InputPointerState.hpp"
#include "InputTextState.hpp"
#include "Keys.hpp"
#include "InputGamepadButton.hpp"
#include "InputControlId.hpp"
#include "StandardPlatformInput.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

void StandardPlatformInput::Configure(::StandardPlatformInputConfiguration* configuration)
{
    if (configuration == nullptr)
    {
throw new ArgumentNullException("configuration");
    }
this->InputSystem->ClearBindings(StandardPlatformActionIds::ContextId);
this->InputSystem->RemoveContextInstances(StandardPlatformActionIds::ContextId);
for (int32_t index = 0; index < configuration->Bindings->get_Count(); index++) {
::StandardPlatformActionBinding *binding = (*configuration->Bindings).get_Item(static_cast<int32_t>(index));
this->InputSystem->RegisterBinding(([&]() {
auto __ctor_arg_0000017C = StandardPlatformActionIds::ContextId;
auto __ctor_arg_0000017D = StandardPlatformActionIds::GetActionId(static_cast<StandardPlatformAction>(binding->Action));
auto __ctor_arg_0000017E = binding->Control;
auto __ctor_arg_0000017F = 1.0f;
return ::InputBinding(__ctor_arg_0000017C, __ctor_arg_0000017D, __ctor_arg_0000017E, __ctor_arg_0000017F);
})());
}
    if (configuration->Bindings->get_Count() > 0)
    {
this->InputSystem->PushContext(StandardPlatformActionIds::ContextId);
    }
}

bool StandardPlatformInput::IsActionDown(::StandardPlatformAction action)
{
return this->InputSystem->IsActionDown(StandardPlatformActionIds::GetActionId(static_cast<StandardPlatformAction>(action)));}

StandardPlatformInput::StandardPlatformInput(::InputSystem* inputSystem) : InputSystem()
{
this->InputSystem = (inputSystem != nullptr ? inputSystem : throw new ArgumentNullException("inputSystem"));
}

bool StandardPlatformInput::WasActionPressed(::StandardPlatformAction action)
{
return this->InputSystem->WasActionPressed(StandardPlatformActionIds::GetActionId(static_cast<StandardPlatformAction>(action)));}

bool StandardPlatformInput::WasActionReleased(::StandardPlatformAction action)
{
return this->InputSystem->WasActionReleased(StandardPlatformActionIds::GetActionId(static_cast<StandardPlatformAction>(action)));}

