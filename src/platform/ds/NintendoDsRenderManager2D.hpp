#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <array>
#include <unordered_map>
#include <vector>

#include "IDrawable2D.hpp"
#include "IRenderVisitor2D.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "platform/ds/NintendoDsScreenTarget.hpp"
#include "RoundedRectCorners.hpp"
#include "RenderManager2D.hpp"

class ICamera;
class FontAsset;
class byte4;
class float4;

namespace helengine::ds {
    /// Captures one frame-local Nintendo DS 2D renderer profiling snapshot for native-console diagnostics.
    struct NintendoDsRenderManager2DProfileSnapshot {
        /// Total time spent drawing the current 2D frame, in milliseconds.
        double TotalFrameMilliseconds;

        /// Time spent drawing text primitives during the current frame, in milliseconds.
        double TextMilliseconds;

        /// Time spent drawing sprite primitives during the current frame, in milliseconds.
        double SpriteMilliseconds;

        /// Time spent drawing rounded-rectangle primitives during the current frame, in milliseconds.
        double RoundedRectMilliseconds;

        /// Number of text primitives drawn during the current frame.
        int32_t TextPrimitiveCount;

        /// Number of sprite primitives drawn during the current frame.
        int32_t SpritePrimitiveCount;

        /// Number of rounded-rectangle primitives drawn during the current frame.
        int32_t RoundedRectPrimitiveCount;
    };

    /// Captures one cached opaque rounded-rectangle raster so repeated DS menu buttons can be blitted without re-evaluating geometry every frame.
    struct NintendoDsOpaqueRoundedRectCacheEntry {
        /// Packed DS pixel payload for the cached rounded rectangle.
        std::vector<uint16_t> Pixels;

        /// Inclusive left edge of the visible rounded-rectangle row segment for each local row.
        std::vector<int32_t> RowLeft;

        /// Exclusive right edge of the visible rounded-rectangle row segment for each local row.
        std::vector<int32_t> RowRight;
    };

    /// Captures one cached text bitmap so repeated DS menu labels can be blitted without relayout or glyph-atlas resampling every frame.
    struct NintendoDsCachedTextBitmapEntry {
        /// Width of the cached text bitmap in pixels.
        int32_t Width;

        /// Height of the cached text bitmap in pixels.
        int32_t Height;

        /// Packed RGBA32 pixels for the cached text bitmap.
        std::vector<uint32_t> Pixels;

        /// Inclusive left edge of the visible glyph coverage for each cached row.
        std::vector<int32_t> RowLeft;

        /// Exclusive right edge of the visible glyph coverage for each cached row.
        std::vector<int32_t> RowRight;
    };

    /// Captures one compact opaque rounded-rectangle cache key without per-frame string allocation.
    struct NintendoDsOpaqueRoundedRectCacheKey {
        /// Rounded-rectangle width in pixels.
        int32_t Width;

        /// Rounded-rectangle height in pixels.
        int32_t Height;

        /// Rounded-corner radius in pixels.
        int32_t Radius;

        /// Border thickness in pixels.
        int32_t BorderThickness;

        /// Rounded-corner mask packed as an integer.
        int32_t CornersMask;

        /// Fill color packed into one 32-bit key field.
        uint32_t FillColor;

        /// Border color packed into one 32-bit key field.
        uint32_t BorderColor;

        /// Compares two rounded-rectangle cache keys for equality.
        /// <param name="other">Key to compare.</param>
        /// <returns>True when all key fields match.</returns>
        bool operator==(const NintendoDsOpaqueRoundedRectCacheKey& other) const;
    };

    /// Hashes one opaque rounded-rectangle cache key for unordered-map lookup.
    struct NintendoDsOpaqueRoundedRectCacheKeyHasher {
        /// Computes a stable hash for one opaque rounded-rectangle cache key.
        /// <param name="key">Key to hash.</param>
        /// <returns>Unordered-map hash value.</returns>
        std::size_t operator()(const NintendoDsOpaqueRoundedRectCacheKey& key) const;
    };

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
        /// Resets per-frame screen-clear state before the active camera list is traversed.
        /// </summary>
        void BeginFrame();

        /// <summary>
        /// Draws one camera's ordered 2D queue into the DS screen selected by the authored camera viewport.
        /// </summary>
        /// <param name="camera">Runtime camera owning the ordered 2D queue.</param>
        void DrawCamera(ICamera* camera);

        /// <summary>
        /// Visits one ordered 2D drawable and dispatches it through its generated-core draw entry point.
        /// </summary>
        /// <param name="drawable">Ordered drawable visited from the active camera queue.</param>
        void Visit(IDrawable2D* drawable) override;

