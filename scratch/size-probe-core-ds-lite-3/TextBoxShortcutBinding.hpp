#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "Keys.hpp"

class TextBoxShortcutBinding
{
public:
    virtual ~TextBoxShortcutBinding() = default;

    ::Keys Key;

    ::Keys get_Key();

    bool RequiresControl;

    bool get_RequiresControl();

    bool RequiresShift;

    bool get_RequiresShift();

    bool RequiresAlt;

    bool get_RequiresAlt();

    bool Matches(::Keys key, bool isControlPressed, bool isShiftPressed, bool isAltPressed);

    TextBoxShortcutBinding(::Keys key, bool requiresControl, bool requiresShift, bool requiresAlt);
};
