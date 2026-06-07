#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <array>
#include <string>
#include <vector>

#include "IDrawable2D.hpp"
#include "IRenderVisitor2D.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "RenderManager2D.hpp"
#include "float4.hpp"
#include "platform/ds/NintendoDsScreenTarget.hpp"

class FontAsset;
class ICamera;
class TextureAsset;

namespace helengine::ds {
    /// Captures one frame-local Nintendo DS 2D renderer profiling snapshot for native-console diagnostics.
    struct NintendoDsRenderManager2DProfileSnapshot {
        /// Total time spent handling the current 2D frame, in milliseconds.
        double TotalFrameMilliseconds;

        /// Time spent handling text primitives during the current frame, in milliseconds.
        double TextMilliseconds;

        /// Time spent handling sprite primitives during the current frame, in milliseconds.
        double SpriteMilliseconds;

        /// Time spent handling rounded-rectangle primitives during the current frame, in milliseconds.
        double RoundedRectMilliseconds;

        /// Reserved clear bucket retained for overlay compatibility, in milliseconds.
        double ClearMilliseconds;

        /// Number of text primitives visited during the current frame.
        int32_t TextPrimitiveCount;

        /// Number of sprite primitives visited during the current frame.
        int32_t SpritePrimitiveCount;

        /// Number of rounded-rectangle primitives visited during the current frame.
        int32_t RoundedRectPrimitiveCount;

        /// Number of unsupported primitives skipped during the current frame.
        int32_t UnsupportedPrimitiveCount;

        /// Number of unsupported text primitives skipped during the current frame.
        int32_t UnsupportedTextPrimitiveCount;

        /// Number of unsupported sprite primitives skipped during the current frame.
        int32_t UnsupportedSpritePrimitiveCount;

        /// Number of unsupported rounded-rectangle primitives skipped during the current frame.
        int32_t UnsupportedRoundedRectPrimitiveCount;
    };

    class NintendoDsRuntimeTexture2D;

    /// Routes Nintendo DS 2D drawables only through real DS hardware paths and skips unsupported work.
    class NintendoDsRenderManager2D : public RenderManager2D, public IRenderVisitor2D {
    public:
        /// <summary>
        /// Creates the Nintendo DS 2D renderer with no active texture-build diagnostics yet.
        /// </summary>
        NintendoDsRenderManager2D();

        /// <summary>
        /// Builds one DS runtime texture from the authored texture asset.
        /// </summary>
        /// <param name="data">Authored texture asset.</param>
        /// <returns>DS runtime texture carrying the adopted cooked pixel payload.</returns>
        RuntimeTexture* BuildTextureFromRaw(TextureAsset* data) override;

        /// <summary>
        /// Builds one DS runtime texture from one builder-owned cooked texture payload serialized on disk.
        /// </summary>
        /// <param name="cookedAssetPath">Absolute NitroFS or host path to the serialized cooked texture asset.</param>
        /// <returns>DS runtime texture carrying the adopted cooked pixel payload.</returns>
        RuntimeTexture* BuildTextureFromCooked(std::string cookedAssetPath) override;

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
        /// Resets per-frame diagnostic state before the active camera list is traversed.
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
        /// Draws one rounded rectangle when the backend can map it to hardware, otherwise skips it.
        /// </summary>
        /// <param name="shape">Rounded-rectangle drawable requested by generated core.</param>
        void DrawRoundedRect(IRoundedRectDrawable2D* shape) override;

        /// <summary>
        /// Draws one sprite when the backend can map it to hardware, otherwise skips it.
        /// </summary>
        /// <param name="sprite">Sprite drawable requested by generated core.</param>
        void DrawSprite(ISpriteDrawable2D* sprite) override;

        /// <summary>
        /// Draws one text primitive when the backend can map it to hardware, otherwise skips it.
        /// </summary>
        /// <param name="text">Text drawable requested by generated core.</param>
        void DrawText(ITextDrawable2D* text) override;

        /// <summary>
        /// Assigns which physical Nintendo DS screen owns the hardware 3D pass for the active frame.
        /// </summary>
        /// <param name="target">Screen that should keep hardware 3D ownership.</param>
        void SetHardware3DScreenTarget(NintendoDsScreenTarget target);

        /// <summary>
        /// Stores whether the bottom DS screen should remain available for visible presentation.
        /// </summary>
        /// <param name="enabled">True when the bottom screen should stay visible.</param>
        void SetBottomScreenPresentationEnabled(bool enabled);

        /// <summary>
        /// Gets whether the bottom DS screen should remain visible for the active frame.
        /// </summary>
        /// <returns>True when the bottom screen remains visible.</returns>
        bool get_BottomScreenPresentationEnabled() const;

