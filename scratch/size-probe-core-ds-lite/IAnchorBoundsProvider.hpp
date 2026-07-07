#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class AnchorSpace;

#include "runtime/native_event.hpp"

class IAnchorBoundsProvider
{
public:
    virtual ::AnchorSpace* get_AnchorSpace() = 0;

    ::Event AnchorBoundsChanged;
};
