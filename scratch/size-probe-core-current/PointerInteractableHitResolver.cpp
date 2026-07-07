#ifdef DrawText
#undef DrawText
#endif
#include "PointerInteractableHitResolver.hpp"
#include "runtime/native_exceptions.hpp"
#include "ICamera.hpp"
#include "IInteractable2D.hpp"
#include "Entity.hpp"
#include "float3.hpp"
#include "ICameraBoundViewportOwner.hpp"
#include "CameraComponent.hpp"
#include "float4.hpp"
#include "PointerInteractableHitResolver.hpp"
#include "IDrawable2D.hpp"
#include "GeometryUtils.hpp"
#include "runtime/native_list.hpp"
#include "RenderTarget.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#include "PointerCursorKind.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "PointerInteraction.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "runtime/native_string.hpp"
#include "float2.hpp"
#include "RenderList2D.hpp"
#include "RenderList3D.hpp"
#include "system/math.hpp"
#include "IClipRegion2D.hpp"
#include "system/math.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

void PointerInteractableHitResolver::GetRelativePointerForInteractable__out4_out5(::IInteractable2D* interactable, int32_t pointerX, int32_t pointerY, ::ICamera* camera, int32_t& relativeX, int32_t& relativeY)
{
    if (interactable == nullptr)
    {
throw new ArgumentNullException("interactable");
    }
int32_t localPointerX;
int32_t localPointerY;
PointerInteractableHitResolver::ResolvePointerInInteractableSpace__out4_out5(interactable, camera, static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY), localPointerX, localPointerY);
::float3 position = interactable->get_Parent()->get_Position();
relativeX = static_cast<int32_t>(Math::Round(localPointerX - position.X));
relativeY = static_cast<int32_t>(Math::Round(localPointerY - position.Y));
}

::IInteractable2D* PointerInteractableHitResolver::ResolveTopInteractableAt(List<::IInteractable2D*>* interactables, List<::IDrawable2D*>* drawables2D, ::ICamera* camera, int32_t pointerX, int32_t pointerY)
{
    if (interactables == nullptr)
    {
throw new ArgumentNullException("interactables");
    }
    if (drawables2D == nullptr)
    {
throw new ArgumentNullException("drawables2D");
    }
    if (camera == nullptr)
    {
throw new ArgumentNullException("camera");
    }
const uint16_t cameraLayerMask = camera->get_LayerMask();
::IInteractable2D *hit = nullptr;
uint8_t hitRenderOrder = 0;
int32_t hitDrawableIndex = -1;
int32_t hitInteractableIndex = -1;
for (int32_t interactableIndex = 0; interactableIndex < interactables->get_Count(); interactableIndex++) {
::IInteractable2D *interactable = (*interactables).get_Item(static_cast<int32_t>(interactableIndex));
    if ((interactable->get_Parent()->get_LayerMask() & cameraLayerMask) == 0)
    {
continue;
    }
int32_t localPointerX;
int32_t localPointerY;
PointerInteractableHitResolver::ResolvePointerInInteractableSpace__out4_out5(interactable, camera, static_cast<int32_t>(pointerX), static_cast<int32_t>(pointerY), localPointerX, localPointerY);
    if (!PointerInteractableHitResolver::IsInsideActiveClipRegions(interactable, static_cast<int32_t>(localPointerX), static_cast<int32_t>(localPointerY)))
    {
continue;
    }
::float3 position = interactable->get_Parent()->get_Position();
::float4 rect = ::float4(position.X, position.Y, interactable->get_Size().X, interactable->get_Size().Y);
    if (!rect.Contains(localPointerX, localPointerY))
    {
continue;
    }
int32_t candidateDrawableIndex;
const uint8_t candidateRenderOrder = PointerInteractableHitResolver::GetTopDrawableRenderOrder__out3(drawables2D, interactable, static_cast<uint16_t>(cameraLayerMask), candidateDrawableIndex);
    if (hit == nullptr || PointerInteractableHitResolver::CandidateIsInFront(static_cast<uint8_t>(candidateRenderOrder), static_cast<int32_t>(candidateDrawableIndex), static_cast<int32_t>(interactableIndex), static_cast<uint8_t>(hitRenderOrder), static_cast<int32_t>(hitDrawableIndex), static_cast<int32_t>(hitInteractableIndex)))
    {
hit = interactable;
hitRenderOrder = candidateRenderOrder;
hitDrawableIndex = candidateDrawableIndex;
hitInteractableIndex = interactableIndex;
    }
}
return hit;}