        /// <summary>
        /// Stores the latest runtime heartbeat frame consumed by boot-time diagnostics.
        /// </summary>
        /// <param name="frameIndex">Heartbeat frame index published by the boot host.</param>
        void SetRuntimeHeartbeatFrame(int32_t frameIndex);

        /// <summary>
        /// Gets the latest 2D profile snapshot for debug overlay diagnostics.
        /// </summary>
        /// <returns>Frame-local 2D profile snapshot.</returns>
        NintendoDsRenderManager2DProfileSnapshot get_ProfileSnapshot() const;

    private:
        /// <summary>
        /// Width of the DS framebuffer in pixels.
        /// </summary>
        static constexpr int32_t FrameBufferWidth = 256;

        /// <summary>
        /// Height of one visible DS screen in pixels.
        /// </summary>
        static constexpr int32_t VisibleScreenHeight = 192;

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
        /// Tracks the active viewport X offset in screen-local pixels.
        /// </summary>
        int32_t ActiveViewportOffsetX;

        /// <summary>
        /// Tracks the active viewport Y offset in screen-local pixels.
        /// </summary>
        int32_t ActiveViewportOffsetY;

        /// <summary>
        /// Tracks the active viewport left clip edge.
        /// </summary>
        int32_t ActiveClipLeft;

        /// <summary>
        /// Tracks the active viewport top clip edge.
        /// </summary>
        int32_t ActiveClipTop;

        /// <summary>
        /// Tracks the active viewport right clip edge.
        /// </summary>
        int32_t ActiveClipRight;

        /// <summary>
        /// Tracks the active viewport bottom clip edge.
        /// </summary>
        int32_t ActiveClipBottom;

        /// <summary>
        /// Stores which physical Nintendo DS screen currently owns the hardware 3D pass.
        /// </summary>
        NintendoDsScreenTarget Hardware3DScreenTarget;

        /// <summary>
        /// Tracks whether the active viewport targets the bottom Nintendo DS screen.
        /// </summary>
        bool ActiveViewportTargetsBottomScreen;

        /// <summary>
        /// Stores whether the bottom DS screen should remain available for visible presentation.
        /// </summary>
        bool BottomScreenPresentationEnabled;

        /// <summary>
        /// Stores the most recent runtime heartbeat frame requested by the Nintendo DS boot host.
        /// </summary>
        int32_t RuntimeHeartbeatFrameIndex;

        /// <summary>
        /// Stores the bottom-screen text background id used by direct DS tile-map text submission.
        /// </summary>
        int32_t BottomScreenTextBackgroundId;

        /// <summary>
        /// Stores the writable tile-map pointer for the bottom-screen text background.
        /// </summary>
        uint16_t* BottomScreenTextMapEntries;

        /// <summary>
        /// Stores the renderer-owned shadow copy of the bottom-screen visible text map.
        /// </summary>
        std::array<uint16_t, 32 * 24> BottomScreenTextShadowEntries;

        /// <summary>
        /// Tracks whether the bottom-screen text background has already been initialized for runtime text submission.
        /// </summary>
        bool BottomScreenTextBackgroundInitialized;

        /// <summary>
        /// Stores the font asset whose glyphs are currently cached into the bottom-screen DS text background tiles.
        /// </summary>
        FontAsset* BottomScreenTextGlyphCacheFont;

        /// <summary>
        /// Stores the uploaded DS text-background tile index for each printable ASCII character slot.
        /// </summary>
        std::array<uint16_t, 95> BottomScreenTextGlyphTileIndices;

        /// <summary>
        /// Tracks whether the current font glyph tiles have already been uploaded into DS background character memory.
        /// </summary>
        bool BottomScreenTextGlyphTilesUploaded;

        /// <summary>
        /// Stores the next sprite slot reserved for debug unsupported-draw markers on the main engine.
        /// </summary>
        int32_t NextMainDebugMarkerSpriteId;

        /// <summary>
        /// Stores the next sprite slot reserved for debug unsupported-draw markers on the sub engine.
        /// </summary>
        int32_t NextSubDebugMarkerSpriteId;

        /// <summary>
        /// Stores whether the main-engine unsupported-draw marker resources have been initialized.
        /// </summary>
        bool MainDebugMarkerInitialized;

        /// <summary>
        /// Stores whether the main-engine sprite hardware state has been initialized for runtime sprite submission.
        /// </summary>
        bool MainSpriteEngineInitialized;

        /// <summary>
        /// Stores whether the sub-engine sprite hardware state has been initialized for runtime sprite submission.
        /// </summary>
        bool SubSpriteEngineInitialized;