        /// <summary>
        /// Draws one rounded rectangle into the DS active screen bitmap framebuffer.
        /// </summary>
        /// <param name="shape">Rounded-rectangle drawable requested by generated core.</param>
        void DrawRoundedRect(IRoundedRectDrawable2D* shape) override;

        /// <summary>
        /// Draws one sprite into the DS active screen bitmap framebuffer.
        /// </summary>
        /// <param name="sprite">Sprite drawable requested by generated core.</param>
        void DrawSprite(ISpriteDrawable2D* sprite) override;

        /// <summary>
        /// Draws one text string into the DS active screen bitmap framebuffer.
        /// </summary>
        /// <param name="text">Text drawable requested by generated core.</param>
        void DrawText(ITextDrawable2D* text) override;

        /// <summary>
        /// Copies the composed CPU-side backbuffers to the visible DS top and bottom bitmap framebuffers.
        /// </summary>
        void PresentFrame();

        /// <summary>
        /// Assigns which physical Nintendo DS screen owns the hardware 3D pass for the active frame.
        /// </summary>
        /// <param name="target">Hardware 3D target screen for the active frame.</param>
        void SetHardware3DScreenTarget(NintendoDsScreenTarget target);

        /// <summary>
        /// Enables or disables presentation of the composed bottom-screen bitmap framebuffer.
        /// </summary>
        /// <param name="enabled">True to present the bottom-screen bitmap framebuffer; otherwise false.</param>
        void SetBottomScreenPresentationEnabled(bool enabled);

        /// <summary>
        /// Gets the latest frame-local 2D renderer profiling snapshot for the native DS diagnostics console.
        /// </summary>
        /// <returns>Current 2D renderer profiling snapshot.</returns>
        NintendoDsRenderManager2DProfileSnapshot get_ProfileSnapshot() const;

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
        /// CPU-side backbuffer used to compose one stable top-screen frame before presenting it to visible VRAM.
        /// </summary>
        std::array<uint16_t, VisibleFrameBufferPixelCount> TopCpuFrameBuffer;

        /// <summary>
        /// CPU-side backbuffer used to compose one stable bottom-screen frame before presenting it to visible VRAM.
        /// </summary>
        std::array<uint16_t, VisibleFrameBufferPixelCount> BottomCpuFrameBuffer;

        /// <summary>
        /// Pointer to the CPU backbuffer currently receiving raster output.
        /// </summary>
        uint16_t* ActiveCpuFrameBuffer;

        /// <summary>
        /// Current X offset applied to rasterized drawables inside the active camera viewport.
        /// </summary>
        int32_t ActiveViewportOffsetX;

        /// <summary>
        /// Current Y offset applied to rasterized drawables inside the active camera viewport.
        /// </summary>
        int32_t ActiveViewportOffsetY;

        /// <summary>
        /// Left clip edge of the active camera viewport.
        /// </summary>
        int32_t ActiveClipLeft;

        /// <summary>
        /// Top clip edge of the active camera viewport.
        /// </summary>
        int32_t ActiveClipTop;

        /// <summary>
        /// Right clip edge of the active camera viewport.
        /// </summary>
        int32_t ActiveClipRight;

        /// <summary>
        /// Bottom clip edge of the active camera viewport.
        /// </summary>
        int32_t ActiveClipBottom;

        /// <summary>
        /// Tracks whether the top screen was cleared during the current frame.
        /// </summary>
        bool TopScreenClearedThisFrame;

        /// <summary>
        /// Tracks whether the bottom screen was cleared during the current frame.
        /// </summary>
        bool BottomScreenClearedThisFrame;

        /// <summary>
        /// Stores which physical Nintendo DS screen currently owns the hardware 3D pass.
        /// </summary>
        NintendoDsScreenTarget Hardware3DScreenTarget;

        /// <summary>
        /// Tracks whether the currently selected viewport targets the bottom Nintendo DS screen.
        /// </summary>
        bool ActiveViewportTargetsBottomScreen;

        /// Stores whether the composed bottom-screen bitmap framebuffer should be copied to visible VRAM.
        bool BottomScreenPresentationEnabled;

        /// Total time spent drawing the current 2D frame, in milliseconds.
        double ProfileTotalFrameMilliseconds;

        /// Time spent drawing text primitives during the current frame, in milliseconds.
        double ProfileTextMilliseconds;

        /// Time spent drawing sprite primitives during the current frame, in milliseconds.
        double ProfileSpriteMilliseconds;

        /// Time spent drawing rounded-rectangle primitives during the current frame, in milliseconds.
        double ProfileRoundedRectMilliseconds;

