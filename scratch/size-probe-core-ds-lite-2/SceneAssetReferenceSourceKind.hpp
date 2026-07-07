#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class SceneAssetReferenceSourceKind
{
    FileSystem = 1,
    Generated = 2
};
