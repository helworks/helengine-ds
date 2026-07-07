#ifdef DrawText
#undef DrawText
#endif
#include "InputSystem.hpp"
#include "InputFrameState.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "InputActionState.hpp"
#include "InputPointerState.hpp"
#include "InputGamepadState.hpp"
#include "InputTextState.hpp"
#include "int2.hpp"
#include "ButtonState.hpp"
#include "IInputBackend.hpp"
#include "MouseState.hpp"
#include "InputActionId.hpp"
#include "InputBinding.hpp"
#include "KeyboardState.hpp"
#include "InputPointerButton.hpp"
#include "InputDeviceKind.hpp"
#include "InputControlKind.hpp"
#include "system/action.hpp"
#include "InputContextId.hpp"
#include "Keys.hpp"
#include "InputGamepadButton.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "InputControlId.hpp"
#include "KeyState.hpp"
#include "InputSystem.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"

::IInputBackend* InputSystem::get_Backend()
{
return this->Backend;
}

void InputSystem::set_Backend(::IInputBackend* value)
{
this->Backend = value;
}

::InputFrameState InputSystem::get_CurrentFrame()
{
return this->CurrentFrame;
}

void InputSystem::set_CurrentFrame(::InputFrameState value)
{
this->CurrentFrame = value;
}

bool InputSystem::get_IsPointerWrapEnabled()
{
return this->ActivePointerWrapEnabled;}

void InputSystem::ClearBindings(::InputContextId contextId)
{
for (int32_t i = this->Bindings->get_Count() - 1; i >= 0; i--) {
    if ((*this->Bindings).get_Item(static_cast<int32_t>(i)).ContextId != contextId)
    {
continue;
    }
this->Bindings->RemoveAt(static_cast<int32_t>(i));
}
}

void InputSystem::ClearContexts()
{
this->ActiveContextStack->Clear();
}

void InputSystem::EarlyUpdate()
{
this->EnsureInputStateCaptured();
this->ResolveBindings();
}

::InputActionState InputSystem::GetActionState(::InputActionId actionId)
{
::InputActionState currentState;
    if (this->CurrentActionStates->TryGetValue(static_cast<int32_t>(actionId.Value), currentState))
    {
return currentState;    }
return ::InputActionState();}

float InputSystem::GetActionValue(::InputActionId actionId)
{
return this->GetActionState(actionId).Value;}

int32_t InputSystem::GetGamepadCount()
{
return this->CurrentFrame.GamepadCount;}

::InputGamepadState InputSystem::GetGamepadState(int32_t index)
{
    if (this->CurrentFrame.Gamepads == nullptr)
    {
return ::InputGamepadState();    }
    if (index < 0 || index >= this->CurrentFrame.GamepadCount || index >= this->CurrentFrame.Gamepads->get_Length())
    {
return ::InputGamepadState();    }
return (*this->CurrentFrame.Gamepads)[index];}

::int2 InputSystem::GetMouseDelta()
{
return this->mouseDelta;}

int32_t InputSystem::GetMouseDeltaX()
{
return this->mouseDelta.X;}

int32_t InputSystem::GetMouseDeltaY()
{
return this->mouseDelta.Y;}

::ButtonState InputSystem::GetMouseLeftButtonState()
{
return this->mouseState.get_LeftButton();}

::ButtonState InputSystem::GetMouseMiddleButtonState()
{
return this->mouseState.get_MiddleButton();}

::int2 InputSystem::GetMousePosition()
{
return ::int2(static_cast<int32_t>(this->mouseState.get_X()), static_cast<int32_t>(this->mouseState.get_Y()));}

::ButtonState InputSystem::GetMouseRightButtonState()
{
return this->mouseState.get_RightButton();}

int32_t InputSystem::GetMouseScrollWheelDelta()
{
return this->mouseState.get_ScrollWheelValue() - this->lastMouseState.get_ScrollWheelValue();}

int32_t InputSystem::GetMouseScrollWheelValue()
{
return this->mouseState.get_ScrollWheelValue();}

int32_t InputSystem::GetMouseX()
{
return this->mouseState.get_X();}

::ButtonState InputSystem::GetMouseXButton1State()
{
return this->mouseState.get_XButton1();}

::ButtonState InputSystem::GetMouseXButton2State()
{
return this->mouseState.get_XButton2();}

int32_t InputSystem::GetMouseY()
{
return this->mouseState.get_Y();}

