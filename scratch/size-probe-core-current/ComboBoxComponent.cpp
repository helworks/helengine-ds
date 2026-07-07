#ifdef DrawText
#undef DrawText
#endif
#include "ComboBoxComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "ComboBoxComponent.hpp"
#include "runtime/native_list.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "ComboBoxUpdateComponent.hpp"
#include "runtime/native_string.hpp"
#include "InputSystem.hpp"
#include "Core.hpp"
#include "float3.hpp"
#include "system/math.hpp"
#include "FontTightMetrics.hpp"
#include "FontAsset.hpp"
#include "ComboBoxItemVisual.hpp"
#include "ICamera.hpp"
#include "float4.hpp"
#include "GeometryUtils.hpp"
#include "ObjectManager.hpp"
#include "Keys.hpp"
#include "RoundedRectComponent.hpp"
#include "ThemeManager.hpp"
#include "InteractableComponent.hpp"
#include "TextComponent.hpp"
#include "RenderOrder2D.hpp"
#include "PointerInteraction.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "IFocusGroup.hpp"
#include "runtime/native_dictionary.hpp"
#include "float4x4.hpp"
#include "IInputBackend.hpp"
#include "InputFrameState.hpp"
#include "InputBinding.hpp"
#include "InputActionState.hpp"
#include "MouseState.hpp"
#include "KeyboardState.hpp"
#include "system/action.hpp"
#include "InputContextId.hpp"
#include "InputActionId.hpp"
#include "InputGamepadState.hpp"
#include "ButtonState.hpp"
#include "InputPointerState.hpp"
#include "InputTextState.hpp"
#include "InputGamepadButton.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "RenderManager2D.hpp"
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
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "float2.hpp"
#include "FontInfo.hpp"
#include "RuntimeTexture.hpp"
#include "FontChar.hpp"
#include "TextureAsset.hpp"
#include "RenderTarget.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#include "IUpdateable.hpp"
#include "IDrawable2D.hpp"
#include "IDrawable3D.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "IInteractable2D.hpp"
#include "PendingUpdateOperation.hpp"
#include "ICameraBoundViewportOwner.hpp"
#include "RoundedRectCorners.hpp"
#include "byte4.hpp"
#include "ThemeManager_ThemePalette.hpp"
#include "ThemeManager_ThemeColors.hpp"
#include "PointerCursorKind.hpp"
#include "TextAlignment.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "UpdateComponent.hpp"
#include "system/math.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "system/diagnostics/stopwatch.hpp"

::int2 ComboBoxComponent::get_Size()
{
return this->size;}

void ComboBoxComponent::set_Size(::int2 value)
{
    if (value.X <= 0 || value.Y <= 0)
    {
throw ([&]() {
auto __ctor_arg_000001EA = "value";
auto __ctor_arg_000001EB = "ComboBox size must be positive.";
return new ArgumentOutOfRangeException(__ctor_arg_000001EA, __ctor_arg_000001EB);
})();
    }
this->size = value;
this->itemHeight = this->size.Y;
this->UpdateLayout();
}

::FontAsset* ComboBoxComponent::get_Font()
{
return this->font;}

void ComboBoxComponent::set_Font(::FontAsset* value)
{
    if (value == nullptr)
    {
throw new ArgumentNullException("value");
    }
this->font = value;
this->UpdateLabelText();
this->UpdateLayout();
}

bool ComboBoxComponent::get_IsOpen()
{
return this->isOpen;}

void ComboBoxComponent::set_IsOpen(bool value)
{
    if (value && this->items->get_Count() == 0)
    {
this->isOpen = false;
this->UpdateDropdownVisibility();
return;    }
    if (this->isOpen == value)
    {
return;    }
this->isOpen = value;
this->UpdateDropdownVisibility();
}

List<std::string>* ComboBoxComponent::get_Items()
{
return this->items;
}

bool ComboBoxComponent::get_HasSelection()
{
return this->selectedIndex >= 0 && this->selectedIndex < this->items->get_Count();
}

int32_t ComboBoxComponent::get_SelectedIndex()
{
return this->selectedIndex;}

