#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IFocusTarget;
class IFocusGroup;
class FontAsset;
class int2;
class TextBoxEditState;
class float3;
class RoundedRectComponent;
class Entity;
class TextComponent;
class InteractableComponent;

#include "Component.hpp"
#include "IFocusTarget.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"
#include "int2.hpp"
#include "runtime/native_datetime.hpp"
#include "float3.hpp"
#include "Keys.hpp"
#include "PointerInteraction.hpp"

class TextBoxComponent : public ::Component, public ::IFocusTarget
{
public:
    virtual ~TextBoxComponent() = default;

    ::Event Submitted;

    ::Event FocusChanged;

    ::Event TextChanged;

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

    const std::string& get_Text();

    void set_Text(std::string value);

    const std::string& get_Placeholder();

    void set_Placeholder(std::string value);

    ::FontAsset* get_Font();

    void set_Font(::FontAsset* value);

    ::int2 get_Size();

    void set_Size(::int2 value);

    bool get_IsFocused();

    void set_IsFocused(bool value);

    float get_CurrentShakeOffsetX();

    void ActivateFromKey(::Keys key);

    bool CanActivateWithKey(::Keys key);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    bool ContainsScreenPoint(int32_t x, int32_t y);

    void ParentEnabledChange(bool newEnabled);

    void SetInvalidState(bool isInvalid);

    void SetRenderOrders(uint8_t backgroundOrder, uint8_t textOrder);

    void SetTargetFocused(bool isFocused);

    TextBoxComponent(::int2 size, ::FontAsset* font, std::string placeholder);

    void TriggerInvalidShake();

    void Update();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    inline static const int32_t TextPaddingX = 8;

    static const float EffectFrameDeltaSeconds;

    inline static const float ShakeDurationSeconds = 0.3f;

    inline static const float ShakeAmplitudePixels = 10.0f;

    inline static const float ShakeFrequencyHz = 16.0f;

    static ::TextBoxComponent* focusedTextBox;

    ::TextBoxEditState* EditState;

    std::string placeholder;

    ::FontAsset* font;

    ::int2 size;

    bool isFocused;

    bool cursorVisible;

    DateTime lastCursorBlink;

    bool hasRenderOrderOverrides;

    uint8_t backgroundRenderOrder;

    uint8_t textRenderOrder;

    bool isInvalid;

    bool isSelectingText;

    bool isShakeActive;

    float shakeElapsedSeconds;

    float currentShakeOffsetX;

    ::float3 shakeBaseLocalPosition;

    ::RoundedRectComponent* backgroundSprite;

    ::Entity* selectionEntity;

    ::RoundedRectComponent* selectionSprite;

    ::Entity* textEntity;

    ::TextComponent* textComponent;

    ::InteractableComponent* interactableComponent;

    void HandleKeyPress(::Keys key, bool isShiftPressed, bool isControlPressed, bool isAltPressed);

    char KeyToChar(::Keys key, bool isShiftPressed);

    void OnCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state);

    double ResolveCharacterAdvance(char character);

    int32_t ResolveCursorPositionFromClick(int32_t clickX);

    double ResolveTextWidth(int32_t startIndex, int32_t endIndex);

    void SetFocusedState(bool value, bool submitOnBlur);

    bool TryHandleShortcut__out4_out5(::Keys key, bool isShiftPressed, bool isControlPressed, bool isAltPressed, bool& textChanged, bool& layoutChanged);

    void UpdateFocusVisual();

    void UpdateSelectionVisual();

    void UpdateSelectionVisual(double textY, double lineHeight);

    void UpdateShakeAnimation();

    void UpdateTextDisplay();

    void UpdateTextLayout();
};
