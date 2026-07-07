#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneEntityAssetIdAllocator
{
public:
    virtual ~SceneEntityAssetIdAllocator() = default;

    uint32_t Allocate();

    void Reset();

    SceneEntityAssetIdAllocator();
private:
    uint32_t NextEntityId;
};
