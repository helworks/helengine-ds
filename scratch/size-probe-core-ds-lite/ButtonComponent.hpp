#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IFocusTarget;
class IAnchorSizeProvider;
class byte4;
class IFocusGroup;
class int2;
class FontAsset;
class Entity;
class RoundedRectComponent;
class TextComponent;
class InteractableComponent;

#include "Component.hpp"
#include "IFocusTarget.hpp"
#include "IAnchorSizeProvider.hpp"
#include "runtime/native_disposable.hpp"
#include "byte4.hpp"
#include "RoundedRectCorners.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"
#include "system/action.hpp"
#include "PointerCursorKind.hpp"
#include "Keys.hpp"
#include "PointerInteraction.hpp"

class ButtonComponent : public ::Component, public ::IFocusTarget, public ::IAnchorSizeProvider
{
public:
    virtual ~ButtonComponent() = default;

    ::RoundedRectCorners Corners;

    ::RoundedRectCorners get_Corners();
    void set_Corners(::RoundedRectCorners value);

    ::IFocusGroup* FocusGroup;

    ::IFocusGroup* get_FocusGroup();
    void set_FocusGroup(::IFocusGroup* value);

    int32_t TabIndex;

    int32_t get_TabIndex();
    void set_TabIndex(int32_t value);

    bool IsDefaultTarget;

    bool get_IsDefaultTarget();
    void set_IsDefaultTarget(bool value);

    bool get_CanReceiveFocus();

    bool IsKeyboardFocused;

    bool get_IsKeyboardFocused();
    void set_IsKeyboardFocused(bool value);

    ::int2 get_Size();

    ::FontAsset* get_Font();

    void set_Font(::FontAsset* value);

    ::int2 get_AnchorSize();

    ::Event Hovered;

    void ActivateFromKey(::Keys key);

    ButtonComponent(std::string text, ::int2 size, ::FontAsset* font, Action<>* onClickAction, float borderThickness);

    bool CanActivateWithKey(::Keys key);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    bool ContainsScreenPoint(int32_t x, int32_t y);

    void ParentEnabledChange(bool newEnabled);

    void SetCornerRadius(float cornerRadius);

    void SetHoverCursor(::PointerCursorKind cursor);

    void SetRenderOrders(uint8_t backgroundOrder, uint8_t textOrder);

    void SetSize(::int2 newSize);

    void SetTargetFocused(bool isFocused);

    void SetText(std::string newText);

    void SetTextColor(::byte4 color);

    void SetVisualPalette(::byte4 idleFillColor, ::byte4 hoverFillColor, ::byte4 pressedFillColor, ::byte4 focusedFillColor, ::byte4 idleBorderColor, ::byte4 focusedBorderColor);

    void UseHoverOnlyBackground();

    void UseSquareCorners();

    void UseTopCorners();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    static ::byte4 TransparentBackgroundColor;

    std::string text;

    ::FontAsset* font;

    ::int2 size;

    Action<>* onClickAction;

    float borderThickness;

    bool HasRenderOrderOverrides;

    uint8_t BackgroundRenderOrder;

    uint8_t TextRenderOrder;

    bool UsesHoverOnlyBackground;

    ::byte4 ButtonTextColor;

    float CornerRadius;

    ::PointerCursorKind HoverCursorKind;

    ::byte4 IdleFillColor;

    ::byte4 HoverFillColor;

    ::byte4 PressedFillColor;

    ::byte4 FocusedFillColor;

    ::byte4 IdleBorderColor;

    ::byte4 FocusedBorderColor;

    ::Entity* textEntity;

    ::RoundedRectComponent* roundedRect;

    ::TextComponent* textComponent;

    ::InteractableComponent* interactableComponent;

    bool isHovering;

    bool isPressed;

    void ApplyTextLayout();

    ::byte4 GetIdleBorderColor();

    ::byte4 GetIdleFillColor();

    void OnCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state);

    void RaiseHovered();

    void UpdateButtonColor();

    void UpdateCornerRadius();
};
