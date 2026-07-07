#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class ITextDrawable2D;
class IAnchorSizeProvider;
class IDrawable2D;
class RuntimeTexture;
class float4;
class int2;
class byte4;
class FontAsset;
class Entity;
class RoundedRectComponent;
class TextComponentSelectionUpdateComponent;

#include "Component.hpp"
#include "ITextDrawable2D.hpp"
#include "IAnchorSizeProvider.hpp"
#include "runtime/native_disposable.hpp"
#include "IDrawable2D.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "byte4.hpp"
#include "runtime/native_string.hpp"
#include "TextAlignment.hpp"

class TextComponent : public ::Component, public ::ITextDrawable2D, public ::IAnchorSizeProvider
{
public:
    virtual ~TextComponent() = default;

    uint8_t get_RenderOrder2D();

    void set_RenderOrder2D(uint8_t value);

    ::RuntimeTexture* Texture;

    ::RuntimeTexture* get_Texture();
    void set_Texture(::RuntimeTexture* value);

    float get_Rotation();

    void set_Rotation(float value);

    ::float4 get_SourceRect();

    void set_SourceRect(::float4 value);

    ::int2 get_Size();

    void set_Size(::int2 value);

    ::int2 get_AnchorSize();

    ::byte4 get_Color();

    void set_Color(::byte4 value);

    const std::string& get_Text();

    void set_Text(std::string value);

    bool get_WrapText();

    void set_WrapText(bool value);

    ::FontAsset* get_Font();

    void set_Font(::FontAsset* value);

    float get_FontScale();

    void set_FontScale(float value);

    ::TextAlignment get_Alignment();

    void set_Alignment(::TextAlignment value);

    int32_t get_TextRenderStateVersion();

    bool ConvertTextToSprite;

    bool get_ConvertTextToSprite();
    void set_ConvertTextToSprite(bool value);

    uint8_t LayerMask;

    uint8_t get_LayerMask();
    void set_LayerMask(uint8_t value);

    bool get_SelectionEnabled();

    void set_SelectionEnabled(bool value);

    bool get_HasSelection();

    int32_t get_SelectionStart();

    int32_t get_SelectionEnd();

    void ClearSelection();

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    virtual void Draw();

    void ParentEnabledChange(bool newEnabled);

    void SelectAll();

    TextComponent();

    void UpdateSelectionInput();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    std::string TextValue;

    bool SelectionEnabledValue;

    bool IsFocusedValue;

    bool IsSelectingTextValue;

    int32_t SelectionAnchorPositionValue;

    int32_t CursorPositionValue;

    ::Entity* SelectionEntityValue;

    ::RoundedRectComponent* SelectionSpriteValue;

    ::TextComponentSelectionUpdateComponent* SelectionUpdateComponentValue;

    float FontScaleValue;

    float RotationValue;

    ::float4 SourceRectValue;

    ::int2 SizeValue;

    ::byte4 ColorValue;

    bool WrapTextValue;

    ::FontAsset* FontValue;

    ::TextAlignment AlignmentValue;

    int32_t TextRenderStateVersionValue;

    uint8_t RenderOrder2DValue;

    void ClampSelectionToTextLength();

    bool ContainsScreenPoint(int32_t x, int32_t y);

    void EnsureSelectionInfrastructure(::Entity* entity);

    double GetResolvedFontScale();

    void HandleSelectionDrag(int32_t pointerX, int32_t pointerY);

    void HandleSelectionKeyboardInput();

    void HandleSelectionPress(int32_t pointerX, int32_t pointerY);

    void HandleSelectionRelease();

    void MarkTextRenderStateDirty();

    void MoveCursorLeft(bool extendSelection);

    void MoveCursorRight(bool extendSelection);

    void MoveCursorToEnd(bool extendSelection);

    void MoveCursorToStart(bool extendSelection);

    double ResolveCharacterAdvance(char character);

    int32_t ResolveCursorPositionFromClick(int32_t pointerX, int32_t pointerY);

    int32_t ResolveLineCount();

    int32_t ResolveLineEndIndex(int32_t lineIndex);

    int32_t ResolveLineIndexFromLocalY(double localY);

    int32_t ResolveLineIndexFromTextIndex(int32_t textIndex);

    int32_t ResolveLineStartIndex(int32_t lineIndex);

    uint8_t ResolveSelectionRenderOrder();

    double ResolveTextWidth(int32_t startIndex, int32_t endIndex);

    double ResolveTextWidthInLine(int32_t lineIndex, int32_t startIndex, int32_t endIndex);

    void SetFocusedState(bool newFocused);

    void UpdateSelectionRenderOrder();

    void UpdateSelectionVisual();
};