::InputPointerState InputSystem::GetPointerState()
{
return this->CurrentFrame.Pointer;}

::InputGamepadState InputSystem::GetPreviousGamepadState(int32_t index)
{
    if (this->previousFrame.Gamepads == nullptr)
    {
return ::InputGamepadState();    }
    if (index < 0 || index >= this->previousFrame.GamepadCount || index >= this->previousFrame.Gamepads->get_Length())
    {
return ::InputGamepadState();    }
return (*this->previousFrame.Gamepads)[index];}

::InputTextState InputSystem::GetTextState()
{
return this->CurrentFrame.Text;}

InputSystem::InputSystem() : Backend(), CurrentFrame(), Bindings(), ActiveContextStack(), CurrentActionStates(), PreviousActionStates(), SeenActionIds(), PreviousActionKeysScratch(), CurrentActionKeysScratch(), ContextActionValuesScratch(), ContextActionKeysScratch(), ResolvedActionKeysScratch(), lastMouseState(), mouseState(), previousFrame(), lastKeyboardState(), keyboardState(), hasCapturedInput(), mouseDelta(), ActivePointerWrapEnabled(), RequestedPointerWrapEnabled(), MouseClientBounds(), PointerWrapDeltaOffset(), KeyboardIsActive(), FrameUpdateHandler()
{
this->Bindings = new List<::InputBinding>();
this->ActiveContextStack = new List<int32_t>();
this->CurrentActionStates = new Dictionary<int32_t, ::InputActionState>();
this->PreviousActionStates = new Dictionary<int32_t, ::InputActionState>();
this->SeenActionIds = new List<int32_t>();
this->PreviousActionKeysScratch = new List<int32_t>();
this->CurrentActionKeysScratch = new List<int32_t>();
this->ContextActionValuesScratch = new Dictionary<int32_t, float>();
this->ContextActionKeysScratch = new List<int32_t>();
this->ResolvedActionKeysScratch = new List<int32_t>();
this->keyboardState = ::KeyboardState();
this->mouseState = ::MouseState(static_cast<int32_t>(0), static_cast<int32_t>(0), static_cast<int32_t>(0), static_cast<ButtonState>(ButtonState::Released), static_cast<ButtonState>(ButtonState::Released), static_cast<ButtonState>(ButtonState::Released), static_cast<ButtonState>(ButtonState::Released), static_cast<ButtonState>(ButtonState::Released));
this->KeyboardIsActive = true;
this->set_CurrentFrame(([&]() {
auto __object_000000EE = ::InputFrameState();
__object_000000EE.set_Keyboard(this->keyboardState);
__object_000000EE.set_Mouse(this->mouseState);
return __object_000000EE;
})());
this->previousFrame = this->CurrentFrame;
}

bool InputSystem::IsActionDown(::InputActionId actionId)
{
return this->GetActionState(actionId).IsDown;}

bool InputSystem::IsKeyDown(::Keys key)
{
return this->keyboardState.IsKeyDown(static_cast<Keys>(key));}

bool InputSystem::IsKeyUp(::Keys key)
{
return this->keyboardState.IsKeyUp(static_cast<Keys>(key));}

void InputSystem::PopContext(::InputContextId contextId)
{
for (int32_t i = this->ActiveContextStack->get_Count() - 1; i >= 0; i--) {
    if ((*this->ActiveContextStack).get_Item(static_cast<int32_t>(i)) != contextId.Value)
    {
continue;
    }
this->ActiveContextStack->RemoveAt(static_cast<int32_t>(i));
return;}
}

void InputSystem::PushContext(::InputContextId contextId)
{
this->ActiveContextStack->Add(static_cast<int32_t>(contextId.Value));
}

void InputSystem::RegisterBinding(::InputBinding binding)
{
this->Bindings->Add(binding);
}

void InputSystem::RemoveContextInstances(::InputContextId contextId)
{
for (int32_t i = this->ActiveContextStack->get_Count() - 1; i >= 0; i--) {
    if ((*this->ActiveContextStack).get_Item(static_cast<int32_t>(i)) != contextId.Value)
    {
continue;
    }
this->ActiveContextStack->RemoveAt(static_cast<int32_t>(i));
}
}

void InputSystem::RequestPointerWrapEnabled()
{
this->RequestedPointerWrapEnabled = true;
}

