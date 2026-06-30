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
    /// Verifies the Nintendo DS top-screen runtime text path owns a dedicated BG0 text background instead of rejecting every non-bottom text drawable.
    /// </summary>
    [Fact]
    public void Source_whenRoutingTopScreenRuntimeText_definesDedicatedTopScreenBg0Path() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("int32_t TopScreenTextBackgroundId;", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint16_t* TopScreenTextMapEntries;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TopScreenTextBackgroundInitialized;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void EnsureTopScreenTextBackgroundReady();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ClearTopScreenTextMap();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void WriteTopScreenTextLine(int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount);", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ResolveTextBackgroundLayer(ITextDrawable2D* text) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::EnsureTopScreenTextBackgroundReady()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_EXT_PALETTE);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bgInit(0, BgType_Text4bpp, BgSize_T_256x256, 31, 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BG_PALETTE[0] = RGB15(0, 0, 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BG_PALETTE[1] = RGB15(31, 31, 31);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if (!ActiveViewportTargetsBottomScreen || !BottomScreenPresentationEnabled)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (ActiveViewportTargetsBottomScreen) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t backgroundLayer = ResolveTextBackgroundLayer(text);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (backgroundLayer != 0) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TraceUnsupportedTextDrawable(text, \"bgLayer\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EnsureBottomScreenTextBackgroundReady();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EnsureTopScreenTextBackgroundReady();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("text->get_RenderOrder2D() == 220", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("text->get_RenderOrder2D() == 42", sourceCode, StringComparison.Ordinal);
        Assert.Contains("WriteTopScreenTextLine(", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS top-screen runtime text path continues past BG0 initialization and writes the submitted line into the top-screen text map.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingTopScreenRuntimeText_doesNotReturnImmediatelyAfterBg0Initialization() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int tryDrawTextStart = sourceCode.IndexOf("bool NintendoDsRenderManager2D::TryDrawHardwareText(ITextDrawable2D* text)", StringComparison.Ordinal);
        int traceUnsupportedTextStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::TraceUnsupportedTextDrawable(ITextDrawable2D* text, const char* reason)", StringComparison.Ordinal);
        string tryDrawTextBody = sourceCode[tryDrawTextStart..traceUnsupportedTextStart];

        Assert.DoesNotContain("if (!ActiveViewportTargetsBottomScreen) {\n            EnsureTopScreenTextBackgroundReady();\n            return true;\n        }", tryDrawTextBody, StringComparison.Ordinal);
        Assert.Contains("if (ActiveViewportTargetsBottomScreen) {", tryDrawTextBody, StringComparison.Ordinal);
        Assert.Contains("int32_t backgroundLayer = ResolveTextBackgroundLayer(text);", tryDrawTextBody, StringComparison.Ordinal);
        Assert.Contains("if (backgroundLayer != 0) {", tryDrawTextBody, StringComparison.Ordinal);
        Assert.DoesNotContain("text->get_RenderOrder2D() == 42", tryDrawTextBody, StringComparison.Ordinal);
        Assert.Contains("EnsureTopScreenTextBackgroundReady();", tryDrawTextBody, StringComparison.Ordinal);
        Assert.Contains("WriteTopScreenTextLine(targetRow, startColumn, visibleLine, writableColumnCount);", tryDrawTextBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS text renderer reuses unchanged BG text submissions by keying off the shared engine-side text render-state version.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingUnchangedText_usesSharedDirtyStateAndSubmissionCache() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("struct NintendoDsHardwareTextSubmissionState", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::unordered_map<ITextDrawable2D*, NintendoDsHardwareTextSubmissionState> HardwareTextSubmissionStates;", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t TextSubmissionFrameStamp;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ResolveTextRenderStateVersion(ITextDrawable2D* text) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryReuseHardwareTextSubmission(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void PrepareHardwareTextSubmissionForRewrite(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void RememberHardwareTextSubmission(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ClearStaleHardwareTextSubmissions();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void InvalidateHardwareTextSubmissionCache(NintendoDsScreenTarget targetScreen);", headerSource, StringComparison.Ordinal);
        Assert.Contains("ClearStaleHardwareTextSubmissions();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("InvalidateHardwareTextSubmissionCache(targetScreen);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return text->get_TextRenderStateVersion();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (!TryReuseHardwareTextSubmission(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PrepareHardwareTextSubmissionForRewrite(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RememberHardwareTextSubmission(", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS text renderer routes top and bottom screens through one shared screen-targeted text pipeline instead of duplicating separate implementations.
    /// </summary>
    [Fact]
    public void Source_whenManagingNintendoDsTextScreens_usesSharedScreenTargetHelpers() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void EnsureScreenTextBackgroundReady(NintendoDsScreenTarget targetScreen);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ClearScreenTextMap(NintendoDsScreenTarget targetScreen);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget targetScreen, FontAsset* font);", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryResolveScreenGlyphTileIndex(NintendoDsScreenTarget targetScreen, FontAsset* font, char character, uint16_t& tileIndex);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void WriteScreenTextLine(NintendoDsScreenTarget targetScreen, int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount);", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint16_t ResolveScreenGlyphTileIndex(NintendoDsScreenTarget targetScreen, char character) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::EnsureScreenTextBackgroundReady(NintendoDsScreenTarget targetScreen)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::ClearScreenTextMap(NintendoDsScreenTarget targetScreen)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget targetScreen, FontAsset* font)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool NintendoDsRenderManager2D::TryResolveScreenGlyphTileIndex(NintendoDsScreenTarget targetScreen, FontAsset* font, char character, uint16_t& tileIndex)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::WriteScreenTextLine(NintendoDsScreenTarget targetScreen, int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t NintendoDsRenderManager2D::ResolveScreenGlyphTileIndex(NintendoDsScreenTarget targetScreen, char character) const", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the legacy per-screen text-glyph wrappers delegate directly into the shared screen-targeted helpers without keeping unreachable duplicate implementations behind early returns.
    /// </summary>
    [Fact]
    public void Source_whenManagingNintendoDsTextGlyphWrappers_keepsOnlySharedHelperDelegation() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int ensureBottomStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::EnsureBottomScreenFontGlyphTilesReady(FontAsset* font)", StringComparison.Ordinal);
        int ensureTopStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::EnsureTopScreenFontGlyphTilesReady(FontAsset* font)", StringComparison.Ordinal);
        int tryBottomStart = sourceCode.IndexOf("bool NintendoDsRenderManager2D::TryResolveBottomScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex)", StringComparison.Ordinal);
        int tryTopStart = sourceCode.IndexOf("bool NintendoDsRenderManager2D::TryResolveTopScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex)", StringComparison.Ordinal);
        int writeScreenStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::WriteScreenTextLine(", StringComparison.Ordinal);

        string ensureBottomBody = sourceCode[ensureBottomStart..ensureTopStart];
        string ensureTopBody = sourceCode[ensureTopStart..tryBottomStart];
        string tryBottomBody = sourceCode[tryBottomStart..tryTopStart];
        string tryTopBody = sourceCode[tryTopStart..writeScreenStart];

        Assert.Contains("EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget::Bottom, font);", ensureBottomBody, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomScreenTextGlyphTilesUploaded", ensureBottomBody, StringComparison.Ordinal);
        Assert.DoesNotContain("return;", ensureBottomBody, StringComparison.Ordinal);
        Assert.Contains("EnsureScreenFontGlyphTilesReady(NintendoDsScreenTarget::Top, font);", ensureTopBody, StringComparison.Ordinal);
        Assert.DoesNotContain("TopScreenTextGlyphTilesUploaded", ensureTopBody, StringComparison.Ordinal);
        Assert.DoesNotContain("return;", ensureTopBody, StringComparison.Ordinal);
        Assert.Contains("return TryResolveScreenGlyphTileIndex(NintendoDsScreenTarget::Bottom, font, character, tileIndex);", tryBottomBody, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomScreenGlyphResolveFailureReason", tryBottomBody, StringComparison.Ordinal);
        Assert.Contains("return TryResolveScreenGlyphTileIndex(NintendoDsScreenTarget::Top, font, character, tileIndex);", tryTopBody, StringComparison.Ordinal);
        Assert.DoesNotContain("TopScreenGlyphResolveFailureReason", tryTopBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the top-screen renderer no longer hardcodes the temporary proof overlay or proof OBJ path.
    /// </summary>
    [Fact]
    public void Source_whenDrawingTopScreenCamera_doesNotKeepTemporaryProofOverlayEnabled() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool NintendoDsRenderManager2D::get_TopScreenProofModeActive() const", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("return true;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteTopScreenTextLine(2, 1, \"HELLO\", 5);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("SubmitTopScreenProofSprite();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS pure-2D main-screen frame setup no longer submits the dedicated top-screen proof OBJ during begin-frame setup.
    /// </summary>
    [Fact]
    public void Source_whenBeginningPure2dMainScreenFrame_doesNotSubmitDedicatedTopProofObj() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        int beginFrameStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::BeginFrame()", StringComparison.Ordinal);
        int drawCameraStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::DrawCamera(ICamera* camera)", StringComparison.Ordinal);
        string beginFrameBody = sourceCode[beginFrameStart..drawCameraStart];

        Assert.Contains("if (!TopScreenTextBackgroundInitialized) {", beginFrameBody, StringComparison.Ordinal);
        Assert.Contains("EnsureTopScreenTextBackgroundReady();", beginFrameBody, StringComparison.Ordinal);
        Assert.Contains("oamClear(&oamMain, 0, 128);", beginFrameBody, StringComparison.Ordinal);
        Assert.DoesNotContain("SubmitTopScreenProofSprite();", beginFrameBody, StringComparison.Ordinal);
        Assert.DoesNotContain("oamUpdate(&oamMain);", beginFrameBody, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteBottomScreenTextLine(0, 1, \"B\", 1);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteBottomScreenTextLine(0, 0, \"T\", 1);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS bottom-screen camera clears stale BG0 text and OBJ state when the active bottom queue is empty.
    /// </summary>
    [Fact]
    public void Source_whenBottomScreenQueueIsEmpty_clearsStaleBg0AndObjState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int drawCameraStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::DrawCamera(ICamera* camera)", StringComparison.Ordinal);
        int visitStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::Visit(IDrawable2D* drawable)", StringComparison.Ordinal);
        string drawCameraBody = sourceCode[drawCameraStart..visitStart];

        Assert.Contains("int32_t renderQueueCount = renderQueue->get_Count();", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("if (targetBottomScreen && renderQueueCount == 0) {", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("ClearBottomScreenTextMap();", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("oamClear(&oamSub, 0, 128);", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("renderQueue->VisitOrdered(this);", drawCameraBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the first bottom-screen draw of a frame clears stale BG0 text and OBJ state only when the bottom queue shape changes.
    /// </summary>
    [Fact]
    public void Source_whenBottomScreenQueueShapeChanges_clearsStaleBg0AndObjStateBeforeSubmittingNewDraws() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        int drawCameraStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::DrawCamera(ICamera* camera)", StringComparison.Ordinal);
        int visitStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::Visit(IDrawable2D* drawable)", StringComparison.Ordinal);
        string drawCameraBody = sourceCode[drawCameraStart..visitStart];

        Assert.Contains("int32_t PreviousBottomScreenQueueCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (targetBottomScreen && BottomScreenPresentationEnabled && !BottomScreenClearedThisFrame) {", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("ClearScreen(camera, true);", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("if (LastBottomScreenQueueCount != PreviousBottomScreenQueueCount) {", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("ClearBottomScreenTextMap();", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("oamClear(&oamSub, 0, 128);", drawCameraBody, StringComparison.Ordinal);
        Assert.Contains("BottomScreenClearedThisFrame = true;", drawCameraBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies regular top-screen sprite and rounded-rectangle submissions are no longer suppressed by the removed proof mode.
    /// </summary>
    [Fact]
    public void Source_whenRenderingTopScreenScene_doesNotSuppressRegularSpriteAndRectDraws() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void NintendoDsRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if (!ActiveViewportTargetsBottomScreen) {\r\n            ProfileRoundedRectMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);\r\n            return;\r\n        }", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if (!ActiveViewportTargetsBottomScreen) {\r\n            ProfileSpriteMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);\r\n            return;\r\n        }", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileRoundedRectMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileSpriteMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS main-screen sprite path supports 256-color OBJ uploads for cooked sprites that exceed the 16-color fallback budget.
    /// </summary>
    [Fact]
    public void Source_whenPreparingMainScreenSprites_enablesExtendedPaletteAnd256ColorObjSupport() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string runtimeTextureHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeTexture2D.hpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);
        string runtimeTextureHeaderSource = File.ReadAllText(runtimeTextureHeaderPath);

        Assert.Contains("bool MainSpriteEngineExtendedPalettesEnabled;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool MainHardwareSpriteUses256Color;", runtimeTextureHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool TryBuildHardwareSpriteIndexed8(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void UploadHardwareSpriteExtendedPalette(", headerSource, StringComparison.Ordinal);
        Assert.Contains("vramSetBankE(VRAM_E_MAIN_SPRITE);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("oamInit(&oamMain, SpriteMapping_1D_32, true);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DISPLAY_SPR_EXT_PALETTE", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SpriteColorFormat_256Color", sourceCode, StringComparison.Ordinal);
        Assert.Contains("VRAM_F_EXT_SPR_PALETTE", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryBuildHardwareSpriteIndexed8(", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS text-isolation pass removes the dedicated top-screen proof sprite helpers so runtime validation only exercises real scene content.
    /// </summary>
    [Fact]
    public void Source_whenIsolatingRuntimeText_doesNotKeepDedicatedTopProofSpriteHelpers() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("void NintendoDsRenderManager2D::EnsureTopScreenProofSpriteResources()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsRenderManager2D::SubmitTopScreenProofSprite()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TopScreenProofSpriteGfx", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("UploadHardwareSpriteExtendedPalette(15, proofPaletteColors);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies bottom-screen presentation no longer appends renderer-owned diagnostic rows while text rendering is being isolated.
    /// </summary>
    [Fact]
    public void Source_whenIsolatingBottomScreenText_doesNotWriteRendererOwnedDiagnosticRows() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int presentStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::PresentBottomScreenFrame()", StringComparison.Ordinal);
        int presentEnd = sourceCode.IndexOf("void NintendoDsRenderManager2D::SetHardware3DScreenTarget", StringComparison.Ordinal);
        string presentBody = sourceCode[presentStart..presentEnd];

        Assert.DoesNotContain("std::string spriteLine = \"TS \";", presentBody, StringComparison.Ordinal);
        Assert.DoesNotContain("std::string queueLine = \"Q T\"", presentBody, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteBottomScreenTextLine(TopSpriteDiagnosticRow, 0, spriteLine, BottomScreenConsoleColumns);", presentBody, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteBottomScreenTextLine(TopSpriteDiagnosticRow, 0, \"\", BottomScreenConsoleColumns);", presentBody, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteBottomScreenTextLine(BottomScreenDiagnosticRow, 0, queueLine, BottomScreenConsoleColumns);", presentBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer presents the bottom screen through the DS text background path instead of the temporary bitmap copy proof.
    /// </summary>
    [Fact]
    public void Source_whenDeclaringNintendoDsRenderManager2d_presentsBottomScreenThroughBg0TextPath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void PresentBottomScreenFrame();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::PresentBottomScreenFrame()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EnsureBottomScreenTextBackgroundReady();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", sourceCode, StringComparison.Ordinal);
        int presentStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::PresentBottomScreenFrame()", StringComparison.Ordinal);
        int presentEnd = sourceCode.IndexOf("void NintendoDsRenderManager2D::SetHardware3DScreenTarget", StringComparison.Ordinal);
        string presentBody = sourceCode[presentStart..presentEnd];
        Assert.Contains("videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", presentBody, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);", presentBody, StringComparison.Ordinal);
        Assert.DoesNotContain("InteractionProbeVisibleColumnCount", headerSource, StringComparison.Ordinal);
        Assert.Contains("if (SubSpriteEngineInitialized || SubDebugMarkerInitialized) {", presentBody, StringComparison.Ordinal);
        Assert.Contains("oamUpdate(&oamSub);", presentBody, StringComparison.Ordinal);
        Assert.Contains("bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 31, 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bgSetPriority(BottomScreenTextBackgroundId, 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bgShow(BottomScreenTextBackgroundId);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("consoleInit(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BG_PALETTE_SUB[0] = RGB15(0, 0, 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BG_PALETTE_SUB[1] = RGB15(31, 31, 31);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextBackgroundInitialized = true;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::string content = \"HELLO WORLD\";", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteBottomScreenTextLine(12, 10, \"HELLO WORLD\", 11);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("dmaCopyHalfWords(3, BottomCpuFrameBuffer.data(), BG_BMP_RAM_SUB(0)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("UploadDiagnosticHelloWorldTiles(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteDiagnosticHelloWorldMap(", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies bottom-screen presentation does not reinitialize the Nintendo DS sub-screen video mode every frame after the BG0 text background has already been prepared.
    /// </summary>
    [Fact]
    public void Source_whenPresentingBottomScreenFrame_doesNotResetSubVideoModeEveryFrame() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int presentStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::PresentBottomScreenFrame()", StringComparison.Ordinal);
        int presentEnd = sourceCode.IndexOf("void NintendoDsRenderManager2D::SetHardware3DScreenTarget", StringComparison.Ordinal);
        string presentBody = sourceCode[presentStart..presentEnd];

        Assert.DoesNotContain("videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", presentBody, StringComparison.Ordinal);
        Assert.Contains("EnsureBottomScreenTextBackgroundReady();", presentBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS bottom-screen BG0 runtime text path consumes cooked font glyph tiles instead of synthetic proof rows.
    /// </summary>
    [Fact]
    public void Source_whenRoutingBottomScreenRuntimeText_usesCookedFontGlyphCacheAndTileMapWrites() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("const std::string& content = text->get_Text();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EnsureBottomScreenFontGlyphTilesReady(font);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextGlyphTileIndices.fill(static_cast<uint16_t>(0));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("for (const auto& characterEntry : *font->get_Characters())", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t tileIndex = static_cast<uint16_t>((characterCode - 32) + 1);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextGlyphTileIndices[static_cast<std::size_t>(characterCode - 32)] = tileIndex;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (!TryResolveBottomScreenGlyphTileIndex(BottomScreenTextGlyphCacheFont, character, tileIndex))", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextMapEntries[mapIndex] = tileIndex;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("char proofCharacter = 'H';", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("constexpr int32_t ProofGlyphRow =", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS runtime text path writes submitted lines directly into sequential console rows without the old injected proof line.
    /// </summary>
    [Fact]
    public void Source_whenRoutingBottomScreenRuntimeText_writesSequentialSubmittedRowsWithoutInjectedProofLine() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("constexpr int32_t BottomScreenRuntimeTextRow = 1;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("constexpr int32_t BottomScreenRuntimeTextColumn = 1;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("int32_t proofRow = BottomScreenRuntimeTextRow + BottomScreenSubmittedTextCountThisFrame;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t baseColumn = static_cast<int32_t>(std::round((parentPosition.X - static_cast<float>(ActiveViewportOffsetX)) / 8.0f));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t baseRow = static_cast<int32_t>(std::round((parentPosition.Y - static_cast<float>(ActiveViewportOffsetY)) / 8.0f));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t startColumn = ResolveAlignedConsoleColumn(baseColumn, boxColumnCount, visibleLength, alignment);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("WriteBottomScreenTextLine(targetRow, startColumn, visibleLine, writableColumnCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenSubmittedTextCountThisFrame++;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("static const std::string ThirdProofLine = \"Update\";", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("WriteBottomScreenTextLine(proofRow + 1", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer routes supported work through explicit hardware-only helpers and silently skips unsupported drawables while retaining counters.
    /// </summary>
    [Fact]
    public void Source_whenRouting2dDrawables_definesHardwareOnlyHelpersAndSilentUnsupportedSkip() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool TryDrawHardwareSprite(ISpriteDrawable2D* sprite);", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryDrawHardwareRectangle(IRoundedRectDrawable2D* shape);", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryDrawSolidHardwareRectangle(int32_t x, int32_t y, int32_t width, int32_t height, const byte4& color);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ReleaseFrameLocalRectangleGraphics();", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::vector<uint16_t> BuildSolidRectangleTilePixels(int32_t tileWidth, int32_t tileHeight, int32_t filledWidth, int32_t filledHeight, uint16_t packedColor) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryDrawHardwareText(ITextDrawable2D* text);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void EnsureBottomScreenTextBackgroundReady();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ClearBottomScreenTextMap();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void WriteBottomScreenTextLine(int32_t row, int32_t column, const std::string& line, int32_t visibleColumnCount);", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint16_t ResolveBottomScreenGlyphTileIndex(char character) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void EnsureBottomScreenFontGlyphTilesReady(FontAsset* font);", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryResolveBottomScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex);", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ResolveAlignedConsoleColumn(int32_t baseColumn, int32_t boxColumnCount, int32_t visibleLength, int32_t alignment) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void LogUnsupportedDrawable(const char* category, IDrawable2D* drawable);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void TraceUnsupportedTextDrawable(ITextDrawable2D* text, const char* reason);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void TraceUnsupportedSpriteDrawable(ISpriteDrawable2D* sprite, const char* reason);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void DrawUnsupportedDrawableMarker(int32_t x, int32_t y, NintendoDsScreenTarget targetScreen);", headerSource, StringComparison.Ordinal);
        Assert.Contains("int2 ResolveUnsupportedDrawableMarkerPosition(IDrawable2D* drawable) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t BottomScreenTextBackgroundId;", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint16_t* BottomScreenTextMapEntries;", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::array<uint16_t, 32 * 24> BottomScreenTextShadowEntries;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool BottomScreenTextBackgroundInitialized;", headerSource, StringComparison.Ordinal);
        Assert.Contains("FontAsset* BottomScreenTextGlyphCacheFont;", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::array<uint16_t, 95> BottomScreenTextGlyphTileIndices;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool BottomScreenTextGlyphTilesUploaded;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t UnsupportedTextTraceCountThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t UnsupportedSpriteTraceCountThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t UnsupportedTextPrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t UnsupportedSpritePrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t UnsupportedRoundedRectPrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileUnsupportedTextPrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileUnsupportedSpritePrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ProfileUnsupportedRoundedRectPrimitiveCount;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool UnsupportedSpriteLoggedThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool UnsupportedTextLoggedThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool UnsupportedRoundedRectLoggedThisFrame;", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::vector<void*> FrameLocalMainRectangleGraphics;", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::vector<void*> FrameLocalSubRectangleGraphics;", headerSource, StringComparison.Ordinal);
        Assert.Contains("TryDrawHardwareRectangle(shape)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryDrawHardwareSprite(sprite)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryDrawHardwareText(text)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryDrawSolidHardwareRectangle(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ReleaseFrameLocalRectangleGraphics();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BuildSolidRectangleTilePixels(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EnsureBottomScreenTextBackgroundReady();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ClearBottomScreenTextMap();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("WriteBottomScreenTextLine(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ResolveBottomScreenGlyphTileIndex(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EnsureBottomScreenFontGlyphTilesReady(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryResolveBottomScreenGlyphTileIndex(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ResolveAlignedConsoleColumn(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TraceUnsupportedTextDrawable(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TraceUnsupportedSpriteDrawable(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextShadowEntries.fill(static_cast<uint16_t>(0));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextGlyphTileIndices.fill(static_cast<uint16_t>(0));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("for (const auto& characterEntry : *font->get_Characters())", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextMapEntries[mapIndex] = tileIndex;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastTextureBuildStage = \"BuildTextureFromCookedOpened\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("asset = ::AssetSerializer::Deserialize(stream);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("::TextureAsset* textureAsset = he_cpp_try_cast<TextureAsset>(asset);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeTexture->ColorFormat = textureAsset->ColorFormat;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeTexture->PaletteColors = textureAsset->PaletteColors;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UnsupportedTextTraceCountThisFrame = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UnsupportedSpriteTraceCountThisFrame = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileUnsupportedTextPrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileUnsupportedSpritePrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileUnsupportedRoundedRectPrimitiveCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UnsupportedSpriteLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UnsupportedTextLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("UnsupportedRoundedRectLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("LogUnsupportedDrawable(\"Sprite\", sprite);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("LogUnsupportedDrawable(\"Text\", text);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("DrawUnsupportedDrawableMarker(markerPosition.X, markerPosition.Y", sourceCode, StringComparison.Ordinal);
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
        Assert.Contains("shape->get_Size()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("shape->get_FillColor()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("shape->get_BorderColor()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("shape->get_BorderThickness()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("(void)shape->get_Radius();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("shape->get_Radius() > 0", sourceCode, StringComparison.Ordinal);
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
        Assert.DoesNotContain("if (static_cast<int32_t>(text->get_Alignment()) != 0)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_Color()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_SourceRect()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("text->get_Parent()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"fontScale\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"wrap\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"alignment\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"color\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"sourceRect\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"charset\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"rotation\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"size\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"texture\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"textureSize\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"prepare\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#if defined(HELENGINE_DS_UNSUPPORTED_TRACE_ENABLED)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::fprintf(stderr,", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileUnsupportedTextPrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileUnsupportedSpritePrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ProfileUnsupportedRoundedRectPrimitiveCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("snapshot.UnsupportedTextPrimitiveCount = ProfileUnsupportedTextPrimitiveCount;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("snapshot.UnsupportedSpritePrimitiveCount = ProfileUnsupportedSpritePrimitiveCount;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("snapshot.UnsupportedRoundedRectPrimitiveCount = ProfileUnsupportedRoundedRectPrimitiveCount;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("return static_cast<uint16_t>(character - 32);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if ((screenX & 7) != 0 || (screenY & 7) != 0)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomScreenTextSweepFrameIndex", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomScreenConsoleRowLastWrittenFrame", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomScreenConsoleRowCachedText", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("SweepExpiredBottomScreenConsoleRows()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("iprintf(\"[helengine-ds]", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomScreenTextClearedThisFrame", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies bottom-screen DS sprite drawables reach the hardware sprite path instead of the software framebuffer raster path.
    /// </summary>
    [Fact]
    public void Source_whenDrawingBottomScreenSprites_routesThemThroughHardwareSubmission() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int drawSpriteStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite)", StringComparison.Ordinal);
        int drawSpriteEnd = sourceCode.IndexOf("void NintendoDsRenderManager2D::DrawText(ITextDrawable2D* text)", StringComparison.Ordinal);
        string drawSpriteBody = sourceCode[drawSpriteStart..drawSpriteEnd];

        Assert.Contains("if (!TryDrawHardwareSprite(sprite)) {", drawSpriteBody, StringComparison.Ordinal);
        Assert.DoesNotContain("if (ActiveViewportTargetsBottomScreen && ActiveCpuFrameBuffer != nullptr)", drawSpriteBody, StringComparison.Ordinal);
        Assert.DoesNotContain("RasterSprite(sprite);", drawSpriteBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies bottom-screen OBJ submissions use a lower hardware priority than BG0 text so menu labels remain visible above background sprites.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingBottomScreenObj_usesLowerPriorityThanBg0Text() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("constexpr int32_t TopScreenSpritePriority = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("constexpr int32_t BottomScreenSpritePriority = 1;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t spritePriority = targetBottomScreen ? BottomScreenSpritePriority : TopScreenSpritePriority;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t spritePriority = targetScreen == NintendoDsScreenTarget::Bottom ? BottomScreenSpritePriority : TopScreenSpritePriority;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bgSetPriority(BottomScreenTextBackgroundId, 0);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies DS paletted sprite tiles are packed in 8x8 OBJ blocks instead of whole-image row-major order.
    /// </summary>
    [Fact]
    public void Source_whenBuildingHardwareSpriteIndexedTileBytes_packsPixelsIn8x8ObjBlocks() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int buildTileBytesStart = sourceCode.IndexOf("std::vector<uint8_t> NintendoDsRenderManager2D::BuildHardwareSpriteIndexedTileBytes", StringComparison.Ordinal);
        int buildTileBytesEnd = sourceCode.IndexOf("void NintendoDsRenderManager2D::EnsureBottomScreenTextBackgroundReady()", StringComparison.Ordinal);
        string buildTileBytesBody = sourceCode[buildTileBytesStart..buildTileBytesEnd];

        Assert.Contains("int32_t blockColumnCount = tileWidth / 8;", buildTileBytesBody, StringComparison.Ordinal);
        Assert.Contains("int32_t blockIndex = (blockRow * blockColumnCount) + blockColumn;", buildTileBytesBody, StringComparison.Ordinal);
        Assert.Contains("int32_t destinationIndex = (blockIndex * 32) + (localY * 4) + (localX / 2);", buildTileBytesBody, StringComparison.Ordinal);
        Assert.DoesNotContain("tilePixels[static_cast<std::size_t>(y * tileWidth + x)] = sourcePixels[static_cast<std::size_t>(sourceY * sourceWidth + sourceX)];", buildTileBytesBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the real DS hardware sprite path uses paletted 16-color OBJ submission for low-color textures and removes the temporary direct proof bar.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingLowColorHardwareSprites_usesPaletted16ColorObjPath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool TryBuildHardwareSpriteIndexed4(", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryResolveHardwareSpritePaletteBank(", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::vector<uint8_t> BuildHardwareSpriteIndexedTileBytes(", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t NextMainSpritePaletteBank;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t NextSubSpritePaletteBank;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t MainHardwareSpritePaletteBank;", File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeTexture2D.hpp")), StringComparison.Ordinal);
        Assert.Contains("int32_t SubHardwareSpritePaletteBank;", File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeTexture2D.hpp")), StringComparison.Ordinal);
        Assert.Contains("TryBuildHardwareSpriteIndexed4(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryResolveHardwareSpritePaletteBank(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BuildHardwareSpriteIndexedTileBytes(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SpriteColorFormat_16Color", sourceCode, StringComparison.Ordinal);
        Assert.Contains("paletteBank", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("DrawBottomScreenProofRectangleDirect();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomScreenProofRectangle", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D renderer restores only the software helpers required for bottom-screen bitmap text and sprite presentation.
    /// </summary>
    [Fact]
    public void Source_whenRestoringBottomScreenSoftwareBitmapPath_definesMinimalSoftwareHelpers() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("void PrewarmTextDrawable(ITextDrawable2D* text);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ClearScreen(ICamera* camera, bool targetBottomScreen);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void BlendPixel(int32_t x, int32_t y, const byte4& color);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void RasterTexturedQuad(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void RasterText(ITextDrawable2D* text);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void RasterSprite(ISpriteDrawable2D* sprite);", headerSource, StringComparison.Ordinal);
        Assert.Contains("RasterText(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RasterTexturedQuad(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BlendPixel(", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS bottom-screen software text path sizes glyph quads from authored atlas metrics instead of the downscaled cooked texture dimensions.
    /// </summary>
    [Fact]
    public void Source_whenRasterizingBottomScreenSoftwareText_usesFontAtlasMetricsForGlyphSize() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("int32_t atlasWidth = font->get_AtlasWidth() > 0 ? font->get_AtlasWidth() : texture->get_Width();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t atlasHeight = font->get_AtlasHeight() > 0 ? font->get_AtlasHeight() : texture->get_Height();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glyph.SourceRect.Z * static_cast<double>(atlasWidth) * fontScale", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glyph.SourceRect.W * static_cast<double>(atlasHeight) * fontScale", sourceCode, StringComparison.Ordinal);
    }
}
