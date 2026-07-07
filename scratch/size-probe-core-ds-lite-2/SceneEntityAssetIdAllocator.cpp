#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityAssetIdAllocator.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneEntityAssetIdAllocator.hpp"
#include "runtime/native_exceptions.hpp"

uint32_t SceneEntityAssetIdAllocator::Allocate()
{
    if (this->NextEntityId == 0u)
    {
throw new InvalidOperationException("Scene entity id allocation overflowed.");
    }
const uint32_t allocatedId = this->NextEntityId;
this->NextEntityId++;
return allocatedId;}

void SceneEntityAssetIdAllocator::Reset()
{
this->NextEntityId = 1u;
}

SceneEntityAssetIdAllocator::SceneEntityAssetIdAllocator() : NextEntityId(0)
{
this->NextEntityId = 1u;
}

