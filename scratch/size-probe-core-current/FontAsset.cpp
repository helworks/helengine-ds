#ifdef DrawText
#undef DrawText
#endif
#include "FontAsset.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/math.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "FontInfo.hpp"
#include "TextureAsset.hpp"
#include "runtime/array.hpp"
#include "NativeOwnership.hpp"
#include "float2.hpp"
#include "FontTightMetrics.hpp"
#include "FontChar.hpp"
#include "RuntimeTexture.hpp"
#include "runtime/native_string.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "FontAsset.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"

int32_t FontAsset::get_LiveInstanceCount()
{
return LiveInstanceCountValue;
}

int32_t FontAsset::get_ConstructedInstanceCount()
{
return ConstructedInstanceCountValue;
}

int32_t FontAsset::get_DisposedInstanceCount()
{
return DisposedInstanceCountValue;
}

int32_t FontAsset::get_LiveCharacterCount()
{
return LiveCharacterCountValue;
}

::FontInfo* FontAsset::get_FontInfo()
{
return this->FontInfo;
}

void FontAsset::set_FontInfo(::FontInfo* value)
{
this->FontInfo = value;
}

::RuntimeTexture* FontAsset::get_Texture()
{
return this->Texture;
}

void FontAsset::set_Texture(::RuntimeTexture* value)
{
this->Texture = value;
}

Dictionary<char, ::FontChar>* FontAsset::get_Characters()
{
return this->Characters;
}

void FontAsset::set_Characters(Dictionary<char, ::FontChar>* value)
{
this->Characters = value;
}

float FontAsset::get_LineHeight()
{
return this->LineHeight;
}

void FontAsset::set_LineHeight(float value)
{
this->LineHeight = value;
}

int32_t FontAsset::get_AtlasWidth()
{
return this->AtlasWidth;
}

void FontAsset::set_AtlasWidth(int32_t value)
{
this->AtlasWidth = value;
}

int32_t FontAsset::get_AtlasHeight()
{
return this->AtlasHeight;
}

void FontAsset::set_AtlasHeight(int32_t value)
{
this->AtlasHeight = value;
}

::TextureAsset* FontAsset::get_SourceTextureAsset()
{
return this->SourceTextureAsset;
}

void FontAsset::set_SourceTextureAsset(::TextureAsset* value)
{
this->SourceTextureAsset = value;
}

const std::string& FontAsset::get_CookedAtlasTextureRelativePath()
{
return this->CookedAtlasTextureRelativePath;
}

void FontAsset::set_CookedAtlasTextureRelativePath(std::string value)
{
this->CookedAtlasTextureRelativePath = value;
}

bool FontAsset::get_IsDisposed()
{
return this->IsDisposed;
}

void FontAsset::set_IsDisposed(bool value)
{
this->IsDisposed = value;
}