        /// Number of text primitives drawn during the current frame.
        int32_t ProfileTextPrimitiveCount;

        /// Number of sprite primitives drawn during the current frame.
        int32_t ProfileSpritePrimitiveCount;

        /// Number of rounded-rectangle primitives drawn during the current frame.
        int32_t ProfileRoundedRectPrimitiveCount;

        /// Caches opaque rounded-rectangle button rasters keyed by geometry and color so repeated DS menu rows can be blitted directly.
        std::unordered_map<NintendoDsOpaqueRoundedRectCacheKey, NintendoDsOpaqueRoundedRectCacheEntry, NintendoDsOpaqueRoundedRectCacheKeyHasher> OpaqueRoundedRectCache;

        /// Caches rasterized text bitmaps keyed by content, font, scale, and color so repeated DS menu labels avoid per-frame glyph layout and atlas sampling.
        std::unordered_map<std::string, NintendoDsCachedTextBitmapEntry> TextBitmapCache;

        /// <summary>
        /// Clears one DS screen framebuffer from one runtime camera clear configuration.
        /// </summary>
        /// <param name="camera">Runtime camera providing the clear settings.</param>
        /// <param name="targetBottomScreen">True when the bottom screen should be cleared; otherwise the top screen.</param>
        void ClearScreen(ICamera* camera, bool targetBottomScreen);

        /// <summary>
        /// Resolves one authored camera viewport into Nintendo DS pixel-space bounds.
        /// </summary>
        /// <param name="camera">Runtime camera providing the authored viewport.</param>
        /// <returns>Viewport rectangle expressed in Nintendo DS pixel-space coordinates.</returns>
        float4 ResolveCameraViewport(ICamera* camera) const;

        /// <summary>
        /// Resolves the target screen and clip rectangle for one Nintendo DS camera viewport.
        /// </summary>
        /// <param name="viewport">Viewport rectangle expressed in Nintendo DS pixel-space coordinates.</param>
        /// <param name="targetBottomScreen">Receives whether the viewport targets the bottom screen.</param>
        /// <param name="viewportX">Receives the screen-local viewport X offset.</param>
        /// <param name="viewportY">Receives the screen-local viewport Y offset.</param>
        /// <param name="viewportWidth">Receives the viewport width in pixels.</param>
        /// <param name="viewportHeight">Receives the viewport height in pixels.</param>
        void ResolveViewportTarget(
            const float4& viewport,
            bool& targetBottomScreen,
            int32_t& viewportX,
            int32_t& viewportY,
            int32_t& viewportWidth,
            int32_t& viewportHeight) const;

        /// <summary>
        /// Selects the active backbuffer and clip rectangle used by subsequent raster operations.
        /// </summary>
        /// <param name="targetBottomScreen">True when the bottom-screen backbuffer should become active.</param>
        /// <param name="viewportX">Screen-local viewport X offset.</param>
        /// <param name="viewportY">Screen-local viewport Y offset.</param>
        /// <param name="viewportWidth">Viewport width in pixels.</param>
        /// <param name="viewportHeight">Viewport height in pixels.</param>
        void SelectViewportTarget(
            bool targetBottomScreen,
            int32_t viewportX,
            int32_t viewportY,
            int32_t viewportWidth,
            int32_t viewportHeight);

        /// <summary>
        /// Blends one source pixel into the DS top-screen framebuffer.
        /// </summary>
        /// <param name="x">Destination X coordinate in framebuffer space.</param>
        /// <param name="y">Destination Y coordinate in framebuffer space.</param>
        /// <param name="color">Source RGBA color to blend.</param>
        void BlendPixel(int32_t x, int32_t y, const byte4& color);

        /// <summary>
        /// Writes one fully opaque pixel into the active DS framebuffer without alpha blending.
        /// </summary>
        /// <param name="x">Destination X coordinate in framebuffer space.</param>
        /// <param name="y">Destination Y coordinate in framebuffer space.</param>
        /// <param name="color">Opaque source RGB color to store.</param>
        void WriteOpaquePixel(int32_t x, int32_t y, const byte4& color);

        /// <summary>
        /// Draws one horizontal span into the active DS framebuffer using either opaque or alpha-blended writes.
        /// </summary>
        /// <param name="destX">Destination rectangle X coordinate in framebuffer space.</param>
        /// <param name="destY">Destination row Y coordinate in framebuffer space.</param>
        /// <param name="startX">Inclusive local start X coordinate inside the destination rectangle.</param>
        /// <param name="endX">Exclusive local end X coordinate inside the destination rectangle.</param>
        /// <param name="color">Span color.</param>
        /// <param name="useOpaqueWrite">True to bypass alpha blending; otherwise false.</param>
        void DrawHorizontalSpan(int32_t destX, int32_t destY, int32_t startX, int32_t endX, const byte4& color, bool useOpaqueWrite);

