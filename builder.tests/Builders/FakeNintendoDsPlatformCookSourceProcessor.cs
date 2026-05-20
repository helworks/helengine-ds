using helengine.editor;

namespace helengine.ds.builder.tests.Builders;

/// <summary>
/// Returns deterministic cooked runtime textures for Nintendo DS builder orchestration tests.
/// </summary>
public sealed class FakeNintendoDsPlatformCookSourceProcessor : INintendoDsPlatformCookSourceProcessor {
    /// <summary>
    /// Stores the template texture returned for source texture work items.
    /// </summary>
    readonly TextureAsset TextureTemplate;

    /// <summary>
    /// Stores the template texture returned for source font-atlas work items.
    /// </summary>
    readonly TextureAsset FontAtlasTextureTemplate;

    /// <summary>
    /// Initializes one fake Nintendo DS source processor with deterministic output templates.
    /// </summary>
    /// <param name="textureTemplate">Template texture returned for source texture work items.</param>
    /// <param name="fontAtlasTextureTemplate">Template texture returned for source font-atlas work items.</param>
    public FakeNintendoDsPlatformCookSourceProcessor(TextureAsset textureTemplate, TextureAsset fontAtlasTextureTemplate) {
        TextureTemplate = textureTemplate ?? throw new ArgumentNullException(nameof(textureTemplate));
        FontAtlasTextureTemplate = fontAtlasTextureTemplate ?? throw new ArgumentNullException(nameof(fontAtlasTextureTemplate));
    }

    /// <summary>
    /// Returns one cloned deterministic texture payload with the supplied runtime identifiers.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute source texture path emitted by the editor build graph.</param>
    /// <param name="assetId">Stable runtime asset identifier the cooked texture should preserve.</param>
    /// <param name="settings">Resolved Nintendo DS texture processor settings supplied by the editor.</param>
    /// <returns>Cloned deterministic cooked texture payload.</returns>
    public TextureAsset CookTexture(string sourceAssetPath, string assetId, TextureAssetProcessorSettings settings) {
        ValidateInputs(sourceAssetPath, assetId, settings);
        return CloneTexture(TextureTemplate, assetId);
    }

    /// <summary>
    /// Returns one cloned deterministic atlas texture payload with the supplied runtime identifiers.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute source font path emitted by the editor build graph.</param>
    /// <param name="assetId">Stable runtime asset identifier the cooked atlas texture should preserve.</param>
    /// <param name="settings">Resolved Nintendo DS texture processor settings supplied by the editor.</param>
    /// <returns>Cloned deterministic cooked atlas texture payload.</returns>
    public TextureAsset CookFontAtlasTexture(string sourceAssetPath, string assetId, TextureAssetProcessorSettings settings) {
        ValidateInputs(sourceAssetPath, assetId, settings);
        return CloneTexture(FontAtlasTextureTemplate, assetId + "#atlas");
    }

    /// <summary>
    /// Validates the source-cook arguments passed by the builder under test.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute source asset path emitted by the editor build graph.</param>
    /// <param name="assetId">Stable runtime asset identifier requested by the builder.</param>
    /// <param name="settings">Resolved Nintendo DS texture processor settings supplied by the editor.</param>
    static void ValidateInputs(string sourceAssetPath, string assetId, TextureAssetProcessorSettings settings) {
        if (string.IsNullOrWhiteSpace(sourceAssetPath)) {
            throw new ArgumentException("Source asset path must be provided.", nameof(sourceAssetPath));
        } else if (string.IsNullOrWhiteSpace(assetId)) {
            throw new ArgumentException("Asset id must be provided.", nameof(assetId));
        } else if (settings == null) {
            throw new ArgumentNullException(nameof(settings));
        }
    }

    /// <summary>
    /// Clones one template texture and stamps the supplied runtime identifiers.
    /// </summary>
    /// <param name="template">Template texture to clone.</param>
    /// <param name="assetId">Stable runtime asset identifier the clone should preserve.</param>
    /// <returns>Cloned texture payload.</returns>
    static TextureAsset CloneTexture(TextureAsset template, string assetId) {
        if (template == null) {
            throw new ArgumentNullException(nameof(template));
        } else if (string.IsNullOrWhiteSpace(assetId)) {
            throw new ArgumentException("Asset id must be provided.", nameof(assetId));
        }

        return new TextureAsset {
            Id = assetId,
            RuntimeAssetId = RuntimeAssetIdGenerator.Generate(assetId),
            Width = template.Width,
            Height = template.Height,
            ColorFormat = template.ColorFormat,
            AlphaPrecision = template.AlphaPrecision,
            Colors = template.Colors == null ? null : [.. template.Colors],
            PaletteColors = template.PaletteColors == null ? null : [.. template.PaletteColors]
        };
    }
}