void ComboBoxComponent::set_SelectedIndex(int32_t value)
{
this->SetSelectedIndexInternal(static_cast<int32_t>(value), true);
}

const std::string& ComboBoxComponent::get_SelectedItem()
{
    if (!this->get_HasSelection())
    {
throw new InvalidOperationException("ComboBox has no selected item.");
    }
return (*this->items).get_Item(static_cast<int32_t>(this->selectedIndex));}

::IFocusGroup* ComboBoxComponent::get_FocusGroup()
{
return this->FocusGroup;
}

void ComboBoxComponent::set_FocusGroup(::IFocusGroup* value)
{
this->FocusGroup = value;
}

int32_t ComboBoxComponent::get_TabIndex()
{
return this->TabIndex;
}

void ComboBoxComponent::set_TabIndex(int32_t value)
{
this->TabIndex = value;
}

bool ComboBoxComponent::get_IsDefaultTarget()
{
return this->IsDefaultTarget;
}

void ComboBoxComponent::set_IsDefaultTarget(bool value)
{
this->IsDefaultTarget = value;
}

bool ComboBoxComponent::get_CanReceiveFocus()
{
return this->Parent != nullptr && this->Parent->get_IsHierarchyEnabled() && this->interactable != nullptr;
}

bool ComboBoxComponent::get_IsKeyboardFocused()
{
return this->IsKeyboardFocused;
}

void ComboBoxComponent::set_IsKeyboardFocused(bool value)
{
this->IsKeyboardFocused = value;
}

void ComboBoxComponent::ActivateFromKey(::Keys key)
{
    if (!this->CanActivateWithKey(static_cast<Keys>(key)) || this->items->get_Count() == 0)
    {
return;    }
this->set_IsOpen(!this->isOpen);
}

bool ComboBoxComponent::CanActivateWithKey(::Keys key)
{
return key == Keys::Enter || key == Keys::Space;}

ComboBoxComponent::ComboBoxComponent(::int2 size, ::FontAsset* font, List<std::string>* items, int32_t selectedIndex) : SelectionChanged(), FocusGroup(), TabIndex(0), IsDefaultTarget(), IsKeyboardFocused(), items(), itemVisuals(), hasRenderOrderOverrides(), font(), size(), itemHeight(0), selectedIndex(0), isOpen(), isHovering(), isPressed(), background(), labelText(), arrowText(), interactable(), labelEntity(), arrowEntity(), listRoot(), listBackground(), backgroundOrder(), textOrder(), listBackgroundOrder(), listTextOrder()
{
    if (size.X <= 0 || size.Y <= 0)
    {
throw ([&]() {
auto __ctor_arg_000001EC = "size";
auto __ctor_arg_000001ED = "ComboBox size must be positive.";
return new ArgumentOutOfRangeException(__ctor_arg_000001EC, __ctor_arg_000001ED);
})();
    }
else {
    if (font == nullptr)
    {
throw new ArgumentNullException("font");
    }
else {
    if (items == nullptr)
    {
throw new ArgumentNullException("items");
    }
}
}
this->size = size;
this->font = font;
this->items = new List<std::string>(static_cast<int32_t>(items->get_Count()));
this->itemVisuals = new List<::ComboBoxItemVisual*>(static_cast<int32_t>(items->get_Count()));
this->itemHeight = size.Y;
this->CopyItems(items);
this->selectedIndex = this->ValidateSelectedIndex(static_cast<int32_t>(this->items->get_Count()), static_cast<int32_t>(selectedIndex));
}

void ComboBoxComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (!this->hasRenderOrderOverrides)
    {
this->UsePanelPresentation();
    }
this->background = new ::RoundedRectComponent();
this->background->set_Size(this->size);
this->background->set_Radius(this->GetCornerRadius(this->size));
this->background->set_BorderThickness(2.0f);
this->background->set_FillColor(ThemeManager::get_Colors()->SurfaceInput);
this->background->set_BorderColor(ThemeManager::get_Colors()->AccentTertiary);
this->background->set_RenderOrder2D(this->backgroundOrder);
entity->AddComponent(this->background);
this->interactable = new ::InteractableComponent();
this->interactable->set_Size(this->size);
this->interactable->CursorEvent += Event::Bind(this, static_cast<void (ComboBoxComponent::*)(int2, int2, PointerInteraction)>(&ComboBoxComponent::HandleMainCursorEvent));
entity->AddComponent(this->interactable);
    if (entity->get_Children() == nullptr)
    {
entity->InitChildren();
    }
