namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS runtime texture teardown so scene-owned textures release their hardware 3D allocations before the runtime texture object is destroyed.
/// </summary>
public class NintendoDsRuntimeTextureReleaseSourceAuditTests {
    /// <summary>
    /// Verifies the DS texture release path deletes uploaded GL textures and resets the runtime upload state instead of leaking 3D texture VRAM across scene transitions.
    /// </summary>
    [Fact]
    public void Source_whenReleasingRuntimeTexture_deletesUploadedHardwareTextureAndResetsUploadState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int releaseTextureStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseTexture(RuntimeTexture* texture)", StringComparison.Ordinal);
        int releaseFontStart = sourceCode.IndexOf("void NintendoDsRenderManager2D::ReleaseFont(FontAsset* font)", StringComparison.Ordinal);
        string releaseTextureBody = sourceCode[releaseTextureStart..releaseFontStart];

        Assert.Contains("glDeleteTextures(1, &dsTexture->HardwareTextureId);", releaseTextureBody, StringComparison.Ordinal);
        Assert.Contains("dsTexture->HardwareTextureId = -1;", releaseTextureBody, StringComparison.Ordinal);
        Assert.Contains("dsTexture->HardwareTextureUploaded = false;", releaseTextureBody, StringComparison.Ordinal);
    }
}