        /// <summary>
        /// Stores whether the sub-engine unsupported-draw marker resources have been initialized.
        /// </summary>
        bool SubDebugMarkerInitialized;

        /// <summary>
        /// Stores the shared main-engine sprite graphics payload for unsupported-draw markers.
        /// </summary>
        void* MainDebugMarkerGfx;

        /// <summary>
        /// Stores the shared sub-engine sprite graphics payload for unsupported-draw markers.
        /// </summary>
        void* SubDebugMarkerGfx;

        /// <summary>
        /// True once one unsupported sprite diagnostic has already been logged during the active frame.
        /// </summary>
        bool UnsupportedSpriteLoggedThisFrame;

        /// <summary>
        /// True once one unsupported text diagnostic has already been logged during the active frame.
        /// </summary>
        bool UnsupportedTextLoggedThisFrame;

        /// <summary>
        /// True once one unsupported rounded-rectangle diagnostic has already been logged during the active frame.
        /// </summary>
        bool UnsupportedRoundedRectLoggedThisFrame;

        /// <summary>
        /// Number of unsupported text trace lines already emitted during the active frame.
        /// </summary>
        int32_t UnsupportedTextTraceCountThisFrame;

        /// <summary>
        /// Number of unsupported sprite trace lines already emitted during the active frame.
        /// </summary>
        int32_t UnsupportedSpriteTraceCountThisFrame;

        /// <summary>
        /// Total time spent handling the current 2D frame, in milliseconds.
        /// </summary>
        double ProfileTotalFrameMilliseconds;

        /// <summary>
        /// Time spent handling text primitives during the current frame, in milliseconds.
        /// </summary>
        double ProfileTextMilliseconds;

        /// <summary>
        /// Time spent handling sprite primitives during the current frame, in milliseconds.
        /// </summary>
        double ProfileSpriteMilliseconds;

        /// <summary>
        /// Time spent handling rounded-rectangle primitives during the current frame, in milliseconds.
        /// </summary>
        double ProfileRoundedRectMilliseconds;

        /// <summary>
        /// Reserved clear bucket retained for overlay compatibility, in milliseconds.
        /// </summary>
        double ProfileClearMilliseconds;

        /// <summary>
        /// Number of text primitives visited during the current frame.
        /// </summary>
        int32_t ProfileTextPrimitiveCount;

        /// <summary>
        /// Number of sprite primitives visited during the current frame.
        /// </summary>
        int32_t ProfileSpritePrimitiveCount;

        /// <summary>
        /// Number of rounded-rectangle primitives visited during the current frame.
        /// </summary>
        int32_t ProfileRoundedRectPrimitiveCount;

        /// <summary>
        /// Number of unsupported primitives skipped during the current frame.
        /// </summary>
        int32_t ProfileUnsupportedPrimitiveCount;

        /// <summary>
        /// Number of unsupported text primitives skipped during the current frame.
        /// </summary>
        int32_t ProfileUnsupportedTextPrimitiveCount;

        /// <summary>
        /// Number of unsupported sprite primitives skipped during the current frame.
        /// </summary>
        int32_t ProfileUnsupportedSpritePrimitiveCount;

        /// <summary>
        /// Number of unsupported rounded-rectangle primitives skipped during the current frame.
        /// </summary>
        int32_t ProfileUnsupportedRoundedRectPrimitiveCount;

        /// <summary>
        /// Stores the most recent net allocator delta observed while releasing one runtime texture.
        /// </summary>
        int32_t LastReleaseTextureNetByteDelta;

        /// <summary>
        /// Stores the most recent net allocator delta observed while releasing one font asset.
        /// </summary>
        int32_t LastReleaseFontNetByteDelta;

        /// <summary>
        /// Releases one font asset and records allocator diagnostics.
        /// </summary>
        /// <param name="font">Font asset to release.</param>
        void ReleaseFont(FontAsset* font);

        /// <summary>
        /// Flushes any DS-owned texture payloads that the runtime marked for deferred release.
        /// </summary>
        void FlushReleasedTextures();

        /// <summary>
        /// Resolves the active camera viewport into Nintendo DS pixel coordinates.
        /// </summary>
        /// <param name="camera">Camera whose viewport should be resolved.</param>
        /// <returns>Viewport rectangle in DS pixel coordinates.</returns>
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
        void ResolveViewportTarget(const float4& viewport, bool& targetBottomScreen, int32_t& viewportX, int32_t& viewportY, int32_t& viewportWidth, int32_t& viewportHeight) const;