void InputSystem::SetBackend(::IInputBackend* backend)
{
this->set_Backend(backend);
this->ApplyBackgroundInputPolicy();
}

void InputSystem::SetFrameUpdateHandler(Action<>* handler)
{
this->FrameUpdateHandler = handler;
}

void InputSystem::SetKeyboardActive(bool isActive)
{
this->KeyboardIsActive = isActive;
}

void InputSystem::SetKeyboardState(::KeyboardState state)
{
this->keyboardState = state;
::InputFrameState currentFrame = this->CurrentFrame;
currentFrame.set_Keyboard(state);
this->set_CurrentFrame(currentFrame);
}

void InputSystem::SetMouseClientBounds(::int2 clientBounds)
{
this->MouseClientBounds = clientBounds;
}

void InputSystem::SetMouseState(::MouseState state)
{
this->mouseState = state;
::InputFrameState currentFrame = this->CurrentFrame;
currentFrame.set_Mouse(state);
this->set_CurrentFrame(currentFrame);
}

void InputSystem::SetPointerWrapEnabled(bool isEnabled)
{
this->ActivePointerWrapEnabled = isEnabled;
this->RequestedPointerWrapEnabled = isEnabled;
}

void InputSystem::Update()
{
this->EnsureInputStateCaptured();
{
auto __finallyGuard_000000EF = he_cpp_make_scope_exit([&]() {
this->CommitPointerWrapState();
this->hasCapturedInput = false;
});
    if (this->FrameUpdateHandler != nullptr)
    {
(*this->FrameUpdateHandler)();
    }
}
}

bool InputSystem::WasActionPressed(::InputActionId actionId)
{
return this->GetActionState(actionId).WasPressed;}

bool InputSystem::WasActionReleased(::InputActionId actionId)
{
return this->GetActionState(actionId).WasReleased;}

bool InputSystem::WasGamepadButtonPressed(int32_t index, ::InputGamepadButton button)
{
::InputGamepadState currentState = this->GetGamepadState(static_cast<int32_t>(index));
    if (!currentState.Connected)
    {
return false;    }
::InputGamepadState previousState = this->GetPreviousGamepadState(static_cast<int32_t>(index));
return currentState.IsButtonDown(static_cast<InputGamepadButton>(button)) && !previousState.IsButtonDown(static_cast<InputGamepadButton>(button));}

bool InputSystem::WasGamepadButtonPressed(::InputGamepadState currentState, ::InputGamepadState previousState, ::InputGamepadButton button)
{
return currentState.IsButtonDown(static_cast<InputGamepadButton>(button)) && !previousState.IsButtonDown(static_cast<InputGamepadButton>(button));}

bool InputSystem::WasKeyPressed(::Keys key)
{
return this->keyboardState.IsKeyDown(static_cast<Keys>(key)) && this->lastKeyboardState.IsKeyUp(static_cast<Keys>(key));}

bool InputSystem::WasKeyReleased(::Keys key)
{
return this->keyboardState.IsKeyUp(static_cast<Keys>(key)) && this->lastKeyboardState.IsKeyDown(static_cast<Keys>(key));}

bool InputSystem::WasMouseLeftButtonPressed()
{
return this->WasMouseButtonPressed(static_cast<ButtonState>(this->mouseState.get_LeftButton()), static_cast<ButtonState>(this->lastMouseState.get_LeftButton()));}

bool InputSystem::WasMouseLeftButtonReleased()
{
return this->WasMouseButtonReleased(static_cast<ButtonState>(this->mouseState.get_LeftButton()), static_cast<ButtonState>(this->lastMouseState.get_LeftButton()));}

bool InputSystem::WasMouseMiddleButtonPressed()
{
return this->WasMouseButtonPressed(static_cast<ButtonState>(this->mouseState.get_MiddleButton()), static_cast<ButtonState>(this->lastMouseState.get_MiddleButton()));}

bool InputSystem::WasMouseMiddleButtonReleased()
{
return this->WasMouseButtonReleased(static_cast<ButtonState>(this->mouseState.get_MiddleButton()), static_cast<ButtonState>(this->lastMouseState.get_MiddleButton()));}

bool InputSystem::WasMouseRightButtonPressed()
{
return this->WasMouseButtonPressed(static_cast<ButtonState>(this->mouseState.get_RightButton()), static_cast<ButtonState>(this->lastMouseState.get_RightButton()));}

