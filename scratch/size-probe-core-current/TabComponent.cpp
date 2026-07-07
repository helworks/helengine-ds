#ifdef DrawText
#undef DrawText
#endif
#include "TabComponent.hpp"
#include "ThemeManager.hpp"
#include "runtime/native_string.hpp"
#include "int2.hpp"
#include "FontAsset.hpp"
#include "system/action.hpp"
#include "runtime/native_event.hpp"
#include "ThemeManager_ThemePalette.hpp"
#include "ThemeManager_ThemeColors.hpp"
#include "ButtonComponent.hpp"
#include "byte4.hpp"
#include "runtime/native_event.hpp"

bool TabComponent::get_IsSelected()
{
return this->IsKeyboardFocused;
}

void TabComponent::SetSelected(bool isSelected)
{
this->SetTargetFocused(isSelected);
}

TabComponent::TabComponent(std::string text, ::int2 size, ::FontAsset* font, Action<>* onClickAction, float borderThickness) : ::ButtonComponent(text, size, font, onClickAction, borderThickness)
{
this->UseTopCorners();
this->SetCornerRadius(static_cast<float>((size.Y * 0.3)));
this->SetTextColor(ThemeManager::get_Colors()->AccentQuaternary);
this->SetVisualPalette(ThemeManager::get_Colors()->SurfacePrimary, ThemeManager::get_Colors()->AccentSecondary, ThemeManager::get_Colors()->AccentTertiary, ThemeManager::get_Colors()->SurfaceInput, ThemeManager::get_Colors()->AccentTertiary, ThemeManager::get_Colors()->AccentTertiary);
}

::RoundedRectCorners TabComponent::get_Corners()
{
return ButtonComponent::get_Corners();
}

void TabComponent::set_Corners(::RoundedRectCorners value)
{
ButtonComponent::set_Corners(value);
}

::IFocusGroup* TabComponent::get_FocusGroup()
{
return ButtonComponent::get_FocusGroup();
}

void TabComponent::set_FocusGroup(::IFocusGroup* value)
{
ButtonComponent::set_FocusGroup(value);
}

int32_t TabComponent::get_TabIndex()
{
return ButtonComponent::get_TabIndex();
}

void TabComponent::set_TabIndex(int32_t value)
{
ButtonComponent::set_TabIndex(value);
}

bool TabComponent::get_IsDefaultTarget()
{
return ButtonComponent::get_IsDefaultTarget();
}

void TabComponent::set_IsDefaultTarget(bool value)
{
ButtonComponent::set_IsDefaultTarget(value);
}

bool TabComponent::get_CanReceiveFocus()
{
return ButtonComponent::get_CanReceiveFocus();
}

bool TabComponent::get_IsKeyboardFocused()
{
return ButtonComponent::get_IsKeyboardFocused();
}

void TabComponent::set_IsKeyboardFocused(bool value)
{
ButtonComponent::set_IsKeyboardFocused(value);
}

::int2 TabComponent::get_Size()
{
return ButtonComponent::get_Size();
}

::FontAsset* TabComponent::get_Font()
{
return ButtonComponent::get_Font();
}

void TabComponent::set_Font(::FontAsset* value)
{
ButtonComponent::set_Font(value);
}

::int2 TabComponent::get_AnchorSize()
{
return ButtonComponent::get_AnchorSize();
}

::Entity* TabComponent::get_Parent()
{
return Component::get_Parent();
}

void TabComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool TabComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* TabComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool TabComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