this->labelEntity = new ::Entity();
this->labelEntity->set_LayerMask(entity->get_LayerMask());
this->labelEntity->set_Enabled(true);
this->labelEntity->InitComponents();
entity->AddChild(this->labelEntity);
this->labelText = new ::TextComponent();
this->labelText->set_Font(this->font);
this->labelText->set_Color(ThemeManager::get_Colors()->InputForegroundPrimary);
this->labelText->set_RenderOrder2D(this->textOrder);
this->labelEntity->AddComponent(this->labelText);
this->arrowEntity = new ::Entity();
this->arrowEntity->set_LayerMask(entity->get_LayerMask());
this->arrowEntity->set_Enabled(true);
this->arrowEntity->InitComponents();
entity->AddChild(this->arrowEntity);
this->arrowText = new ::TextComponent();
this->arrowText->set_Font(this->font);
this->arrowText->set_Color(ThemeManager::get_Colors()->InputForegroundSecondary);
this->arrowText->set_RenderOrder2D(this->textOrder);
this->arrowEntity->AddComponent(this->arrowText);
this->listRoot = new ::Entity();
this->listRoot->set_LayerMask(entity->get_LayerMask());
this->listRoot->InitComponents();
this->listRoot->InitChildren();
entity->AddChild(this->listRoot);
this->listBackground = new ::RoundedRectComponent();
this->listBackground->set_RenderOrder2D(this->listBackgroundOrder);
this->listBackground->set_BorderThickness(1.0f);
this->listBackground->set_FillColor(ThemeManager::get_Colors()->SurfacePrimary);
this->listBackground->set_BorderColor(ThemeManager::get_Colors()->AccentTertiary);
this->listRoot->AddComponent(this->listBackground);
::ComboBoxUpdateComponent *updateComponent = new ::ComboBoxUpdateComponent(this);
updateComponent->set_UpdateOrder(Core::Instance->ObjectManager->GetUpdateOrderForLayer(static_cast<int32_t>(1)));
entity->AddComponent(updateComponent);
this->EnsureItemVisuals(static_cast<int32_t>(this->items->get_Count()));
this->UpdateLabelText();
this->UpdateLayout();
this->UpdateDropdownVisibility();
}

void ComboBoxComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
this->isHovering = false;
this->isPressed = false;
this->ResetItemStates();
this->SetTargetFocused(false);
}

bool ComboBoxComponent::ContainsScreenPoint(int32_t x, int32_t y)
{
    if (this->Parent == nullptr)
    {
return false;    }
::float3 origin = this->Parent->get_Position();
return x >= origin.X && x < origin.X + this->size.X && y >= origin.Y && y < origin.Y + this->size.Y;}

void ComboBoxComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (!newEnabled)
    {
this->isHovering = false;
this->isPressed = false;
this->ResetItemStates();
this->SetTargetFocused(false);
    }
}

void ComboBoxComponent::SetItems(List<std::string>* items, int32_t selectedIndex)
{
    if (items == nullptr)
    {
throw new ArgumentNullException("items");
    }
this->ValidateItems(items);
const int32_t validatedIndex = this->ValidateSelectedIndex(static_cast<int32_t>(items->get_Count()), static_cast<int32_t>(selectedIndex));
const bool selectionChanged = this->selectedIndex != validatedIndex;
this->items->Clear();
for (int32_t i = 0; i < items->get_Count(); i++) {
this->items->Add((*items).get_Item(static_cast<int32_t>(i)));
}
this->selectedIndex = validatedIndex;
    if (this->items->get_Count() == 0 && this->isOpen)
    {
this->isOpen = false;
    }
this->UpdateLabelText();
this->UpdateLayout();
this->UpdateDropdownVisibility();
    if (selectionChanged && this->get_HasSelection() && true)
    {
this->SelectionChanged.Invoke(this->selectedIndex, (*this->items).get_Item(static_cast<int32_t>(this->selectedIndex)));
    }
}

