#ifdef DrawText
#undef DrawText
#endif
#include "InputContextId.hpp"
#include "InputContextId.hpp"

InputContextId::InputContextId() : Value(0)
{
}

int32_t InputContextId::get_Value()
{
return this->Value;
}

bool InputContextId::Equals(::InputContextId other)
{
return this->Value == other.Value;}

bool InputContextId::Equals(void* obj)
{
    if (obj != nullptr)
    {
return this->Equals((*static_cast<InputContextId*>(obj)));    }
return false;}

int32_t InputContextId::GetHashCode()
{
return this->Value;}

InputContextId::InputContextId(int32_t value) : Value(0)
{
this->Value = value;
}

bool operator!=(::InputContextId left, ::InputContextId right)
{
return !left.Equals(right);}

bool operator==(::InputContextId left, ::InputContextId right)
{
return left.Equals(right);}

