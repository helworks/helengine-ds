#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class EditorBinaryRecordKind
{
    Asset = 1,
    AssetImportSettings = 2,
    ShaderCacheMetadata = 3,
    FontAsset = 4
};
