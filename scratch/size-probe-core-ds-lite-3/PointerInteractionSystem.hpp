#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Core;
class InputSystem;
class IInteractable2D;
class ICamera;
class IDrawable2D;
class float4;

#include "PointerCursorKind.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "float4.hpp"

class PointerInteractionSystem
{
public:
    virtual ~PointerInteractionSystem() = default;

    ::Core* Core;

    ::Core* get_Core();
    void set_Core(::Core* value);

    ::InputSystem* Input;

    ::InputSystem* get_Input();
    void set_Input(::InputSystem* value);

    ::IInteractable2D* Highlighted;

    ::IInteractable2D* get_Highlighted();
    void set_Highlighted(::IInteractable2D* value);

    ::IInteractable2D* Hovering;

    ::IInteractable2D* get_Hovering();
    void set_Hovering(::IInteractable2D* value);

    ::PointerCursorKind get_HoverCursor();

    void ClearInteractionFor(::IInteractable2D* interactable);

    PointerInteractionSystem(::Core* core, ::InputSystem* inputSystem);

    void Update();
private:
    ::ICamera* capturedCamera;

    ::ICamera* FindCameraForInteractableAt(::IInteractable2D* interactable, int32_t x, int32_t y);

    void ResolveTopInteractableAt__out4_out5(List<::IInteractable2D*>* interactables, List<::IDrawable2D*>* drawables2D, int32_t x, int32_t y, ::IInteractable2D*& hitInteractable, ::ICamera*& hitCamera);

    ::float4 ResolveViewportInWindowSpace(::ICamera* camera);
};