        /// <summary>
        /// Blends one constant-color horizontal span directly against the active framebuffer row without per-pixel clip checks.
        /// </summary>
        /// <param name="destX">Destination rectangle X coordinate in framebuffer space.</param>
        /// <param name="destY">Destination row Y coordinate in framebuffer space.</param>
        /// <param name="startX">Inclusive local start X coordinate inside the destination rectangle.</param>
        /// <param name="endX">Exclusive local end X coordinate inside the destination rectangle.</param>
        /// <param name="color">Constant span color.</param>
        void BlendHorizontalSpan(int32_t destX, int32_t destY, int32_t startX, int32_t endX, const byte4& color);

        /// <summary>
        /// Returns whether one destination rectangle lies fully outside the active viewport clip rectangle.
        /// </summary>
        /// <param name="destX">Destination X coordinate in framebuffer space.</param>
        /// <param name="destY">Destination Y coordinate in framebuffer space.</param>
        /// <param name="width">Destination width in pixels.</param>
        /// <param name="height">Destination height in pixels.</param>
        /// <returns>True when the rectangle does not intersect the active clip rectangle.</returns>
        bool IsDestinationRectOutsideActiveClip(int32_t destX, int32_t destY, int32_t width, int32_t height) const;

        /// <summary>
        /// Attempts to draw one opaque rounded rectangle by blitting a cached raster instead of recomputing row geometry.
        /// </summary>
        /// <param name="destX">Destination X coordinate in framebuffer space.</param>
        /// <param name="destY">Destination Y coordinate in framebuffer space.</param>
        /// <param name="width">Rounded-rectangle width in pixels.</param>
        /// <param name="height">Rounded-rectangle height in pixels.</param>
        /// <param name="radius">Rounded-corner radius in pixels.</param>
        /// <param name="borderThickness">Border thickness in pixels.</param>
        /// <param name="corners">Rounded-corner mask.</param>
        /// <param name="fillColor">Opaque fill color.</param>
        /// <param name="borderColor">Opaque border color.</param>
        /// <returns>True when the rectangle was drawn through the cache path; otherwise false.</returns>
        bool TryRasterCachedOpaqueRoundedRect(
            int32_t destX,
            int32_t destY,
            int32_t width,
            int32_t height,
            int32_t radius,
            int32_t borderThickness,
            RoundedRectCorners corners,
            const byte4& fillColor,
            const byte4& borderColor);

        /// <summary>
        /// Attempts to draw one text drawable through a cached precomposed bitmap instead of per-frame glyph layout and atlas sampling.
        /// </summary>
        /// <param name="text">Text drawable to draw.</param>
        /// <param name="texture">Runtime font atlas texture.</param>
        /// <param name="font">Font asset referenced by the drawable.</param>
        /// <returns>True when the text was drawn through the cache path; otherwise false.</returns>
        bool TryRasterCachedTextBitmap(ITextDrawable2D* text, NintendoDsRuntimeTexture2D* texture, FontAsset* font);

        /// <summary>
        /// Computes the rounded horizontal inset contributed by the enabled corners for one rectangle row.
        /// </summary>
        /// <param name="localY">Row index inside the rectangle.</param>
        /// <param name="width">Rectangle width in pixels.</param>
        /// <param name="height">Rectangle height in pixels.</param>
        /// <param name="radius">Rounded-corner radius in pixels.</param>
        /// <param name="corners">Rounded-corner mask.</param>
        /// <returns>Horizontal inset in pixels for the supplied row.</returns>
        int32_t ComputeRoundedRectRowInset(int32_t localY, int32_t width, int32_t height, int32_t radius, RoundedRectCorners corners) const;

        /// <summary>
        /// Computes one side-specific rounded inset for the supplied rectangle row.
        /// </summary>
        /// <param name="localY">Row index inside the rectangle.</param>
        /// <param name="width">Rectangle width in pixels.</param>
        /// <param name="height">Rectangle height in pixels.</param>
        /// <param name="radius">Rounded-corner radius in pixels.</param>
        /// <param name="corners">Rounded-corner mask.</param>
        /// <param name="leftSide">True to compute the left inset; false for the right inset.</param>
        /// <returns>Horizontal inset in pixels for the requested side.</returns>
        int32_t ComputeRoundedRectSideInset(int32_t localY, int32_t width, int32_t height, int32_t radius, RoundedRectCorners corners, bool leftSide) const;

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
