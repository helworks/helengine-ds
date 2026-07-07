#ifdef DrawText
#undef DrawText
#endif
#include "TextBoxUpdateComponent.hpp"
#include "TextBoxComponent.hpp"
#include "InputSystem.hpp"
#include "Core.hpp"
#include "runtime/native_event.hpp"
#include "IFocusGroup.hpp"
#include "runtime/native_string.hpp"
#include "FontAsset.hpp"
#include "int2.hpp"
#include "TextBoxEditState.hpp"
#include "runtime/native_datetime.hpp"
#include "float3.hpp"
#include "RoundedRectComponent.hpp"
#include "Entity.hpp"
#include "TextComponent.hpp"
#include "InteractableComponent.hpp"
#include "Keys.hpp"
#include "PointerInteraction.hpp"
#include "IInputBackend.hpp"
#include "InputFrameState.hpp"
#include "runtime/native_list.hpp"
#include "InputBinding.hpp"
#include "runtime/native_dictionary.hpp"
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
#include "ObjectManager.hpp"
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
#include "TextBoxUpdateComponent.hpp"
#include "runtime/native_datetime.hpp"
#include "runtime/native_event.hpp"
#include "system/diagnostics/stopwatch.hpp"

TextBoxUpdateComponent::TextBoxUpdateComponent(::TextBoxComponent* textBox) : textBox()
{
this->textBox = textBox;
}

void TextBoxUpdateComponent::Update()
{
this->textBox->Update();
    if (!this->textBox->get_IsFocused())
    {
return;    }
::InputSystem *input = Core::Instance->Input;
    if (!input->WasMouseLeftButtonPressed())
    {
return;    }
const int32_t pointerX = input->GetMouseX();
const int32_t pointerY = input->GetMouseY();
    if (!this->textBox->ContainsScreenPoint(static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY)))
    {
this->textBox->set_IsFocused(false);
    }
}

uint8_t TextBoxUpdateComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void TextBoxUpdateComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* TextBoxUpdateComponent::get_Parent()
{
return Component::get_Parent();
}

void TextBoxUpdateComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool TextBoxUpdateComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* TextBoxUpdateComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool TextBoxUpdateComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

