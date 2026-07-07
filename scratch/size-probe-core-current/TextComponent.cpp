#ifdef DrawText
#undef DrawText
#endif
#include "TextComponent.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "runtime/native_string.hpp"
#include "system/math.hpp"
#include "FontChar.hpp"
#include "float3.hpp"
#include "float2.hpp"
#include "FontAsset.hpp"
#include "InputSystem.hpp"
#include "RenderManager2D.hpp"
#include "byte4.hpp"
#include "float4.hpp"
#include "TextAlignment.hpp"
#include "ButtonState.hpp"
#include "RoundedRectComponent.hpp"
#include "ThemeManager.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "Keys.hpp"
#include "int2.hpp"
#include "RuntimeTexture.hpp"
#include "runtime/native_dictionary.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "IUpdateable.hpp"
#include "IDrawable2D.hpp"
#include "IDrawable3D.hpp"
#include "ICamera.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "IInteractable2D.hpp"
#include "PendingUpdateOperation.hpp"
#include "ICameraBoundViewportOwner.hpp"
#include "FontInfo.hpp"
#include "TextureAsset.hpp"
#include "FontTightMetrics.hpp"
#include "InputFrameState.hpp"
#include "InputBinding.hpp"
#include "InputActionState.hpp"
#include "MouseState.hpp"
#include "KeyboardState.hpp"
#include "system/action.hpp"
#include "InputContextId.hpp"
#include "InputActionId.hpp"
#include "InputGamepadState.hpp"
#include "InputPointerState.hpp"
#include "InputTextState.hpp"
#include "InputGamepadButton.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "RoundedRectCorners.hpp"
#include "runtime/native_event.hpp"
#include "ThemeManager_ThemePalette.hpp"
#include "ThemeManager_ThemeColors.hpp"
#include "TextComponent.hpp"
#include "UpdateComponent.hpp"
#include "system/math.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/diagnostics/stopwatch.hpp"

uint8_t TextComponent::get_RenderOrder2D()
{
return this->RenderOrder2DValue;}

void TextComponent::set_RenderOrder2D(uint8_t value)
{
    if (this->RenderOrder2DValue != value)
    {
this->RenderOrder2DValue = value;
this->UpdateSelectionRenderOrder();
    if (this->Parent != nullptr && this->Parent->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RemoveFromRender2D(this);
Core::Instance->ObjectManager->RegisterForRender2D(this);
    }
    }
}

::RuntimeTexture* TextComponent::get_Texture()
{
return this->Texture;
}

void TextComponent::set_Texture(::RuntimeTexture* value)
{
this->Texture = value;
}

float TextComponent::get_Rotation()
{
return this->RotationValue;}

void TextComponent::set_Rotation(float value)
{
    if (this->RotationValue == value)
    {
return;    }
this->RotationValue = value;
this->MarkTextRenderStateDirty();
}

::float4 TextComponent::get_SourceRect()
{
return this->SourceRectValue;}

void TextComponent::set_SourceRect(::float4 value)
{
    if (this->SourceRectValue.X == value.X && this->SourceRectValue.Y == value.Y && this->SourceRectValue.Z == value.Z && this->SourceRectValue.W == value.W)
    {
return;    }
this->SourceRectValue = value;
this->MarkTextRenderStateDirty();
}

::int2 TextComponent::get_Size()
{
return this->SizeValue;}

void TextComponent::set_Size(::int2 value)
{
    if (this->SizeValue.X == value.X && this->SizeValue.Y == value.Y)
    {
return;    }
this->SizeValue = value;
this->MarkTextRenderStateDirty();
}

::int2 TextComponent::get_AnchorSize()
{
return this->get_Size();
}

::byte4 TextComponent::get_Color()
{
return this->ColorValue;}

void TextComponent::set_Color(::byte4 value)
{
    if (this->ColorValue.X == value.X && this->ColorValue.Y == value.Y && this->ColorValue.Z == value.Z && this->ColorValue.W == value.W)
    {
return;    }
this->ColorValue = value;
this->MarkTextRenderStateDirty();
}

const std::string& TextComponent::get_Text()
{
return this->TextValue;}

