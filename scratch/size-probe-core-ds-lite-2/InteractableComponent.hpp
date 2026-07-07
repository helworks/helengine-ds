#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IInteractable2D;
class int2;
class Entity;

#include "Component.hpp"
#include "IInteractable2D.hpp"
#include "runtime/native_disposable.hpp"
#include "PointerCursorKind.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "PointerInteraction.hpp"

class InteractableComponent : public ::Component, public ::IInteractable2D
{
public:
    virtual ~InteractableComponent() = default;

    InteractableComponent();

    ::PointerCursorKind HoverCursor;

    ::PointerCursorKind get_HoverCursor();
    void set_HoverCursor(::PointerCursorKind value);

    ::int2 Size;

    ::int2 get_Size();
    void set_Size(::int2 value);

    ::Event CursorEvent;

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    virtual void OnCursor(::int2 relPos, ::int2 delta, ::PointerInteraction state);

    void ParentEnabledChange(bool newEnabled);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
};
