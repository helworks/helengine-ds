#include "platform/ds/NintendoDsRenderManager2D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

extern "C" {
#include <nds/arm9/background.h>
#include <nds/interrupts.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/system.h>
#include <nds/timers.h>
}

#include "Entity.hpp"
#include "Asset.hpp"
#include "AssetSerializer.hpp"
#include "CameraClearSettings.hpp"
#include "Component.hpp"
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

        /// Stable packaged cooked font path used by the renderer-owned top-screen real-font proof line.
        constexpr const char* TopScreenProofFontRelativePath = "cooked/fonts/demodiscbody.hefont";

        /// OBJ priority used for top-screen sprites so they can remain in front of lower-priority backgrounds.
        constexpr int32_t TopScreenSpritePriority = 0;

        /// OBJ priority used for bottom-screen sprites so BG0 text remains visible in front of background art.
        constexpr int32_t BottomScreenSpritePriority = 1;

        /// Indicates whether the current DS run has already recorded the first top-screen queue count.
        bool TopScreenQueueTraceRecorded = false;

        /// Number of top-screen 2D primitive visit lines already captured for the current DS run.
        int32_t TopScreenVisitTraceCount = 0;

        /// Stores the host-visible trace path used for top-screen sprite rejection diagnostics.
        constexpr const char* TopScreenRejectTracePath = "C:/tmp/helengine-ds-logs/helengine-ds-top-screen-reject.log";

        /// Stores whether the current process has already reset the top-screen reject trace file.
        bool TopScreenRejectTraceReset = false;

        /// Caps the number of host-side top-screen reject lines recorded during one DS run.
        int32_t TopScreenRejectTraceLineCount = 0;

        /// Suppresses bottom-screen text tracing so runtime performance metrics are not polluted by host-side file I/O.
        /// <param name="line">Trace payload to append.</param>
        void AppendBottomScreenTextTraceLine(const std::string& line) {
            (void)line;
        }

        /// Suppresses top-screen reject tracing so runtime performance metrics are not polluted by host-side file I/O.
        /// <param name="line">Trace payload to append.</param>
        void AppendTopScreenRejectTraceLine(const std::string& line) {
            if (line.empty()) {
                return;
            }

            constexpr int32_t MaximumTopScreenRejectTraceLineCount = 64;
            if (TopScreenRejectTraceLineCount >= MaximumTopScreenRejectTraceLineCount) {
                return;
            }

            FILE* file = std::fopen(TopScreenRejectTracePath, TopScreenRejectTraceReset ? "ab" : "wb");
            if (file == nullptr) {
                return;
            }

            TopScreenRejectTraceReset = true;
            std::fwrite(line.data(), sizeof(char), line.size(), file);
            std::fwrite("\n", sizeof(char), 1, file);
            std::fflush(file);
            std::fclose(file);
            TopScreenRejectTraceLineCount++;
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

        /// Enables one-time BeginFrame stage markers while isolating the DS draw hang.
        constexpr bool EnableBeginFrameStageMarkers = true;

        /// Stores whether the one-time BeginFrame stage markers have already completed.
        bool BeginFrameStageMarkersCompleted = false;

        /// Paints one visible top-screen diagnostic marker into the bootstrap bitmap.
        /// <param name="color">Marker color to present.</param>
        void PaintBeginFrameStageMarker(uint16_t color) {
            if (!EnableBeginFrameStageMarkers || BeginFrameStageMarkersCompleted) {
                return;
            }

            uint16_t* frameBuffer = reinterpret_cast<uint16_t*>(BG_BMP_RAM(0));
            if (frameBuffer == nullptr) {
                return;
            }

            constexpr int32_t MarkerSize = 12;
            constexpr int32_t MarkerOffsetX = 16;
            for (int32_t row = 0; row < MarkerSize; row++) {
                for (int32_t column = 0; column < MarkerSize; column++) {
                    frameBuffer[(row * 256) + MarkerOffsetX + column] = color;
                }
            }

            for (int32_t frameIndex = 0; frameIndex < 20; frameIndex++) {
                swiWaitForVBlank();
            }
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
        , TopScreenClearedThisFrame(false)
        , RuntimeHeartbeatFrameIndex(-1)
        , BottomScreenTextBackgroundId(-1)
        , TopScreenTextBackgroundId(-1)
        , BottomScreenTextMapEntries(nullptr)
        , TopScreenTextMapEntries(nullptr)
        , BottomScreenTextShadowEntries()
        , TopScreenTextShadowEntries()
        , HardwareTextSubmissionStates()
        , DeferredHardwareTextSubmissionClears()
        , TextSubmissionFrameStamp(0)
        , BottomScreenTextBackgroundInitialized(false)
        , TopScreenTextBackgroundInitialized(false)
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
        , NextMainAffineSpriteMatrixId(0)
        , NextSubAffineSpriteMatrixId(0)
        , NextMainSpritePaletteBank(0)
        , NextSubSpritePaletteBank(0)
        , MainSolidRectanglePaletteColors()
        , SubSolidRectanglePaletteColors()
        , FrameLocalMainRectangleGraphics()
        , FrameLocalSubRectangleGraphics()
        , MainDebugMarkerInitialized(false)
        , MainSpriteEngineInitialized(false)
        , MainSpriteEngineExtendedPalettesEnabled(false)
        , SubSpriteEngineInitialized(false)
        , SubDebugMarkerInitialized(false)
        , MainDebugMarkerGfx(nullptr)
        , SubDebugMarkerGfx(nullptr)
        , UnsupportedSpriteLoggedThisFrame(false)
        , UnsupportedTextLoggedThisFrame(false)
        , UnsupportedRoundedRectLoggedThisFrame(false)
        , UnsupportedTextTraceCountThisFrame(0)
        , UnsupportedSpriteTraceCountThisFrame(0)
        , BottomScreenSubmittedTextCountThisFrame(0)
        , LastTopScreenQueueCount(0)
        , LastBottomScreenQueueCount(0)
        , PreviousBottomScreenQueueCount(-1)
        , PreviousPlatformOwnedOverlayRow(0)
        , PreviousPlatformOwnedOverlayColumn(0)
        , PreviousPlatformOwnedOverlayRowSpan(0)
        , PreviousPlatformOwnedOverlayRowStep(1)
        , PreviousPlatformOwnedOverlayVisibleColumnCount(0)
        , PreviousPlatformOwnedOverlayLines()
        , BottomScreenUnsupportedTextCountThisFrame(0)
        , BottomScreenUnsupportedTextReasonThisFrame()
        , BottomScreenUnsupportedTextSampleThisFrame()
        , TopScreenUnsupportedSpriteReasonThisFrame()
        , TopScreenUnsupportedSpriteRenderOrderThisFrame(0)
        , TopScreenUnsupportedSpriteWidthThisFrame(0)
        , TopScreenUnsupportedSpriteHeightThisFrame(0)
        , TopScreenUnsupportedSpriteTextureWidthThisFrame(0)
        , TopScreenUnsupportedSpriteTextureHeightThisFrame(0)
        , ProfileTotalFrameMilliseconds(0.0)
        , ProfileCameraMilliseconds(0.0)
        , ProfileTextMilliseconds(0.0)
        , ProfileSpriteMilliseconds(0.0)
        , ProfileRoundedRectMilliseconds(0.0)
        , ProfileClearMilliseconds(0.0)
        , ProfileTextPrimitiveCount(0)
        , ProfileTextCacheHitCount(0)
        , ProfileTextRewriteCount(0)
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

        if (dsTexture->HardwareTextureUploaded || dsTexture->HardwareTextureId >= 0) {
            glDeleteTextures(1, &dsTexture->HardwareTextureId);
            dsTexture->HardwareTextureId = -1;
            dsTexture->HardwareTextureUploaded = false;
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
        dsTexture->MainHardwareSpriteUses256Color = false;
        dsTexture->SubHardwareSpriteUses256Color = false;
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
        PaintBeginFrameStageMarker(RGB15(31, 0, 0) | BIT(15));
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
        TopScreenClearedThisFrame = false;
        NextMainDebugMarkerSpriteId = 0;
        NextSubDebugMarkerSpriteId = 0;
        NextMainAffineSpriteMatrixId = 0;
        NextSubAffineSpriteMatrixId = 0;
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
        TopScreenUnsupportedSpriteReasonThisFrame.clear();
        TopScreenUnsupportedSpriteRenderOrderThisFrame = 0;
        TopScreenUnsupportedSpriteWidthThisFrame = 0;
        TopScreenUnsupportedSpriteHeightThisFrame = 0;
        TopScreenUnsupportedSpriteTextureWidthThisFrame = 0;
        TopScreenUnsupportedSpriteTextureHeightThisFrame = 0;
        BottomScreenGlyphResolveFailureReason.clear();
        TopScreenGlyphResolveFailureReason.clear();
        ProfileTotalFrameMilliseconds = 0.0;
        ProfileCameraMilliseconds = 0.0;
        ProfileTextMilliseconds = 0.0;
        ProfileSpriteMilliseconds = 0.0;
        ProfileRoundedRectMilliseconds = 0.0;
        ProfileClearMilliseconds = 0.0;
        ProfileTextPrimitiveCount = 0;
        ProfileTextCacheHitCount = 0;
        ProfileTextRewriteCount = 0;
        ProfileSpritePrimitiveCount = 0;
        ProfileRoundedRectPrimitiveCount = 0;
        ProfileUnsupportedPrimitiveCount = 0;
        ProfileUnsupportedTextPrimitiveCount = 0;
        ProfileUnsupportedSpritePrimitiveCount = 0;
        ProfileUnsupportedRoundedRectPrimitiveCount = 0;
        DeferredHardwareTextSubmissionClears.clear();
        if (TextSubmissionFrameStamp == std::numeric_limits<uint32_t>::max()) {
            TextSubmissionFrameStamp = 1;
            for (auto& cachedSubmission : HardwareTextSubmissionStates) {
                cachedSubmission.second.LastVisitedFrameStamp = 0;
            }
        } else {
            TextSubmissionFrameStamp++;
        }
        PaintBeginFrameStageMarker(RGB15(0, 31, 0) | BIT(15));
        if (!TopScreenTextBackgroundInitialized) {
            EnsureTopScreenTextBackgroundReady();
        }
        PaintBeginFrameStageMarker(RGB15(0, 0, 31) | BIT(15));
        if (MainSpriteEngineInitialized || MainDebugMarkerInitialized) {
            oamClear(&oamMain, 0, 128);
        }
        // Keep the sub-screen text map persistent across frames so static BG0 text remains visible
        // even when the bottom 2D queue is skipped for a frame by the current camera traversal.
        if (SubSpriteEngineInitialized || SubDebugMarkerInitialized) {
        }

        AppendBottomScreenTextTraceLine(
            "[frame-begin] submitted=" + std::to_string(BottomScreenSubmittedTextCountThisFrame)
            + " unsupported=" + std::to_string(BottomScreenUnsupportedTextCountThisFrame));
        PaintBeginFrameStageMarker(RGB15(31, 0, 31) | BIT(15));
        BeginFrameStageMarkersCompleted = true;
    }

    /// Draws one camera's ordered 2D queue into the DS screen selected by the authored camera viewport.
    /// <param name="camera">Runtime camera owning the ordered 2D queue.</param>
    void NintendoDsRenderManager2D::DrawCamera(ICamera* camera) {
        uint32_t timingStartTicks = cpuGetTiming();
        auto profileCameraScope = he_cpp_make_scope_exit([&]() {
            double elapsedMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
            ProfileCameraMilliseconds += elapsedMilliseconds;
            ProfileTotalFrameMilliseconds = ProfileCameraMilliseconds;
        });

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

        if (targetBottomScreen) {
            if (BottomScreenPresentationEnabled && !BottomScreenClearedThisFrame) {
                ClearScreen(camera, true);
                BottomScreenClearedThisFrame = true;
            }
        } else if (!TopScreenClearedThisFrame) {
            ClearScreen(camera, false);
            TopScreenClearedThisFrame = true;
        }

        SelectViewportTarget(targetBottomScreen, viewportX, viewportY, viewportWidth, viewportHeight);
        IRenderQueue2D* renderQueue = camera->get_RenderQueue2D();
        if (renderQueue == nullptr) {
            return;
        }

        int32_t renderQueueCount = renderQueue->get_Count();
        if (!targetBottomScreen && !TopScreenQueueTraceRecorded) {
            AppendTopScreenRejectTraceLine("[helengine-ds] top-queue count=" + std::to_string(renderQueueCount));
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
        if (parent == nullptr) {
            return;
        }

        Entity* hierarchyEntity = parent;
        while (hierarchyEntity != nullptr) {
            if (!hierarchyEntity->get_Enabled()) {
                return;
            }

            hierarchyEntity = hierarchyEntity->get_Parent();
        }

        drawable->Draw();
    }

    /// Draws one rounded rectangle when the backend can map it to hardware, otherwise skips it.
    /// <param name="shape">Rounded-rectangle drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
        uint32_t timingStartTicks = cpuGetTiming();
        ProfileRoundedRectPrimitiveCount++;
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
        BuildHardwareSpriteTileSpans(clippedWidth, tileWidths, false);
        BuildHardwareSpriteTileSpans(clippedHeight, tileHeights, false);
        if (tileWidths.empty() || tileHeights.empty()) {
            return false;
        }

        bool targetBottomScreen = ActiveViewportTargetsBottomScreen;
        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        std::vector<void*>& frameLocalGraphics = targetBottomScreen ? FrameLocalSubRectangleGraphics : FrameLocalMainRectangleGraphics;
        std::array<uint16_t, 16>& rectanglePaletteColors = targetBottomScreen ? SubSolidRectanglePaletteColors : MainSolidRectanglePaletteColors;
        int32_t spritePriority = targetBottomScreen ? BottomScreenSpritePriority : TopScreenSpritePriority;
        EnsureSpriteEngineReady(targetBottomScreen ? NintendoDsScreenTarget::Bottom : NintendoDsScreenTarget::Top);

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
                if (!TryResolveSingleHardwareSpriteSize(int2(tileWidth, tileHeight), spriteSize)) {
                    return false;
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
                    spritePriority,
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

    /// Draws one sprite through the hardware OBJ path when possible, otherwise falling back to the active software framebuffer when available.
    /// <param name="sprite">Sprite drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
        uint32_t timingStartTicks = cpuGetTiming();
        ProfileSpritePrimitiveCount++;
        if (!ActiveViewportTargetsBottomScreen && TopScreenVisitTraceCount < 8) {
            std::string line = "[helengine-ds] top-visit sprite";
            if (sprite != nullptr) {
                line += " renderOrder=" + std::to_string(sprite->get_RenderOrder2D());
            }

            AppendTopScreenRejectTraceLine(line);
            TopScreenVisitTraceCount++;
        }
        if (ActiveViewportTargetsBottomScreen) {
            if (!TryDrawHardwareSprite(sprite)) {
                ProfileUnsupportedPrimitiveCount++;
                ProfileUnsupportedSpritePrimitiveCount++;
            }
        } else if (!TryDrawHardwareSprite(sprite)) {
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
        } else if (!TryDrawHardwareText(text)) {
            ProfileUnsupportedPrimitiveCount++;
            ProfileUnsupportedTextPrimitiveCount++;
        }

        ProfileTextMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
    }

    /// Copies the composed bottom-screen software bitmap framebuffer to visible Nintendo DS sub-screen VRAM.
    void NintendoDsRenderManager2D::PresentBottomScreenFrame() {
        ClearStaleHardwareTextSubmissions();
        if (!BottomScreenPresentationEnabled) {
            return;
        }

        AppendBottomScreenTextTraceLine(
            "[present] submitted=" + std::to_string(BottomScreenSubmittedTextCountThisFrame)
            + " unsupported=" + std::to_string(BottomScreenUnsupportedTextCountThisFrame)
            + " reason=" + BottomScreenUnsupportedTextReasonThisFrame);
        EnsureBottomScreenTextBackgroundReady();
        if (!BottomScreenClearedThisFrame && LastBottomScreenQueueCount == 0 && PreviousBottomScreenQueueCount != 0) {
            ClearBottomScreenTextMap();
            if (SubSpriteEngineInitialized || SubDebugMarkerInitialized) {
                oamClear(&oamSub, 0, 128);
            }
        }

        PresentPlatformOwnedPerformanceOverlayTextRows();

        if (MainSpriteEngineInitialized || MainDebugMarkerInitialized) {
            oamUpdate(&oamMain);
        }
        if (SubSpriteEngineInitialized || SubDebugMarkerInitialized) {
            oamUpdate(&oamSub);
        }

        PreviousBottomScreenQueueCount = LastBottomScreenQueueCount;
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
        return false;
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
        snapshot.CameraMilliseconds = ProfileCameraMilliseconds;
        snapshot.TextMilliseconds = ProfileTextMilliseconds;
        snapshot.SpriteMilliseconds = ProfileSpriteMilliseconds;
        snapshot.RoundedRectMilliseconds = ProfileRoundedRectMilliseconds;
        snapshot.ClearMilliseconds = ProfileClearMilliseconds;
        snapshot.TextPrimitiveCount = ProfileTextPrimitiveCount;
        snapshot.TextCacheHitCount = ProfileTextCacheHitCount;
        snapshot.TextRewriteCount = ProfileTextRewriteCount;
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
        uint32_t timingStartTicks = cpuGetTiming();
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        CameraClearSettings clearSettings = camera->get_ClearSettings();
        uint16_t clearColor = 0;
        if (clearSettings.get_ClearColorEnabled()) {
            clearColor = NintendoDsColorPacker::PackOpaqueColor(clearSettings.get_ClearColor());
        }

        NintendoDsScreenTarget targetScreen = targetBottomScreen
            ? NintendoDsScreenTarget::Bottom
            : NintendoDsScreenTarget::Top;

        EnsureScreenTextBackgroundReady(targetScreen);
        if (targetBottomScreen) {
            BG_PALETTE_SUB[0] = clearColor;
        } else {
            BG_PALETTE[0] = clearColor;
        }

        ProfileClearMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
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
        ActiveCpuFrameBuffer = nullptr;
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
            double advanceWidth = glyph.AdvanceWidth > 0.0f
                ? static_cast<double>(glyph.AdvanceWidth) * fontScale
                : static_cast<double>(glyphWidth);
            RasterTexturedQuad(texture, glyph.SourceRect, glyphX, glyphY, glyphWidth, glyphHeight, color);
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
        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        std::vector<void*>& spriteGraphics = targetBottomScreen ? runtimeTexture->SubHardwareSpriteGraphics : runtimeTexture->MainHardwareSpriteGraphics;
        int32_t paletteBank = targetBottomScreen ? runtimeTexture->SubHardwareSpritePaletteBank : runtimeTexture->MainHardwareSpritePaletteBank;
        bool spriteUses256Color = targetBottomScreen ? runtimeTexture->SubHardwareSpriteUses256Color : runtimeTexture->MainHardwareSpriteUses256Color;
        int32_t spritePriority = targetBottomScreen ? BottomScreenSpritePriority : TopScreenSpritePriority;
        SpriteColorFormat spriteColorFormat = spriteUses256Color ? SpriteColorFormat_256Color : SpriteColorFormat_16Color;
        if (spriteGraphics.empty() || paletteBank < 0) {
            TraceUnsupportedSpriteDrawable(sprite, "prepare");
            return false;
        }

        std::vector<int32_t> tileWidths;
        std::vector<int32_t> tileHeights;
        SpriteSize singleHardwareSpriteSize = SpriteSize_8x8;
        if (TryResolveSingleHardwareSpriteSize(hardwareSpriteSize, singleHardwareSpriteSize)) {
            tileWidths.push_back(hardwareSpriteSize.X);
            tileHeights.push_back(hardwareSpriteSize.Y);
        } else {
            BuildHardwareSpriteTileSpans(hardwareSpriteSize.X, tileWidths, true);
            BuildHardwareSpriteTileSpans(hardwareSpriteSize.Y, tileHeights, true);
        }
        if (tileWidths.empty() || tileHeights.empty()) {
            TraceUnsupportedSpriteDrawable(sprite, "prepare");
            return false;
        }

        int32_t tileCount = static_cast<int32_t>(tileWidths.size() * tileHeights.size());
        int32_t nextSpriteId = targetBottomScreen ? NextSubDebugMarkerSpriteId : NextMainDebugMarkerSpriteId;
        if (nextSpriteId + tileCount > 128) {
            TraceUnsupportedSpriteDrawable(sprite, "budget");
            return false;
        }

        int32_t affineAngle = 0;
        int32_t affineScaleX = 0;
        int32_t affineScaleY = 0;
        bool useDoubleSize = false;
        bool useAffineTransform = TryResolveAffineHardwareSpriteTransform(
            parent,
            drawableSize,
            hardwareSpriteSize,
            affineAngle,
            affineScaleX,
            affineScaleY,
            useDoubleSize);
        if (useAffineTransform
            && affineAngle == 0
            && affineScaleX == 256
            && affineScaleY == 256
            && !useDoubleSize) {
            useAffineTransform = false;
        }

        int32_t affineMatrixId = -1;
        if (useAffineTransform) {
            int32_t& nextAffineSpriteMatrixId = targetBottomScreen ? NextSubAffineSpriteMatrixId : NextMainAffineSpriteMatrixId;
            if (nextAffineSpriteMatrixId >= 32) {
                TraceUnsupportedSpriteDrawable(sprite, "affineBudget");
                return false;
            }

            affineMatrixId = nextAffineSpriteMatrixId;
            nextAffineSpriteMatrixId++;
            oamRotateScale(oamState, affineMatrixId, affineAngle, affineScaleX, affineScaleY);
        }

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

        double spriteScaleX = 1.0;
        double spriteScaleY = 1.0;
        double spriteCenterX = 0.0;
        double spriteCenterY = 0.0;
        int64_t spriteCenterXFixed = 0;
        int64_t spriteCenterYFixed = 0;
        int64_t visualCosineFixed = 0;
        int64_t visualSineFixed = 0;
        constexpr int64_t FixedPointScale = 256;
        if (useAffineTransform) {
            float3 entityScale = parent->get_Scale();
            spriteScaleX = ResolveQuantizedAffineVisualScale(affineScaleX);
            spriteScaleY = ResolveQuantizedAffineVisualScale(affineScaleY);
            spriteCenterX = static_cast<double>(parentPosition.X)
                + static_cast<double>(ActiveViewportOffsetX)
                + (static_cast<double>(drawableSize.X) * static_cast<double>(entityScale.X) * 0.5);
            spriteCenterY = static_cast<double>(parentPosition.Y)
                + static_cast<double>(ActiveViewportOffsetY)
                + (static_cast<double>(drawableSize.Y) * static_cast<double>(entityScale.Y) * 0.5);

            double visualRotationRadians = ResolveQuantizedAffineVisualRotationRadians(affineAngle);
            spriteCenterXFixed = static_cast<int64_t>(std::llround(spriteCenterX * static_cast<double>(FixedPointScale)));
            spriteCenterYFixed = static_cast<int64_t>(std::llround(spriteCenterY * static_cast<double>(FixedPointScale)));
            visualCosineFixed = static_cast<int64_t>(std::llround(std::cos(visualRotationRadians) * static_cast<double>(FixedPointScale)));
            visualSineFixed = static_cast<int64_t>(std::llround(std::sin(visualRotationRadians) * static_cast<double>(FixedPointScale)));
        }
        int32_t snappedSpriteCenterX = RoundFixedPointToNearestInteger(spriteCenterXFixed, FixedPointScale);
        int32_t snappedSpriteCenterY = RoundFixedPointToNearestInteger(spriteCenterYFixed, FixedPointScale);

        int32_t spriteGraphicsIndex = 0;
        int32_t tileY = clampedY;
        int32_t tileOriginY = 0;
        int32_t tileRowIndex = 0;
        for (int32_t tileHeight : tileHeights) {
            int32_t tileX = clampedX;
            int32_t tileOriginX = 0;
            int32_t tileColumnIndex = 0;
            for (int32_t tileWidth : tileWidths) {
                SpriteSize spriteSize = SpriteSize_8x8;
                if (!TryResolveSingleHardwareSpriteSize(int2(tileWidth, tileHeight), spriteSize)) {
                    TraceUnsupportedSpriteDrawable(sprite, "tileShape");
                    return false;
                }

                int32_t drawTileX = tileX;
                int32_t drawTileY = tileY;
                if (useAffineTransform) {
                    double localCenterX = (static_cast<double>(tileOriginX) + (static_cast<double>(tileWidth) * 0.5))
                        - (static_cast<double>(hardwareSpriteSize.X) * 0.5);
                    double localCenterY = (static_cast<double>(tileOriginY) + (static_cast<double>(tileHeight) * 0.5))
                        - (static_cast<double>(hardwareSpriteSize.Y) * 0.5);
                    int64_t scaledCenterXFixed = static_cast<int64_t>(std::llround(localCenterX * spriteScaleX * static_cast<double>(FixedPointScale)));
                    int64_t scaledCenterYFixed = static_cast<int64_t>(std::llround(localCenterY * spriteScaleY * static_cast<double>(FixedPointScale)));
                    int64_t rotatedCenterXFixed = ((scaledCenterXFixed * visualCosineFixed) - (scaledCenterYFixed * visualSineFixed)) / FixedPointScale;
                    int64_t rotatedCenterYFixed = ((scaledCenterXFixed * visualSineFixed) + (scaledCenterYFixed * visualCosineFixed)) / FixedPointScale;
                    int64_t affineAnchorOffsetXFixed = useDoubleSize
                        ? static_cast<int64_t>(std::llround(static_cast<double>(tileWidth) * spriteScaleX * static_cast<double>(FixedPointScale)))
                        : static_cast<int64_t>(std::llround(static_cast<double>(tileWidth) * spriteScaleX * static_cast<double>(FixedPointScale) * 0.5));
                    int64_t affineAnchorOffsetYFixed = useDoubleSize
                        ? static_cast<int64_t>(std::llround(static_cast<double>(tileHeight) * spriteScaleY * static_cast<double>(FixedPointScale)))
                        : static_cast<int64_t>(std::llround(static_cast<double>(tileHeight) * spriteScaleY * static_cast<double>(FixedPointScale) * 0.5));
                    int64_t drawTileXFixed = spriteCenterXFixed + rotatedCenterXFixed - affineAnchorOffsetXFixed;
                    int64_t drawTileYFixed = spriteCenterYFixed + rotatedCenterYFixed - affineAnchorOffsetYFixed;
                    int64_t relativeTileOffsetXFixed = drawTileXFixed - spriteCenterXFixed;
                    int64_t relativeTileOffsetYFixed = drawTileYFixed - spriteCenterYFixed;
                    drawTileX = snappedSpriteCenterX + RoundFixedPointTowardZeroInteger(relativeTileOffsetXFixed, FixedPointScale);
                    drawTileY = snappedSpriteCenterY + RoundFixedPointTowardZeroInteger(relativeTileOffsetYFixed, FixedPointScale);
                }

                int32_t affineInteriorOverlapX = useAffineTransform && tileWidths.size() > 1
                    ? tileColumnIndex
                    : 0;
                int32_t affineInteriorOverlapY = useAffineTransform && tileHeights.size() > 1
                    ? tileRowIndex
                    : 0;
                drawTileX -= affineInteriorOverlapX;
                drawTileY -= affineInteriorOverlapY;

                oamSet(
                    oamState,
                    nextSpriteId,
                    drawTileX,
                    drawTileY,
                    spritePriority,
                    paletteBank,
                    spriteSize,
                    spriteColorFormat,
                    spriteGraphics[static_cast<std::size_t>(spriteGraphicsIndex)],
                    useAffineTransform ? affineMatrixId : -1,
                    useAffineTransform ? useDoubleSize : false,
                    false,
                    false,
                    false,
                    false);
                nextSpriteId++;
                spriteGraphicsIndex++;
                tileX += tileWidth;
                tileOriginX += tileWidth;
                tileColumnIndex++;
            }

            tileY += tileHeight;
            tileOriginY += tileHeight;
            tileRowIndex++;
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
        EnsureSpriteEngineReady(targetBottomScreen ? NintendoDsScreenTarget::Bottom : NintendoDsScreenTarget::Top);
        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        std::vector<void*>& spriteGraphics = targetBottomScreen ? runtimeTexture->SubHardwareSpriteGraphics : runtimeTexture->MainHardwareSpriteGraphics;
        bool& spritePrepared = targetBottomScreen ? runtimeTexture->SubHardwareSpritePrepared : runtimeTexture->MainHardwareSpritePrepared;
        bool& spriteUses256Color = targetBottomScreen ? runtimeTexture->SubHardwareSpriteUses256Color : runtimeTexture->MainHardwareSpriteUses256Color;
        int32_t& spriteTileCount = targetBottomScreen ? runtimeTexture->SubHardwareSpriteTileCount : runtimeTexture->MainHardwareSpriteTileCount;

        std::vector<int32_t> tileWidths;
        std::vector<int32_t> tileHeights;
        SpriteSize singleHardwareSpriteSize = SpriteSize_8x8;
        if (TryResolveSingleHardwareSpriteSize(drawableSize, singleHardwareSpriteSize)) {
            tileWidths.push_back(drawableSize.X);
            tileHeights.push_back(drawableSize.Y);
        } else {
            BuildHardwareSpriteTileSpans(drawableSize.X, tileWidths, true);
            BuildHardwareSpriteTileSpans(drawableSize.Y, tileHeights, true);
        }
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
        spriteUses256Color = false;
        spriteTileCount = 0;

        std::vector<uint8_t> sourceIndices;
        std::array<uint16_t, 16> paletteColors {};
        int32_t paletteBank = -1;
        if (!TryResolveHardwareSpritePaletteBank(runtimeTexture, targetBottomScreen, paletteBank)) {
            return false;
        }

        SpriteColorFormat spriteColorFormat = SpriteColorFormat_16Color;
        if (TryBuildHardwareSpriteIndexed4(runtimeTexture, sourceIndices, paletteColors)) {
            UploadHardwareSpritePalette(targetBottomScreen, paletteBank, paletteColors);
        } else {
            if (targetBottomScreen) {
                return false;
            }

            std::array<uint16_t, 256> extendedPaletteColors {};
            if (!TryBuildHardwareSpriteIndexed8(runtimeTexture, sourceIndices, extendedPaletteColors)) {
                return false;
            }

            UploadHardwareSpriteExtendedPalette(paletteBank, extendedPaletteColors);
            spriteColorFormat = SpriteColorFormat_256Color;
            spriteUses256Color = true;
        }

        int32_t tileOriginY = 0;
        for (int32_t tileHeight : tileHeights) {
            int32_t tileOriginX = 0;
            for (int32_t tileWidth : tileWidths) {
                SpriteSize spriteSize = SpriteSize_8x8;
                if (!TryResolveSingleHardwareSpriteSize(int2(tileWidth, tileHeight), spriteSize)) {
                    for (void* allocatedGraphics : spriteGraphics) {
                        if (allocatedGraphics != nullptr) {
                            oamFreeGfx(oamState, allocatedGraphics);
                        }
                    }

                    spriteGraphics.clear();
                    spriteUses256Color = false;
                    return false;
                }

                void* tileGraphics = oamAllocateGfx(oamState, spriteSize, spriteColorFormat);
                if (tileGraphics == nullptr) {
                    for (void* allocatedGraphics : spriteGraphics) {
                        if (allocatedGraphics != nullptr) {
                            oamFreeGfx(oamState, allocatedGraphics);
                        }
                    }

                    spriteGraphics.clear();
                    spriteUses256Color = false;
                    return false;
                }

                std::vector<uint8_t> tileBytes = spriteColorFormat == SpriteColorFormat_256Color
                    ? BuildHardwareSpriteIndexed8TileBytes(
                        sourceIndices,
                        drawableSize.X,
                        drawableSize.Y,
                        tileOriginX,
                        tileOriginY,
                        tileWidth,
                        tileHeight)
                    : BuildHardwareSpriteIndexedTileBytes(
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
                    spriteUses256Color = false;
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

    /// Ensures the requested DS screen owns initialized OBJ hardware state before sprite submission begins.
    /// <param name="targetScreen">Physical DS screen whose OBJ hardware should be ready.</param>
    void NintendoDsRenderManager2D::EnsureSpriteEngineReady(NintendoDsScreenTarget targetScreen) {
        if (targetScreen == NintendoDsScreenTarget::Bottom) {
            if (SubSpriteEngineInitialized) {
                return;
            }

            vramSetBankI(VRAM_I_SUB_SPRITE);
            oamInit(&oamSub, SpriteMapping_1D_32, false);
            oamClear(&oamSub, 0, 128);
            SubSpriteEngineInitialized = true;
            return;
        }

        if (MainSpriteEngineInitialized) {
            return;
        }

        vramSetBankE(VRAM_E_MAIN_SPRITE);
        vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
        oamInit(&oamMain, SpriteMapping_1D_32, true);
        oamClear(&oamMain, 0, 128);
        MainSpriteEngineInitialized = true;
        MainSpriteEngineExtendedPalettesEnabled = true;
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

    /// Attempts to express one runtime texture as an 8bpp paletted DS sprite source.
    /// <param name="runtimeTexture">Runtime texture to evaluate.</param>
    /// <param name="sourceIndices">Receives one unpacked palette-index buffer in row-major order.</param>
    /// <param name="paletteColors">Receives one 256-entry DS sprite palette with entry zero reserved for transparency.</param>
    /// <returns>True when the runtime texture can be represented as one 8bpp DS sprite source.</returns>
    bool NintendoDsRenderManager2D::TryBuildHardwareSpriteIndexed8(NintendoDsRuntimeTexture2D* runtimeTexture, std::vector<uint8_t>& sourceIndices, std::array<uint16_t, 256>& paletteColors) const {
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
        uniqueOpaqueColors.reserve(255);
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
                if (uniqueOpaqueColors.size() >= 255) {
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

    /// Uploads one prepared 256-entry DS extended sprite palette into the main-engine palette memory.
    /// <param name="paletteBank">Extended palette bank to overwrite.</param>
    /// <param name="paletteColors">Prepared 256-entry palette to upload.</param>
    void NintendoDsRenderManager2D::UploadHardwareSpriteExtendedPalette(int32_t paletteBank, const std::array<uint16_t, 256>& paletteColors) const {
        if (paletteBank < 0 || paletteBank >= 16 || !MainSpriteEngineExtendedPalettesEnabled) {
            return;
        }

        vramSetBankF(VRAM_F_LCD);
        for (int32_t paletteIndex = 0; paletteIndex < 256; paletteIndex++) {
            VRAM_F_EXT_SPR_PALETTE[paletteBank][paletteIndex] = paletteColors[static_cast<std::size_t>(paletteIndex)];
        }

        vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
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
        BuildHardwareSpriteTileSpans(drawableSize.X, tileWidths, true);
        BuildHardwareSpriteTileSpans(drawableSize.Y, tileHeights, true);
        if (tileWidths.empty() || tileHeights.empty()) {
            return false;
        }

        constexpr int32_t MaximumHardwareSpriteTileCount = 32;
        return static_cast<int32_t>(tileWidths.size() * tileHeights.size()) <= MaximumHardwareSpriteTileCount;
    }

    /// Resolves one authored sprite size to a single DS OBJ shape that can rotate and scale as one affine sprite.
    /// <param name="drawableSize">Sprite size to resolve.</param>
    /// <param name="spriteSize">Receives the matching DS OBJ shape.</param>
    /// <returns>True when the sprite fits one hardware OBJ shape.</returns>
    bool NintendoDsRenderManager2D::TryResolveSingleHardwareSpriteSize(const int2& drawableSize, SpriteSize& spriteSize) const {
        if (drawableSize.X == 8 && drawableSize.Y == 8) {
            spriteSize = SpriteSize_8x8;
            return true;
        } else if (drawableSize.X == 8 && drawableSize.Y == 16) {
            spriteSize = SpriteSize_8x16;
            return true;
        } else if (drawableSize.X == 8 && drawableSize.Y == 32) {
            spriteSize = SpriteSize_8x32;
            return true;
        } else if (drawableSize.X == 16 && drawableSize.Y == 8) {
            spriteSize = SpriteSize_16x8;
            return true;
        } else if (drawableSize.X == 16 && drawableSize.Y == 16) {
            spriteSize = SpriteSize_16x16;
            return true;
        } else if (drawableSize.X == 16 && drawableSize.Y == 32) {
            spriteSize = SpriteSize_16x32;
            return true;
        } else if (drawableSize.X == 32 && drawableSize.Y == 8) {
            spriteSize = SpriteSize_32x8;
            return true;
        } else if (drawableSize.X == 32 && drawableSize.Y == 16) {
            spriteSize = SpriteSize_32x16;
            return true;
        } else if (drawableSize.X == 32 && drawableSize.Y == 32) {
            spriteSize = SpriteSize_32x32;
            return true;
        } else if (drawableSize.X == 32 && drawableSize.Y == 64) {
            spriteSize = SpriteSize_32x64;
            return true;
        } else if (drawableSize.X == 64 && drawableSize.Y == 32) {
            spriteSize = SpriteSize_64x32;
            return true;
        } else if (drawableSize.X == 64 && drawableSize.Y == 64) {
            spriteSize = SpriteSize_64x64;
            return true;
        }

        return false;
    }

    /// Resolves one entity-driven 2D sprite transform into DS affine angle and scale values.
    /// <param name="parent">Entity that owns the sprite transform.</param>
    /// <param name="drawableSize">Authored destination size before entity scale.</param>
    /// <param name="hardwareSpriteSize">Prepared DS OBJ source size.</param>
    /// <param name="affineAngle">Receives the DS affine angle unit.</param>
    /// <param name="affineScaleX">Receives the DS fixed-point horizontal scale.</param>
    /// <param name="affineScaleY">Receives the DS fixed-point vertical scale.</param>
    /// <param name="useDoubleSize">Receives whether the DS doubled affine box should be enabled.</param>
    /// <returns>True when the transform can be expressed through one DS affine OBJ matrix.</returns>
    bool NintendoDsRenderManager2D::TryResolveAffineHardwareSpriteTransform(
        Entity* parent,
        const int2& drawableSize,
        const int2& hardwareSpriteSize,
        int32_t& affineAngle,
        int32_t& affineScaleX,
        int32_t& affineScaleY,
        bool& useDoubleSize) const {
        affineAngle = 0;
        affineScaleX = 0;
        affineScaleY = 0;
        useDoubleSize = false;
        if (parent == nullptr
            || drawableSize.X <= 0
            || drawableSize.Y <= 0
            || hardwareSpriteSize.X <= 0
            || hardwareSpriteSize.Y <= 0) {
            return false;
        }

        float4 orientation = parent->get_Orientation();
        if (std::abs(orientation.X) > 0.001f || std::abs(orientation.Y) > 0.001f) {
            return false;
        }

        float3 entityScale = parent->get_Scale();
        if (entityScale.X <= 0.0f || entityScale.Y <= 0.0f || std::abs(entityScale.Z - 1.0f) > 0.001f) {
            return false;
        }

        double drawWidth = static_cast<double>(drawableSize.X) * static_cast<double>(entityScale.X);
        double drawHeight = static_cast<double>(drawableSize.Y) * static_cast<double>(entityScale.Y);
        double scaleX = drawWidth / static_cast<double>(hardwareSpriteSize.X);
        double scaleY = drawHeight / static_cast<double>(hardwareSpriteSize.Y);
        if (scaleX <= 0.0 || scaleY <= 0.0) {
            return false;
        }

        double zRotationRadians = ResolveSpriteZRotationRadians(orientation);
        constexpr double fullTurnRadians = 6.28318530717958647692;
        double normalizedRotation = std::fmod(-zRotationRadians, fullTurnRadians);
        if (normalizedRotation < 0.0) {
            normalizedRotation += fullTurnRadians;
        }

        constexpr double libndsFullTurnAngleUnits = 32768.0;
        affineAngle = static_cast<int32_t>(std::round(normalizedRotation * (libndsFullTurnAngleUnits / fullTurnRadians)));
        affineScaleX = static_cast<int32_t>(std::floor(256.0 / scaleX));
        affineScaleY = static_cast<int32_t>(std::floor(256.0 / scaleY));
        if (affineScaleX <= 0 || affineScaleY <= 0) {
            return false;
        }

        useDoubleSize = std::abs(zRotationRadians) > 0.001
            || std::abs(scaleX - 1.0) > 0.001
            || std::abs(scaleY - 1.0) > 0.001;
        return true;
    }

    /// Converts one fixed-point coordinate to an integer pixel by rounding to the nearest pixel so shared affine sprite anchors stay visually locked across multi-tile submissions.
    /// <param name="fixedValue">Fixed-point coordinate numerator.</param>
    /// <param name="fixedScale">Fixed-point denominator.</param>
    /// <returns>Integer pixel coordinate rounded to the nearest pixel.</returns>
    int32_t NintendoDsRenderManager2D::RoundFixedPointToNearestInteger(int64_t fixedValue, int64_t fixedScale) const {
        if (fixedScale <= 0) {
            throw std::invalid_argument("Nintendo DS fixed-point scale must be greater than zero.");
        }

        if (fixedValue >= 0) {
            return static_cast<int32_t>((fixedValue + (fixedScale / 2)) / fixedScale);
        }

        return static_cast<int32_t>((fixedValue - (fixedScale / 2)) / fixedScale);
    }

    /// Converts one fixed-point relative offset to an integer pixel by rounding toward zero so affine interior tile seams bias inward toward the composed sprite center.
    /// <param name="fixedValue">Fixed-point relative offset numerator.</param>
    /// <param name="fixedScale">Fixed-point denominator.</param>
    /// <returns>Integer pixel offset rounded toward zero.</returns>
    int32_t NintendoDsRenderManager2D::RoundFixedPointTowardZeroInteger(int64_t fixedValue, int64_t fixedScale) const {
        if (fixedScale <= 0) {
            throw std::invalid_argument("Nintendo DS fixed-point scale must be greater than zero.");
        }

        return static_cast<int32_t>(fixedValue / fixedScale);
    }

    /// Reconstructs the effective visual rotation produced by one quantized DS affine angle value.
    /// <param name="affineAngle">Quantized DS affine angle units submitted to libnds.</param>
    /// <returns>Signed visual rotation in radians that matches the DS hardware matrix.</returns>
    double NintendoDsRenderManager2D::ResolveQuantizedAffineVisualRotationRadians(int32_t affineAngle) const {
        constexpr double fullTurnRadians = 6.28318530717958647692;
        constexpr double libndsFullTurnAngleUnits = 32768.0;
        double quantizedHardwareRotationRadians = static_cast<double>(affineAngle) * (fullTurnRadians / libndsFullTurnAngleUnits);
        return -quantizedHardwareRotationRadians;
    }

    /// Reconstructs the effective visual scale produced by one quantized DS affine inverse-scale value.
    /// <param name="affineScale">Quantized DS affine inverse-scale value submitted to libnds.</param>
    /// <returns>Effective visual scale represented by the quantized affine value.</returns>
    double NintendoDsRenderManager2D::ResolveQuantizedAffineVisualScale(int32_t affineScale) const {
        if (affineScale <= 0) {
            throw std::invalid_argument("Nintendo DS affine scale must be greater than zero.");
        }

        return 256.0 / static_cast<double>(affineScale);
    }

    /// Resolves the signed Z-axis rotation encoded in one engine quaternion.
    /// <param name="orientation">Quaternion to inspect.</param>
    /// <returns>Signed Z rotation in radians.</returns>
    double NintendoDsRenderManager2D::ResolveSpriteZRotationRadians(const float4& orientation) const {
        double z = static_cast<double>(orientation.Z);
        double w = static_cast<double>(orientation.W);
        return std::atan2(2.0 * z * w, 1.0 - (2.0 * z * z));
    }

    /// Expands one authored sprite dimension into a minimal set of DS OBJ tile spans.
    /// <param name="length">Authored sprite width or height in pixels.</param>
    /// <param name="spans">Receives the DS OBJ tile spans that cover the authored dimension.</param>
    void NintendoDsRenderManager2D::BuildHardwareSpriteTileSpans(int32_t length, std::vector<int32_t>& spans, bool prefer64PixelSpans) const {
        spans.clear();
        if (length <= 0) {
            return;
        }

        int32_t remainingLength = length;
        while (remainingLength > 0) {
            int32_t tileSpan = 8;
            if (prefer64PixelSpans && remainingLength >= 64) {
                tileSpan = 64;
            } else if (remainingLength >= 32) {
                tileSpan = 32;
            } else if (remainingLength <= 8) {
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

    /// Builds one temporary DS 8bpp tile payload copied from one authored sprite texture region.
    /// <param name="sourceIndices">Decoded palette indices in row-major order.</param>
    /// <param name="sourceWidth">Authored source texture width in pixels.</param>
    /// <param name="sourceHeight">Authored source texture height in pixels.</param>
    /// <param name="tileOriginX">Source pixel X offset for the tile copy.</param>
    /// <param name="tileOriginY">Source pixel Y offset for the tile copy.</param>
    /// <param name="tileWidth">Prepared DS OBJ tile width in pixels.</param>
    /// <param name="tileHeight">Prepared DS OBJ tile height in pixels.</param>
    /// <returns>Padded DS OBJ tile bytes in 8bpp tiled order.</returns>
    std::vector<uint8_t> NintendoDsRenderManager2D::BuildHardwareSpriteIndexed8TileBytes(const std::vector<uint8_t>& sourceIndices, int32_t sourceWidth, int32_t sourceHeight, int32_t tileOriginX, int32_t tileOriginY, int32_t tileWidth, int32_t tileHeight) const {
        if (sourceWidth <= 0 || sourceHeight <= 0 || tileWidth <= 0 || tileHeight <= 0) {
            return {};
        }

        std::vector<uint8_t> tileBytes(static_cast<std::size_t>(tileWidth * tileHeight), 0);
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

                    for (int32_t localX = 0; localX < 8; localX++) {
                        int32_t sourceX = tileOriginX + (blockColumn * 8) + localX;
                        uint8_t paletteIndex = sourceX >= 0 && sourceX < sourceWidth
                            ? sourceIndices[static_cast<std::size_t>(sourceY * sourceWidth + sourceX)]
                            : static_cast<uint8_t>(0);
                        int32_t destinationIndex = (blockIndex * 64) + (localY * 8) + localX;
                        tileBytes[static_cast<std::size_t>(destinationIndex)] = paletteIndex;
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
            videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_EXT_PALETTE);
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
        InvalidateHardwareTextSubmissionCache(targetScreen);
    }

    /// Clears the top-screen DS text background map through the renderer-owned shadow state.
    void NintendoDsRenderManager2D::ClearTopScreenTextMap() {
        ClearScreenTextMap(NintendoDsScreenTarget::Top);
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

        if (targetBottomScreen) {
            AppendBottomScreenTextTraceLine(
                "[font-upload] lineHeight=" + std::to_string(font->get_LineHeight())
                + " atlas=" + std::to_string(font->get_AtlasWidth()) + "x" + std::to_string(font->get_AtlasHeight()));
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
        constexpr int32_t ConsoleRows = VisibleScreenHeight / 8;
        int32_t safeRow = std::clamp(row, static_cast<int32_t>(0), ConsoleRows - 1);
        int32_t safeColumn = std::clamp(column, static_cast<int32_t>(0), ConsoleColumns - 1);
        int32_t writableColumns = std::clamp(visibleColumnCount, static_cast<int32_t>(0), ConsoleColumns - safeColumn);
        int32_t rowOffset = safeRow * ConsoleColumns;
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

    /// Clears one rectangular run of bottom-screen BG text rows used by the platform-owned FPS overlay.
    /// <param name="row">Zero-based first BG text row to clear.</param>
    /// <param name="column">Zero-based first BG text column to clear.</param>
    /// <param name="rowSpan">Number of BG text rows to clear.</param>
    /// <param name="visibleColumnCount">Number of writable columns to clear per row.</param>
    void NintendoDsRenderManager2D::ClearBottomScreenTextRowSpan(int32_t row, int32_t column, int32_t rowSpan, int32_t visibleColumnCount) {
        constexpr int32_t ConsoleRows = VisibleScreenHeight / 8;
        int32_t safeRow = std::clamp(row, static_cast<int32_t>(0), ConsoleRows - 1);
        int32_t safeRowSpan = std::clamp(rowSpan, static_cast<int32_t>(0), ConsoleRows - safeRow);
        for (int32_t rowOffset = 0; rowOffset < safeRowSpan; rowOffset++) {
            WriteBottomScreenTextLine(safeRow + rowOffset, column, std::string(), visibleColumnCount);
        }
    }

    /// Presents the resolved platform-owned FPS overlay rows directly on the bottom-screen BG text layer.
    void NintendoDsRenderManager2D::PresentPlatformOwnedPerformanceOverlayTextRows() {
        constexpr int32_t ConsoleColumns = FrameBufferWidth / 8;
        constexpr int32_t ConsoleRows = VisibleScreenHeight / 8;
        Core* core = Core::get_Instance();
        if (core == nullptr || !core->get_UsesPlatformOwnedPerformanceOverlayPresentation()) {
            if (PreviousPlatformOwnedOverlayRowSpan > 0) {
                ClearBottomScreenTextRowSpan(
                    PreviousPlatformOwnedOverlayRow,
                    PreviousPlatformOwnedOverlayColumn,
                    PreviousPlatformOwnedOverlayRowSpan,
                    PreviousPlatformOwnedOverlayVisibleColumnCount);
                PreviousPlatformOwnedOverlayRowSpan = 0;
                PreviousPlatformOwnedOverlayLines.clear();
            }

            return;
        }

        FontAsset* font = core->get_ResolvedPerformanceOverlayFont();
        if (font == nullptr) {
            if (PreviousPlatformOwnedOverlayRowSpan > 0) {
                ClearBottomScreenTextRowSpan(
                    PreviousPlatformOwnedOverlayRow,
                    PreviousPlatformOwnedOverlayColumn,
                    PreviousPlatformOwnedOverlayRowSpan,
                    PreviousPlatformOwnedOverlayVisibleColumnCount);
                PreviousPlatformOwnedOverlayRowSpan = 0;
                PreviousPlatformOwnedOverlayLines.clear();
            }

            return;
        }

        EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget::Bottom, font);
        std::vector<std::string> visibleLines;
        visibleLines.reserve(8);
        AppendVisibleOverlayLines(core->get_ResolvedPerformanceOverlayUpdateText(), visibleLines);
        AppendVisibleOverlayLines(core->get_ResolvedPerformanceOverlayRenderText(), visibleLines);
        AppendVisibleOverlayLines(core->get_ResolvedPerformanceOverlayDetailText(), visibleLines);
        AppendVisibleOverlayLines(core->get_ResolvedPerformanceOverlayAdditionalText(), visibleLines);

        int2 padding = core->get_ResolvedPerformanceOverlayPadding();
        int32_t startColumn = std::clamp(static_cast<int32_t>(std::lround(static_cast<double>(padding.X) / 8.0)), static_cast<int32_t>(0), ConsoleColumns - 1);
        int32_t startRow = std::clamp(static_cast<int32_t>(std::lround(static_cast<double>(padding.Y) / 8.0)), static_cast<int32_t>(0), ConsoleRows - 1);
        int32_t visibleColumnCount = ConsoleColumns - startColumn;
        int32_t rowStep = std::max(
            static_cast<int32_t>(1),
            static_cast<int32_t>(std::lround((static_cast<double>(font->get_LineHeight()) * static_cast<double>(core->get_ResolvedPerformanceOverlayFontScale())) / 8.0)));
        int32_t maxVisibleLineCount = std::max(static_cast<int32_t>(0), ((ConsoleRows - startRow - 1) / rowStep) + 1);
        if (static_cast<int32_t>(visibleLines.size()) > maxVisibleLineCount) {
            visibleLines.resize(static_cast<std::size_t>(maxVisibleLineCount));
        }

        if (visibleLines.empty()) {
            if (PreviousPlatformOwnedOverlayRowSpan > 0) {
                ClearBottomScreenTextRowSpan(
                    PreviousPlatformOwnedOverlayRow,
                    PreviousPlatformOwnedOverlayColumn,
                    PreviousPlatformOwnedOverlayRowSpan,
                    PreviousPlatformOwnedOverlayVisibleColumnCount);
                PreviousPlatformOwnedOverlayRowSpan = 0;
                PreviousPlatformOwnedOverlayLines.clear();
            }

            return;
        }

        bool layoutChanged = PreviousPlatformOwnedOverlayRowSpan > 0
            && (PreviousPlatformOwnedOverlayRow != startRow
                || PreviousPlatformOwnedOverlayColumn != startColumn
                || PreviousPlatformOwnedOverlayRowStep != rowStep
                || PreviousPlatformOwnedOverlayVisibleColumnCount != visibleColumnCount);

        if (layoutChanged) {
            ClearBottomScreenTextRowSpan(
                PreviousPlatformOwnedOverlayRow,
                PreviousPlatformOwnedOverlayColumn,
                PreviousPlatformOwnedOverlayRowSpan,
                PreviousPlatformOwnedOverlayVisibleColumnCount);
            PreviousPlatformOwnedOverlayLines.clear();
            PreviousPlatformOwnedOverlayRowSpan = 0;
        }

        int32_t previousLineCount = static_cast<int32_t>(PreviousPlatformOwnedOverlayLines.size());
        int32_t currentLineCount = static_cast<int32_t>(visibleLines.size());
        int32_t maxLineCount = std::max(previousLineCount, currentLineCount);
        for (int32_t lineIndex = 0; lineIndex < maxLineCount; lineIndex++) {
            int32_t targetRow = startRow + (lineIndex * rowStep);
            if (lineIndex >= currentLineCount) {
                WriteBottomScreenTextLine(targetRow, startColumn, std::string(), visibleColumnCount);
                continue;
            }

            if (layoutChanged
                || lineIndex >= previousLineCount
                || PreviousPlatformOwnedOverlayLines[static_cast<std::size_t>(lineIndex)] != visibleLines[static_cast<std::size_t>(lineIndex)]) {
                WriteBottomScreenTextLine(
                    targetRow,
                    startColumn,
                    visibleLines[static_cast<std::size_t>(lineIndex)],
                    visibleColumnCount);
            }
        }

        PreviousPlatformOwnedOverlayRow = startRow;
        PreviousPlatformOwnedOverlayColumn = startColumn;
        PreviousPlatformOwnedOverlayRowStep = rowStep;
        PreviousPlatformOwnedOverlayVisibleColumnCount = visibleColumnCount;
        PreviousPlatformOwnedOverlayRowSpan = std::min(ConsoleRows - startRow, ((static_cast<int32_t>(visibleLines.size()) - 1) * rowStep) + 1);
        PreviousPlatformOwnedOverlayLines = visibleLines;
    }

    /// Appends visible non-empty overlay lines from one possibly multi-line text block.
    /// <param name="textBlock">Source overlay text block that may contain newline separators.</param>
    /// <param name="destination">Ordered visible overlay lines receiving the parsed content.</param>
    void NintendoDsRenderManager2D::AppendVisibleOverlayLines(const std::string& textBlock, std::vector<std::string>& destination) const {
        if (textBlock.empty()) {
            return;
        }

        std::string currentLine;
        for (char character : textBlock) {
            if (character == '\r') {
                continue;
            }

            if (character == '\n') {
                if (!currentLine.empty()) {
                    destination.push_back(currentLine);
                }

                currentLine.clear();
                continue;
            }

            currentLine.push_back(character);
        }

        if (!currentLine.empty()) {
            destination.push_back(currentLine);
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
            return false;
        }

        Entity* parent = text->get_Parent();
        if (parent == nullptr) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "parent");
            return false;
        }

        float3 parentPosition = parent->get_Position();
        int32_t screenX = static_cast<int32_t>(std::round(parentPosition.X)) + ActiveViewportOffsetX;
        int32_t screenY = static_cast<int32_t>(std::round(parentPosition.Y)) + ActiveViewportOffsetY;
        int32_t baseColumn = screenX / 8;
        int32_t baseRow = screenY / 8;
        int32_t textRenderStateVersion = ResolveTextRenderStateVersion(text);
        int32_t backgroundLayer = ResolveTextBackgroundLayer(text);
        if (backgroundLayer != 0) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "bgLayer");
            return false;
        }

        if (ActiveViewportTargetsBottomScreen && !BottomScreenPresentationEnabled) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "screen");
            return false;
        }

        FontAsset* font = text->get_Font();
        if (font == nullptr) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "font");
            return false;
        }

        float fontScale = text->get_FontScale();
        if (fontScale <= 0.001f) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "fontScale");
            return false;
        }

        if (text->get_WrapText()) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "wrap");
            return false;
        }

        int32_t alignment = static_cast<int32_t>(text->get_Alignment());
        if (alignment < 0 || alignment > 2) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "alignment");
            return false;
        }

        byte4 color = text->get_Color();
        if (color.W == 0) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "color");
            return false;
        }

        float4 sourceRect = text->get_SourceRect();
        if (std::abs(sourceRect.X) > 0.001f
            || std::abs(sourceRect.Y) > 0.001f
            || std::abs(sourceRect.Z - 1.0f) > 0.001f
            || std::abs(sourceRect.W - 1.0f) > 0.001f) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "sourceRect");
            return false;
        }

        const std::string& content = text->get_Text();
        if (content.empty()) {
            ClearHardwareTextSubmission(text);
            return true;
        }

        for (char character : content) {
            if (character != ' ' && (static_cast<unsigned char>(character) < 32 || static_cast<unsigned char>(character) > 126)) {
                ClearHardwareTextSubmission(text);
                TraceUnsupportedTextDrawable(text, "charset");
                return false;
            }
        }

        std::size_t lineBreakIndex = content.find('\n');
        std::string visibleLine = lineBreakIndex == std::string::npos
            ? content
            : content.substr(0, lineBreakIndex);
        if (visibleLine.empty()) {
            ClearHardwareTextSubmission(text);
            if (ActiveViewportTargetsBottomScreen) {
                BottomScreenSubmittedTextCountThisFrame++;
            }

            return true;
        }

        constexpr int32_t ConsoleColumns = FrameBufferWidth / 8;
        constexpr int32_t ConsoleRows = VisibleScreenHeight / 8;
        int2 textSize = text->get_Size();
        int32_t visibleLength = static_cast<int32_t>(visibleLine.size());
        int32_t fullBoxColumnCount = std::max<int32_t>(1, static_cast<int32_t>(std::ceil(static_cast<double>(textSize.X) / 8.0)));
        if (textSize.X <= 0 || textSize.Y <= 0) {
            ClearHardwareTextSubmission(text);
            if (ActiveViewportTargetsBottomScreen) {
                BottomScreenSubmittedTextCountThisFrame++;
            }

            return true;
        }

        int32_t targetRow = baseRow;
        if (targetRow < 0 || targetRow >= ConsoleRows) {
            ClearHardwareTextSubmission(text);
            if (ActiveViewportTargetsBottomScreen) {
                BottomScreenSubmittedTextCountThisFrame++;
            }

            return true;
        }

        int32_t unclampedStartColumn = ResolveAlignedConsoleColumnUnclamped(baseColumn, fullBoxColumnCount, visibleLength, alignment);
        std::string visibleGlyphLine = visibleLine;
        int32_t startColumn = unclampedStartColumn;
        int32_t writableColumnCount = visibleLength;

        if (ActiveViewportTargetsBottomScreen) {
            AppendBottomScreenTextTraceLine(
                "[draw-bottom] renderOrder=" + std::to_string(text->get_RenderOrder2D())
                + " lineHeight=" + std::to_string(font->get_LineHeight())
                + " atlas=" + std::to_string(font->get_AtlasWidth()) + "x" + std::to_string(font->get_AtlasHeight())
                + " text=" + visibleLine);
            EnsureBottomScreenTextBackgroundReady();
            if (BottomScreenTextMapEntries == nullptr) {
                ClearHardwareTextSubmission(text);
                TraceUnsupportedTextDrawable(text, "glyphMap");
                return false;
            }

            EnsureBottomScreenFontGlyphTilesReady(font);
            if (!BottomScreenGlyphResolveFailureReason.empty()) {
                ClearHardwareTextSubmission(text);
                TraceUnsupportedTextDrawable(text, BottomScreenGlyphResolveFailureReason.c_str());
                return false;
            }

            if (!TryReuseHardwareTextSubmission(
                text,
                NintendoDsScreenTarget::Bottom,
                targetRow,
                startColumn,
                writableColumnCount,
                textRenderStateVersion)) {
                ProfileTextRewriteCount++;
                PrepareHardwareTextSubmissionForRewrite(
                    text,
                    NintendoDsScreenTarget::Bottom,
                    targetRow,
                    startColumn,
                    writableColumnCount);
                WriteBottomScreenTextLine(targetRow, startColumn, visibleGlyphLine, writableColumnCount);
                InvalidateCurrentFrameOverlappingHardwareTextSubmissions(
                    text,
                    NintendoDsScreenTarget::Bottom,
                    targetRow,
                    startColumn,
                    writableColumnCount);
                RememberHardwareTextSubmission(
                    text,
                    NintendoDsScreenTarget::Bottom,
                    targetRow,
                    baseRow,
                    startColumn,
                    baseColumn,
                    writableColumnCount,
                    textRenderStateVersion);
            }

            BottomScreenSubmittedTextCountThisFrame++;
            return true;
        }

        EnsureTopScreenTextBackgroundReady();
        if (TopScreenTextMapEntries == nullptr) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, "glyphMap");
            return false;
        }

        EnsureTopScreenFontGlyphTilesReady(font);
        if (!TopScreenGlyphResolveFailureReason.empty()) {
            ClearHardwareTextSubmission(text);
            TraceUnsupportedTextDrawable(text, TopScreenGlyphResolveFailureReason.c_str());
            return false;
        }

        if (!TryReuseHardwareTextSubmission(
            text,
            NintendoDsScreenTarget::Top,
            targetRow,
            startColumn,
            writableColumnCount,
            textRenderStateVersion)) {
            ProfileTextRewriteCount++;
            PrepareHardwareTextSubmissionForRewrite(
                text,
                NintendoDsScreenTarget::Top,
                targetRow,
                startColumn,
                writableColumnCount);
            WriteTopScreenTextLine(targetRow, startColumn, visibleGlyphLine, writableColumnCount);
            InvalidateCurrentFrameOverlappingHardwareTextSubmissions(
                text,
                NintendoDsScreenTarget::Top,
                targetRow,
                startColumn,
                writableColumnCount);
            RememberHardwareTextSubmission(
                text,
                NintendoDsScreenTarget::Top,
                targetRow,
                baseRow,
                startColumn,
                baseColumn,
                writableColumnCount,
                textRenderStateVersion);
        }
        return true;
    }

    /// Resolves the shared engine-side text render-state version used to detect visible text changes.
    /// <param name="text">Text drawable whose shared render-state version should be read.</param>
    /// <returns>Shared render-state version or zero when the drawable is missing.</returns>
    int32_t NintendoDsRenderManager2D::ResolveTextRenderStateVersion(ITextDrawable2D* text) const {
        if (text == nullptr) {
            return 0;
        }

        return text->get_TextRenderStateVersion();
    }

    /// Reuses one cached DS text submission when the shared engine state and BG text placement still match.
    /// <param name="text">Text drawable that may already own one cached DS text run.</param>
    /// <param name="targetScreen">Physical DS screen targeted by the current draw attempt.</param>
    /// <param name="row">Resolved BG text row for the current draw attempt.</param>
    /// <param name="column">Resolved BG text start column for the current draw attempt.</param>
    /// <param name="writableColumnCount">Resolved writable BG text width for the current draw attempt.</param>
    /// <param name="textRenderStateVersion">Shared engine-side text render-state version observed for the draw attempt.</param>
    /// <returns>True when the cached DS tile-map submission is still valid and no rewrite is required.</returns>
    bool NintendoDsRenderManager2D::TryReuseHardwareTextSubmission(
        ITextDrawable2D* text,
        NintendoDsScreenTarget targetScreen,
        int32_t row,
        int32_t column,
        int32_t writableColumnCount,
        int32_t textRenderStateVersion) {
        auto cachedSubmission = HardwareTextSubmissionStates.find(text);
        if (cachedSubmission == HardwareTextSubmissionStates.end()) {
            return false;
        }

        NintendoDsHardwareTextSubmissionState& submissionState = cachedSubmission->second;
        if (submissionState.TargetScreen != targetScreen
            || submissionState.Row != row
            || submissionState.Column != column
            || submissionState.WritableColumnCount != writableColumnCount
            || submissionState.TextRenderStateVersion != textRenderStateVersion) {
            return false;
        }

        submissionState.LastVisitedFrameStamp = TextSubmissionFrameStamp;
        ProfileTextCacheHitCount++;
        return true;
    }

    /// Reuses one cached DS text submission when the shared text state and parent-derived grid position are unchanged.
    /// <param name="text">Text drawable that may already own one cached DS text run.</param>
    /// <param name="targetScreen">Physical DS screen targeted by the current draw attempt.</param>
    /// <param name="baseRow">Unaligned BG text row resolved directly from the drawable parent position.</param>
    /// <param name="baseColumn">Unaligned BG text column resolved directly from the drawable parent position.</param>
    /// <param name="textRenderStateVersion">Shared engine-side text render-state version observed for the draw attempt.</param>
    /// <returns>True when the cached DS tile-map submission is still valid and no string or layout recomputation is required.</returns>
    bool NintendoDsRenderManager2D::TryReuseHardwareTextSubmissionAtCachedPlacement(
        ITextDrawable2D* text,
        NintendoDsScreenTarget targetScreen,
        int32_t baseRow,
        int32_t baseColumn,
        int32_t textRenderStateVersion) {
        auto cachedSubmission = HardwareTextSubmissionStates.find(text);
        if (cachedSubmission == HardwareTextSubmissionStates.end()) {
            return false;
        }

        NintendoDsHardwareTextSubmissionState& submissionState = cachedSubmission->second;
        if (submissionState.TargetScreen != targetScreen
            || submissionState.BaseRow != baseRow
            || submissionState.BaseColumn != baseColumn
            || submissionState.TextRenderStateVersion != textRenderStateVersion) {
            return false;
        }

        submissionState.LastVisitedFrameStamp = TextSubmissionFrameStamp;
        ProfileTextCacheHitCount++;
        return true;
    }

    /// Clears any previously cached DS text span that would overlap or outlive the upcoming rewrite.
    /// <param name="text">Text drawable whose cached submission is about to be rewritten.</param>
    /// <param name="targetScreen">Physical DS screen targeted by the new draw attempt.</param>
    /// <param name="row">Resolved BG text row for the new draw attempt.</param>
    /// <param name="column">Resolved BG text start column for the new draw attempt.</param>
    /// <param name="writableColumnCount">Resolved writable BG text width for the new draw attempt.</param>
    void NintendoDsRenderManager2D::PrepareHardwareTextSubmissionForRewrite(
        ITextDrawable2D* text,
        NintendoDsScreenTarget targetScreen,
        int32_t row,
        int32_t column,
        int32_t writableColumnCount) {
        auto cachedSubmission = HardwareTextSubmissionStates.find(text);
        if (cachedSubmission == HardwareTextSubmissionStates.end()) {
            return;
        }

        const NintendoDsHardwareTextSubmissionState& previousState = cachedSubmission->second;
        if (previousState.TargetScreen != targetScreen
            || previousState.Row != row
            || previousState.Column != column) {
            QueueHardwareTextSubmissionForDeferredClear(previousState);
            return;
        }

        if (previousState.WritableColumnCount <= writableColumnCount) {
            return;
        }

        int32_t trailingColumn = column + writableColumnCount;
        int32_t trailingColumnCount = previousState.WritableColumnCount - writableColumnCount;
        WriteScreenTextLine(
            targetScreen,
            row,
            trailingColumn,
            std::string(),
            trailingColumnCount);
    }

    /// Stores the latest DS text-map submission for one drawable after the visible row has been rewritten.
    /// <param name="text">Text drawable whose latest hardware submission should be cached.</param>
    /// <param name="targetScreen">Physical DS screen that now owns the drawable text run.</param>
    /// <param name="row">Resolved BG text row for the latest draw attempt.</param>
    /// <param name="baseRow">Unaligned BG text row resolved directly from the drawable parent position.</param>
    /// <param name="column">Resolved BG text start column for the latest draw attempt.</param>
    /// <param name="baseColumn">Unaligned BG text column resolved directly from the drawable parent position.</param>
    /// <param name="writableColumnCount">Resolved writable BG text width for the latest draw attempt.</param>
    /// <param name="textRenderStateVersion">Shared engine-side text render-state version observed for the latest draw attempt.</param>
    void NintendoDsRenderManager2D::RememberHardwareTextSubmission(
        ITextDrawable2D* text,
        NintendoDsScreenTarget targetScreen,
        int32_t row,
        int32_t baseRow,
        int32_t column,
        int32_t baseColumn,
        int32_t writableColumnCount,
        int32_t textRenderStateVersion) {
        NintendoDsHardwareTextSubmissionState submissionState {};
        submissionState.TargetScreen = targetScreen;
        submissionState.Row = row;
        submissionState.BaseRow = baseRow;
        submissionState.Column = column;
        submissionState.BaseColumn = baseColumn;
        submissionState.WritableColumnCount = writableColumnCount;
        submissionState.TextRenderStateVersion = textRenderStateVersion;
        submissionState.LastVisitedFrameStamp = TextSubmissionFrameStamp;
        HardwareTextSubmissionStates[text] = submissionState;
    }

    /// Drops current-frame cache entries whose row coverage was already overwritten by a later text submission in the same frame.
    /// <param name="text">Text drawable that now owns the final visible row segment.</param>
    /// <param name="targetScreen">Physical DS screen targeted by the new draw attempt.</param>
    /// <param name="row">Resolved BG text row for the new draw attempt.</param>
    /// <param name="column">Resolved BG text start column for the new draw attempt.</param>
    /// <param name="writableColumnCount">Resolved writable BG text width for the new draw attempt.</param>
    void NintendoDsRenderManager2D::InvalidateCurrentFrameOverlappingHardwareTextSubmissions(
        ITextDrawable2D* text,
        NintendoDsScreenTarget targetScreen,
        int32_t row,
        int32_t column,
        int32_t writableColumnCount) {
        int32_t submissionEndColumn = column + writableColumnCount;
        for (auto cachedSubmission = HardwareTextSubmissionStates.begin(); cachedSubmission != HardwareTextSubmissionStates.end();) {
            if (cachedSubmission->first == text) {
                ++cachedSubmission;
                continue;
            }

            const NintendoDsHardwareTextSubmissionState& submissionState = cachedSubmission->second;
            if (submissionState.LastVisitedFrameStamp != TextSubmissionFrameStamp
                || submissionState.TargetScreen != targetScreen
                || submissionState.Row != row) {
                ++cachedSubmission;
                continue;
            }

            int32_t cachedEndColumn = submissionState.Column + submissionState.WritableColumnCount;
            bool overlapsColumns = submissionState.Column < submissionEndColumn && cachedEndColumn > column;
            if (!overlapsColumns) {
                ++cachedSubmission;
                continue;
            }

            cachedSubmission = HardwareTextSubmissionStates.erase(cachedSubmission);
        }
    }

    /// Clears one previously cached DS text-map region back to blanks.
    /// <param name="submissionState">Cached hardware text region that should be cleared.</param>
    void NintendoDsRenderManager2D::ClearHardwareTextSubmission(const NintendoDsHardwareTextSubmissionState& submissionState) {
        WriteScreenTextLine(
            submissionState.TargetScreen,
            submissionState.Row,
            submissionState.Column,
            std::string(),
            submissionState.WritableColumnCount);
    }

    /// Clears and forgets one cached DS text submission owned by the supplied drawable when one exists.
    /// <param name="text">Text drawable whose cached submission should be removed.</param>
    void NintendoDsRenderManager2D::ClearHardwareTextSubmission(ITextDrawable2D* text) {
        auto cachedSubmission = HardwareTextSubmissionStates.find(text);
        if (cachedSubmission == HardwareTextSubmissionStates.end()) {
            return;
        }

        QueueHardwareTextSubmissionForDeferredClear(cachedSubmission->second);
        HardwareTextSubmissionStates.erase(cachedSubmission);
    }

    /// Clears cached text runs that were not visited during the current frame traversal.
    void NintendoDsRenderManager2D::ClearStaleHardwareTextSubmissions() {
        ClearDeferredHardwareTextSubmissions();
        for (auto cachedSubmission = HardwareTextSubmissionStates.begin(); cachedSubmission != HardwareTextSubmissionStates.end();) {
            if (cachedSubmission->second.LastVisitedFrameStamp == TextSubmissionFrameStamp) {
                ++cachedSubmission;
                continue;
            }

            ClearHardwareTextSubmissionColumnsOutsideCurrentFrameCoverage(cachedSubmission->second);
            cachedSubmission = HardwareTextSubmissionStates.erase(cachedSubmission);
        }
    }

    /// Queues one superseded DS text span for end-of-frame cleanup after current-frame row coverage is known.
    /// <param name="submissionState">Cached text span that should be cleared after current-frame draws finish.</param>
    void NintendoDsRenderManager2D::QueueHardwareTextSubmissionForDeferredClear(const NintendoDsHardwareTextSubmissionState& submissionState) {
        DeferredHardwareTextSubmissionClears.push_back(submissionState);
    }

    /// Clears superseded DS text spans after current-frame text submissions establish the final visible row coverage.
    void NintendoDsRenderManager2D::ClearDeferredHardwareTextSubmissions() {
        for (const NintendoDsHardwareTextSubmissionState& submissionState : DeferredHardwareTextSubmissionClears) {
            ClearHardwareTextSubmissionColumnsOutsideCurrentFrameCoverage(submissionState);
        }

        DeferredHardwareTextSubmissionClears.clear();
    }

    /// Clears the stale columns of one cached text run that are not rewritten by current-frame submissions on the same DS text row.
    /// <param name="submissionState">Cached text region being considered for stale cleanup.</param>
    void NintendoDsRenderManager2D::ClearHardwareTextSubmissionColumnsOutsideCurrentFrameCoverage(const NintendoDsHardwareTextSubmissionState& submissionState) {
        int32_t submissionStartColumn = submissionState.Column;
        int32_t submissionEndColumn = submissionState.Column + submissionState.WritableColumnCount;
        int32_t clearCursor = submissionStartColumn;
        while (clearCursor < submissionEndColumn) {
            bool foundCoveredColumns = false;
            int32_t nextCoveredColumn = submissionEndColumn;
            int32_t nextCoveredEndColumn = submissionEndColumn;
            for (const auto& cachedSubmission : HardwareTextSubmissionStates) {
                const NintendoDsHardwareTextSubmissionState& currentSubmission = cachedSubmission.second;
                if (currentSubmission.LastVisitedFrameStamp != TextSubmissionFrameStamp) {
                    continue;
                }

                if (currentSubmission.TargetScreen != submissionState.TargetScreen || currentSubmission.Row != submissionState.Row) {
                    continue;
                }

                int32_t currentStartColumn = std::max(submissionStartColumn, currentSubmission.Column);
                int32_t currentEndColumn = std::min(submissionEndColumn, currentSubmission.Column + currentSubmission.WritableColumnCount);
                if (currentStartColumn >= currentEndColumn || currentEndColumn <= clearCursor) {
                    continue;
                }

                if (!foundCoveredColumns
                    || currentStartColumn < nextCoveredColumn
                    || (currentStartColumn == nextCoveredColumn && currentEndColumn > nextCoveredEndColumn)) {
                    foundCoveredColumns = true;
                    nextCoveredColumn = currentStartColumn;
                    nextCoveredEndColumn = currentEndColumn;
                }
            }

            if (!foundCoveredColumns) {
                WriteScreenTextLine(
                    submissionState.TargetScreen,
                    submissionState.Row,
                    clearCursor,
                    std::string(),
                    submissionEndColumn - clearCursor);
                return;
            }

            if (nextCoveredColumn > clearCursor) {
                WriteScreenTextLine(
                    submissionState.TargetScreen,
                    submissionState.Row,
                    clearCursor,
                    std::string(),
                    nextCoveredColumn - clearCursor);
            }

            clearCursor = std::max(clearCursor, nextCoveredEndColumn);
        }
    }

    /// Invalidates cached DS text submissions that target one specific physical screen.
    /// <param name="targetScreen">Physical DS screen whose cached text runs should be discarded.</param>
    void NintendoDsRenderManager2D::InvalidateHardwareTextSubmissionCache(NintendoDsScreenTarget targetScreen) {
        for (auto cachedSubmission = HardwareTextSubmissionStates.begin(); cachedSubmission != HardwareTextSubmissionStates.end();) {
            if (cachedSubmission->second.TargetScreen != targetScreen) {
                ++cachedSubmission;
                continue;
            }

            cachedSubmission = HardwareTextSubmissionStates.erase(cachedSubmission);
        }
    }

    /// Resolves the authored synthetic DS text background-layer override carried by the submitted text component.
    /// <param name="text">Text drawable whose synthetic platform member should be queried.</param>
    /// <returns>Requested DS text background-layer index or zero when no override was authored.</returns>
    int32_t NintendoDsRenderManager2D::ResolveTextBackgroundLayer(ITextDrawable2D* text) const {
        if (text == nullptr) {
            return 0;
        }

        Component* component = dynamic_cast<Component*>(text);
        if (component == nullptr) {
            return 0;
        }

        return component->GetSyntheticInt32MemberOrDefault(std::string("BGLayer"), 0);
    }

    /// Resolves the console start column for one aligned text run inside its authored text box before screen-edge clamping.
    /// <param name="baseColumn">Left-edge console column derived from the drawable position.</param>
    /// <param name="boxColumnCount">Width of the authored text box expressed in console columns.</param>
    /// <param name="visibleLength">Visible text length expressed in console columns.</param>
    /// <param name="alignment">Generated-core text alignment value.</param>
    /// <returns>Console start column before screen-edge clamping.</returns>
    int32_t NintendoDsRenderManager2D::ResolveAlignedConsoleColumnUnclamped(int32_t baseColumn, int32_t boxColumnCount, int32_t visibleLength, int32_t alignment) const {
        int32_t safeBoxColumnCount = std::max(boxColumnCount, visibleLength);
        int32_t offsetColumns = 0;
        if (alignment == 1) {
            offsetColumns = std::max((safeBoxColumnCount - visibleLength) / 2, static_cast<int32_t>(0));
        } else if (alignment == 2) {
            offsetColumns = std::max(safeBoxColumnCount - visibleLength, static_cast<int32_t>(0));
        }

        return baseColumn + offsetColumns;
    }

    /// Resolves the console start column for one aligned text run inside its authored text box.
    /// <param name="baseColumn">Left-edge console column derived from the drawable position.</param>
    /// <param name="boxColumnCount">Width of the authored text box expressed in console columns.</param>
    /// <param name="visibleLength">Visible text length expressed in console columns.</param>
    /// <param name="alignment">Generated-core text alignment value.</param>
    /// <returns>Console start column clamped to the visible DS text grid.</returns>
    int32_t NintendoDsRenderManager2D::ResolveAlignedConsoleColumn(int32_t baseColumn, int32_t boxColumnCount, int32_t visibleLength, int32_t alignment) const {
        int32_t unclampedColumn = ResolveAlignedConsoleColumnUnclamped(baseColumn, boxColumnCount, visibleLength, alignment);
        int32_t maximumColumn = (FrameBufferWidth / 8) - 1;
        return std::clamp(unclampedColumn, static_cast<int32_t>(0), maximumColumn);
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
            if (TopScreenUnsupportedSpriteReasonThisFrame.empty()) {
                TopScreenUnsupportedSpriteReasonThisFrame = reason == nullptr ? "unknown" : reason;
                if (sprite != nullptr) {
                    int2 spriteSize = sprite->get_Size();
                    TopScreenUnsupportedSpriteRenderOrderThisFrame = sprite->get_RenderOrder2D();
                    TopScreenUnsupportedSpriteWidthThisFrame = spriteSize.X;
                    TopScreenUnsupportedSpriteHeightThisFrame = spriteSize.Y;

                    RuntimeTexture* runtimeTextureBase = sprite->get_Texture();
                    NintendoDsRuntimeTexture2D* runtimeTexture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(runtimeTextureBase);
                    if (runtimeTexture != nullptr) {
                        TopScreenUnsupportedSpriteTextureWidthThisFrame = runtimeTexture->get_Width();
                        TopScreenUnsupportedSpriteTextureHeightThisFrame = runtimeTexture->get_Height();
                    }
                }
            }

            std::string line = "[helengine-ds] top-sprite-reject reason=";
            line += reason == nullptr ? "unknown" : reason;
            if (sprite != nullptr) {
                int2 spriteSize = sprite->get_Size();
                line += " renderOrder=" + std::to_string(sprite->get_RenderOrder2D());
                line += " size=" + std::to_string(spriteSize.X) + "x" + std::to_string(spriteSize.Y);

                RuntimeTexture* runtimeTextureBase = sprite->get_Texture();
                NintendoDsRuntimeTexture2D* runtimeTexture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(runtimeTextureBase);
                if (runtimeTexture != nullptr) {
                    line += " texture=" + std::to_string(runtimeTexture->get_Width()) + "x" + std::to_string(runtimeTexture->get_Height());
                }
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
        int32_t spritePriority = targetScreen == NintendoDsScreenTarget::Bottom ? BottomScreenSpritePriority : TopScreenSpritePriority;
        int32_t clampedX = std::clamp(x, static_cast<int32_t>(0), static_cast<int32_t>(FrameBufferWidth - 8));
        int32_t clampedY = std::clamp(y, static_cast<int32_t>(0), static_cast<int32_t>(VisibleScreenHeight - 8));
        oamSet(
            oamState,
            spriteId,
            clampedX,
            clampedY,
            spritePriority,
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
                EnsureSpriteEngineReady(NintendoDsScreenTarget::Bottom);

                SubDebugMarkerGfx = oamAllocateGfx(&oamSub, SpriteSize_8x8, SpriteColorFormat_16Color);
                std::memset(SubDebugMarkerGfx, 0x11, UnsupportedDebugMarkerTileBytes);
                SPRITE_PALETTE_SUB[0] = 0;
                SPRITE_PALETTE_SUB[1] = UnsupportedDebugMarkerColor;
                SubDebugMarkerInitialized = true;
            }

            return;
        }

        if (!MainDebugMarkerInitialized) {
            EnsureSpriteEngineReady(NintendoDsScreenTarget::Top);

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