void TextComponent::set_Text(std::string value)
{
const std::string normalizedValue = value;
    if (String::Equals(this->TextValue, normalizedValue, StringComparison::Ordinal))
    {
return;    }
this->TextValue = normalizedValue;
this->ClampSelectionToTextLength();
this->UpdateSelectionVisual();
this->MarkTextRenderStateDirty();
}

bool TextComponent::get_WrapText()
{
return this->WrapTextValue;}

void TextComponent::set_WrapText(bool value)
{
    if (this->WrapTextValue == value)
    {
return;    }
this->WrapTextValue = value;
this->MarkTextRenderStateDirty();
}

::FontAsset* TextComponent::get_Font()
{
return this->FontValue;}

void TextComponent::set_Font(::FontAsset* value)
{
    if ((this->FontValue == value))
    {
return;    }
this->FontValue = value;
this->UpdateSelectionVisual();
this->MarkTextRenderStateDirty();
}

float TextComponent::get_FontScale()
{
return this->FontScaleValue;}

void TextComponent::set_FontScale(float value)
{
    if (value <= 0.0f)
    {
throw ([&]() {
auto __ctor_arg_0000021B = "value";
auto __ctor_arg_0000021C = "Font scale must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_0000021B, __ctor_arg_0000021C);
})();
    }
    if (this->FontScaleValue != value)
    {
this->FontScaleValue = value;
this->UpdateSelectionVisual();
this->MarkTextRenderStateDirty();
    }
}

::TextAlignment TextComponent::get_Alignment()
{
return this->AlignmentValue;}

void TextComponent::set_Alignment(::TextAlignment value)
{
    if (this->AlignmentValue == value)
    {
return;    }
this->AlignmentValue = value;
this->MarkTextRenderStateDirty();
}

int32_t TextComponent::get_TextRenderStateVersion()
{
return this->TextRenderStateVersionValue;}

bool TextComponent::get_ConvertTextToSprite()
{
return this->ConvertTextToSprite;
}

void TextComponent::set_ConvertTextToSprite(bool value)
{
this->ConvertTextToSprite = value;
}

uint8_t TextComponent::get_LayerMask()
{
return this->LayerMask;
}

void TextComponent::set_LayerMask(uint8_t value)
{
this->LayerMask = value;
}

bool TextComponent::get_SelectionEnabled()
{
return this->SelectionEnabledValue;}

void TextComponent::set_SelectionEnabled(bool value)
{
    if (this->SelectionEnabledValue == value)
    {
return;    }
this->SelectionEnabledValue = value;
    if (!this->SelectionEnabledValue)
    {
this->IsFocusedValue = false;
this->IsSelectingTextValue = false;
this->ClearSelection();
    if (this->SelectionEntityValue != nullptr)
    {
this->SelectionEntityValue->set_Enabled(false);
    }
this->UpdateSelectionVisual();
return;    }
    if (this->Parent != nullptr)
    {
this->EnsureSelectionInfrastructure(this->Parent);
    if (this->SelectionEntityValue != nullptr)
    {
this->SelectionEntityValue->set_Enabled(this->Parent->get_IsHierarchyEnabled());
    }
    if (this->Parent->get_IsHierarchyEnabled() && this->RenderOrder2DValue == 0)
    {
Core::Instance->ObjectManager->RemoveFromRender2D(this);
Core::Instance->ObjectManager->RegisterForRender2D(this);
    }
this->UpdateSelectionVisual();
    }
}

bool TextComponent::get_HasSelection()
{
return this->SelectionAnchorPositionValue != this->CursorPositionValue;}

int32_t TextComponent::get_SelectionStart()
{
return Math::Min(static_cast<int32_t>(this->SelectionAnchorPositionValue), static_cast<int32_t>(this->CursorPositionValue));}

int32_t TextComponent::get_SelectionEnd()
{
return Math::Max(static_cast<int32_t>(this->SelectionAnchorPositionValue), static_cast<int32_t>(this->CursorPositionValue));}

void TextComponent::ClearSelection()
{
this->SelectionAnchorPositionValue = this->CursorPositionValue;
this->UpdateSelectionVisual();
}

void TextComponent::ComponentAdded(::Entity* entity)
{
    if (this->get_SelectionEnabled())
    {
this->EnsureSelectionInfrastructure(entity);
    }
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::Instance->ObjectManager->RegisterForRender2D(this);
    }
    if (this->get_SelectionEnabled())
    {
    if (this->SelectionEntityValue != nullptr)
    {
this->SelectionEntityValue->set_Enabled(entity->get_IsHierarchyEnabled());
    }
this->UpdateSelectionVisual();
    }
}

void TextComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
Core::Instance->ObjectManager->RemoveFromRender2D(this);
this->IsFocusedValue = false;
this->IsSelectingTextValue = false;
    if (this->SelectionUpdateComponentValue != nullptr && entity != nullptr)
    {
entity->RemoveComponent(this->SelectionUpdateComponentValue);
this->SelectionUpdateComponentValue = nullptr;
    }
    if (this->SelectionEntityValue != nullptr)
    {
this->SelectionEntityValue->set_Enabled(false);
    }
}

void TextComponent::Draw()
{
    if (this->get_Font() == nullptr)
    {
return;    }
Core::Instance->RenderManager2D->DrawText(this);
}

void TextComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
Core::Instance->ObjectManager->RegisterForRender2D(this);
    }
else {
Core::Instance->ObjectManager->RemoveFromRender2D(this);
this->IsFocusedValue = false;
this->IsSelectingTextValue = false;
}
    if (this->SelectionEntityValue != nullptr)
    {
this->SelectionEntityValue->set_Enabled(newEnabled && this->get_SelectionEnabled());
    }
this->UpdateSelectionVisual();
}

void TextComponent::SelectAll()
{
this->SelectionAnchorPositionValue = 0;
this->CursorPositionValue = static_cast<int32_t>(this->TextValue.size());
this->UpdateSelectionVisual();
}

TextComponent::TextComponent() : Texture(), ConvertTextToSprite(), LayerMask(), TextValue(), SelectionEnabledValue(), IsFocusedValue(), IsSelectingTextValue(), SelectionAnchorPositionValue(0), CursorPositionValue(0), SelectionEntityValue(), SelectionSpriteValue(), SelectionUpdateComponentValue(), FontScaleValue(), RotationValue(), SourceRectValue(), SizeValue(), ColorValue(), WrapTextValue(), FontValue(), AlignmentValue(), TextRenderStateVersionValue(0), RenderOrder2DValue()
{
this->TextValue = "";
this->SelectionAnchorPositionValue = 0;
this->CursorPositionValue = 0;
this->TextRenderStateVersionValue = 1;
this->set_Color(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
this->set_SourceRect(::float4(0.0f, 0.0f, 1.0f, 1.0f));
this->set_WrapText(false);
this->FontScaleValue = 1.0f;
this->set_Alignment(TextAlignment::Left);
this->set_ConvertTextToSprite(false);
}

void TextComponent::UpdateSelectionInput()
{
    if (!this->get_SelectionEnabled() || this->Parent == nullptr || this->get_Font() == nullptr || !this->Parent->get_IsHierarchyEnabled())
    {
return;    }
::InputSystem *input = Core::Instance->Input;
    if (input == nullptr)
    {
return;    }
const int32_t pointerX = input->GetMouseX();
const int32_t pointerY = input->GetMouseY();
    if (input->WasMouseLeftButtonPressed())
    {
this->HandleSelectionPress(static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY));
    }
else {
    if (input->GetMouseLeftButtonState() == ButtonState::Pressed && this->IsSelectingTextValue)
    {
this->HandleSelectionDrag(static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY));
    }
else {
    if (input->WasMouseLeftButtonReleased())
    {
this->HandleSelectionRelease();
    }
else {
this->HandleSelectionKeyboardInput();
}
}
}
}

::Entity* TextComponent::get_Parent()
{
return Component::get_Parent();
}

void TextComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool TextComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* TextComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool TextComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void TextComponent::ClampSelectionToTextLength()
{
const int32_t textLength = static_cast<int32_t>(this->TextValue.size());
    if (this->CursorPositionValue < 0)
    {
this->CursorPositionValue = 0;
    }
else {
    if (this->CursorPositionValue > textLength)
    {
this->CursorPositionValue = textLength;
    }
}
    if (this->SelectionAnchorPositionValue < 0)
    {
this->SelectionAnchorPositionValue = 0;
    }
else {
    if (this->SelectionAnchorPositionValue > textLength)
    {
this->SelectionAnchorPositionValue = textLength;
    }
}
}

