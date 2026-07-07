#ifdef DrawText
#undef DrawText
#endif
#include "RenderCommandListBuilder2D.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderCommandList2D.hpp"
#include "runtime/native_list.hpp"
#include "IRenderQueue2D.hpp"
#include "NativeOwnership.hpp"
#include "ClipRegionStackBuilder2D.hpp"
#include "system/math.hpp"
#include "float4.hpp"
#include "IClipRegion2D.hpp"
#include "int2.hpp"
#include "ISpriteDrawable2D.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "ITextDrawable2D.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "FontAsset.hpp"
#include "runtime/native_string.hpp"
#include "TextLayoutUtils.hpp"
#include "FontChar.hpp"
#include "IDrawable2D.hpp"
#include "RenderCommand2DType.hpp"
#include "RuntimeTexture.hpp"
#include "byte4.hpp"
#include "RoundedRectCorners.hpp"
#include "IRenderVisitor2D.hpp"
#include "runtime/array.hpp"
#include "float2.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "TextAlignment.hpp"
#include "FontInfo.hpp"
#include "runtime/native_dictionary.hpp"
#include "TextureAsset.hpp"
#include "FontTightMetrics.hpp"
#include "RenderCommandListBuilder2D.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

::RenderCommandList2D* RenderCommandListBuilder2D::Build(::IRenderQueue2D* renderQueue)
{
    if (renderQueue == nullptr)
    {
throw new ArgumentNullException("renderQueue");
    }
    if (this->CommandListValue == nullptr)
    {
this->CommandListValue = new ::RenderCommandList2D(static_cast<int32_t>(Math::Max(static_cast<int32_t>(renderQueue->get_Count()), static_cast<int32_t>(4))));
    }
else {
this->CommandListValue->Reset();
}
this->ActiveClipChain->Clear();
this->NextClipChain->Clear();
renderQueue->VisitOrdered(this);
this->EmitTrailingClipPops();
return this->CommandListValue;}

void RenderCommandListBuilder2D::Dispose()
{
    if (this->IsDisposedValue)
    {
return;    }
this->ActiveClipChain->Clear();
this->NextClipChain->Clear();
if (this->CommandListValue != nullptr)
{
this->CommandListValue->Dispose();
delete this->CommandListValue;
}
delete this->ClipRegionStackBuilder;
delete this->ActiveClipChain;
delete this->NextClipChain;
this->CommandListValue = nullptr;
this->IsDisposedValue = true;
}

RenderCommandListBuilder2D::RenderCommandListBuilder2D() : CommandListValue(), ClipRegionStackBuilder(), ActiveClipChain(), NextClipChain(), IsDisposedValue()
{
this->ClipRegionStackBuilder = new ::ClipRegionStackBuilder2D();
this->ActiveClipChain = new List<::IClipRegion2D*>();
this->NextClipChain = new List<::IClipRegion2D*>();
}

void RenderCommandListBuilder2D::Visit(::IDrawable2D* drawable)
{
    if (drawable == nullptr || drawable->get_Parent() == nullptr || !drawable->get_Parent()->get_IsHierarchyEnabled())
    {
return;    }
this->ClipRegionStackBuilder->BuildClipChain(drawable, this->NextClipChain);
    if (this->ShouldSkipDrawableBecauseItIsFullyClipped(drawable))
    {
return;    }
this->SyncClipTransitions();
    ISpriteDrawable2D* sprite = he_cpp_try_cast<ISpriteDrawable2D>(drawable);
    if (sprite != nullptr)
    {
this->EmitSprite(sprite);
return;    }
    ITextDrawable2D* text = he_cpp_try_cast<ITextDrawable2D>(drawable);
    if (text != nullptr)
    {
this->EmitText(text);
return;    }
    IRoundedRectDrawable2D* roundedRect = he_cpp_try_cast<IRoundedRectDrawable2D>(drawable);
    if (roundedRect != nullptr)
    {
this->EmitRoundedRect(roundedRect);
return;    }
throw new InvalidOperationException("Unsupported 2D drawable type.");
}

void RenderCommandListBuilder2D::EmitRoundedRect(::IRoundedRectDrawable2D* roundedRect)
{
::float3 position = roundedRect->get_Parent()->get_Position();
this->CommandListValue->AddRoundedRect(::float4(position.X, position.Y, roundedRect->get_Size().X, roundedRect->get_Size().Y), roundedRect->get_Radius(), roundedRect->get_BorderThickness(), static_cast<RoundedRectCorners>(roundedRect->get_Corners()), roundedRect->get_FillColor(), roundedRect->get_BorderColor());
}

void RenderCommandListBuilder2D::EmitSprite(::ISpriteDrawable2D* sprite)
{
    if (sprite->get_Texture() == nullptr)
    {
return;    }
::int2 size = sprite->get_Size();
::float3 scale = sprite->get_Parent()->get_Scale();
const float width = (size.X > 0 ? size.X : sprite->get_Texture()->Width) * scale.X;
const float height = (size.Y > 0 ? size.Y : sprite->get_Texture()->Height) * scale.Y;
::float3 position = sprite->get_Parent()->get_Position();
::float3 rotatedRight = float4::RotateVector(float3::get_UnitX(), sprite->get_Parent()->get_Orientation());
const float rotationRadians = static_cast<float>(Math::Atan2(rotatedRight.Y, rotatedRight.X));
this->CommandListValue->AddTexturedQuad(sprite->get_Texture(), ::float4(position.X, position.Y, width, height), sprite->get_SourceRect(), sprite->get_Color(), rotationRadians);
}

