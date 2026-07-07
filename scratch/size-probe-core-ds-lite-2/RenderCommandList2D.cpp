#ifdef DrawText
#undef DrawText
#endif
#include "RenderCommandList2D.hpp"
#include "NativeOwnership.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderCommand2DType.hpp"
#include "float4.hpp"
#include "RuntimeTexture.hpp"
#include "byte4.hpp"
#include "RoundedRectCorners.hpp"
#include "runtime/array.hpp"
#include "float3.hpp"
#include "runtime/native_string.hpp"
#include "RenderCommandList2D.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

int32_t RenderCommandList2D::get_Count()
{
return this->CommandTypes->get_Count();}

void RenderCommandList2D::AddClipPop()
{
this->CommandTypes->Add(static_cast<RenderCommand2DType>(RenderCommand2DType::ClipPop));
this->PayloadIndices->Add(static_cast<int32_t>(-1));
}

void RenderCommandList2D::AddClipPush(::float4 clipRect)
{
const int32_t payloadIndex = this->ClipRects->get_Count();
this->ClipRects->Add(clipRect);
this->CommandTypes->Add(static_cast<RenderCommand2DType>(RenderCommand2DType::ClipPush));
this->PayloadIndices->Add(static_cast<int32_t>(payloadIndex));
}

void RenderCommandList2D::AddGlyphQuad(::RuntimeTexture* texture, ::float4 bounds, ::float4 sourceRect, ::byte4 color)
{
    if (texture == nullptr)
    {
throw new ArgumentNullException("texture");
    }
const int32_t payloadIndex = this->GlyphTextures->get_Count();
this->GlyphTextures->Add(texture);
this->GlyphBounds->Add(bounds);
this->GlyphSourceRects->Add(sourceRect);
this->GlyphColors->Add(color);
this->CommandTypes->Add(static_cast<RenderCommand2DType>(RenderCommand2DType::GlyphQuad));
this->PayloadIndices->Add(static_cast<int32_t>(payloadIndex));
}

void RenderCommandList2D::AddRoundedRect(::float4 bounds, float radius, float borderThickness, ::RoundedRectCorners corners, ::byte4 fillColor, ::byte4 borderColor)
{
const int32_t payloadIndex = this->RoundedRectBounds->get_Count();
this->RoundedRectBounds->Add(bounds);
this->RoundedRectRadii->Add(radius);
this->RoundedRectBorderThicknesses->Add(borderThickness);
this->RoundedRectCornersValues->Add(static_cast<RoundedRectCorners>(corners));
this->RoundedRectFillColors->Add(fillColor);
this->RoundedRectBorderColors->Add(borderColor);
this->CommandTypes->Add(static_cast<RenderCommand2DType>(RenderCommand2DType::RoundedRect));
this->PayloadIndices->Add(static_cast<int32_t>(payloadIndex));
}

void RenderCommandList2D::AddTexturedQuad(::RuntimeTexture* texture, ::float4 bounds, ::float4 sourceRect, ::byte4 color, float rotationRadians)
{
    if (texture == nullptr)
    {
throw new ArgumentNullException("texture");
    }
const int32_t payloadIndex = this->QuadTextures->get_Count();
this->QuadTextures->Add(texture);
this->QuadBounds->Add(bounds);
this->QuadSourceRects->Add(sourceRect);
this->QuadColors->Add(color);
this->QuadRotations->Add(rotationRadians);
this->CommandTypes->Add(static_cast<RenderCommand2DType>(RenderCommand2DType::TexturedQuad));
this->PayloadIndices->Add(static_cast<int32_t>(payloadIndex));
}