bool TextComponent::ContainsScreenPoint(int32_t x, int32_t y)
{
    if (this->Parent == nullptr || this->get_Font() == nullptr)
    {
return false;    }
::float3 position = this->Parent->get_Position();
::float2 textSize = this->get_Font()->MeasureString(this->TextValue);
const double fontScale = this->GetResolvedFontScale();
const double textWidth = textSize.X * fontScale;
const double textHeight = textSize.Y * fontScale;
return x >= position.X && x < position.X + textWidth && y >= position.Y && y < position.Y + textHeight;}

void TextComponent::EnsureSelectionInfrastructure(::Entity* entity)
{
    if (this->SelectionEntityValue == nullptr)
    {
    if (entity->get_Children() == nullptr)
    {
entity->InitChildren();
    }
this->SelectionEntityValue = new ::Entity();
this->SelectionEntityValue->set_LayerMask(entity->get_LayerMask());
this->SelectionEntityValue->set_Enabled(false);
this->SelectionEntityValue->InitComponents();
entity->AddChild(this->SelectionEntityValue);
this->SelectionSpriteValue = new ::RoundedRectComponent();
this->SelectionSpriteValue->set_Radius(2.0f);
this->SelectionSpriteValue->set_BorderThickness(0.0f);
this->SelectionSpriteValue->set_FillColor(::byte4(static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.X), static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.Y), static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.Z), static_cast<uint8_t>(96)));
this->SelectionSpriteValue->set_BorderColor(this->SelectionSpriteValue->FillColor);
this->SelectionSpriteValue->set_RenderOrder2D(this->ResolveSelectionRenderOrder());
this->SelectionEntityValue->AddComponent(this->SelectionSpriteValue);
    }
    if (this->SelectionUpdateComponentValue == nullptr)
    {
this->SelectionUpdateComponentValue = new ::TextComponentSelectionUpdateComponent(this);
this->SelectionUpdateComponentValue->set_UpdateOrder(Core::Instance->ObjectManager->GetUpdateOrderForLayer(static_cast<int32_t>(1)));
entity->AddComponent(this->SelectionUpdateComponentValue);
    }
}

double TextComponent::GetResolvedFontScale()
{
return Math::Max(static_cast<double>(this->FontScaleValue), 0.0001);}

void TextComponent::HandleSelectionDrag(int32_t pointerX, int32_t pointerY)
{
    if (!this->get_SelectionEnabled() || !this->IsSelectingTextValue || this->get_Font() == nullptr)
    {
return;    }
this->CursorPositionValue = this->ResolveCursorPositionFromClick(static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY));
this->ClampSelectionToTextLength();
this->UpdateSelectionVisual();
}

void TextComponent::HandleSelectionKeyboardInput()
{
    if (!this->get_SelectionEnabled() || !this->IsFocusedValue)
    {
return;    }
::InputSystem *input = Core::Instance->Input;
const bool isShiftPressed = input->IsKeyDown(static_cast<Keys>(Keys::LeftShift)) || input->IsKeyDown(static_cast<Keys>(Keys::RightShift));
const bool isControlPressed = input->IsKeyDown(static_cast<Keys>(Keys::LeftControl)) || input->IsKeyDown(static_cast<Keys>(Keys::RightControl));
for (int32_t i = 0; i < 255; i++) {
::Keys key = static_cast<Keys>(i);
    if (!input->WasKeyPressed(static_cast<Keys>(key)))
    {
continue;
    }
    if (isControlPressed && key == Keys::A)
    {
this->SelectAll();
    }
else {
    if (key == Keys::Left)
    {
this->MoveCursorLeft(isShiftPressed);
    }
else {
    if (key == Keys::Right)
    {
this->MoveCursorRight(isShiftPressed);
    }
else {
    if (key == Keys::Home)
    {
this->MoveCursorToStart(isShiftPressed);
    }
else {
    if (key == Keys::End)
    {
this->MoveCursorToEnd(isShiftPressed);
    }
}
}
}
}
}
}

