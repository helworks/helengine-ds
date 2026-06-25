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
class byte4;

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
        /// Copies the composed bottom-screen software bitmap framebuffer to visible Nintendo DS sub-screen VRAM.
        /// </summary>
        void PresentBottomScreenFrame();

        /// <summary>
        /// Assigns which physical Nintendo DS screen owns the hardware 3D pass for the active frame.
        /// </summary>
        /// <param name="target">Screen that should keep hardware 3D ownership.</param>
        void SetHardware3DScreenTarget(NintendoDsScreenTarget target);

        /// <summary>
        /// Stores the current frame's top and bottom 2D queue counts so bottom-screen diagnostics can expose menu traversal state.
        /// </summary>
        /// <param name="topScreenQueueCount">Top-screen 2D queue count resolved for the current frame.</param>
        /// <param name="bottomScreenQueueCount">Bottom-screen 2D queue count resolved for the current frame.</param>
        void SetFrameQueueCounts(int32_t topScreenQueueCount, int32_t bottomScreenQueueCount);

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
        /// Gets whether the temporary DS proof mode that forces the top screen into BG0 and OBJ validation is active.
        /// </summary>
        /// <returns>True when the top-screen proof mode should suppress main-engine 3D routing.</returns>
        bool get_TopScreenProofModeActive() const;

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

        /// <summary>
        /// Gets the number of bottom-screen text primitives that reached the DS hardware path during the latest frame.
        /// </summary>
        /// <returns>Bottom-screen text primitive submission count.</returns>
        int32_t get_LastBottomScreenSubmittedTextCount() const;

        /// <summary>
        /// Gets the number of bottom-screen text primitives rejected by the DS hardware path during the latest frame.
        /// </summary>
        /// <returns>Bottom-screen text rejection count.</returns>
        int32_t get_LastBottomScreenUnsupportedTextCount() const;

        /// <summary>
        /// Gets the first bottom-screen text reject reason recorded during the latest frame.
        /// </summary>
        /// <returns>Bottom-screen text reject reason label, or an empty string when no rejection occurred.</returns>
        std::string get_LastBottomScreenUnsupportedTextReason() const;

        /// <summary>
        /// Gets the first bottom-screen rejected text content recorded during the latest frame.
        /// </summary>
        /// <returns>Bottom-screen rejected text content, or an empty string when no rejection occurred.</returns>
        std::string get_LastBottomScreenUnsupportedTextSample() const;

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
        /// Number of visible pixels stored in one bottom-screen software framebuffer.
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
        /// CPU-side backbuffer used to compose one stable bottom-screen frame before presenting it to visible VRAM.
        /// </summary>
        std::array<uint16_t, VisibleFrameBufferPixelCount> BottomCpuFrameBuffer;

        /// <summary>
        /// Pointer to the active software backbuffer receiving bottom-screen raster output.
        /// </summary>
        uint16_t* ActiveCpuFrameBuffer;

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
        /// Tracks whether the bottom-screen software framebuffer has already been cleared during the active frame.
        /// </summary>
        bool BottomScreenClearedThisFrame;

        /// <summary>
        /// Stores the most recent runtime heartbeat frame requested by the Nintendo DS boot host.
        /// </summary>
        int32_t RuntimeHeartbeatFrameIndex;

        /// <summary>
        /// Stores the bottom-screen text background id used by direct DS tile-map text submission.
        /// </summary>
        int32_t BottomScreenTextBackgroundId;

        /// <summary>
        /// Stores the top-screen text background id used by direct DS tile-map text submission.
        /// </summary>
        int32_t TopScreenTextBackgroundId;

        /// <summary>
        /// Stores the writable tile-map pointer for the bottom-screen text background.
        /// </summary>
        uint16_t* BottomScreenTextMapEntries;

        /// <summary>
        /// Stores the writable tile-map pointer for the top-screen text background.
        /// </summary>
        uint16_t* TopScreenTextMapEntries;

        /// <summary>
        /// Stores the renderer-owned shadow copy of the bottom-screen visible text map.
        /// </summary>
        std::array<uint16_t, 32 * 24> BottomScreenTextShadowEntries;

        /// <summary>
        /// Stores the renderer-owned shadow copy of the top-screen visible text map.
        /// </summary>
        std::array<uint16_t, 32 * 24> TopScreenTextShadowEntries;

        /// <summary>
        /// Tracks whether the bottom-screen text background has already been initialized for runtime text submission.
        /// </summary>
        bool BottomScreenTextBackgroundInitialized;

        /// <summary>
        /// Tracks whether the top-screen text background has already been initialized for runtime text submission.
        /// </summary>
        bool TopScreenTextBackgroundInitialized;

        /// <summary>
        /// Tracks whether the handwritten BG0 proof text has already been written into the visible tile map.
        /// </summary>
        bool BottomScreenProofTextInitialized;

        /// <summary>
        /// Stores the font asset whose glyphs are currently cached into the bottom-screen DS text background tiles.
        /// </summary>
        FontAsset* BottomScreenTextGlyphCacheFont;

        /// <summary>
        /// Stores the font asset whose glyphs are currently cached into the top-screen DS text background tiles.
        /// </summary>
        FontAsset* TopScreenTextGlyphCacheFont;

        /// <summary>
        /// Stores the uploaded DS text-background tile index for each printable ASCII character slot.
        /// </summary>
        std::array<uint16_t, 95> BottomScreenTextGlyphTileIndices;

        /// <summary>
        /// Stores the uploaded DS text-background tile index for each printable ASCII character slot on the top screen.
        /// </summary>
        std::array<uint16_t, 95> TopScreenTextGlyphTileIndices;

        /// <summary>
        /// Tracks whether the current font glyph tiles have already been uploaded into DS background character memory.
        /// </summary>
        bool BottomScreenTextGlyphTilesUploaded;

        /// <summary>
        /// Tracks whether the current top-screen font glyph tiles have already been uploaded into DS background character memory.
        /// </summary>
        bool TopScreenTextGlyphTilesUploaded;

        /// <summary>
        /// Stores the most recent detailed glyph-cache failure reason encountered while resolving bottom-screen text.
        /// </summary>
        std::string BottomScreenGlyphResolveFailureReason;

        /// <summary>
        /// Stores the most recent detailed glyph-cache failure reason encountered while resolving top-screen text.
        /// </summary>
        std::string TopScreenGlyphResolveFailureReason;

        /// <summary>
        /// Stores the next sprite slot reserved for debug unsupported-draw markers on the main engine.
        /// </summary>
        int32_t NextMainDebugMarkerSpriteId;

        /// <summary>
        /// Stores the next sprite slot reserved for debug unsupported-draw markers on the sub engine.
        /// </summary>
        int32_t NextSubDebugMarkerSpriteId;

        /// <summary>
        /// Stores the next top-screen 16-color sprite palette bank reserved for runtime sprite submission.
        /// </summary>
        int32_t NextMainSpritePaletteBank;

        /// <summary>
        /// Stores the next bottom-screen 16-color sprite palette bank reserved for runtime sprite submission.
        /// </summary>
        int32_t NextSubSpritePaletteBank;

        /// <summary>
        /// Stores the DS palette color assigned to each top-screen solid-rectangle sprite palette bank, or <c>0xFFFF</c> when unused.
        /// </summary>
        std::array<uint16_t, 16> MainSolidRectanglePaletteColors;

        /// <summary>
        /// Stores the DS palette color assigned to each bottom-screen solid-rectangle sprite palette bank, or <c>0xFFFF</c> when unused.
        /// </summary>
        std::array<uint16_t, 16> SubSolidRectanglePaletteColors;

        /// <summary>
        /// Tracks frame-local top-screen OBJ graphics allocations created for plain rectangle rendering.
        /// </summary>
        std::vector<void*> FrameLocalMainRectangleGraphics;

        /// <summary>
        /// Tracks frame-local bottom-screen OBJ graphics allocations created for plain rectangle rendering.
        /// </summary>
        std::vector<void*> FrameLocalSubRectangleGraphics;

        /// <summary>
        /// Stores whether the main-engine unsupported-draw marker resources have been initialized.
        /// </summary>
        bool MainDebugMarkerInitialized;

        /// <summary>
        /// Stores whether the dedicated top-screen proof OBJ resources have been initialized.
        /// </summary>
        bool TopScreenProofSpriteInitialized;

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
        /// Stores the shared main-engine sprite graphics payload used by the dedicated top-screen proof OBJ.
        /// </summary>
        void* TopScreenProofSpriteGfx;

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
        /// Number of bottom-screen text primitives that reached the DS hardware path during the active frame.
        /// </summary>
        int32_t BottomScreenSubmittedTextCountThisFrame;

        /// <summary>
        /// Top-screen 2D queue count recorded for the active frame so bottom-screen diagnostics can surface menu traversal state.
        /// </summary>
        int32_t LastTopScreenQueueCount;

        /// <summary>
        /// Bottom-screen 2D queue count recorded for the active frame so bottom-screen diagnostics can surface menu traversal state.
        /// </summary>
        int32_t LastBottomScreenQueueCount;

        /// <summary>
        /// Number of bottom-screen text primitives rejected by the DS hardware path during the active frame.
        /// </summary>
        int32_t BottomScreenUnsupportedTextCountThisFrame;

        /// <summary>
        /// First bottom-screen text reject reason recorded during the active frame.
        /// </summary>
        std::string BottomScreenUnsupportedTextReasonThisFrame;

        /// <summary>
        /// First bottom-screen rejected text content recorded during the active frame.
        /// </summary>
        std::string BottomScreenUnsupportedTextSampleThisFrame;

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
        /// Clears one software-composed Nintendo DS screen framebuffer from one runtime camera clear configuration.
        /// </summary>
        /// <param name="camera">Runtime camera providing the clear settings.</param>
        /// <param name="targetBottomScreen">True when the bottom screen should be cleared; otherwise false.</param>
        void ClearScreen(ICamera* camera, bool targetBottomScreen);

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
        /// Blends one source pixel into the active bottom-screen software framebuffer.
        /// </summary>
        /// <param name="x">Destination X coordinate in framebuffer space.</param>
        /// <param name="y">Destination Y coordinate in framebuffer space.</param>
        /// <param name="color">Source RGBA color to blend.</param>
        void BlendPixel(int32_t x, int32_t y, const byte4& color);

        /// <summary>
        /// Writes one fully opaque pixel into the active bottom-screen software framebuffer without alpha blending.
        /// </summary>
        /// <param name="x">Destination X coordinate in framebuffer space.</param>
        /// <param name="y">Destination Y coordinate in framebuffer space.</param>
        /// <param name="color">Opaque source RGB color to store.</param>
        void WriteOpaquePixel(int32_t x, int32_t y, const byte4& color);

        /// <summary>
        /// Decodes one indexed runtime-texture sample into the shared byte4 color representation.
        /// </summary>
        /// <param name="texture">Runtime texture containing the sampled pixel payload.</param>
        /// <param name="sampleX">Texture-space X coordinate.</param>
        /// <param name="sampleY">Texture-space Y coordinate.</param>
        /// <returns>Decoded RGBA color.</returns>
        byte4 ReadIndexedColor(NintendoDsRuntimeTexture2D* texture, int32_t sampleX, int32_t sampleY) const;

        /// <summary>
        /// Rasterizes one textured quad into the active bottom-screen software framebuffer.
        /// </summary>
        /// <param name="texture">Runtime texture providing sampled texels.</param>
        /// <param name="sourceRect">Normalized source rectangle inside the texture atlas.</param>
        /// <param name="destX">Destination X coordinate in screen space.</param>
        /// <param name="destY">Destination Y coordinate in screen space.</param>
        /// <param name="destWidth">Destination width in pixels.</param>
        /// <param name="destHeight">Destination height in pixels.</param>
        /// <param name="modulationColor">Per-drawable modulation color.</param>
        void RasterTexturedQuad(NintendoDsRuntimeTexture2D* texture, const float4& sourceRect, int32_t destX, int32_t destY, int32_t destWidth, int32_t destHeight, const byte4& modulationColor);

        /// <summary>
        /// Rasterizes one sprite drawable into the bottom-screen software framebuffer.
        /// </summary>
        /// <param name="sprite">Sprite drawable to rasterize.</param>
        void RasterSprite(ISpriteDrawable2D* sprite);

        /// <summary>
        /// Rasterizes one text drawable into the bottom-screen software framebuffer.
        /// </summary>
        /// <param name="text">Text drawable to rasterize.</param>
        void RasterText(ITextDrawable2D* text);

        /// <summary>
        /// Attempts to submit one rounded-rectangle drawable through the DS plain-rectangle hardware path.
        /// </summary>
        /// <param name="shape">Rounded-rectangle drawable to evaluate.</param>
        /// <returns>True when the drawable was submitted as one plain hardware rectangle.</returns>
        bool TryDrawHardwareRectangle(IRoundedRectDrawable2D* shape);

        /// <summary>
        /// Attempts to submit one solid-color rectangle through DS paletted OBJ tiles.
        /// </summary>
        /// <param name="x">Screen-local left coordinate in pixels.</param>
        /// <param name="y">Screen-local top coordinate in pixels.</param>
        /// <param name="width">Rectangle width in pixels.</param>
        /// <param name="height">Rectangle height in pixels.</param>
        /// <param name="color">Opaque rectangle color.</param>
        /// <returns>True when the rectangle was submitted through DS hardware sprites.</returns>
        bool TryDrawSolidHardwareRectangle(int32_t x, int32_t y, int32_t width, int32_t height, const byte4& color);

        /// <summary>
        /// Releases any frame-local DS OBJ graphics allocations created for plain rectangle rendering.
        /// </summary>
        void ReleaseFrameLocalRectangleGraphics();

        /// <summary>
        /// Builds one tiled 4bpp DS OBJ payload for a clipped solid rectangle tile.
        /// </summary>
        /// <param name="tileWidth">Prepared DS OBJ tile width in pixels.</param>
        /// <param name="tileHeight">Prepared DS OBJ tile height in pixels.</param>
        /// <param name="filledWidth">Visible filled width inside the tile in pixels.</param>
        /// <param name="filledHeight">Visible filled height inside the tile in pixels.</param>
        /// <param name="packedColor">Packed DS RGB15 color associated with palette entry one.</param>
        /// <returns>Prepared DS OBJ tile payload stored as packed 16-bit words.</returns>
        std::vector<uint16_t> BuildSolidRectangleTilePixels(int32_t tileWidth, int32_t tileHeight, int32_t filledWidth, int32_t filledHeight, uint16_t packedColor) const;

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
        /// Attempts to express one runtime texture as a 4bpp paletted DS sprite source.
        /// </summary>
        /// <param name="runtimeTexture">Runtime texture to evaluate.</param>
        /// <param name="sourceIndices">Receives one unpacked palette-index buffer in row-major order.</param>
        /// <param name="paletteColors">Receives one 16-entry DS sprite palette with entry zero reserved for transparency.</param>
        /// <returns>True when the runtime texture can be represented as one 4bpp DS sprite source.</returns>
        bool TryBuildHardwareSpriteIndexed4(NintendoDsRuntimeTexture2D* runtimeTexture, std::vector<uint8_t>& sourceIndices, std::array<uint16_t, 16>& paletteColors) const;

        /// <summary>
        /// Resolves or allocates one DS sprite palette bank for the requested runtime texture on the active screen.
        /// </summary>
        /// <param name="runtimeTexture">Runtime texture requesting one palette bank.</param>
        /// <param name="targetBottomScreen">True when the sub-screen palette should be used.</param>
        /// <param name="paletteBank">Receives the resolved palette bank.</param>
        /// <returns>True when a palette bank was available.</returns>
        bool TryResolveHardwareSpritePaletteBank(NintendoDsRuntimeTexture2D* runtimeTexture, bool targetBottomScreen, int32_t& paletteBank);

        /// <summary>
        /// Uploads one prepared 16-entry DS sprite palette into the palette memory for the requested screen and bank.
        /// </summary>
        /// <param name="targetBottomScreen">True when the sub-screen palette should be updated.</param>
        /// <param name="paletteBank">Palette bank to overwrite.</param>
        /// <param name="paletteColors">Prepared 16-entry palette to upload.</param>
        void UploadHardwareSpritePalette(bool targetBottomScreen, int32_t paletteBank, const std::array<uint16_t, 16>& paletteColors) const;

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
        /// Builds one temporary DS 4bpp tile payload copied from one authored sprite texture region.
        /// </summary>
        /// <param name="sourceIndices">Decoded palette indices in row-major order.</param>
        /// <param name="sourceWidth">Authored source texture width in pixels.</param>
        /// <param name="sourceHeight">Authored source texture height in pixels.</param>
        /// <param name="tileOriginX">Source pixel X offset for the tile copy.</param>
        /// <param name="tileOriginY">Source pixel Y offset for the tile copy.</param>
        /// <param name="tileWidth">Prepared DS OBJ tile width in pixels.</param>
        /// <param name="tileHeight">Prepared DS OBJ tile height in pixels.</param>
        /// <returns>Padded DS OBJ tile bytes in 4bpp tiled order.</returns>
        std::vector<uint8_t> BuildHardwareSpriteIndexedTileBytes(const std::vector<uint8_t>& sourceIndices, int32_t sourceWidth, int32_t sourceHeight, int32_t tileOriginX, int32_t tileOriginY, int32_t tileWidth, int32_t tileHeight) const;

        /// <summary>
        /// Attempts to submit one text drawable through a DS hardware-backed path.
        /// </summary>
        /// <param name="text">Text drawable to evaluate.</param>
        /// <returns>True when the text was submitted to DS hardware.</returns>
        bool TryDrawHardwareText(ITextDrawable2D* text);

        /// <summary>
        /// Ensures the requested DS screen owns one initialized BG0 text background ready for shared text submission.
        /// </summary>
        /// <param name="targetScreen">Physical DS screen whose text background should be ready.</param>
        void EnsureScreenTextBackgroundReady(NintendoDsScreenTarget targetScreen);

        /// <summary>
        /// Ensures the bottom-screen DS text background exists for direct tile-map text submission.
        /// </summary>
        void EnsureBottomScreenTextBackgroundReady();

        /// <summary>
        /// Ensures the top-screen DS text background exists for direct tile-map text submission.
        /// </summary>
        void EnsureTopScreenTextBackgroundReady();

        /// <summary>
        /// Clears the bottom-screen DS text background map through the renderer-owned shadow state.
        /// </summary>
        void ClearBottomScreenTextMap();

        /// <summary>
        /// Clears the requested DS text background map through the renderer-owned shadow state.
        /// </summary>
        /// <param name="targetScreen">Physical DS screen whose text map should be cleared.</param>
        void ClearScreenTextMap(NintendoDsScreenTarget targetScreen);

        /// <summary>
        /// Clears the top-screen DS text background map through the renderer-owned shadow state.
        /// </summary>
        void ClearTopScreenTextMap();

        /// <summary>
        /// Writes one DS-only handwritten proof string into the top-screen text map without depending on authored font assets.
        /// </summary>
        void WriteTopScreenProofText();

        /// <summary>
        /// Resolves the real demo-disc body font used by the renderer-owned top-screen cooked-font proof line.
        /// </summary>
        /// <returns>Loaded runtime font asset used by the top-screen cooked-font proof.</returns>
        FontAsset* ResolveRequiredTopScreenProofFont() const;

        /// <summary>
        /// Ensures one dedicated top-screen proof OBJ exists so BG0 and OBJ coexistence can be validated with known content.
        /// </summary>
        void EnsureTopScreenProofSpriteResources();

        /// <summary>
        /// Submits one dedicated top-screen proof OBJ at a fixed position for BG0 and OBJ coexistence validation.
        /// </summary>
        void SubmitTopScreenProofSprite();

        /// <summary>
        /// Paints one solid-color proof box into the bottom-screen DS text background map.
        /// </summary>
        /// <param name="column">Left tile column of the proof box.</param>
        /// <param name="row">Top tile row of the proof box.</param>
        /// <param name="widthInTiles">Width of the proof box in tile cells.</param>
        /// <param name="heightInTiles">Height of the proof box in tile cells.</param>
        /// <param name="tileIndex">Tile index used to fill the proof box.</param>
        void PaintBottomScreenProofBox(int32_t column, int32_t row, int32_t widthInTiles, int32_t heightInTiles, uint16_t tileIndex);

        /// <summary>
        /// Ensures the active font has uploaded glyph tiles ready for bottom-screen DS text-background submission.
        /// </summary>
        /// <param name="font">Font whose cooked glyph atlas should back the bottom-screen text background.</param>
        void EnsureBottomScreenFontGlyphTilesReady(FontAsset* font);

        /// <summary>
        /// Ensures the active font has uploaded glyph tiles ready for the requested DS text background submission path.
        /// </summary>
        /// <param name="targetScreen">Physical DS screen whose text background should receive the glyph tiles.</param>
        /// <param name="font">Font whose cooked glyph atlas should back the requested screen.</param>
        void EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget targetScreen, FontAsset* font);

        /// <summary>
        /// Ensures the active font has uploaded glyph tiles ready for top-screen DS text-background submission.
        /// </summary>
        /// <param name="font">Font whose cooked glyph atlas should back the top-screen text background.</param>
        void EnsureTopScreenFontGlyphTilesReady(FontAsset* font);

        /// <summary>
        /// Resolves one printable character into the uploaded DS text-background tile index for the active font.
        /// </summary>
        /// <param name="font">Font whose uploaded glyph cache should be queried.</param>
        /// <param name="character">Printable character to map.</param>
        /// <param name="tileIndex">Receives the uploaded tile index when the glyph is available.</param>
        /// <returns>True when the glyph was uploaded and can be referenced from the text background map.</returns>
        bool TryResolveBottomScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex);

        /// <summary>
        /// Resolves one printable character into the uploaded DS text-background tile index for the requested screen.
        /// </summary>
        /// <param name="targetScreen">Physical DS screen whose text background glyph cache should be queried.</param>
        /// <param name="font">Font whose uploaded glyph cache should be queried.</param>
        /// <param name="character">Printable character to map.</param>
        /// <param name="tileIndex">Receives the uploaded tile index when the glyph is available.</param>
        /// <returns>True when the glyph was uploaded and can be referenced from the requested text background map.</returns>
        bool TryResolveScreenGlyphTileIndex(NintendoDsScreenTarget targetScreen, FontAsset* font, char character, uint16_t& tileIndex);

        /// <summary>
        /// Resolves one printable character into the uploaded top-screen DS text-background tile index for the active font.
        /// </summary>
        /// <param name="font">Font whose uploaded glyph cache should be queried.</param>
        /// <param name="character">Printable character to map.</param>
        /// <param name="tileIndex">Receives the uploaded tile index when the glyph is available.</param>
        /// <returns>True when the glyph was uploaded and can be referenced from the top-screen text background map.</returns>
        bool TryResolveTopScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex);

        /// <summary>
        /// Writes one text line into the bottom-screen DS text background at the requested cell position.
        /// </summary>
        /// <param name="row">Zero-based text row.</param>
        /// <param name="column">Zero-based text column.</param>
        /// <param name="line">Visible line content to write.</param>
        /// <param name="visibleColumnCount">Number of writable columns in the row segment.</param>
        void WriteBottomScreenTextLine(int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount);

        /// <summary>
        /// Writes one text line into the requested DS text background at the requested cell position.
        /// </summary>
        /// <param name="targetScreen">Physical DS screen whose text background should be written.</param>
        /// <param name="row">Zero-based text row.</param>
        /// <param name="column">Zero-based text column.</param>
        /// <param name="line">Visible line content to write.</param>
        /// <param name="visibleColumnCount">Number of writable columns in the row segment.</param>
        void WriteScreenTextLine(NintendoDsScreenTarget targetScreen, int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount);

        /// <summary>
        /// Writes one text line into the top-screen DS text background at the requested cell position.
        /// </summary>
        /// <param name="row">Zero-based text row.</param>
        /// <param name="column">Zero-based text column.</param>
        /// <param name="line">Visible line content to write.</param>
        /// <param name="visibleColumnCount">Number of writable columns in the row segment.</param>
        void WriteTopScreenTextLine(int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount);

        /// <summary>
        /// Resolves one printable ASCII character into the DS text-background glyph tile index.
        /// </summary>
        /// <param name="character">Printable character to map.</param>
        /// <returns>Glyph tile index or zero for blank/unsupported characters.</returns>
        uint16_t ResolveBottomScreenGlyphTileIndex(char character) const;

        /// <summary>
        /// Resolves one printable ASCII character into the DS text-background glyph tile index for the requested screen.
        /// </summary>
        /// <param name="targetScreen">Physical DS screen whose text background glyph cache should be queried.</param>
        /// <param name="character">Printable character to map.</param>
        /// <returns>Glyph tile index or zero for blank or unsupported characters.</returns>
        uint16_t ResolveScreenGlyphTileIndex(NintendoDsScreenTarget targetScreen, char character) const;

        /// <summary>
        /// Resolves one printable ASCII character into the top-screen DS text-background glyph tile index.
        /// </summary>
        /// <param name="character">Printable character to map.</param>
        /// <returns>Glyph tile index or zero for blank/unsupported characters.</returns>
        uint16_t ResolveTopScreenGlyphTileIndex(char character) const;

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