void RenderCommandListBuilder2D::EmitText(::ITextDrawable2D* text)
{
::FontAsset *font = text->get_Font();
const double fontScale = Math::Max(static_cast<double>(text->get_FontScale()), 0.0001);
    if (text->get_WrapText())
    {
const std::string wrappedContent = TextLayoutUtils::WrapText(text->get_Text(), font, static_cast<int32_t>(Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(text->get_Size().X / fontScale))))));
double wrappedOffsetX = 0.0;
double wrappedOffsetY = 0.0;
const double wrappedLineHeight = Math::Max(static_cast<double>(font->LineHeight) * fontScale, 1.0);
const double wrappedBaseX = Math::Round(text->get_Parent()->get_Position().X);
const double wrappedBaseY = Math::Round(text->get_Parent()->get_Position().Y);
for (int32_t index = 0; index < static_cast<int32_t>(wrappedContent.size()); index++) {
const char character = wrappedContent[index];
    if (character == '\n')
    {
wrappedOffsetY += wrappedLineHeight;
wrappedOffsetX = 0.0;
continue;
    }
    if (character == ' ')
    {
wrappedOffsetX += font->FontInfo->SpaceWidth * fontScale;
continue;
    }
::FontChar glyph;
    if (!font->Characters->TryGetValue(static_cast<char>(character), glyph))
    {
continue;
    }
const double glyphWidth = glyph.SourceRect.Z * font->AtlasWidth * fontScale;
const double glyphHeight = glyph.SourceRect.W * font->AtlasHeight * fontScale;
const double snappedLineOffsetY = Math::Round(wrappedOffsetY);
this->CommandListValue->AddGlyphQuad(font->Texture, ::float4(static_cast<float>((wrappedBaseX + wrappedOffsetX)), static_cast<float>((wrappedBaseY + snappedLineOffsetY + (glyph.OffsetY * fontScale))), static_cast<float>(glyphWidth), static_cast<float>(glyphHeight)), glyph.SourceRect, text->get_Color());
const double advanceWidth = glyph.AdvanceWidth > 0.0f ? glyph.AdvanceWidth * fontScale : glyphWidth;
wrappedOffsetX += advanceWidth;
}
return;    }
    if (String::IsNullOrEmpty(text->get_Text()))
    {
return;    }
double offsetX = 0.0;
double offsetY = 0.0;
const double lineHeight = Math::Max(static_cast<double>(font->LineHeight) * fontScale, 1.0);
const double baseX = Math::Round(text->get_Parent()->get_Position().X);
const double baseY = Math::Round(text->get_Parent()->get_Position().Y);
for (int32_t index = 0; index < static_cast<int32_t>(text->get_Text().size()); index++) {
const char character = text->get_Text()[index];
    if (character == '\n')
    {
offsetY += lineHeight;
offsetX = 0.0;
continue;
    }
    if (character == ' ')
    {
offsetX += font->FontInfo->SpaceWidth * fontScale;
continue;
    }
::FontChar glyph_00003DB3;
    if (!font->Characters->TryGetValue(static_cast<char>(character), glyph_00003DB3))
    {
continue;
    }
const double glyphWidth = glyph_00003DB3.SourceRect.Z * font->AtlasWidth * fontScale;
const double glyphHeight = glyph_00003DB3.SourceRect.W * font->AtlasHeight * fontScale;
const double snappedLineOffsetY = Math::Round(offsetY);
this->CommandListValue->AddGlyphQuad(font->Texture, ::float4(static_cast<float>((baseX + offsetX)), static_cast<float>((baseY + snappedLineOffsetY + (glyph_00003DB3.OffsetY * fontScale))), static_cast<float>(glyphWidth), static_cast<float>(glyphHeight)), glyph_00003DB3.SourceRect, text->get_Color());
const double advanceWidth = glyph_00003DB3.AdvanceWidth > 0.0f ? glyph_00003DB3.AdvanceWidth * fontScale : glyphWidth;
offsetX += advanceWidth;
}
}

void RenderCommandListBuilder2D::EmitTrailingClipPops()
{
while (this->ActiveClipChain->get_Count() > 0) {
this->CommandListValue->AddClipPop();
this->ActiveClipChain->RemoveAt(static_cast<int32_t>(this->ActiveClipChain->get_Count() - 1));
}
}

