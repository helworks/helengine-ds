#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IPhysicsRuntime;
class Entity;

#include "IPhysicsRuntime.hpp"
#include "runtime/native_list.hpp"

class ISceneBindablePhysicsRuntime : public ::IPhysicsRuntime
{
public:
    virtual int32_t get_RegisteredBodyCount() = 0;

    virtual void BindScene(List<::Entity*>* rootEntities) = 0;
};
