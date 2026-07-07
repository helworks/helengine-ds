#ifdef DrawText
#undef DrawText
#endif
#include "InputTextState.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

InputTextState::InputTextState() : Characters(), CharacterCount(0)
{
}

Array<char>* InputTextState::get_Characters()
{
return this->Characters;
}

void InputTextState::set_Characters(Array<char>* value)
{
this->Characters = value;
}

int32_t InputTextState::get_CharacterCount()
{
return this->CharacterCount;
}

void InputTextState::set_CharacterCount(int32_t value)
{
this->CharacterCount = value;
}

