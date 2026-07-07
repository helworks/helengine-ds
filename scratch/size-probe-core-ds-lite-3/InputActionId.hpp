#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_equatable.hpp"

class InputActionId
{
public:
    InputActionId();

    int32_t Value;

    int32_t get_Value();

    bool Equals(::InputActionId other);

    bool Equals(void* obj);

    int32_t GetHashCode();

    InputActionId(int32_t value);

    friend bool operator!=(::InputActionId left, ::InputActionId right);

    friend bool operator==(::InputActionId left, ::InputActionId right);
};
