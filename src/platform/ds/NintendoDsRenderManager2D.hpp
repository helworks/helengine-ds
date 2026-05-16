#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <array>

#include "IDrawable2D.hpp"
#include "IRenderVisitor2D.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "RoundedRectCorners.hpp"
#include "RenderManager2D.hpp"

class ICamera;
class byte4;
class float4;

namespace helengine::ds {
    class NintendoDsRuntimeTexture2D;

    /// Provides the Nintendo DS software 2D runtime surface used by the demo-disc menu scene.
    class NintendoDsRenderManager2D : public RenderManager2D, public IRenderVisitor2D {
    public:
        /// <summary>
        /// Creates the Nintendo DS 2D renderer with no active texture-build diagnostics yet.
        /// </summary>
        NintendoDsRenderManager2D();

        /// <summary>
        /// Builds one DS software runtime texture from the authored RGBA texture asset.
        /// </summary>
        /// <param name="data">Authored texture asset.</param>
        /// <returns>DS runtime texture carrying the adopted RGBA pixel payload.</returns>
        RuntimeTexture* BuildTextureFromRaw(TextureAsset* data) override;

        /// <summary>
        /// Releases one DS runtime texture and its adopted pixel payload.
        /// </summary>
        /// <param name="texture">Runtime texture to release.</param>
        void ReleaseTexture(RuntimeTexture* texture) override;

        /// <summary>
        /// Gets the last texture-build stage reached by the DS 2D runtime texture path.
        /// </summary>
        /// <returns>Short texture-build stage label for diagnostics.</returns>
        std::string get_LastTextureBuildStage() const;

        /// <summary>
        /// Gets the last texture asset id observed by the DS 2D runtime texture path.
        /// </summary>
        /// <returns>Texture asset id recorded for diagnostics.</returns>
        std::string get_LastTextureAssetId() const;

        /// <summary>
        /// Gets the last texture width observed by the DS 2D runtime texture path.
        /// </summary>
        /// <returns>Texture width recorded for diagnostics.</returns>
        int32_t get_LastTextureWidth() const;

        /// <summary>
        /// Gets the last texture height observed by the DS 2D runtime texture path.
        /// </summary>
        /// <returns>Texture height recorded for diagnostics.</returns>
        int32_t get_LastTextureHeight() const;

        /// <summary>
        /// Gets the last raw color payload length observed by the DS 2D runtime texture path.
        /// </summary>
        /// <returns>Texture color payload length recorded for diagnostics.</returns>
        int32_t get_LastTextureColorLength() const;

        /// <summary>
        /// Draws one camera's ordered 2D queue into the DS top-screen bitmap framebuffer.
        /// </summary>
        /// <param name="camera">Runtime camera owning the ordered 2D queue.</param>
        void DrawCamera(ICamera* camera);

        /// <summary>
        /// Visits one ordered 2D drawable and dispatches it through its generated-core draw entry point.
        /// </summary>
        /// <param name="drawable">Ordered drawable visited from the active camera queue.</param>
        void Visit(IDrawable2D* drawable) override;

        /// <summary>
        /// Draws one rounded rectangle into the DS top-screen bitmap framebuffer.
        /// </summary>
        /// <param name="shape">Rounded-rectangle drawable requested by generated core.</param>
        void DrawRoundedRect(IRoundedRectDrawable2D* shape) override;

        /// <summary>
        /// Draws one sprite into the DS top-screen bitmap framebuffer.
        /// </summary>
        /// <param name="sprite">Sprite drawable requested by generated core.</param>
        void DrawSprite(ISpriteDrawable2D* sprite) override;

        /// <summary>
        /// Draws one text string into the DS top-screen bitmap framebuffer.
        /// </summary>
        /// <param name="text">Text drawable requested by generated core.</param>
        void DrawText(ITextDrawable2D* text) override;

    private:
        /// <summary>
        /// Width of the DS top-screen bitmap framebuffer in pixels.
        /// </summary>
        static constexpr int32_t FrameBufferWidth = 256;

        /// <summary>
        /// Height of the DS bitmap backing store in pixels.
        /// </summary>
        static constexpr int32_t FrameBufferHeight = 256;

        /// <summary>
        /// Visible DS top-screen height in pixels.
        /// </summary>
        static constexpr int32_t VisibleScreenHeight = 192;

        /// <summary>
        /// Number of visible pixels copied to the top-screen framebuffer each frame.
        /// </summary>
        static constexpr int32_t VisibleFrameBufferPixelCount = FrameBufferWidth * VisibleScreenHeight;

        /// <summary>
        /// Last texture-build stage reached by the DS 2D texture materialization path.
        /// </summary>
        std::string LastTextureBuildStage;

        /// <summary>
        /// Last texture asset id observed by the DS 2D texture materialization path.
        /// </summary>
        std::string LastTextureAssetId;

