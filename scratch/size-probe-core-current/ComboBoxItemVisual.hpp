#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class RoundedRectComponent;
class TextComponent;
class InteractableComponent;
class FontAsset;
class int2;

#include "runtime/native_event.hpp"
#include "int2.hpp"
#include "PointerInteraction.hpp"

class ComboBoxItemVisual
{
public:
    virtual ~ComboBoxItemVisual() = default;

    ::Entity* Root;

    ::Entity* get_Root();

    ::RoundedRectComponent* Background;

    ::RoundedRectComponent* get_Background();

    ::Entity* LabelHost;

    ::Entity* get_LabelHost();

    ::TextComponent* Label;

    ::TextComponent* get_Label();

    ::InteractableComponent* Interactable;

    ::InteractableComponent* get_Interactable();

    ::Event CursorEvent;

    int32_t Index;

    int32_t get_Index();
    void set_Index(int32_t value);

    bool IsHovering;

    bool get_IsHovering();
    void set_IsHovering(bool value);

    bool IsPressed;

    bool get_IsPressed();
    void set_IsPressed(bool value);

    ComboBoxItemVisual(::FontAsset* font, uint16_t layerMask, uint8_t backgroundOrder, uint8_t textOrder);
private:
    void HandleCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state);
};
