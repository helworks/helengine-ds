#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

enum class PendingSceneOperationKind
{
    Load,
    Unload
};
