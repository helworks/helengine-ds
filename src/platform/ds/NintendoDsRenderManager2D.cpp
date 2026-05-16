#include "platform/ds/NintendoDsRenderManager2D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <algorithm>
#include <cmath>

extern "C" {
#include <nds/arm9/background.h>
#include <nds/dma.h>
#include <nds/timers.h>
}

#include "CameraClearSettings.hpp"
#include "Entity.hpp"
#include "FontAsset.hpp"
#include "FontChar.hpp"
#include "FontInfo.hpp"
#include "ICamera.hpp"
#include "IRenderQueue2D.hpp"
#include "TextLayoutUtils.hpp"
#include "TextureAsset.hpp"
#include "byte4.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "platform/ds/NintendoDsColorPacker.hpp"
#include "platform/ds/NintendoDsRuntimeTexture2D.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::ds {
    namespace {
        /// Stores the number of bytes per RGBA32 texel in authored runtime texture payloads.
        constexpr int32_t Rgba32BytesPerPixel = 4;

        /// Stores the number of bytes per RGBA4444 texel in authored runtime texture payloads.
        constexpr int32_t Rgba4444BytesPerPixel = 2;

        /// Stores the number of bytes used by one indexed-8 texel.
        constexpr int32_t Indexed8BytesPerPixel = 1;

        /// Stores the number of RGBA bytes carried by one palette entry.
        constexpr int32_t PaletteEntryBytes = 4;

        /// Expands one packed Nintendo DS 5-bit color channel into the 8-bit range used by software blending.
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

        /// Packs one byte4 color into one 32-bit RGBA cache pixel.
        uint32_t PackByte4ToUint32(const byte4& color) {
            return static_cast<uint32_t>(color.X)
                | (static_cast<uint32_t>(color.Y) << 8)
                | (static_cast<uint32_t>(color.Z) << 16)
                | (static_cast<uint32_t>(color.W) << 24);
        }

        /// Unpacks one cached 32-bit RGBA pixel into the shared byte4 representation.
        byte4 UnpackUint32ToByte4(uint32_t packedColor) {
            return byte4(
                static_cast<uint8_t>(packedColor & 255),
                static_cast<uint8_t>((packedColor >> 8) & 255),
                static_cast<uint8_t>((packedColor >> 16) & 255),
                static_cast<uint8_t>((packedColor >> 24) & 255));
        }

        /// Decodes one packed RGBA4444 texel into the shared 8-bit RGBA color representation used by the DS software compositor.
        byte4 UnpackRgba4444(uint16_t packedColor) {
            return byte4(
                ExpandFourBitChannel(packedColor, 0),
                ExpandFourBitChannel(packedColor, 4),
                ExpandFourBitChannel(packedColor, 8),
                ExpandFourBitChannel(packedColor, 12));
        }

        /// Resolves the expected cooked payload length for one texture asset based on its serialized texture format.
        int32_t GetExpectedColorLength(TextureAssetColorFormat colorFormat, int32_t width, int32_t height) {
            if (colorFormat == TextureAssetColorFormat::Rgba32) {
                return width * height * Rgba32BytesPerPixel;
            } else if (colorFormat == TextureAssetColorFormat::Rgba4444) {
                return width * height * Rgba4444BytesPerPixel;
            } else if (colorFormat == TextureAssetColorFormat::Indexed4) {
                return ((width * height) + 1) / 2;
            } else if (colorFormat == TextureAssetColorFormat::Indexed8) {
                return width * height * Indexed8BytesPerPixel;
            }

            throw new InvalidOperationException("Nintendo DS 2D renderer received an unsupported texture color format.");
        }

        /// Resolves the maximum supported cooked palette payload length for one indexed texture format.
        int32_t GetMaximumPaletteColorLength(TextureAssetColorFormat colorFormat) {
            if (colorFormat == TextureAssetColorFormat::Indexed4) {
                return 16 * PaletteEntryBytes;
            } else if (colorFormat == TextureAssetColorFormat::Indexed8) {
                return 256 * PaletteEntryBytes;
            }

            return 0;
        }

        /// Reads one packed 4-bit palette index from the DS indexed-4 texel payload.
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

        /// Packs one byte4 color into one 32-bit cache-key field.
        uint32_t PackOpaqueRoundedRectCacheColor(const byte4& color) {
            return static_cast<uint32_t>(color.X)
                | (static_cast<uint32_t>(color.Y) << 8)
                | (static_cast<uint32_t>(color.Z) << 16)
                | (static_cast<uint32_t>(color.W) << 24);
        }

        /// Builds one deterministic cache key for opaque rounded-rectangle rasters that can be reused across DS menu frames.
        NintendoDsOpaqueRoundedRectCacheKey BuildOpaqueRoundedRectCacheKey(
            int32_t width,
            int32_t height,
            int32_t radius,
            int32_t borderThickness,
            RoundedRectCorners corners,
            const byte4& fillColor,
            const byte4& borderColor) {
            return NintendoDsOpaqueRoundedRectCacheKey {
                width,
                height,
                radius,
                borderThickness,
                static_cast<int32_t>(corners),
                PackOpaqueRoundedRectCacheColor(fillColor),
                PackOpaqueRoundedRectCacheColor(borderColor)
            };
        }
    }

    /// Compares two rounded-rectangle cache keys for equality.
    /// <param name="other">Key to compare.</param>
    /// <returns>True when all key fields match.</returns>
    bool NintendoDsOpaqueRoundedRectCacheKey::operator==(const NintendoDsOpaqueRoundedRectCacheKey& other) const {
        return Width == other.Width
            && Height == other.Height
            && Radius == other.Radius
            && BorderThickness == other.BorderThickness
            && CornersMask == other.CornersMask
            && FillColor == other.FillColor
            && BorderColor == other.BorderColor;
    }

    /// Computes a stable hash for one opaque rounded-rectangle cache key.
    /// <param name="key">Key to hash.</param>
    /// <returns>Unordered-map hash value.</returns>
    std::size_t NintendoDsOpaqueRoundedRectCacheKeyHasher::operator()(const NintendoDsOpaqueRoundedRectCacheKey& key) const {
        std::size_t hash = static_cast<std::size_t>(key.Width);
        hash = (hash * 1315423911u) ^ static_cast<std::size_t>(key.Height);
        hash = (hash * 1315423911u) ^ static_cast<std::size_t>(key.Radius);
        hash = (hash * 1315423911u) ^ static_cast<std::size_t>(key.BorderThickness);
        hash = (hash * 1315423911u) ^ static_cast<std::size_t>(key.CornersMask);
        hash = (hash * 1315423911u) ^ static_cast<std::size_t>(key.FillColor);
        hash = (hash * 1315423911u) ^ static_cast<std::size_t>(key.BorderColor);
        return hash;
    }

    /// Creates the Nintendo DS 2D renderer with no active texture-build diagnostics yet.
    NintendoDsRenderManager2D::NintendoDsRenderManager2D()
        : LastTextureBuildStage("NotStarted")
        , LastTextureAssetId()
        , LastTextureWidth(0)
        , LastTextureHeight(0)
        , LastTextureColorLength(0)
        , ActiveCpuFrameBuffer(nullptr)
        , ActiveViewportOffsetX(0)
        , ActiveViewportOffsetY(0)
        , ActiveClipLeft(0)
        , ActiveClipTop(0)
        , ActiveClipRight(FrameBufferWidth)
        , ActiveClipBottom(VisibleScreenHeight)
        , TopScreenClearedThisFrame(false)
        , BottomScreenClearedThisFrame(false)
        , Hardware3DScreenTarget(NintendoDsScreenTarget::None)
        , ActiveViewportTargetsBottomScreen(false)
        , BottomScreenPresentationEnabled(true)
        , ProfileTotalFrameMilliseconds(0.0)
        , ProfileTextMilliseconds(0.0)
        , ProfileSpriteMilliseconds(0.0)
        , ProfileRoundedRectMilliseconds(0.0)
        , ProfileTextPrimitiveCount(0)
        , ProfileSpritePrimitiveCount(0)
        , ProfileRoundedRectPrimitiveCount(0) {
    }

    /// Builds one DS software runtime texture from the authored texture asset.
    /// <param name="data">Authored texture asset.</param>
    /// <returns>DS runtime texture carrying the adopted cooked pixel payload.</returns>
    RuntimeTexture* NintendoDsRenderManager2D::BuildTextureFromRaw(TextureAsset* data) {
        LastTextureBuildStage = "BuildTextureFromRawBegin";
        LastTextureAssetId = data != nullptr ? data->get_Id() : std::string();
        LastTextureWidth = data != nullptr ? data->Width : 0;
        LastTextureHeight = data != nullptr ? data->Height : 0;
        LastTextureColorLength = data != nullptr && data->Colors != nullptr ? data->Colors->Length : 0;
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        } else if (data->Colors == nullptr || data->Colors->Data == nullptr) {
            throw new InvalidOperationException("TextureAsset colors were required for Nintendo DS 2D rendering.");
        }

        int32_t expectedLength = GetExpectedColorLength(data->ColorFormat, static_cast<int32_t>(data->Width), static_cast<int32_t>(data->Height));
        if (data->Colors->Length != expectedLength) {
            throw new InvalidOperationException("TextureAsset color payload length did not match the authored dimensions.");
        }

        if (data->ColorFormat == TextureAssetColorFormat::Indexed4 || data->ColorFormat == TextureAssetColorFormat::Indexed8) {
            if (data->PaletteColors == nullptr || data->PaletteColors->Data == nullptr) {
                throw new InvalidOperationException("Indexed TextureAsset payloads required palette colors for Nintendo DS 2D rendering.");
            } else if ((data->PaletteColors->Length % PaletteEntryBytes) != 0) {
                throw new InvalidOperationException("TextureAsset palette payload length must be aligned to RGBA palette entries.");
            }

            int32_t maximumPaletteColorLength = GetMaximumPaletteColorLength(data->ColorFormat);
            if (data->PaletteColors->Length < PaletteEntryBytes || data->PaletteColors->Length > maximumPaletteColorLength) {
                throw new InvalidOperationException("TextureAsset palette payload length exceeded the supported Nintendo DS indexed-texture limits.");
            }
        }

        NintendoDsRuntimeTexture2D* texture = new NintendoDsRuntimeTexture2D();
        texture->set_Id(data->get_Id());
        texture->set_Width(data->Width);
        texture->set_Height(data->Height);
        texture->ColorFormat = data->ColorFormat;
        texture->AlphaPrecision = data->AlphaPrecision;
        texture->Colors = data->Colors;
        texture->PaletteColors = data->PaletteColors != nullptr ? data->PaletteColors : Array<uint8_t>::Empty();
        data->Colors = Array<uint8_t>::Empty();
        data->PaletteColors = Array<uint8_t>::Empty();
        LastTextureBuildStage = "BuildTextureFromRawComplete";
        return texture;
    }

    /// Releases one DS runtime texture and its adopted pixel payload.
    /// <param name="texture">Runtime texture to release.</param>
    void NintendoDsRenderManager2D::ReleaseTexture(RuntimeTexture* texture) {
        if (texture == nullptr) {
            return;
        }

        NintendoDsRuntimeTexture2D* runtimeTexture = dynamic_cast<NintendoDsRuntimeTexture2D*>(texture);
        if (runtimeTexture != nullptr && runtimeTexture->Colors != nullptr && runtimeTexture->Colors != Array<uint8_t>::Empty()) {
            delete runtimeTexture->Colors;
            runtimeTexture->Colors = Array<uint8_t>::Empty();
        }

        if (runtimeTexture != nullptr && runtimeTexture->PaletteColors != nullptr && runtimeTexture->PaletteColors != Array<uint8_t>::Empty()) {
            delete runtimeTexture->PaletteColors;
            runtimeTexture->PaletteColors = Array<uint8_t>::Empty();
        }

        texture->Dispose();
        delete texture;
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

    /// Resets per-frame screen-clear state before the active camera list is traversed.
    void NintendoDsRenderManager2D::BeginFrame() {
        ActiveCpuFrameBuffer = nullptr;
        ActiveViewportOffsetX = 0;
        ActiveViewportOffsetY = 0;
        ActiveClipLeft = 0;
        ActiveClipTop = 0;
        ActiveClipRight = FrameBufferWidth;
        ActiveClipBottom = VisibleScreenHeight;
        TopScreenClearedThisFrame = false;
        BottomScreenClearedThisFrame = false;
        Hardware3DScreenTarget = NintendoDsScreenTarget::None;
        ActiveViewportTargetsBottomScreen = false;
        ProfileTotalFrameMilliseconds = 0.0;
        ProfileTextMilliseconds = 0.0;
        ProfileSpriteMilliseconds = 0.0;
        ProfileRoundedRectMilliseconds = 0.0;
        ProfileTextPrimitiveCount = 0;
        ProfileSpritePrimitiveCount = 0;
        ProfileRoundedRectPrimitiveCount = 0;
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

        if (!targetBottomScreen && !TopScreenClearedThisFrame) {
            ClearScreen(camera, false);
            TopScreenClearedThisFrame = true;
        } else if (targetBottomScreen && !BottomScreenClearedThisFrame) {
            ClearScreen(camera, true);
            BottomScreenClearedThisFrame = true;
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

    /// Draws one rounded rectangle into the DS top-screen bitmap framebuffer.
    /// <param name="shape">Rounded-rectangle drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
        cpuStartTiming(0);
        RasterRoundedRect(shape);
        ProfileRoundedRectMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuEndTiming());
        ProfileRoundedRectPrimitiveCount++;
    }

    /// Draws one sprite into the DS top-screen bitmap framebuffer.
    /// <param name="sprite">Sprite drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
        cpuStartTiming(0);
        RasterSprite(sprite);
        ProfileSpriteMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuEndTiming());
        ProfileSpritePrimitiveCount++;
    }

    /// Draws one text string into the DS top-screen bitmap framebuffer.
    /// <param name="text">Text drawable requested by generated core.</param>
    void NintendoDsRenderManager2D::DrawText(ITextDrawable2D* text) {
        cpuStartTiming(0);
        RasterText(text);
        ProfileTextMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuEndTiming());
        ProfileTextPrimitiveCount++;
    }

    /// Clears one DS screen framebuffer from one runtime camera clear configuration.
    /// <param name="camera">Runtime camera providing the clear settings.</param>
    /// <param name="targetBottomScreen">True when the bottom screen should be cleared; otherwise the top screen.</param>
    void NintendoDsRenderManager2D::ClearScreen(ICamera* camera, bool targetBottomScreen) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        uint16_t clearColor = PackOpaqueByteColor(0, 0, 0);
        CameraClearSettings clearSettings = camera->get_ClearSettings();
        if (clearSettings.get_ClearColorEnabled()) {
            clearColor = NintendoDsColorPacker::PackOpaqueColor(clearSettings.get_ClearColor());
        }

        uint16_t* frameBuffer = targetBottomScreen ? BottomCpuFrameBuffer.data() : TopCpuFrameBuffer.data();
        std::fill_n(frameBuffer, VisibleFrameBufferPixelCount, clearColor);
    }

    /// Copies the composed CPU-side backbuffers to the visible DS top and bottom bitmap framebuffers.
    void NintendoDsRenderManager2D::PresentFrame() {
        if (Hardware3DScreenTarget == NintendoDsScreenTarget::Top) {
            dmaCopyHalfWords(3, TopCpuFrameBuffer.data(), BG_BMP_RAM(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
        } else if (Hardware3DScreenTarget == NintendoDsScreenTarget::Bottom) {
            dmaCopyHalfWords(3, BottomCpuFrameBuffer.data(), BG_BMP_RAM(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
        } else {
            dmaCopyHalfWords(3, TopCpuFrameBuffer.data(), BG_BMP_RAM(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
        }

        if (BottomScreenPresentationEnabled) {
            if (Hardware3DScreenTarget == NintendoDsScreenTarget::Bottom) {
                dmaCopyHalfWords(3, TopCpuFrameBuffer.data(), BG_BMP_RAM_SUB(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
            } else {
                dmaCopyHalfWords(3, BottomCpuFrameBuffer.data(), BG_BMP_RAM_SUB(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));
            }
        }
    }

    /// Assigns which physical Nintendo DS screen owns the hardware 3D pass for the active frame.
    /// <param name="target">Hardware 3D target screen for the active frame.</param>
    void NintendoDsRenderManager2D::SetHardware3DScreenTarget(NintendoDsScreenTarget target) {
        Hardware3DScreenTarget = target;
    }

    /// Enables or disables presentation of the composed bottom-screen bitmap framebuffer.
    /// <param name="enabled">True to present the bottom-screen bitmap framebuffer; otherwise false.</param>
    void NintendoDsRenderManager2D::SetBottomScreenPresentationEnabled(bool enabled) {
        BottomScreenPresentationEnabled = enabled;
    }

    /// Gets the latest frame-local 2D renderer profiling snapshot for the native DS diagnostics console.
    /// <returns>Current 2D renderer profiling snapshot.</returns>
    NintendoDsRenderManager2DProfileSnapshot NintendoDsRenderManager2D::get_ProfileSnapshot() const {
        NintendoDsRenderManager2DProfileSnapshot snapshot {
            ProfileTotalFrameMilliseconds,
            ProfileTextMilliseconds,
            ProfileSpriteMilliseconds,
            ProfileRoundedRectMilliseconds,
            ProfileTextPrimitiveCount,
            ProfileSpritePrimitiveCount,
            ProfileRoundedRectPrimitiveCount
        };
        snapshot.TotalFrameMilliseconds = snapshot.TextMilliseconds + snapshot.SpriteMilliseconds + snapshot.RoundedRectMilliseconds;
        return snapshot;
    }

    /// Resolves one authored camera viewport into Nintendo DS pixel-space bounds.
    /// <param name="camera">Runtime camera providing the authored viewport.</param>
    /// <returns>Viewport rectangle expressed in Nintendo DS pixel-space coordinates.</returns>
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

    /// Selects the active backbuffer and clip rectangle used by subsequent raster operations.
    /// <param name="targetBottomScreen">True when the bottom-screen backbuffer should become active.</param>
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
        ActiveCpuFrameBuffer = targetBottomScreen ? BottomCpuFrameBuffer.data() : TopCpuFrameBuffer.data();
        ActiveViewportOffsetX = viewportX;
        ActiveViewportOffsetY = viewportY;
        ActiveClipLeft = std::max(static_cast<int32_t>(0), viewportX);
        ActiveClipTop = std::max(static_cast<int32_t>(0), viewportY);
        ActiveClipRight = std::min(FrameBufferWidth, viewportX + viewportWidth);
        ActiveClipBottom = std::min(VisibleScreenHeight, viewportY + viewportHeight);
    }

    /// Blends one source pixel into the DS top-screen framebuffer.
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

        uint16_t* frameBuffer = ActiveCpuFrameBuffer;
        int32_t pixelIndex = (y * FrameBufferWidth) + x;
        if (color.W >= 255) {
            frameBuffer[pixelIndex] = PackOpaqueByteColor(color.X, color.Y, color.Z);
            return;
        }

        uint16_t destinationColor = frameBuffer[pixelIndex];
        uint8_t destRed = ExpandFiveBitChannel(destinationColor, 0);
        uint8_t destGreen = ExpandFiveBitChannel(destinationColor, 5);
        uint8_t destBlue = ExpandFiveBitChannel(destinationColor, 10);
        int32_t inverseAlpha = 255 - color.W;
        uint8_t outRed = static_cast<uint8_t>(((static_cast<int32_t>(color.X) * color.W) + (static_cast<int32_t>(destRed) * inverseAlpha) + 127) / 255);
        uint8_t outGreen = static_cast<uint8_t>(((static_cast<int32_t>(color.Y) * color.W) + (static_cast<int32_t>(destGreen) * inverseAlpha) + 127) / 255);
        uint8_t outBlue = static_cast<uint8_t>(((static_cast<int32_t>(color.Z) * color.W) + (static_cast<int32_t>(destBlue) * inverseAlpha) + 127) / 255);
        frameBuffer[pixelIndex] = PackOpaqueByteColor(outRed, outGreen, outBlue);
    }

    /// Writes one fully opaque pixel into the active DS framebuffer without alpha blending.
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

    /// Draws one horizontal span into the active DS framebuffer using either opaque or alpha-blended writes.
    /// <param name="destX">Destination rectangle X coordinate in framebuffer space.</param>
    /// <param name="destY">Destination row Y coordinate in framebuffer space.</param>
    /// <param name="startX">Inclusive local start X coordinate inside the destination rectangle.</param>
    /// <param name="endX">Exclusive local end X coordinate inside the destination rectangle.</param>
    /// <param name="color">Span color.</param>
    /// <param name="useOpaqueWrite">True to bypass alpha blending; otherwise false.</param>
    void NintendoDsRenderManager2D::DrawHorizontalSpan(int32_t destX, int32_t destY, int32_t startX, int32_t endX, const byte4& color, bool useOpaqueWrite) {
        if (endX <= startX) {
            return;
        }

        if (useOpaqueWrite) {
            uint16_t packedColor = PackOpaqueByteColor(color.X, color.Y, color.Z);
            uint16_t* rowStart = ActiveCpuFrameBuffer + (destY * FrameBufferWidth) + destX;
            dmaFillHalfWords(packedColor | (static_cast<uint32_t>(packedColor) << 16), rowStart + startX, static_cast<uint32_t>((endX - startX) * sizeof(uint16_t)));
            return;
        }

        BlendHorizontalSpan(destX, destY, startX, endX, color);
    }

    /// Blends one constant-color horizontal span directly against the active framebuffer row without per-pixel clip checks.
    /// <param name="destX">Destination rectangle X coordinate in framebuffer space.</param>
    /// <param name="destY">Destination row Y coordinate in framebuffer space.</param>
    /// <param name="startX">Inclusive local start X coordinate inside the destination rectangle.</param>
    /// <param name="endX">Exclusive local end X coordinate inside the destination rectangle.</param>
    /// <param name="color">Constant span color.</param>
    void NintendoDsRenderManager2D::BlendHorizontalSpan(int32_t destX, int32_t destY, int32_t startX, int32_t endX, const byte4& color) {
        if (endX <= startX || ActiveCpuFrameBuffer == nullptr || color.W == 0) {
            return;
        }

        uint16_t* rowStart = ActiveCpuFrameBuffer + (destY * FrameBufferWidth) + destX + startX;
        uint8_t sourceRed = static_cast<uint8_t>(color.X >> 3);
        uint8_t sourceGreen = static_cast<uint8_t>(color.Y >> 3);
        uint8_t sourceBlue = static_cast<uint8_t>(color.Z >> 3);
        uint8_t inverseAlpha = static_cast<uint8_t>(255 - color.W);
        uint32_t alpha = color.W;
        std::array<uint16_t, 32> blendedRedLookup;
        std::array<uint16_t, 32> blendedGreenLookup;
        std::array<uint16_t, 32> blendedBlueLookup;
        for (int32_t channel = 0; channel < 32; channel++) {
            blendedRedLookup[channel] = static_cast<uint16_t>(((static_cast<uint32_t>(sourceRed) * alpha) + (static_cast<uint32_t>(channel) * inverseAlpha) + 127) / 255);
            blendedGreenLookup[channel] = static_cast<uint16_t>((((static_cast<uint32_t>(sourceGreen) * alpha) + (static_cast<uint32_t>(channel) * inverseAlpha) + 127) / 255) << 5);
            blendedBlueLookup[channel] = static_cast<uint16_t>((((static_cast<uint32_t>(sourceBlue) * alpha) + (static_cast<uint32_t>(channel) * inverseAlpha) + 127) / 255) << 10);
        }

        int32_t spanWidth = endX - startX;
        for (int32_t pixelIndex = 0; pixelIndex < spanWidth; pixelIndex++) {
            uint16_t destinationColor = rowStart[pixelIndex];
            rowStart[pixelIndex] = static_cast<uint16_t>(
                BIT(15)
                | blendedRedLookup[destinationColor & 31]
                | blendedGreenLookup[(destinationColor >> 5) & 31]
                | blendedBlueLookup[(destinationColor >> 10) & 31]);
        }
    }

    /// Returns whether one destination rectangle lies fully outside the active viewport clip rectangle.
    /// <param name="destX">Destination X coordinate in framebuffer space.</param>
    /// <param name="destY">Destination Y coordinate in framebuffer space.</param>
    /// <param name="width">Destination width in pixels.</param>
    /// <param name="height">Destination height in pixels.</param>
    /// <returns>True when the rectangle does not intersect the active clip rectangle.</returns>
    bool NintendoDsRenderManager2D::IsDestinationRectOutsideActiveClip(int32_t destX, int32_t destY, int32_t width, int32_t height) const {
        if (width <= 0 || height <= 0) {
            return true;
        }

        return destX >= ActiveClipRight
            || destY >= ActiveClipBottom
            || destX + width <= ActiveClipLeft
            || destY + height <= ActiveClipTop;
    }

    /// Attempts to draw one opaque rounded rectangle by blitting a cached raster instead of recomputing row geometry.
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
    bool NintendoDsRenderManager2D::TryRasterCachedOpaqueRoundedRect(
        int32_t destX,
        int32_t destY,
        int32_t width,
        int32_t height,
        int32_t radius,
        int32_t borderThickness,
        RoundedRectCorners corners,
        const byte4& fillColor,
        const byte4& borderColor) {
        NintendoDsOpaqueRoundedRectCacheKey cacheKey = BuildOpaqueRoundedRectCacheKey(width, height, radius, borderThickness, corners, fillColor, borderColor);
        std::unordered_map<NintendoDsOpaqueRoundedRectCacheKey, NintendoDsOpaqueRoundedRectCacheEntry, NintendoDsOpaqueRoundedRectCacheKeyHasher>::iterator cachedEntryIterator = OpaqueRoundedRectCache.find(cacheKey);
        if (cachedEntryIterator == OpaqueRoundedRectCache.end()) {
            NintendoDsOpaqueRoundedRectCacheEntry entry;
            entry.Pixels.assign(static_cast<size_t>(width * height), 0);
            entry.RowLeft.assign(static_cast<size_t>(height), 0);
            entry.RowRight.assign(static_cast<size_t>(height), 0);

            uint16_t packedFillColor = PackOpaqueByteColor(fillColor.X, fillColor.Y, fillColor.Z);
            uint16_t packedBorderColor = PackOpaqueByteColor(borderColor.X, borderColor.Y, borderColor.Z);
            int32_t innerWidth = std::max(static_cast<int32_t>(0), width - (borderThickness * 2));
            int32_t innerHeight = std::max(static_cast<int32_t>(0), height - (borderThickness * 2));
            int32_t innerRadius = std::max(static_cast<int32_t>(0), radius - borderThickness);

            for (int32_t localY = 0; localY < height; localY++) {
                int32_t outerLeft = ComputeRoundedRectSideInset(localY, width, height, radius, corners, true);
                int32_t outerRight = width - ComputeRoundedRectSideInset(localY, width, height, radius, corners, false);
                entry.RowLeft[localY] = outerLeft;
                entry.RowRight[localY] = outerRight;
                if (outerRight <= outerLeft) {
                    continue;
                }

                uint16_t* rowStart = entry.Pixels.data() + (localY * width);
                if (borderThickness <= 0) {
                    std::fill_n(rowStart + outerLeft, outerRight - outerLeft, packedFillColor);
                    continue;
                }

                if (innerWidth <= 0 || innerHeight <= 0) {
                    std::fill_n(rowStart + outerLeft, outerRight - outerLeft, packedBorderColor);
                    continue;
                }

                int32_t innerLocalY = localY - borderThickness;
                if (innerLocalY < 0 || innerLocalY >= innerHeight) {
                    std::fill_n(rowStart + outerLeft, outerRight - outerLeft, packedBorderColor);
                    continue;
                }

                int32_t innerLeft = borderThickness + ComputeRoundedRectSideInset(innerLocalY, innerWidth, innerHeight, innerRadius, corners, true);
                int32_t innerRight = borderThickness + innerWidth - ComputeRoundedRectSideInset(innerLocalY, innerWidth, innerHeight, innerRadius, corners, false);
                if (innerRight <= innerLeft) {
                    std::fill_n(rowStart + outerLeft, outerRight - outerLeft, packedBorderColor);
                    continue;
                }

                std::fill_n(rowStart + outerLeft, innerLeft - outerLeft, packedBorderColor);
                std::fill_n(rowStart + innerLeft, innerRight - innerLeft, packedFillColor);
                std::fill_n(rowStart + innerRight, outerRight - innerRight, packedBorderColor);
            }

            cachedEntryIterator = OpaqueRoundedRectCache.emplace(cacheKey, std::move(entry)).first;
        }

        const NintendoDsOpaqueRoundedRectCacheEntry& cachedEntry = cachedEntryIterator->second;
        int32_t startY = std::max(static_cast<int32_t>(0), ActiveClipTop - destY);
        int32_t endY = std::min(height, ActiveClipBottom - destY);
        int32_t clipLeft = std::max(static_cast<int32_t>(0), ActiveClipLeft - destX);
        int32_t clipRight = std::min(width, ActiveClipRight - destX);
        for (int32_t localY = startY; localY < endY; localY++) {
            int32_t rowLeft = std::max(cachedEntry.RowLeft[localY], clipLeft);
            int32_t rowRight = std::min(cachedEntry.RowRight[localY], clipRight);
            if (rowRight <= rowLeft) {
                continue;
            }

            const uint16_t* sourceRow = cachedEntry.Pixels.data() + (localY * width);
            uint16_t* destinationRow = ActiveCpuFrameBuffer + ((destY + localY) * FrameBufferWidth) + destX;
            std::copy_n(sourceRow + rowLeft, rowRight - rowLeft, destinationRow + rowLeft);
        }

        return true;
    }

    /// Computes the rounded horizontal inset contributed by the enabled corners for one rectangle row.
    /// <param name="localY">Row index inside the rectangle.</param>
    /// <param name="width">Rectangle width in pixels.</param>
    /// <param name="height">Rectangle height in pixels.</param>
    /// <param name="radius">Rounded-corner radius in pixels.</param>
    /// <param name="corners">Rounded-corner mask.</param>
    /// <returns>Horizontal inset in pixels for the supplied row.</returns>
    int32_t NintendoDsRenderManager2D::ComputeRoundedRectRowInset(int32_t localY, int32_t width, int32_t height, int32_t radius, RoundedRectCorners corners) const {
        int32_t leftInset = ComputeRoundedRectSideInset(localY, width, height, radius, corners, true);
        int32_t rightInset = ComputeRoundedRectSideInset(localY, width, height, radius, corners, false);
        return std::max(leftInset, rightInset);
    }

    /// Computes one side-specific rounded inset for the supplied rectangle row.
    /// <param name="localY">Row index inside the rectangle.</param>
    /// <param name="width">Rectangle width in pixels.</param>
    /// <param name="height">Rectangle height in pixels.</param>
    /// <param name="radius">Rounded-corner radius in pixels.</param>
    /// <param name="corners">Rounded-corner mask.</param>
    /// <param name="leftSide">True to compute the left inset; false for the right inset.</param>
    /// <returns>Horizontal inset in pixels for the requested side.</returns>
    int32_t NintendoDsRenderManager2D::ComputeRoundedRectSideInset(
        int32_t localY,
        int32_t width,
        int32_t height,
        int32_t radius,
        RoundedRectCorners corners,
        bool leftSide) const {
        if (radius <= 0 || localY < 0 || localY >= height) {
            return 0;
        }

        double pixelY = static_cast<double>(localY) + 0.5;
        double radiusValue = static_cast<double>(radius);
        bool roundedTop = leftSide
            ? IsCornerRounded(corners, RoundedRectCorners::TopLeft)
            : IsCornerRounded(corners, RoundedRectCorners::TopRight);
        bool roundedBottom = leftSide
            ? IsCornerRounded(corners, RoundedRectCorners::BottomLeft)
            : IsCornerRounded(corners, RoundedRectCorners::BottomRight);
        double rowDistance = -1.0;

        if (roundedTop && pixelY < radiusValue) {
            double deltaY = pixelY - radiusValue;
            rowDistance = std::sqrt(std::max(0.0, (radiusValue * radiusValue) - (deltaY * deltaY)));
        } else if (roundedBottom && pixelY > static_cast<double>(height) - radiusValue) {
            double deltaY = pixelY - (static_cast<double>(height) - radiusValue);
            rowDistance = std::sqrt(std::max(0.0, (radiusValue * radiusValue) - (deltaY * deltaY)));
        }

        if (rowDistance < 0.0) {
            return 0;
        }

        double inset = radiusValue - rowDistance;
        int32_t insetPixels = std::max(static_cast<int32_t>(0), static_cast<int32_t>(std::ceil(inset - 0.0001)));
        return std::min(radius, insetPixels);
    }

    /// Reads one cooked indexed texel and resolves it through the runtime palette payload.
    /// <param name="texture">Runtime texture carrying indexed texel and palette payloads.</param>
    /// <param name="sampleX">Sample X coordinate in texture space.</param>
    /// <param name="sampleY">Sample Y coordinate in texture space.</param>
    /// <returns>Decoded RGBA texel.</returns>
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

    /// Rasterizes one textured quad into the DS active screen framebuffer.
    /// <param name="texture">DS software texture carrying the sampled pixel payload.</param>
    /// <param name="sourceRect">Normalized source rectangle inside the texture atlas.</param>
    /// <param name="destX">Destination X coordinate in screen space.</param>
    /// <param name="destY">Destination Y coordinate in screen space.</param>
    /// <param name="destWidth">Destination width in pixels.</param>
    /// <param name="destHeight">Destination height in pixels.</param>
    /// <param name="modulationColor">Per-drawable modulation color applied to sampled texels.</param>
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
        if (IsDestinationRectOutsideActiveClip(destX, destY, destWidth, destHeight)) {
            return;
        }

        int32_t sourceX = std::clamp(static_cast<int32_t>(std::round(sourceRect.X * textureWidth)), static_cast<int32_t>(0), textureWidth - 1);
        int32_t sourceY = std::clamp(static_cast<int32_t>(std::round(sourceRect.Y * textureHeight)), static_cast<int32_t>(0), textureHeight - 1);
        int32_t sourceWidth = std::clamp(static_cast<int32_t>(std::round(sourceRect.Z * textureWidth)), static_cast<int32_t>(1), textureWidth - sourceX);
        int32_t sourceHeight = std::clamp(static_cast<int32_t>(std::round(sourceRect.W * textureHeight)), static_cast<int32_t>(1), textureHeight - sourceY);
        int32_t startX = std::max(static_cast<int32_t>(0), ActiveClipLeft - destX);
        int32_t endX = std::min(destWidth, ActiveClipRight - destX);
        int32_t startY = std::max(static_cast<int32_t>(0), ActiveClipTop - destY);
        int32_t endY = std::min(destHeight, ActiveClipBottom - destY);
        int32_t sourceXStep = (sourceWidth << 16) / destWidth;
        int32_t sourceYStep = (sourceHeight << 16) / destHeight;
        if (texture->ColorFormat == TextureAssetColorFormat::Rgba4444) {
            int32_t sampleYFixed = startY * sourceYStep;
            for (int32_t y = startY; y < endY; y++) {
                int32_t sampleY = sourceY + (sampleYFixed >> 16);
                int32_t sampleXFixed = startX * sourceXStep;
                for (int32_t x = startX; x < endX; x++) {
                    int32_t sampleX = sourceX + (sampleXFixed >> 16);
                    int32_t sourceIndex = ((sampleY * textureWidth) + sampleX) * 2;
                    uint16_t packedColor = static_cast<uint16_t>(texture->Colors->Data[sourceIndex] | (texture->Colors->Data[sourceIndex + 1] << 8));
                    byte4 sampledColor = UnpackRgba4444(packedColor);
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

    /// Rasterizes one rounded rectangle into the DS active screen framebuffer.
    /// <param name="shape">Rounded-rectangle drawable to rasterize.</param>
    void NintendoDsRenderManager2D::RasterRoundedRect(IRoundedRectDrawable2D* shape) {
        if (shape == nullptr) {
            throw new ArgumentNullException("shape");
        } else if (shape->get_Parent() == nullptr) {
            return;
        }

        int2 size = shape->get_Size();
        int32_t width = size.X;
        int32_t height = size.Y;
        if (width <= 0 || height <= 0) {
            return;
        }

        float3 position = shape->get_Parent()->get_Position();
        int32_t destX = ActiveViewportOffsetX + static_cast<int32_t>(std::round(position.X));
        int32_t destY = ActiveViewportOffsetY + static_cast<int32_t>(std::round(position.Y));
        if (IsDestinationRectOutsideActiveClip(destX, destY, width, height)) {
            return;
        }

        int32_t maxRadius = std::min(width, height) / 2;
        int32_t radius = std::clamp(static_cast<int32_t>(std::round(shape->get_Radius())), static_cast<int32_t>(0), maxRadius);
        int32_t borderThickness = std::max(static_cast<int32_t>(0), static_cast<int32_t>(std::round(shape->get_BorderThickness())));
        RoundedRectCorners corners = shape->get_Corners();
        byte4 fillColor = shape->get_FillColor();
        byte4 borderColor = shape->get_BorderColor();
        bool useOpaqueFillWrite = fillColor.W >= 255;
        bool useOpaqueBorderWrite = borderColor.W >= 255;
        if (useOpaqueFillWrite && useOpaqueBorderWrite) {
            if (TryRasterCachedOpaqueRoundedRect(destX, destY, width, height, radius, borderThickness, corners, fillColor, borderColor)) {
                return;
            }
        }

        int32_t innerWidth = std::max(static_cast<int32_t>(0), width - (borderThickness * 2));
        int32_t innerHeight = std::max(static_cast<int32_t>(0), height - (borderThickness * 2));
        int32_t innerRadius = std::max(static_cast<int32_t>(0), radius - borderThickness);
        int32_t endX = std::min(width, ActiveClipRight - destX);
        int32_t startY = std::max(static_cast<int32_t>(0), ActiveClipTop - destY);
        int32_t endY = std::min(height, ActiveClipBottom - destY);

        for (int32_t localY = startY; localY < endY; localY++) {
            int32_t outerInset = ComputeRoundedRectRowInset(localY, width, height, radius, corners);
            int32_t outerLeft = std::max(std::max(static_cast<int32_t>(0), ActiveClipLeft - destX), ComputeRoundedRectSideInset(localY, width, height, radius, corners, true));
            int32_t outerRight = std::min(endX, width - ComputeRoundedRectSideInset(localY, width, height, radius, corners, false));
            if (outerRight <= outerLeft) {
                continue;
            }

            if (borderThickness <= 0) {
                DrawHorizontalSpan(destX, destY + localY, outerLeft, outerRight, fillColor, useOpaqueFillWrite);
                continue;
            }

            if (innerWidth <= 0 || innerHeight <= 0) {
                DrawHorizontalSpan(destX, destY + localY, outerLeft, outerRight, borderColor, useOpaqueBorderWrite);
                continue;
            }

            int32_t innerLocalY = localY - borderThickness;
            if (innerLocalY < 0 || innerLocalY >= innerHeight) {
                DrawHorizontalSpan(destX, destY + localY, outerLeft, outerRight, borderColor, useOpaqueBorderWrite);
                continue;
            }

            int32_t innerInset = ComputeRoundedRectRowInset(innerLocalY, innerWidth, innerHeight, innerRadius, corners);
            int32_t innerLeft = std::max(outerLeft, borderThickness + ComputeRoundedRectSideInset(innerLocalY, innerWidth, innerHeight, innerRadius, corners, true));
            int32_t innerRight = std::min(outerRight, borderThickness + innerWidth - ComputeRoundedRectSideInset(innerLocalY, innerWidth, innerHeight, innerRadius, corners, false));
            if (innerRight <= innerLeft) {
                DrawHorizontalSpan(destX, destY + localY, outerLeft, outerRight, borderColor, useOpaqueBorderWrite);
                continue;
            }

            DrawHorizontalSpan(destX, destY + localY, outerLeft, innerLeft, borderColor, useOpaqueBorderWrite);
            DrawHorizontalSpan(destX, destY + localY, innerLeft, innerRight, fillColor, useOpaqueFillWrite);
            DrawHorizontalSpan(destX, destY + localY, innerRight, outerRight, borderColor, useOpaqueBorderWrite);
            if (outerInset < 0 || innerInset < 0) {
                throw new InvalidOperationException("Nintendo DS rounded-rectangle rasterization computed an invalid inset.");
            }
        }
    }

    /// Rasterizes one sprite into the DS active screen framebuffer.
    /// <param name="sprite">Sprite drawable to rasterize.</param>
    void NintendoDsRenderManager2D::RasterSprite(ISpriteDrawable2D* sprite) {
        if (sprite == nullptr) {
            throw new ArgumentNullException("sprite");
        } else if (sprite->get_Parent() == nullptr) {
            return;
        }

        NintendoDsRuntimeTexture2D* texture = dynamic_cast<NintendoDsRuntimeTexture2D*>(sprite->get_Texture());
        if (texture == nullptr) {
            return;
        }

        int32_t width = texture->get_Width();
        int32_t height = texture->get_Height();
        int2 spriteSize = sprite->get_Size();
        if (spriteSize.X > 0) {
            width = spriteSize.X;
        }
        if (spriteSize.Y > 0) {
            height = spriteSize.Y;
        }

        float3 position = sprite->get_Parent()->get_Position();
        RasterTexturedQuad(
            texture,
            sprite->get_SourceRect(),
            ActiveViewportOffsetX + static_cast<int32_t>(std::round(position.X)),
            ActiveViewportOffsetY + static_cast<int32_t>(std::round(position.Y)),
            width,
            height,
            sprite->get_Color());
    }

    /// Attempts to draw one text drawable through a cached precomposed bitmap instead of per-frame glyph layout and atlas sampling.
    /// <param name="text">Text drawable to draw.</param>
    /// <param name="texture">Runtime font atlas texture.</param>
    /// <param name="font">Font asset referenced by the drawable.</param>
    /// <returns>True when the text was drawn through the cache path; otherwise false.</returns>
    bool NintendoDsRenderManager2D::TryRasterCachedTextBitmap(ITextDrawable2D* text, NintendoDsRuntimeTexture2D* texture, FontAsset* font) {
        if (text == nullptr || texture == nullptr || font == nullptr || text->get_Parent() == nullptr) {
            return false;
        } else if (texture->ColorFormat != TextureAssetColorFormat::Indexed4 && texture->ColorFormat != TextureAssetColorFormat::Indexed8) {
            return false;
        }

        std::string content = text->get_Text();
        double fontScale = std::max(static_cast<double>(text->get_FontScale()), 0.0001);
        if (text->get_WrapText()) {
            int32_t maxWidth = 1;
            int2 textSize = text->get_Size();
            if (textSize.X > 0) {
                maxWidth = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(textSize.X / fontScale)));
            }

            content = TextLayoutUtils::WrapText(content, font, maxWidth);
        }

        byte4 color = text->get_Color();
        std::string cacheKey = texture->get_Id()
            + "|"
            + content
            + "|"
            + std::to_string(static_cast<int32_t>(std::round(fontScale * 1000.0)))
            + "|"
            + std::to_string(color.X)
            + ","
            + std::to_string(color.Y)
            + ","
            + std::to_string(color.Z)
            + ","
            + std::to_string(color.W);
        std::unordered_map<std::string, NintendoDsCachedTextBitmapEntry>::iterator cachedEntryIterator = TextBitmapCache.find(cacheKey);
        if (cachedEntryIterator == TextBitmapCache.end()) {
            double offsetX = 0.0;
            double offsetY = 0.0;
            double lineHeight = std::max(static_cast<double>(font->get_LineHeight()) * fontScale, 1.0);
            int32_t minX = 0;
            int32_t minY = 0;
            int32_t maxX = 0;
            int32_t maxY = 0;
            bool hasVisibleGlyph = false;

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

                int32_t glyphWidth = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(glyph.SourceRect.Z * font->get_AtlasWidth() * fontScale)));
                int32_t glyphHeight = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(glyph.SourceRect.W * font->get_AtlasHeight() * fontScale)));
                int32_t glyphX = static_cast<int32_t>(std::round(offsetX));
                int32_t glyphY = static_cast<int32_t>(std::round(std::round(offsetY) + (static_cast<double>(glyph.OffsetY) * fontScale)));
                if (!hasVisibleGlyph) {
                    minX = glyphX;
                    minY = glyphY;
                    maxX = glyphX + glyphWidth;
                    maxY = glyphY + glyphHeight;
                    hasVisibleGlyph = true;
                } else {
                    minX = std::min(minX, glyphX);
                    minY = std::min(minY, glyphY);
                    maxX = std::max(maxX, glyphX + glyphWidth);
                    maxY = std::max(maxY, glyphY + glyphHeight);
                }

                double advanceWidth = glyph.AdvanceWidth > 0.0f
                    ? static_cast<double>(glyph.AdvanceWidth) * fontScale
                    : static_cast<double>(glyphWidth);
                offsetX += advanceWidth;
            }

            NintendoDsCachedTextBitmapEntry entry;
            entry.Width = std::max(static_cast<int32_t>(1), maxX - minX);
            entry.Height = std::max(static_cast<int32_t>(1), maxY - minY);
            entry.Pixels.assign(static_cast<size_t>(entry.Width * entry.Height), 0);
            entry.RowLeft.assign(static_cast<size_t>(entry.Height), entry.Width);
            entry.RowRight.assign(static_cast<size_t>(entry.Height), 0);

            if (hasVisibleGlyph) {
                offsetX = 0.0;
                offsetY = 0.0;
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

                    int32_t glyphWidth = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(glyph.SourceRect.Z * font->get_AtlasWidth() * fontScale)));
                    int32_t glyphHeight = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(glyph.SourceRect.W * font->get_AtlasHeight() * fontScale)));
                    int32_t glyphX = static_cast<int32_t>(std::round(offsetX)) - minX;
                    int32_t glyphY = static_cast<int32_t>(std::round(std::round(offsetY) + (static_cast<double>(glyph.OffsetY) * fontScale))) - minY;
                    int32_t sourceX = std::clamp(static_cast<int32_t>(std::round(glyph.SourceRect.X * texture->get_Width())), static_cast<int32_t>(0), texture->get_Width() - 1);
                    int32_t sourceY = std::clamp(static_cast<int32_t>(std::round(glyph.SourceRect.Y * texture->get_Height())), static_cast<int32_t>(0), texture->get_Height() - 1);
                    int32_t sourceWidth = std::clamp(static_cast<int32_t>(std::round(glyph.SourceRect.Z * texture->get_Width())), static_cast<int32_t>(1), texture->get_Width() - sourceX);
                    int32_t sourceHeight = std::clamp(static_cast<int32_t>(std::round(glyph.SourceRect.W * texture->get_Height())), static_cast<int32_t>(1), texture->get_Height() - sourceY);
                    int32_t sourceXStep = (sourceWidth << 16) / glyphWidth;
                    int32_t sourceYStep = (sourceHeight << 16) / glyphHeight;
                    int32_t sampleYFixed = 0;
                    for (int32_t localY = 0; localY < glyphHeight; localY++) {
                        int32_t sampleY = sourceY + (sampleYFixed >> 16);
                        int32_t sampleXFixed = 0;
                        int32_t destinationY = glyphY + localY;
                        for (int32_t localX = 0; localX < glyphWidth; localX++) {
                            int32_t sampleX = sourceX + (sampleXFixed >> 16);
                            byte4 sampledColor = ReadIndexedColor(texture, sampleX, sampleY);
                            byte4 blendedColor(
                                MultiplyByteChannel(sampledColor.X, color.X),
                                MultiplyByteChannel(sampledColor.Y, color.Y),
                                MultiplyByteChannel(sampledColor.Z, color.Z),
                                MultiplyByteChannel(sampledColor.W, color.W));
                            if (blendedColor.W > 0) {
                                int32_t destinationX = glyphX + localX;
                                if (destinationX >= 0 && destinationX < entry.Width && destinationY >= 0 && destinationY < entry.Height) {
                                    entry.Pixels[static_cast<size_t>((destinationY * entry.Width) + destinationX)] = PackByte4ToUint32(blendedColor);
                                    entry.RowLeft[destinationY] = std::min(entry.RowLeft[destinationY], destinationX);
                                    entry.RowRight[destinationY] = std::max(entry.RowRight[destinationY], destinationX + 1);
                                }
                            }
                            sampleXFixed += sourceXStep;
                        }

                        sampleYFixed += sourceYStep;
                    }

                    double advanceWidth = glyph.AdvanceWidth > 0.0f
                        ? static_cast<double>(glyph.AdvanceWidth) * fontScale
                        : static_cast<double>(glyphWidth);
                    offsetX += advanceWidth;
                }
            }

            for (int32_t rowIndex = 0; rowIndex < entry.Height; rowIndex++) {
                if (entry.RowRight[rowIndex] <= entry.RowLeft[rowIndex]) {
                    entry.RowLeft[rowIndex] = 0;
                    entry.RowRight[rowIndex] = 0;
                }
            }

            cachedEntryIterator = TextBitmapCache.emplace(cacheKey, std::move(entry)).first;
        }

        const NintendoDsCachedTextBitmapEntry& entry = cachedEntryIterator->second;
        float3 position = text->get_Parent()->get_Position();
        int32_t destX = ActiveViewportOffsetX + static_cast<int32_t>(std::round(position.X));
        int32_t destY = ActiveViewportOffsetY + static_cast<int32_t>(std::round(position.Y));
        if (IsDestinationRectOutsideActiveClip(destX, destY, entry.Width, entry.Height)) {
            return true;
        }

        int32_t startY = std::max(static_cast<int32_t>(0), ActiveClipTop - destY);
        int32_t endY = std::min(entry.Height, ActiveClipBottom - destY);
        int32_t clipLeft = std::max(static_cast<int32_t>(0), ActiveClipLeft - destX);
        int32_t clipRight = std::min(entry.Width, ActiveClipRight - destX);
        for (int32_t localY = startY; localY < endY; localY++) {
            int32_t rowLeft = std::max(entry.RowLeft[localY], clipLeft);
            int32_t rowRight = std::min(entry.RowRight[localY], clipRight);
            if (rowRight <= rowLeft) {
                continue;
            }

            for (int32_t localX = rowLeft; localX < rowRight; localX++) {
                uint32_t packedPixel = entry.Pixels[static_cast<size_t>((localY * entry.Width) + localX)];
                if (packedPixel == 0) {
                    continue;
                }

                byte4 cachedColor = UnpackUint32ToByte4(packedPixel);
                if (cachedColor.W >= 255) {
                    WriteOpaquePixel(destX + localX, destY + localY, cachedColor);
                } else {
                    BlendPixel(destX + localX, destY + localY, cachedColor);
                }
            }
        }

        return true;
    }

    /// Rasterizes one text drawable into the DS active screen framebuffer.
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

        if (TryRasterCachedTextBitmap(text, texture, font)) {
            return;
        }

        std::string content = text->get_Text();
        double fontScale = std::max(static_cast<double>(text->get_FontScale()), 0.0001);
        if (text->get_WrapText()) {
            int32_t maxWidth = 1;
            int2 textSize = text->get_Size();
            if (textSize.X > 0) {
                maxWidth = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(textSize.X / fontScale)));
            }

            content = TextLayoutUtils::WrapText(content, font, maxWidth);
        }

        float3 position = text->get_Parent()->get_Position();
        double offsetX = 0.0;
        double offsetY = 0.0;
        double lineHeight = std::max(static_cast<double>(font->get_LineHeight()) * fontScale, 1.0);
        double baseX = ActiveViewportOffsetX + std::round(position.X);
        double baseY = ActiveViewportOffsetY + std::round(position.Y);
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

            int32_t glyphWidth = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(glyph.SourceRect.Z * font->get_AtlasWidth() * fontScale)));
            int32_t glyphHeight = std::max(static_cast<int32_t>(1), static_cast<int32_t>(std::round(glyph.SourceRect.W * font->get_AtlasHeight() * fontScale)));
            int32_t glyphX = static_cast<int32_t>(std::round(baseX + offsetX));
            int32_t glyphY = static_cast<int32_t>(std::round(baseY + std::round(offsetY) + (static_cast<double>(glyph.OffsetY) * fontScale)));
            RasterTexturedQuad(texture, glyph.SourceRect, glyphX, glyphY, glyphWidth, glyphHeight, color);

            double advanceWidth = glyph.AdvanceWidth > 0.0f
                ? static_cast<double>(glyph.AdvanceWidth) * fontScale
                : static_cast<double>(glyphWidth);
            offsetX += advanceWidth;
        }
    }

    /// Tests whether one pixel center lies inside the supplied rounded-rectangle silhouette.
    /// <param name="localX">Pixel-local X coordinate inside the rectangle.</param>
    /// <param name="localY">Pixel-local Y coordinate inside the rectangle.</param>
    /// <param name="width">Rectangle width in pixels.</param>
    /// <param name="height">Rectangle height in pixels.</param>
    /// <param name="radius">Rounded-corner radius in pixels.</param>
    /// <param name="corners">Rounded-corner mask.</param>
    /// <returns>True when the pixel center lies inside the silhouette.</returns>
    bool NintendoDsRenderManager2D::IsPointInsideRoundedRect(
        int32_t localX,
        int32_t localY,
        int32_t width,
        int32_t height,
        int32_t radius,
        RoundedRectCorners corners) const {
        if (localX < 0 || localY < 0 || localX >= width || localY >= height) {
            return false;
        } else if (radius <= 0) {
            return true;
        }

        double pixelX = static_cast<double>(localX) + 0.5;
        double pixelY = static_cast<double>(localY) + 0.5;
        double radiusValue = static_cast<double>(radius);

        if (pixelX < radiusValue && pixelY < radiusValue && IsCornerRounded(corners, RoundedRectCorners::TopLeft)) {
            double deltaX = pixelX - radiusValue;
            double deltaY = pixelY - radiusValue;
            return (deltaX * deltaX) + (deltaY * deltaY) <= radiusValue * radiusValue;
        }

        if (pixelX > static_cast<double>(width) - radiusValue && pixelY < radiusValue && IsCornerRounded(corners, RoundedRectCorners::TopRight)) {
            double deltaX = pixelX - (static_cast<double>(width) - radiusValue);
            double deltaY = pixelY - radiusValue;
            return (deltaX * deltaX) + (deltaY * deltaY) <= radiusValue * radiusValue;
        }

        if (pixelX < radiusValue && pixelY > static_cast<double>(height) - radiusValue && IsCornerRounded(corners, RoundedRectCorners::BottomLeft)) {
            double deltaX = pixelX - radiusValue;
            double deltaY = pixelY - (static_cast<double>(height) - radiusValue);
            return (deltaX * deltaX) + (deltaY * deltaY) <= radiusValue * radiusValue;
        }

        if (pixelX > static_cast<double>(width) - radiusValue && pixelY > static_cast<double>(height) - radiusValue && IsCornerRounded(corners, RoundedRectCorners::BottomRight)) {
            double deltaX = pixelX - (static_cast<double>(width) - radiusValue);
            double deltaY = pixelY - (static_cast<double>(height) - radiusValue);
            return (deltaX * deltaX) + (deltaY * deltaY) <= radiusValue * radiusValue;
        }

        return true;
    }

    /// Resolves whether one rounded-rectangle corner flag is enabled on the supplied mask.
    /// <param name="corners">Rounded-corner mask to query.</param>
    /// <param name="corner">Corner flag to test.</param>
    /// <returns>True when the flag is enabled on the mask.</returns>
    bool NintendoDsRenderManager2D::IsCornerRounded(RoundedRectCorners corners, RoundedRectCorners corner) const {
        return (static_cast<int32_t>(corners) & static_cast<int32_t>(corner)) != 0;
    }
}
#endif
