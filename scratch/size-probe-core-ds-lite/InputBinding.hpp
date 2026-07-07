#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class InputContextId;
class InputActionId;
class InputControlId;

#include "InputContextId.hpp"
#include "InputActionId.hpp"
#include "InputControlId.hpp"

class InputBinding
{
public:
    InputBinding();

    ::InputContextId ContextId;

    ::InputContextId get_ContextId();

    ::InputActionId ActionId;

    ::InputActionId get_ActionId();

    ::InputControlId Control;

    ::InputControlId get_Control();

    float Scale;

    float get_Scale();

    InputBinding(::InputContextId contextId, ::InputActionId actionId, ::InputControlId control, float scale);
};
