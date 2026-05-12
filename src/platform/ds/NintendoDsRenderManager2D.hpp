#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "RenderManager2D.hpp"

namespace helengine::ds {
    /// Provides the minimal 2D runtime surface required to initialize generated core on Nintendo DS.
    class NintendoDsRenderManager2D : public RenderManager2D {
    public:
        /// <summary>
        /// Builds one placeholder runtime texture from the authored texture asset.
        /// </summary>
        /// <param name="data">Authored texture asset.</param>
        /// <returns>Placeholder runtime texture carrying the authored metadata.</returns>
        RuntimeTexture* BuildTextureFromRaw(TextureAsset* data) override;

        /// <summary>
        /// Accepts one rounded-rectangle draw request without presenting it.
        /// </summary>
        /// <param name="shape">Rounded-rectangle drawable requested by generated core.</param>
        void DrawRoundedRect(IRoundedRectDrawable2D* shape) override;

        /// <summary>
        /// Accepts one sprite draw request without presenting it.
        /// </summary>
        /// <param name="sprite">Sprite drawable requested by generated core.</param>
        void DrawSprite(ISpriteDrawable2D* sprite) override;

        /// <summary>
        /// Accepts one text draw request without presenting it.
        /// </summary>
        /// <param name="text">Text drawable requested by generated core.</param>
        void DrawText(ITextDrawable2D* text) override;
    };
}
#endif
