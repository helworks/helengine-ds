#ifdef DrawText
#undef DrawText
#endif
#include "TextBoxShortcutBinding.hpp"
#include "Keys.hpp"
#include "TextBoxShortcutBinding.hpp"

::Keys TextBoxShortcutBinding::get_Key()
{
return this->Key;
}

bool TextBoxShortcutBinding::get_RequiresControl()
{
return this->RequiresControl;
}

bool TextBoxShortcutBinding::get_RequiresShift()
{
return this->RequiresShift;
}

bool TextBoxShortcutBinding::get_RequiresAlt()
{
return this->RequiresAlt;
}

bool TextBoxShortcutBinding::Matches(::Keys key, bool isControlPressed, bool isShiftPressed, bool isAltPressed)
{
    if (this->Key != key)
    {
return false;    }
    if (this->RequiresControl != isControlPressed)
    {
return false;    }
    if (this->RequiresShift != isShiftPressed)
    {
return false;    }
return this->RequiresAlt == isAltPressed;}

TextBoxShortcutBinding::TextBoxShortcutBinding(::Keys key, bool requiresControl, bool requiresShift, bool requiresAlt) : Key(), RequiresControl(), RequiresShift(), RequiresAlt()
{
this->Key = key;
this->RequiresControl = requiresControl;
this->RequiresShift = requiresShift;
this->RequiresAlt = requiresAlt;
}