void FontAsset::ApplyProcessedSourceTextureAsset(::TextureAsset* processedSourceTextureAsset)
{
    if (processedSourceTextureAsset == nullptr)
    {
throw new ArgumentNullException("processedSourceTextureAsset");
    }
else {
    if (processedSourceTextureAsset->Width < 1 || processedSourceTextureAsset->Height < 1)
    {
throw new InvalidOperationException("Processed font atlas textures must have positive dimensions.");
    }
}
const int32_t originalAtlasWidth = Math::Max(static_cast<int32_t>(this->AtlasWidth), static_cast<int32_t>(1));
const int32_t originalAtlasHeight = Math::Max(static_cast<int32_t>(this->AtlasHeight), static_cast<int32_t>(1));
const double scaleX = static_cast<double>(processedSourceTextureAsset->Width) / originalAtlasWidth;
const double scaleY = static_cast<double>(processedSourceTextureAsset->Height) / originalAtlasHeight;
const bool atlasWasResized = originalAtlasWidth != processedSourceTextureAsset->Width || originalAtlasHeight != processedSourceTextureAsset->Height;
    if (atlasWasResized)
    {
this->set_AtlasWidth(processedSourceTextureAsset->Width);
this->set_AtlasHeight(processedSourceTextureAsset->Height);
this->set_LineHeight(static_cast<float>((this->LineHeight * scaleY)));
    if (this->FontInfo != nullptr)
    {
this->FontInfo->set_LineSpacing(Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(this->FontInfo->LineSpacing * scaleY)))));
this->FontInfo->set_SpaceWidth(static_cast<float>((this->FontInfo->SpaceWidth * scaleX)));
    }
    if (this->Characters != nullptr)
    {
List<char> *keys = new List<char>();
auto __localDeleteGuard_000001C6 = he_cpp_make_scope_exit([&]() {
delete keys;
});
for (const auto& key : this->Characters->Keys()) {
keys->Add(static_cast<char>(key));
}
for (int32_t keyIndex = 0; keyIndex < keys->get_Count(); keyIndex++) {
const char key = (*keys).get_Item(static_cast<int32_t>(keyIndex));
::FontChar glyph = (*this->Characters).get_Item(static_cast<char>(key));
glyph.OffsetY = static_cast<float>((glyph.OffsetY * scaleY));
glyph.AdvanceWidth = static_cast<float>((glyph.AdvanceWidth * scaleX));
glyph.BearingX = static_cast<float>((glyph.BearingX * scaleX));
glyph.BearingY = static_cast<float>((glyph.BearingY * scaleY));
(*this->Characters).get_Item(static_cast<char>(key)) = glyph;
}
    }
    }
else {
this->set_AtlasWidth(processedSourceTextureAsset->Width);
this->set_AtlasHeight(processedSourceTextureAsset->Height);
}
this->set_SourceTextureAsset(processedSourceTextureAsset);
    if (this->Texture != nullptr)
    {
this->Texture->set_Width(processedSourceTextureAsset->Width);
this->Texture->set_Height(processedSourceTextureAsset->Height);
    }
}

void FontAsset::AttachCookedRuntimeTexture(::RuntimeTexture* runtimeTexture)
{
    if (runtimeTexture == nullptr)
    {
throw new ArgumentNullException("runtimeTexture");
    }
this->set_Texture(runtimeTexture);
    if (runtimeTexture->Width > 0)
    {
this->set_AtlasWidth(runtimeTexture->Width);
    }
    if (runtimeTexture->Height > 0)
    {
this->set_AtlasHeight(runtimeTexture->Height);
    }
}

void FontAsset::AttachProcessedTexture(::RuntimeTexture* runtimeTexture, ::TextureAsset* processedSourceTextureAsset)
{
    if (runtimeTexture == nullptr)
    {
throw new ArgumentNullException("runtimeTexture");
    }
else {
    if (processedSourceTextureAsset == nullptr)
    {
throw new ArgumentNullException("processedSourceTextureAsset");
    }
}
this->set_Texture(runtimeTexture);
this->ApplyProcessedSourceTextureAsset(processedSourceTextureAsset);
}

void FontAsset::Dispose()
{
    if (this->IsDisposed)
    {
return;    }
Dictionary<char, ::FontChar> *characters = this->Characters;
::FontInfo *fontInfo = this->FontInfo;
::TextureAsset *sourceTextureAsset = this->SourceTextureAsset;
Array<uint8_t> *sourceTextureColors = sourceTextureAsset == nullptr ? nullptr : sourceTextureAsset->Colors;
Array<uint8_t> *sourceTexturePaletteColors = sourceTextureAsset == nullptr ? nullptr : sourceTextureAsset->PaletteColors;
const bool sourceTextureColorsUsesSharedEmptyArray = (sourceTextureColors == Array<uint8_t>::Empty());
const bool sourceTexturePaletteColorsUsesSharedEmptyArray = (sourceTexturePaletteColors == Array<uint8_t>::Empty());
LiveInstanceCountValue--;
DisposedInstanceCountValue++;
LiveCharacterCountValue -= characters == nullptr ? 0 : characters->get_Count();
this->set_Texture(nullptr);
this->set_Characters(nullptr);
this->set_FontInfo(nullptr);
this->set_SourceTextureAsset(nullptr);
    if (sourceTextureAsset != nullptr)
    {
sourceTextureAsset->Colors = nullptr;
sourceTextureAsset->PaletteColors = nullptr;
    }
    if (characters != nullptr)
    {
characters->Clear();
    }
delete characters;
delete fontInfo;
    if (!sourceTextureColorsUsesSharedEmptyArray)
    {
delete sourceTextureColors;
    }
    if (!sourceTexturePaletteColorsUsesSharedEmptyArray)
    {
delete sourceTexturePaletteColors;
    }
delete sourceTextureAsset;
this->set_IsDisposed(true);
}

