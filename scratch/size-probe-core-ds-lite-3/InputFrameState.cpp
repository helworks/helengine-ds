#ifdef DrawText
#undef DrawText
#endif
#include "InputFrameState.hpp"
#include "KeyboardState.hpp"
#include "MouseState.hpp"
#include "InputPointerState.hpp"
#include "runtime/array.hpp"
#include "InputGamepadState.hpp"
#include "InputTextState.hpp"
#include "runtime/array.hpp"

InputFrameState::InputFrameState() : Keyboard(), Mouse(), Pointer(), Gamepads(), GamepadCount(0), Text()
{
}

::KeyboardState InputFrameState::get_Keyboard()
{
return this->Keyboard;
}

void InputFrameState::set_Keyboard(::KeyboardState value)
{
this->Keyboard = value;
}

::MouseState InputFrameState::get_Mouse()
{
return this->Mouse;
}

void InputFrameState::set_Mouse(::MouseState value)
{
this->Mouse = value;
}

::InputPointerState InputFrameState::get_Pointer()
{
return this->Pointer;
}

void InputFrameState::set_Pointer(::InputPointerState value)
{
this->Pointer = value;
}

Array<::InputGamepadState>* InputFrameState::get_Gamepads()
{
return this->Gamepads;
}

void InputFrameState::set_Gamepads(Array<::InputGamepadState>* value)
{
this->Gamepads = value;
}

int32_t InputFrameState::get_GamepadCount()
{
return this->GamepadCount;
}

void InputFrameState::set_GamepadCount(int32_t value)
{
this->GamepadCount = value;
}

::InputTextState InputFrameState::get_Text()
{
return this->Text;
}

void InputFrameState::set_Text(::InputTextState value)
{
this->Text = value;
}

