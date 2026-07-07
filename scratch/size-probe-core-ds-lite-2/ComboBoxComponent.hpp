#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IFocusTarget;
class int2;
class FontAsset;
class IFocusGroup;
class ComboBoxItemVisual;
class RoundedRectComponent;
class TextComponent;
class InteractableComponent;
class Entity;
class ICamera;

#include "Component.hpp"
#include "IFocusTarget.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_event.hpp"
#include "int2.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "Keys.hpp"
#include "PointerInteraction.hpp"

class ComboBoxComponent : public ::Component, public ::IFocusTarget
{
public:
    virtual ~ComboBoxComponent() = default;

    ::Event SelectionChanged;

    ::int2 get_Size();

    void set_Size(::int2 value);

    ::FontAsset* get_Font();

    void set_Font(::FontAsset* value);

    bool get_IsOpen();

    void set_IsOpen(bool value);

    List<std::string>* get_Items();

    bool get_HasSelection();

    int32_t get_SelectedIndex();

    void set_SelectedIndex(int32_t value);

    const std::string& get_SelectedItem();

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

    void ActivateFromKey(::Keys key);

    bool CanActivateWithKey(::Keys key);

    ComboBoxComponent(::int2 size, ::FontAsset* font, List<std::string>* items, int32_t selectedIndex);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    bool ContainsScreenPoint(int32_t x, int32_t y);

    void ParentEnabledChange(bool newEnabled);

    void SetItems(List<std::string>* items, int32_t selectedIndex);

    void SetRenderOrders(uint8_t backgroundOrder, uint8_t textOrder, uint8_t listBackgroundOrder, uint8_t listTextOrder);

    void SetTargetFocused(bool isFocused);

    void Update();

    void UseModalPresentation();

    void UsePanelPresentation();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    inline static const int32_t TextPaddingX = 8;

    inline static const int32_t ArrowPaddingX = 8;

    inline static const int32_t ListGap = 2;

    inline static const std::string ArrowGlyph = "v";

    List<std::string>* items;

    List<::ComboBoxItemVisual*>* itemVisuals;

    bool hasRenderOrderOverrides;

    ::FontAsset* font;

    ::int2 size;

    int32_t itemHeight;

    int32_t selectedIndex;

    bool isOpen;

    bool isHovering;

    bool isPressed;

    ::RoundedRectComponent* background;

    ::TextComponent* labelText;

    ::TextComponent* arrowText;

    ::InteractableComponent* interactable;

    ::Entity* labelEntity;

    ::Entity* arrowEntity;

    ::Entity* listRoot;

    ::RoundedRectComponent* listBackground;

    uint8_t backgroundOrder;

    uint8_t textOrder;

    uint8_t listBackgroundOrder;

    uint8_t listTextOrder;

    void ApplyRenderOrders();

    void CopyItems(List<std::string>* source);

    ::ComboBoxItemVisual* CreateItemVisual();

    void EnsureItemVisuals(int32_t count);

    ::ICamera* FindTopmostCameraAt(int32_t x, int32_t y, uint16_t layerMask);

    float GetCornerRadius(::int2 size);

    void HandleItemCursorEvent(::ComboBoxItemVisual* entry, ::int2 relPos, ::int2 delta, ::PointerInteraction state);

    void HandleMainCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state);

    void HideItemVisuals();

    bool IsPointerInsideCombo(int32_t mouseX, int32_t mouseY);

    void ResetItemStates();

    void SetSelectedIndexInternal(int32_t index, bool raiseEvent);

    void UpdateAllItemStates();

    void UpdateDropdownVisibility();

    void UpdateItemVisualState(::ComboBoxItemVisual* entry, bool isSelected);

    void UpdateLabelLayout();

    void UpdateLabelText();

    void UpdateLayout();

    void UpdateListLayout();

    void UpdateMainLayout();

    void UpdateMainVisual();

    void ValidateItems(List<std::string>* items);

    int32_t ValidateSelectedIndex(int32_t itemCount, int32_t index);
};
