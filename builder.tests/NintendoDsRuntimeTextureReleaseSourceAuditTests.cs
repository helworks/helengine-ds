namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS runtime texture teardown so scene-owned textures survive scene teardown until a safe renderer frame.
/// </summary>
public class NintendoDsRuntimeTextureReleaseSourceAuditTests {
    /// <summary>
    /// Verifies raw DS texture construction copies transient asset arrays and the runtime texture owns their eventual cleanup.
    /// </summary>
    [Fact]
    public void Source_whenBuildingRawTexture_copiesPayloadAndRuntimeTextureOwnsCleanup() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rendererSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string runtimeTextureHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeTexture2D.hpp");
        string runtimeTextureSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeTexture2D.cpp");
        string rendererSource = File.ReadAllText(rendererSourcePath);
        string runtimeTextureHeader = File.ReadAllText(runtimeTextureHeaderPath);
        string runtimeTextureSource = File.ReadAllText(runtimeTextureSourcePath);

        int rawBuildStart = rendererSource.IndexOf("RuntimeTexture* NintendoDsRenderManager2D::BuildTextureFromRaw(TextureAsset* data)", StringComparison.Ordinal);
        int updateRegionStart = rendererSource.IndexOf("void NintendoDsRenderManager2D::UpdateTextureRegionCore(", rawBuildStart, StringComparison.Ordinal);
        string rawBuildBody = rendererSource[rawBuildStart..updateRegionStart];

        Assert.DoesNotContain("runtimeTexture->Colors = data->Colors;", rawBuildBody, StringComparison.Ordinal);
        Assert.DoesNotContain("runtimeTexture->PaletteColors = data->PaletteColors;", rawBuildBody, StringComparison.Ordinal);
        Assert.Contains("new Array<uint8_t>(data->Colors->Length)", rawBuildBody, StringComparison.Ordinal);
        Assert.Contains("Array<uint8_t>::Copy(data->Colors", rawBuildBody, StringComparison.Ordinal);
        Assert.Contains("new Array<uint8_t>(data->PaletteColors->Length)", rawBuildBody, StringComparison.Ordinal);
        Assert.Contains("Array<uint8_t>::Copy(data->PaletteColors", rawBuildBody, StringComparison.Ordinal);

        Assert.Contains("~NintendoDsRuntimeTexture2D() override;", runtimeTextureHeader, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRuntimeTexture2D::~NintendoDsRuntimeTexture2D()", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("colors != nullptr && colors != Array<uint8_t>::Empty()", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("paletteColors != nullptr && paletteColors != Array<uint8_t>::Empty()", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("delete colors;", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("delete paletteColors;", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("paletteColors != colors", runtimeTextureSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies cooked texture loading releases deserialized source arrays after copying and also cleans them on failure without deleting the Empty singleton.
    /// </summary>
    [Fact]
    public void Source_whenLoadingCookedTexture_releasesDeserializedArraysExactlyOnce() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void ReleaseOwnedTextureArrays(TextureAsset* textureAsset)", sourceCode, StringComparison.Ordinal);
        Assert.Equal(2, sourceCode.Split("ReleaseOwnedTextureArrays(textureAsset);", StringSplitOptions.None).Length - 1);
        Assert.Contains("if (colors != nullptr && colors != Array<uint8_t>::Empty())", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (paletteColors != nullptr && paletteColors != Array<uint8_t>::Empty() && paletteColors != colors)", sourceCode, StringComparison.Ordinal);
    }

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
