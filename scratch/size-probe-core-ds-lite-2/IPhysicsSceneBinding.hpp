#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IPhysicsSceneBinding
{
public:
    virtual void SynchronizeFromScene() = 0;

    virtual void SynchronizeToScene() = 0;
};
