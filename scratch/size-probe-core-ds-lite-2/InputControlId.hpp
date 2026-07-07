#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_equatable.hpp"
#include "InputDeviceKind.hpp"
#include "InputControlKind.hpp"

class InputControlId
{
public:
    InputControlId();

    ::InputDeviceKind DeviceKind;

    ::InputDeviceKind get_DeviceKind();

    ::InputControlKind ControlKind;

    ::InputControlKind get_ControlKind();

    int32_t DeviceIndex;

    int32_t get_DeviceIndex();

    int32_t ControlIndex;

    int32_t get_ControlIndex();

    bool Equals(::InputControlId other);

    bool Equals(void* obj);

    int32_t GetHashCode();

    InputControlId(::InputDeviceKind deviceKind, ::InputControlKind controlKind, int32_t deviceIndex, int32_t controlIndex);

    friend bool operator!=(::InputControlId left, ::InputControlId right);

    friend bool operator==(::InputControlId left, ::InputControlId right);
};