bool InputSystem::WasMouseRightButtonReleased()
{
return this->WasMouseButtonReleased(static_cast<ButtonState>(this->mouseState.get_RightButton()), static_cast<ButtonState>(this->lastMouseState.get_RightButton()));}

bool InputSystem::WasMouseXButton1Pressed()
{
return this->WasMouseButtonPressed(static_cast<ButtonState>(this->mouseState.get_XButton1()), static_cast<ButtonState>(this->lastMouseState.get_XButton1()));}

bool InputSystem::WasMouseXButton1Released()
{
return this->WasMouseButtonReleased(static_cast<ButtonState>(this->mouseState.get_XButton1()), static_cast<ButtonState>(this->lastMouseState.get_XButton1()));}

bool InputSystem::WasMouseXButton2Pressed()
{
return this->WasMouseButtonPressed(static_cast<ButtonState>(this->mouseState.get_XButton2()), static_cast<ButtonState>(this->lastMouseState.get_XButton2()));}

bool InputSystem::WasMouseXButton2Released()
{
return this->WasMouseButtonReleased(static_cast<ButtonState>(this->mouseState.get_XButton2()), static_cast<ButtonState>(this->lastMouseState.get_XButton2()));}

void InputSystem::ApplyActionTransitions()
{
this->ResolvedActionKeysScratch->Clear();
for (const auto& actionKey : this->CurrentActionStates->Keys()) {
this->ResolvedActionKeysScratch->Add(static_cast<int32_t>(actionKey));
}
for (int32_t i = 0; i < this->ResolvedActionKeysScratch->get_Count(); i++) {
const int32_t actionKey = (*this->ResolvedActionKeysScratch).get_Item(static_cast<int32_t>(i));
::InputActionState currentState = (*this->CurrentActionStates).get_Item(static_cast<int32_t>(actionKey));
::InputActionState previousState;
    if (!this->PreviousActionStates->TryGetValue(static_cast<int32_t>(actionKey), previousState))
    {
previousState = ::InputActionState();
    }
currentState.set_IsDown(currentState.Value != 0.0f);
currentState.set_WasPressed(currentState.IsDown && !previousState.IsDown);
currentState.set_WasReleased(!currentState.IsDown && previousState.IsDown);
(*this->CurrentActionStates).get_Item(static_cast<int32_t>(actionKey)) = currentState;
}
}

void InputSystem::ApplyBackgroundInputPolicy()
{
}

void InputSystem::ApplyPointerWrap()
{
    if (!this->ActivePointerWrapEnabled)
    {
return;    }
    if (this->MouseClientBounds.X <= 1 || this->MouseClientBounds.Y <= 1)
    {
return;    }
int32_t wrappedX = this->mouseState.get_X();
int32_t wrappedY = this->mouseState.get_Y();
int32_t deltaOffsetX = 0;
int32_t deltaOffsetY = 0;
    if (this->mouseState.get_X() <= 0)
    {
wrappedX = this->MouseClientBounds.X - 2;
deltaOffsetX = -(this->MouseClientBounds.X - 2);
    }
else {
    if (this->mouseState.get_X() >= this->MouseClientBounds.X - 1)
    {
wrappedX = 1;
deltaOffsetX = this->MouseClientBounds.X - 2;
    }
}
    if (this->mouseState.get_Y() <= 0)
    {
wrappedY = this->MouseClientBounds.Y - 2;
deltaOffsetY = -(this->MouseClientBounds.Y - 2);
    }
else {
    if (this->mouseState.get_Y() >= this->MouseClientBounds.Y - 1)
    {
wrappedY = 1;
deltaOffsetY = this->MouseClientBounds.Y - 2;
    }
}
    if (deltaOffsetX == 0 && deltaOffsetY == 0)
    {
return;    }
this->mouseState.set_X(wrappedX);
this->mouseState.set_Y(wrappedY);
this->PointerWrapDeltaOffset = ::int2(static_cast<int32_t>(deltaOffsetX), static_cast<int32_t>(deltaOffsetY));
::InputFrameState currentFrame = this->CurrentFrame;
currentFrame.set_Mouse(this->mouseState);
::InputPointerState pointer = currentFrame.Pointer;
pointer.set_X(wrappedX);
pointer.set_Y(wrappedY);
const int32_t pointerDeltaX = pointer.DeltaX + deltaOffsetX;
const int32_t pointerDeltaY = pointer.DeltaY + deltaOffsetY;
pointer.set_DeltaX(pointerDeltaX);
pointer.set_DeltaY(pointerDeltaY);
currentFrame.set_Pointer(pointer);
this->set_CurrentFrame(currentFrame);
}