void RenderCommandList2D::Dispose()
{
    if (this->IsDisposedValue)
    {
return;    }
this->Reset();
delete this->CommandTypes;
delete this->PayloadIndices;
delete this->ClipRects;
delete this->QuadTextures;
delete this->QuadBounds;
delete this->QuadSourceRects;
delete this->QuadColors;
delete this->QuadRotations;
delete this->GlyphTextures;
delete this->GlyphBounds;
delete this->GlyphSourceRects;
delete this->GlyphColors;
delete this->RoundedRectBounds;
delete this->RoundedRectRadii;
delete this->RoundedRectBorderThicknesses;
delete this->RoundedRectCornersValues;
delete this->RoundedRectFillColors;
delete this->RoundedRectBorderColors;
this->IsDisposedValue = true;
}

int32_t RenderCommandList2D::GetClipPushPayloadIndex(int32_t commandIndex)
{
return (*this->PayloadIndices).get_Item(static_cast<int32_t>(commandIndex));}

::float4 RenderCommandList2D::GetClipPushRect(int32_t payloadIndex)
{
return (*this->ClipRects).get_Item(static_cast<int32_t>(payloadIndex));}

::RenderCommand2DType RenderCommandList2D::GetCommandType(int32_t commandIndex)
{
return (*this->CommandTypes).get_Item(static_cast<int32_t>(commandIndex));}

::float4 RenderCommandList2D::GetGlyphQuadBounds(int32_t payloadIndex)
{
return (*this->GlyphBounds).get_Item(static_cast<int32_t>(payloadIndex));}

::byte4 RenderCommandList2D::GetGlyphQuadColor(int32_t payloadIndex)
{
return (*this->GlyphColors).get_Item(static_cast<int32_t>(payloadIndex));}

int32_t RenderCommandList2D::GetGlyphQuadPayloadIndex(int32_t commandIndex)
{
return (*this->PayloadIndices).get_Item(static_cast<int32_t>(commandIndex));}

::float4 RenderCommandList2D::GetGlyphQuadSourceRect(int32_t payloadIndex)
{
return (*this->GlyphSourceRects).get_Item(static_cast<int32_t>(payloadIndex));}

::RuntimeTexture* RenderCommandList2D::GetGlyphQuadTexture(int32_t payloadIndex)
{
return (*this->GlyphTextures).get_Item(static_cast<int32_t>(payloadIndex));}

::byte4 RenderCommandList2D::GetRoundedRectBorderColor(int32_t payloadIndex)
{
return (*this->RoundedRectBorderColors).get_Item(static_cast<int32_t>(payloadIndex));}

float RenderCommandList2D::GetRoundedRectBorderThickness(int32_t payloadIndex)
{
return (*this->RoundedRectBorderThicknesses).get_Item(static_cast<int32_t>(payloadIndex));}

::float4 RenderCommandList2D::GetRoundedRectBounds(int32_t payloadIndex)
{
return (*this->RoundedRectBounds).get_Item(static_cast<int32_t>(payloadIndex));}

::RoundedRectCorners RenderCommandList2D::GetRoundedRectCorners(int32_t payloadIndex)
{
return (*this->RoundedRectCornersValues).get_Item(static_cast<int32_t>(payloadIndex));}

::byte4 RenderCommandList2D::GetRoundedRectFillColor(int32_t payloadIndex)
{
return (*this->RoundedRectFillColors).get_Item(static_cast<int32_t>(payloadIndex));}

int32_t RenderCommandList2D::GetRoundedRectPayloadIndex(int32_t commandIndex)
{
return (*this->PayloadIndices).get_Item(static_cast<int32_t>(commandIndex));}

float RenderCommandList2D::GetRoundedRectRadius(int32_t payloadIndex)
{
return (*this->RoundedRectRadii).get_Item(static_cast<int32_t>(payloadIndex));}

::float4 RenderCommandList2D::GetTexturedQuadBounds(int32_t payloadIndex)
{
return (*this->QuadBounds).get_Item(static_cast<int32_t>(payloadIndex));}

::byte4 RenderCommandList2D::GetTexturedQuadColor(int32_t payloadIndex)
{
return (*this->QuadColors).get_Item(static_cast<int32_t>(payloadIndex));}

