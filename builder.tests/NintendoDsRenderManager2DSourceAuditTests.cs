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
        Assert.Contains("std::copy_n(TopCpuFrameBuffer.data(), VisibleFrameBufferPixelCount, BG_BMP_RAM(0));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::copy_n(BottomCpuFrameBuffer.data(), VisibleFrameBufferPixelCount, BG_BMP_RAM_SUB(0));", sourceCode, StringComparison.Ordinal);
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
        Assert.Contains("WriteOpaquePixel(destX + localX, destY + localY, borderColor);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("WriteOpaquePixel(destX + localX, destY + localY, fillColor);", sourceCode, StringComparison.Ordinal);
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
        Assert.Contains("double ProfileTotalFrameMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileTextPrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileSpritePrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileRoundedRectPrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("ProfileTotalFrameMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileTextMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileSpriteMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileRoundedRectMilliseconds = 0.0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileTextPrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileSpritePrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileRoundedRectPrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileTextPrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileSpritePrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileRoundedRectPrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileTotalFrameMilliseconds +=", sourceCode, StringComparison.Ordinal);
    }
}
