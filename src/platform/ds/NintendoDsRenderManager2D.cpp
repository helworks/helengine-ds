#include "platform/ds/NintendoDsRenderManager2D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

extern "C" {
#include <nds/arm9/background.h>
#include <nds/interrupts.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <nds/system.h>
#include <nds/timers.h>
}

#include "Entity.hpp"
#include "Asset.hpp"
#include "AssetSerializer.hpp"
#include "Core.hpp"
#include "FontAsset.hpp"
#include "FontInfo.hpp"
#include "ICamera.hpp"
#include "IRenderQueue2D.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "SceneAssetReference.hpp"
#include "SceneAssetReferenceFactory.hpp"
#include "TextureAsset.hpp"
#include "platform/ds/NintendoDsColorPacker.hpp"
#include "platform/ds/NintendoDsRuntimeTexture2D.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/file.hpp"

namespace helengine::ds {
    namespace {
        /// Number of bytes stored in one serialized RGBA32 pixel.
        constexpr int32_t Rgba32BytesPerPixel = 4;

        /// Number of bytes stored in one serialized palette entry.
        constexpr int32_t PaletteEntryBytes = 4;

        /// Glyph tile index reserved for the proof `H` used by bottom-screen diagnostics.
        constexpr uint16_t BottomScreenProofHTileIndex = 240;

        /// Glyph tile index reserved for the proof `E` used by bottom-screen diagnostics.
        constexpr uint16_t BottomScreenProofETileIndex = 241;

        /// Glyph tile index reserved for the proof `L` used by bottom-screen diagnostics.
        constexpr uint16_t BottomScreenProofLTileIndex = 242;

        /// Glyph tile index reserved for the proof `O` used by bottom-screen diagnostics.
        constexpr uint16_t BottomScreenProofOTileIndex = 243;

        /// Glyph tile index reserved for the proof `H` used by top-screen diagnostics.
        constexpr uint16_t TopScreenProofHTileIndex = 503;

        /// Glyph tile index reserved for the proof `E` used by top-screen diagnostics.
        constexpr uint16_t TopScreenProofETileIndex = 504;

        /// Glyph tile index reserved for the proof `L` used by top-screen diagnostics.
        constexpr uint16_t TopScreenProofLTileIndex = 505;

        /// Glyph tile index reserved for the proof `O` used by top-screen diagnostics.
        constexpr uint16_t TopScreenProofOTileIndex = 506;

        /// Stable cooked font path used by the renderer-owned top-screen real-font proof line.
        constexpr const char* TopScreenProofFontRelativePath = "Fonts/DemoDiscBody.ttf";

        /// Host trace path used to capture DS bottom-screen text submissions when emulator stdout is insufficient.
        constexpr const char* BottomScreenTextTracePath = "C:/tmp/helengine-ds-bottom-text-trace.log";

        /// Indicates whether the current DS run has already cleared the bottom-screen text trace file.
        bool BottomScreenTextTraceReset = false;

        /// Host trace path used to capture top-screen menu drawable reject reasons when emulator stdout is insufficient.
        constexpr const char* TopScreenRejectTracePath = "C:/tmp/helengine-ds-top-screen-reject-trace.log";

        /// Indicates whether the current DS run has already cleared the top-screen reject trace file.
        bool TopScreenRejectTraceReset = false;

        /// Indicates whether the current DS run has already recorded the first top-screen queue count.
        bool TopScreenQueueTraceRecorded = false;

        /// Number of top-screen 2D primitive visit lines already captured for the current DS run.
        int32_t TopScreenVisitTraceCount = 0;

        /// Appends one line to the host-side DS bottom-screen trace file without affecting gameplay behavior on failure.
        /// <param name="line">Trace payload to append.</param>
        void AppendBottomScreenTextTraceLine(const std::string& line) {
            try {
                if (!BottomScreenTextTraceReset) {
                    ::File::Delete(BottomScreenTextTracePath);
                    BottomScreenTextTraceReset = true;
                }

                ::FileStream stream(BottomScreenTextTracePath, ::FileMode::Append);
                stream.Write(reinterpret_cast<const uint8_t*>(line.data()), 0, line.size());
                uint8_t newline = static_cast<uint8_t>('\n');
                stream.Write(&newline, 0, 1);
                stream.Flush();
                stream.Close();
            } catch (...) {
            }
        }

        /// Appends one line to the host-side DS top-screen reject trace file without affecting gameplay behavior on failure.
        /// <param name="line">Trace payload to append.</param>
        void AppendTopScreenRejectTraceLine(const std::string& line) {
            try {
                if (!TopScreenRejectTraceReset) {
                    ::File::Delete(TopScreenRejectTracePath);
                    TopScreenRejectTraceReset = true;
                }

                ::FileStream stream(TopScreenRejectTracePath, ::FileMode::Append);
                stream.Write(reinterpret_cast<const uint8_t*>(line.data()), 0, line.size());
                uint8_t newline = static_cast<uint8_t>('\n');
                stream.Write(&newline, 0, 1);
                stream.Flush();
                stream.Close();
            } catch (...) {
            }
        }

        /// Expands one packed 5-bit Nintendo DS bitmap channel into the shared 8-bit range.
        uint8_t ExpandFiveBitChannel(uint16_t packedColor, int32_t shift) {
            uint8_t channel = static_cast<uint8_t>((packedColor >> shift) & 31);
            return static_cast<uint8_t>((channel << 3) | (channel >> 2));
        }

        /// Packs one 8-bit RGB color into Nintendo DS bitmap format with the visible bit enabled.
        uint16_t PackOpaqueByteColor(uint8_t red, uint8_t green, uint8_t blue) {
            return static_cast<uint16_t>(
                BIT(15)
                | ((red >> 3) & 31)
                | (((green >> 3) & 31) << 5)
                | (((blue >> 3) & 31) << 10));
        }

        /// Multiplies one 8-bit channel by another using 255-based normalization.
        uint8_t MultiplyByteChannel(uint8_t left, uint8_t right) {
            return static_cast<uint8_t>((static_cast<int32_t>(left) * static_cast<int32_t>(right) + 127) / 255);
        }

        /// Expands one packed RGBA4444 channel into the 8-bit range used by the DS software compositor.
        uint8_t ExpandFourBitChannel(uint16_t packedColor, int32_t shift) {
            uint8_t channel = static_cast<uint8_t>((packedColor >> shift) & 15);
            return static_cast<uint8_t>((channel << 4) | channel);
        }

        /// Reads one indexed4 palette slot from one packed nibble payload.
        uint8_t ReadPackedNibbleIndex(Array<uint8_t>* colors, int32_t textureWidth, int32_t sampleX, int32_t sampleY) {
            int32_t pixelIndex = (sampleY * textureWidth) + sampleX;
            int32_t sourceIndex = pixelIndex / 2;
            uint8_t packedIndices = colors->Data[sourceIndex];
            if ((pixelIndex & 1) == 0) {
                return static_cast<uint8_t>(packedIndices & 0x0F);
            }

            return static_cast<uint8_t>((packedIndices >> 4) & 0x0F);
        }

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
        , BottomCpuFrameBuffer()
        , ActiveCpuFrameBuffer(nullptr)
        , ActiveViewportOffsetX(0)
        , ActiveViewportOffsetY(0)
        , ActiveClipLeft(0)
        , ActiveClipTop(0)
        , ActiveClipRight(FrameBufferWidth)
        , ActiveClipBottom(VisibleScreenHeight)
        , Hardware3DScreenTarget(NintendoDsScreenTarget::None)
        , ActiveViewportTargetsBottomScreen(false)
        , BottomScreenPresentationEnabled(true)
        , BottomScreenClearedThisFrame(false)
        , RuntimeHeartbeatFrameIndex(-1)
        , BottomScreenTextBackgroundId(-1)
        , TopScreenTextBackgroundId(-1)
        , BottomScreenTextMapEntries(nullptr)
        , TopScreenTextMapEntries(nullptr)
        , BottomScreenTextShadowEntries()
        , TopScreenTextShadowEntries()
        , BottomScreenTextBackgroundInitialized(false)
        , TopScreenTextBackgroundInitialized(false)
        , BottomScreenProofTextInitialized(false)
        , BottomScreenTextGlyphCacheFont(nullptr)
        , TopScreenTextGlyphCacheFont(nullptr)
        , BottomScreenTextGlyphTileIndices()
        , TopScreenTextGlyphTileIndices()
        , BottomScreenTextGlyphTilesUploaded(false)
        , TopScreenTextGlyphTilesUploaded(false)
        , BottomScreenGlyphResolveFailureReason()
        , TopScreenGlyphResolveFailureReason()
        , NextMainDebugMarkerSpriteId(0)
        , NextSubDebugMarkerSpriteId(0)
        , NextMainSpritePaletteBank(0)
        , NextSubSpritePaletteBank(0)
        , MainSolidRectanglePaletteColors()
        , SubSolidRectanglePaletteColors()
        , FrameLocalMainRectangleGraphics()
        , FrameLocalSubRectangleGraphics()
        , MainDebugMarkerInitialized(false)
        , TopScreenProofSpriteInitialized(false)
        , MainSpriteEngineInitialized(false)
        , SubSpriteEngineInitialized(false)
        , SubDebugMarkerInitialized(false)
        , MainDebugMarkerGfx(nullptr)
        , TopScreenProofSpriteGfx(nullptr)
        , SubDebugMarkerGfx(nullptr)
        , UnsupportedSpriteLoggedThisFrame(false)
        , UnsupportedTextLoggedThisFrame(false)
        , UnsupportedRoundedRectLoggedThisFrame(false)
        , UnsupportedTextTraceCountThisFrame(0)
        , UnsupportedSpriteTraceCountThisFrame(0)
        , BottomScreenSubmittedTextCountThisFrame(0)
        , LastTopScreenQueueCount(0)
        , LastBottomScreenQueueCount(0)
        , BottomScreenUnsupportedTextCountThisFrame(0)
        , BottomScreenUnsupportedTextReasonThisFrame()
        , BottomScreenUnsupportedTextSampleThisFrame()
        , ProfileTotalFrameMilliseconds(0.0)
        , ProfileTextMilliseconds(0.0)
        , ProfileSpriteMilliseconds(0.0)
        , ProfileRoundedRectMilliseconds(0.0)
        , ProfileClearMilliseconds(0.0)
        , ProfileTextPrimitiveCount(0)
        , ProfileSpritePrimitiveCount(0)
        , ProfileRoundedRectPrimitiveCount(0)
        , ProfileUnsupportedPrimitiveCount(0)
        , ProfileUnsupportedTextPrimitiveCount(0)
        , ProfileUnsupportedSpritePrimitiveCount(0)
        , ProfileUnsupportedRoundedRectPrimitiveCount(0)
        , LastReleaseTextureNetByteDelta(0)
        , LastReleaseFontNetByteDelta(0) {
        BottomScreenTextShadowEntries.fill(static_cast<uint16_t>(0));
        TopScreenTextShadowEntries.fill(static_cast<uint16_t>(0));
        BottomScreenTextGlyphTileIndices.fill(static_cast<uint16_t>(0));
        TopScreenTextGlyphTileIndices.fill(static_cast<uint16_t>(0));
        MainSolidRectanglePaletteColors.fill(static_cast<uint16_t>(0xFFFF));
        SubSolidRectanglePaletteColors.fill(static_cast<uint16_t>(0xFFFF));
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
        LastTextureBuildStage = "BuildTextureFromCookedBegin";
        LastTextureAssetId = cookedAssetPath;
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("Cooked texture asset path must be provided.", "cookedAssetPath");
        }

        ::FileStream* stream = nullptr;
        ::Asset* asset = nullptr;
        try {
            stream = ::File::OpenRead(cookedAssetPath);
            LastTextureBuildStage = "BuildTextureFromCookedOpened";
            asset = ::AssetSerializer::Deserialize(stream);
            LastTextureBuildStage = "BuildTextureFromCookedDeserialized";
            delete stream;
            stream = nullptr;

            ::TextureAsset* textureAsset = he_cpp_try_cast<TextureAsset>(asset);
            if (textureAsset == nullptr) {
                throw new InvalidOperationException("Nintendo DS cooked texture payloads must deserialize as TextureAsset.");
            }

            LastTextureBuildStage = "BuildTextureFromCookedTyped";
            LastTextureWidth = textureAsset->Width;
            LastTextureHeight = textureAsset->Height;
            LastTextureColorLength = textureAsset->Colors != nullptr ? textureAsset->Colors->Length : 0;
            NintendoDsRuntimeTexture2D* runtimeTexture = new NintendoDsRuntimeTexture2D();
            runtimeTexture->set_Width(textureAsset->Width);
            runtimeTexture->set_Height(textureAsset->Height);
            runtimeTexture->ColorFormat = textureAsset->ColorFormat;
            runtimeTexture->AlphaPrecision = textureAsset->AlphaPrecision;
            runtimeTexture->Colors = textureAsset->Colors;
            runtimeTexture->PaletteColors = textureAsset->PaletteColors;
            runtimeTexture->HardwareTextureId = -1;
            runtimeTexture->HardwareTextureUploaded = false;
            textureAsset->Colors = Array<uint8_t>::Empty();
            textureAsset->PaletteColors = Array<uint8_t>::Empty();
            delete textureAsset;
            LastTextureBuildStage = "BuildTextureFromCookedComplete";
            return runtimeTexture;
        } catch (...) {
            if (stream != nullptr) {
                delete stream;
            }
            if (asset != nullptr) {
                delete asset;
            }

            throw;
        }
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

