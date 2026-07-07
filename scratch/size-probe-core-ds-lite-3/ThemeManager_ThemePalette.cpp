#ifdef DrawText
#undef DrawText
#endif
#include "ThemeManager_ThemePalette.hpp"
#include "ThemeManager_ThemeColors.hpp"
#include "runtime/native_exceptions.hpp"
#include "ThemeManager_ThemePalette.hpp"
#include "runtime/native_exceptions.hpp"

::ThemeManager_ThemeColors* ThemeManager_ThemePalette::get_Colors()
{
return this->Colors;
}

ThemeManager_ThemePalette::ThemeManager_ThemePalette(::ThemeManager_ThemeColors* colors) : Colors()
{
this->Colors = (colors != nullptr ? colors : throw new ArgumentNullException("colors"));
}

