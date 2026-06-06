#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <string>

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
        /// Attempts to submit one text drawable through a DS hardware-backed path.
        /// </summary>
        /// <param name="text">Text drawable to evaluate.</param>
        /// <returns>True when the text was submitted to DS hardware.</returns>
        bool TryDrawHardwareText(ITextDrawable2D* text);

        /// <summary>
        /// Emits one debug-only unsupported-draw diagnostic without changing runtime fallback behavior.
        /// </summary>
        /// <param name="category">Short unsupported category label.</param>
        /// <param name="drawable">Drawable that could not be expressed through DS hardware.</param>
        void LogUnsupportedDrawable(const char* category, IDrawable2D* drawable);

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
