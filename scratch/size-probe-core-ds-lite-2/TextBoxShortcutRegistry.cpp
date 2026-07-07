#ifdef DrawText
#undef DrawText
#endif
#include "TextBoxShortcutRegistry.hpp"
#include "TextBoxShortcutBinding.hpp"
#include "Keys.hpp"
#include "TextBoxShortcutRegistry.hpp"

::TextBoxShortcutBinding* TextBoxShortcutRegistry::get_SelectAllShortcut()
{
return this->SelectAllShortcut;
}

void TextBoxShortcutRegistry::set_SelectAllShortcut(::TextBoxShortcutBinding* value)
{
this->SelectAllShortcut = value;
}

::TextBoxShortcutBinding* TextBoxShortcutRegistry::get_CopyShortcut()
{
return this->CopyShortcut;
}

void TextBoxShortcutRegistry::set_CopyShortcut(::TextBoxShortcutBinding* value)
{
this->CopyShortcut = value;
}

::TextBoxShortcutBinding* TextBoxShortcutRegistry::get_PasteShortcut()
{
return this->PasteShortcut;
}

void TextBoxShortcutRegistry::set_PasteShortcut(::TextBoxShortcutBinding* value)
{
this->PasteShortcut = value;
}

TextBoxShortcutRegistry::TextBoxShortcutRegistry() : SelectAllShortcut(), CopyShortcut(), PasteShortcut()
{
this->set_SelectAllShortcut(new ::TextBoxShortcutBinding(static_cast<Keys>(Keys::A), true, false, false));
this->set_CopyShortcut(new ::TextBoxShortcutBinding(static_cast<Keys>(Keys::C), true, false, false));
this->set_PasteShortcut(new ::TextBoxShortcutBinding(static_cast<Keys>(Keys::V), true, false, false));
}

