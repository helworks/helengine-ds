#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class InputControlId;

#include "StandardPlatformAction.hpp"
#include "InputControlId.hpp"

class StandardPlatformActionBinding
{
public:
    virtual ~StandardPlatformActionBinding() = default;

    ::StandardPlatformAction Action;

    ::StandardPlatformAction get_Action();

    ::InputControlId Control;

    ::InputControlId get_Control();

    StandardPlatformActionBinding(::StandardPlatformAction action, ::InputControlId control);
};