void InputSystem::ApplyPointerWrapState()
{
    if (!this->ActivePointerWrapEnabled)
    {
return;    }
this->ApplyPointerWrap();
}

uint64_t InputSystem::BuildPointerButtonMask(::MouseState state)
{
uint64_t buttons = 0;
    if (state.get_LeftButton() == ButtonState::Pressed)
    {
buttons |= 1UL << static_cast<int32_t>(InputPointerButton::Primary);
    }
    if (state.get_RightButton() == ButtonState::Pressed)
    {
buttons |= 1UL << static_cast<int32_t>(InputPointerButton::Secondary);
    }
    if (state.get_MiddleButton() == ButtonState::Pressed)
    {
buttons |= 1UL << static_cast<int32_t>(InputPointerButton::Middle);
    }
    if (state.get_XButton1() == ButtonState::Pressed)
    {
buttons |= 1UL << static_cast<int32_t>(InputPointerButton::Back);
    }
    if (state.get_XButton2() == ButtonState::Pressed)
    {
buttons |= 1UL << static_cast<int32_t>(InputPointerButton::Forward);
    }
return buttons;}

void InputSystem::CaptureInputState()
{
this->previousFrame = this->CurrentFrame;
this->lastMouseState = this->mouseState;
this->lastKeyboardState = this->keyboardState;
    if (this->Backend != nullptr)
    {
::InputFrameState backendFrame = this->Backend->CaptureFrame();
    if (this->KeyboardIsActive)
    {
this->keyboardState = backendFrame.Keyboard;
    }
this->mouseState = backendFrame.Mouse;
this->set_CurrentFrame(backendFrame);
    }
else {
::InputFrameState currentFrame = this->CurrentFrame;
currentFrame.set_Keyboard(this->keyboardState);
currentFrame.set_Mouse(this->mouseState);
this->set_CurrentFrame(currentFrame);
}
this->ApplyPointerWrapState();
::int2 pointerWrapDeltaOffset = this->ConsumePointerWrapDeltaOffset();
this->mouseDelta = ::int2(static_cast<int32_t>(this->mouseState.get_X() - this->lastMouseState.get_X() + pointerWrapDeltaOffset.X), static_cast<int32_t>(this->mouseState.get_Y() - this->lastMouseState.get_Y() + pointerWrapDeltaOffset.Y));
::InputFrameState updatedFrame = this->CurrentFrame;
updatedFrame.set_Keyboard(this->keyboardState);
updatedFrame.set_Mouse(this->mouseState);
updatedFrame.set_Pointer(([&]() {
auto __object_000000F0 = ::InputPointerState();
__object_000000F0.set_Connected(true);
__object_000000F0.set_X(this->mouseState.get_X());
__object_000000F0.set_Y(this->mouseState.get_Y());
__object_000000F0.set_DeltaX(this->mouseDelta.X);
__object_000000F0.set_DeltaY(this->mouseDelta.Y);
__object_000000F0.set_ScrollDelta(this->GetMouseScrollWheelDelta());
__object_000000F0.set_Buttons(this->BuildPointerButtonMask(this->mouseState));
return __object_000000F0;
})());
this->set_CurrentFrame(updatedFrame);
this->hasCapturedInput = true;
}

void InputSystem::ClearActionStatesWithoutBindings()
{
this->CurrentActionStates->Clear();
this->PreviousActionStates->Clear();
this->SeenActionIds->Clear();
}

void InputSystem::CommitPointerWrapState()
{
this->ActivePointerWrapEnabled = this->RequestedPointerWrapEnabled;
this->RequestedPointerWrapEnabled = false;
}

::int2 InputSystem::ConsumePointerWrapDeltaOffset()
{
::int2 pointerWrapDeltaOffset = this->PointerWrapDeltaOffset;
this->PointerWrapDeltaOffset = ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));
return pointerWrapDeltaOffset;}

void InputSystem::EnsureInputStateCaptured()
{
    if (this->hasCapturedInput)
    {
return;    }
this->CaptureInputState();
}

