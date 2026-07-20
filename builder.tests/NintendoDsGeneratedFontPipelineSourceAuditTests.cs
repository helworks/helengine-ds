namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the editor-side generated-font pipeline required to carry the Nintendo DS debug font from scene save through packaging.
/// </summary>
public class NintendoDsGeneratedFontPipelineSourceAuditTests {
    /// <summary>
    /// Verifies the editor app exposes the dedicated Nintendo DS generated debug font factory.
    /// </summary>
    [Fact]
    public void Sources_whenNintendoDsGeneratedDebugFontIsSupported_exposeDedicatedConsolasFactory() {
        string source = File.ReadAllText(@"C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\NintendoDsDebugFontFactory.cs");
        string gdiFontProcessorSource = File.ReadAllText(@"C:\dev\helworks\helengine\engine\helengine.editor.windows\content\font\GDIFontProcessor.cs");

        Assert.Contains("const float BottomOverlayFontPixelSize = 8f;", source, StringComparison.Ordinal);
        Assert.Contains("new Font(\"Consolas\", BottomOverlayFontPixelSize, FontStyle.Regular, GraphicsUnit.Pixel)", source, StringComparison.Ordinal);
        Assert.Contains("CreateBottomOverlayFont()", source, StringComparison.Ordinal);
        Assert.Contains("' '", gdiFontProcessorSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies scene font resolution and runtime packaging recognize the dedicated Nintendo DS generated font asset id.
    /// </summary>
    [Fact]
    public void Sources_whenNintendoDsGeneratedDebugFontIsSupported_wireResolverAndPackagersToDsGeneratedFontAssetId() {
        string resolverSource = File.ReadAllText(@"C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\EditorSceneAssetReferenceResolver.cs");
        string generatedFontResolverSource = File.ReadAllText(@"C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\GeneratedEditorFontAssetResolver.cs");
        string packagingTransformSource = File.ReadAllText(@"C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs");
        string buildPackagerSource = File.ReadAllText(@"C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs");

        Assert.Contains("GeneratedEditorFontAssetResolver.ResolveRequiredFontAsset", resolverSource, StringComparison.Ordinal);
        Assert.Contains("const string NintendoDsDebugFontFactoryTypeName = \"helengine.editor.app.NintendoDsDebugFontFactory\";", generatedFontResolverSource, StringComparison.Ordinal);
        Assert.Contains("const string CreateBottomOverlayFontMethodName = \"CreateBottomOverlayFont\";", generatedFontResolverSource, StringComparison.Ordinal);
        Assert.Contains("Assembly.Load(EditorAppAssemblyName)", generatedFontResolverSource, StringComparison.Ordinal);
        Assert.Contains("const string NintendoDsDebugFontAssetId = \"ds-debug-font\";", packagingTransformSource, StringComparison.Ordinal);
        Assert.Contains("const string NintendoDsDebugFontRelativePath = \"cooked/fonts/ds-debug.hefont\";", packagingTransformSource, StringComparison.Ordinal);
        Assert.Contains("CreateFontFileReference(NintendoDsDebugFontRelativePath)", packagingTransformSource, StringComparison.Ordinal);
        Assert.Contains("const string NintendoDsDebugFontAssetId = \"ds-debug-font\";", buildPackagerSource, StringComparison.Ordinal);
        Assert.Contains("const string NintendoDsDebugFontRelativePath = \"cooked/fonts/ds-debug.hefont\";", buildPackagerSource, StringComparison.Ordinal);
        Assert.Contains("RewriteGeneratedNintendoDsDebugFontReference", buildPackagerSource, StringComparison.Ordinal);
    }
}