void ComboBoxComponent::SetRenderOrders(uint8_t backgroundOrder, uint8_t textOrder, uint8_t listBackgroundOrder, uint8_t listTextOrder)
{
this->hasRenderOrderOverrides = true;
this->backgroundOrder = backgroundOrder;
this->textOrder = textOrder;
this->listBackgroundOrder = listBackgroundOrder;
this->listTextOrder = listTextOrder;
this->ApplyRenderOrders();
}

void ComboBoxComponent::SetTargetFocused(bool isFocused)
{
this->set_IsKeyboardFocused(isFocused);
    if (!isFocused && this->isOpen)
    {
this->set_IsOpen(false);
    }
this->UpdateMainVisual();
}

void ComboBoxComponent::Update()
{
    if (!this->isOpen || this->Parent == nullptr || this->listRoot == nullptr)
    {
return;    }
::InputSystem *inputManager = Core::Instance->Input;
    if (!inputManager->WasMouseLeftButtonPressed())
    {
return;    }
const int32_t mouseX = inputManager->GetMouseX();
const int32_t mouseY = inputManager->GetMouseY();
    if (this->IsPointerInsideCombo(static_cast<int32_t>(mouseX), static_cast<int32_t>(mouseY)))
    {
return;    }
this->set_IsOpen(false);
}

void ComboBoxComponent::UseModalPresentation()
{
this->SetRenderOrders(static_cast<uint8_t>(RenderOrder2D::ModalBackground), static_cast<uint8_t>(RenderOrder2D::ModalForeground), static_cast<uint8_t>(RenderOrder2D::ModalOverlayBackground), static_cast<uint8_t>(RenderOrder2D::ModalOverlayForeground));
}

void ComboBoxComponent::UsePanelPresentation()
{
this->SetRenderOrders(static_cast<uint8_t>(RenderOrder2D::PanelSurface), static_cast<uint8_t>(RenderOrder2D::PanelForeground), static_cast<uint8_t>(RenderOrder2D::OverlayBackground), static_cast<uint8_t>(RenderOrder2D::OverlayForeground));
}

::Entity* ComboBoxComponent::get_Parent()
{
return Component::get_Parent();
}

void ComboBoxComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool ComboBoxComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* ComboBoxComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool ComboBoxComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void ComboBoxComponent::ApplyRenderOrders()
{
    if (this->background != nullptr)
    {
this->background->set_RenderOrder2D(this->backgroundOrder);
    }
    if (this->labelText != nullptr)
    {
this->labelText->set_RenderOrder2D(this->textOrder);
    }
    if (this->arrowText != nullptr)
    {
this->arrowText->set_RenderOrder2D(this->textOrder);
    }
    if (this->listBackground != nullptr)
    {
this->listBackground->set_RenderOrder2D(this->listBackgroundOrder);
    }
for (int32_t i = 0; i < this->itemVisuals->get_Count(); i++) {
::ComboBoxItemVisual *entry = (*this->itemVisuals).get_Item(static_cast<int32_t>(i));
entry->Background->set_RenderOrder2D(this->listBackgroundOrder);
entry->Label->set_RenderOrder2D(this->listTextOrder);
}
}

void ComboBoxComponent::CopyItems(List<std::string>* source)
{
this->ValidateItems(source);
for (int32_t i = 0; i < source->get_Count(); i++) {
this->items->Add((*source).get_Item(static_cast<int32_t>(i)));
}
}

::ComboBoxItemVisual* ComboBoxComponent::CreateItemVisual()
{
::ComboBoxItemVisual *entry = new ::ComboBoxItemVisual(this->font, static_cast<uint16_t>(this->listRoot->get_LayerMask()), static_cast<uint8_t>(this->listBackgroundOrder), static_cast<uint8_t>(this->listTextOrder));
entry->Background->set_FillColor(ThemeManager::get_Colors()->SurfaceInput);
entry->Background->set_BorderColor(ThemeManager::get_Colors()->AccentTertiary);
entry->Label->set_Color(ThemeManager::get_Colors()->InputForegroundPrimary);
return entry;}

