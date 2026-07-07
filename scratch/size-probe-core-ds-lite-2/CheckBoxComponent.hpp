#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class int2;
class FontAsset;
class RoundedRectComponent;
class InteractableComponent;
class Entity;
class TextComponent;

#include "Component.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/native_event.hpp"
#include "int2.hpp"
#include "PointerInteraction.hpp"

class CheckBoxComponent : public ::Component
{
public:
    virtual ~CheckBoxComponent() = default;

    ::Event CheckedChanged;

    ::int2 get_Size();

    void set_Size(::int2 value);

    bool get_IsChecked();

    void set_IsChecked(bool value);

    CheckBoxComponent(::int2 size, ::FontAsset* font, bool isChecked);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void ParentEnabledChange(bool newEnabled);

    void SetRenderOrders(uint8_t backgroundOrder, uint8_t checkMarkOrder);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    ::int2 SizeValue;

    ::FontAsset* Font;

    bool IsCheckedValue;

    bool IsHovering;

    bool IsPressed;

    bool HasRenderOrderOverrides;

    uint8_t BackgroundRenderOrder;

    uint8_t CheckMarkRenderOrder;

    ::RoundedRectComponent* Background;

    ::InteractableComponent* Interactable;

    ::Entity* CheckMarkEntity;

    ::TextComponent* CheckMark;

    void HandleCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state);

    void SetCheckedState(bool isChecked, bool raiseEvent);

    void UpdateCheckMarkLayout();

    void UpdateVisualState();
};