        /// <summary>
        /// Last texture width observed by the DS 2D texture materialization path.
        /// </summary>
        int32_t LastTextureWidth;

        /// <summary>
        /// Last texture height observed by the DS 2D texture materialization path.
        /// </summary>
        int32_t LastTextureHeight;

        /// <summary>
        /// Last texture color payload length observed by the DS 2D texture materialization path.
        /// </summary>
        int32_t LastTextureColorLength;

        /// <summary>
        /// CPU-side backbuffer used to compose one stable menu frame before presenting it to visible VRAM.
        /// </summary>
        std::array<uint16_t, VisibleFrameBufferPixelCount> CpuFrameBuffer;

        /// <summary>
        /// Clears the DS top-screen framebuffer from one runtime camera clear configuration.
        /// </summary>
        /// <param name="camera">Runtime camera providing the clear settings.</param>
        void ClearCamera(ICamera* camera);

        /// <summary>
        /// Copies the composed CPU-side backbuffer to the visible DS top-screen bitmap framebuffer.
        /// </summary>
        void PresentFrame();

        /// <summary>
        /// Blends one source pixel into the DS top-screen framebuffer.
        /// </summary>
        /// <param name="x">Destination X coordinate in framebuffer space.</param>
        /// <param name="y">Destination Y coordinate in framebuffer space.</param>
        /// <param name="color">Source RGBA color to blend.</param>
        void BlendPixel(int32_t x, int32_t y, const byte4& color);

        /// <summary>
        /// Reads one cooked indexed texel and resolves it through the runtime palette payload.
        /// </summary>
        /// <param name="texture">Runtime texture carrying indexed texel and palette payloads.</param>
        /// <param name="sampleX">Sample X coordinate in texture space.</param>
        /// <param name="sampleY">Sample Y coordinate in texture space.</param>
        /// <returns>Decoded RGBA texel.</returns>
        byte4 ReadIndexedColor(NintendoDsRuntimeTexture2D* texture, int32_t sampleX, int32_t sampleY) const;

        /// <summary>
        /// Rasterizes one textured quad into the DS top-screen framebuffer.
        /// </summary>
        /// <param name="texture">DS software texture carrying the sampled pixel payload.</param>
        /// <param name="sourceRect">Normalized source rectangle inside the texture atlas.</param>
        /// <param name="destX">Destination X coordinate in screen space.</param>
        /// <param name="destY">Destination Y coordinate in screen space.</param>
        /// <param name="destWidth">Destination width in pixels.</param>
        /// <param name="destHeight">Destination height in pixels.</param>
        /// <param name="modulationColor">Per-drawable modulation color applied to sampled texels.</param>
        void RasterTexturedQuad(
            NintendoDsRuntimeTexture2D* texture,
            const float4& sourceRect,
            int32_t destX,
            int32_t destY,
            int32_t destWidth,
            int32_t destHeight,
            const byte4& modulationColor);

        /// <summary>
        /// Rasterizes one rounded rectangle into the DS top-screen framebuffer.
        /// </summary>
        /// <param name="shape">Rounded-rectangle drawable to rasterize.</param>
        void RasterRoundedRect(IRoundedRectDrawable2D* shape);

        /// <summary>
        /// Rasterizes one sprite into the DS top-screen framebuffer.
        /// </summary>
        /// <param name="sprite">Sprite drawable to rasterize.</param>
        void RasterSprite(ISpriteDrawable2D* sprite);

        /// <summary>
        /// Rasterizes one text drawable into the DS top-screen framebuffer.
        /// </summary>
        /// <param name="text">Text drawable to rasterize.</param>
        void RasterText(ITextDrawable2D* text);

        /// <summary>
        /// Tests whether one pixel center lies inside the supplied rounded-rectangle silhouette.
        /// </summary>
        /// <param name="localX">Pixel-local X coordinate inside the rectangle.</param>
        /// <param name="localY">Pixel-local Y coordinate inside the rectangle.</param>
        /// <param name="width">Rectangle width in pixels.</param>
        /// <param name="height">Rectangle height in pixels.</param>
        /// <param name="radius">Rounded-corner radius in pixels.</param>
        /// <param name="corners">Rounded-corner mask.</param>
        /// <returns>True when the pixel center lies inside the silhouette.</returns>
        bool IsPointInsideRoundedRect(
            int32_t localX,
            int32_t localY,
            int32_t width,
            int32_t height,
            int32_t radius,
            RoundedRectCorners corners) const;

        /// <summary>
        /// Resolves whether one rounded-rectangle corner flag is enabled on the supplied mask.
        /// </summary>
        /// <param name="corners">Rounded-corner mask to query.</param>
        /// <param name="corner">Corner flag to test.</param>
        /// <returns>True when the flag is enabled on the mask.</returns>
        bool IsCornerRounded(RoundedRectCorners corners, RoundedRectCorners corner) const;
    };
}
#endif
