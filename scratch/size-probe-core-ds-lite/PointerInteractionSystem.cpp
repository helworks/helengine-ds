#ifdef DrawText
#undef DrawText
#endif
#include "PointerInteractionSystem.hpp"
#include "runtime/native_exceptions.hpp"
#include "ObjectManager.hpp"
#include "Core.hpp"
#include "runtime/native_list.hpp"
#include "PointerInteraction.hpp"
#include "InputSystem.hpp"
#include "PointerInteractableHitResolver.hpp"
#include "int2.hpp"
#include "IInteractable2D.hpp"
#include "ICamera.hpp"
#include "float4.hpp"
#include "RenderManager3D.hpp"
#include "PointerCursorKind.hpp"
#include "IDrawable2D.hpp"
#include "CameraViewportResolver.hpp"
#include "Entity.hpp"
#include "IUpdateable.hpp"
#include "IDrawable3D.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "PendingUpdateOperation.hpp"
#include "CoreInitializationOptions.hpp"
#include "ICameraBoundViewportOwner.hpp"
#include "ContentManager.hpp"
#include "IEntityFactory.hpp"
#include "runtime/native_string.hpp"
#include "FontAsset.hpp"
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
#include "runtime/native_dictionary.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
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
#include "Keys.hpp"
#include "InputGamepadButton.hpp"
#include "runtime/native_event.hpp"
#include "RenderTarget.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#include "float3.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeModel.hpp"
#include "ModelAsset.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "system/diagnostics/stopwatch.hpp"

::Core* PointerInteractionSystem::get_Core()
{
return this->Core;
}

void PointerInteractionSystem::set_Core(::Core* value)
{
this->Core = value;
}

::InputSystem* PointerInteractionSystem::get_Input()
{
return this->Input;
}

void PointerInteractionSystem::set_Input(::InputSystem* value)
{
this->Input = value;
}

::IInteractable2D* PointerInteractionSystem::get_Highlighted()
{
return this->Highlighted;
}

void PointerInteractionSystem::set_Highlighted(::IInteractable2D* value)
{
this->Highlighted = value;
}

::IInteractable2D* PointerInteractionSystem::get_Hovering()
{
return this->Hovering;
}

void PointerInteractionSystem::set_Hovering(::IInteractable2D* value)
{
this->Hovering = value;
}

::PointerCursorKind PointerInteractionSystem::get_HoverCursor()
{
    if (this->Hovering == nullptr)
    {
return PointerCursorKind::Default;    }
return this->Hovering->get_HoverCursor();}

void PointerInteractionSystem::ClearInteractionFor(::IInteractable2D* interactable)
{
    if (interactable == nullptr)
    {
throw new ArgumentNullException("interactable");
    }
    if ((this->Hovering == interactable))
    {
this->set_Hovering(nullptr);
    }
    if ((this->Highlighted == interactable))
    {
this->set_Highlighted(nullptr);
this->capturedCamera = nullptr;
    }
}

PointerInteractionSystem::PointerInteractionSystem(::Core* core, ::InputSystem* inputSystem) : Core(), Input(), Highlighted(), Hovering(), capturedCamera()
{
this->set_Core((core != nullptr ? core : throw new ArgumentNullException("core")));
this->set_Input((inputSystem != nullptr ? inputSystem : throw new ArgumentNullException("inputSystem")));
}

