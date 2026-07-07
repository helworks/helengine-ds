#ifdef DrawText
#undef DrawText
#endif
#include "ButtonComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "RenderOrder2D.hpp"
#include "Entity.hpp"
#include "float3.hpp"
#include "byte4.hpp"
#include "FontTightMetrics.hpp"
#include "FontAsset.hpp"
#include "system/math.hpp"
#include "ThemeManager.hpp"
#include "RoundedRectCorners.hpp"
#include "Keys.hpp"
#include "RoundedRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "TextComponent.hpp"
#include "int2.hpp"
#include "PointerInteraction.hpp"
#include "IFocusGroup.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"
#include "system/action.hpp"
#include "PointerCursorKind.hpp"
#include "runtime/native_dictionary.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "float2.hpp"
#include "FontInfo.hpp"
#include "RuntimeTexture.hpp"
#include "FontChar.hpp"
#include "TextureAsset.hpp"
#include "ThemeManager_ThemePalette.hpp"
#include "ThemeManager_ThemeColors.hpp"
#include "TextAlignment.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "ButtonComponent.hpp"
#include "system/math.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"

::RoundedRectCorners ButtonComponent::get_Corners()
{
return this->Corners;
}

void ButtonComponent::set_Corners(::RoundedRectCorners value)
{
this->Corners = value;
}

::IFocusGroup* ButtonComponent::get_FocusGroup()
{
return this->FocusGroup;
}

void ButtonComponent::set_FocusGroup(::IFocusGroup* value)
{
this->FocusGroup = value;
}

int32_t ButtonComponent::get_TabIndex()
{
return this->TabIndex;
}

void ButtonComponent::set_TabIndex(int32_t value)
{
this->TabIndex = value;
}

bool ButtonComponent::get_IsDefaultTarget()
{
return this->IsDefaultTarget;
}

void ButtonComponent::set_IsDefaultTarget(bool value)
{
this->IsDefaultTarget = value;
}

bool ButtonComponent::get_CanReceiveFocus()
{
return this->Parent != nullptr && this->Parent->get_IsHierarchyEnabled() && this->interactableComponent != nullptr;
}

bool ButtonComponent::get_IsKeyboardFocused()
{
return this->IsKeyboardFocused;
}

void ButtonComponent::set_IsKeyboardFocused(bool value)
{
this->IsKeyboardFocused = value;
}

::int2 ButtonComponent::get_Size()
{
return this->size;
}

::FontAsset* ButtonComponent::get_Font()
{
return this->font;}

void ButtonComponent::set_Font(::FontAsset* value)
{
    if (value == nullptr)
    {
throw new ArgumentNullException("value");
    }
this->font = value;
    if (this->textComponent != nullptr)
    {
this->textComponent->set_Font(this->font);
    }
this->ApplyTextLayout();
}

::int2 ButtonComponent::get_AnchorSize()
{
return this->size;
}

void ButtonComponent::ActivateFromKey(::Keys key)
{
    if (!this->CanActivateWithKey(static_cast<Keys>(key)))
    {
return;    }
if (this->onClickAction != nullptr)
{
(*this->onClickAction)();
}
}

ButtonComponent::ButtonComponent(std::string text, ::int2 size, ::FontAsset* font, Action<>* onClickAction, float borderThickness) : Corners(), FocusGroup(), TabIndex(0), IsDefaultTarget(), IsKeyboardFocused(), Hovered(), text(), font(), size(), onClickAction(), borderThickness(), HasRenderOrderOverrides(), BackgroundRenderOrder(), TextRenderOrder(), UsesHoverOnlyBackground(), ButtonTextColor(), CornerRadius(), HoverCursorKind(), IdleFillColor(), HoverFillColor(), PressedFillColor(), FocusedFillColor(), IdleBorderColor(), FocusedBorderColor(), textEntity(), roundedRect(), textComponent(), interactableComponent(), isHovering(), isPressed()
{
this->text = text;
this->size = size;
this->font = font;
this->onClickAction = onClickAction;
this->borderThickness = borderThickness;
this->ButtonTextColor = ThemeManager::get_Colors()->TextOnAccent;
this->IdleFillColor = ThemeManager::get_Colors()->AccentSecondary;
this->HoverFillColor = ThemeManager::get_Colors()->AccentPrimary;
this->PressedFillColor = ThemeManager::get_Colors()->AccentTertiary;
this->FocusedFillColor = this->IdleFillColor;
this->IdleBorderColor = ThemeManager::get_Colors()->AccentTertiary;
this->FocusedBorderColor = ThemeManager::get_Colors()->AccentPrimary;
this->set_Corners(RoundedRectCorners::All);
this->UpdateCornerRadius();
}

bool ButtonComponent::CanActivateWithKey(::Keys key)
{
return key == Keys::Enter || key == Keys::Space;}

void ButtonComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (!entity->get_Enabled())
    {
return;    }
uint8_t backgroundOrder = RenderOrder2D::PanelSurface;
uint8_t textOrder = RenderOrder2D::PanelForeground;
    if (this->HasRenderOrderOverrides)
    {
backgroundOrder = this->BackgroundRenderOrder;
textOrder = this->TextRenderOrder;
    }
