#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class InputSystem;
class StandardPlatformInputConfiguration;

#include "StandardPlatformAction.hpp"

class StandardPlatformInput
{
public:
    virtual ~StandardPlatformInput() = default;

    void Configure(::StandardPlatformInputConfiguration* configuration);

    bool IsActionDown(::StandardPlatformAction action);

    StandardPlatformInput(::InputSystem* inputSystem);

    bool WasActionPressed(::StandardPlatformAction action);

    bool WasActionReleased(::StandardPlatformAction action);
private:
    ::InputSystem* InputSystem;
};