void TextComponent::HandleSelectionPress(int32_t pointerX, int32_t pointerY)
{
    if (!this->get_SelectionEnabled() || !this->ContainsScreenPoint(static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY)))
    {
this->SetFocusedState(false);
return;    }
this->SetFocusedState(true);
const int32_t cursor = this->ResolveCursorPositionFromClick(static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY));
this->CursorPositionValue = cursor;
this->SelectionAnchorPositionValue = cursor;
this->IsSelectingTextValue = true;
this->UpdateSelectionVisual();
}

void TextComponent::HandleSelectionRelease()
{
this->IsSelectingTextValue = false;
}

void TextComponent::MarkTextRenderStateDirty()
{
    if (this->TextRenderStateVersionValue == 2147483647)
    {
this->TextRenderStateVersionValue = 1;
return;    }
this->TextRenderStateVersionValue++;
}

void TextComponent::MoveCursorLeft(bool extendSelection)
{
    if (extendSelection)
    {
    if (!this->get_HasSelection())
    {
this->SelectionAnchorPositionValue = this->CursorPositionValue;
    }
this->CursorPositionValue = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(this->CursorPositionValue - 1));
    }
else {
    if (this->get_HasSelection())
    {
this->CursorPositionValue = this->get_SelectionStart();
this->SelectionAnchorPositionValue = this->CursorPositionValue;
    }
else {
this->CursorPositionValue = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(this->CursorPositionValue - 1));
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}
}
this->UpdateSelectionVisual();
}

void TextComponent::MoveCursorRight(bool extendSelection)
{
    if (extendSelection)
    {
    if (!this->get_HasSelection())
    {
this->SelectionAnchorPositionValue = this->CursorPositionValue;
    }
this->CursorPositionValue = Math::Min(static_cast<int32_t>(static_cast<int32_t>(this->TextValue.size())), static_cast<int32_t>(this->CursorPositionValue + 1));
    }
else {
    if (this->get_HasSelection())
    {
this->CursorPositionValue = this->get_SelectionEnd();
this->SelectionAnchorPositionValue = this->CursorPositionValue;
    }
else {
this->CursorPositionValue = Math::Min(static_cast<int32_t>(static_cast<int32_t>(this->TextValue.size())), static_cast<int32_t>(this->CursorPositionValue + 1));
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}
}
this->UpdateSelectionVisual();
}

void TextComponent::MoveCursorToEnd(bool extendSelection)
{
    if (extendSelection)
    {
    if (!this->get_HasSelection())
    {
this->SelectionAnchorPositionValue = this->CursorPositionValue;
    }
this->CursorPositionValue = static_cast<int32_t>(this->TextValue.size());
    }
else {
this->CursorPositionValue = static_cast<int32_t>(this->TextValue.size());
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}
this->UpdateSelectionVisual();
}

void TextComponent::MoveCursorToStart(bool extendSelection)
{
    if (extendSelection)
    {
    if (!this->get_HasSelection())
    {
this->SelectionAnchorPositionValue = this->CursorPositionValue;
    }
this->CursorPositionValue = 0;
    }
else {
this->CursorPositionValue = 0;
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}
this->UpdateSelectionVisual();
}

double TextComponent::ResolveCharacterAdvance(char character)
{
const double fontScale = this->GetResolvedFontScale();
    if (character == '\r' || character == '\n')
    {
return 0.0;    }
    if (character == ' ')
    {
return Math::Max(static_cast<double>(this->get_Font()->FontInfo->SpaceWidth), 1.0) * fontScale;    }
::FontChar glyph;
    if (this->get_Font()->Characters != nullptr && this->get_Font()->Characters->TryGetValue(static_cast<char>(character), glyph))
    {
    if (glyph.AdvanceWidth > 0.0f)
    {
return glyph.AdvanceWidth * fontScale;    }
const double sourceWidth = static_cast<double>(glyph.SourceRect.Z);
    if (sourceWidth > 0.0)
    {
return sourceWidth * fontScale;    }
    }
return 1.0 * fontScale;}