void ComboBoxComponent::EnsureItemVisuals(int32_t count)
{
    if (this->listRoot == nullptr)
    {
return;    }
for (int32_t i = this->itemVisuals->get_Count(); i < count; i++) {
::ComboBoxItemVisual *entry = this->CreateItemVisual();
entry->CursorEvent += Event::Bind(this, static_cast<void (ComboBoxComponent::*)(ComboBoxItemVisual*, int2, int2, PointerInteraction)>(&ComboBoxComponent::HandleItemCursorEvent));
this->listRoot->AddChild(entry->Root);
this->itemVisuals->Add(entry);
}
}

::ICamera* ComboBoxComponent::FindTopmostCameraAt(int32_t x, int32_t y, uint16_t layerMask)
{
List<::ICamera*> *cameras = Core::Instance->ObjectManager->Cameras;
for (int32_t i = cameras->get_Count() - 1; i >= 0; i--) {
::ICamera *camera = (*cameras).get_Item(static_cast<int32_t>(i));
    if ((camera->get_LayerMask() & layerMask) == 0)
    {
continue;
    }
::float4 vp = camera->get_Viewport();
    if (x >= vp.X && x < vp.X + vp.Z && y >= vp.Y && y < vp.Y + vp.W)
    {
return camera;    }
}
return nullptr;}

float ComboBoxComponent::GetCornerRadius(::int2 size)
{
const double minAxis = Math::Min(static_cast<int32_t>(size.X), static_cast<int32_t>(size.Y));
return static_cast<float>((minAxis * 0.15));}

void ComboBoxComponent::HandleItemCursorEvent(::ComboBoxItemVisual* entry, ::int2 relPos, ::int2 delta, ::PointerInteraction state)
{
switch (state) {
case PointerInteraction::Hover: {
entry->set_IsHovering(true);
this->UpdateItemVisualState(entry, entry->Index == this->selectedIndex);
break;
}
case PointerInteraction::Press: {
entry->set_IsPressed(true);
this->UpdateItemVisualState(entry, entry->Index == this->selectedIndex);
break;
}
case PointerInteraction::Release: {
const bool shouldSelect = entry->IsPressed && entry->IsHovering;
entry->set_IsPressed(false);
this->UpdateItemVisualState(entry, entry->Index == this->selectedIndex);
    if (shouldSelect)
    {
this->SetSelectedIndexInternal(static_cast<int32_t>(entry->Index), true);
this->set_IsOpen(false);
    }
break;
}
case PointerInteraction::Leave: {
entry->set_IsHovering(false);
entry->set_IsPressed(false);
this->UpdateItemVisualState(entry, entry->Index == this->selectedIndex);
break;
}
case PointerInteraction::None: {
break;
}
}

}

void ComboBoxComponent::HandleMainCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state)
{
switch (state) {
case PointerInteraction::Hover: {
    if (!this->isHovering)
    {
this->isHovering = true;
this->UpdateMainVisual();
    }
break;
}
case PointerInteraction::Press: {
this->isPressed = true;
this->UpdateMainVisual();
break;
}
case PointerInteraction::Release: {
const bool shouldToggle = this->isPressed && this->isHovering;
this->isPressed = false;
this->UpdateMainVisual();
    if (shouldToggle && this->items->get_Count() > 0)
    {
this->set_IsOpen(!this->isOpen);
    }
break;
}
case PointerInteraction::Leave: {
    if (this->isHovering || this->isPressed)
    {
this->isHovering = false;
this->isPressed = false;
this->UpdateMainVisual();
    }
break;
}
case PointerInteraction::None: {
break;
}
}

}

