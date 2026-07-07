#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeTexture;
class TextureAsset;
class IRoundedRectDrawable2D;
class ISpriteDrawable2D;
class ITextDrawable2D;
class FontAsset;

#include "runtime/native_disposable.hpp"
#include "runtime/native_string.hpp"

class RenderManager2D : public ::IDisposable
{
public:
    virtual ~RenderManager2D() = default;

    virtual ::RuntimeTexture* BuildTextureFromCooked(std::string cookedAssetPath);

    virtual ::RuntimeTexture* BuildTextureFromRaw(::TextureAsset* data) = 0;

    virtual void Dispose();

    virtual void Draw();

    virtual void DrawRoundedRect(::IRoundedRectDrawable2D* shape) = 0;

    virtual void DrawSprite(::ISpriteDrawable2D* sprite) = 0;

    virtual void DrawText(::ITextDrawable2D* text) = 0;

    virtual void FlushReleasedTextures();

    virtual void ReleaseFont(::FontAsset* font);

    virtual void ReleaseTexture(::RuntimeTexture* texture);

    virtual void Update();
};
