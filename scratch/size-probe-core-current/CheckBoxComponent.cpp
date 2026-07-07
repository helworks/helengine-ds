#ifdef DrawText
#undef DrawText
#endif
#include "CheckBoxComponent.hpp"
#include "Component.hpp"
#include "RenderOrder2D.hpp"
#include "Entity.hpp"
#include "RoundedRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "PointerCursorKind.hpp"
#include "TextComponent.hpp"
#include "ThemeManager.hpp"
#include "PointerInteraction.hpp"
#include "float3.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "FontAsset.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "RoundedRectCorners.hpp"
#include "byte4.hpp"
#include "RuntimeTexture.hpp"
#include "TextAlignment.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "ThemeManager_ThemePalette.hpp"
#include "ThemeManager_ThemeColors.hpp"
#include "float2.hpp"
#include "runtime/native_exceptions.hpp"
#include "CheckBoxComponent.hpp"
#include "system/math.hpp"
#include "system/action.hpp"
#include "system/math.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::int2 CheckBoxComponent::get_Size()
{
return this->SizeValue;}

void CheckBoxComponent::set_Size(::int2 value)
{
this->SizeValue = value;
    if (this->Background != nullptr)
    {
this->Background->set_Size(value);
    }
    if (this->Interactable != nullptr)
    {
this->Interactable->set_Size(value);
    }
this->UpdateCheckMarkLayout();
}

bool CheckBoxComponent::get_IsChecked()
{
return this->IsCheckedValue;}

void CheckBoxComponent::set_IsChecked(bool value)
{
this->SetCheckedState(value, false);
}

CheckBoxComponent::CheckBoxComponent(::int2 size, ::FontAsset* font, bool isChecked) : CheckedChanged(), SizeValue(), Font(), IsCheckedValue(), IsHovering(), IsPressed(), HasRenderOrderOverrides(), BackgroundRenderOrder(), CheckMarkRenderOrder(), Background(), Interactable(), CheckMarkEntity(), CheckMark()
{
    if (font == nullptr)
    {
throw new ArgumentNullException("font");
    }
this->SizeValue = size;
this->Font = font;
this->IsCheckedValue = isChecked;
}

void CheckBoxComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
uint8_t backgroundOrder = RenderOrder2D::PanelSurface;
uint8_t textOrder = RenderOrder2D::PanelForeground;
    if (this->HasRenderOrderOverrides)
    {
backgroundOrder = this->BackgroundRenderOrder;
textOrder = this->CheckMarkRenderOrder;
    }
this->Background = new ::RoundedRectComponent();
this->Background->set_Size(this->SizeValue);
this->Background->set_Radius(static_cast<float>((Math::Min(static_cast<int32_t>(this->SizeValue.X), static_cast<int32_t>(this->SizeValue.Y)) * 0.15)));
this->Background->set_BorderThickness(2.0f);
this->Background->set_RenderOrder2D(backgroundOrder);
entity->AddComponent(this->Background);
this->Interactable = new ::InteractableComponent();
this->Interactable->set_Size(this->SizeValue);
this->Interactable->set_HoverCursor(PointerCursorKind::Hand);
this->Interactable->CursorEvent += Event::Bind(this, static_cast<void (CheckBoxComponent::*)(int2, int2, PointerInteraction)>(&CheckBoxComponent::HandleCursorEvent));
entity->AddComponent(this->Interactable);
this->CheckMarkEntity = new ::Entity();
this->CheckMarkEntity->set_LayerMask(entity->get_LayerMask());
this->CheckMarkEntity->set_Enabled(true);
this->CheckMarkEntity->InitComponents();
    if (entity->get_Children() == nullptr)
    {
entity->InitChildren();
    }
entity->AddChild(this->CheckMarkEntity);
this->CheckMark = new ::TextComponent();
this->CheckMark->set_Font(this->Font);
this->CheckMark->set_Color(ThemeManager::get_Colors()->InputForegroundPrimary);
this->CheckMark->set_RenderOrder2D(textOrder);
this->CheckMarkEntity->AddComponent(this->CheckMark);
this->UpdateCheckMarkLayout();
this->UpdateVisualState();
}

void CheckBoxComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
this->IsHovering = false;
this->IsPressed = false;
}

void CheckBoxComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (!newEnabled)
    {
this->IsHovering = false;
this->IsPressed = false;
this->UpdateVisualState();
    }
    if (this->CheckMarkEntity != nullptr)
    {
this->CheckMarkEntity->set_Enabled(newEnabled);
    }
}

void CheckBoxComponent::SetRenderOrders(uint8_t backgroundOrder, uint8_t checkMarkOrder)
{
this->HasRenderOrderOverrides = true;
this->BackgroundRenderOrder = backgroundOrder;
this->CheckMarkRenderOrder = checkMarkOrder;
    if (this->Background != nullptr)
    {
this->Background->set_RenderOrder2D(backgroundOrder);
    }
    if (this->CheckMark != nullptr)
    {
this->CheckMark->set_RenderOrder2D(checkMarkOrder);
    }
}

::Entity* CheckBoxComponent::get_Parent()
{
return Component::get_Parent();
}

void CheckBoxComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool CheckBoxComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* CheckBoxComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool CheckBoxComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void CheckBoxComponent::HandleCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state)
{
switch (state) {
case PointerInteraction::Hover: {
    if (!this->IsHovering)
    {
this->IsHovering = true;
this->UpdateVisualState();
    }
break;
}
case PointerInteraction::Press: {
this->IsPressed = true;
this->UpdateVisualState();
break;
}
case PointerInteraction::Release: {
    if (this->IsPressed && this->IsHovering)
    {
this->SetCheckedState(!this->IsCheckedValue, true);
    }
this->IsPressed = false;
this->UpdateVisualState();
break;
}
case PointerInteraction::Leave: {
    if (this->IsHovering || this->IsPressed)
    {
this->IsHovering = false;
this->IsPressed = false;
this->UpdateVisualState();
    }
break;
}
case PointerInteraction::None: {
break;
}
}

}

void CheckBoxComponent::SetCheckedState(bool isChecked, bool raiseEvent)
{
    if (this->IsCheckedValue == isChecked)
    {
this->UpdateVisualState();
return;    }
this->IsCheckedValue = isChecked;
this->UpdateVisualState();
    if (raiseEvent && true)
    {
this->CheckedChanged.Invoke(this, IsCheckedValue);
    }
}

void CheckBoxComponent::UpdateCheckMarkLayout()
{
    if (this->CheckMarkEntity == nullptr || this->CheckMark == nullptr)
    {
return;    }
this->CheckMarkEntity->set_Position(::float3(0.0f, 0.0f, 0.1f));
this->CheckMark->set_Size(([&]() {
auto __ctor_arg_0000019F = static_cast<int32_t>(1);
auto __ctor_arg_000001A0 = static_cast<int32_t>(Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Ceiling(Math::Max(static_cast<double>(this->Font->LineHeight), 1.0))))));
return ::int2(__ctor_arg_0000019F, __ctor_arg_000001A0);
})());
}

void CheckBoxComponent::UpdateVisualState()
{
    if (this->Background == nullptr)
    {
return;    }
this->Background->set_BorderColor(this->IsCheckedValue || this->IsHovering ? ThemeManager::get_Colors()->AccentPrimary : ThemeManager::get_Colors()->AccentTertiary);
    if (this->IsPressed)
    {
this->Background->set_FillColor(ThemeManager::get_Colors()->AccentTertiary);
    }
else {
    if (this->IsCheckedValue)
    {
this->Background->set_FillColor(ThemeManager::get_Colors()->AccentSecondary);
    }
else {
    if (this->IsHovering)
    {
this->Background->set_FillColor(ThemeManager::get_Colors()->SurfaceInput);
    }
else {
this->Background->set_FillColor(ThemeManager::get_Colors()->SurfaceInput);
}
}
}
    if (this->CheckMark != nullptr)
    {
this->CheckMark->set_Text(String::Empty);
    }
}

