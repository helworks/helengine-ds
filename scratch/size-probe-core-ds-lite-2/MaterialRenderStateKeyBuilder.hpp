#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class MaterialRenderState;

class MaterialRenderStateKeyBuilder
{
public:
    virtual ~MaterialRenderStateKeyBuilder() = default;

    static int32_t Build(::MaterialRenderState* renderState);
};
