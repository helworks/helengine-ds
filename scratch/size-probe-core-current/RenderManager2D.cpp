#ifdef DrawText
#undef DrawText
#endif
#include "RenderManager2D.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "NativeOwnership.hpp"
#include "RuntimeTexture.hpp"
#include "FontAsset.hpp"
#include "TextureAsset.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "runtime/array.hpp"
#include "FontInfo.hpp"
#include "runtime/native_dictionary.hpp"
#include "FontChar.hpp"
#include "float2.hpp"
#include "FontTightMetrics.hpp"
#include "RenderManager2D.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::RuntimeTexture* RenderManager2D::BuildTextureFromCooked(std::string cookedAssetPath)
{
    if (String::IsNullOrWhiteSpace(cookedAssetPath))
    {
throw ([&]() {
auto __ctor_arg_00000013 = "Cooked texture asset path must be provided.";
auto __ctor_arg_00000014 = "cookedAssetPath";
return new ArgumentException(__ctor_arg_00000013, __ctor_arg_00000014);
})();
    }
throw new NotSupportedException("This renderer does not support platform-owned cooked texture creation.");
}

void RenderManager2D::Dispose()
{
}

void RenderManager2D::Draw()
{
}

void RenderManager2D::FlushReleasedTextures()
{
}

void RenderManager2D::ReleaseFont(::FontAsset* font)
{
    if (font == nullptr)
    {
throw new ArgumentNullException("font");
    }
::RuntimeTexture *texture = font->Texture;
    if (texture != nullptr && !texture->IsDisposed)
    {
this->ReleaseTexture(texture);
    }
font->Dispose();
delete font;
}

void RenderManager2D::ReleaseTexture(::RuntimeTexture* texture)
{
    if (texture == nullptr)
    {
throw new ArgumentNullException("texture");
    }
if (texture != nullptr)
{
texture->Dispose();
delete texture;
}
}

void RenderManager2D::Update()
{
}

