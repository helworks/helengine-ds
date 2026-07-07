#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRenderVisitor2D;
class RenderCommandList2D;
class ClipRegionStackBuilder2D;
class IClipRegion2D;
class IRenderQueue2D;
class IDrawable2D;
class IRoundedRectDrawable2D;
class ISpriteDrawable2D;
class ITextDrawable2D;
class float4;

#include "IRenderVisitor2D.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/native_list.hpp"
#include "float4.hpp"

class RenderCommandListBuilder2D : public ::IRenderVisitor2D, public ::IDisposable
{
public:
    virtual ~RenderCommandListBuilder2D() = default;

    ::RenderCommandList2D* Build(::IRenderQueue2D* renderQueue);

    void Dispose();

    RenderCommandListBuilder2D();

    void Visit(::IDrawable2D* drawable);
private:
    ::RenderCommandList2D* CommandListValue;

    ::ClipRegionStackBuilder2D* ClipRegionStackBuilder;

    List<::IClipRegion2D*>* ActiveClipChain;

    List<::IClipRegion2D*>* NextClipChain;

    bool IsDisposedValue;

    void EmitRoundedRect(::IRoundedRectDrawable2D* roundedRect);

    void EmitSprite(::ISpriteDrawable2D* sprite);

    void EmitText(::ITextDrawable2D* text);

    void EmitTrailingClipPops();

    int32_t GetSharedPrefixLength();

    bool RectsOverlap(::float4 first, ::float4 second);

    ::float4 ResolveClipRectForPush(::IClipRegion2D* clipRegion);

    ::float4 ResolveEffectiveClipRectForNextDrawable();

    bool ShouldSkipDrawableBecauseItIsFullyClipped(::IDrawable2D* drawable);

    void SyncClipTransitions();

    bool TryResolveDrawableBounds__out1(::IDrawable2D* drawable, ::float4& bounds);
};