int32_t RenderCommandList2D::GetTexturedQuadPayloadIndex(int32_t commandIndex)
{
return (*this->PayloadIndices).get_Item(static_cast<int32_t>(commandIndex));}

float RenderCommandList2D::GetTexturedQuadRotation(int32_t payloadIndex)
{
return (*this->QuadRotations).get_Item(static_cast<int32_t>(payloadIndex));}

::float4 RenderCommandList2D::GetTexturedQuadSourceRect(int32_t payloadIndex)
{
return (*this->QuadSourceRects).get_Item(static_cast<int32_t>(payloadIndex));}

::RuntimeTexture* RenderCommandList2D::GetTexturedQuadTexture(int32_t payloadIndex)
{
return (*this->QuadTextures).get_Item(static_cast<int32_t>(payloadIndex));}

RenderCommandList2D::RenderCommandList2D(int32_t initialCapacity) : CommandTypes(), PayloadIndices(), ClipRects(), QuadTextures(), QuadBounds(), QuadSourceRects(), QuadColors(), QuadRotations(), GlyphTextures(), GlyphBounds(), GlyphSourceRects(), GlyphColors(), RoundedRectBounds(), RoundedRectRadii(), RoundedRectBorderThicknesses(), RoundedRectCornersValues(), RoundedRectFillColors(), RoundedRectBorderColors(), IsDisposedValue()
{
    if (initialCapacity < 0)
    {
throw new ArgumentOutOfRangeException("initialCapacity");
    }
this->CommandTypes = new List<::RenderCommand2DType>(static_cast<int32_t>(initialCapacity));
this->PayloadIndices = new List<int32_t>(static_cast<int32_t>(initialCapacity));
this->ClipRects = new List<::float4>(static_cast<int32_t>(initialCapacity));
this->QuadTextures = new List<::RuntimeTexture*>(static_cast<int32_t>(initialCapacity));
this->QuadBounds = new List<::float4>(static_cast<int32_t>(initialCapacity));
this->QuadSourceRects = new List<::float4>(static_cast<int32_t>(initialCapacity));
this->QuadColors = new List<::byte4>(static_cast<int32_t>(initialCapacity));
this->QuadRotations = new List<float>(static_cast<int32_t>(initialCapacity));
this->GlyphTextures = new List<::RuntimeTexture*>(static_cast<int32_t>(initialCapacity));
this->GlyphBounds = new List<::float4>(static_cast<int32_t>(initialCapacity));
this->GlyphSourceRects = new List<::float4>(static_cast<int32_t>(initialCapacity));
this->GlyphColors = new List<::byte4>(static_cast<int32_t>(initialCapacity));
this->RoundedRectBounds = new List<::float4>(static_cast<int32_t>(initialCapacity));
this->RoundedRectRadii = new List<float>(static_cast<int32_t>(initialCapacity));
this->RoundedRectBorderThicknesses = new List<float>(static_cast<int32_t>(initialCapacity));
this->RoundedRectCornersValues = new List<::RoundedRectCorners>(static_cast<int32_t>(initialCapacity));
this->RoundedRectFillColors = new List<::byte4>(static_cast<int32_t>(initialCapacity));
this->RoundedRectBorderColors = new List<::byte4>(static_cast<int32_t>(initialCapacity));
}

void RenderCommandList2D::Reset()
{
this->CommandTypes->Clear();
this->PayloadIndices->Clear();
this->ClipRects->Clear();
this->QuadTextures->Clear();
this->QuadBounds->Clear();
this->QuadSourceRects->Clear();
this->QuadColors->Clear();
this->QuadRotations->Clear();
this->GlyphTextures->Clear();
this->GlyphBounds->Clear();
this->GlyphSourceRects->Clear();
this->GlyphColors->Clear();
this->RoundedRectBounds->Clear();
this->RoundedRectRadii->Clear();
this->RoundedRectBorderThicknesses->Clear();
this->RoundedRectCornersValues->Clear();
this->RoundedRectFillColors->Clear();
this->RoundedRectBorderColors->Clear();
}