        for (void* mainSpriteGraphics : dsTexture->MainHardwareSpriteGraphics) {
            if (mainSpriteGraphics == nullptr) {
                continue;
            }

            oamFreeGfx(&oamMain, mainSpriteGraphics);
        }
        dsTexture->MainHardwareSpriteGraphics.clear();
        for (void* subSpriteGraphics : dsTexture->SubHardwareSpriteGraphics) {
            if (subSpriteGraphics == nullptr) {
                continue;
            }

            oamFreeGfx(&oamSub, subSpriteGraphics);
        }
        dsTexture->SubHardwareSpriteGraphics.clear();
        dsTexture->MainHardwareSpritePrepared = false;
        dsTexture->SubHardwareSpritePrepared = false;
        dsTexture->MainHardwareSpriteTileCount = 0;
        dsTexture->SubHardwareSpriteTileCount = 0;
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
        ReleaseFrameLocalRectangleGraphics();
        ActiveCpuFrameBuffer = nullptr;
        ActiveViewportOffsetX = 0;
        ActiveViewportOffsetY = 0;
        ActiveClipLeft = 0;
        ActiveClipTop = 0;
        ActiveClipRight = FrameBufferWidth;
        ActiveClipBottom = VisibleScreenHeight;
        Hardware3DScreenTarget = NintendoDsScreenTarget::None;
        ActiveViewportTargetsBottomScreen = false;
        BottomScreenClearedThisFrame = false;
        NextMainDebugMarkerSpriteId = 0;
        NextSubDebugMarkerSpriteId = 0;
        UnsupportedSpriteLoggedThisFrame = false;
        UnsupportedTextLoggedThisFrame = false;
        UnsupportedRoundedRectLoggedThisFrame = false;
        UnsupportedTextTraceCountThisFrame = 0;
        UnsupportedSpriteTraceCountThisFrame = 0;
        BottomScreenSubmittedTextCountThisFrame = 0;
        LastTopScreenQueueCount = 0;
        LastBottomScreenQueueCount = 0;
        BottomScreenUnsupportedTextCountThisFrame = 0;
        BottomScreenUnsupportedTextReasonThisFrame.clear();
        BottomScreenUnsupportedTextSampleThisFrame.clear();
        BottomScreenGlyphResolveFailureReason.clear();
        TopScreenGlyphResolveFailureReason.clear();
        ProfileTotalFrameMilliseconds = 0.0;
        ProfileTextMilliseconds = 0.0;
        ProfileSpriteMilliseconds = 0.0;
        ProfileRoundedRectMilliseconds = 0.0;
        ProfileClearMilliseconds = 0.0;
        ProfileTextPrimitiveCount = 0;
        ProfileSpritePrimitiveCount = 0;
        ProfileRoundedRectPrimitiveCount = 0;
        ProfileUnsupportedPrimitiveCount = 0;
        ProfileUnsupportedTextPrimitiveCount = 0;
        ProfileUnsupportedSpritePrimitiveCount = 0;
        ProfileUnsupportedRoundedRectPrimitiveCount = 0;
        if (!TopScreenTextBackgroundInitialized) {
            EnsureTopScreenTextBackgroundReady();
        }
        if (MainSpriteEngineInitialized || MainDebugMarkerInitialized) {
            oamClear(&oamMain, 0, 128);
        }
        SubmitTopScreenProofSprite();
        if (BottomScreenTextBackgroundInitialized) {
            ClearBottomScreenTextMap();
        }
        if (SubSpriteEngineInitialized || SubDebugMarkerInitialized) {
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

        if (targetBottomScreen && BottomScreenPresentationEnabled && !BottomScreenClearedThisFrame) {
            ClearScreen(camera, true);
            BottomScreenClearedThisFrame = true;
        }

        SelectViewportTarget(targetBottomScreen, viewportX, viewportY, viewportWidth, viewportHeight);
        IRenderQueue2D* renderQueue = camera->get_RenderQueue2D();
        if (renderQueue == nullptr) {
            return;
        }

        if (!targetBottomScreen && !TopScreenQueueTraceRecorded) {
            AppendTopScreenRejectTraceLine("[helengine-ds] top-queue count=" + std::to_string(renderQueue->get_Count()));
            TopScreenQueueTraceRecorded = true;
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
        if (!ActiveViewportTargetsBottomScreen) {
            ProfileRoundedRectMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
            return;
        }
        if (!ActiveViewportTargetsBottomScreen && TopScreenVisitTraceCount < 8) {
            AppendTopScreenRejectTraceLine("[helengine-ds] top-visit roundedRect");
            TopScreenVisitTraceCount++;
        }
        if (!TryDrawHardwareRectangle(shape)) {
            ProfileUnsupportedPrimitiveCount++;
            ProfileUnsupportedRoundedRectPrimitiveCount++;
            LogUnsupportedDrawable("RoundedRect", shape);
            int2 markerPosition = ResolveUnsupportedDrawableMarkerPosition(shape);
            DrawUnsupportedDrawableMarker(
                markerPosition.X,
                markerPosition.Y,
                ActiveViewportTargetsBottomScreen ? NintendoDsScreenTarget::Bottom : NintendoDsScreenTarget::Top);
        }

        ProfileRoundedRectMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
    }

    /// Attempts to submit one rounded-rectangle drawable through the DS plain-rectangle hardware path.
    /// <param name="shape">Rounded-rectangle drawable to evaluate.</param>
    /// <returns>True when the drawable was submitted as one plain hardware rectangle.</returns>
    bool NintendoDsRenderManager2D::TryDrawHardwareRectangle(IRoundedRectDrawable2D* shape) {
        if (shape == nullptr) {
            return false;
        }

        Entity* parent = shape->get_Parent();
        if (parent == nullptr) {
            return false;
        }

        (void)shape->get_Radius();
        int2 size = shape->get_Size();
        if (size.X <= 0 || size.Y <= 0) {
            return false;
        }

        float3 parentPosition = parent->get_Position();
        int32_t screenX = static_cast<int32_t>(std::round(parentPosition.X)) + ActiveViewportOffsetX;
        int32_t screenY = static_cast<int32_t>(std::round(parentPosition.Y)) + ActiveViewportOffsetY;
        int32_t borderThickness = std::max(static_cast<int32_t>(0), static_cast<int32_t>(std::round(shape->get_BorderThickness())));
        byte4 fillColor = shape->get_FillColor();
        byte4 borderColor = shape->get_BorderColor();
        if (fillColor.W != 0 && fillColor.W != 255) {
            return false;
        }
        if (borderColor.W != 0 && borderColor.W != 255) {
            return false;
        }

        bool drewAnyPixels = false;
        if (fillColor.W == 255) {
            int32_t inset = std::min(borderThickness, std::min(size.X / 2, size.Y / 2));
            int32_t fillX = screenX + inset;
            int32_t fillY = screenY + inset;
            int32_t fillWidth = size.X - (inset * 2);
            int32_t fillHeight = size.Y - (inset * 2);
            if (fillWidth > 0 && fillHeight > 0) {
                drewAnyPixels = TryDrawSolidHardwareRectangle(fillX, fillY, fillWidth, fillHeight, fillColor) || drewAnyPixels;
            }
        }

        if (borderThickness > 0 && borderColor.W == 255) {
            int32_t horizontalBorderWidth = size.X;
            int32_t horizontalBorderHeight = std::min(borderThickness, size.Y);
            int32_t verticalBorderWidth = std::min(borderThickness, size.X);
            int32_t verticalBorderHeight = std::max(static_cast<int32_t>(0), size.Y - (horizontalBorderHeight * 2));
            drewAnyPixels = TryDrawSolidHardwareRectangle(screenX, screenY, horizontalBorderWidth, horizontalBorderHeight, borderColor) || drewAnyPixels;
            drewAnyPixels = TryDrawSolidHardwareRectangle(screenX, screenY + size.Y - horizontalBorderHeight, horizontalBorderWidth, horizontalBorderHeight, borderColor) || drewAnyPixels;
            drewAnyPixels = TryDrawSolidHardwareRectangle(screenX, screenY + horizontalBorderHeight, verticalBorderWidth, verticalBorderHeight, borderColor) || drewAnyPixels;
            drewAnyPixels = TryDrawSolidHardwareRectangle(screenX + size.X - verticalBorderWidth, screenY + horizontalBorderHeight, verticalBorderWidth, verticalBorderHeight, borderColor) || drewAnyPixels;
        }

        if (fillColor.W == 0 && (borderThickness <= 0 || borderColor.W == 0)) {
            return true;
        }

        return drewAnyPixels;
    }

    /// Attempts to submit one solid-color rectangle through DS paletted OBJ tiles.
    /// <param name="x">Screen-local left coordinate in pixels.</param>
    /// <param name="y">Screen-local top coordinate in pixels.</param>
    /// <param name="width">Rectangle width in pixels.</param>
    /// <param name="height">Rectangle height in pixels.</param>
    /// <param name="color">Opaque rectangle color.</param>
    /// <returns>True when the rectangle was submitted through DS hardware sprites.</returns>
    bool NintendoDsRenderManager2D::TryDrawSolidHardwareRectangle(int32_t x, int32_t y, int32_t width, int32_t height, const byte4& color) {
        if (width <= 0 || height <= 0 || color.W != 255) {
            return false;
        }

        int32_t drawLeft = std::max(x, ActiveClipLeft);
        int32_t drawTop = std::max(y, ActiveClipTop);
        int32_t drawRight = std::min(x + width, ActiveClipRight);
        int32_t drawBottom = std::min(y + height, ActiveClipBottom);
        if (drawLeft >= drawRight || drawTop >= drawBottom) {
            return true;
        }

        int32_t clippedWidth = drawRight - drawLeft;
        int32_t clippedHeight = drawBottom - drawTop;
        std::vector<int32_t> tileWidths;
        std::vector<int32_t> tileHeights;
        BuildHardwareSpriteTileSpans(clippedWidth, tileWidths);
        BuildHardwareSpriteTileSpans(clippedHeight, tileHeights);
        if (tileWidths.empty() || tileHeights.empty()) {
            return false;
        }

        bool targetBottomScreen = ActiveViewportTargetsBottomScreen;
        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        std::vector<void*>& frameLocalGraphics = targetBottomScreen ? FrameLocalSubRectangleGraphics : FrameLocalMainRectangleGraphics;
        std::array<uint16_t, 16>& rectanglePaletteColors = targetBottomScreen ? SubSolidRectanglePaletteColors : MainSolidRectanglePaletteColors;
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

        uint16_t packedColor = PackOpaqueByteColor(color.X, color.Y, color.Z);
        int32_t paletteBank = -1;
        for (int32_t colorBank = 0; colorBank < static_cast<int32_t>(rectanglePaletteColors.size()); colorBank++) {
            if (rectanglePaletteColors[static_cast<std::size_t>(colorBank)] != packedColor) {
                continue;
            }

            paletteBank = colorBank;
            break;
        }

        if (paletteBank < 0) {
            int32_t& nextPaletteBank = targetBottomScreen ? NextSubSpritePaletteBank : NextMainSpritePaletteBank;
            if (nextPaletteBank >= 16) {
                return false;
            }

            paletteBank = nextPaletteBank;
            rectanglePaletteColors[static_cast<std::size_t>(paletteBank)] = packedColor;
            nextPaletteBank++;

            std::array<uint16_t, 16> paletteColors {};
            paletteColors[1] = packedColor;
            UploadHardwareSpritePalette(targetBottomScreen, paletteBank, paletteColors);
        }

        int32_t tileCount = static_cast<int32_t>(tileWidths.size() * tileHeights.size());
        int32_t nextSpriteId = targetBottomScreen ? NextSubDebugMarkerSpriteId : NextMainDebugMarkerSpriteId;
        if (nextSpriteId + tileCount > 128) {
            return false;
        }

        std::size_t allocationStartIndex = frameLocalGraphics.size();
        int32_t tileY = drawTop;
        for (int32_t tileHeight : tileHeights) {
            int32_t tileX = drawLeft;
            for (int32_t tileWidth : tileWidths) {
                SpriteSize spriteSize = SpriteSize_8x8;
                if (tileWidth == 8 && tileHeight == 16) {
                    spriteSize = SpriteSize_8x16;
                } else if (tileWidth == 8 && tileHeight == 32) {
                    spriteSize = SpriteSize_8x32;
                } else if (tileWidth == 16 && tileHeight == 8) {
                    spriteSize = SpriteSize_16x8;
                } else if (tileWidth == 16 && tileHeight == 16) {
                    spriteSize = SpriteSize_16x16;
                } else if (tileWidth == 16 && tileHeight == 32) {
                    spriteSize = SpriteSize_16x32;
                } else if (tileWidth == 32 && tileHeight == 8) {
                    spriteSize = SpriteSize_32x8;
                } else if (tileWidth == 32 && tileHeight == 16) {
                    spriteSize = SpriteSize_32x16;
                } else if (tileWidth == 32 && tileHeight == 32) {
                    spriteSize = SpriteSize_32x32;
                }

                void* tileGraphics = oamAllocateGfx(oamState, spriteSize, SpriteColorFormat_16Color);
                if (tileGraphics == nullptr) {
                    for (std::size_t graphicsIndex = allocationStartIndex; graphicsIndex < frameLocalGraphics.size(); graphicsIndex++) {
                        void* allocatedGraphics = frameLocalGraphics[graphicsIndex];
                        if (allocatedGraphics != nullptr) {
                            oamFreeGfx(oamState, allocatedGraphics);
                        }
                    }

                    frameLocalGraphics.resize(allocationStartIndex);
                    return false;
                }

                std::vector<uint16_t> tilePixels = BuildSolidRectangleTilePixels(tileWidth, tileHeight, tileWidth, tileHeight, packedColor);
                if (tilePixels.empty()) {
                    oamFreeGfx(oamState, tileGraphics);
                    for (std::size_t graphicsIndex = allocationStartIndex; graphicsIndex < frameLocalGraphics.size(); graphicsIndex++) {
                        void* allocatedGraphics = frameLocalGraphics[graphicsIndex];
                        if (allocatedGraphics != nullptr) {
                            oamFreeGfx(oamState, allocatedGraphics);
                        }
                    }

                    frameLocalGraphics.resize(allocationStartIndex);
                    return false;
                }

                std::memcpy(tileGraphics, tilePixels.data(), tilePixels.size() * sizeof(uint16_t));
                frameLocalGraphics.push_back(tileGraphics);
                oamSet(
                    oamState,
                    nextSpriteId,
                    tileX,
                    tileY,
                    0,
                    paletteBank,
                    spriteSize,
                    SpriteColorFormat_16Color,
                    tileGraphics,
                    0,
                    false,
                    false,
                    false,
                    false,
                    false);
                nextSpriteId++;
                tileX += tileWidth;
            }

            tileY += tileHeight;
        }

        if (targetBottomScreen) {
            NextSubDebugMarkerSpriteId = nextSpriteId;
        } else {
            NextMainDebugMarkerSpriteId = nextSpriteId;
        }

        return true;
    }

    /// Releases any frame-local DS OBJ graphics allocations created for plain rectangle rendering.
    void NintendoDsRenderManager2D::ReleaseFrameLocalRectangleGraphics() {
        for (void* graphics : FrameLocalMainRectangleGraphics) {
            if (graphics != nullptr) {
                oamFreeGfx(&oamMain, graphics);
            }
        }

        for (void* graphics : FrameLocalSubRectangleGraphics) {
            if (graphics != nullptr) {
                oamFreeGfx(&oamSub, graphics);
            }
        }

        FrameLocalMainRectangleGraphics.clear();
        FrameLocalSubRectangleGraphics.clear();
    }

    /// Builds one tiled 4bpp DS OBJ payload for a clipped solid rectangle tile.
    /// <param name="tileWidth">Prepared DS OBJ tile width in pixels.</param>
    /// <param name="tileHeight">Prepared DS OBJ tile height in pixels.</param>
    /// <param name="filledWidth">Visible filled width inside the tile in pixels.</param>
    /// <param name="filledHeight">Visible filled height inside the tile in pixels.</param>
    /// <param name="packedColor">Packed DS RGB15 color associated with palette entry one.</param>
    /// <returns>Prepared DS OBJ tile payload stored as packed 16-bit words.</returns>
    std::vector<uint16_t> NintendoDsRenderManager2D::BuildSolidRectangleTilePixels(int32_t tileWidth, int32_t tileHeight, int32_t filledWidth, int32_t filledHeight, uint16_t packedColor) const {
        (void)packedColor;
        if (tileWidth <= 0 || tileHeight <= 0) {
            return {};
        }

        int32_t blockColumnCount = tileWidth / 8;
        int32_t blockRowCount = tileHeight / 8;
        if (blockColumnCount <= 0 || blockRowCount <= 0) {
            return {};
        }

        std::vector<uint16_t> tileWords(static_cast<std::size_t>((tileWidth * tileHeight) / 4), 0);
        for (int32_t blockRow = 0; blockRow < blockRowCount; blockRow++) {
            for (int32_t blockColumn = 0; blockColumn < blockColumnCount; blockColumn++) {
                int32_t blockIndex = (blockRow * blockColumnCount) + blockColumn;
                for (int32_t localY = 0; localY < 8; localY++) {
                    for (int32_t localWordX = 0; localWordX < 2; localWordX++) {
                        uint16_t packedWord = 0;
                        for (int32_t nibbleIndex = 0; nibbleIndex < 4; nibbleIndex++) {
                            int32_t localX = (localWordX * 4) + nibbleIndex;
                            int32_t sourceX = (blockColumn * 8) + localX;
                            int32_t sourceY = (blockRow * 8) + localY;
                            if (sourceX >= filledWidth || sourceY >= filledHeight) {
                                continue;
                            }

                            packedWord |= static_cast<uint16_t>(1 << (nibbleIndex * 4));
                        }

                        int32_t destinationIndex = (blockIndex * 16) + (localY * 2) + localWordX;
                        tileWords[static_cast<std::size_t>(destinationIndex)] = packedWord;
                    }
                }
            }
        }

        return tileWords;
    }

    /// Draws one sprite when the backend can map it to hardware, otherwise skips it.
    /// <param name="sprite">Sprite drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
        uint32_t timingStartTicks = cpuGetTiming();
        ProfileSpritePrimitiveCount++;
        if (!ActiveViewportTargetsBottomScreen) {
            ProfileSpriteMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
            return;
        }
        if (!ActiveViewportTargetsBottomScreen && TopScreenVisitTraceCount < 8) {
            std::string line = "[helengine-ds] top-visit sprite";
            if (sprite != nullptr) {
                line += " renderOrder=" + std::to_string(sprite->get_RenderOrder2D());
            }

            AppendTopScreenRejectTraceLine(line);
            TopScreenVisitTraceCount++;
        }
        if (!TryDrawHardwareSprite(sprite)) {
            ProfileUnsupportedPrimitiveCount++;
            ProfileUnsupportedSpritePrimitiveCount++;
        }

        ProfileSpriteMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
    }

