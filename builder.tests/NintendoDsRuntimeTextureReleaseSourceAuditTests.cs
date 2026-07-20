namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS runtime texture teardown so scene-owned textures survive scene teardown until a safe renderer frame.
/// </summary>
public class NintendoDsRuntimeTextureReleaseSourceAuditTests {
    /// <summary>
    /// Verifies the DS texture release request is queued and the queued path still deletes uploaded GL textures and resets upload state.
    /// </summary>
    [Fact]
    public void Source_whenReleasingRuntimeTexture_deletesUploadedHardwareTextureAndResetsUploadState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int releaseTextureStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseTexture(RuntimeTexture* texture)", StringComparison.Ordinal);
        int releaseFontStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseFont(FontAsset* font)", StringComparison.Ordinal);
        string releaseTextureBody = sourceCode[releaseTextureStart..releaseFontStart];

        Assert.Contains("PendingReleasedTextures.push_back(texture);", releaseTextureBody, StringComparison.Ordinal);
        Assert.DoesNotContain("delete dsTexture;", releaseTextureBody, StringComparison.Ordinal);

        int immediateReleaseStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseTextureImmediately(RuntimeTexture* texture)", StringComparison.Ordinal);
        int releaseFontImmediateStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseFontImmediately(FontAsset* font)", immediateReleaseStart, StringComparison.Ordinal);
        string immediateReleaseBody = sourceCode[immediateReleaseStart..releaseFontImmediateStart];
        Assert.Contains("glDeleteTextures(1, &dsTexture->HardwareTextureId);", immediateReleaseBody, StringComparison.Ordinal);
        Assert.Contains("dsTexture->HardwareTextureId = -1;", immediateReleaseBody, StringComparison.Ordinal);
        Assert.Contains("dsTexture->HardwareTextureUploaded = false;", immediateReleaseBody, StringComparison.Ordinal);
        Assert.Contains("dsTexture->Dispose();", immediateReleaseBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS font release path queues the font and releases its attached runtime atlas only at the safe frame boundary.
    /// </summary>
    [Fact]
    public void Source_whenReleasingFont_releasesAttachedRuntimeAtlasBeforeDeletingFont() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int releaseFontStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseFont(FontAsset* font)", StringComparison.Ordinal);
        int flushDeferredReleasesStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::FlushDeferredReleasesForFrame()", releaseFontStart, StringComparison.Ordinal);
        string releaseFontBody = sourceCode[releaseFontStart..flushDeferredReleasesStart];

        Assert.Contains("PendingReleasedFonts.push_back(font);", releaseFontBody, StringComparison.Ordinal);
        Assert.DoesNotContain("delete font;", releaseFontBody, StringComparison.Ordinal);

        int immediateReleaseStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseFontImmediately(FontAsset* font)", releaseFontStart, StringComparison.Ordinal);
        int flushReleasedTexturesStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::FlushReleasedTextures()", immediateReleaseStart, StringComparison.Ordinal);
        string immediateReleaseBody = sourceCode[immediateReleaseStart..flushReleasedTexturesStart];
        Assert.Contains("RuntimeTexture* texture = font->get_Texture();", immediateReleaseBody, StringComparison.Ordinal);
        Assert.Contains("ReleaseTexture(texture);", immediateReleaseBody, StringComparison.Ordinal);
        Assert.Contains("font->Dispose();", immediateReleaseBody, StringComparison.Ordinal);
        Assert.Contains("delete font;", immediateReleaseBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS font release path invalidates cached glyph-font pointers before deleting the font asset.
    /// </summary>
    [Fact]
    public void Source_whenReleasingFont_invalidatesGlyphCachesBeforeDeletingFont() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int releaseFontStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseFont(FontAsset* font)", StringComparison.Ordinal);
        int flushReleasedTexturesStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::FlushReleasedTextures()", releaseFontStart, StringComparison.Ordinal);
        string releaseFontBody = sourceCode[releaseFontStart..flushReleasedTexturesStart];

        Assert.Contains("BottomScreenTextGlyphCacheFonts[layerIndex] == font", releaseFontBody, StringComparison.Ordinal);
        Assert.Contains("TopScreenTextGlyphCacheFonts[layerIndex] == font", releaseFontBody, StringComparison.Ordinal);
        Assert.Contains("BottomScreenTextGlyphTilesUploaded[layerIndex] = false;", releaseFontBody, StringComparison.Ordinal);
        Assert.Contains("TopScreenTextGlyphTilesUploaded[layerIndex] = false;", releaseFontBody, StringComparison.Ordinal);
    }
}
