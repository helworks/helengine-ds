#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ThemeManager_ThemeColors;

class ThemeManager_ThemePalette
{
public:
    virtual ~ThemeManager_ThemePalette() = default;

    ::ThemeManager_ThemeColors* Colors;

    ::ThemeManager_ThemeColors* get_Colors();

    ThemeManager_ThemePalette(::ThemeManager_ThemeColors* colors);
};