void PointerInteractionSystem::Update()
{
::ObjectManager *objectManager = this->Core->ObjectManager;
List<::IInteractable2D*> *interactables = objectManager->Interactables;
List<::IDrawable2D*> *drawables2D = objectManager->Drawables2D;
::PointerInteraction interaction = PointerInteraction::None;
    if (this->Input->WasMouseLeftButtonReleased())
    {
interaction = PointerInteraction::Release;
    }
else {
    if (this->Input->WasMouseLeftButtonPressed())
    {
interaction = PointerInteraction::Press;
    }
}
const int32_t mouseX = this->Input->GetMouseX();
const int32_t mouseY = this->Input->GetMouseY();
    if (this->Highlighted != nullptr)
    {
int32_t pointerX;
int32_t pointerY;
PointerInteractableHitResolver::GetRelativePointerForInteractable__out4_out5(this->Highlighted, static_cast<int32_t>(this->Input->GetMouseX()), static_cast<int32_t>(this->Input->GetMouseY()), this->capturedCamera, pointerX, pointerY);
const int32_t deltaX = this->Input->GetMouseDeltaX();
const int32_t deltaY = this->Input->GetMouseDeltaY();
    if (interaction == PointerInteraction::None && (deltaX != 0 || deltaY != 0))
    {
interaction = PointerInteraction::Hover;
    }
::int2 pointer = ::int2(static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY));
::int2 delta = ::int2(static_cast<int32_t>(deltaX), static_cast<int32_t>(deltaY));
this->Highlighted->OnCursor(pointer, delta, static_cast<PointerInteraction>(interaction));
    if (interaction == PointerInteraction::Release)
    {
this->set_Highlighted(nullptr);
this->capturedCamera = nullptr;
    }
return;    }
::IInteractable2D* hit;
::ICamera* hitCamera;
this->ResolveTopInteractableAt__out4_out5(interactables, drawables2D, static_cast<int32_t>(this->Input->GetMouseX()), static_cast<int32_t>(this->Input->GetMouseY()), hit, hitCamera);
const bool hoveringChanged = hit != this->Hovering;
    if (hoveringChanged && this->Hovering != nullptr)
    {
int32_t prevPointerX;
int32_t prevPointerY;
::ICamera *hoverCamera = this->FindCameraForInteractableAt(this->Hovering, static_cast<int32_t>(this->Input->GetMouseX()), static_cast<int32_t>(this->Input->GetMouseY()));
PointerInteractableHitResolver::GetRelativePointerForInteractable__out4_out5(this->Hovering, static_cast<int32_t>(this->Input->GetMouseX()), static_cast<int32_t>(this->Input->GetMouseY()), hoverCamera, prevPointerX, prevPointerY);
::int2 previousPointer = ::int2(static_cast<int32_t>(prevPointerX), static_cast<int32_t>(prevPointerY));
::int2 zeroDelta = ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));
this->Hovering->OnCursor(previousPointer, zeroDelta, static_cast<PointerInteraction>(PointerInteraction::Leave));
    }
this->set_Hovering(hit);
    if (this->Hovering == nullptr)
    {
return;    }
int32_t currentPointerX;
int32_t currentPointerY;
PointerInteractableHitResolver::GetRelativePointerForInteractable__out4_out5(this->Hovering, static_cast<int32_t>(this->Input->GetMouseX()), static_cast<int32_t>(this->Input->GetMouseY()), hitCamera, currentPointerX, currentPointerY);
const int32_t currentDeltaX = this->Input->GetMouseDeltaX();
const int32_t currentDeltaY = this->Input->GetMouseDeltaY();
    if (interaction == PointerInteraction::Press)
    {
    if (hoveringChanged)
    {
::int2 hoverPointer = ::int2(static_cast<int32_t>(currentPointerX), static_cast<int32_t>(currentPointerY));
::int2 hoverDelta = ::int2(static_cast<int32_t>(currentDeltaX), static_cast<int32_t>(currentDeltaY));
this->Hovering->OnCursor(hoverPointer, hoverDelta, static_cast<PointerInteraction>(PointerInteraction::Hover));
    }
this->set_Highlighted(this->Hovering);
this->capturedCamera = hitCamera;
::int2 pressPointer = ::int2(static_cast<int32_t>(currentPointerX), static_cast<int32_t>(currentPointerY));
::int2 pressDelta = ::int2(static_cast<int32_t>(currentDeltaX), static_cast<int32_t>(currentDeltaY));
this->Hovering->OnCursor(pressPointer, pressDelta, static_cast<PointerInteraction>(PointerInteraction::Press));
    }
