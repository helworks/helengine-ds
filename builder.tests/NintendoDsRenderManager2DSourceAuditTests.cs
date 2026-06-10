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
        Assert.Contains("videoSetModeSub(MODE_0_2D);", sourceCode, StringComparison.Ordinal);
        int presentStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::PresentBottomScreenFrame()", StringComparison.Ordinal);
        int presentEnd = sourceCode.IndexOf("void NintendoDsRenderManager2D::SetHardware3DScreenTarget", StringComparison.Ordinal);
        string presentBody = sourceCode[presentStart..presentEnd];
        Assert.DoesNotContain("videoSetModeSub(MODE_0_2D);", presentBody, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);", presentBody, StringComparison.Ordinal);
        Assert.DoesNotContain("vramSetBankC(VRAM_C_SUB_BG);", presentBody, StringComparison.Ordinal);
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
    /// Verifies the Nintendo DS bottom-screen BG0 proof consumes one real glyph from the cooked font path instead of synthetic tiles.
    /// </summary>
    [Fact]
    public void Source_whenProvingBottomScreenBg0Text_usesOneCookedFontGlyphTile() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("const std::string& content = text->get_Text();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EnsureBottomScreenFontGlyphTilesReady(font);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (BottomScreenSubmittedTextCountThisFrame > 0)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("char proofCharacter = 'H';", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryResolveBottomScreenGlyphTileIndex(font, proofCharacter, tileIndex)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ClearBottomScreenTextMap();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("constexpr int32_t ProofGlyphRow = 10;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("constexpr int32_t ProofGlyphColumn = 10;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextMapEntries[mapIndex] = tileIndex;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("constexpr int32_t EntryMarkerRow =", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("constexpr int32_t ProofGlyphRowCount =", sourceCode, StringComparison.Ordinal);
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
        Assert.Contains("TryDrawHardwareSprite(sprite)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TryDrawHardwareText(text)", sourceCode, StringComparison.Ordinal);
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
        Assert.DoesNotContain("LogUnsupportedDrawable(\"RoundedRect\", shape);", sourceCode, StringComparison.Ordinal);
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
