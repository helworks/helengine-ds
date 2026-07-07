#ifdef DrawText
#undef DrawText
#endif
#include "ThemeManager.hpp"
#include "ThemeManager_ThemeColors.hpp"
#include "ThemeManager_ThemePalette.hpp"
#include "ThemeManager.hpp"
#include "byte4.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"

::Event ThemeManager::ThemeChanged;

::ThemeManager_ThemePalette* ThemeManager::Current = ThemeManager::CreateNeon90s();

::ThemeManager_ThemePalette* ThemeManager::get_Current()
{
return ThemeManager::Current;
}

void ThemeManager::set_Current(::ThemeManager_ThemePalette* value)
{
ThemeManager::Current = value;
}

::ThemeManager_ThemeColors* ThemeManager::get_Colors()
{
return Current->Colors;
}

::ThemeManager_ThemePalette* ThemeManager::CreateDarkTheme()
{
::ThemeManager_ThemeColors *colors = ([&]() {
auto __object_00000186 = new ::ThemeManager_ThemeColors();
__object_00000186->set_BackgroundPrimary(::byte4(static_cast<uint8_t>(20), static_cast<uint8_t>(20), static_cast<uint8_t>(20), static_cast<uint8_t>(255)));
__object_00000186->set_SurfacePrimary(::byte4(static_cast<uint8_t>(35), static_cast<uint8_t>(35), static_cast<uint8_t>(35), static_cast<uint8_t>(255)));
__object_00000186->set_SurfaceInput(::byte4(static_cast<uint8_t>(10), static_cast<uint8_t>(10), static_cast<uint8_t>(10), static_cast<uint8_t>(255)));
__object_00000186->set_AccentPrimary(::byte4(static_cast<uint8_t>(0), static_cast<uint8_t>(123), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
__object_00000186->set_AccentSecondary(::byte4(static_cast<uint8_t>(108), static_cast<uint8_t>(117), static_cast<uint8_t>(125), static_cast<uint8_t>(255)));
__object_00000186->set_AccentTertiary(::byte4(static_cast<uint8_t>(52), static_cast<uint8_t>(58), static_cast<uint8_t>(64), static_cast<uint8_t>(255)));
__object_00000186->set_AccentQuaternary(::byte4(static_cast<uint8_t>(173), static_cast<uint8_t>(181), static_cast<uint8_t>(189), static_cast<uint8_t>(255)));
__object_00000186->set_StateWarning(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(193), static_cast<uint8_t>(7), static_cast<uint8_t>(255)));
__object_00000186->set_StateDanger(::byte4(static_cast<uint8_t>(220), static_cast<uint8_t>(53), static_cast<uint8_t>(69), static_cast<uint8_t>(255)));
__object_00000186->set_StateSuccess(::byte4(static_cast<uint8_t>(40), static_cast<uint8_t>(167), static_cast<uint8_t>(69), static_cast<uint8_t>(255)));
__object_00000186->set_InputForegroundPrimary(::byte4(static_cast<uint8_t>(0), static_cast<uint8_t>(255), static_cast<uint8_t>(0), static_cast<uint8_t>(255)));
__object_00000186->set_InputForegroundSecondary(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(0), static_cast<uint8_t>(255)));
__object_00000186->set_TextPrimary(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
__object_00000186->set_TextSecondary(::byte4(static_cast<uint8_t>(173), static_cast<uint8_t>(181), static_cast<uint8_t>(189), static_cast<uint8_t>(255)));
__object_00000186->set_TextOnAccent(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
return __object_00000186;
})();
return new ::ThemeManager_ThemePalette(colors);}

::ThemeManager_ThemePalette* ThemeManager::CreateLightTheme()
{
::ThemeManager_ThemeColors *colors = ([&]() {
auto __object_00000187 = new ::ThemeManager_ThemeColors();
__object_00000187->set_BackgroundPrimary(::byte4(static_cast<uint8_t>(248), static_cast<uint8_t>(249), static_cast<uint8_t>(250), static_cast<uint8_t>(255)));
__object_00000187->set_SurfacePrimary(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
__object_00000187->set_SurfaceInput(::byte4(static_cast<uint8_t>(233), static_cast<uint8_t>(236), static_cast<uint8_t>(239), static_cast<uint8_t>(255)));
__object_00000187->set_AccentPrimary(::byte4(static_cast<uint8_t>(0), static_cast<uint8_t>(123), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
__object_00000187->set_AccentSecondary(::byte4(static_cast<uint8_t>(108), static_cast<uint8_t>(117), static_cast<uint8_t>(125), static_cast<uint8_t>(255)));
__object_00000187->set_AccentTertiary(::byte4(static_cast<uint8_t>(52), static_cast<uint8_t>(58), static_cast<uint8_t>(64), static_cast<uint8_t>(255)));
__object_00000187->set_AccentQuaternary(::byte4(static_cast<uint8_t>(73), static_cast<uint8_t>(80), static_cast<uint8_t>(87), static_cast<uint8_t>(255)));
__object_00000187->set_StateWarning(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(193), static_cast<uint8_t>(7), static_cast<uint8_t>(255)));
__object_00000187->set_StateDanger(::byte4(static_cast<uint8_t>(220), static_cast<uint8_t>(53), static_cast<uint8_t>(69), static_cast<uint8_t>(255)));
__object_00000187->set_StateSuccess(::byte4(static_cast<uint8_t>(40), static_cast<uint8_t>(167), static_cast<uint8_t>(69), static_cast<uint8_t>(255)));
__object_00000187->set_InputForegroundPrimary(::byte4(static_cast<uint8_t>(33), static_cast<uint8_t>(37), static_cast<uint8_t>(41), static_cast<uint8_t>(255)));
__object_00000187->set_InputForegroundSecondary(::byte4(static_cast<uint8_t>(52), static_cast<uint8_t>(58), static_cast<uint8_t>(64), static_cast<uint8_t>(255)));
__object_00000187->set_TextPrimary(::byte4(static_cast<uint8_t>(33), static_cast<uint8_t>(37), static_cast<uint8_t>(41), static_cast<uint8_t>(255)));
__object_00000187->set_TextSecondary(::byte4(static_cast<uint8_t>(73), static_cast<uint8_t>(80), static_cast<uint8_t>(87), static_cast<uint8_t>(255)));
__object_00000187->set_TextOnAccent(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
return __object_00000187;
})();
return new ::ThemeManager_ThemePalette(colors);}

::ThemeManager_ThemePalette* ThemeManager::CreateNeon90s()
{
::ThemeManager_ThemeColors *colors = ([&]() {
auto __object_00000188 = new ::ThemeManager_ThemeColors();
__object_00000188->set_BackgroundPrimary(::byte4(static_cast<uint8_t>(25), static_cast<uint8_t>(15), static_cast<uint8_t>(35), static_cast<uint8_t>(255)));
__object_00000188->set_SurfacePrimary(::byte4(static_cast<uint8_t>(40), static_cast<uint8_t>(25), static_cast<uint8_t>(50), static_cast<uint8_t>(255)));
__object_00000188->set_SurfaceInput(::byte4(static_cast<uint8_t>(15), static_cast<uint8_t>(15), static_cast<uint8_t>(15), static_cast<uint8_t>(255)));
__object_00000188->set_AccentPrimary(::byte4(static_cast<uint8_t>(194), static_cast<uint8_t>(49), static_cast<uint8_t>(175), static_cast<uint8_t>(255)));
__object_00000188->set_AccentSecondary(::byte4(static_cast<uint8_t>(141), static_cast<uint8_t>(49), static_cast<uint8_t>(194), static_cast<uint8_t>(255)));
__object_00000188->set_AccentTertiary(::byte4(static_cast<uint8_t>(68), static_cast<uint8_t>(49), static_cast<uint8_t>(194), static_cast<uint8_t>(255)));
__object_00000188->set_AccentQuaternary(::byte4(static_cast<uint8_t>(204), static_cast<uint8_t>(204), static_cast<uint8_t>(204), static_cast<uint8_t>(255)));
__object_00000188->set_StateWarning(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(178), static_cast<uint8_t>(102), static_cast<uint8_t>(255)));
__object_00000188->set_StateDanger(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(80), static_cast<uint8_t>(80), static_cast<uint8_t>(255)));
__object_00000188->set_StateSuccess(::byte4(static_cast<uint8_t>(102), static_cast<uint8_t>(255), static_cast<uint8_t>(153), static_cast<uint8_t>(255)));
__object_00000188->set_InputForegroundPrimary(::byte4(static_cast<uint8_t>(0), static_cast<uint8_t>(255), static_cast<uint8_t>(0), static_cast<uint8_t>(255)));
__object_00000188->set_InputForegroundSecondary(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(0), static_cast<uint8_t>(255)));
__object_00000188->set_TextPrimary(::byte4(static_cast<uint8_t>(25), static_cast<uint8_t>(15), static_cast<uint8_t>(35), static_cast<uint8_t>(255)));
__object_00000188->set_TextSecondary(::byte4(static_cast<uint8_t>(40), static_cast<uint8_t>(25), static_cast<uint8_t>(50), static_cast<uint8_t>(255)));
__object_00000188->set_TextOnAccent(::byte4(static_cast<uint8_t>(25), static_cast<uint8_t>(15), static_cast<uint8_t>(35), static_cast<uint8_t>(255)));
return __object_00000188;
})();
return new ::ThemeManager_ThemePalette(colors);}

void ThemeManager::SetTheme(::ThemeManager_ThemePalette* palette)
{
ThemeManager::set_Current((palette != nullptr ? palette : throw new ArgumentNullException("palette")));
ThemeChanged.Invoke(nullptr, nullptr);
}