else {
    if (hoveringChanged || currentDeltaX != 0 || currentDeltaY != 0)
    {
::int2 hoverPointer = ::int2(static_cast<int32_t>(currentPointerX), static_cast<int32_t>(currentPointerY));
::int2 hoverDelta = ::int2(static_cast<int32_t>(currentDeltaX), static_cast<int32_t>(currentDeltaY));
this->Hovering->OnCursor(hoverPointer, hoverDelta, static_cast<PointerInteraction>(PointerInteraction::Hover));
    }
}
}

::ICamera* PointerInteractionSystem::FindCameraForInteractableAt(::IInteractable2D* interactable, int32_t x, int32_t y)
{
    if (interactable == nullptr)
    {
return nullptr;    }
List<::ICamera*> *cameras = this->Core->ObjectManager->Cameras;
::ICamera *matchedCamera = nullptr;
uint8_t winningDrawOrder = 0;
int32_t winningCameraIndex = -1;
for (int32_t i = 0; i < cameras->get_Count(); i++) {
::ICamera *camera = (*cameras).get_Item(static_cast<int32_t>(i));
    if (!this->ResolveViewportInWindowSpace(camera).Contains(x, y))
    {
continue;
    }
    if ((interactable->get_Parent()->get_LayerMask() & camera->get_LayerMask()) == 0)
    {
continue;
    }
const uint8_t candidateDrawOrder = camera->get_CameraDrawOrder();
    if (matchedCamera == nullptr || candidateDrawOrder > winningDrawOrder || (candidateDrawOrder == winningDrawOrder && i > winningCameraIndex))
    {
matchedCamera = camera;
winningDrawOrder = candidateDrawOrder;
winningCameraIndex = i;
    }
}
return matchedCamera;}

void PointerInteractionSystem::ResolveTopInteractableAt__out4_out5(List<::IInteractable2D*>* interactables, List<::IDrawable2D*>* drawables2D, int32_t x, int32_t y, ::IInteractable2D*& hitInteractable, ::ICamera*& hitCamera)
{
List<::ICamera*> *cameras = this->Core->ObjectManager->Cameras;
hitInteractable = nullptr;
hitCamera = nullptr;
uint8_t winningDrawOrder = 0;
int32_t winningCameraIndex = -1;
for (int32_t i = 0; i < cameras->get_Count(); i++) {
::ICamera *camera = (*cameras).get_Item(static_cast<int32_t>(i));
    if (!this->ResolveViewportInWindowSpace(camera).Contains(x, y))
    {
continue;
    }
::IInteractable2D *candidateInteractable = PointerInteractableHitResolver::ResolveTopInteractableAt(interactables, drawables2D, camera, static_cast<int32_t>(x), static_cast<int32_t>(y));
    if (candidateInteractable == nullptr)
    {
continue;
    }
const uint8_t candidateDrawOrder = camera->get_CameraDrawOrder();
    if (hitInteractable == nullptr || candidateDrawOrder > winningDrawOrder || (candidateDrawOrder == winningDrawOrder && i > winningCameraIndex))
    {
hitInteractable = candidateInteractable;
hitCamera = camera;
winningDrawOrder = candidateDrawOrder;
winningCameraIndex = i;
    }
}
}

::float4 PointerInteractionSystem::ResolveViewportInWindowSpace(::ICamera* camera)
{
    if (camera == nullptr)
    {
throw new ArgumentNullException("camera");
    }
    if (this->Core->RenderManager3D == nullptr)
    {
return camera->get_Viewport();    }
::int2 mainWindowSize = this->Core->RenderManager3D->MainWindowSize;
    if (mainWindowSize.X <= 0 || mainWindowSize.Y <= 0)
    {
return camera->get_Viewport();    }
return CameraViewportResolver::ResolveViewport(camera->get_Viewport(), mainWindowSize.X, mainWindowSize.Y);}