int32_t RenderCommandListBuilder2D::GetSharedPrefixLength()
{
int32_t sharedPrefixLength = 0;
const int32_t maxSharedLength = Math::Min(static_cast<int32_t>(this->ActiveClipChain->get_Count()), static_cast<int32_t>(this->NextClipChain->get_Count()));
while (sharedPrefixLength < maxSharedLength && ((*this->ActiveClipChain).get_Item(static_cast<int32_t>(sharedPrefixLength)) == (*this->NextClipChain).get_Item(static_cast<int32_t>(sharedPrefixLength)))) {
sharedPrefixLength++;
}
return sharedPrefixLength;}

bool RenderCommandListBuilder2D::RectsOverlap(::float4 first, ::float4 second)
{
    if (first.Z <= 0.0f || first.W <= 0.0f || second.Z <= 0.0f || second.W <= 0.0f)
    {
return false;    }
const float firstRight = first.X + first.Z;
const float firstBottom = first.Y + first.W;
const float secondRight = second.X + second.Z;
const float secondBottom = second.Y + second.W;
return first.X < secondRight && firstRight > second.X && first.Y < secondBottom && firstBottom > second.Y;}

::float4 RenderCommandListBuilder2D::ResolveClipRectForPush(::IClipRegion2D* clipRegion)
{
::float4 resolvedRect = clipRegion->GetClipRect();
    if (this->ActiveClipChain->get_Count() <= 0)
    {
return resolvedRect;    }
::float4 currentRect = (*this->ActiveClipChain).get_Item(static_cast<int32_t>(this->ActiveClipChain->get_Count() - 1))->GetClipRect();
return this->ClipRegionStackBuilder->Intersect(currentRect, resolvedRect);}

::float4 RenderCommandListBuilder2D::ResolveEffectiveClipRectForNextDrawable()
{
::float4 effectiveRect = (*this->NextClipChain).get_Item(static_cast<int32_t>(0))->GetClipRect();
for (int32_t index = 1; index < this->NextClipChain->get_Count(); index++) {
effectiveRect = this->ClipRegionStackBuilder->Intersect(effectiveRect, (*this->NextClipChain).get_Item(static_cast<int32_t>(index))->GetClipRect());
}
return effectiveRect;}

bool RenderCommandListBuilder2D::ShouldSkipDrawableBecauseItIsFullyClipped(::IDrawable2D* drawable)
{
    if (drawable == nullptr)
    {
return true;    }
    if (this->NextClipChain->get_Count() <= 0)
    {
return false;    }
::float4 drawableBounds;
    if (!this->TryResolveDrawableBounds__out1(drawable, drawableBounds))
    {
return false;    }
::float4 effectiveClipRect = this->ResolveEffectiveClipRectForNextDrawable();
return !this->RectsOverlap(drawableBounds, effectiveClipRect);}

void RenderCommandListBuilder2D::SyncClipTransitions()
{
const int32_t sharedPrefixLength = this->GetSharedPrefixLength();
while (this->ActiveClipChain->get_Count() > sharedPrefixLength) {
this->CommandListValue->AddClipPop();
this->ActiveClipChain->RemoveAt(static_cast<int32_t>(this->ActiveClipChain->get_Count() - 1));
}
while (this->ActiveClipChain->get_Count() < this->NextClipChain->get_Count()) {
::IClipRegion2D *clipRegion = (*this->NextClipChain).get_Item(static_cast<int32_t>(this->ActiveClipChain->get_Count()));
::float4 resolvedRect = this->ResolveClipRectForPush(clipRegion);
this->CommandListValue->AddClipPush(resolvedRect);
this->ActiveClipChain->Add(clipRegion);
}
}

bool RenderCommandListBuilder2D::TryResolveDrawableBounds__out1(::IDrawable2D* drawable, ::float4& bounds)
{
bounds = ::float4();
    if (drawable == nullptr || drawable->get_Parent() == nullptr)
    {
return false;    }
    ISpriteDrawable2D* sprite = he_cpp_try_cast<ISpriteDrawable2D>(drawable);
    if (sprite != nullptr)
    {
    if (sprite->get_Texture() == nullptr)
    {
return false;    }
::int2 size = sprite->get_Size();
const float width = size.X > 0 ? size.X : sprite->get_Texture()->Width;
const float height = size.Y > 0 ? size.Y : sprite->get_Texture()->Height;
::float3 position = sprite->get_Parent()->get_Position();
bounds = ::float4(position.X, position.Y, width, height);
return true;    }
    ITextDrawable2D* text = he_cpp_try_cast<ITextDrawable2D>(drawable);
    if (text != nullptr)
    {
    if (text->get_Size().X <= 0 || text->get_Size().Y <= 0)
    {
return false;    }
::float3 position = text->get_Parent()->get_Position();
bounds = ::float4(position.X, position.Y, text->get_Size().X, text->get_Size().Y);
return true;    }
    IRoundedRectDrawable2D* roundedRect = he_cpp_try_cast<IRoundedRectDrawable2D>(drawable);
    if (roundedRect != nullptr)
    {
    if (roundedRect->get_Size().X <= 0 || roundedRect->get_Size().Y <= 0)
    {
return false;    }
::float3 position = roundedRect->get_Parent()->get_Position();
bounds = ::float4(position.X, position.Y, roundedRect->get_Size().X, roundedRect->get_Size().Y);
return true;    }
return false;}