bool PointerInteractableHitResolver::CandidateIsInFront(uint8_t candidateRenderOrder, int32_t candidateDrawableIndex, int32_t candidateInteractableIndex, uint8_t currentRenderOrder, int32_t currentDrawableIndex, int32_t currentInteractableIndex)
{
    if (candidateRenderOrder != currentRenderOrder)
    {
return candidateRenderOrder > currentRenderOrder;    }
    if (candidateDrawableIndex != currentDrawableIndex)
    {
return candidateDrawableIndex > currentDrawableIndex;    }
return candidateInteractableIndex > currentInteractableIndex;}

::ICameraBoundViewportOwner* PointerInteractableHitResolver::FindNearestViewportOwner(::Entity* entity)
{
::Entity *current = entity;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < current->get_Components()->get_Count(); componentIndex++) {
    ICameraBoundViewportOwner* viewportOwner = he_cpp_try_cast<ICameraBoundViewportOwner>((*current->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (viewportOwner != nullptr)
    {
return viewportOwner;    }
}
    }
current = current->Parent;
}
return nullptr;}

uint8_t PointerInteractableHitResolver::GetTopDrawableRenderOrder__out3(List<::IDrawable2D*>* drawables2D, ::IInteractable2D* interactable, uint16_t cameraLayerMask, int32_t& candidateDrawableIndex)
{
candidateDrawableIndex = -1;
uint8_t renderOrder = 0;
    if (drawables2D == nullptr || interactable == nullptr)
    {
return renderOrder;    }
for (int32_t drawableIndex = 0; drawableIndex < drawables2D->get_Count(); drawableIndex++) {
::IDrawable2D *drawable = (*drawables2D).get_Item(static_cast<int32_t>(drawableIndex));
    if (drawable->get_Parent() != interactable->get_Parent())
    {
continue;
    }
    if ((drawable->get_Parent()->get_LayerMask() & cameraLayerMask) == 0)
    {
continue;
    }
    if (candidateDrawableIndex < 0 || drawable->get_RenderOrder2D() >= renderOrder)
    {
renderOrder = drawable->get_RenderOrder2D();
candidateDrawableIndex = drawableIndex;
    }
}
return renderOrder;}

bool PointerInteractableHitResolver::IsInsideActiveClipRegions(::IInteractable2D* interactable, int32_t pointerX, int32_t pointerY)
{
    if (interactable == nullptr || interactable->get_Parent() == nullptr)
    {
return false;    }
::Entity *current = interactable->get_Parent();
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < current->get_Components()->get_Count(); componentIndex++) {
    IClipRegion2D* clipRegion = he_cpp_try_cast<IClipRegion2D>((*current->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (clipRegion != nullptr)
    {
::float4 clipRect = clipRegion->GetClipRect();
    if (!GeometryUtils::IsPointInsideRect(pointerX, pointerY, ::float3(clipRect.X, clipRect.Y, 0.0f), static_cast<int32_t>(static_cast<int32_t>(clipRect.Z)), static_cast<int32_t>(static_cast<int32_t>(clipRect.W))))
    {
return false;    }
    }
}
    }
current = current->Parent;
}
return true;}

void PointerInteractableHitResolver::ResolvePointerInInteractableSpace__out4_out5(::IInteractable2D* interactable, ::ICamera* camera, int32_t pointerX, int32_t pointerY, int32_t& resolvedPointerX, int32_t& resolvedPointerY)
{
resolvedPointerX = pointerX;
resolvedPointerY = pointerY;
    if (interactable == nullptr || interactable->get_Parent() == nullptr || camera == nullptr)
    {
return;    }
::ICameraBoundViewportOwner *viewportOwner = PointerInteractableHitResolver::FindNearestViewportOwner(interactable->get_Parent());
    if (viewportOwner == nullptr)
    {
return;    }
::CameraComponent *boundCamera = viewportOwner->GetBoundCameraComponent();
    if (!(boundCamera == camera))
    {
return;    }
::float4 viewportBounds = viewportOwner->get_ResolvedViewportBounds();
resolvedPointerX -= static_cast<int32_t>(Math::Round(viewportBounds.X));
resolvedPointerY -= static_cast<int32_t>(Math::Round(viewportBounds.Y));
}