        /// <summary>
        /// Selects the active viewport routing used by subsequent draw calls.
        /// </summary>
        /// <param name="targetBottomScreen">True when the bottom screen should become active.</param>
        /// <param name="viewportX">Screen-local viewport X offset.</param>
        /// <param name="viewportY">Screen-local viewport Y offset.</param>
        /// <param name="viewportWidth">Viewport width in pixels.</param>
        /// <param name="viewportHeight">Viewport height in pixels.</param>
        void SelectViewportTarget(bool targetBottomScreen, int32_t viewportX, int32_t viewportY, int32_t viewportWidth, int32_t viewportHeight);

        /// <summary>
        /// Attempts to submit one sprite drawable through a DS hardware-backed path.
        /// </summary>
        /// <param name="sprite">Sprite drawable to evaluate.</param>
        /// <returns>True when the sprite was submitted to DS hardware.</returns>
        bool TryDrawHardwareSprite(ISpriteDrawable2D* sprite);

        /// <summary>
        /// Ensures one runtime texture has prepared DS OBJ graphics for the first-pass sprite path.
        /// </summary>
        /// <param name="runtimeTexture">Runtime texture that may own cached DS OBJ graphics.</param>
        /// <returns>True when the runtime texture is ready for first-pass sprite submission.</returns>
        bool TryPrepareHardwareSpriteGraphics(NintendoDsRuntimeTexture2D* runtimeTexture);

        /// <summary>
        /// Checks whether one runtime texture uses a format accepted by the first-pass DS sprite path.
        /// </summary>
        /// <param name="runtimeTexture">Runtime texture to inspect.</param>
        /// <returns>True when the texture format is accepted by the first-pass sprite path.</returns>
        bool IsHardwareSpriteFormatSupported(NintendoDsRuntimeTexture2D* runtimeTexture) const;

        /// <summary>
        /// Checks whether one authored sprite size fits inside one first-pass DS OBJ shape.
        /// </summary>
        /// <param name="drawableSize">Authored sprite size requested by generated core.</param>
        /// <returns>True when the size fits one first-pass DS OBJ shape.</returns>
        bool IsSupportedHardwareSpriteSize(const int2& drawableSize) const;

        /// <summary>
        /// Expands one authored sprite dimension into a minimal set of DS OBJ tile spans.
        /// </summary>
        /// <param name="length">Authored sprite width or height in pixels.</param>
        /// <param name="spans">Receives the DS OBJ tile spans that cover the authored dimension.</param>
        void BuildHardwareSpriteTileSpans(int32_t length, std::vector<int32_t>& spans) const;

        /// <summary>
        /// Builds one temporary DS bitmap-sprite pixel payload from the cooked runtime texture.
        /// </summary>
        /// <param name="runtimeTexture">Runtime texture carrying the cooked source texel payload.</param>
        /// <returns>Direct-color DS sprite pixels in row-major order.</returns>
        std::vector<uint16_t> BuildHardwareSpritePixels(NintendoDsRuntimeTexture2D* runtimeTexture) const;

        /// <summary>
        /// Builds one padded DS OBJ tile payload copied from one authored sprite texture region.
        /// </summary>
        /// <param name="sourcePixels">Decoded authored sprite pixels in row-major order.</param>
        /// <param name="sourceWidth">Authored source texture width in pixels.</param>
        /// <param name="sourceHeight">Authored source texture height in pixels.</param>
        /// <param name="tileOriginX">Source pixel X offset for the tile copy.</param>
        /// <param name="tileOriginY">Source pixel Y offset for the tile copy.</param>
        /// <param name="tileWidth">Prepared DS OBJ tile width in pixels.</param>
        /// <param name="tileHeight">Prepared DS OBJ tile height in pixels.</param>
        /// <returns>Padded DS OBJ tile pixels in row-major order.</returns>
        std::vector<uint16_t> BuildHardwareSpriteTilePixels(const std::vector<uint16_t>& sourcePixels, int32_t sourceWidth, int32_t sourceHeight, int32_t tileOriginX, int32_t tileOriginY, int32_t tileWidth, int32_t tileHeight) const;

        /// <summary>
        /// Attempts to submit one text drawable through a DS hardware-backed path.
        /// </summary>
        /// <param name="text">Text drawable to evaluate.</param>
        /// <returns>True when the text was submitted to DS hardware.</returns>
        bool TryDrawHardwareText(ITextDrawable2D* text);

        /// <summary>
        /// Ensures the bottom-screen DS text background exists for direct tile-map text submission.
        /// </summary>
        void EnsureBottomScreenTextBackgroundReady();