this->roundedRect = new ::RoundedRectComponent();
this->roundedRect->set_Size(this->size);
this->roundedRect->set_Corners(this->Corners);
this->roundedRect->set_Radius(this->CornerRadius);
this->roundedRect->set_BorderThickness(this->borderThickness);
this->roundedRect->set_FillColor(ThemeManager::get_Colors()->AccentSecondary);
this->roundedRect->set_BorderColor(ThemeManager::get_Colors()->AccentTertiary);
this->roundedRect->set_RenderOrder2D(backgroundOrder);
entity->AddComponent(this->roundedRect);
this->UpdateButtonColor();
this->interactableComponent = new ::InteractableComponent();
this->interactableComponent->set_Size(this->size);
this->interactableComponent->set_HoverCursor(this->HoverCursorKind);
this->interactableComponent->CursorEvent += Event::Bind(this, static_cast<void (ButtonComponent::*)(int2, int2, PointerInteraction)>(&ButtonComponent::OnCursorEvent));
entity->AddComponent(this->interactableComponent);
this->textEntity = new ::Entity();
this->textEntity->set_LayerMask(entity->get_LayerMask());
this->textEntity->set_Enabled(true);
this->textEntity->InitComponents();
entity->InitChildren();
entity->AddChild(this->textEntity);
this->textComponent = new ::TextComponent();
this->textComponent->set_Text(this->text);
this->textComponent->set_Font(this->font);
this->textComponent->set_Color(this->ButtonTextColor);
this->textComponent->set_Size(::int2(static_cast<int32_t>(1), static_cast<int32_t>(1)));
this->textComponent->set_RenderOrder2D(textOrder);
this->textEntity->AddComponent(this->textComponent);
this->ApplyTextLayout();
}

void ButtonComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
this->isHovering = false;
this->isPressed = false;
this->SetTargetFocused(false);
}

bool ButtonComponent::ContainsScreenPoint(int32_t x, int32_t y)
{
    if (this->Parent == nullptr)
    {
return false;    }
::float3 position = this->Parent->get_Position();
return x >= position.X && x < position.X + this->size.X && y >= position.Y && y < position.Y + this->size.Y;}

void ButtonComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (!newEnabled)
    {
this->isHovering = false;
this->isPressed = false;
this->SetTargetFocused(false);
    }
    if (this->textEntity != nullptr)
    {
this->textEntity->set_Enabled(newEnabled);
    }
}

void ButtonComponent::SetCornerRadius(float cornerRadius)
{
    if (cornerRadius < 0.0f)
    {
throw ([&]() {
auto __ctor_arg_0000020A = "cornerRadius";
auto __ctor_arg_0000020B = "Corner radius must not be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_0000020A, __ctor_arg_0000020B);
})();
    }
this->CornerRadius = cornerRadius;
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_Radius(this->CornerRadius);
    }
}

void ButtonComponent::SetHoverCursor(::PointerCursorKind cursor)
{
this->HoverCursorKind = cursor;
    if (this->interactableComponent != nullptr)
    {
this->interactableComponent->set_HoverCursor(cursor);
    }
}

void ButtonComponent::SetRenderOrders(uint8_t backgroundOrder, uint8_t textOrder)
{
this->HasRenderOrderOverrides = true;
this->BackgroundRenderOrder = backgroundOrder;
this->TextRenderOrder = textOrder;
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_RenderOrder2D(backgroundOrder);
    }
    if (this->textComponent != nullptr)
    {
this->textComponent->set_RenderOrder2D(textOrder);
    }
}

void ButtonComponent::SetSize(::int2 newSize)
{
    if (newSize.X < 1 || newSize.Y < 1)
    {
throw ([&]() {
auto __ctor_arg_0000020C = "newSize";
auto __ctor_arg_0000020D = "Button size must be positive.";
return new ArgumentOutOfRangeException(__ctor_arg_0000020C, __ctor_arg_0000020D);
})();
    }
this->size = newSize;
    if (this->Corners != RoundedRectCorners::None)
    {
this->UpdateCornerRadius();
    }
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_Size(this->size);
this->roundedRect->set_Corners(this->Corners);
this->roundedRect->set_Radius(this->CornerRadius);
    }
    if (this->interactableComponent != nullptr)
    {
this->interactableComponent->set_Size(this->size);
this->interactableComponent->set_HoverCursor(this->HoverCursorKind);
    }
    if (this->textEntity == nullptr || this->textComponent == nullptr)
    {
return;    }
this->ApplyTextLayout();
}

void ButtonComponent::SetTargetFocused(bool isFocused)
{
    if (this->IsKeyboardFocused == isFocused)
    {
this->UpdateButtonColor();
return;    }
this->set_IsKeyboardFocused(isFocused);
this->UpdateButtonColor();
}

void ButtonComponent::SetText(std::string newText)
{
this->text = newText;
    if (this->textComponent != nullptr)
    {
this->textComponent->set_Text(this->text);
    }
}