    /// Draws one text primitive when the backend can map it to hardware, otherwise skips it.
    /// <param name="text">Text drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawText(ITextDrawable2D* text) {
        uint32_t timingStartTicks = cpuGetTiming();
        ProfileTextPrimitiveCount++;
        if (!ActiveViewportTargetsBottomScreen && TopScreenVisitTraceCount < 8) {
            std::string line = "[helengine-ds] top-visit text";
            if (text != nullptr) {
                line += " renderOrder=" + std::to_string(text->get_RenderOrder2D());
                line += " text=" + text->get_Text();
            }

            AppendTopScreenRejectTraceLine(line);
            TopScreenVisitTraceCount++;
        }
        if (ActiveViewportTargetsBottomScreen) {
            if (!TryDrawHardwareText(text)) {
                ProfileUnsupportedPrimitiveCount++;
                ProfileUnsupportedTextPrimitiveCount++;
            }
        } else if (ActiveCpuFrameBuffer != nullptr) {
            RasterText(text);
        } else if (!TryDrawHardwareText(text)) {
            ProfileUnsupportedPrimitiveCount++;
            ProfileUnsupportedTextPrimitiveCount++;
        }

        ProfileTextMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
    }

    /// Copies the composed bottom-screen software bitmap framebuffer to visible Nintendo DS sub-screen VRAM.
    void NintendoDsRenderManager2D::PresentBottomScreenFrame() {
        if (!BottomScreenPresentationEnabled) {
            return;
        }

        EnsureBottomScreenTextBackgroundReady();
        if (BottomScreenTextMapEntries != nullptr) {
            std::string queueLine = "Q T" + std::to_string(LastTopScreenQueueCount)
                + " B" + std::to_string(LastBottomScreenQueueCount)
                + " H";
            if (Hardware3DScreenTarget == NintendoDsScreenTarget::Top) {
                queueLine += "T";
            } else if (Hardware3DScreenTarget == NintendoDsScreenTarget::Bottom) {
                queueLine += "B";
            } else {
                queueLine += "N";
            }

            constexpr int32_t BottomScreenConsoleColumns = FrameBufferWidth / 8;
            constexpr int32_t BottomScreenDiagnosticRow = 23;
            WriteBottomScreenTextLine(BottomScreenDiagnosticRow, 0, queueLine, BottomScreenConsoleColumns);
        }
        videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);
        if (MainSpriteEngineInitialized || MainDebugMarkerInitialized) {
            oamUpdate(&oamMain);
        }
        if (SubSpriteEngineInitialized || SubDebugMarkerInitialized) {
            oamUpdate(&oamSub);
        }
    }

    /// Stores which physical Nintendo DS screen currently owns the hardware 3D pass.
    /// <param name="target">Screen that should keep hardware 3D ownership.</param>
    void NintendoDsRenderManager2D::SetHardware3DScreenTarget(NintendoDsScreenTarget target) {
        Hardware3DScreenTarget = target;
    }

    /// Stores the current frame's top and bottom 2D queue counts so bottom-screen diagnostics can expose menu traversal state.
    /// <param name="topScreenQueueCount">Top-screen 2D queue count resolved for the current frame.</param>
    /// <param name="bottomScreenQueueCount">Bottom-screen 2D queue count resolved for the current frame.</param>
    void NintendoDsRenderManager2D::SetFrameQueueCounts(int32_t topScreenQueueCount, int32_t bottomScreenQueueCount) {
        LastTopScreenQueueCount = topScreenQueueCount < 0 ? 0 : topScreenQueueCount;
        LastBottomScreenQueueCount = bottomScreenQueueCount < 0 ? 0 : bottomScreenQueueCount;
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

    /// Gets whether the temporary DS proof mode that forces the top screen into BG0 and OBJ validation is active.
    /// <returns>True when the top-screen proof mode should suppress main-engine 3D routing.</returns>
    bool NintendoDsRenderManager2D::get_TopScreenProofModeActive() const {
        return true;
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
        snapshot.UnsupportedTextPrimitiveCount = ProfileUnsupportedTextPrimitiveCount;
        snapshot.UnsupportedSpritePrimitiveCount = ProfileUnsupportedSpritePrimitiveCount;
        snapshot.UnsupportedRoundedRectPrimitiveCount = ProfileUnsupportedRoundedRectPrimitiveCount;
        return snapshot;
    }

    /// Gets the number of bottom-screen text primitives that reached the DS hardware path during the latest frame.
    /// <returns>Bottom-screen text primitive submission count.</returns>
    int32_t NintendoDsRenderManager2D::get_LastBottomScreenSubmittedTextCount() const {
        return BottomScreenSubmittedTextCountThisFrame;
    }

    /// Gets the number of bottom-screen text primitives rejected by the DS hardware path during the latest frame.
    /// <returns>Bottom-screen text rejection count.</returns>
    int32_t NintendoDsRenderManager2D::get_LastBottomScreenUnsupportedTextCount() const {
        return BottomScreenUnsupportedTextCountThisFrame;
    }

    /// Gets the first bottom-screen text reject reason recorded during the latest frame.
    /// <returns>Bottom-screen text reject reason label, or an empty string when no rejection occurred.</returns>
    std::string NintendoDsRenderManager2D::get_LastBottomScreenUnsupportedTextReason() const {
        return BottomScreenUnsupportedTextReasonThisFrame;
    }

    /// Gets the first bottom-screen rejected text content recorded during the latest frame.
    /// <returns>Bottom-screen rejected text content, or an empty string when no rejection occurred.</returns>
    std::string NintendoDsRenderManager2D::get_LastBottomScreenUnsupportedTextSample() const {
        return BottomScreenUnsupportedTextSampleThisFrame;
    }

    /// Clears one software-composed Nintendo DS screen framebuffer from one runtime camera clear configuration.
    /// <param name="camera">Runtime camera providing the clear settings.</param>
    /// <param name="targetBottomScreen">True when the bottom screen should be cleared; otherwise false.</param>
    void NintendoDsRenderManager2D::ClearScreen(ICamera* camera, bool targetBottomScreen) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        uint16_t clearColor = PackOpaqueByteColor(0, 0, 0);
        CameraClearSettings clearSettings = camera->get_ClearSettings();
        if (clearSettings.get_ClearColorEnabled()) {
            clearColor = NintendoDsColorPacker::PackOpaqueColor(clearSettings.get_ClearColor());
        }

        uint16_t* frameBuffer = targetBottomScreen ? BottomCpuFrameBuffer.data() : nullptr;
        if (frameBuffer != nullptr) {
            std::fill_n(frameBuffer, VisibleFrameBufferPixelCount, clearColor);
        }
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
        ActiveCpuFrameBuffer = targetBottomScreen && BottomScreenPresentationEnabled
            ? BottomCpuFrameBuffer.data()
            : nullptr;
        ActiveViewportOffsetX = viewportX;
        ActiveViewportOffsetY = viewportY;
        ActiveClipLeft = std::max(static_cast<int32_t>(0), viewportX);
        ActiveClipTop = std::max(static_cast<int32_t>(0), viewportY);
        ActiveClipRight = std::min(FrameBufferWidth, viewportX + viewportWidth);
        ActiveClipBottom = std::min(VisibleScreenHeight, viewportY + viewportHeight);
    }

    /// Blends one source pixel into the active bottom-screen software framebuffer.
    /// <param name="x">Destination X coordinate in framebuffer space.</param>
    /// <param name="y">Destination Y coordinate in framebuffer space.</param>
    /// <param name="color">Source RGBA color to blend.</param>
    void NintendoDsRenderManager2D::BlendPixel(int32_t x, int32_t y, const byte4& color) {
        if (ActiveCpuFrameBuffer == nullptr
            || x < ActiveClipLeft
            || y < ActiveClipTop
            || x >= ActiveClipRight
            || y >= ActiveClipBottom
            || color.W == 0) {
            return;
        }

        int32_t pixelIndex = (y * FrameBufferWidth) + x;
        if (color.W >= 255) {
            ActiveCpuFrameBuffer[pixelIndex] = PackOpaqueByteColor(color.X, color.Y, color.Z);
            return;
        }

        uint16_t destinationColor = ActiveCpuFrameBuffer[pixelIndex];
        uint8_t destRed = ExpandFiveBitChannel(destinationColor, 0);
        uint8_t destGreen = ExpandFiveBitChannel(destinationColor, 5);
        uint8_t destBlue = ExpandFiveBitChannel(destinationColor, 10);
        int32_t inverseAlpha = 255 - color.W;
        uint8_t outRed = static_cast<uint8_t>(((static_cast<int32_t>(color.X) * color.W) + (static_cast<int32_t>(destRed) * inverseAlpha) + 127) / 255);
        uint8_t outGreen = static_cast<uint8_t>(((static_cast<int32_t>(color.Y) * color.W) + (static_cast<int32_t>(destGreen) * inverseAlpha) + 127) / 255);
        uint8_t outBlue = static_cast<uint8_t>(((static_cast<int32_t>(color.Z) * color.W) + (static_cast<int32_t>(destBlue) * inverseAlpha) + 127) / 255);
        ActiveCpuFrameBuffer[pixelIndex] = PackOpaqueByteColor(outRed, outGreen, outBlue);
    }

    /// Writes one fully opaque pixel into the active bottom-screen software framebuffer without alpha blending.
    /// <param name="x">Destination X coordinate in framebuffer space.</param>
    /// <param name="y">Destination Y coordinate in framebuffer space.</param>
    /// <param name="color">Opaque source RGB color to store.</param>
    void NintendoDsRenderManager2D::WriteOpaquePixel(int32_t x, int32_t y, const byte4& color) {
        if (ActiveCpuFrameBuffer == nullptr
            || x < ActiveClipLeft
            || y < ActiveClipTop
            || x >= ActiveClipRight
            || y >= ActiveClipBottom) {
            return;
        }

        ActiveCpuFrameBuffer[(y * FrameBufferWidth) + x] = PackOpaqueByteColor(color.X, color.Y, color.Z);
    }

    /// Decodes one indexed runtime-texture sample into the shared byte4 color representation.
    /// <param name="texture">Runtime texture containing the sampled pixel payload.</param>
    /// <param name="sampleX">Texture-space X coordinate.</param>
    /// <param name="sampleY">Texture-space Y coordinate.</param>
    /// <returns>Decoded RGBA color.</returns>
    byte4 NintendoDsRenderManager2D::ReadIndexedColor(NintendoDsRuntimeTexture2D* texture, int32_t sampleX, int32_t sampleY) const {
        if (texture == nullptr) {
            throw new ArgumentNullException("texture");
        } else if (texture->PaletteColors == nullptr || texture->PaletteColors->Data == nullptr) {
            throw new InvalidOperationException("Nintendo DS indexed runtime texture did not contain a copied palette payload.");
        }

        uint8_t paletteIndex = texture->ColorFormat == TextureAssetColorFormat::Indexed4
            ? ReadPackedNibbleIndex(texture->Colors, texture->get_Width(), sampleX, sampleY)
            : texture->Colors->Data[(sampleY * texture->get_Width()) + sampleX];
        int32_t paletteOffset = static_cast<int32_t>(paletteIndex) * PaletteEntryBytes;
        if (paletteOffset < 0 || paletteOffset + 3 >= texture->PaletteColors->Length) {
            throw new InvalidOperationException("Nintendo DS indexed runtime texture palette index exceeded the cooked palette payload.");
        }

        return byte4(
            texture->PaletteColors->Data[paletteOffset],
            texture->PaletteColors->Data[paletteOffset + 1],
            texture->PaletteColors->Data[paletteOffset + 2],
            texture->PaletteColors->Data[paletteOffset + 3]);
    }

    /// Rasterizes one textured quad into the active bottom-screen software framebuffer.
    /// <param name="texture">Runtime texture providing sampled texels.</param>
    /// <param name="sourceRect">Normalized source rectangle inside the texture atlas.</param>
    /// <param name="destX">Destination X coordinate in screen space.</param>
    /// <param name="destY">Destination Y coordinate in screen space.</param>
    /// <param name="destWidth">Destination width in pixels.</param>
    /// <param name="destHeight">Destination height in pixels.</param>
    /// <param name="modulationColor">Per-drawable modulation color.</param>
    void NintendoDsRenderManager2D::RasterTexturedQuad(
        NintendoDsRuntimeTexture2D* texture,
        const float4& sourceRect,
        int32_t destX,
        int32_t destY,
        int32_t destWidth,
        int32_t destHeight,
        const byte4& modulationColor) {
        if (texture == nullptr) {
            throw new ArgumentNullException("texture");
        } else if (texture->Colors == nullptr || texture->Colors->Data == nullptr) {
            throw new InvalidOperationException("Nintendo DS runtime texture did not contain a copied color payload.");
        } else if (destWidth <= 0 || destHeight <= 0) {
            return;
        }

        int32_t textureWidth = texture->get_Width();
        int32_t textureHeight = texture->get_Height();
        if (textureWidth <= 0 || textureHeight <= 0) {
            return;
        }

        int32_t startX = std::max(static_cast<int32_t>(0), ActiveClipLeft - destX);
        int32_t endX = std::min(destWidth, ActiveClipRight - destX);
        int32_t startY = std::max(static_cast<int32_t>(0), ActiveClipTop - destY);
        int32_t endY = std::min(destHeight, ActiveClipBottom - destY);
        if (endX <= startX || endY <= startY) {
            return;
        }

        int32_t sourceX = std::clamp(static_cast<int32_t>(std::round(sourceRect.X * textureWidth)), static_cast<int32_t>(0), textureWidth - 1);
        int32_t sourceY = std::clamp(static_cast<int32_t>(std::round(sourceRect.Y * textureHeight)), static_cast<int32_t>(0), textureHeight - 1);
        int32_t sourceWidth = std::clamp(static_cast<int32_t>(std::round(sourceRect.Z * textureWidth)), static_cast<int32_t>(1), textureWidth - sourceX);
        int32_t sourceHeight = std::clamp(static_cast<int32_t>(std::round(sourceRect.W * textureHeight)), static_cast<int32_t>(1), textureHeight - sourceY);
        int32_t sourceXStep = (sourceWidth << 16) / destWidth;
        int32_t sourceYStep = (sourceHeight << 16) / destHeight;
        if (texture->ColorFormat == TextureAssetColorFormat::Rgba4444) {
            int32_t sampleYFixed = startY * sourceYStep;
            for (int32_t y = startY; y < endY; y++) {
                int32_t sampleY = sourceY + (sampleYFixed >> 16);
                int32_t sampleXFixed = startX * sourceXStep;
                for (int32_t x = startX; x < endX; x++) {
                    int32_t sampleX = sourceX + (sampleXFixed >> 16);
                    int32_t sourceIndex = ((sampleY * textureWidth) + sampleX) * sizeof(uint16_t);
                    uint16_t packedColor = static_cast<uint16_t>(texture->Colors->Data[sourceIndex] | (static_cast<uint16_t>(texture->Colors->Data[sourceIndex + 1]) << 8));
                    byte4 sampledColor(
                        ExpandFourBitChannel(packedColor, 0),
                        ExpandFourBitChannel(packedColor, 4),
                        ExpandFourBitChannel(packedColor, 8),
                        ExpandFourBitChannel(packedColor, 12));
                    byte4 blendedColor(
                        MultiplyByteChannel(sampledColor.X, modulationColor.X),
                        MultiplyByteChannel(sampledColor.Y, modulationColor.Y),
                        MultiplyByteChannel(sampledColor.Z, modulationColor.Z),
                        MultiplyByteChannel(sampledColor.W, modulationColor.W));
                    BlendPixel(destX + x, destY + y, blendedColor);
                    sampleXFixed += sourceXStep;
                }

                sampleYFixed += sourceYStep;
            }
        } else if (texture->ColorFormat == TextureAssetColorFormat::Rgba32) {
            int32_t sampleYFixed = startY * sourceYStep;
            for (int32_t y = startY; y < endY; y++) {
                int32_t sampleY = sourceY + (sampleYFixed >> 16);
                int32_t sampleXFixed = startX * sourceXStep;
                for (int32_t x = startX; x < endX; x++) {
                    int32_t sampleX = sourceX + (sampleXFixed >> 16);
                    int32_t sourceIndex = ((sampleY * textureWidth) + sampleX) * Rgba32BytesPerPixel;
                    byte4 sampledColor(
                        texture->Colors->Data[sourceIndex],
                        texture->Colors->Data[sourceIndex + 1],
                        texture->Colors->Data[sourceIndex + 2],
                        texture->Colors->Data[sourceIndex + 3]);
                    byte4 blendedColor(
                        MultiplyByteChannel(sampledColor.X, modulationColor.X),
                        MultiplyByteChannel(sampledColor.Y, modulationColor.Y),
                        MultiplyByteChannel(sampledColor.Z, modulationColor.Z),
                        MultiplyByteChannel(sampledColor.W, modulationColor.W));
                    BlendPixel(destX + x, destY + y, blendedColor);
                    sampleXFixed += sourceXStep;
                }

                sampleYFixed += sourceYStep;
            }
        } else if (texture->ColorFormat == TextureAssetColorFormat::Indexed4 || texture->ColorFormat == TextureAssetColorFormat::Indexed8) {
            int32_t sampleYFixed = startY * sourceYStep;
            for (int32_t y = startY; y < endY; y++) {
                int32_t sampleY = sourceY + (sampleYFixed >> 16);
                int32_t sampleXFixed = startX * sourceXStep;
                for (int32_t x = startX; x < endX; x++) {
                    int32_t sampleX = sourceX + (sampleXFixed >> 16);
                    byte4 sampledColor = ReadIndexedColor(texture, sampleX, sampleY);
                    byte4 blendedColor(
                        MultiplyByteChannel(sampledColor.X, modulationColor.X),
                        MultiplyByteChannel(sampledColor.Y, modulationColor.Y),
                        MultiplyByteChannel(sampledColor.Z, modulationColor.Z),
                        MultiplyByteChannel(sampledColor.W, modulationColor.W));
                    BlendPixel(destX + x, destY + y, blendedColor);
                    sampleXFixed += sourceXStep;
                }

                sampleYFixed += sourceYStep;
            }
        } else {
            throw new InvalidOperationException("Nintendo DS 2D renderer encountered an unsupported runtime texture format.");
        }
    }

    /// Rasterizes one sprite drawable into the bottom-screen software framebuffer.
    /// <param name="sprite">Sprite drawable to rasterize.</param>
    void NintendoDsRenderManager2D::RasterSprite(ISpriteDrawable2D* sprite) {
        if (sprite == nullptr) {
            throw new ArgumentNullException("sprite");
        } else if (sprite->get_Parent() == nullptr) {
            return;
        }

        RuntimeTexture* runtimeTextureBase = sprite->get_Texture();
        NintendoDsRuntimeTexture2D* texture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(runtimeTextureBase);
        if (texture == nullptr) {
            return;
        }

        float3 position = sprite->get_Parent()->get_Position();
        int2 size = sprite->get_Size();
        if (size.X <= 0 || size.Y <= 0) {
            return;
        }

        RasterTexturedQuad(
            texture,
            sprite->get_SourceRect(),
            ActiveViewportOffsetX + static_cast<int32_t>(std::round(position.X)),
            ActiveViewportOffsetY + static_cast<int32_t>(std::round(position.Y)),
            size.X,
            size.Y,
            sprite->get_Color());
    }

    /// Rasterizes one text drawable into the bottom-screen software framebuffer.
    /// <param name="text">Text drawable to rasterize.</param>
    void NintendoDsRenderManager2D::RasterText(ITextDrawable2D* text) {
        if (text == nullptr) {
            throw new ArgumentNullException("text");
        } else if (text->get_Parent() == nullptr) {
            return;
        }

        FontAsset* font = text->get_Font();
        if (font == nullptr) {
            return;
        }

        NintendoDsRuntimeTexture2D* texture = dynamic_cast<NintendoDsRuntimeTexture2D*>(font->get_Texture());
        if (texture == nullptr) {
            return;
        }

        std::string content = text->get_Text();
        double fontScale = std::max(static_cast<double>(text->get_FontScale()), 0.0001);
        float3 position = text->get_Parent()->get_Position();
        double offsetX = 0.0;
        double offsetY = 0.0;
        double lineHeight = std::max(static_cast<double>(font->get_LineHeight()) * fontScale, 1.0);
        double baseX = ActiveViewportOffsetX + std::round(position.X);
        double baseY = ActiveViewportOffsetY + std::round(position.Y);
        int32_t atlasWidth = font->get_AtlasWidth() > 0 ? font->get_AtlasWidth() : texture->get_Width();
        int32_t atlasHeight = font->get_AtlasHeight() > 0 ? font->get_AtlasHeight() : texture->get_Height();
        byte4 color = text->get_Color();

        for (char character : content) {
            if (character == '\n') {
                offsetY += lineHeight;
                offsetX = 0.0;
                continue;
            }

            if (character == ' ') {
                offsetX += static_cast<double>(font->get_FontInfo()->get_SpaceWidth()) * fontScale;
                continue;
            }

            FontChar glyph;
            if (font->get_Characters() == nullptr || !font->get_Characters()->TryGetValue(character, glyph)) {
                continue;
            }

            int32_t glyphWidth = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(glyph.SourceRect.Z * static_cast<double>(atlasWidth) * fontScale)));
            int32_t glyphHeight = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(glyph.SourceRect.W * static_cast<double>(atlasHeight) * fontScale)));
            int32_t glyphX = static_cast<int32_t>(std::round(baseX + offsetX));
            int32_t glyphY = static_cast<int32_t>(std::round(baseY + offsetY + (static_cast<double>(glyph.OffsetY) * fontScale)));
            RasterTexturedQuad(texture, glyph.SourceRect, glyphX, glyphY, glyphWidth, glyphHeight, color);

            double advanceWidth = glyph.AdvanceWidth > 0.0f
                ? static_cast<double>(glyph.AdvanceWidth) * fontScale
                : static_cast<double>(glyphWidth);
            offsetX += advanceWidth;
        }
    }

    /// Attempts to submit one sprite drawable through a DS hardware-backed path.
    /// <param name="sprite">Sprite drawable to evaluate.</param>
    /// <returns>True when the sprite was submitted to DS hardware.</returns>
    bool NintendoDsRenderManager2D::TryDrawHardwareSprite(ISpriteDrawable2D* sprite) {
        if (sprite == nullptr) {
            TraceUnsupportedSpriteDrawable(sprite, "null");
            return false;
        }

        Entity* parent = sprite->get_Parent();
        if (parent == nullptr) {
            TraceUnsupportedSpriteDrawable(sprite, "parent");
            return false;
        }

        float4 orientation = parent->get_Orientation();
        if (std::abs(orientation.X) > 0.001f
            || std::abs(orientation.Y) > 0.001f
            || std::abs(orientation.Z) > 0.001f
            || std::abs(orientation.W - 1.0f) > 0.001f) {
            TraceUnsupportedSpriteDrawable(sprite, "rotation");
            return false;
        }

        byte4 color = sprite->get_Color();
        if (color.X != 255 || color.Y != 255 || color.Z != 255 || color.W != 255) {
            TraceUnsupportedSpriteDrawable(sprite, "color");
            return false;
        }

        float4 sourceRect = sprite->get_SourceRect();
        if (std::abs(sourceRect.X) > 0.001f
            || std::abs(sourceRect.Y) > 0.001f
            || std::abs(sourceRect.Z - 1.0f) > 0.001f
            || std::abs(sourceRect.W - 1.0f) > 0.001f) {
            TraceUnsupportedSpriteDrawable(sprite, "sourceRect");
            return false;
        }

        int2 drawableSize = sprite->get_Size();
        if (!IsSupportedHardwareSpriteSize(drawableSize)) {
            TraceUnsupportedSpriteDrawable(sprite, "size");
            return false;
        }

        RuntimeTexture* runtimeTextureBase = sprite->get_Texture();
        NintendoDsRuntimeTexture2D* runtimeTexture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(runtimeTextureBase);
        if (runtimeTexture == nullptr) {
            TraceUnsupportedSpriteDrawable(sprite, "texture");
            return false;
        }

        int2 hardwareSpriteSize(runtimeTexture->get_Width(), runtimeTexture->get_Height());
        if (!IsSupportedHardwareSpriteSize(hardwareSpriteSize)) {
            TraceUnsupportedSpriteDrawable(sprite, "textureSize");
            return false;
        }

        if (!TryPrepareHardwareSpriteGraphics(runtimeTexture)) {
            TraceUnsupportedSpriteDrawable(sprite, "prepare");
            return false;
        }

        bool targetBottomScreen = ActiveViewportTargetsBottomScreen;
        float3 parentPosition = parent->get_Position();
        int32_t spriteOffsetX = static_cast<int32_t>(std::round((static_cast<double>(drawableSize.X) - static_cast<double>(hardwareSpriteSize.X)) * 0.5));
        int32_t spriteOffsetY = static_cast<int32_t>(std::round((static_cast<double>(drawableSize.Y) - static_cast<double>(hardwareSpriteSize.Y)) * 0.5));
        int32_t maxX = std::max(static_cast<int32_t>(0), FrameBufferWidth - hardwareSpriteSize.X);
        int32_t maxY = std::max(static_cast<int32_t>(0), VisibleScreenHeight - hardwareSpriteSize.Y);
        int32_t clampedX = std::clamp(
            static_cast<int32_t>(std::round(parentPosition.X)) + ActiveViewportOffsetX + spriteOffsetX,
            static_cast<int32_t>(0),
            maxX);
        int32_t clampedY = std::clamp(
            static_cast<int32_t>(std::round(parentPosition.Y)) + ActiveViewportOffsetY + spriteOffsetY,
            static_cast<int32_t>(0),
            maxY);

        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        std::vector<void*>& spriteGraphics = targetBottomScreen ? runtimeTexture->SubHardwareSpriteGraphics : runtimeTexture->MainHardwareSpriteGraphics;
        int32_t paletteBank = targetBottomScreen ? runtimeTexture->SubHardwareSpritePaletteBank : runtimeTexture->MainHardwareSpritePaletteBank;
        std::vector<int32_t> tileWidths;
        std::vector<int32_t> tileHeights;
        BuildHardwareSpriteTileSpans(hardwareSpriteSize.X, tileWidths);
        BuildHardwareSpriteTileSpans(hardwareSpriteSize.Y, tileHeights);
        if (tileWidths.empty() || tileHeights.empty() || spriteGraphics.empty() || paletteBank < 0) {
            TraceUnsupportedSpriteDrawable(sprite, "prepare");
            return false;
        }

        int32_t tileCount = static_cast<int32_t>(tileWidths.size() * tileHeights.size());
        int32_t nextSpriteId = targetBottomScreen ? NextSubDebugMarkerSpriteId : NextMainDebugMarkerSpriteId;
        if (nextSpriteId + tileCount > 128) {
            TraceUnsupportedSpriteDrawable(sprite, "budget");
            return false;
        }

        int32_t spriteGraphicsIndex = 0;
        int32_t tileY = clampedY;
        for (int32_t tileHeight : tileHeights) {
            int32_t tileX = clampedX;
            for (int32_t tileWidth : tileWidths) {
                SpriteSize spriteSize = SpriteSize_8x8;
                if (tileWidth == 8 && tileHeight == 16) {
                    spriteSize = SpriteSize_8x16;
                } else if (tileWidth == 8 && tileHeight == 32) {
                    spriteSize = SpriteSize_8x32;
                } else if (tileWidth == 16 && tileHeight == 8) {
                    spriteSize = SpriteSize_16x8;
                } else if (tileWidth == 16 && tileHeight == 16) {
                    spriteSize = SpriteSize_16x16;
                } else if (tileWidth == 16 && tileHeight == 32) {
                    spriteSize = SpriteSize_16x32;
                } else if (tileWidth == 32 && tileHeight == 8) {
                    spriteSize = SpriteSize_32x8;
                } else if (tileWidth == 32 && tileHeight == 16) {
                    spriteSize = SpriteSize_32x16;
                } else if (tileWidth == 32 && tileHeight == 32) {
                    spriteSize = SpriteSize_32x32;
                }

                oamSet(
                    oamState,
                    nextSpriteId,
                    tileX,
                    tileY,
                    0,
                    paletteBank,
                    spriteSize,
                    SpriteColorFormat_16Color,
                    spriteGraphics[static_cast<std::size_t>(spriteGraphicsIndex)],
                    0,
                    false,
                    false,
                    false,
                    false,
                    false);
                nextSpriteId++;
                spriteGraphicsIndex++;
                tileX += tileWidth;
            }

            tileY += tileHeight;
        }

        if (targetBottomScreen) {
            NextSubDebugMarkerSpriteId = nextSpriteId;
        } else {
            NextMainDebugMarkerSpriteId = nextSpriteId;
        }

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

        bool targetBottomScreen = ActiveViewportTargetsBottomScreen;
        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        std::vector<void*>& spriteGraphics = targetBottomScreen ? runtimeTexture->SubHardwareSpriteGraphics : runtimeTexture->MainHardwareSpriteGraphics;
        bool& spritePrepared = targetBottomScreen ? runtimeTexture->SubHardwareSpritePrepared : runtimeTexture->MainHardwareSpritePrepared;
        int32_t& spriteTileCount = targetBottomScreen ? runtimeTexture->SubHardwareSpriteTileCount : runtimeTexture->MainHardwareSpriteTileCount;
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

        std::vector<int32_t> tileWidths;
        std::vector<int32_t> tileHeights;
        BuildHardwareSpriteTileSpans(drawableSize.X, tileWidths);
        BuildHardwareSpriteTileSpans(drawableSize.Y, tileHeights);
        if (tileWidths.empty() || tileHeights.empty()) {
            return false;
        }

        constexpr int32_t MaximumHardwareSpriteTileCount = 32;
        int32_t tileCount = static_cast<int32_t>(tileWidths.size() * tileHeights.size());
        if (tileCount <= 0 || tileCount > MaximumHardwareSpriteTileCount) {
            return false;
        }

        if (spritePrepared && static_cast<int32_t>(spriteGraphics.size()) == tileCount) {
            return true;
        }

        for (void* existingGraphics : spriteGraphics) {
            if (existingGraphics == nullptr) {
                continue;
            }

            oamFreeGfx(oamState, existingGraphics);
        }
        spriteGraphics.clear();
        spritePrepared = false;
        spriteTileCount = 0;

        std::vector<uint8_t> sourceIndices;
        std::array<uint16_t, 16> paletteColors {};
        if (!TryBuildHardwareSpriteIndexed4(runtimeTexture, sourceIndices, paletteColors)) {
            return false;
        }

        int32_t paletteBank = -1;
        if (!TryResolveHardwareSpritePaletteBank(runtimeTexture, targetBottomScreen, paletteBank)) {
            return false;
        }

        UploadHardwareSpritePalette(targetBottomScreen, paletteBank, paletteColors);
        int32_t tileOriginY = 0;
        for (int32_t tileHeight : tileHeights) {
            int32_t tileOriginX = 0;
            for (int32_t tileWidth : tileWidths) {
                SpriteSize spriteSize = SpriteSize_8x8;
                if (tileWidth == 8 && tileHeight == 16) {
                    spriteSize = SpriteSize_8x16;
                } else if (tileWidth == 8 && tileHeight == 32) {
                    spriteSize = SpriteSize_8x32;
                } else if (tileWidth == 16 && tileHeight == 8) {
                    spriteSize = SpriteSize_16x8;
                } else if (tileWidth == 16 && tileHeight == 16) {
                    spriteSize = SpriteSize_16x16;
                } else if (tileWidth == 16 && tileHeight == 32) {
                    spriteSize = SpriteSize_16x32;
                } else if (tileWidth == 32 && tileHeight == 8) {
                    spriteSize = SpriteSize_32x8;
                } else if (tileWidth == 32 && tileHeight == 16) {
                    spriteSize = SpriteSize_32x16;
                } else if (tileWidth == 32 && tileHeight == 32) {
                    spriteSize = SpriteSize_32x32;
                }

                void* tileGraphics = oamAllocateGfx(oamState, spriteSize, SpriteColorFormat_16Color);
                if (tileGraphics == nullptr) {
                    for (void* allocatedGraphics : spriteGraphics) {
                        if (allocatedGraphics != nullptr) {
                            oamFreeGfx(oamState, allocatedGraphics);
                        }
                    }

                    spriteGraphics.clear();
                    return false;
                }

                std::vector<uint8_t> tileBytes = BuildHardwareSpriteIndexedTileBytes(
                    sourceIndices,
                    drawableSize.X,
                    drawableSize.Y,
                    tileOriginX,
                    tileOriginY,
                    tileWidth,
                    tileHeight);
                if (tileBytes.empty()) {
                    oamFreeGfx(oamState, tileGraphics);
                    for (void* allocatedGraphics : spriteGraphics) {
                        if (allocatedGraphics != nullptr) {
                            oamFreeGfx(oamState, allocatedGraphics);
                        }
                    }

                    spriteGraphics.clear();
                    return false;
                }

                std::memcpy(
                    tileGraphics,
                    tileBytes.data(),
                    tileBytes.size());
                spriteGraphics.push_back(tileGraphics);
                tileOriginX += tileWidth;
            }

            tileOriginY += tileHeight;
        }

        spritePrepared = true;
        spriteTileCount = tileCount;
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

    /// Attempts to express one runtime texture as a 4bpp paletted DS sprite source.
    /// <param name="runtimeTexture">Runtime texture to evaluate.</param>
    /// <param name="sourceIndices">Receives one unpacked palette-index buffer in row-major order.</param>
    /// <param name="paletteColors">Receives one 16-entry DS sprite palette with entry zero reserved for transparency.</param>
    /// <returns>True when the runtime texture can be represented as one 4bpp DS sprite source.</returns>
    bool NintendoDsRenderManager2D::TryBuildHardwareSpriteIndexed4(NintendoDsRuntimeTexture2D* runtimeTexture, std::vector<uint8_t>& sourceIndices, std::array<uint16_t, 16>& paletteColors) const {
        sourceIndices.clear();
        paletteColors.fill(static_cast<uint16_t>(0));
        if (runtimeTexture == nullptr || runtimeTexture->Colors == nullptr || runtimeTexture->Colors->Data == nullptr) {
            return false;
        }

        int32_t textureWidth = runtimeTexture->get_Width();
        int32_t textureHeight = runtimeTexture->get_Height();
        if (textureWidth <= 0 || textureHeight <= 0) {
            return false;
        }

        sourceIndices.resize(static_cast<std::size_t>(textureWidth * textureHeight), 0);
        std::vector<uint16_t> uniqueOpaqueColors;
        uniqueOpaqueColors.reserve(15);
        for (int32_t pixelIndex = 0; pixelIndex < textureWidth * textureHeight; pixelIndex++) {
            uint8_t red = 0;
            uint8_t green = 0;
            uint8_t blue = 0;
            uint8_t alpha = 0;
            if (runtimeTexture->ColorFormat == TextureAssetColorFormat::Rgba4444) {
                int32_t sourceIndex = pixelIndex * 2;
                uint16_t packedColor = static_cast<uint16_t>(runtimeTexture->Colors->Data[sourceIndex] | (runtimeTexture->Colors->Data[sourceIndex + 1] << 8));
                red = static_cast<uint8_t>(((packedColor >> 0) & 15) * 17);
                green = static_cast<uint8_t>(((packedColor >> 4) & 15) * 17);
                blue = static_cast<uint8_t>(((packedColor >> 8) & 15) * 17);
                alpha = static_cast<uint8_t>(((packedColor >> 12) & 15) * 17);
            } else if ((runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed4 || runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed8)
                && runtimeTexture->PaletteColors != nullptr
                && runtimeTexture->PaletteColors->Data != nullptr) {
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
                    return false;
                }

                red = runtimeTexture->PaletteColors->Data[paletteOffset];
                green = runtimeTexture->PaletteColors->Data[paletteOffset + 1];
                blue = runtimeTexture->PaletteColors->Data[paletteOffset + 2];
                alpha = runtimeTexture->PaletteColors->Data[paletteOffset + 3];
            } else {
                return false;
            }

            if (alpha < 128) {
                sourceIndices[static_cast<std::size_t>(pixelIndex)] = 0;
                continue;
            }

            uint16_t packedPaletteColor = RGB15((red >> 3) & 31, (green >> 3) & 31, (blue >> 3) & 31);
            uint8_t resolvedPaletteIndex = 0;
            bool foundPaletteColor = false;
            for (int32_t colorIndex = 0; colorIndex < static_cast<int32_t>(uniqueOpaqueColors.size()); colorIndex++) {
                if (uniqueOpaqueColors[static_cast<std::size_t>(colorIndex)] == packedPaletteColor) {
                    resolvedPaletteIndex = static_cast<uint8_t>(colorIndex + 1);
                    foundPaletteColor = true;
                    break;
                }
            }

            if (!foundPaletteColor) {
                if (uniqueOpaqueColors.size() >= 15) {
                    return false;
                }

                uniqueOpaqueColors.push_back(packedPaletteColor);
                resolvedPaletteIndex = static_cast<uint8_t>(uniqueOpaqueColors.size());
                paletteColors[static_cast<std::size_t>(resolvedPaletteIndex)] = packedPaletteColor;
            }

            sourceIndices[static_cast<std::size_t>(pixelIndex)] = resolvedPaletteIndex;
        }

        return true;
    }

    /// Resolves or allocates one DS sprite palette bank for the requested runtime texture on the active screen.
    /// <param name="runtimeTexture">Runtime texture requesting one palette bank.</param>
    /// <param name="targetBottomScreen">True when the sub-screen palette should be used.</param>
    /// <param name="paletteBank">Receives the resolved palette bank.</param>
    /// <returns>True when a palette bank was available.</returns>
    bool NintendoDsRenderManager2D::TryResolveHardwareSpritePaletteBank(NintendoDsRuntimeTexture2D* runtimeTexture, bool targetBottomScreen, int32_t& paletteBank) {
        if (runtimeTexture == nullptr) {
            paletteBank = -1;
            return false;
        }

        int32_t& cachedPaletteBank = targetBottomScreen ? runtimeTexture->SubHardwareSpritePaletteBank : runtimeTexture->MainHardwareSpritePaletteBank;
        if (cachedPaletteBank >= 0) {
            paletteBank = cachedPaletteBank;
            return true;
        }

        int32_t& nextPaletteBank = targetBottomScreen ? NextSubSpritePaletteBank : NextMainSpritePaletteBank;
        if (nextPaletteBank >= 16) {
            paletteBank = -1;
            return false;
        }

        cachedPaletteBank = nextPaletteBank;
        paletteBank = cachedPaletteBank;
        nextPaletteBank++;
        return true;
    }

    /// Uploads one prepared 16-entry DS sprite palette into the palette memory for the requested screen and bank.
    /// <param name="targetBottomScreen">True when the sub-screen palette should be updated.</param>
    /// <param name="paletteBank">Palette bank to overwrite.</param>
    /// <param name="paletteColors">Prepared 16-entry palette to upload.</param>
    void NintendoDsRenderManager2D::UploadHardwareSpritePalette(bool targetBottomScreen, int32_t paletteBank, const std::array<uint16_t, 16>& paletteColors) const {
        if (paletteBank < 0 || paletteBank >= 16) {
            return;
        }

        uint16_t* paletteMemory = targetBottomScreen ? SPRITE_PALETTE_SUB : SPRITE_PALETTE;
        int32_t paletteOffset = paletteBank * 16;
        for (int32_t paletteIndex = 0; paletteIndex < 16; paletteIndex++) {
            paletteMemory[paletteOffset + paletteIndex] = paletteColors[static_cast<std::size_t>(paletteIndex)];
        }
    }

    /// Checks whether one authored sprite size fits inside one first-pass DS OBJ shape.
    /// <param name="drawableSize">Authored sprite size requested by generated core.</param>
    /// <returns>True when the size fits one first-pass DS OBJ shape.</returns>
    bool NintendoDsRenderManager2D::IsSupportedHardwareSpriteSize(const int2& drawableSize) const {
        if (drawableSize.X <= 0 || drawableSize.Y <= 0) {
            return false;
        }

        std::vector<int32_t> tileWidths;
        std::vector<int32_t> tileHeights;
        BuildHardwareSpriteTileSpans(drawableSize.X, tileWidths);
        BuildHardwareSpriteTileSpans(drawableSize.Y, tileHeights);
        if (tileWidths.empty() || tileHeights.empty()) {
            return false;
        }

        constexpr int32_t MaximumHardwareSpriteTileCount = 32;
        return static_cast<int32_t>(tileWidths.size() * tileHeights.size()) <= MaximumHardwareSpriteTileCount;
    }

    /// Expands one authored sprite dimension into a minimal set of DS OBJ tile spans.
    /// <param name="length">Authored sprite width or height in pixels.</param>
    /// <param name="spans">Receives the DS OBJ tile spans that cover the authored dimension.</param>
    void NintendoDsRenderManager2D::BuildHardwareSpriteTileSpans(int32_t length, std::vector<int32_t>& spans) const {
        spans.clear();
        if (length <= 0) {
            return;
        }

        int32_t remainingLength = length;
        while (remainingLength > 0) {
            int32_t tileSpan = 32;
            if (remainingLength <= 8) {
                tileSpan = 8;
            } else if (remainingLength <= 16) {
                tileSpan = 16;
            }

            spans.push_back(tileSpan);
            remainingLength -= tileSpan;
        }
    }

    /// Builds one temporary DS 4bpp tile payload copied from one authored sprite texture region.
    /// <param name="sourceIndices">Decoded palette indices in row-major order.</param>
    /// <param name="sourceWidth">Authored source texture width in pixels.</param>
    /// <param name="sourceHeight">Authored source texture height in pixels.</param>
    /// <param name="tileOriginX">Source pixel X offset for the tile copy.</param>
    /// <param name="tileOriginY">Source pixel Y offset for the tile copy.</param>
    /// <param name="tileWidth">Prepared DS OBJ tile width in pixels.</param>
    /// <param name="tileHeight">Prepared DS OBJ tile height in pixels.</param>
    /// <returns>Padded DS OBJ tile bytes in 4bpp tiled order.</returns>
    std::vector<uint8_t> NintendoDsRenderManager2D::BuildHardwareSpriteIndexedTileBytes(const std::vector<uint8_t>& sourceIndices, int32_t sourceWidth, int32_t sourceHeight, int32_t tileOriginX, int32_t tileOriginY, int32_t tileWidth, int32_t tileHeight) const {
        if (sourceWidth <= 0 || sourceHeight <= 0 || tileWidth <= 0 || tileHeight <= 0) {
            return {};
        }

        std::vector<uint8_t> tileBytes(static_cast<std::size_t>((tileWidth * tileHeight) / 2), 0);
        int32_t blockColumnCount = tileWidth / 8;
        int32_t blockRowCount = tileHeight / 8;
        for (int32_t blockRow = 0; blockRow < blockRowCount; blockRow++) {
            for (int32_t blockColumn = 0; blockColumn < blockColumnCount; blockColumn++) {
                int32_t blockIndex = (blockRow * blockColumnCount) + blockColumn;
                for (int32_t localY = 0; localY < 8; localY++) {
                    int32_t sourceY = tileOriginY + (blockRow * 8) + localY;
                    if (sourceY < 0 || sourceY >= sourceHeight) {
                        continue;
                    }

                    for (int32_t localX = 0; localX < 8; localX += 2) {
                        int32_t sourceX0 = tileOriginX + (blockColumn * 8) + localX;
                        int32_t sourceX1 = sourceX0 + 1;
                        uint8_t paletteIndex0 = sourceX0 >= 0 && sourceX0 < sourceWidth
                            ? sourceIndices[static_cast<std::size_t>(sourceY * sourceWidth + sourceX0)]
                            : static_cast<uint8_t>(0);
                        uint8_t paletteIndex1 = sourceX1 >= 0 && sourceX1 < sourceWidth
                            ? sourceIndices[static_cast<std::size_t>(sourceY * sourceWidth + sourceX1)]
                            : static_cast<uint8_t>(0);
                        int32_t destinationIndex = (blockIndex * 32) + (localY * 4) + (localX / 2);
                        tileBytes[static_cast<std::size_t>(destinationIndex)] = static_cast<uint8_t>((paletteIndex0 & 15) | ((paletteIndex1 & 15) << 4));
                    }
                }
            }
        }

        return tileBytes;
    }

    /// Ensures the requested DS screen owns one initialized BG0 text background ready for shared text submission.
    /// <param name="targetScreen">Physical DS screen whose text background should be ready.</param>
    void NintendoDsRenderManager2D::EnsureScreenTextBackgroundReady(NintendoDsScreenTarget targetScreen) {
        bool targetBottomScreen = targetScreen == NintendoDsScreenTarget::Bottom;
        bool backgroundInitialized = targetBottomScreen ? BottomScreenTextBackgroundInitialized : TopScreenTextBackgroundInitialized;
        if (backgroundInitialized) {
            return;
        }

        if (targetBottomScreen) {
            videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);
            vramSetBankC(VRAM_C_SUB_BG);
            BottomScreenTextBackgroundId = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 31, 0);
            if (BottomScreenTextBackgroundId < 0) {
                return;
            }

            bgSetPriority(BottomScreenTextBackgroundId, 0);
            bgShow(BottomScreenTextBackgroundId);
            BottomScreenTextMapEntries = static_cast<uint16_t*>(bgGetMapPtr(BottomScreenTextBackgroundId));
            BottomScreenTextShadowEntries.fill(static_cast<uint16_t>(0));
            ClearScreenTextMap(targetScreen);

            uint8_t* backgroundGraphics = reinterpret_cast<uint8_t*>(bgGetGfxPtr(BottomScreenTextBackgroundId));
            if (backgroundGraphics != nullptr) {
                constexpr std::array<uint8_t, 32> HProofGlyphTilePixels = {
                    0x01, 0x00, 0x00, 0x10,
                    0x01, 0x00, 0x00, 0x10,
                    0x01, 0x00, 0x00, 0x10,
                    0x11, 0x11, 0x11, 0x11,
                    0x01, 0x00, 0x00, 0x10,
                    0x01, 0x00, 0x00, 0x10,
                    0x01, 0x00, 0x00, 0x10,
                    0x00, 0x00, 0x00, 0x00
                };
                constexpr std::array<uint8_t, 32> EProofGlyphTilePixels = {
                    0x11, 0x11, 0x11, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x11, 0x11, 0x00, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x11, 0x11, 0x11, 0x00,
                    0x00, 0x00, 0x00, 0x00
                };
                constexpr std::array<uint8_t, 32> LProofGlyphTilePixels = {
                    0x01, 0x00, 0x00, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x01, 0x00, 0x00, 0x00,
                    0x11, 0x11, 0x11, 0x11,
                    0x00, 0x00, 0x00, 0x00
                };
                constexpr std::array<uint8_t, 32> OProofGlyphTilePixels = {
                    0x11, 0x11, 0x11, 0x01,
                    0x01, 0x00, 0x00, 0x01,
                    0x01, 0x00, 0x00, 0x01,
                    0x01, 0x00, 0x00, 0x01,
                    0x01, 0x00, 0x00, 0x01,
                    0x01, 0x00, 0x00, 0x01,
                    0x11, 0x11, 0x11, 0x01,
                    0x00, 0x00, 0x00, 0x00
                };
                std::memcpy(backgroundGraphics + (static_cast<std::size_t>(BottomScreenProofHTileIndex) * 32), HProofGlyphTilePixels.data(), HProofGlyphTilePixels.size());
                std::memcpy(backgroundGraphics + (static_cast<std::size_t>(BottomScreenProofETileIndex) * 32), EProofGlyphTilePixels.data(), EProofGlyphTilePixels.size());
                std::memcpy(backgroundGraphics + (static_cast<std::size_t>(BottomScreenProofLTileIndex) * 32), LProofGlyphTilePixels.data(), LProofGlyphTilePixels.size());
                std::memcpy(backgroundGraphics + (static_cast<std::size_t>(BottomScreenProofOTileIndex) * 32), OProofGlyphTilePixels.data(), OProofGlyphTilePixels.size());
            }

            BG_PALETTE_SUB[0] = RGB15(0, 0, 0);
            BG_PALETTE_SUB[1] = RGB15(31, 31, 31);
            BottomScreenTextBackgroundInitialized = true;
            return;
        }

        if (Hardware3DScreenTarget == NintendoDsScreenTarget::None) {
            videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);
        }

        TopScreenTextBackgroundId = bgInit(0, BgType_Text4bpp, BgSize_T_256x256, 31, 0);
        if (TopScreenTextBackgroundId < 0) {
            return;
        }

        bgSetPriority(TopScreenTextBackgroundId, 0);
        bgShow(TopScreenTextBackgroundId);
        TopScreenTextMapEntries = static_cast<uint16_t*>(bgGetMapPtr(TopScreenTextBackgroundId));
        TopScreenTextShadowEntries.fill(static_cast<uint16_t>(0));
        ClearScreenTextMap(targetScreen);

        uint8_t* backgroundGraphics = reinterpret_cast<uint8_t*>(bgGetGfxPtr(TopScreenTextBackgroundId));
        if (backgroundGraphics != nullptr) {
            std::memset(backgroundGraphics, 0, 32);
            constexpr std::array<uint8_t, 32> HProofGlyphTilePixels = {
                0x01, 0x00, 0x00, 0x10,
                0x01, 0x00, 0x00, 0x10,
                0x01, 0x00, 0x00, 0x10,
                0x11, 0x11, 0x11, 0x11,
                0x01, 0x00, 0x00, 0x10,
                0x01, 0x00, 0x00, 0x10,
                0x01, 0x00, 0x00, 0x10,
                0x00, 0x00, 0x00, 0x00
            };
            constexpr std::array<uint8_t, 32> EProofGlyphTilePixels = {
                0x11, 0x11, 0x11, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x11, 0x11, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x11, 0x11, 0x11, 0x00,
                0x00, 0x00, 0x00, 0x00
            };
            constexpr std::array<uint8_t, 32> LProofGlyphTilePixels = {
                0x01, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00,
                0x11, 0x11, 0x11, 0x11,
                0x00, 0x00, 0x00, 0x00
            };
            constexpr std::array<uint8_t, 32> OProofGlyphTilePixels = {
                0x11, 0x11, 0x11, 0x01,
                0x01, 0x00, 0x00, 0x01,
                0x01, 0x00, 0x00, 0x01,
                0x01, 0x00, 0x00, 0x01,
                0x01, 0x00, 0x00, 0x01,
                0x01, 0x00, 0x00, 0x01,
                0x11, 0x11, 0x11, 0x01,
                0x00, 0x00, 0x00, 0x00
            };
            std::memcpy(backgroundGraphics + (static_cast<std::size_t>(TopScreenProofHTileIndex) * 32), HProofGlyphTilePixels.data(), HProofGlyphTilePixels.size());
            std::memcpy(backgroundGraphics + (static_cast<std::size_t>(TopScreenProofETileIndex) * 32), EProofGlyphTilePixels.data(), EProofGlyphTilePixels.size());
            std::memcpy(backgroundGraphics + (static_cast<std::size_t>(TopScreenProofLTileIndex) * 32), LProofGlyphTilePixels.data(), LProofGlyphTilePixels.size());
            std::memcpy(backgroundGraphics + (static_cast<std::size_t>(TopScreenProofOTileIndex) * 32), OProofGlyphTilePixels.data(), OProofGlyphTilePixels.size());
        }

        BG_PALETTE[0] = RGB15(0, 0, 0);
        BG_PALETTE[1] = RGB15(31, 31, 31);
        TopScreenTextBackgroundInitialized = true;
        WriteTopScreenProofText();
    }

    /// Ensures the bottom-screen DS text background exists for direct tile-map text submission.
    void NintendoDsRenderManager2D::EnsureBottomScreenTextBackgroundReady() {
        EnsureScreenTextBackgroundReady(NintendoDsScreenTarget::Bottom);
    }

    /// Ensures the top-screen DS text background exists for direct tile-map text submission.
    void NintendoDsRenderManager2D::EnsureTopScreenTextBackgroundReady() {
        EnsureScreenTextBackgroundReady(NintendoDsScreenTarget::Top);
    }

    /// Clears the bottom-screen DS text background map through the renderer-owned shadow state.
    void NintendoDsRenderManager2D::ClearBottomScreenTextMap() {
        ClearScreenTextMap(NintendoDsScreenTarget::Bottom);
    }

    /// Clears the requested DS text background map through the renderer-owned shadow state.
    /// <param name="targetScreen">Physical DS screen whose text map should be cleared.</param>
    void NintendoDsRenderManager2D::ClearScreenTextMap(NintendoDsScreenTarget targetScreen) {
        bool targetBottomScreen = targetScreen == NintendoDsScreenTarget::Bottom;
        uint16_t* textMapEntries = targetBottomScreen ? BottomScreenTextMapEntries : TopScreenTextMapEntries;
        if (textMapEntries == nullptr) {
            return;
        }

        std::array<uint16_t, 32 * 24>& shadowEntries = targetBottomScreen
            ? BottomScreenTextShadowEntries
            : TopScreenTextShadowEntries;
        shadowEntries.fill(static_cast<uint16_t>(0));
        std::memcpy(
            textMapEntries,
            shadowEntries.data(),
            shadowEntries.size() * sizeof(uint16_t));
    }

    /// Clears the top-screen DS text background map through the renderer-owned shadow state.
    void NintendoDsRenderManager2D::ClearTopScreenTextMap() {
        ClearScreenTextMap(NintendoDsScreenTarget::Top);
    }

    /// Resolves the real demo-disc body font used by the renderer-owned top-screen cooked-font proof line.
    /// <returns>Loaded runtime font asset used by the top-screen cooked-font proof.</returns>
    FontAsset* NintendoDsRenderManager2D::ResolveRequiredTopScreenProofFont() const {
        Core* core = Core::get_Instance();
        if (core == nullptr) {
            throw new InvalidOperationException("Nintendo DS top-screen proof font resolution requires a live Core instance.");
        }

        RuntimeSceneAssetReferenceResolver* resolver = core->get_SceneAssetReferenceResolver();
        if (resolver == nullptr) {
            throw new InvalidOperationException("Nintendo DS top-screen proof font resolution requires a runtime scene asset reference resolver.");
        }

        SceneAssetReference* reference = SceneAssetReferenceFactory::CreateFileSystemFont(TopScreenProofFontRelativePath);
        if (reference == nullptr) {
            throw new InvalidOperationException("Nintendo DS top-screen proof font reference creation failed.");
        }

        auto cleanup = he_cpp_make_scope_exit([&]() {
            delete reference;
        });

        FontAsset* font = resolver->ResolveFont(reference);
        if (font == nullptr) {
            throw new InvalidOperationException("Nintendo DS top-screen proof font resolution returned null.");
        }

        return font;
    }

    /// Writes the renderer-owned top-screen BG0 proof lines: one handwritten control line and one cooked-font line.
    void NintendoDsRenderManager2D::WriteTopScreenProofText() {
        if (TopScreenTextMapEntries == nullptr) {
            return;
        }

        constexpr int32_t ConsoleColumns = FrameBufferWidth / 8;
        constexpr int32_t HandwrittenProofRow = 1;
        constexpr int32_t ProofColumn = 1;
        std::array<uint16_t, 5> proofGlyphTileIndices = {
            TopScreenProofHTileIndex,
            TopScreenProofETileIndex,
            TopScreenProofLTileIndex,
            TopScreenProofLTileIndex,
            TopScreenProofOTileIndex
        };

        ClearTopScreenTextMap();

        int32_t rowOffset = HandwrittenProofRow * ConsoleColumns;
        for (int32_t index = 0; index < static_cast<int32_t>(proofGlyphTileIndices.size()); index++) {
            int32_t mapIndex = rowOffset + ProofColumn + index;
            uint16_t tileIndex = proofGlyphTileIndices[static_cast<std::size_t>(index)];
            TopScreenTextShadowEntries[static_cast<std::size_t>(mapIndex)] = tileIndex;
            TopScreenTextMapEntries[mapIndex] = tileIndex;
        }

        FontAsset* proofFont = ResolveRequiredTopScreenProofFont();
        EnsureTopScreenFontGlyphTilesReady(proofFont);
        WriteTopScreenTextLine(2, 1, "HELLO", 5);
    }

    /// Ensures one dedicated top-screen proof OBJ exists so BG0 and OBJ coexistence can be validated with known content.
    void NintendoDsRenderManager2D::EnsureTopScreenProofSpriteResources() {
        constexpr std::size_t ProofSpriteTileBytes = 32;
        if (TopScreenProofSpriteInitialized) {
            return;
        }

        if (!MainSpriteEngineInitialized) {
            vramSetBankG(VRAM_G_MAIN_SPRITE);
            oamInit(&oamMain, SpriteMapping_1D_32, false);
            oamClear(&oamMain, 0, 128);
            MainSpriteEngineInitialized = true;
        }

        TopScreenProofSpriteGfx = oamAllocateGfx(&oamMain, SpriteSize_8x8, SpriteColorFormat_16Color);
        if (TopScreenProofSpriteGfx == nullptr) {
            return;
        }

        std::memset(TopScreenProofSpriteGfx, 0x11, ProofSpriteTileBytes);
        SPRITE_PALETTE[0] = 0;
        SPRITE_PALETTE[1] = RGB15(31, 31, 31);
        TopScreenProofSpriteInitialized = true;
    }

    /// Submits one dedicated top-screen proof OBJ at a fixed position for BG0 and OBJ coexistence validation.
    void NintendoDsRenderManager2D::SubmitTopScreenProofSprite() {
        constexpr int32_t ProofSpriteId = 0;
        constexpr int32_t ProofSpriteX = 72;
        constexpr int32_t ProofSpriteY = 8;
        EnsureTopScreenProofSpriteResources();
        if (!TopScreenProofSpriteInitialized || TopScreenProofSpriteGfx == nullptr) {
            return;
        }

        oamSet(
            &oamMain,
            ProofSpriteId,
            ProofSpriteX,
            ProofSpriteY,
            0,
            0,
            SpriteSize_8x8,
            SpriteColorFormat_16Color,
            TopScreenProofSpriteGfx,
            -1,
            false,
            false,
            false,
            false,
            false);
    }

    /// Paints one solid-color proof box into the bottom-screen DS text background map.
    /// <param name="column">Left tile column of the proof box.</param>
    /// <param name="row">Top tile row of the proof box.</param>
    /// <param name="widthInTiles">Width of the proof box in tile cells.</param>
    /// <param name="heightInTiles">Height of the proof box in tile cells.</param>
    /// <param name="tileIndex">Tile index used to fill the proof box.</param>
    void NintendoDsRenderManager2D::PaintBottomScreenProofBox(int32_t column, int32_t row, int32_t widthInTiles, int32_t heightInTiles, uint16_t tileIndex) {
        if (BottomScreenTextMapEntries == nullptr) {
            return;
        }

        for (int32_t rowOffset = 0; rowOffset < heightInTiles; rowOffset++) {
            for (int32_t columnOffset = 0; columnOffset < widthInTiles; columnOffset++) {
                int32_t targetRow = row + rowOffset;
                int32_t targetColumn = column + columnOffset;
                if (targetRow < 0 || targetRow >= 24 || targetColumn < 0 || targetColumn >= 32) {
                    continue;
                }

                int32_t mapIndex = (targetRow * 32) + targetColumn;
                BottomScreenTextShadowEntries[static_cast<std::size_t>(mapIndex)] = tileIndex;
                BottomScreenTextMapEntries[mapIndex] = tileIndex;
            }
        }
    }

    /// Ensures the active font has uploaded glyph tiles ready for the requested DS text background submission path.
    /// <param name="targetScreen">Physical DS screen whose text background should receive the glyph tiles.</param>
    /// <param name="font">Font whose cooked glyph atlas should back the requested screen.</param>
    void NintendoDsRenderManager2D::EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget targetScreen, FontAsset* font) {
        if (font == nullptr) {
            throw new ArgumentNullException("font");
        }

        bool targetBottomScreen = targetScreen == NintendoDsScreenTarget::Bottom;
        bool& glyphTilesUploaded = targetBottomScreen ? BottomScreenTextGlyphTilesUploaded : TopScreenTextGlyphTilesUploaded;
        FontAsset*& glyphCacheFont = targetBottomScreen ? BottomScreenTextGlyphCacheFont : TopScreenTextGlyphCacheFont;
        std::array<uint16_t, 95>& glyphTileIndices = targetBottomScreen ? BottomScreenTextGlyphTileIndices : TopScreenTextGlyphTileIndices;
        std::string& glyphResolveFailureReason = targetBottomScreen ? BottomScreenGlyphResolveFailureReason : TopScreenGlyphResolveFailureReason;
        if (glyphTilesUploaded && glyphCacheFont == font) {
            return;
        }

        EnsureScreenTextBackgroundReady(targetScreen);
        glyphTileIndices.fill(static_cast<uint16_t>(0));
        glyphCacheFont = font;
        glyphTilesUploaded = false;
        NintendoDsRuntimeTexture2D* runtimeTexture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(font->get_Texture());
        if (runtimeTexture == nullptr) {
            glyphResolveFailureReason = "glyphTextureNull";
            return;
        }
        if (runtimeTexture->ColorFormat != TextureAssetColorFormat::Indexed4) {
            glyphResolveFailureReason = "glyphTextureFormat";
            return;
        }
        if (runtimeTexture->Colors == nullptr
            || runtimeTexture->Colors->Data == nullptr
            || runtimeTexture->PaletteColors == nullptr
            || runtimeTexture->PaletteColors->Data == nullptr
            || runtimeTexture->get_Width() <= 0
            || runtimeTexture->get_Height() <= 0) {
            glyphResolveFailureReason = "glyphTextureData";
            return;
        }

        int32_t backgroundId = targetBottomScreen ? BottomScreenTextBackgroundId : TopScreenTextBackgroundId;
        if (backgroundId < 0) {
            glyphResolveFailureReason = "glyphBackground";
            return;
        }

        uint8_t* backgroundGraphics = reinterpret_cast<uint8_t*>(bgGetGfxPtr(backgroundId));
        if (backgroundGraphics == nullptr) {
            glyphResolveFailureReason = "glyphBackground";
            return;
        }

        std::memset(
            backgroundGraphics,
            0,
            static_cast<std::size_t>((glyphTileIndices.size() + 1) * 32));
        for (int32_t paletteIndex = 0; paletteIndex < 16; paletteIndex++) {
            if (targetBottomScreen) {
                BG_PALETTE_SUB[paletteIndex] = 0;
            } else {
                BG_PALETTE[paletteIndex] = 0;
            }
        }
        if (targetBottomScreen) {
            BG_PALETTE_SUB[1] = RGB15(31, 31, 31);
        } else {
            BG_PALETTE[1] = RGB15(31, 31, 31);
        }

        int32_t availablePaletteEntries = std::min<int32_t>(static_cast<int32_t>(runtimeTexture->PaletteColors->Length / 4), 16);
        std::array<uint8_t, 16> paletteIndexRemap {};
        for (int32_t paletteIndex = 0; paletteIndex < availablePaletteEntries; paletteIndex++) {
            int32_t paletteOffset = paletteIndex * 4;
            uint8_t alpha = runtimeTexture->PaletteColors->Data[paletteOffset + 3];
            paletteIndexRemap[static_cast<std::size_t>(paletteIndex)] = alpha > 0 ? 1 : 0;
        }

        if (font->get_Characters() == nullptr) {
            glyphResolveFailureReason = "glyphCharacterMap";
            return;
        }

        for (const auto& characterEntry : *font->get_Characters()) {
            char character = characterEntry.first;
            int32_t characterCode = static_cast<int32_t>(static_cast<unsigned char>(character));
            if (characterCode < 32 || characterCode > 126) {
                continue;
            }

            const FontChar& glyph = characterEntry.second;

            int32_t sourceX = static_cast<int32_t>(std::round(glyph.SourceRect.X * static_cast<float>(font->get_AtlasWidth())));
            int32_t sourceY = static_cast<int32_t>(std::round(glyph.SourceRect.Y * static_cast<float>(font->get_AtlasHeight())));
            int32_t sourceWidth = static_cast<int32_t>(std::round(glyph.SourceRect.Z * static_cast<float>(font->get_AtlasWidth())));
            int32_t sourceHeight = static_cast<int32_t>(std::round(glyph.SourceRect.W * static_cast<float>(font->get_AtlasHeight())));
            if (sourceWidth < 1
                || sourceHeight < 1
                || sourceX < 0
                || sourceY < 0
                || sourceX + sourceWidth > runtimeTexture->get_Width()
                || sourceY + sourceHeight > runtimeTexture->get_Height()) {
                glyphResolveFailureReason = "glyphSourceRect";
                continue;
            }

            uint16_t tileIndex = static_cast<uint16_t>((characterCode - 32) + 1);
            std::array<uint8_t, 32> tilePixels {};
            int32_t tileCopyWidth = std::min(sourceWidth, static_cast<int32_t>(8));
            int32_t tileCopyHeight = std::min(sourceHeight, static_cast<int32_t>(8));
            int32_t sourceStartY = std::max(sourceHeight - tileCopyHeight, static_cast<int32_t>(0));
            int32_t destinationOffsetX = std::max(static_cast<int32_t>(8) - tileCopyWidth, static_cast<int32_t>(0));
            int32_t destinationOffsetY = std::max(static_cast<int32_t>(8) - tileCopyHeight, static_cast<int32_t>(0));
            for (int32_t y = 0; y < tileCopyHeight; y++) {
                for (int32_t x = 0; x < tileCopyWidth; x++) {
                    int32_t sourcePixelIndex = ((sourceY + sourceStartY + y) * runtimeTexture->get_Width()) + (sourceX + x);
                    uint8_t paletteIndex = static_cast<uint8_t>(runtimeTexture->Colors->Data[sourcePixelIndex / 2] & 15);
                    if ((sourcePixelIndex & 1) != 0) {
                        paletteIndex = static_cast<uint8_t>((runtimeTexture->Colors->Data[sourcePixelIndex / 2] >> 4) & 15);
                    }

                    paletteIndex = paletteIndex >= availablePaletteEntries
                        ? static_cast<uint8_t>(0)
                        : paletteIndexRemap[static_cast<std::size_t>(paletteIndex)];

                    int32_t destinationX = destinationOffsetX + x;
                    int32_t destinationY = destinationOffsetY + y;
                    std::size_t tileByteIndex = static_cast<std::size_t>(destinationY * 4) + static_cast<std::size_t>(destinationX / 2);
                    if ((destinationX & 1) == 0) {
                        tilePixels[tileByteIndex] = static_cast<uint8_t>((tilePixels[tileByteIndex] & 0xF0) | (paletteIndex & 0x0F));
                    } else {
                        tilePixels[tileByteIndex] = static_cast<uint8_t>((tilePixels[tileByteIndex] & 0x0F) | ((paletteIndex & 0x0F) << 4));
                    }
                }
            }

            std::memcpy(
                backgroundGraphics + (static_cast<std::size_t>(tileIndex) * 32),
                tilePixels.data(),
                tilePixels.size());
            glyphTileIndices[static_cast<std::size_t>(characterCode - 32)] = tileIndex;
        }

        glyphTilesUploaded = true;
        glyphResolveFailureReason.clear();
    }

    /// Ensures the active font has uploaded glyph tiles ready for bottom-screen DS text-background submission.
    /// <param name="font">Font whose cooked glyph atlas should back the bottom-screen text background.</param>
    void NintendoDsRenderManager2D::EnsureBottomScreenFontGlyphTilesReady(FontAsset* font) {
        EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget::Bottom, font);
    }

    /// Ensures the active font has uploaded glyph tiles ready for top-screen DS text-background submission.
    /// <param name="font">Font whose cooked glyph atlas should back the top-screen text background.</param>
    void NintendoDsRenderManager2D::EnsureTopScreenFontGlyphTilesReady(FontAsset* font) {
        EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget::Top, font);
    }

    /// Resolves one printable character into the uploaded DS text-background tile index for the requested screen.
    /// <param name="targetScreen">Physical DS screen whose text background glyph cache should be queried.</param>
    /// <param name="font">Font whose uploaded glyph cache should be queried.</param>
    /// <param name="character">Printable character to map.</param>
    /// <param name="tileIndex">Receives the uploaded tile index when the glyph is available.</param>
    /// <returns>True when the glyph was uploaded and can be referenced from the requested text background map.</returns>
    bool NintendoDsRenderManager2D::TryResolveScreenGlyphTileIndex(NintendoDsScreenTarget targetScreen, FontAsset* font, char character, uint16_t& tileIndex) {
        tileIndex = 0;
        if (font == nullptr || character < 32 || character > 126) {
            return false;
        }

        EnsureScreenFontGlyphTilesReady(targetScreen, font);
        std::string& glyphResolveFailureReason = targetScreen == NintendoDsScreenTarget::Bottom
            ? BottomScreenGlyphResolveFailureReason
            : TopScreenGlyphResolveFailureReason;
        if (!glyphResolveFailureReason.empty()) {
            return false;
        }

        std::array<uint16_t, 95>& glyphTileIndices = targetScreen == NintendoDsScreenTarget::Bottom
            ? BottomScreenTextGlyphTileIndices
            : TopScreenTextGlyphTileIndices;
        uint16_t resolvedTileIndex = glyphTileIndices[static_cast<std::size_t>(character - 32)];
        if (resolvedTileIndex == 0) {
            glyphResolveFailureReason = "glyphTileZero";
            return false;
        }

        glyphResolveFailureReason.clear();
        tileIndex = resolvedTileIndex;
        return true;
    }

    /// Resolves one printable character into the uploaded DS text-background tile index for the active font.
    /// <param name="font">Font whose uploaded glyph cache should be queried.</param>
    /// <param name="character">Printable character to map.</param>
    /// <param name="tileIndex">Receives the uploaded tile index when the glyph is available.</param>
    /// <returns>True when the glyph was uploaded and can be referenced from the text background map.</returns>
    bool NintendoDsRenderManager2D::TryResolveBottomScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex) {
        return TryResolveScreenGlyphTileIndex(NintendoDsScreenTarget::Bottom, font, character, tileIndex);
    }

    /// Resolves one printable character into the uploaded top-screen DS text-background tile index for the active font.
    /// <param name="font">Font whose uploaded glyph cache should be queried.</param>
    /// <param name="character">Printable character to map.</param>
    /// <param name="tileIndex">Receives the uploaded tile index when the glyph is available.</param>
    /// <returns>True when the glyph was uploaded and can be referenced from the top-screen text background map.</returns>
    bool NintendoDsRenderManager2D::TryResolveTopScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex) {
        return TryResolveScreenGlyphTileIndex(NintendoDsScreenTarget::Top, font, character, tileIndex);
    }

    /// Writes one text line into the requested DS text background at the requested cell position.
    /// <param name="targetScreen">Physical DS screen whose text background should be written.</param>
    /// <param name="row">Zero-based text row.</param>
    /// <param name="column">Zero-based text column.</param>
    /// <param name="line">Visible line content to write.</param>
    /// <param name="visibleColumnCount">Number of writable columns in the row segment.</param>
    void NintendoDsRenderManager2D::WriteScreenTextLine(NintendoDsScreenTarget targetScreen, int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount) {
        bool targetBottomScreen = targetScreen == NintendoDsScreenTarget::Bottom;
        uint16_t* textMapEntries = targetBottomScreen ? BottomScreenTextMapEntries : TopScreenTextMapEntries;
        if (textMapEntries == nullptr) {
            return;
        }

        constexpr int32_t ConsoleColumns = FrameBufferWidth / 8;
        int32_t safeColumn = std::clamp(column, static_cast<int32_t>(0), ConsoleColumns - 1);
        int32_t writableColumns = std::clamp(visibleColumnCount, static_cast<int32_t>(0), ConsoleColumns - safeColumn);
        int32_t rowOffset = row * ConsoleColumns;
        FontAsset* glyphCacheFont = targetBottomScreen ? BottomScreenTextGlyphCacheFont : TopScreenTextGlyphCacheFont;
        std::array<uint16_t, 32 * 24>& shadowEntries = targetBottomScreen ? BottomScreenTextShadowEntries : TopScreenTextShadowEntries;
        for (int32_t index = 0; index < writableColumns; index++) {
            uint16_t tileIndex = 0;
            if (index < static_cast<int32_t>(line.size())) {
                char character = line[static_cast<std::size_t>(index)];
                if (!TryResolveScreenGlyphTileIndex(targetScreen, glyphCacheFont, character, tileIndex)) {
                    tileIndex = 0;
                }
            }

            int32_t mapIndex = rowOffset + safeColumn + index;
            shadowEntries[static_cast<std::size_t>(mapIndex)] = tileIndex;
            textMapEntries[mapIndex] = tileIndex;
        }
    }

    /// Writes one text line into the bottom-screen DS text background at the requested cell position.
    void NintendoDsRenderManager2D::WriteBottomScreenTextLine(int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount) {
        WriteScreenTextLine(NintendoDsScreenTarget::Bottom, row, column, line, visibleColumnCount);
    }

    /// Writes one text line into the top-screen DS text background at the requested cell position.
    /// <param name="row">Zero-based text row.</param>
    /// <param name="column">Zero-based text column.</param>
    /// <param name="line">Visible line content to write.</param>
    /// <param name="visibleColumnCount">Number of writable columns in the row segment.</param>
    void NintendoDsRenderManager2D::WriteTopScreenTextLine(int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount) {
        WriteScreenTextLine(NintendoDsScreenTarget::Top, row, column, line, visibleColumnCount);
    }

    /// Resolves one printable ASCII character into the DS text-background glyph tile index.
    uint16_t NintendoDsRenderManager2D::ResolveScreenGlyphTileIndex(NintendoDsScreenTarget targetScreen, char character) const {
        FontAsset* glyphCacheFont = targetScreen == NintendoDsScreenTarget::Bottom ? BottomScreenTextGlyphCacheFont : TopScreenTextGlyphCacheFont;
        if (glyphCacheFont == nullptr || character < 32 || character > 126) {
            return 0;
        }

        const std::array<uint16_t, 95>& glyphTileIndices = targetScreen == NintendoDsScreenTarget::Bottom
            ? BottomScreenTextGlyphTileIndices
            : TopScreenTextGlyphTileIndices;
        return glyphTileIndices[static_cast<std::size_t>(character - 32)];
    }

    /// Resolves one printable ASCII character into the DS text-background glyph tile index.
    uint16_t NintendoDsRenderManager2D::ResolveBottomScreenGlyphTileIndex(char character) const {
        return ResolveScreenGlyphTileIndex(NintendoDsScreenTarget::Bottom, character);
    }

    /// Resolves one printable ASCII character into the top-screen DS text-background glyph tile index.
    uint16_t NintendoDsRenderManager2D::ResolveTopScreenGlyphTileIndex(char character) const {
        return ResolveScreenGlyphTileIndex(NintendoDsScreenTarget::Top, character);
    }

    /// Attempts to submit one text drawable through a DS hardware-backed path.
    /// <param name="text">Text drawable to evaluate.</param>
    /// <returns>True when the text was submitted to DS hardware.</returns>
    bool NintendoDsRenderManager2D::TryDrawHardwareText(ITextDrawable2D* text) {
        if (text == nullptr) {
            TraceUnsupportedTextDrawable(text, "null");
            return false;
        }

        if (!ActiveViewportTargetsBottomScreen) {
            EnsureTopScreenTextBackgroundReady();
            return true;
        }

        FontAsset* font = text->get_Font();
        if (font == nullptr) {
            TraceUnsupportedTextDrawable(text, "font");
            return false;
        }

        Entity* parent = text->get_Parent();
        if (parent == nullptr) {
            TraceUnsupportedTextDrawable(text, "parent");
            return false;
        }

        float fontScale = text->get_FontScale();
        if (fontScale <= 0.001f) {
            TraceUnsupportedTextDrawable(text, "fontScale");
            return false;
        }

        if (text->get_WrapText()) {
            TraceUnsupportedTextDrawable(text, "wrap");
            return false;
        }

        int32_t alignment = static_cast<int32_t>(text->get_Alignment());
        if (alignment < 0 || alignment > 2) {
            TraceUnsupportedTextDrawable(text, "alignment");
            return false;
        }

        byte4 color = text->get_Color();
        if (color.W == 0) {
            TraceUnsupportedTextDrawable(text, "color");
            return false;
        }

        float4 sourceRect = text->get_SourceRect();
        if (std::abs(sourceRect.X) > 0.001f
            || std::abs(sourceRect.Y) > 0.001f
            || std::abs(sourceRect.Z - 1.0f) > 0.001f
            || std::abs(sourceRect.W - 1.0f) > 0.001f) {
            TraceUnsupportedTextDrawable(text, "sourceRect");
            return false;
        }

        const std::string& content = text->get_Text();
        if (content.empty()) {
            return true;
        }

        for (char character : content) {
            if (character != ' ' && (static_cast<unsigned char>(character) < 32 || static_cast<unsigned char>(character) > 126)) {
                TraceUnsupportedTextDrawable(text, "charset");
                return false;
            }
        }

        std::size_t lineBreakIndex = content.find('\n');
        std::string visibleLine = lineBreakIndex == std::string::npos
            ? content
            : content.substr(0, lineBreakIndex);
        if (ActiveViewportTargetsBottomScreen) {
            if (!BottomScreenPresentationEnabled) {
                TraceUnsupportedTextDrawable(text, "screen");
                return false;
            }
            if (text->get_RenderOrder2D() == 220) {
                EnsureBottomScreenTextBackgroundReady();
            } else {
                TraceUnsupportedTextDrawable(text, "renderOrder");
                return false;
            }
            if (BottomScreenTextMapEntries == nullptr) {
                TraceUnsupportedTextDrawable(text, "glyphMap");
                return false;
            }

            constexpr int32_t MaximumBottomScreenRuntimeRows = 6;
            if (BottomScreenSubmittedTextCountThisFrame >= MaximumBottomScreenRuntimeRows) {
                return true;
            }

            EnsureBottomScreenFontGlyphTilesReady(font);
            if (!BottomScreenGlyphResolveFailureReason.empty()) {
                TraceUnsupportedTextDrawable(text, BottomScreenGlyphResolveFailureReason.c_str());
                return false;
            }

            constexpr int32_t BottomScreenRuntimeTextRow = 1;
            constexpr int32_t BottomScreenRuntimeTextColumn = 1;
            constexpr int32_t BottomScreenConsoleColumns = FrameBufferWidth / 8;
            int32_t proofRow = BottomScreenRuntimeTextRow + BottomScreenSubmittedTextCountThisFrame;
            if (visibleLine.empty()) {
                BottomScreenSubmittedTextCountThisFrame++;
                return true;
            }

            int32_t writableColumnCount = BottomScreenConsoleColumns - BottomScreenRuntimeTextColumn;
            WriteBottomScreenTextLine(proofRow, BottomScreenRuntimeTextColumn, visibleLine, writableColumnCount);

            BottomScreenSubmittedTextCountThisFrame++;
            return true;
        }

        if (text->get_RenderOrder2D() == 42) {
            EnsureTopScreenTextBackgroundReady();
        } else {
            TraceUnsupportedTextDrawable(text, "renderOrder");
            return false;
        }
        if (TopScreenTextMapEntries == nullptr) {
            TraceUnsupportedTextDrawable(text, "glyphMap");
            return false;
        }

        EnsureTopScreenFontGlyphTilesReady(font);
        if (!TopScreenGlyphResolveFailureReason.empty()) {
            TraceUnsupportedTextDrawable(text, TopScreenGlyphResolveFailureReason.c_str());
            return false;
        }

        if (visibleLine.empty()) {
            return true;
        }

        constexpr int32_t ConsoleColumns = FrameBufferWidth / 8;
        constexpr int32_t ConsoleRows = VisibleScreenHeight / 8;
        float3 parentPosition = parent->get_Position();
        int32_t baseColumn = static_cast<int32_t>(std::round((parentPosition.X - static_cast<float>(ActiveViewportOffsetX)) / 8.0f));
        int32_t baseRow = static_cast<int32_t>(std::round((parentPosition.Y - static_cast<float>(ActiveViewportOffsetY)) / 8.0f));
        int2 textSize = text->get_Size();
        int32_t visibleLength = std::min<int32_t>(static_cast<int32_t>(visibleLine.size()), ConsoleColumns);
        int32_t textBoxWidth = textSize.X > 1 ? static_cast<int32_t>(textSize.X) : 1;
        int32_t boxColumnCount = std::max<int32_t>(
            1,
            static_cast<int32_t>(std::ceil(static_cast<double>(textBoxWidth) / 8.0)));
        int32_t startColumn = ResolveAlignedConsoleColumn(baseColumn, boxColumnCount, visibleLength, alignment);
        int32_t targetRow = std::clamp(baseRow, static_cast<int32_t>(0), ConsoleRows - 1);
        int32_t writableColumnCount = std::min<int32_t>(
            std::max(boxColumnCount, visibleLength),
            ConsoleColumns - startColumn);
        WriteTopScreenTextLine(targetRow, startColumn, visibleLine, writableColumnCount);
        return true;
    }

    /// Resolves the console start column for one aligned text run inside its authored text box.
    /// <param name="baseColumn">Left-edge console column derived from the drawable position.</param>
    /// <param name="boxColumnCount">Width of the authored text box expressed in console columns.</param>
    /// <param name="visibleLength">Visible text length expressed in console columns.</param>
    /// <param name="alignment">Generated-core text alignment value.</param>
    /// <returns>Console start column clamped to the visible DS text grid.</returns>
    int32_t NintendoDsRenderManager2D::ResolveAlignedConsoleColumn(int32_t baseColumn, int32_t boxColumnCount, int32_t visibleLength, int32_t alignment) const {
        int32_t safeBoxColumnCount = std::max(boxColumnCount, visibleLength);
        int32_t offsetColumns = 0;
        if (alignment == 1) {
            offsetColumns = std::max((safeBoxColumnCount - visibleLength) / 2, static_cast<int32_t>(0));
        } else if (alignment == 2) {
            offsetColumns = std::max(safeBoxColumnCount - visibleLength, static_cast<int32_t>(0));
        }

        int32_t maximumColumn = (FrameBufferWidth / 8) - 1;
        return std::clamp(baseColumn + offsetColumns, static_cast<int32_t>(0), maximumColumn);
    }

    /// Emits one debug-only host trace for one unsupported text drawable without touching the DS console.
    /// <param name="text">Text drawable that could not be expressed through DS hardware.</param>
    /// <param name="reason">Short reject reason label.</param>
    void NintendoDsRenderManager2D::TraceUnsupportedTextDrawable(ITextDrawable2D* text, const char* reason) {
        if (ActiveViewportTargetsBottomScreen) {
            BottomScreenUnsupportedTextCountThisFrame++;
            if (BottomScreenUnsupportedTextReasonThisFrame.empty() && reason != nullptr) {
                BottomScreenUnsupportedTextReasonThisFrame = reason;
            }
            if (BottomScreenUnsupportedTextSampleThisFrame.empty() && text != nullptr) {
                BottomScreenUnsupportedTextSampleThisFrame = text->get_Text();
            }
        } else {
            std::string line = "[helengine-ds] top-text-reject reason=";
            line += reason == nullptr ? "unknown" : reason;
            if (text != nullptr) {
                line += " renderOrder=" + std::to_string(text->get_RenderOrder2D());
                line += " text=" + text->get_Text();
            }

            AppendTopScreenRejectTraceLine(line);
            int2 marker = ResolveUnsupportedDrawableMarkerPosition(text);
            byte4 markerColor(255, 0, 0, 255);
            if (reason != nullptr && std::strcmp(reason, "glyphMap") == 0) {
                markerColor = byte4(0, 255, 0, 255);
            } else if (reason != nullptr && std::strcmp(reason, "renderOrder") == 0) {
                markerColor = byte4(255, 255, 0, 255);
            } else if (reason != nullptr && std::strcmp(reason, "screen") == 0) {
                markerColor = byte4(255, 0, 0, 255);
            } else if (reason != nullptr && std::strncmp(reason, "glyph", 5) == 0) {
                markerColor = byte4(0, 255, 255, 255);
            }

            TryDrawSolidHardwareRectangle(marker.X, marker.Y, 16, 16, markerColor);
        }

#if defined(HELENGINE_DS_UNSUPPORTED_TRACE_ENABLED)
        constexpr int32_t MaximumUnsupportedTraceLinesPerFrame = 8;
        if (UnsupportedTextTraceCountThisFrame >= MaximumUnsupportedTraceLinesPerFrame) {
            return;
        }

        UnsupportedTextTraceCountThisFrame++;
        int2 anchor = ResolveUnsupportedDrawableMarkerPosition(text);
        const char* safeReason = reason == nullptr ? "unknown" : reason;
        const char* screenName = ActiveViewportTargetsBottomScreen ? "bottom" : "top";
        std::fprintf(stderr, "[helengine-ds] unsupported text reason=%s screen=%s pos=%d,%d\n", safeReason, screenName, static_cast<int>(anchor.X), static_cast<int>(anchor.Y));
        std::fflush(stderr);
        std::printf("[helengine-ds] unsupported text reason=%s screen=%s pos=%d,%d\n", safeReason, screenName, static_cast<int>(anchor.X), static_cast<int>(anchor.Y));
        std::fflush(stdout);
#else
        (void)text;
        (void)reason;
#endif
    }

    /// Emits one debug-only host trace for one unsupported sprite drawable without touching the DS console.
    /// <param name="sprite">Sprite drawable that could not be expressed through DS hardware.</param>
    /// <param name="reason">Short reject reason label.</param>
    void NintendoDsRenderManager2D::TraceUnsupportedSpriteDrawable(ISpriteDrawable2D* sprite, const char* reason) {
        if (!ActiveViewportTargetsBottomScreen) {
            std::string line = "[helengine-ds] top-sprite-reject reason=";
            line += reason == nullptr ? "unknown" : reason;
            if (sprite != nullptr) {
                int2 spriteSize = sprite->get_Size();
                line += " renderOrder=" + std::to_string(sprite->get_RenderOrder2D());
                line += " size=" + std::to_string(spriteSize.X) + "x" + std::to_string(spriteSize.Y);
            }

            AppendTopScreenRejectTraceLine(line);
            int2 marker = ResolveUnsupportedDrawableMarkerPosition(sprite);
            byte4 markerColor(255, 0, 255, 255);
            if (reason != nullptr && std::strcmp(reason, "textureSize") == 0) {
                markerColor = byte4(255, 0, 255, 255);
            } else if (reason != nullptr && std::strcmp(reason, "prepare") == 0) {
                markerColor = byte4(0, 255, 255, 255);
            } else if (reason != nullptr && std::strcmp(reason, "size") == 0) {
                markerColor = byte4(255, 255, 0, 255);
            } else if (reason != nullptr && std::strcmp(reason, "texture") == 0) {
                markerColor = byte4(255, 0, 0, 255);
            }

            TryDrawSolidHardwareRectangle(marker.X, marker.Y, 16, 16, markerColor);
        }

#if defined(HELENGINE_DS_UNSUPPORTED_TRACE_ENABLED)
        constexpr int32_t MaximumUnsupportedTraceLinesPerFrame = 8;
        if (UnsupportedSpriteTraceCountThisFrame >= MaximumUnsupportedTraceLinesPerFrame) {
            return;
        }

        UnsupportedSpriteTraceCountThisFrame++;
        int2 anchor = ResolveUnsupportedDrawableMarkerPosition(sprite);
        const char* safeReason = reason == nullptr ? "unknown" : reason;
        const char* screenName = ActiveViewportTargetsBottomScreen ? "bottom" : "top";
        std::fprintf(stderr, "[helengine-ds] unsupported sprite reason=%s screen=%s pos=%d,%d\n", safeReason, screenName, static_cast<int>(anchor.X), static_cast<int>(anchor.Y));
        std::fflush(stderr);
        std::printf("[helengine-ds] unsupported sprite reason=%s screen=%s pos=%d,%d\n", safeReason, screenName, static_cast<int>(anchor.X), static_cast<int>(anchor.Y));
        std::fflush(stdout);
#else
        (void)sprite;
        (void)reason;
#endif
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
        if (targetScreen == NintendoDsScreenTarget::Top) {
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

        if (!MainDebugMarkerInitialized) {
            if (!MainSpriteEngineInitialized) {
                vramSetBankG(VRAM_G_MAIN_SPRITE);
                oamInit(&oamMain, SpriteMapping_1D_32, false);
                oamClear(&oamMain, 0, 128);
                MainSpriteEngineInitialized = true;
            }

            MainDebugMarkerGfx = oamAllocateGfx(&oamMain, SpriteSize_8x8, SpriteColorFormat_16Color);
            std::memset(MainDebugMarkerGfx, 0x11, UnsupportedDebugMarkerTileBytes);
            SPRITE_PALETTE[0] = 0;
            SPRITE_PALETTE[1] = UnsupportedDebugMarkerColor;
            MainDebugMarkerInitialized = true;
        }
#else
        (void)targetScreen;
#endif
    }

}
#endif
