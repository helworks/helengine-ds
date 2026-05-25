namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS 2D renderer source so menu draw calls route into concrete raster helpers instead of no-op discard bodies.
/// </summary>
public class NintendoDsRenderManager2DSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS 2D renderer traverses the active camera queue and dispatches drawables through their generated-core draw entry points.
    /// </summary>
    [Fact]
    public void Source_whenRenderingMenuScene_implementsCameraQueueVisitorTraversal() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("public RenderManager2D, public IRenderVisitor2D", headerSource, StringComparison.Ordinal);
        Assert.Contains("void BeginFrame();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void DrawCamera(ICamera* camera);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void Visit(IDrawable2D* drawable) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("camera->get_RenderQueue2D()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ResolveViewportTarget(viewport, targetBottomScreen, viewportX, viewportY, viewportWidth, viewportHeight);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SelectViewportTarget(targetBottomScreen, viewportX, viewportY, viewportWidth, viewportHeight);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderQueue->VisitOrdered(this);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("drawable->Draw();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer routes sprite, text, and rounded-rectangle draws to concrete raster helpers.
    /// </summary>
    [Fact]
    public void Source_whenDrawingMenuPrimitives_noLongerUsesNoOpDiscardBodies() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("std::array<uint16_t, VisibleFrameBufferPixelCount> TopCpuFrameBuffer;", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::array<uint16_t, VisibleFrameBufferPixelCount> BottomCpuFrameBuffer;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void PresentFrame();", headerSource, StringComparison.Ordinal);
        Assert.Contains("dmaCopyHalfWords(3, TopCpuFrameBuffer.data(), BG_BMP_RAM(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("dmaCopyHalfWords(3, BottomCpuFrameBuffer.data(), BG_BMP_RAM_SUB(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RasterRoundedRect(shape);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RasterSprite(sprite);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RasterText(text);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)shape;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)sprite;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)text;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D texture builder adopts raw texture payloads instead of allocating a second RGBA copy.
    /// </summary>
    [Fact]
    public void Source_whenBuildingRuntimeTexture_adoptsRawColorPayloadWithoutDuplicateAllocation() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void ReleaseTexture(RuntimeTexture* texture) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("texture->Colors = data->Colors;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("data->Colors = Array<uint8_t>::Empty();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("new Array<uint8_t>(data->Colors->Length);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::memcpy(texture->Colors->Data, data->Colors->Data", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer can rebuild one builder-owned cooked texture payload by deserializing the packaged texture asset and forwarding it through the raw builder path.
    /// </summary>
    [Fact]
    public void Source_whenResolvingCookedPlatformOwnedTexture_reusesRawTextureBuilderPath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("RuntimeTexture* BuildTextureFromCooked(std::string cookedAssetPath) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("#include \"AssetSerializer.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#include \"Asset.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#include \"runtime/native_cast.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#include \"system/io/file.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("::FileStream* stream = nullptr;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("::Asset* asset = nullptr;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("stream = ::File::OpenRead(cookedAssetPath);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastTextureBuildStage = \"BuildTextureFromCookedOpened\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("asset = ::AssetSerializer::Deserialize(stream);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastTextureBuildStage = \"BuildTextureFromCookedDeserialized\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("::TextureAsset* cookedTextureAsset = he_cpp_try_cast<TextureAsset>(asset);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastTextureBuildStage = \"BuildTextureFromCookedTyped\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RuntimeTexture* runtimeTexture = BuildTextureFromRaw(cookedTextureAsset);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer preserves cooked texture formats and branches packed texture sampling through a dedicated decode path.
    /// </summary>
    [Fact]
    public void Source_whenBuildingPackedTextures_tracksColorFormatAndDecodesRgba4444Samples() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeTexture2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("TextureAssetColorFormat ColorFormat;", headerSource, StringComparison.Ordinal);
        Assert.Contains("texture->ColorFormat = data->ColorFormat;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (texture->ColorFormat == TextureAssetColorFormat::Rgba4444)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t sourceIndex = ((sampleY * textureWidth) + sampleX) * 2;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer decodes indexed cooked textures through dedicated palette branches.
    /// </summary>
    [Fact]
    public void Source_whenBuildingIndexedTextures_decodesPalettePayloadsForIndexed4AndIndexed8() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("TextureAssetColorFormat::Indexed4", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TextureAssetColorFormat::Indexed8", sourceCode, StringComparison.Ordinal);
        Assert.Contains("texture->PaletteColors", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ReadIndexedColor(texture, sampleX, sampleY)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D text path honors the shared font-scale contract used by desktop renderers.
    /// </summary>
    [Fact]
    public void Source_whenDrawingText_appliesFontScaleToWrappingGlyphBoundsAndAdvance() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("double fontScale = std::max(static_cast<double>(text->get_FontScale()), 0.0001);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::round(textSize.X / fontScale)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("static_cast<double>(font->get_LineHeight()) * fontScale", sourceCode, StringComparison.Ordinal);
        Assert.Contains("font->get_FontInfo()->get_SpaceWidth()) * fontScale", sourceCode, StringComparison.Ordinal);
        Assert.Contains("font->get_AtlasWidth() * fontScale", sourceCode, StringComparison.Ordinal);
        Assert.Contains("font->get_AtlasHeight() * fontScale", sourceCode, StringComparison.Ordinal);
        Assert.Contains("static_cast<double>(glyph.AdvanceWidth) * fontScale", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer routes camera viewports to independent top and bottom screen backbuffers.
    /// </summary>
    [Fact]
    public void Source_whenCameraViewportTargetsBottomScreen_routesDrawsAndClippingToBottomBuffer() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("uint16_t* ActiveCpuFrameBuffer;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ActiveViewportOffsetX;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ActiveViewportOffsetY;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool BottomScreenClearedThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("targetBottomScreen = resolvedViewportY >= VisibleScreenHeight;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("viewportY = targetBottomScreen ? resolvedViewportY - VisibleScreenHeight : resolvedViewportY;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ActiveCpuFrameBuffer = targetBottomScreen ? BottomCpuFrameBuffer.data() : TopCpuFrameBuffer.data();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ActiveViewportOffsetX + static_cast<int32_t>(std::round(position.X))", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ActiveViewportOffsetY + static_cast<int32_t>(std::round(position.Y))", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer exposes frame presentation to the owning 3D frame loop instead of leaving it private.
    /// </summary>
    [Fact]
    public void Source_whenPresentingComposite2dFrame_exposesPresentFrameBeforePrivateSection() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string headerSource = File.ReadAllText(headerPath);

        int presentFrameIndex = headerSource.IndexOf("void PresentFrame();", StringComparison.Ordinal);
        int privateSectionIndex = headerSource.IndexOf("private:", StringComparison.Ordinal);

        Assert.True(presentFrameIndex >= 0, "Expected NintendoDsRenderManager2D to declare PresentFrame().");
        Assert.True(privateSectionIndex >= 0, "Expected NintendoDsRenderManager2D to declare a private section.");
        Assert.True(
            presentFrameIndex < privateSectionIndex,
            "Expected NintendoDsRenderManager2D::PresentFrame() to remain publicly callable from the owning render loop.");
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer rejects fully clipped quads and clamps raster loops to the active viewport clip rectangle.
    /// </summary>
    [Fact]
    public void Source_whenRasterizingMenuPrimitives_appliesEarlyClipCullingBeforePerPixelLoops() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool IsDestinationRectOutsideActiveClip(int32_t destX, int32_t destY, int32_t width, int32_t height) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (IsDestinationRectOutsideActiveClip(destX, destY, destWidth, destHeight)) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t startX = std::max(static_cast<int32_t>(0), ActiveClipLeft - destX);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t endX = std::min(destWidth, ActiveClipRight - destX);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t startY = std::max(static_cast<int32_t>(0), ActiveClipTop - destY);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t endY = std::min(destHeight, ActiveClipBottom - destY);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer avoids per-pixel divisions for textured quads and bypasses alpha blending for opaque rounded-rectangle fills.
    /// </summary>
    [Fact]
    public void Source_whenOptimizingSoftware2dHotPath_usesFixedPointTextureStepsAndOpaquePixelWrites() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void WriteOpaquePixel(int32_t x, int32_t y, const byte4& color);", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t sourceXStep = (sourceWidth << 16) / destWidth;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t sourceYStep = (sourceHeight << 16) / destHeight;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t sampleYFixed = startY * sourceYStep;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t sampleXFixed = startX * sourceXStep;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("sampleXFixed += sourceXStep;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("sampleYFixed += sourceYStep;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool useOpaqueFillWrite = fillColor.W >= 255;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool useOpaqueBorderWrite = borderColor.W >= 255;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DrawHorizontalSpan(destX, destY + localY, outerLeft, innerLeft, borderColor, useOpaqueBorderWrite);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DrawHorizontalSpan(destX, destY + localY, innerLeft, innerRight, fillColor, useOpaqueFillWrite);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS rounded-rectangle rasterizer uses horizontal spans instead of per-pixel silhouette tests in the inner loop.
    /// </summary>
    [Fact]
    public void Source_whenOptimizingRoundedRectRaster_usesSpanFillInsteadOfPerPixelShapeTests() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void DrawHorizontalSpan(int32_t destX, int32_t destY, int32_t startX, int32_t endX, const byte4& color, bool useOpaqueWrite);", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ComputeRoundedRectRowInset(int32_t localY, int32_t width, int32_t height, int32_t radius, RoundedRectCorners corners) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t outerInset = ComputeRoundedRectRowInset(localY, width, height, radius, corners);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DrawHorizontalSpan(destX, destY + localY, outerLeft, outerRight, borderColor, useOpaqueBorderWrite);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DrawHorizontalSpan(destX, destY + localY, innerLeft, innerRight, fillColor, useOpaqueFillWrite);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if (!IsPointInsideRoundedRect(localX, localY, width, height, radius, corners))", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS rounded-rectangle span writer uses direct row fills for opaque spans instead of per-pixel helper calls.
    /// </summary>
    [Fact]
    public void Source_whenWritingOpaqueRoundedRectSpans_usesDirectFramebufferRowFill() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("uint16_t packedColor = PackOpaqueByteColor(color.X, color.Y, color.Z);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t* rowStart = ActiveCpuFrameBuffer + (destY * FrameBufferWidth) + destX;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("dmaFillHalfWords(packedColor | (static_cast<uint32_t>(packedColor) << 16), rowStart + startX, static_cast<uint32_t>((endX - startX) * sizeof(uint16_t)));", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteOpaquePixel(destX + localX, destY, color);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D presenter uses DMA copies when pushing composed framebuffers into visible VRAM.
    /// </summary>
    [Fact]
    public void Source_whenPresentingDs2dFrame_usesDmaCopyToVisibleVram() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include <nds/dma.h>", sourceCode, StringComparison.Ordinal);
        Assert.Contains("dmaCopyHalfWords(3, TopCpuFrameBuffer.data(), BG_BMP_RAM(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("dmaCopyHalfWords(3, BottomCpuFrameBuffer.data(), BG_BMP_RAM_SUB(0), VisibleFrameBufferPixelCount * sizeof(uint16_t));", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::copy_n(TopCpuFrameBuffer.data(), VisibleFrameBufferPixelCount, BG_BMP_RAM(0));", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer caches opaque rounded-rectangle button geometry and blits cached row runs instead of recomputing spans every frame.
    /// </summary>
    [Fact]
    public void Source_whenDrawingOpaqueRoundedRects_usesCachedRowRunBitmaps() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("struct NintendoDsOpaqueRoundedRectCacheEntry", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::unordered_map<NintendoDsOpaqueRoundedRectCacheKey, NintendoDsOpaqueRoundedRectCacheEntry, NintendoDsOpaqueRoundedRectCacheKeyHasher> OpaqueRoundedRectCache;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryRasterCachedOpaqueRoundedRect(", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (TryRasterCachedOpaqueRoundedRect(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("OpaqueRoundedRectCache.find(cacheKey)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("cachedEntry.RowLeft[localY]", sourceCode, StringComparison.Ordinal);
        Assert.Contains("cachedEntry.RowRight[localY]", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::copy_n(sourceRow + rowLeft, rowRight - rowLeft, destinationRow + rowLeft);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("dmaCopyHalfWords(3, sourceRow + rowLeft, destinationRow + rowLeft, static_cast<uint32_t>((rowRight - rowLeft) * sizeof(uint16_t)));", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer caches rasterized text bitmaps so repeated static labels avoid per-frame glyph layout and glyph-quad rasterization.
    /// </summary>
    [Fact]
    public void Source_whenDrawingStaticText_usesCachedTextBitmapBlits() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("struct NintendoDsCachedTextBitmapEntry", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::unordered_map<std::string, NintendoDsCachedTextBitmapEntry> TextBitmapCache;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryRasterCachedTextBitmap(ITextDrawable2D* text, NintendoDsRuntimeTexture2D* texture, FontAsset* font);", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (TryRasterCachedTextBitmap(text, texture, font)) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TextBitmapCache.find(cacheKey)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("entry.Pixels", sourceCode, StringComparison.Ordinal);
        Assert.Contains("entry.RowLeft[localY]", sourceCode, StringComparison.Ordinal);
        Assert.Contains("entry.RowRight[localY]", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS text bitmap cache is bounded so dynamic overlay strings cannot leak memory indefinitely.
    /// </summary>
    [Fact]
    public void Source_whenCachingTextBitmaps_capsCacheGrowthAndFallsBackToUncachedRasterOnOverflow() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("static constexpr int32_t MaximumCachedTextBitmapEntryCount = 64;", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (static_cast<int32_t>(TextBitmapCache.size()) >= MaximumCachedTextBitmapEntryCount) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("cachedEntryIterator = TextBitmapCache.emplace(cacheKey, std::move(entry)).first;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS opaque rounded-rectangle cache is bounded so color-animated menu chrome cannot leak cached surfaces indefinitely.
    /// </summary>
    [Fact]
    public void Source_whenCachingOpaqueRoundedRects_capsCacheGrowthAndFallsBackToDirectRasterOnOverflow() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("static constexpr int32_t MaximumCachedOpaqueRoundedRectEntryCount = 64;", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (static_cast<int32_t>(OpaqueRoundedRectCache.size()) >= MaximumCachedOpaqueRoundedRectEntryCount) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("cachedEntryIterator = OpaqueRoundedRectCache.emplace(cacheKey, std::move(entry)).first;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS texture release path leaves top-level runtime texture ownership to the shared scene manager.
    /// </summary>
    [Fact]
    public void Source_whenReleasingTexture_releasesOnlyNintendoDsPixelPayloads() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);
        int releaseTextureStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseTexture(RuntimeTexture* texture)", StringComparison.Ordinal);
        int releaseFontStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseFont(FontAsset* font)", StringComparison.Ordinal);
        string releaseTextureBody = sourceCode[releaseTextureStart..releaseFontStart];

        Assert.Contains("delete runtimeTexture->Colors;", releaseTextureBody, StringComparison.Ordinal);
        Assert.Contains("delete runtimeTexture->PaletteColors;", releaseTextureBody, StringComparison.Ordinal);
        Assert.DoesNotContain("texture->Dispose();", releaseTextureBody, StringComparison.Ordinal);
        Assert.DoesNotContain("delete texture;", releaseTextureBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS font release path frees the owned atlas runtime texture before disposing the font asset itself.
    /// </summary>
    [Fact]
    public void Source_whenReleasingFont_releasesFontAtlasTextureBeforeDeletingFontAsset() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("RuntimeTexture* texture = font->get_Texture();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (texture != nullptr && !texture->get_IsDisposed()) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ReleaseTexture(texture);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("texture->Dispose();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("delete texture;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("font->Dispose();", sourceCode, StringComparison.Ordinal);
        Assert.True(
            sourceCode.IndexOf("ReleaseTexture(texture);", StringComparison.Ordinal)
                < sourceCode.IndexOf("font->Dispose();", StringComparison.Ordinal),
            "Expected DS font release to free the atlas texture before disposing the font asset.");
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer drops transient text and rounded-rectangle caches when the shared scene manager flushes released textures during scene transitions.
    /// </summary>
    [Fact]
    public void Source_whenFlushingReleasedTextures_clearsTransient2dOptimizationCaches() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void FlushReleasedTextures() override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::FlushReleasedTextures()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("swap(TextBitmapCache);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("swap(OpaqueRoundedRectCache);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer exposes live 2D cache counts so the bottom-screen diagnostics can distinguish cache growth from allocator accounting drift.
    /// </summary>
    [Fact]
    public void Source_whenReporting2dDiagnostics_exposesCacheEntryCounts() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("int32_t get_OpaqueRoundedRectCacheEntryCount() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t get_TextBitmapCacheEntryCount() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("return static_cast<int32_t>(OpaqueRoundedRectCache.size());", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return static_cast<int32_t>(TextBitmapCache.size());", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer blends constant-color rounded-rectangle spans through one row-local fast path instead of per-pixel BlendPixel calls.
    /// </summary>
    [Fact]
    public void Source_whenDrawingTranslucentRoundedRectSpans_usesRowLocalAlphaBlendLoop() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void BlendHorizontalSpan(int32_t destX, int32_t destY, int32_t startX, int32_t endX, const byte4& color);", headerSource, StringComparison.Ordinal);
        Assert.Contains("BlendHorizontalSpan(destX, destY, startX, endX, color);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t* rowStart = ActiveCpuFrameBuffer + (destY * FrameBufferWidth) + destX + startX;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint8_t sourceRed = static_cast<uint8_t>(color.X >> 3);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint8_t inverseAlpha = static_cast<uint8_t>(255 - color.W);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("destinationColor & 31", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BlendPixel(destX + localX, destY, color);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS translucent span blender precomputes 5-bit blend tables for constant rounded-rectangle colors instead of multiplying per pixel.
    /// </summary>
    [Fact]
    public void Source_whenBlendingConstantRoundedRectColor_usesPrecomputedFiveBitLookupTables() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("std::array<uint16_t, 32> blendedRedLookup;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::array<uint16_t, 32> blendedGreenLookup;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::array<uint16_t, 32> blendedBlueLookup;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("for (int32_t channel = 0; channel < 32; channel++) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("blendedRedLookup[channel]", sourceCode, StringComparison.Ordinal);
        Assert.Contains("blendedGreenLookup[channel]", sourceCode, StringComparison.Ordinal);
        Assert.Contains("blendedBlueLookup[channel]", sourceCode, StringComparison.Ordinal);
        Assert.Contains("blendedRedLookup[destinationColor & 31]", sourceCode, StringComparison.Ordinal);
        Assert.Contains("blendedGreenLookup[(destinationColor >> 5) & 31]", sourceCode, StringComparison.Ordinal);
        Assert.Contains("blendedBlueLookup[(destinationColor >> 10) & 31]", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer exposes frame-local profiling buckets for the native bottom-screen console.
    /// </summary>
    [Fact]
    public void Source_whenProfilingNintendoDs2dRenderer_tracksPerPrimitiveTimingAndCounts() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("struct NintendoDsRenderManager2DProfileSnapshot", headerSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRenderManager2DProfileSnapshot get_ProfileSnapshot() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double ProfileTextMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double ProfileSpriteMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double ProfileRoundedRectMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double ProfileClearMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double ProfileTotalFrameMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileTextPrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileSpritePrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileRoundedRectPrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("ProfileTotalFrameMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileTextMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileSpriteMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileRoundedRectMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileClearMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileTextPrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileSpritePrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileRoundedRectPrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileClearMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileTextPrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileSpritePrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileRoundedRectPrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool TryRasterFastIndexedText(ITextDrawable2D* text, NintendoDsRuntimeTexture2D* texture, FontAsset* font);", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (TryRasterFastIndexedText(text, texture, font)) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t packedOpaqueColor = PackOpaqueByteColor(sourceRed, sourceGreen, sourceBlue);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t* destinationRow = ActiveCpuFrameBuffer + (destinationY * FrameBufferWidth);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("RasterTexturedQuad(texture, glyph.SourceRect, glyphX, glyphY, glyphWidth, glyphHeight, color);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t timingStartTicks = cpuGetTiming();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("cpuGetTiming() - timingStartTicks", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("cpuStartTiming(0);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("cpuEndTiming()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("timerTicks2usec", sourceCode, StringComparison.Ordinal);
        Assert.Contains("snapshot.TotalFrameMilliseconds = snapshot.TextMilliseconds + snapshot.SpriteMilliseconds + snapshot.RoundedRectMilliseconds + snapshot.ClearMilliseconds;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::chrono::steady_clock", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("timerStart(0, ClockDivider_1024, 0, nullptr);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies DS native diagnostics text is filtered out of the software bitmap renderer so debug rows do not dominate the 2D timing bucket.
    /// </summary>
    [Fact]
    public void Source_whenDrawingNativeDiagnosticsText_skipsSoftwareBitmapRasterization() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool IsNativeDebugOverlayText(ITextDrawable2D* text) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (IsNativeDebugOverlayText(text)) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("StartsWith(textValue, \"Render FPS:\")", sourceCode, StringComparison.Ordinal);
        Assert.Contains("StartsWith(textValue, \"D3A \")", sourceCode, StringComparison.Ordinal);
        Assert.Contains("StartsWith(textValue, \"D2D \")", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EnsureActiveViewportCleared();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool FrameHasVisibleSoftware2DWork;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool get_FrameHasVisibleSoftware2DWork() const;", headerSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D presenter distinguishes overlay-on-3D presentation from pure 2D presentation by tracking per-frame hardware 3D ownership.
    /// </summary>
    [Fact]
    public void Source_whenOneScreenOwnsHardware3d_presents2dAsOverlayOnThatScreenAnd2dOnlyOnTheOther() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"platform/ds/NintendoDsScreenTarget.hpp\"", headerSource, StringComparison.Ordinal);
        Assert.Contains("void SetHardware3DScreenTarget(NintendoDsScreenTarget target);", headerSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsScreenTarget Hardware3DScreenTarget;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::SetHardware3DScreenTarget(NintendoDsScreenTarget target)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Hardware3DScreenTarget = target;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (Hardware3DScreenTarget == NintendoDsScreenTarget::Top)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (Hardware3DScreenTarget == NintendoDsScreenTarget::Bottom)", sourceCode, StringComparison.Ordinal);
    }
}
