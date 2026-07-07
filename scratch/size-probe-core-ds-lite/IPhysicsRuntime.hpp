#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IPhysicsRuntime
{
public:
    virtual void Step(double stepSeconds) = 0;
};
