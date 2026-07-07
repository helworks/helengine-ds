#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class int2;

#include "PointerCursorKind.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "PointerInteraction.hpp"

class IInteractable2D
{
public:
    virtual ::Entity* get_Parent() = 0;

    virtual ::PointerCursorKind get_HoverCursor() = 0;

    virtual ::int2 get_Size() = 0;

    virtual void set_Size(::int2 value) = 0;

    ::Event CursorEvent;

    virtual void OnCursor(::int2 relPos, ::int2 delta, ::PointerInteraction state) = 0;
};