float InputSystem::GetGamepadAxisValue(::InputGamepadState gamepad, int32_t axisIndex)
{
switch (axisIndex) {
case 0: {
return gamepad.LeftStickX;}
case 1: {
return gamepad.LeftStickY;}
case 2: {
return gamepad.RightStickX;}
case 3: {
return gamepad.RightStickY;}
case 4: {
return gamepad.LeftTrigger;}
case 5: {
return gamepad.RightTrigger;}
default:  {
return 0.0f;}
}

}

bool InputSystem::IsGamepadButtonDown(::InputGamepadState gamepad, int32_t buttonIndex)
{
    if (buttonIndex < 0 || buttonIndex >= 64)
    {
return false;    }
return (gamepad.Buttons & (1UL << buttonIndex)) != 0;}

bool InputSystem::IsPointerButtonDown(::InputPointerState pointer, int32_t buttonIndex)
{
    if (buttonIndex < 0 || buttonIndex >= 64)
    {
return false;    }
return (pointer.Buttons & (1UL << buttonIndex)) != 0;}

float InputSystem::ResolveBindingValue(::InputBinding binding)
{
switch (binding.Control.DeviceKind) {
case InputDeviceKind::Gamepad: {
return this->ResolveGamepadBindingValue(binding);}
case InputDeviceKind::Pointer: {
return this->ResolvePointerBindingValue(binding);}
default:  {
return 0.0f;}
}

}

void InputSystem::ResolveBindings()
{
    if (this->Bindings->get_Count() == 0 || this->ActiveContextStack->get_Count() == 0)
    {
    if (this->CurrentActionStates->get_Count() == 0 && this->PreviousActionStates->get_Count() == 0)
    {
this->SeenActionIds->Clear();
return;    }
this->ClearActionStatesWithoutBindings();
return;    }
this->PreviousActionKeysScratch->Clear();
for (const auto& actionKey : this->PreviousActionStates->Keys()) {
this->PreviousActionKeysScratch->Add(static_cast<int32_t>(actionKey));
}
for (int32_t index = 0; index < this->PreviousActionKeysScratch->get_Count(); index++) {
const int32_t actionKey = (*this->PreviousActionKeysScratch).get_Item(static_cast<int32_t>(index));
this->PreviousActionStates->Remove(static_cast<int32_t>(actionKey));
}
this->CurrentActionKeysScratch->Clear();
for (const auto& actionKey : this->CurrentActionStates->Keys()) {
this->CurrentActionKeysScratch->Add(static_cast<int32_t>(actionKey));
}
for (int32_t index = 0; index < this->CurrentActionKeysScratch->get_Count(); index++) {
const int32_t actionKey = (*this->CurrentActionKeysScratch).get_Item(static_cast<int32_t>(index));
(*this->PreviousActionStates).get_Item(static_cast<int32_t>(actionKey)) = (*this->CurrentActionStates).get_Item(static_cast<int32_t>(actionKey));
this->CurrentActionStates->Remove(static_cast<int32_t>(actionKey));
}
this->SeenActionIds->Clear();
for (int32_t contextIndex = this->ActiveContextStack->get_Count() - 1; contextIndex >= 0; contextIndex--) {
const int32_t activeContextValue = (*this->ActiveContextStack).get_Item(static_cast<int32_t>(contextIndex));
this->ContextActionValuesScratch->Clear();
this->ContextActionKeysScratch->Clear();
for (int32_t bindingIndex = 0; bindingIndex < this->Bindings->get_Count(); bindingIndex++) {
::InputBinding binding = (*this->Bindings).get_Item(static_cast<int32_t>(bindingIndex));
    if (binding.ContextId.Value != activeContextValue)
    {
continue;
    }
const float value = this->ResolveBindingValue(binding);
    if (!this->SeenActionIds->Contains(static_cast<int32_t>(binding.ActionId.Value)))
    {
this->SeenActionIds->Add(static_cast<int32_t>(binding.ActionId.Value));
    }
    if (value == 0.0f)
    {
continue;
    }
const int32_t actionKey = binding.ActionId.Value;
float currentContextValue;
    if (this->ContextActionValuesScratch->TryGetValue(static_cast<int32_t>(actionKey), currentContextValue) && Math::Abs(value) <= Math::Abs(currentContextValue))
    {
continue;
    }
(*this->ContextActionValuesScratch).get_Item(static_cast<int32_t>(actionKey)) = value;
    if (!this->ContextActionKeysScratch->Contains(static_cast<int32_t>(actionKey)))
    {
this->ContextActionKeysScratch->Add(static_cast<int32_t>(actionKey));
    }
}
for (const auto& actionKey : *this->ContextActionKeysScratch) {
    if (CurrentActionStates->ContainsKey(static_cast<int32_t>(actionKey)))
    {
continue;
    }
StoreActionValue(::InputActionId(static_cast<int32_t>(actionKey)), (*ContextActionValuesScratch).get_Item(static_cast<int32_t>(actionKey)));
}
}
for (const auto& actionKey : *this->SeenActionIds) {
    if (CurrentActionStates->ContainsKey(static_cast<int32_t>(actionKey)))
    {
continue;
    }
::InputActionState previousState;
    if (!PreviousActionStates->TryGetValue(static_cast<int32_t>(actionKey), previousState))
    {
previousState = ::InputActionState();
    }
(*CurrentActionStates).get_Item(static_cast<int32_t>(actionKey)) = ([&]() {
auto __object_000000F1 = ::InputActionState();
__object_000000F1.set_Value(0.0f);
__object_000000F1.set_IsDown(false);
__object_000000F1.set_WasPressed(false);
__object_000000F1.set_WasReleased(previousState.IsDown);
return __object_000000F1;
})();
}
this->ApplyActionTransitions();
}

