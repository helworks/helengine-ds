#ifdef DrawText
#undef DrawText
#endif
#include "InputActionId.hpp"
#include "InputActionId.hpp"

InputActionId::InputActionId() : Value(0)
{
}

int32_t InputActionId::get_Value()
{
return this->Value;
}

bool InputActionId::Equals(::InputActionId other)
{
return this->Value == other.Value;}

bool InputActionId::Equals(void* obj)
{
    if (obj != nullptr)
    {
return this->Equals((*static_cast<InputActionId*>(obj)));    }
return false;}

int32_t InputActionId::GetHashCode()
{
return this->Value;}

InputActionId::InputActionId(int32_t value) : Value(0)
{
this->Value = value;
}

bool operator!=(::InputActionId left, ::InputActionId right)
{
return !left.Equals(right);}

bool operator==(::InputActionId left, ::InputActionId right)
{
return left.Equals(right);}

