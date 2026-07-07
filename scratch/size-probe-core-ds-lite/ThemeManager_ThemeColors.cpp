#ifdef DrawText
#undef DrawText
#endif
#include "ThemeManager_ThemeColors.hpp"
#include "byte4.hpp"

ThemeManager_ThemeColors::ThemeManager_ThemeColors() : BackgroundPrimary(), SurfacePrimary(), SurfaceInput(), AccentPrimary(), AccentSecondary(), AccentTertiary(), AccentQuaternary(), StateDanger(), StateWarning(), StateSuccess(), InputForegroundPrimary(), InputForegroundSecondary(), TextPrimary(), TextSecondary(), TextOnAccent()
{
}

::byte4 ThemeManager_ThemeColors::get_BackgroundPrimary()
{
return this->BackgroundPrimary;
}

void ThemeManager_ThemeColors::set_BackgroundPrimary(::byte4 value)
{
this->BackgroundPrimary = value;
}

::byte4 ThemeManager_ThemeColors::get_SurfacePrimary()
{
return this->SurfacePrimary;
}

void ThemeManager_ThemeColors::set_SurfacePrimary(::byte4 value)
{
this->SurfacePrimary = value;
}

::byte4 ThemeManager_ThemeColors::get_SurfaceInput()
{
return this->SurfaceInput;
}

void ThemeManager_ThemeColors::set_SurfaceInput(::byte4 value)
{
this->SurfaceInput = value;
}

::byte4 ThemeManager_ThemeColors::get_AccentPrimary()
{
return this->AccentPrimary;
}

void ThemeManager_ThemeColors::set_AccentPrimary(::byte4 value)
{
this->AccentPrimary = value;
}

::byte4 ThemeManager_ThemeColors::get_AccentSecondary()
{
return this->AccentSecondary;
}

void ThemeManager_ThemeColors::set_AccentSecondary(::byte4 value)
{
this->AccentSecondary = value;
}

::byte4 ThemeManager_ThemeColors::get_AccentTertiary()
{
return this->AccentTertiary;
}

void ThemeManager_ThemeColors::set_AccentTertiary(::byte4 value)
{
this->AccentTertiary = value;
}

::byte4 ThemeManager_ThemeColors::get_AccentQuaternary()
{
return this->AccentQuaternary;
}

void ThemeManager_ThemeColors::set_AccentQuaternary(::byte4 value)
{
this->AccentQuaternary = value;
}

::byte4 ThemeManager_ThemeColors::get_StateDanger()
{
return this->StateDanger;
}

void ThemeManager_ThemeColors::set_StateDanger(::byte4 value)
{
this->StateDanger = value;
}

::byte4 ThemeManager_ThemeColors::get_StateWarning()
{
return this->StateWarning;
}

void ThemeManager_ThemeColors::set_StateWarning(::byte4 value)
{
this->StateWarning = value;
}

::byte4 ThemeManager_ThemeColors::get_StateSuccess()
{
return this->StateSuccess;
}

void ThemeManager_ThemeColors::set_StateSuccess(::byte4 value)
{
this->StateSuccess = value;
}

::byte4 ThemeManager_ThemeColors::get_InputForegroundPrimary()
{
return this->InputForegroundPrimary;
}

void ThemeManager_ThemeColors::set_InputForegroundPrimary(::byte4 value)
{
this->InputForegroundPrimary = value;
}

::byte4 ThemeManager_ThemeColors::get_InputForegroundSecondary()
{
return this->InputForegroundSecondary;
}

void ThemeManager_ThemeColors::set_InputForegroundSecondary(::byte4 value)
{
this->InputForegroundSecondary = value;
}

::byte4 ThemeManager_ThemeColors::get_TextPrimary()
{
return this->TextPrimary;
}

void ThemeManager_ThemeColors::set_TextPrimary(::byte4 value)
{
this->TextPrimary = value;
}

::byte4 ThemeManager_ThemeColors::get_TextSecondary()
{
return this->TextSecondary;
}

void ThemeManager_ThemeColors::set_TextSecondary(::byte4 value)
{
this->TextSecondary = value;
}

::byte4 ThemeManager_ThemeColors::get_TextOnAccent()
{
return this->TextOnAccent;
}

void ThemeManager_ThemeColors::set_TextOnAccent(::byte4 value)
{
this->TextOnAccent = value;
}