void ComboBoxComponent::HideItemVisuals()
{
for (int32_t i = 0; i < this->itemVisuals->get_Count(); i++) {
::ComboBoxItemVisual *entry = (*this->itemVisuals).get_Item(static_cast<int32_t>(i));
entry->Root->set_Enabled(false);
entry->LabelHost->set_Enabled(false);
entry->Label->set_Text(String::Empty);
entry->Label->set_Size(::int2(static_cast<int32_t>(0), static_cast<int32_t>(0)));
}
}

bool ComboBoxComponent::IsPointerInsideCombo(int32_t mouseX, int32_t mouseY)
{
::ICamera *camera = this->FindTopmostCameraAt(static_cast<int32_t>(mouseX), static_cast<int32_t>(mouseY), static_cast<uint16_t>(this->Parent->get_LayerMask()));
    if (camera == nullptr)
    {
return false;    }
::float4 viewport = camera->get_Viewport();
const double localX = mouseX - viewport.X;
const double localY = mouseY - viewport.Y;
::float3 origin = this->Parent->get_Position();
    if (GeometryUtils::IsPointInsideRect(localX, localY, origin, static_cast<int32_t>(this->size.X), static_cast<int32_t>(this->size.Y)))
    {
return true;    }
    if (!this->isOpen)
    {
return false;    }
const int32_t listHeight = this->itemHeight * this->items->get_Count();
    if (listHeight <= 0)
    {
return false;    }
::float3 listPosition = this->listRoot->get_Position();
return GeometryUtils::IsPointInsideRect(localX, localY, listPosition, static_cast<int32_t>(this->size.X), static_cast<int32_t>(listHeight));}

void ComboBoxComponent::ResetItemStates()
{
for (int32_t i = 0; i < this->itemVisuals->get_Count(); i++) {
::ComboBoxItemVisual *entry = (*this->itemVisuals).get_Item(static_cast<int32_t>(i));
entry->set_IsHovering(false);
entry->set_IsPressed(false);
}
}

void ComboBoxComponent::SetSelectedIndexInternal(int32_t index, bool raiseEvent)
{
const int32_t validated = this->ValidateSelectedIndex(static_cast<int32_t>(this->items->get_Count()), static_cast<int32_t>(index));
    if (this->selectedIndex == validated)
    {
return;    }
this->selectedIndex = validated;
this->UpdateLabelText();
this->UpdateAllItemStates();
    if (raiseEvent && this->get_HasSelection() && true)
    {
this->SelectionChanged.Invoke(this->selectedIndex, (*this->items).get_Item(static_cast<int32_t>(this->selectedIndex)));
    }
}

void ComboBoxComponent::UpdateAllItemStates()
{
const int32_t count = Math::Min(static_cast<int32_t>(this->items->get_Count()), static_cast<int32_t>(this->itemVisuals->get_Count()));
for (int32_t i = 0; i < count; i++) {
this->UpdateItemVisualState((*this->itemVisuals).get_Item(static_cast<int32_t>(i)), i == this->selectedIndex);
}
}

void ComboBoxComponent::UpdateDropdownVisibility()
{
    if (this->listRoot == nullptr)
    {
return;    }
const bool shouldShow = this->isOpen && this->items->get_Count() > 0;
this->listRoot->set_Enabled(shouldShow);
this->UpdateListLayout();
    if (!shouldShow)
    {
this->HideItemVisuals();
this->ResetItemStates();
    }
this->UpdateMainVisual();
}

void ComboBoxComponent::UpdateItemVisualState(::ComboBoxItemVisual* entry, bool isSelected)
{
    if (entry->IsPressed)
    {
entry->Background->set_FillColor(ThemeManager::get_Colors()->AccentSecondary);
    }
else {
    if (entry->IsHovering)
    {
entry->Background->set_FillColor(ThemeManager::get_Colors()->AccentPrimary);
    }
else {
    if (isSelected)
    {
entry->Background->set_FillColor(ThemeManager::get_Colors()->AccentTertiary);
    }
else {
entry->Background->set_FillColor(ThemeManager::get_Colors()->SurfaceInput);
}
}
}
}

