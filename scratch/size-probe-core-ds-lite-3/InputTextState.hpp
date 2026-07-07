#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/array.hpp"

class InputTextState
{
public:
    InputTextState();

    Array<char>* Characters;

    Array<char>* get_Characters();
    void set_Characters(Array<char>* value);

    int32_t CharacterCount;

    int32_t get_CharacterCount();
    void set_CharacterCount(int32_t value);
};
