#include "platform/ds/NintendoDsRenderManager2D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "RuntimeTexture.hpp"
#include "TextureAsset.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::ds {
    /// Builds one placeholder runtime texture from the authored texture asset.
    /// <param name="data">Authored texture asset.</param>
    /// <returns>Placeholder runtime texture carrying the authored metadata.</returns>
    RuntimeTexture* NintendoDsRenderManager2D::BuildTextureFromRaw(TextureAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        RuntimeTexture* texture = new RuntimeTexture();
        texture->set_Id(data->get_Id());
        texture->set_Width(data->Width);
        texture->set_Height(data->Height);
        return texture;
    }

    /// Accepts one rounded-rectangle draw request without presenting it.
    /// <param name="shape">Rounded-rectangle drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
        (void)shape;
    }

    /// Accepts one sprite draw request without presenting it.
    /// <param name="sprite">Sprite drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
        (void)sprite;
    }

    /// Accepts one text draw request without presenting it.
    /// <param name="text">Text drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawText(ITextDrawable2D* text) {
        (void)text;
    }
}
#endif