void ComboBoxComponent::UpdateLabelLayout()
{
    if (this->labelEntity == nullptr || this->labelText == nullptr || this->arrowEntity == nullptr || this->arrowText == nullptr || this->font == nullptr)
    {
return;    }
const double lineHeight = Math::Max(static_cast<double>(this->font->LineHeight), 1.0);
const double labelY = Math::Round((this->size.Y - lineHeight) / 2.0, static_cast<MidpointRounding>(MidpointRounding::AwayFromZero));
::FontTightMetrics labelMetrics = this->font->MeasureTight(this->labelText->get_Text());
const int32_t labelWidth = static_cast<int32_t>(Math::Ceiling(labelMetrics.Width));
const int32_t labelHeight = static_cast<int32_t>(Math::Ceiling(Math::Max(static_cast<double>(labelMetrics.get_Height()), 1.0)));
this->labelText->set_Size(::int2(static_cast<int32_t>(labelWidth), static_cast<int32_t>(labelHeight)));
this->labelEntity->set_Position(::float3(TextPaddingX, static_cast<float>(labelY), 0.1f));
::FontTightMetrics arrowMetrics = this->font->MeasureTight(ArrowGlyph);
const int32_t arrowWidth = static_cast<int32_t>(Math::Ceiling(arrowMetrics.Width));
const int32_t arrowHeight = static_cast<int32_t>(Math::Ceiling(Math::Max(static_cast<double>(arrowMetrics.get_Height()), 1.0)));
this->arrowText->set_Size(::int2(static_cast<int32_t>(arrowWidth), static_cast<int32_t>(arrowHeight)));
double arrowX = this->size.X - ArrowPaddingX - arrowMetrics.Width;
    if (arrowX < TextPaddingX)
    {
arrowX = TextPaddingX;
    }
arrowX = Math::Round(arrowX, static_cast<MidpointRounding>(MidpointRounding::AwayFromZero));
this->arrowEntity->set_Position(::float3(static_cast<float>(arrowX), static_cast<float>(labelY), 0.1f));
}

void ComboBoxComponent::UpdateLabelText()
{
    if (this->labelText == nullptr || this->arrowText == nullptr)
    {
return;    }
const std::string displayText = this->get_HasSelection() ? (*this->items).get_Item(static_cast<int32_t>(this->selectedIndex)) : String::Empty;
this->labelText->set_Text(displayText);
this->labelText->set_Color(this->get_HasSelection() ? ThemeManager::get_Colors()->InputForegroundPrimary : ThemeManager::get_Colors()->InputForegroundSecondary);
this->arrowText->set_Text(ArrowGlyph);
this->arrowText->set_Color(ThemeManager::get_Colors()->InputForegroundSecondary);
this->UpdateLabelLayout();
}

void ComboBoxComponent::UpdateLayout()
{
this->UpdateMainLayout();
this->UpdateListLayout();
}

