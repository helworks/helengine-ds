#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneEntityReference
{
public:
    virtual ~SceneEntityReference() = default;

    SceneEntityReference();

    uint32_t EntityId;

    uint32_t get_EntityId();
    void set_EntityId(uint32_t value);
};
