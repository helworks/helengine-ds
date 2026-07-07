#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class FontInfo;
class RuntimeTexture;
class FontChar;
class TextureAsset;
class float2;
class FontTightMetrics;

#include "runtime/native_disposable.hpp"
#include "runtime/native_dictionary.hpp"
#include "FontChar.hpp"
#include "runtime/native_string.hpp"
#include "float2.hpp"
#include "FontTightMetrics.hpp"

class FontAsset : public ::IDisposable
{
public:
    virtual ~FontAsset() = default;

    static int32_t get_LiveInstanceCount();

    static int32_t get_ConstructedInstanceCount();

    static int32_t get_DisposedInstanceCount();

    static int32_t get_LiveCharacterCount();

    ::FontInfo* FontInfo;

    ::FontInfo* get_FontInfo();
    void set_FontInfo(::FontInfo* value);

    ::RuntimeTexture* Texture;

    ::RuntimeTexture* get_Texture();
    void set_Texture(::RuntimeTexture* value);

    Dictionary<char, ::FontChar>* Characters;

    Dictionary<char, ::FontChar>* get_Characters();
    void set_Characters(Dictionary<char, ::FontChar>* value);

    float LineHeight;

    float get_LineHeight();
    void set_LineHeight(float value);

    int32_t AtlasWidth;

    int32_t get_AtlasWidth();
    void set_AtlasWidth(int32_t value);

    int32_t AtlasHeight;

    int32_t get_AtlasHeight();
    void set_AtlasHeight(int32_t value);

    ::TextureAsset* SourceTextureAsset;

    ::TextureAsset* get_SourceTextureAsset();
    void set_SourceTextureAsset(::TextureAsset* value);

    std::string CookedAtlasTextureRelativePath;

    const std::string& get_CookedAtlasTextureRelativePath();
    void set_CookedAtlasTextureRelativePath(std::string value);

    bool IsDisposed;

    bool get_IsDisposed();
    void set_IsDisposed(bool value);

    void ApplyProcessedSourceTextureAsset(::TextureAsset* processedSourceTextureAsset);

    void AttachCookedRuntimeTexture(::RuntimeTexture* runtimeTexture);

    void AttachProcessedTexture(::RuntimeTexture* runtimeTexture, ::TextureAsset* processedSourceTextureAsset);

    void Dispose();

    FontAsset(::FontInfo* fontInfo, ::RuntimeTexture* tex, Dictionary<char, ::FontChar>* chars, float lineHeight, int32_t atlasWidth, int32_t atlasHeight);

    ::float2 MeasureString(std::string text);

    ::FontTightMetrics MeasureTight(std::string text);
private:
    static int32_t LiveInstanceCountValue;

    static int32_t ConstructedInstanceCountValue;

    static int32_t DisposedInstanceCountValue;

    static int32_t LiveCharacterCountValue;
};