int32_t TextComponent::ResolveCursorPositionFromClick(int32_t pointerX, int32_t pointerY)
{
    if (this->get_Font() == nullptr || this->Parent == nullptr)
    {
return 0;    }
    if (String::IsNullOrEmpty(this->TextValue))
    {
return 0;    }
::float3 textPosition = this->Parent->get_Position();
const double textX = Math::Max(0.0, static_cast<double>(pointerX) - textPosition.X);
const double textY = Math::Max(0.0, static_cast<double>(pointerY) - textPosition.Y);
const int32_t lineIndex = this->ResolveLineIndexFromLocalY(textY);
const int32_t lineStartIndex = this->ResolveLineStartIndex(static_cast<int32_t>(lineIndex));
const int32_t lineEndIndex = this->ResolveLineEndIndex(static_cast<int32_t>(lineIndex));
double cursorX = 0.0;
for (int32_t index = lineStartIndex; index < lineEndIndex; index++) {
const double advance = this->ResolveCharacterAdvance(static_cast<char>(this->TextValue[index]));
    if (textX < cursorX + (advance * 0.5))
    {
return index;    }
cursorX += advance;
}
return lineEndIndex;}

int32_t TextComponent::ResolveLineCount()
{
    if (String::IsNullOrEmpty(this->TextValue))
    {
return 1;    }
int32_t lineCount = 1;
for (int32_t index = 0; index < static_cast<int32_t>(this->TextValue.size()); index++) {
    if (this->TextValue[index] == '\n')
    {
lineCount++;
    }
}
return lineCount;}

int32_t TextComponent::ResolveLineEndIndex(int32_t lineIndex)
{
const int32_t lineStartIndex = this->ResolveLineStartIndex(static_cast<int32_t>(lineIndex));
for (int32_t index = lineStartIndex; index < static_cast<int32_t>(this->TextValue.size()); index++) {
    if (this->TextValue[index] == '\n')
    {
return index;    }
}
return static_cast<int32_t>(this->TextValue.size());}

int32_t TextComponent::ResolveLineIndexFromLocalY(double localY)
{
const double lineHeight = Math::Max(static_cast<double>(this->get_Font()->LineHeight) * this->GetResolvedFontScale(), 1.0);
const int32_t lineIndex = static_cast<int32_t>((localY / lineHeight));
const int32_t maxLineIndex = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(this->ResolveLineCount() - 1));
return Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(Math::Min(static_cast<int32_t>(lineIndex), static_cast<int32_t>(maxLineIndex))));}

int32_t TextComponent::ResolveLineIndexFromTextIndex(int32_t textIndex)
{
const int32_t clampedIndex = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(Math::Min(static_cast<int32_t>(textIndex), static_cast<int32_t>(static_cast<int32_t>(this->TextValue.size())))));
int32_t lineIndex = 0;
for (int32_t index = 0; index < clampedIndex && index < static_cast<int32_t>(this->TextValue.size()); index++) {
    if (this->TextValue[index] == '\n')
    {
lineIndex++;
    }
}
return lineIndex;}

int32_t TextComponent::ResolveLineStartIndex(int32_t lineIndex)
{
    if (lineIndex <= 0)
    {
return 0;    }
int32_t currentLineIndex = 0;
for (int32_t index = 0; index < static_cast<int32_t>(this->TextValue.size()); index++) {
    if (this->TextValue[index] != '\n')
    {
continue;
    }
currentLineIndex++;
    if (currentLineIndex == lineIndex)
    {
return index + 1;    }
}
return static_cast<int32_t>(this->TextValue.size());}

uint8_t TextComponent::ResolveSelectionRenderOrder()
{
    if (this->RenderOrder2DValue == 0)
    {
return 0;    }
return static_cast<uint8_t>((this->RenderOrder2DValue - 1));}

double TextComponent::ResolveTextWidth(int32_t startIndex, int32_t endIndex)
{
    if (this->get_Font() == nullptr)
    {
return 0.0;    }
const int32_t clampedStart = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(Math::Min(static_cast<int32_t>(startIndex), static_cast<int32_t>(static_cast<int32_t>(this->TextValue.size())))));
const int32_t clampedEnd = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(Math::Min(static_cast<int32_t>(endIndex), static_cast<int32_t>(static_cast<int32_t>(this->TextValue.size())))));
double width = 0.0;
for (int32_t index = clampedStart; index < clampedEnd; index++) {
width += this->ResolveCharacterAdvance(static_cast<char>(this->TextValue[index]));
}
return width;}