float InputSystem::ResolveGamepadBindingValue(::InputBinding binding)
{
::InputGamepadState gamepad;
    if (!this->TryGetGamepad__out1(static_cast<int32_t>(binding.Control.DeviceIndex), gamepad))
    {
return 0.0f;    }
switch (binding.Control.ControlKind) {
case InputControlKind::Button: {
return this->IsGamepadButtonDown(gamepad, static_cast<int32_t>(binding.Control.ControlIndex)) ? binding.Scale : 0.0f;}
case InputControlKind::Axis: {
return this->GetGamepadAxisValue(gamepad, static_cast<int32_t>(binding.Control.ControlIndex)) * binding.Scale;}
default:  {
return 0.0f;}
}

}

float InputSystem::ResolvePointerBindingValue(::InputBinding binding)
{
switch (binding.Control.ControlKind) {
case InputControlKind::Button: {
return this->IsPointerButtonDown(this->CurrentFrame.Pointer, static_cast<int32_t>(binding.Control.ControlIndex)) ? binding.Scale : 0.0f;}
case InputControlKind::PointerDelta: {
    if (binding.Control.ControlIndex == 0)
    {
return this->CurrentFrame.Pointer.DeltaX * binding.Scale;    }
return this->CurrentFrame.Pointer.DeltaY * binding.Scale;}
case InputControlKind::ScrollWheel: {
return this->CurrentFrame.Pointer.ScrollDelta * binding.Scale;}
default:  {
return 0.0f;}
}

}

void InputSystem::StoreActionValue(::InputActionId actionId, float value)
{
const int32_t key = actionId.Value;
::InputActionState currentState;
    if (!this->CurrentActionStates->TryGetValue(static_cast<int32_t>(key), currentState))
    {
currentState = ::InputActionState();
    }
    if (Math::Abs(value) <= Math::Abs(currentState.Value))
    {
return;    }
currentState.set_Value(value);
currentState.set_IsDown(value != 0.0f);
(*this->CurrentActionStates).get_Item(static_cast<int32_t>(key)) = currentState;
}

bool InputSystem::TryGetGamepad__out1(int32_t deviceIndex, ::InputGamepadState& gamepad)
{
gamepad = ::InputGamepadState();
    if (this->CurrentFrame.Gamepads == nullptr)
    {
return false;    }
    if (deviceIndex < 0 || deviceIndex >= this->CurrentFrame.GamepadCount || deviceIndex >= this->CurrentFrame.Gamepads->get_Length())
    {
return false;    }
gamepad = (*this->CurrentFrame.Gamepads)[deviceIndex];
return gamepad.Connected;}

bool InputSystem::WasMouseButtonPressed(::ButtonState currentState, ::ButtonState previousState)
{
return currentState == ButtonState::Pressed && previousState == ButtonState::Released;}

bool InputSystem::WasMouseButtonReleased(::ButtonState currentState, ::ButtonState previousState)
{
return currentState == ButtonState::Released && previousState == ButtonState::Pressed;}

