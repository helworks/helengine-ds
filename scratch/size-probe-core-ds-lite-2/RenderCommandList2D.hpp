#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4;
class RuntimeTexture;
class byte4;

#include "runtime/native_disposable.hpp"
#include "runtime/native_list.hpp"
#include "RenderCommand2DType.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "float4.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "byte4.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "RoundedRectCorners.hpp"

class RenderCommandList2D : public ::IDisposable
{
public:
    virtual ~RenderCommandList2D() = default;

    int32_t get_Count();

    void AddClipPop();

    void AddClipPush(::float4 clipRect);

    void AddGlyphQuad(::RuntimeTexture* texture, ::float4 bounds, ::float4 sourceRect, ::byte4 color);

    void AddRoundedRect(::float4 bounds, float radius, float borderThickness, ::RoundedRectCorners corners, ::byte4 fillColor, ::byte4 borderColor);

    void AddTexturedQuad(::RuntimeTexture* texture, ::float4 bounds, ::float4 sourceRect, ::byte4 color, float rotationRadians);

    void Dispose();

    int32_t GetClipPushPayloadIndex(int32_t commandIndex);

    ::float4 GetClipPushRect(int32_t payloadIndex);

    ::RenderCommand2DType GetCommandType(int32_t commandIndex);

    ::float4 GetGlyphQuadBounds(int32_t payloadIndex);

    ::byte4 GetGlyphQuadColor(int32_t payloadIndex);

    int32_t GetGlyphQuadPayloadIndex(int32_t commandIndex);

    ::float4 GetGlyphQuadSourceRect(int32_t payloadIndex);

    ::RuntimeTexture* GetGlyphQuadTexture(int32_t payloadIndex);

    ::byte4 GetRoundedRectBorderColor(int32_t payloadIndex);

    float GetRoundedRectBorderThickness(int32_t payloadIndex);

    ::float4 GetRoundedRectBounds(int32_t payloadIndex);

    ::RoundedRectCorners GetRoundedRectCorners(int32_t payloadIndex);

    ::byte4 GetRoundedRectFillColor(int32_t payloadIndex);

    int32_t GetRoundedRectPayloadIndex(int32_t commandIndex);

    float GetRoundedRectRadius(int32_t payloadIndex);

    ::float4 GetTexturedQuadBounds(int32_t payloadIndex);

    ::byte4 GetTexturedQuadColor(int32_t payloadIndex);

    int32_t GetTexturedQuadPayloadIndex(int32_t commandIndex);

    float GetTexturedQuadRotation(int32_t payloadIndex);

    ::float4 GetTexturedQuadSourceRect(int32_t payloadIndex);

    ::RuntimeTexture* GetTexturedQuadTexture(int32_t payloadIndex);

    RenderCommandList2D(int32_t initialCapacity);

    void Reset();
private:
    List<::RenderCommand2DType>* CommandTypes;

    List<int32_t>* PayloadIndices;

    List<::float4>* ClipRects;

    List<::RuntimeTexture*>* QuadTextures;

    List<::float4>* QuadBounds;

    List<::float4>* QuadSourceRects;

    List<::byte4>* QuadColors;

    List<float>* QuadRotations;

    List<::RuntimeTexture*>* GlyphTextures;

    List<::float4>* GlyphBounds;

    List<::float4>* GlyphSourceRects;

    List<::byte4>* GlyphColors;

    List<::float4>* RoundedRectBounds;

    List<float>* RoundedRectRadii;

    List<float>* RoundedRectBorderThicknesses;

    List<::RoundedRectCorners>* RoundedRectCornersValues;

    List<::byte4>* RoundedRectFillColors;

    List<::byte4>* RoundedRectBorderColors;

    bool IsDisposedValue;
};