double TextComponent::ResolveTextWidthInLine(int32_t lineIndex, int32_t startIndex, int32_t endIndex)
{
const int32_t lineStartIndex = this->ResolveLineStartIndex(static_cast<int32_t>(lineIndex));
const int32_t lineEndIndex = this->ResolveLineEndIndex(static_cast<int32_t>(lineIndex));
const int32_t clampedStartIndex = Math::Max(static_cast<int32_t>(lineStartIndex), static_cast<int32_t>(Math::Min(static_cast<int32_t>(startIndex), static_cast<int32_t>(lineEndIndex))));
const int32_t clampedEndIndex = Math::Max(static_cast<int32_t>(clampedStartIndex), static_cast<int32_t>(Math::Min(static_cast<int32_t>(endIndex), static_cast<int32_t>(lineEndIndex))));
return this->ResolveTextWidth(static_cast<int32_t>(clampedStartIndex), static_cast<int32_t>(clampedEndIndex));}

void TextComponent::SetFocusedState(bool newFocused)
{
    if (this->IsFocusedValue == newFocused)
    {
return;    }
this->IsFocusedValue = newFocused;
this->IsSelectingTextValue = false;
    if (this->IsFocusedValue)
    {
this->CursorPositionValue = static_cast<int32_t>(this->TextValue.size());
this->SelectionAnchorPositionValue = this->CursorPositionValue;
    }
}

void TextComponent::UpdateSelectionRenderOrder()
{
    if (this->SelectionSpriteValue == nullptr)
    {
return;    }
this->SelectionSpriteValue->set_RenderOrder2D(this->ResolveSelectionRenderOrder());
}

void TextComponent::UpdateSelectionVisual()
{
    if (this->SelectionEntityValue == nullptr || this->SelectionSpriteValue == nullptr)
    {
return;    }
    if (!this->get_SelectionEnabled() || this->get_Font() == nullptr || !this->get_HasSelection() || String::IsNullOrEmpty(this->TextValue))
    {
this->SelectionEntityValue->set_LocalPosition(::float3(0.0f, 0.0f, 0.05f));
this->SelectionSpriteValue->set_Size(::int2(static_cast<int32_t>(0), static_cast<int32_t>(0)));
this->SelectionSpriteValue->set_FillColor(::byte4(static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.X), static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.Y), static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.Z), static_cast<uint8_t>(0)));
this->SelectionSpriteValue->set_BorderColor(this->SelectionSpriteValue->FillColor);
return;    }
const int32_t selectionLineIndex = this->ResolveLineIndexFromTextIndex(static_cast<int32_t>(this->get_SelectionStart()));
const int32_t selectionLineEndIndex = this->ResolveLineEndIndex(static_cast<int32_t>(selectionLineIndex));
const double selectionStartX = this->ResolveTextWidthInLine(static_cast<int32_t>(selectionLineIndex), static_cast<int32_t>(this->ResolveLineStartIndex(static_cast<int32_t>(selectionLineIndex))), static_cast<int32_t>(this->get_SelectionStart()));
const double selectionWidth = this->ResolveTextWidthInLine(static_cast<int32_t>(selectionLineIndex), static_cast<int32_t>(this->get_SelectionStart()), static_cast<int32_t>(Math::Min(static_cast<int32_t>(this->get_SelectionEnd()), static_cast<int32_t>(selectionLineEndIndex))));
const double lineHeight = Math::Max(static_cast<double>(this->get_Font()->LineHeight) * this->GetResolvedFontScale(), 1.0);
this->SelectionEntityValue->set_LocalPosition(::float3(static_cast<float>(selectionStartX), static_cast<float>((selectionLineIndex * lineHeight)), 0.05f));
this->SelectionSpriteValue->set_Size(([&]() {
auto __ctor_arg_0000021D = static_cast<int32_t>(static_cast<int32_t>(Math::Ceiling(selectionWidth)));
auto __ctor_arg_0000021E = static_cast<int32_t>(static_cast<int32_t>(Math::Ceiling(lineHeight)));
return ::int2(__ctor_arg_0000021D, __ctor_arg_0000021E);
})());
this->SelectionSpriteValue->set_FillColor(::byte4(static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.X), static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.Y), static_cast<uint8_t>(ThemeManager::get_Colors()->AccentPrimary.Z), static_cast<uint8_t>(96)));
this->SelectionSpriteValue->set_BorderColor(this->SelectionSpriteValue->FillColor);
}

