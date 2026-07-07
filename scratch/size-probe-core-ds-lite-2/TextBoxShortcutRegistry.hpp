#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class TextBoxShortcutBinding;

class TextBoxShortcutRegistry
{
public:
    virtual ~TextBoxShortcutRegistry() = default;

    ::TextBoxShortcutBinding* SelectAllShortcut;

    ::TextBoxShortcutBinding* get_SelectAllShortcut();
    void set_SelectAllShortcut(::TextBoxShortcutBinding* value);

    ::TextBoxShortcutBinding* CopyShortcut;

    ::TextBoxShortcutBinding* get_CopyShortcut();
    void set_CopyShortcut(::TextBoxShortcutBinding* value);

    ::TextBoxShortcutBinding* PasteShortcut;

    ::TextBoxShortcutBinding* get_PasteShortcut();
    void set_PasteShortcut(::TextBoxShortcutBinding* value);

    TextBoxShortcutRegistry();
};
