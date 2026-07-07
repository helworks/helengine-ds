#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class InputActionState
{
public:
    InputActionState();

    float Value;

    float get_Value();
    void set_Value(float value);

    bool IsDown;

    bool get_IsDown();
    void set_IsDown(bool value);

    bool WasPressed;

    bool get_WasPressed();
    void set_WasPressed(bool value);

    bool WasReleased;

    bool get_WasReleased();
    void set_WasReleased(bool value);
};
