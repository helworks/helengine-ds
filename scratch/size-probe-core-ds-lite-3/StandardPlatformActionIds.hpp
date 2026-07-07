#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class InputContextId;
class InputActionId;

#include "InputContextId.hpp"
#include "InputActionId.hpp"
#include "StandardPlatformAction.hpp"

class StandardPlatformActionIds
{
public:
    virtual ~StandardPlatformActionIds() = default;

    static ::InputContextId ContextId;

    static ::InputContextId get_ContextId();

    static ::InputActionId AcceptActionId;

    static ::InputActionId get_AcceptActionId();

    static ::InputActionId ReturnActionId;

    static ::InputActionId get_ReturnActionId();

    static ::InputActionId GetActionId(::StandardPlatformAction action);
};
