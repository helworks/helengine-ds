#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class int2;

#include "int2.hpp"
#include "ButtonState.hpp"
#include "runtime/native_string.hpp"

class MouseState
{
public:
    MouseState();

    int32_t get_X();

    void set_X(int32_t value);

    int32_t get_Y();

    void set_Y(int32_t value);

    ::int2 get_Position();

    ::ButtonState get_LeftButton();

    void set_LeftButton(::ButtonState value);

    ::ButtonState get_MiddleButton();

    void set_MiddleButton(::ButtonState value);

    ::ButtonState get_RightButton();

    void set_RightButton(::ButtonState value);

    int32_t get_ScrollWheelValue();

    void set_ScrollWheelValue(int32_t value);

    int32_t get_HorizontalScrollWheelValue();

    void set_HorizontalScrollWheelValue(int32_t value);

    ::ButtonState get_XButton1();

    void set_XButton1(::ButtonState value);

    ::ButtonState get_XButton2();

    void set_XButton2(::ButtonState value);

    bool Equals(void* obj);

    int32_t GetHashCode();

    MouseState(int32_t x, int32_t y, int32_t scrollWheel, ::ButtonState leftButton, ::ButtonState middleButton, ::ButtonState rightButton, ::ButtonState xButton1, ::ButtonState xButton2);

    MouseState(int32_t x, int32_t y, int32_t scrollWheel, ::ButtonState leftButton, ::ButtonState middleButton, ::ButtonState rightButton, ::ButtonState xButton1, ::ButtonState xButton2, int32_t horizontalScrollWheel);

    std::string ToString();

    friend bool operator!=(::MouseState left, ::MouseState right);

    friend bool operator==(::MouseState left, ::MouseState right);
private:
    inline static const uint8_t LeftButtonFlag = 1;

    inline static const uint8_t RightButtonFlag = 2;

    inline static const uint8_t MiddleButtonFlag = 4;

    inline static const uint8_t XButton1Flag = 8;

    inline static const uint8_t XButton2Flag = 16;

    int32_t _x;

    int32_t _y;

    int32_t _scrollWheelValue;

    int32_t _horizontalScrollWheelValue;

    uint8_t _buttons;
};