void ComboBoxComponent::UpdateListLayout()
{
    if (this->listRoot == nullptr || this->listBackground == nullptr)
    {
return;    }
this->listRoot->set_Position(::float3(0.0f, this->size.Y + ListGap, 0.2f));
int32_t listHeight = this->itemHeight * this->items->get_Count();
    if (listHeight <= 0)
    {
listHeight = 1;
    }
this->listBackground->set_Size(::int2(static_cast<int32_t>(this->size.X), static_cast<int32_t>(listHeight)));
    if (this->background != nullptr)
    {
this->listBackground->set_Radius(this->background->Radius);
    }
this->EnsureItemVisuals(static_cast<int32_t>(this->items->get_Count()));
const double lineHeight = Math::Max(static_cast<double>(this->font->LineHeight), 1.0);
const bool shouldShow = this->isOpen && this->items->get_Count() > 0;
for (int32_t i = 0; i < this->itemVisuals->get_Count(); i++) {
::ComboBoxItemVisual *entry = (*this->itemVisuals).get_Item(static_cast<int32_t>(i));
const bool isActive = i < this->items->get_Count();
const bool isVisible = isActive && shouldShow;
entry->Root->set_Enabled(isVisible);
entry->LabelHost->set_Enabled(isVisible);
    if (!isActive)
    {
entry->Label->set_Text(String::Empty);
entry->Label->set_Size(::int2(static_cast<int32_t>(0), static_cast<int32_t>(0)));
continue;
    }
    if (!shouldShow)
    {
entry->Label->set_Text(String::Empty);
entry->Label->set_Size(::int2(static_cast<int32_t>(0), static_cast<int32_t>(0)));
continue;
    }
entry->set_Index(i);
entry->Root->set_Position(::float3(0.0f, this->itemHeight * i, 0.1f));
entry->Background->set_Size(::int2(static_cast<int32_t>(this->size.X), static_cast<int32_t>(this->itemHeight)));
entry->Background->set_Radius(0.0f);
entry->Background->set_BorderColor(ThemeManager::get_Colors()->AccentTertiary);
entry->Interactable->set_Size(::int2(static_cast<int32_t>(this->size.X), static_cast<int32_t>(this->itemHeight)));
const std::string itemText = (*this->items).get_Item(static_cast<int32_t>(i));
entry->Label->set_Text(itemText);
entry->Label->set_Font(this->font);
entry->Label->set_Color(ThemeManager::get_Colors()->InputForegroundPrimary);
::FontTightMetrics itemMetrics = this->font->MeasureTight(itemText);
entry->Label->set_Size(([&]() {
auto __ctor_arg_000001EE = static_cast<int32_t>(static_cast<int32_t>(Math::Ceiling(itemMetrics.Width)));
auto __ctor_arg_000001EF = static_cast<int32_t>(static_cast<int32_t>(Math::Ceiling(Math::Max(static_cast<double>(itemMetrics.get_Height()), 1.0))));
return ::int2(__ctor_arg_000001EE, __ctor_arg_000001EF);
})());
const double textY = Math::Round((this->itemHeight - lineHeight) / 2.0, static_cast<MidpointRounding>(MidpointRounding::AwayFromZero));
entry->LabelHost->set_Position(::float3(TextPaddingX, static_cast<float>(textY), 0.1f));
this->UpdateItemVisualState(entry, i == this->selectedIndex);
}
}

void ComboBoxComponent::UpdateMainLayout()
{
    if (this->background == nullptr || this->interactable == nullptr)
    {
return;    }
this->background->set_Size(this->size);
this->background->set_Radius(this->GetCornerRadius(this->size));
this->interactable->set_Size(this->size);
this->UpdateLabelLayout();
}

void ComboBoxComponent::UpdateMainVisual()
{
    if (this->background == nullptr)
    {
return;    }
this->background->set_BorderColor(this->IsKeyboardFocused ? ThemeManager::get_Colors()->AccentPrimary : ThemeManager::get_Colors()->AccentTertiary);
    if (this->listBackground != nullptr)
    {
this->listBackground->set_BorderColor(this->background->BorderColor);
    }
    if (this->isPressed || this->isOpen)
    {
this->background->set_FillColor(ThemeManager::get_Colors()->AccentSecondary);
    }
else {
    if (this->isHovering)
    {
this->background->set_FillColor(ThemeManager::get_Colors()->AccentPrimary);
    }
else {
this->background->set_FillColor(ThemeManager::get_Colors()->SurfaceInput);
}
}
}

void ComboBoxComponent::ValidateItems(List<std::string>* items)
{
for (int32_t i = 0; i < items->get_Count(); i++) {
    if (String::IsNullOrEmpty((*items).get_Item(static_cast<int32_t>(i))))
    {
throw ([&]() {
auto __ctor_arg_000001F0 = "ComboBox items must not contain null entries.";
auto __ctor_arg_000001F1 = "items";
return new ArgumentException(__ctor_arg_000001F0, __ctor_arg_000001F1);
})();
    }
}
}

int32_t ComboBoxComponent::ValidateSelectedIndex(int32_t itemCount, int32_t index)
{
    if (index < -1 || index >= itemCount)
    {
throw ([&]() {
auto __ctor_arg_000001F2 = "index";
auto __ctor_arg_000001F3 = "SelectedIndex must be -1 or within the item range.";
return new ArgumentOutOfRangeException(__ctor_arg_000001F2, __ctor_arg_000001F3);
})();
    }
return index;}

