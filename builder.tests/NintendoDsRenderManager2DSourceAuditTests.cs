namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS 2D renderer source so the backend keeps only hardware-backed rendering paths.
/// </summary>
public class NintendoDsRenderManager2DSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS 2D renderer still traverses the active camera queue through the generated-core visitor flow.
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
    /// Verifies the Nintendo DS 2D renderer no longer declares CPU framebuffers, software presentation, or software-visibility state.
    /// </summary>
    [Fact]
    public void Source_whenDeclaringNintendoDsRenderManager2d_removesCpuCompositeState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("TopCpuFrameBuffer", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomCpuFrameBuffer", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("ActiveCpuFrameBuffer", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("void PresentFrame();", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("FrameHasVisibleSoftware2DWork", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("TextBitmapCache", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("dmaCopyHalfWords(3, TopCpuFrameBuffer.data()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("dmaCopyHalfWords(3, BottomCpuFrameBuffer.data()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsRenderManager2D::PresentFrame()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("FrameHasVisibleSoftware2DWork =", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer routes supported work through explicit hardware-only helpers and keeps unsupported handling visible.
    /// </summary>
    [Fact]
    public void Source_whenRouting2dDrawables_definesHardwareOnlyHelpersAndUnsupportedMarkerFlow() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool TryDrawHardwareSprite(ISpriteDrawable2D* sprite);", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryDrawHardwareText(ITextDrawable2D* text);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void LogUnsupportedDrawable(const char* category, IDrawable2D* drawable);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void DrawUnsupportedDrawableMarker(int32_t x, int32_t y, NintendoDsScreenTarget targetScreen);", headerSource, StringComparison.Ordinal);
        Assert.Contains("int2 ResolveUnsupportedDrawableMarkerPosition(IDrawable2D* drawable) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t BottomScreenTextSweepFrameIndex;", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::array<int32_t, 24> BottomScreenConsoleRowLastWrittenFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void SweepExpiredBottomScreenConsoleRows();", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool UnsupportedSpriteLoggedThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool UnsupportedTextLoggedThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool UnsupportedRoundedRectLoggedThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("TryDrawHardwareSprite(sprite)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryDrawHardwareText(text)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LogUnsupportedDrawable(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DrawUnsupportedDrawableMarker(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ResolveUnsupportedDrawableMarkerPosition(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenConsoleRowLastWrittenFrame.fill(-1);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SweepExpiredBottomScreenConsoleRows();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenConsoleRowLastWrittenFrame[static_cast<std::size_t>(currentRow)] = RuntimeHeartbeatFrameIndex;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UnsupportedSpriteLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UnsupportedTextLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UnsupportedRoundedRectLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("DrawUnsupportedDrawableMarker(ActiveViewportOffsetX, ActiveViewportOffsetY", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if (sprite == nullptr || ActiveViewportTargetsBottomScreen)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain(
            "bool NintendoDsRenderManager2D::TryDrawHardwareSprite(ISpriteDrawable2D* sprite) {\r\n        if (sprite == nullptr) {\r\n            return false;\r\n        }\r\n\r\n        return false;\r\n    }",
            sourceCode,
            StringComparison.Ordinal);
        Assert.Contains("sprite->get_Texture()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("sprite->get_Rotation()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("sprite->get_Color()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("sprite->get_SourceRect()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("sprite->get_Size()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("sprite->get_Parent()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("vramSetBankI(VRAM_I_SUB_SPRITE);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("targetBottomScreen ? &oamSub : &oamMain", sourceCode, StringComparison.Ordinal);
        Assert.Contains("drawable->get_Parent()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("parent->get_Position()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("oamSet(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain(
            "bool NintendoDsRenderManager2D::TryDrawHardwareText(ITextDrawable2D* text) {\r\n        if (text == nullptr) {\r\n            return false;\r\n        }\r\n\r\n        return false;\r\n    }",
            sourceCode,
            StringComparison.Ordinal);
        Assert.Contains("ActiveViewportTargetsBottomScreen", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_Text()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_WrapText()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_Font()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_FontScale()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_Alignment()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_Color()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_SourceRect()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_Parent()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if ((screenX & 7) != 0 || (screenY & 7) != 0)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::round(static_cast<double>(screenX) / 8.0)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::round(static_cast<double>(screenY) / 8.0)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::clamp(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"\\x1b[%d;%dH%*s\", currentRow, column, visibleColumnCount, \"\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"\\x1b[%d;0H%*s\", row, ConsoleColumns, \"\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("constexpr int32_t BottomScreenConsoleRowPersistenceFrames = 60;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomScreenTextClearedThisFrame", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("consoleClear();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer no longer preserves software raster helpers or software text-cache prewarm entry points.
    /// </summary>
    [Fact]
    public void Source_whenEnforcingHardwareOnlyPolicy_removesSoftwareFallbackHelpers() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("void PrewarmTextDrawable(ITextDrawable2D* text);", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("RasterRoundedRect(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("RasterText(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TextFallbackGlyphCount", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("TextCachedBitmapRunCount", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("TextCachedBitmapCopiedPixelCount", headerSource, StringComparison.Ordinal);
    }
}