FontAsset::FontAsset(::FontInfo* fontInfo, ::RuntimeTexture* tex, Dictionary<char, ::FontChar>* chars, float lineHeight, int32_t atlasWidth, int32_t atlasHeight) : FontInfo(), Texture(), Characters(), LineHeight(), AtlasWidth(0), AtlasHeight(0), SourceTextureAsset(), CookedAtlasTextureRelativePath(), IsDisposed()
{
this->set_LineHeight(lineHeight);
this->set_FontInfo(fontInfo);
this->set_Texture(tex);
this->set_Characters(chars);
this->set_AtlasWidth(atlasWidth);
this->set_AtlasHeight(atlasHeight);
ConstructedInstanceCountValue++;
LiveInstanceCountValue++;
LiveCharacterCountValue += chars == nullptr ? 0 : chars->get_Count();
}

::float2 FontAsset::MeasureString(std::string text)
{
float x = 0.0f;
float y = 0.0f;
float maxX = 0.0f;
const float line = Math::Max(this->LineHeight, 1.0f);
for (int32_t i = 0; i < static_cast<int32_t>(text.size()); i++) {
const char c = text[i];
    if (c == '\n')
    {
    if (x > maxX)
    {
maxX = x;
    }
y += line;
x = 0.0f;
continue;
    }
    if (c == ' ')
    {
x += this->FontInfo->SpaceWidth;
continue;
    }
::FontChar ch;
    if (this->Characters->TryGetValue(static_cast<char>(c), ch))
    {
const float adv = ch.AdvanceWidth > 0 ? ch.AdvanceWidth : ch.SourceRect.Z;
x += adv;
    }
}
    if (x > maxX)
    {
maxX = x;
    }
return ::float2(maxX, y + line);}

::FontTightMetrics FontAsset::MeasureTight(std::string text)
{
float width = 0.0f;
float minTop = 3.4028234663852886e38f;
float maxBottom = -3.4028234663852886e38f;
for (int32_t i = 0; i < static_cast<int32_t>(text.size()); i++) {
const char c = text[i];
    if (c == ' ')
    {
width += this->FontInfo->SpaceWidth;
continue;
    }
::FontChar ch;
    if (!this->Characters->TryGetValue(static_cast<char>(c), ch))
    {
continue;
    }
const float advance = ch.AdvanceWidth > 0 ? ch.AdvanceWidth : (ch.SourceRect.Z * this->AtlasWidth);
width += advance;
const float glyphTop = ch.OffsetY;
const float glyphBottom = ch.OffsetY + (ch.SourceRect.W * this->AtlasHeight);
    if (glyphTop < minTop)
    {
minTop = glyphTop;
    }
    if (glyphBottom > maxBottom)
    {
maxBottom = glyphBottom;
    }
}
    if (minTop == 3.4028234663852886e38f)
    {
minTop = 0.0f;
maxBottom = this->LineHeight > 0 ? this->LineHeight : 1.0f;
    }
return ::FontTightMetrics(width, minTop, maxBottom);}

int32_t FontAsset::LiveInstanceCountValue = 0;

int32_t FontAsset::ConstructedInstanceCountValue = 0;

int32_t FontAsset::DisposedInstanceCountValue = 0;

int32_t FontAsset::LiveCharacterCountValue = 0;