void ButtonComponent::SetTextColor(::byte4 color)
{
this->ButtonTextColor = color;
    if (this->textComponent != nullptr)
    {
this->textComponent->set_Color(color);
    }
}

void ButtonComponent::SetVisualPalette(::byte4 idleFillColor, ::byte4 hoverFillColor, ::byte4 pressedFillColor, ::byte4 focusedFillColor, ::byte4 idleBorderColor, ::byte4 focusedBorderColor)
{
this->IdleFillColor = idleFillColor;
this->HoverFillColor = hoverFillColor;
this->PressedFillColor = pressedFillColor;
this->FocusedFillColor = focusedFillColor;
this->IdleBorderColor = idleBorderColor;
this->FocusedBorderColor = focusedBorderColor;
this->UpdateButtonColor();
}

void ButtonComponent::UseHoverOnlyBackground()
{
this->UsesHoverOnlyBackground = true;
this->UpdateButtonColor();
}

void ButtonComponent::UseSquareCorners()
{
this->set_Corners(RoundedRectCorners::None);
this->CornerRadius = 0.0f;
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_Corners(this->Corners);
this->roundedRect->set_Radius(this->CornerRadius);
    }
}

void ButtonComponent::UseTopCorners()
{
this->set_Corners(static_cast<RoundedRectCorners>((static_cast<int32_t>(RoundedRectCorners::TopLeft) + static_cast<int32_t>(RoundedRectCorners::TopRight))));
this->UpdateCornerRadius();
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_Corners(this->Corners);
this->roundedRect->set_Radius(this->CornerRadius);
    }
}

::Entity* ButtonComponent::get_Parent()
{
return Component::get_Parent();
}

void ButtonComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool ButtonComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* ButtonComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool ButtonComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

::byte4 ButtonComponent::TransparentBackgroundColor = ::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(0));

void ButtonComponent::ApplyTextLayout()
{
    if (this->textEntity == nullptr || this->textComponent == nullptr)
    {
return;    }
::FontTightMetrics tight = this->font->MeasureTight(this->text);
const double lineHeight = Math::Max(static_cast<double>(this->font->LineHeight), 1.0);
double px = (static_cast<double>(this->size.X) - tight.Width) / 2.0;
double py = (static_cast<double>(this->size.Y) - lineHeight) / 2.0;
px = Math::Round(px);
py = Math::Round(py);
this->textEntity->set_Position(::float3(static_cast<float>(px), static_cast<float>(py), 0.1f));
this->textComponent->set_Size(([&]() {
auto __ctor_arg_0000020E = static_cast<int32_t>(static_cast<int32_t>(Math::Ceiling(tight.Width)));
auto __ctor_arg_0000020F = static_cast<int32_t>(static_cast<int32_t>(Math::Ceiling(lineHeight)));
return ::int2(__ctor_arg_0000020E, __ctor_arg_0000020F);
})());
}

::byte4 ButtonComponent::GetIdleBorderColor()
{
    if (this->UsesHoverOnlyBackground)
    {
return TransparentBackgroundColor;    }
return this->IdleBorderColor;}

::byte4 ButtonComponent::GetIdleFillColor()
{
    if (this->UsesHoverOnlyBackground)
    {
return TransparentBackgroundColor;    }
return this->IdleFillColor;}

void ButtonComponent::OnCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state)
{
switch (state) {
case PointerInteraction::Hover: {
    if (!this->isHovering)
    {
this->isHovering = true;
this->UpdateButtonColor();
this->RaiseHovered();
    }
break;
}
case PointerInteraction::Press: {
this->isPressed = true;
this->UpdateButtonColor();
break;
}
case PointerInteraction::Release: {
    if (this->isPressed && this->isHovering)
    {
if (this->onClickAction != nullptr)
{
(*this->onClickAction)();
}
    }
this->isPressed = false;
this->UpdateButtonColor();
break;
}
case PointerInteraction::Leave: {
    if (this->isHovering || this->isPressed)
    {
this->isHovering = false;
this->isPressed = false;
this->UpdateButtonColor();
    }
break;
}
case PointerInteraction::None: {
break;
}
}

}

void ButtonComponent::RaiseHovered()
{
    if (true)
    {
this->Hovered.Invoke();
    }
}

void ButtonComponent::UpdateButtonColor()
{
    if (this->roundedRect == nullptr)
    {
return;    }
this->roundedRect->set_BorderColor(this->IsKeyboardFocused ? this->FocusedBorderColor : this->GetIdleBorderColor());
    if (this->isPressed)
    {
this->roundedRect->set_FillColor(this->PressedFillColor);
    }
else {
    if (this->isHovering)
    {
this->roundedRect->set_FillColor(this->HoverFillColor);
    }
else {
    if (this->IsKeyboardFocused)
    {
this->roundedRect->set_FillColor(this->FocusedFillColor);
    }
else {
this->roundedRect->set_FillColor(this->GetIdleFillColor());
}
}
}
}

void ButtonComponent::UpdateCornerRadius()
{
this->CornerRadius = static_cast<float>((Math::Min(static_cast<double>(this->size.X), static_cast<double>(this->size.Y)) * 0.15));
}

