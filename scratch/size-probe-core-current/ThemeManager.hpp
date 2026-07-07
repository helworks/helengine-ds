#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ThemeManager_ThemePalette;
class ThemeManager_ThemeColors;

#include "runtime/native_event.hpp"

class ThemeManager
{
friend class ThemeManager_ThemeColors;
friend class ThemeManager_ThemePalette;
public:
    virtual ~ThemeManager() = default;

    static ::Event ThemeChanged;

    static ::ThemeManager_ThemePalette* Current;

    static ::ThemeManager_ThemePalette* get_Current();
    static void set_Current(::ThemeManager_ThemePalette* value);

    static ::ThemeManager_ThemeColors* get_Colors();

    static ::ThemeManager_ThemePalette* CreateDarkTheme();

    static ::ThemeManager_ThemePalette* CreateLightTheme();

    static ::ThemeManager_ThemePalette* CreateNeon90s();

    static void SetTheme(::ThemeManager_ThemePalette* palette);
};