        /// <summary>
        /// Clears the bottom-screen DS text background map through the renderer-owned shadow state.
        /// </summary>
        void ClearBottomScreenTextMap();

        /// <summary>
        /// Ensures the active font has uploaded glyph tiles ready for bottom-screen DS text-background submission.
        /// </summary>
        /// <param name="font">Font whose cooked glyph atlas should back the bottom-screen text background.</param>
        void EnsureBottomScreenFontGlyphTilesReady(FontAsset* font);

        /// <summary>
        /// Resolves one printable character into the uploaded DS text-background tile index for the active font.
        /// </summary>
        /// <param name="font">Font whose uploaded glyph cache should be queried.</param>
        /// <param name="character">Printable character to map.</param>
        /// <param name="tileIndex">Receives the uploaded tile index when the glyph is available.</param>
        /// <returns>True when the glyph was uploaded and can be referenced from the text background map.</returns>
        bool TryResolveBottomScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex);

        /// <summary>
        /// Writes one text line into the bottom-screen DS text background at the requested cell position.
        /// </summary>
        /// <param name="row">Zero-based text row.</param>
        /// <param name="column">Zero-based text column.</param>
        /// <param name="line">Visible line content to write.</param>
        /// <param name="visibleColumnCount">Number of writable columns in the row segment.</param>
        void WriteBottomScreenTextLine(int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount);

        /// <summary>
        /// Resolves one printable ASCII character into the DS text-background glyph tile index.
        /// </summary>
        /// <param name="character">Printable character to map.</param>
        /// <returns>Glyph tile index or zero for blank/unsupported characters.</returns>
        uint16_t ResolveBottomScreenGlyphTileIndex(char character) const;

        /// <summary>
        /// Resolves the console start column for one aligned text run inside its authored text box.
        /// </summary>
        /// <param name="baseColumn">Left-edge console column derived from the drawable position.</param>
        /// <param name="boxColumnCount">Width of the authored text box expressed in console columns.</param>
        /// <param name="visibleLength">Visible text length expressed in console columns.</param>
        /// <param name="alignment">Generated-core text alignment value.</param>
        /// <returns>Console start column clamped to the visible DS text grid.</returns>
        int32_t ResolveAlignedConsoleColumn(int32_t baseColumn, int32_t boxColumnCount, int32_t visibleLength, int32_t alignment) const;

        /// <summary>
        /// Emits one debug-only host trace for one unsupported text drawable without touching the DS console.
        /// </summary>
        /// <param name="text">Text drawable that could not be expressed through DS hardware.</param>
        /// <param name="reason">Short reject reason label.</param>
        void TraceUnsupportedTextDrawable(ITextDrawable2D* text, const char* reason);

        /// <summary>
        /// Emits one debug-only host trace for one unsupported sprite drawable without touching the DS console.
        /// </summary>
        /// <param name="sprite">Sprite drawable that could not be expressed through DS hardware.</param>
        /// <param name="reason">Short reject reason label.</param>
        void TraceUnsupportedSpriteDrawable(ISpriteDrawable2D* sprite, const char* reason);

        /// <summary>
        /// Emits one debug-only unsupported-draw diagnostic without changing runtime fallback behavior.
        /// </summary>
        /// <param name="category">Short unsupported category label.</param>
        /// <param name="drawable">Drawable that could not be expressed through DS hardware.</param>
        void LogUnsupportedDrawable(const char* category, IDrawable2D* drawable);

        /// <summary>
        /// Resolves the screen-space anchor used by unsupported-draw markers for one drawable.
        /// </summary>
        /// <param name="drawable">Drawable that could not be expressed through DS hardware.</param>
        /// <returns>Best-effort screen-space anchor for the diagnostic marker.</returns>
        int2 ResolveUnsupportedDrawableMarkerPosition(IDrawable2D* drawable) const;

        /// <summary>
        /// Draws one debug-only magenta marker through DS sprite hardware for unsupported drawables.
        /// </summary>
        /// <param name="x">Marker X coordinate in screen-local pixels.</param>
        /// <param name="y">Marker Y coordinate in screen-local pixels.</param>
        /// <param name="targetScreen">Physical DS screen that should show the marker.</param>
        void DrawUnsupportedDrawableMarker(int32_t x, int32_t y, NintendoDsScreenTarget targetScreen);

        /// <summary>
        /// Ensures the DS sprite resources used by unsupported-draw markers exist for the requested screen.
        /// </summary>
        /// <param name="targetScreen">Physical DS screen that will own the marker sprite resources.</param>
        void EnsureUnsupportedMarkerResources(NintendoDsScreenTarget targetScreen);

    };
}
#endif
