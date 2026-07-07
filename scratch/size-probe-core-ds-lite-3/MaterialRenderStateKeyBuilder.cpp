#ifdef DrawText
#undef DrawText
#endif
#include "MaterialRenderStateKeyBuilder.hpp"
#include "runtime/native_exceptions.hpp"
#include "MaterialRenderState.hpp"
#include "MaterialBlendMode.hpp"
#include "MaterialCullMode.hpp"
#include "runtime/native_exceptions.hpp"

int32_t MaterialRenderStateKeyBuilder::Build(::MaterialRenderState* renderState)
{
    if (renderState == nullptr)
    {
throw new ArgumentNullException("renderState");
    }
int32_t key = static_cast<int32_t>(renderState->BlendMode) & 0xFF;
key |= (static_cast<int32_t>(renderState->CullMode) & 0xFF) << 8;
    if (renderState->DepthTestEnabled)
    {
key |= 1 << 16;
    }
    if (renderState->DepthWriteEnabled)
    {
key |= 1 << 17;
    }
return key;}

