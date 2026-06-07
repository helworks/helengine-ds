#include "platform/ds/NintendoDsRenderManager2D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

extern "C" {
#include <nds/arm9/console.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <nds/system.h>
#include <nds/timers.h>
}

#include "Entity.hpp"
#include "FontAsset.hpp"
#include "ICamera.hpp"
#include "IRenderQueue2D.hpp"
#include "TextureAsset.hpp"
#include "platform/ds/NintendoDsRuntimeTexture2D.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::ds {
    namespace {
        /// Converts one CPU timing sample captured through libnds into milliseconds.
        double ConvertCpuTimingTicksToMilliseconds(uint32_t ticks) {
            return static_cast<double>(timerTicks2usec(ticks)) / 1000.0;
        }
    }

    /// Creates the Nintendo DS 2D renderer with no active texture-build diagnostics yet.
    NintendoDsRenderManager2D::NintendoDsRenderManager2D()
        : LastTextureBuildStage("NotStarted")
        , LastTextureAssetId()
        , LastTextureWidth(0)
        , LastTextureHeight(0)
        , LastTextureColorLength(0)
        , ActiveViewportOffsetX(0)
        , ActiveViewportOffsetY(0)
        , ActiveClipLeft(0)
        , ActiveClipTop(0)
        , ActiveClipRight(FrameBufferWidth)
        , ActiveClipBottom(VisibleScreenHeight)
        , Hardware3DScreenTarget(NintendoDsScreenTarget::None)
        , ActiveViewportTargetsBottomScreen(false)
        , BottomScreenPresentationEnabled(true)
        , RuntimeHeartbeatFrameIndex(-1)
        , BottomScreenTextSweepFrameIndex(-1)
        , BottomScreenConsoleRowLastWrittenFrame()
        , NextMainDebugMarkerSpriteId(0)
        , NextSubDebugMarkerSpriteId(0)
        , MainDebugMarkerInitialized(false)
        , MainSpriteEngineInitialized(false)
        , SubSpriteEngineInitialized(false)
        , SubDebugMarkerInitialized(false)
        , MainDebugMarkerGfx(nullptr)
        , SubDebugMarkerGfx(nullptr)
        , UnsupportedSpriteLoggedThisFrame(false)
        , UnsupportedTextLoggedThisFrame(false)
        , UnsupportedRoundedRectLoggedThisFrame(false)
        , ProfileTotalFrameMilliseconds(0.0)
        , ProfileTextMilliseconds(0.0)
        , ProfileSpriteMilliseconds(0.0)
        , ProfileRoundedRectMilliseconds(0.0)
        , ProfileClearMilliseconds(0.0)
        , ProfileTextPrimitiveCount(0)
        , ProfileSpritePrimitiveCount(0)
        , ProfileRoundedRectPrimitiveCount(0)
        , ProfileUnsupportedPrimitiveCount(0)
        , LastReleaseTextureNetByteDelta(0)
        , LastReleaseFontNetByteDelta(0) {
        BottomScreenConsoleRowLastWrittenFrame.fill(-1);
    }

    /// Builds one DS runtime texture from the authored texture asset.
    /// <param name="data">Authored texture asset.</param>
    /// <returns>DS runtime texture carrying the adopted cooked pixel payload.</returns>
    RuntimeTexture* NintendoDsRenderManager2D::BuildTextureFromRaw(TextureAsset* data) {
        LastTextureBuildStage = "BuildTextureFromRaw";
        LastTextureAssetId = data != nullptr ? data->get_Id() : std::string();
        LastTextureWidth = data != nullptr ? data->Width : 0;
        LastTextureHeight = data != nullptr ? data->Height : 0;
        LastTextureColorLength = data != nullptr && data->Colors != nullptr ? data->Colors->Length : 0;
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        NintendoDsRuntimeTexture2D* runtimeTexture = new NintendoDsRuntimeTexture2D();
        runtimeTexture->set_Width(data->Width);
        runtimeTexture->set_Height(data->Height);
        runtimeTexture->ColorFormat = data->ColorFormat;
        runtimeTexture->AlphaPrecision = data->AlphaPrecision;
        runtimeTexture->Colors = data->Colors;
        runtimeTexture->PaletteColors = data->PaletteColors;
        runtimeTexture->HardwareTextureId = -1;
        runtimeTexture->HardwareTextureUploaded = false;
        LastTextureBuildStage = "BuildTextureFromRawComplete";
        return runtimeTexture;
    }

    /// Builds one DS runtime texture from one builder-owned cooked texture payload serialized on disk.
    /// <param name="cookedAssetPath">Absolute NitroFS or host path to the serialized cooked texture asset.</param>
    /// <returns>DS runtime texture carrying the adopted cooked pixel payload.</returns>
    RuntimeTexture* NintendoDsRenderManager2D::BuildTextureFromCooked(std::string cookedAssetPath) {
        LastTextureBuildStage = "BuildTextureFromCooked";
        LastTextureAssetId = cookedAssetPath;
        LastTextureWidth = 0;
        LastTextureHeight = 0;
        LastTextureColorLength = 0;
        NintendoDsRuntimeTexture2D* runtimeTexture = new NintendoDsRuntimeTexture2D();
        runtimeTexture->set_Width(0);
        runtimeTexture->set_Height(0);
        runtimeTexture->HardwareTextureId = -1;
        runtimeTexture->HardwareTextureUploaded = false;
        runtimeTexture->Colors = nullptr;
        runtimeTexture->PaletteColors = nullptr;
        LastTextureBuildStage = "BuildTextureFromCookedComplete";
        return runtimeTexture;
    }

    /// Releases one DS runtime texture and its adopted pixel payload.
    /// <param name="texture">Runtime texture to release.</param>
    void NintendoDsRenderManager2D::ReleaseTexture(RuntimeTexture* texture) {
        LastReleaseTextureNetByteDelta = 0;
        NintendoDsRuntimeTexture2D* dsTexture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(texture);
        if (dsTexture == nullptr) {
            delete texture;
            return;
        }

        if (dsTexture->MainHardwareSpriteGraphics != nullptr) {
            oamFreeGfx(&oamMain, dsTexture->MainHardwareSpriteGraphics);
            dsTexture->MainHardwareSpriteGraphics = nullptr;
        }
        if (dsTexture->SubHardwareSpriteGraphics != nullptr) {
            oamFreeGfx(&oamSub, dsTexture->SubHardwareSpriteGraphics);
            dsTexture->SubHardwareSpriteGraphics = nullptr;
        }
        dsTexture->MainHardwareSpritePrepared = false;
        dsTexture->SubHardwareSpritePrepared = false;
        dsTexture->HardwareSpriteWidth = 0;
        dsTexture->HardwareSpriteHeight = 0;
        delete dsTexture;
    }

    /// Releases one font asset and records allocator diagnostics.
    /// <param name="font">Font asset to release.</param>
    void NintendoDsRenderManager2D::ReleaseFont(FontAsset* font) {
        LastReleaseFontNetByteDelta = 0;
        delete font;
    }

    /// Flushes any DS-owned texture payloads that the runtime marked for deferred release.
    void NintendoDsRenderManager2D::FlushReleasedTextures() {
    }

    /// Gets the last texture-build stage reached by the DS 2D runtime texture path.
    /// <returns>Short texture-build stage label for diagnostics.</returns>
    std::string NintendoDsRenderManager2D::get_LastTextureBuildStage() const {
        return LastTextureBuildStage;
    }

    /// Gets the last texture asset id observed by the DS 2D runtime texture path.
    /// <returns>Texture asset id recorded for diagnostics.</returns>
    std::string NintendoDsRenderManager2D::get_LastTextureAssetId() const {
        return LastTextureAssetId;
    }

    /// Gets the last texture width observed by the DS 2D runtime texture path.
    /// <returns>Texture width recorded for diagnostics.</returns>
    int32_t NintendoDsRenderManager2D::get_LastTextureWidth() const {
        return LastTextureWidth;
    }

    /// Gets the last texture height observed by the DS 2D runtime texture path.
    /// <returns>Texture height recorded for diagnostics.</returns>
    int32_t NintendoDsRenderManager2D::get_LastTextureHeight() const {
        return LastTextureHeight;
    }

    /// Gets the last raw color payload length observed by the DS 2D runtime texture path.
    /// <returns>Texture color payload length recorded for diagnostics.</returns>
    int32_t NintendoDsRenderManager2D::get_LastTextureColorLength() const {
        return LastTextureColorLength;
    }

    /// Resets per-frame diagnostic state before the active camera list is traversed.
    void NintendoDsRenderManager2D::BeginFrame() {
        ActiveViewportOffsetX = 0;
        ActiveViewportOffsetY = 0;
        ActiveClipLeft = 0;
        ActiveClipTop = 0;
        ActiveClipRight = FrameBufferWidth;
        ActiveClipBottom = VisibleScreenHeight;
        Hardware3DScreenTarget = NintendoDsScreenTarget::None;
        ActiveViewportTargetsBottomScreen = false;
        NextMainDebugMarkerSpriteId = 0;
        NextSubDebugMarkerSpriteId = 0;
        UnsupportedSpriteLoggedThisFrame = false;
        UnsupportedTextLoggedThisFrame = false;
        UnsupportedRoundedRectLoggedThisFrame = false;
        ProfileTotalFrameMilliseconds = 0.0;
        ProfileTextMilliseconds = 0.0;
        ProfileSpriteMilliseconds = 0.0;
        ProfileRoundedRectMilliseconds = 0.0;
        ProfileClearMilliseconds = 0.0;
        ProfileTextPrimitiveCount = 0;
        ProfileSpritePrimitiveCount = 0;
        ProfileRoundedRectPrimitiveCount = 0;
        ProfileUnsupportedPrimitiveCount = 0;
        if (MainSpriteEngineInitialized || MainDebugMarkerInitialized) {
            oamClear(&oamMain, 0, 128);
            oamUpdate(&oamMain);
        }
        if (SubSpriteEngineInitialized || SubDebugMarkerInitialized) {
            oamClear(&oamSub, 0, 128);
            oamUpdate(&oamSub);
        }
    }

    /// Draws one camera's ordered 2D queue into the DS screen selected by the authored camera viewport.
    /// <param name="camera">Runtime camera owning the ordered 2D queue.</param>
    void NintendoDsRenderManager2D::DrawCamera(ICamera* camera) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        float4 viewport = ResolveCameraViewport(camera);
        bool targetBottomScreen = false;
        int32_t viewportX = 0;
        int32_t viewportY = 0;
        int32_t viewportWidth = 0;
        int32_t viewportHeight = 0;
        ResolveViewportTarget(viewport, targetBottomScreen, viewportX, viewportY, viewportWidth, viewportHeight);
        if (viewportWidth <= 0 || viewportHeight <= 0) {
            return;
        }

        SelectViewportTarget(targetBottomScreen, viewportX, viewportY, viewportWidth, viewportHeight);
        IRenderQueue2D* renderQueue = camera->get_RenderQueue2D();
        if (renderQueue == nullptr) {
            return;
        }

        renderQueue->VisitOrdered(this);
    }

    /// Visits one ordered 2D drawable and dispatches it through its generated-core draw entry point.
    /// <param name="drawable">Ordered drawable visited from the active camera queue.</param>
    void NintendoDsRenderManager2D::Visit(IDrawable2D* drawable) {
        if (drawable == nullptr) {
            return;
        }

        Entity* parent = drawable->get_Parent();
        if (parent == nullptr || !parent->get_Enabled()) {
            return;
        }

        drawable->Draw();
    }

    /// Draws one rounded rectangle when the backend can map it to hardware, otherwise skips it.
    /// <param name="shape">Rounded-rectangle drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
        uint32_t timingStartTicks = cpuGetTiming();
        ProfileRoundedRectPrimitiveCount++;
        ProfileUnsupportedPrimitiveCount++;
        LogUnsupportedDrawable("RoundedRect", shape);
        int2 markerPosition = ResolveUnsupportedDrawableMarkerPosition(shape);
        DrawUnsupportedDrawableMarker(markerPosition.X, markerPosition.Y, ActiveViewportTargetsBottomScreen ? NintendoDsScreenTarget::Bottom : NintendoDsScreenTarget::Top);
        ProfileRoundedRectMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
    }

    /// Draws one sprite when the backend can map it to hardware, otherwise skips it.
    /// <param name="sprite">Sprite drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
        uint32_t timingStartTicks = cpuGetTiming();
        ProfileSpritePrimitiveCount++;
        if (!TryDrawHardwareSprite(sprite)) {
            ProfileUnsupportedPrimitiveCount++;
            LogUnsupportedDrawable("Sprite", sprite);
            int2 markerPosition = ResolveUnsupportedDrawableMarkerPosition(sprite);
            DrawUnsupportedDrawableMarker(markerPosition.X, markerPosition.Y, ActiveViewportTargetsBottomScreen ? NintendoDsScreenTarget::Bottom : NintendoDsScreenTarget::Top);
        }

        ProfileSpriteMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
    }

    /// Draws one text primitive when the backend can map it to hardware, otherwise skips it.
    /// <param name="text">Text drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawText(ITextDrawable2D* text) {
        uint32_t timingStartTicks = cpuGetTiming();
        ProfileTextPrimitiveCount++;
        if (!TryDrawHardwareText(text)) {
            ProfileUnsupportedPrimitiveCount++;
            LogUnsupportedDrawable("Text", text);
            int2 markerPosition = ResolveUnsupportedDrawableMarkerPosition(text);
            DrawUnsupportedDrawableMarker(markerPosition.X, markerPosition.Y, ActiveViewportTargetsBottomScreen ? NintendoDsScreenTarget::Bottom : NintendoDsScreenTarget::Top);
        }

        ProfileTextMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
    }

    /// Stores which physical Nintendo DS screen currently owns the hardware 3D pass.
    /// <param name="target">Screen that should keep hardware 3D ownership.</param>
    void NintendoDsRenderManager2D::SetHardware3DScreenTarget(NintendoDsScreenTarget target) {
        Hardware3DScreenTarget = target;
    }

    /// Stores whether the bottom DS screen should remain available for visible presentation.
    /// <param name="enabled">True when the bottom screen should stay visible.</param>
    void NintendoDsRenderManager2D::SetBottomScreenPresentationEnabled(bool enabled) {
        BottomScreenPresentationEnabled = enabled;
    }

    /// Gets whether the bottom DS screen should remain visible for the active frame.
    /// <returns>True when the bottom screen remains visible.</returns>
    bool NintendoDsRenderManager2D::get_BottomScreenPresentationEnabled() const {
        return BottomScreenPresentationEnabled;
    }

    /// Stores the latest runtime heartbeat frame consumed by boot-time diagnostics.
    /// <param name="frameIndex">Heartbeat frame index published by the boot host.</param>
    void NintendoDsRenderManager2D::SetRuntimeHeartbeatFrame(int32_t frameIndex) {
        RuntimeHeartbeatFrameIndex = frameIndex;
    }

    /// Gets the latest 2D profile snapshot for debug overlay diagnostics.
    /// <returns>Frame-local 2D profile snapshot.</returns>
    NintendoDsRenderManager2DProfileSnapshot NintendoDsRenderManager2D::get_ProfileSnapshot() const {
        NintendoDsRenderManager2DProfileSnapshot snapshot {};
        snapshot.TotalFrameMilliseconds = ProfileTotalFrameMilliseconds;
        snapshot.TextMilliseconds = ProfileTextMilliseconds;
        snapshot.SpriteMilliseconds = ProfileSpriteMilliseconds;
        snapshot.RoundedRectMilliseconds = ProfileRoundedRectMilliseconds;
        snapshot.ClearMilliseconds = ProfileClearMilliseconds;
        snapshot.TextPrimitiveCount = ProfileTextPrimitiveCount;
        snapshot.SpritePrimitiveCount = ProfileSpritePrimitiveCount;
        snapshot.RoundedRectPrimitiveCount = ProfileRoundedRectPrimitiveCount;
        snapshot.UnsupportedPrimitiveCount = ProfileUnsupportedPrimitiveCount;
        return snapshot;
    }

    /// Resolves the active camera viewport into Nintendo DS pixel coordinates.
    /// <param name="camera">Camera whose viewport should be resolved.</param>
    /// <returns>Viewport rectangle in DS pixel coordinates.</returns>
    float4 NintendoDsRenderManager2D::ResolveCameraViewport(ICamera* camera) const {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        float4 viewport = camera->get_Viewport();
        if (viewport.Z <= 1.0f && viewport.W <= 1.0f) {
            return float4(
                viewport.X * FrameBufferWidth,
                viewport.Y * VisibleScreenHeight,
                viewport.Z * FrameBufferWidth,
                viewport.W * VisibleScreenHeight);
        }

        return viewport;
    }

    /// Resolves the target screen and clip rectangle for one Nintendo DS camera viewport.
    /// <param name="viewport">Viewport rectangle expressed in Nintendo DS pixel-space coordinates.</param>
    /// <param name="targetBottomScreen">Receives whether the viewport targets the bottom screen.</param>
    /// <param name="viewportX">Receives the screen-local viewport X offset.</param>
    /// <param name="viewportY">Receives the screen-local viewport Y offset.</param>
    /// <param name="viewportWidth">Receives the viewport width in pixels.</param>
    /// <param name="viewportHeight">Receives the viewport height in pixels.</param>
    void NintendoDsRenderManager2D::ResolveViewportTarget(
        const float4& viewport,
        bool& targetBottomScreen,
        int32_t& viewportX,
        int32_t& viewportY,
        int32_t& viewportWidth,
        int32_t& viewportHeight) const {
        viewportX = static_cast<int32_t>(std::round(viewport.X));
        int32_t resolvedViewportY = static_cast<int32_t>(std::round(viewport.Y));
        viewportWidth = std::max(static_cast<int32_t>(0), static_cast<int32_t>(std::round(viewport.Z)));
        viewportHeight = std::max(static_cast<int32_t>(0), static_cast<int32_t>(std::round(viewport.W)));
        targetBottomScreen = resolvedViewportY >= VisibleScreenHeight;
        viewportY = targetBottomScreen ? resolvedViewportY - VisibleScreenHeight : resolvedViewportY;

        if (!targetBottomScreen && resolvedViewportY + viewportHeight > VisibleScreenHeight) {
            throw new InvalidOperationException("Nintendo DS 2D cameras must not span from the top screen into the bottom screen.");
        } else if (targetBottomScreen && viewportY + viewportHeight > VisibleScreenHeight) {
            throw new InvalidOperationException("Nintendo DS 2D cameras must stay fully inside the bottom screen.");
        }
    }

    /// Selects the active viewport routing used by subsequent draw calls.
    /// <param name="targetBottomScreen">True when the bottom screen should become active.</param>
    /// <param name="viewportX">Screen-local viewport X offset.</param>
    /// <param name="viewportY">Screen-local viewport Y offset.</param>
    /// <param name="viewportWidth">Viewport width in pixels.</param>
    /// <param name="viewportHeight">Viewport height in pixels.</param>
    void NintendoDsRenderManager2D::SelectViewportTarget(
        bool targetBottomScreen,
        int32_t viewportX,
        int32_t viewportY,
        int32_t viewportWidth,
        int32_t viewportHeight) {
        ActiveViewportTargetsBottomScreen = targetBottomScreen;
        ActiveViewportOffsetX = viewportX;
        ActiveViewportOffsetY = viewportY;
        ActiveClipLeft = std::max(static_cast<int32_t>(0), viewportX);
        ActiveClipTop = std::max(static_cast<int32_t>(0), viewportY);
        ActiveClipRight = std::min(FrameBufferWidth, viewportX + viewportWidth);
        ActiveClipBottom = std::min(VisibleScreenHeight, viewportY + viewportHeight);
    }

    /// Attempts to submit one sprite drawable through a DS hardware-backed path.
    /// <param name="sprite">Sprite drawable to evaluate.</param>
    /// <returns>True when the sprite was submitted to DS hardware.</returns>
    bool NintendoDsRenderManager2D::TryDrawHardwareSprite(ISpriteDrawable2D* sprite) {
        if (sprite == nullptr) {
            return false;
        }

        Entity* parent = sprite->get_Parent();
        if (parent == nullptr) {
            return false;
        }

        float rotation = sprite->get_Rotation();
        if (std::abs(rotation) > 0.001f) {
            return false;
        }

        byte4 color = sprite->get_Color();
        if (color.X != 255 || color.Y != 255 || color.Z != 255 || color.W != 255) {
            return false;
        }

        float4 sourceRect = sprite->get_SourceRect();
        if (std::abs(sourceRect.X) > 0.001f
            || std::abs(sourceRect.Y) > 0.001f
            || std::abs(sourceRect.Z - 1.0f) > 0.001f
            || std::abs(sourceRect.W - 1.0f) > 0.001f) {
            return false;
        }

        int2 drawableSize = sprite->get_Size();
        if (!IsSupportedHardwareSpriteSize(drawableSize)) {
            return false;
        }

        RuntimeTexture* runtimeTextureBase = sprite->get_Texture();
        NintendoDsRuntimeTexture2D* runtimeTexture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(runtimeTextureBase);
        if (runtimeTexture == nullptr) {
            return false;
        }

        if (runtimeTexture->get_Width() != drawableSize.X || runtimeTexture->get_Height() != drawableSize.Y) {
            return false;
        }

        if (!TryPrepareHardwareSpriteGraphics(runtimeTexture)) {
            return false;
        }

        float3 parentPosition = parent->get_Position();
        int32_t maxX = std::max(static_cast<int32_t>(0), FrameBufferWidth - drawableSize.X);
        int32_t maxY = std::max(static_cast<int32_t>(0), VisibleScreenHeight - drawableSize.Y);
        int32_t clampedX = std::clamp(
            static_cast<int32_t>(std::round(parentPosition.X)) + ActiveViewportOffsetX,
            static_cast<int32_t>(0),
            maxX);
        int32_t clampedY = std::clamp(
            static_cast<int32_t>(std::round(parentPosition.Y)) + ActiveViewportOffsetY,
            static_cast<int32_t>(0),
            maxY);
        SpriteSize spriteSize = SpriteSize_8x8;
        if (drawableSize.X == 16 && drawableSize.Y == 16) {
            spriteSize = SpriteSize_16x16;
        } else if (drawableSize.X == 32 && drawableSize.Y == 32) {
            spriteSize = SpriteSize_32x32;
        } else if (drawableSize.X == 64 && drawableSize.Y == 64) {
            spriteSize = SpriteSize_64x64;
        }

        bool targetBottomScreen = ActiveViewportTargetsBottomScreen;
        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        int32_t spriteId = targetBottomScreen ? NextSubDebugMarkerSpriteId : NextMainDebugMarkerSpriteId;
        void* spriteGraphics = targetBottomScreen ? runtimeTexture->SubHardwareSpriteGraphics : runtimeTexture->MainHardwareSpriteGraphics;
        oamSet(
            oamState,
            spriteId,
            clampedX,
            clampedY,
            0,
            0,
            spriteSize,
            SpriteColorFormat_Bmp,
            spriteGraphics,
            -1,
            false,
            false,
            false,
            false,
            false);
        if (targetBottomScreen) {
            NextSubDebugMarkerSpriteId++;
        } else {
            NextMainDebugMarkerSpriteId++;
        }

        oamUpdate(oamState);
        return true;
    }

    /// Ensures one runtime texture has prepared DS OBJ graphics for the first-pass sprite path.
    /// <param name="runtimeTexture">Runtime texture that may own cached DS OBJ graphics.</param>
    /// <returns>True when the runtime texture is ready for first-pass sprite submission.</returns>
    bool NintendoDsRenderManager2D::TryPrepareHardwareSpriteGraphics(NintendoDsRuntimeTexture2D* runtimeTexture) {
        if (runtimeTexture == nullptr) {
            return false;
        }

        if (!IsHardwareSpriteFormatSupported(runtimeTexture)) {
            return false;
        }

        int2 drawableSize(runtimeTexture->get_Width(), runtimeTexture->get_Height());
        if (!IsSupportedHardwareSpriteSize(drawableSize)) {
            return false;
        }

        if (runtimeTexture->Colors == nullptr || runtimeTexture->Colors->Data == nullptr) {
            return false;
        }

        SpriteSize spriteSize = SpriteSize_8x8;
        if (drawableSize.X == 16 && drawableSize.Y == 16) {
            spriteSize = SpriteSize_16x16;
        } else if (drawableSize.X == 32 && drawableSize.Y == 32) {
            spriteSize = SpriteSize_32x32;
        } else if (drawableSize.X == 64 && drawableSize.Y == 64) {
            spriteSize = SpriteSize_64x64;
        }

        bool targetBottomScreen = ActiveViewportTargetsBottomScreen;
        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        void*& spriteGraphics = targetBottomScreen ? runtimeTexture->SubHardwareSpriteGraphics : runtimeTexture->MainHardwareSpriteGraphics;
        bool& spritePrepared = targetBottomScreen ? runtimeTexture->SubHardwareSpritePrepared : runtimeTexture->MainHardwareSpritePrepared;
        if (targetBottomScreen) {
            if (!SubSpriteEngineInitialized) {
                vramSetBankI(VRAM_I_SUB_SPRITE);
                oamInit(&oamSub, SpriteMapping_1D_32, false);
                oamClear(&oamSub, 0, 128);
                SubSpriteEngineInitialized = true;
            }
        } else if (!MainSpriteEngineInitialized) {
            vramSetBankG(VRAM_G_MAIN_SPRITE);
            oamInit(&oamMain, SpriteMapping_1D_32, false);
            oamClear(&oamMain, 0, 128);
            MainSpriteEngineInitialized = true;
        }

        if (spritePrepared && spriteGraphics != nullptr) {
            return true;
        }

        spriteGraphics = oamAllocateGfx(oamState, spriteSize, SpriteColorFormat_Bmp);
        if (spriteGraphics == nullptr) {
            return false;
        }

        std::vector<uint16_t> spritePixels = BuildHardwareSpritePixels(runtimeTexture);
        if (spritePixels.empty()) {
            oamFreeGfx(oamState, spriteGraphics);
            spriteGraphics = nullptr;
            return false;
        }

        std::memcpy(
            spriteGraphics,
            spritePixels.data(),
            spritePixels.size() * sizeof(uint16_t));
        spritePrepared = true;
        runtimeTexture->HardwareSpriteWidth = drawableSize.X;
        runtimeTexture->HardwareSpriteHeight = drawableSize.Y;
        return true;
    }

    /// Checks whether one runtime texture uses a format accepted by the first-pass DS sprite path.
    /// <param name="runtimeTexture">Runtime texture to inspect.</param>
    /// <returns>True when the texture format is accepted by the first-pass sprite path.</returns>
    bool NintendoDsRenderManager2D::IsHardwareSpriteFormatSupported(NintendoDsRuntimeTexture2D* runtimeTexture) const {
        if (runtimeTexture == nullptr) {
            return false;
        }

        return runtimeTexture->ColorFormat == TextureAssetColorFormat::Rgba4444
            || runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed4
            || runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed8;
    }

    /// Checks whether one authored sprite size fits inside one first-pass DS OBJ shape.
    /// <param name="drawableSize">Authored sprite size requested by generated core.</param>
    /// <returns>True when the size fits one first-pass DS OBJ shape.</returns>
    bool NintendoDsRenderManager2D::IsSupportedHardwareSpriteSize(const int2& drawableSize) const {
        return (drawableSize.X == 8 && drawableSize.Y == 8)
            || (drawableSize.X == 16 && drawableSize.Y == 16)
            || (drawableSize.X == 32 && drawableSize.Y == 32)
            || (drawableSize.X == 64 && drawableSize.Y == 64);
    }

    /// Builds one temporary DS bitmap-sprite pixel payload from the cooked runtime texture.
    /// <param name="runtimeTexture">Runtime texture carrying the cooked source texel payload.</param>
    /// <returns>Direct-color DS sprite pixels in row-major order.</returns>
    std::vector<uint16_t> NintendoDsRenderManager2D::BuildHardwareSpritePixels(NintendoDsRuntimeTexture2D* runtimeTexture) const {
        if (runtimeTexture == nullptr) {
            return {};
        }

        int32_t textureWidth = runtimeTexture->get_Width();
        int32_t textureHeight = runtimeTexture->get_Height();
        if (textureWidth <= 0 || textureHeight <= 0) {
            return {};
        }

        std::vector<uint16_t> hardwarePixels(static_cast<std::size_t>(textureWidth * textureHeight), 0);
        if (runtimeTexture->ColorFormat == TextureAssetColorFormat::Rgba4444) {
            for (int32_t pixelIndex = 0; pixelIndex < textureWidth * textureHeight; pixelIndex++) {
                int32_t sourceIndex = pixelIndex * 2;
                uint16_t packedColor = static_cast<uint16_t>(runtimeTexture->Colors->Data[sourceIndex] | (runtimeTexture->Colors->Data[sourceIndex + 1] << 8));
                uint8_t red = static_cast<uint8_t>(((packedColor >> 0) & 15) * 17);
                uint8_t green = static_cast<uint8_t>(((packedColor >> 4) & 15) * 17);
                uint8_t blue = static_cast<uint8_t>(((packedColor >> 8) & 15) * 17);
                uint8_t alpha = static_cast<uint8_t>(((packedColor >> 12) & 15) * 17);
                hardwarePixels[static_cast<std::size_t>(pixelIndex)] = alpha < 128
                    ? static_cast<uint16_t>(0)
                    : static_cast<uint16_t>(
                        BIT(15)
                        | ((red >> 3) & 31)
                        | (((green >> 3) & 31) << 5)
                        | (((blue >> 3) & 31) << 10));
            }

            return hardwarePixels;
        }

        if ((runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed4 || runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed8)
            && runtimeTexture->PaletteColors != nullptr
            && runtimeTexture->PaletteColors->Data != nullptr) {
            for (int32_t pixelIndex = 0; pixelIndex < textureWidth * textureHeight; pixelIndex++) {
                uint8_t paletteIndex;
                if (runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed4) {
                    uint8_t packedIndices = runtimeTexture->Colors->Data[pixelIndex / 2];
                    paletteIndex = (pixelIndex & 1) == 0
                        ? static_cast<uint8_t>(packedIndices & 15)
                        : static_cast<uint8_t>((packedIndices >> 4) & 15);
                } else {
                    paletteIndex = runtimeTexture->Colors->Data[pixelIndex];
                }

                int32_t paletteOffset = static_cast<int32_t>(paletteIndex) * 4;
                if (paletteOffset < 0 || paletteOffset + 3 >= runtimeTexture->PaletteColors->Length) {
                    return {};
                }

                uint8_t red = runtimeTexture->PaletteColors->Data[paletteOffset];
                uint8_t green = runtimeTexture->PaletteColors->Data[paletteOffset + 1];
                uint8_t blue = runtimeTexture->PaletteColors->Data[paletteOffset + 2];
                uint8_t alpha = runtimeTexture->PaletteColors->Data[paletteOffset + 3];
                hardwarePixels[static_cast<std::size_t>(pixelIndex)] = alpha < 128
                    ? static_cast<uint16_t>(0)
                    : static_cast<uint16_t>(
                        BIT(15)
                        | ((red >> 3) & 31)
                        | (((green >> 3) & 31) << 5)
                        | (((blue >> 3) & 31) << 10));
            }

            return hardwarePixels;
        }

        return {};
    }

    /// Attempts to submit one text drawable through a DS hardware-backed path.
    /// <param name="text">Text drawable to evaluate.</param>
    /// <returns>True when the text was submitted to DS hardware.</returns>
    bool NintendoDsRenderManager2D::TryDrawHardwareText(ITextDrawable2D* text) {
        if (text == nullptr) {
            return false;
        }

        if (!ActiveViewportTargetsBottomScreen || !BottomScreenPresentationEnabled) {
            return false;
        }

        Entity* parent = text->get_Parent();
        if (parent == nullptr) {
            return false;
        }

        FontAsset* font = text->get_Font();
        if (font == nullptr) {
            return false;
        }

        float fontScale = text->get_FontScale();
        if (std::abs(fontScale - 1.0f) > 0.001f) {
            return false;
        }

        if (text->get_WrapText()) {
            return false;
        }

        if (static_cast<int32_t>(text->get_Alignment()) != 0) {
            return false;
        }

        byte4 color = text->get_Color();
        if (color.X != 255 || color.Y != 255 || color.Z != 255 || color.W != 255) {
            return false;
        }

        float4 sourceRect = text->get_SourceRect();
        if (std::abs(sourceRect.X) > 0.001f
            || std::abs(sourceRect.Y) > 0.001f
            || std::abs(sourceRect.Z - 1.0f) > 0.001f
            || std::abs(sourceRect.W - 1.0f) > 0.001f) {
            return false;
        }

        std::string content = text->get_Text();
        if (content.empty()) {
            return true;
        }

        float3 parentPosition = parent->get_Position();
        int32_t screenX = static_cast<int32_t>(std::round(parentPosition.X)) + ActiveViewportOffsetX;
        int32_t screenY = static_cast<int32_t>(std::round(parentPosition.Y)) + ActiveViewportOffsetY;
        if (screenX < 0 || screenY < 0 || screenX >= FrameBufferWidth || screenY >= VisibleScreenHeight) {
            return false;
        }

        constexpr int32_t ConsoleColumns = FrameBufferWidth / 8;
        constexpr int32_t ConsoleRows = VisibleScreenHeight / 8;
        int32_t column = std::clamp(
            static_cast<int32_t>(std::round(static_cast<double>(screenX) / 8.0)),
            static_cast<int32_t>(0),
            ConsoleColumns - 1);
        int32_t row = std::clamp(
            static_cast<int32_t>(std::round(static_cast<double>(screenY) / 8.0)),
            static_cast<int32_t>(0),
            ConsoleRows - 1);
        if (column < 0 || column >= ConsoleColumns || row < 0 || row >= ConsoleRows) {
            return false;
        }

        SweepExpiredBottomScreenConsoleRows();

        for (char character : content) {
            if (character == '\r' || character == '\n' || character == ' ') {
                continue;
            }

            if (static_cast<unsigned char>(character) < 32 || static_cast<unsigned char>(character) > 126) {
                return false;
            }
        }

        std::size_t lineStart = 0;
        int32_t currentRow = row;
        while (lineStart <= content.size()) {
            if (currentRow >= ConsoleRows) {
                break;
            }

            std::size_t lineEnd = content.find('\n', lineStart);
            if (lineEnd == std::string::npos) {
                lineEnd = content.size();
            }

            std::string line = content.substr(lineStart, lineEnd - lineStart);
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            if (column < ConsoleColumns) {
                int32_t visibleColumnCount = ConsoleColumns - column;
                std::size_t visibleLength = std::min<std::size_t>(line.size(), static_cast<std::size_t>(visibleColumnCount));
                iprintf("\x1b[%d;%dH%*s", currentRow, column, visibleColumnCount, "");
                if (visibleLength > 0) {
                    iprintf("\x1b[%d;%dH%.*s", currentRow, column, static_cast<int>(visibleLength), line.c_str());
                }
                BottomScreenConsoleRowLastWrittenFrame[static_cast<std::size_t>(currentRow)] = RuntimeHeartbeatFrameIndex;
            }

            if (lineEnd == content.size()) {
                break;
            }

            lineStart = lineEnd + 1;
            currentRow++;
        }

        return true;
    }

    /// Clears any stale bottom-screen console rows whose text has not been refreshed within the active persistence window.
    void NintendoDsRenderManager2D::SweepExpiredBottomScreenConsoleRows() {
        constexpr int32_t ConsoleRows = VisibleScreenHeight / 8;
        constexpr int32_t ConsoleColumns = FrameBufferWidth / 8;
        /// Retain rows for roughly one second so the half-second DebugComponent refresh cadence does not visibly blink live text.
        constexpr int32_t BottomScreenConsoleRowPersistenceFrames = 60;

        if (RuntimeHeartbeatFrameIndex < 0 || BottomScreenTextSweepFrameIndex == RuntimeHeartbeatFrameIndex) {
            return;
        }

        BottomScreenTextSweepFrameIndex = RuntimeHeartbeatFrameIndex;
        for (int32_t row = 0; row < ConsoleRows; row++) {
            int32_t lastWrittenFrame = BottomScreenConsoleRowLastWrittenFrame[static_cast<std::size_t>(row)];
            if (lastWrittenFrame < 0) {
                continue;
            }

            if (RuntimeHeartbeatFrameIndex - lastWrittenFrame < BottomScreenConsoleRowPersistenceFrames) {
                continue;
            }

            iprintf("\x1b[%d;0H%*s", row, ConsoleColumns, "");
            BottomScreenConsoleRowLastWrittenFrame[static_cast<std::size_t>(row)] = -1;
        }
    }

    /// Emits one debug-only unsupported-draw diagnostic without changing runtime fallback behavior.
    /// <param name="category">Short unsupported category label.</param>
    /// <param name="drawable">Drawable that could not be expressed through DS hardware.</param>
    void NintendoDsRenderManager2D::LogUnsupportedDrawable(const char* category, IDrawable2D* drawable) {
#if !defined(NDEBUG)
        const char* safeCategory = category == nullptr ? "Unknown" : category;
        if (std::strcmp(safeCategory, "Sprite") == 0) {
            if (UnsupportedSpriteLoggedThisFrame) {
                return;
            }

            UnsupportedSpriteLoggedThisFrame = true;
        } else if (std::strcmp(safeCategory, "Text") == 0) {
            if (UnsupportedTextLoggedThisFrame) {
                return;
            }

            UnsupportedTextLoggedThisFrame = true;
        } else if (std::strcmp(safeCategory, "RoundedRect") == 0) {
            if (UnsupportedRoundedRectLoggedThisFrame) {
                return;
            }

            UnsupportedRoundedRectLoggedThisFrame = true;
        }

        (void)safeCategory;
        (void)drawable;
#else
        (void)category;
        (void)drawable;
#endif
    }

    /// Resolves the screen-space anchor used by unsupported-draw markers for one drawable.
    /// <param name="drawable">Drawable that could not be expressed through DS hardware.</param>
    /// <returns>Best-effort screen-space anchor for the diagnostic marker.</returns>
    int2 NintendoDsRenderManager2D::ResolveUnsupportedDrawableMarkerPosition(IDrawable2D* drawable) const {
        if (drawable == nullptr) {
            return int2(ActiveViewportOffsetX, ActiveViewportOffsetY);
        }

        Entity* parent = drawable->get_Parent();
        if (parent == nullptr) {
            return int2(ActiveViewportOffsetX, ActiveViewportOffsetY);
        }

        float3 parentPosition = parent->get_Position();
        return int2(
            static_cast<int32_t>(std::round(parentPosition.X)) + ActiveViewportOffsetX,
            static_cast<int32_t>(std::round(parentPosition.Y)) + ActiveViewportOffsetY);
    }

    /// Draws one debug-only magenta marker through DS sprite hardware for unsupported drawables.
    /// <param name="x">Marker X coordinate in screen-local pixels.</param>
    /// <param name="y">Marker Y coordinate in screen-local pixels.</param>
    /// <param name="targetScreen">Physical DS screen that should show the marker.</param>
    void NintendoDsRenderManager2D::DrawUnsupportedDrawableMarker(int32_t x, int32_t y, NintendoDsScreenTarget targetScreen) {
#if !defined(NDEBUG)
        if (targetScreen == NintendoDsScreenTarget::None) {
            return;
        }

        EnsureUnsupportedMarkerResources(targetScreen);
        OamState* oamState = targetScreen == NintendoDsScreenTarget::Bottom ? &oamSub : &oamMain;
        int32_t spriteId = targetScreen == NintendoDsScreenTarget::Bottom ? NextSubDebugMarkerSpriteId : NextMainDebugMarkerSpriteId;
        void* spriteGfx = targetScreen == NintendoDsScreenTarget::Bottom ? SubDebugMarkerGfx : MainDebugMarkerGfx;
        int32_t clampedX = std::clamp(x, static_cast<int32_t>(0), static_cast<int32_t>(FrameBufferWidth - 8));
        int32_t clampedY = std::clamp(y, static_cast<int32_t>(0), static_cast<int32_t>(VisibleScreenHeight - 8));
        oamSet(
            oamState,
            spriteId,
            clampedX,
            clampedY,
            0,
            0,
            SpriteSize_8x8,
            SpriteColorFormat_16Color,
            spriteGfx,
            -1,
            false,
            false,
            false,
            false,
            false);
        if (targetScreen == NintendoDsScreenTarget::Bottom) {
            NextSubDebugMarkerSpriteId++;
        } else {
            NextMainDebugMarkerSpriteId++;
        }

        oamUpdate(oamState);
#else
        (void)x;
        (void)y;
        (void)targetScreen;
#endif
    }

    /// Ensures the DS sprite resources used by unsupported-draw markers exist for the requested screen.
    /// <param name="targetScreen">Physical DS screen that will own the marker sprite resources.</param>
    void NintendoDsRenderManager2D::EnsureUnsupportedMarkerResources(NintendoDsScreenTarget targetScreen) {
#if !defined(NDEBUG)
        constexpr uint16_t UnsupportedDebugMarkerColor = RGB15(31, 0, 31);
        constexpr std::size_t UnsupportedDebugMarkerTileBytes = 32;
        if (targetScreen == NintendoDsScreenTarget::Bottom) {
            if (!SubDebugMarkerInitialized) {
                if (!SubSpriteEngineInitialized) {
                    vramSetBankI(VRAM_I_SUB_SPRITE);
                    oamInit(&oamSub, SpriteMapping_1D_32, false);
                    oamClear(&oamSub, 0, 128);
                    SubSpriteEngineInitialized = true;
                }

                SubDebugMarkerGfx = oamAllocateGfx(&oamSub, SpriteSize_8x8, SpriteColorFormat_16Color);
                std::memset(SubDebugMarkerGfx, 0x11, UnsupportedDebugMarkerTileBytes);
                SPRITE_PALETTE_SUB[0] = 0;
                SPRITE_PALETTE_SUB[1] = UnsupportedDebugMarkerColor;
                SubDebugMarkerInitialized = true;
            }

            return;
        }

        (void)UnsupportedDebugMarkerColor;
        (void)UnsupportedDebugMarkerTileBytes;
#else
        (void)targetScreen;
#endif
    }
}
#endif
