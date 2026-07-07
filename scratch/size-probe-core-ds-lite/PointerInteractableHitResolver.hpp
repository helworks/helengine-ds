#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IInteractable2D;
class ICamera;
class IDrawable2D;
class ICameraBoundViewportOwner;
class Entity;

#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"

class PointerInteractableHitResolver
{
public:
    virtual ~PointerInteractableHitResolver() = default;

    static void GetRelativePointerForInteractable__out4_out5(::IInteractable2D* interactable, int32_t pointerX, int32_t pointerY, ::ICamera* camera, int32_t& relativeX, int32_t& relativeY);

    static ::IInteractable2D* ResolveTopInteractableAt(List<::IInteractable2D*>* interactables, List<::IDrawable2D*>* drawables2D, ::ICamera* camera, int32_t pointerX, int32_t pointerY);
private:
    static bool CandidateIsInFront(uint8_t candidateRenderOrder, int32_t candidateDrawableIndex, int32_t candidateInteractableIndex, uint8_t currentRenderOrder, int32_t currentDrawableIndex, int32_t currentInteractableIndex);

    static ::ICameraBoundViewportOwner* FindNearestViewportOwner(::Entity* entity);

    static uint8_t GetTopDrawableRenderOrder__out3(List<::IDrawable2D*>* drawables2D, ::IInteractable2D* interactable, uint16_t cameraLayerMask, int32_t& candidateDrawableIndex);

    static bool IsInsideActiveClipRegions(::IInteractable2D* interactable, int32_t pointerX, int32_t pointerY);

    static void ResolvePointerInInteractableSpace__out4_out5(::IInteractable2D* interactable, ::ICamera* camera, int32_t pointerX, int32_t pointerY, int32_t& resolvedPointerX, int32_t& resolvedPointerY);
};
