using System.Runtime.Versioning;

namespace helengine.ds.builder;

/// <summary>
/// Imports source textures and source fonts through Nintendo DS-local decode and atlas-generation services for builder-owned cook work items.
/// </summary>
[SupportedOSPlatform("windows")]
public sealed class NintendoDsPlatformCookSourceProcessor : INintendoDsPlatformCookSourceProcessor {
    /// <summary>
    /// Stable metadata suffix appended to cooked font asset ids when generating their atlas texture ids.
    /// </summary>
    const string FontAtlasSuffix = "#atlas";

    /// <summary>
    /// Shared texture processor used to apply Nintendo DS texture settings after source decode.
    /// </summary>
    readonly TextureAssetProcessor TextureAssetProcessor;

    /// <summary>
    /// Stores the source image decoder used for builder-owned texture work items.
    /// </summary>
    readonly NintendoDsSourceTextureDecoder SourceTextureDecoder;

    /// <summary>
    /// Stores the raw source font atlas importer used for builder-owned font-atlas work items.
    /// </summary>
    readonly NintendoDsSourceFontAtlasTextureImporter SourceFontAtlasTextureImporter;

    /// <summary>
    /// Initializes the shared source processor used by Nintendo DS builder-owned work items.
    /// </summary>
    public NintendoDsPlatformCookSourceProcessor() {
        TextureAssetProcessor = new TextureAssetProcessor();
        SourceTextureDecoder = new NintendoDsSourceTextureDecoder();
        SourceFontAtlasTextureImporter = new NintendoDsSourceFontAtlasTextureImporter();
    }

    /// <summary>
    /// Imports one source texture asset and applies the resolved Nintendo DS texture processor settings.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute source texture path emitted by the editor build graph.</param>
    /// <param name="assetId">Stable runtime asset identifier the cooked texture should preserve.</param>
    /// <param name="settings">Resolved Nintendo DS texture processor settings supplied by the build graph.</param>
    /// <returns>Processed runtime texture payload ready for serialization.</returns>
    public TextureAsset CookTexture(string sourceAssetPath, string assetId, TextureAssetProcessorSettings settings) {
        if (string.IsNullOrWhiteSpace(sourceAssetPath)) {
            throw new ArgumentException("Source texture path must be provided.", nameof(sourceAssetPath));
        } else if (string.IsNullOrWhiteSpace(assetId)) {
            throw new ArgumentException("Texture asset id must be provided.", nameof(assetId));
        } else if (settings == null) {
            throw new ArgumentNullException(nameof(settings));
        } else if (!File.Exists(sourceAssetPath)) {
            throw new FileNotFoundException("Source texture file was not found.", sourceAssetPath);
        }

        TextureAsset importedTextureAsset = SourceTextureDecoder.Decode(sourceAssetPath);
        TextureAsset cookedTextureAsset = TextureAssetProcessor.Apply(importedTextureAsset, settings);
        cookedTextureAsset.Id = assetId;
        cookedTextureAsset.RuntimeAssetId = RuntimeAssetIdGenerator.Generate(assetId);
        return cookedTextureAsset;
    }

    /// <summary>
    /// Imports one source font asset and applies the resolved Nintendo DS atlas texture processor settings.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute source font path emitted by the editor build graph.</param>
    /// <param name="assetId">Stable runtime asset identifier the cooked atlas texture should preserve.</param>
    /// <param name="settings">Resolved Nintendo DS texture processor settings supplied by the build graph.</param>
    /// <returns>Processed runtime atlas texture payload ready for serialization.</returns>
    public TextureAsset CookFontAtlasTexture(string sourceAssetPath, string assetId, TextureAssetProcessorSettings settings) {
        if (string.IsNullOrWhiteSpace(sourceAssetPath)) {
            throw new ArgumentException("Source font path must be provided.", nameof(sourceAssetPath));
        } else if (string.IsNullOrWhiteSpace(assetId)) {
            throw new ArgumentException("Font atlas asset id must be provided.", nameof(assetId));
        } else if (settings == null) {
            throw new ArgumentNullException(nameof(settings));
        } else if (!File.Exists(sourceAssetPath)) {
            throw new FileNotFoundException("Source font file was not found.", sourceAssetPath);
        }

        TextureAsset importedAtlasTextureAsset;
        try {
            importedAtlasTextureAsset = LoadSourceFontAtlasTexture(sourceAssetPath);
        } catch (Exception exception) {
            throw new InvalidOperationException($"Nintendo DS font-atlas cooking failed for source '{sourceAssetPath}'.", exception);
        }
        TextureAsset cookedAtlasAsset = TextureAssetProcessor.Apply(importedAtlasTextureAsset, settings);
        cookedAtlasAsset.Id = assetId + FontAtlasSuffix;
        cookedAtlasAsset.RuntimeAssetId = RuntimeAssetIdGenerator.Generate(cookedAtlasAsset.Id);
        return cookedAtlasAsset;
    }

    /// <summary>
        /// Loads one source font asset from either a packaged <c>.hefont</c> document, one serialized texture-asset payload, or one raw source font file.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute source font path emitted by the editor build graph.</param>
    /// <returns>Imported or deserialized raw atlas texture asset.</returns>
    TextureAsset LoadSourceFontAtlasTexture(string sourceAssetPath) {
        if (string.IsNullOrWhiteSpace(sourceAssetPath)) {
            throw new ArgumentException("Source font path must be provided.", nameof(sourceAssetPath));
        }

        if (string.Equals(Path.GetExtension(sourceAssetPath), ".hefont", StringComparison.OrdinalIgnoreCase)) {
            using FileStream stream = new FileStream(sourceAssetPath, FileMode.Open, FileAccess.Read, FileShare.Read);
            FontAsset fontAsset = global::helengine.files.FontAssetBinarySerializer.Deserialize(stream);
            if (fontAsset?.SourceTextureAsset == null) {
                throw new InvalidOperationException($"Packaged font source '{sourceAssetPath}' did not provide an atlas texture payload.");
            }

            return fontAsset.SourceTextureAsset;
        }

            if (string.Equals(Path.GetExtension(sourceAssetPath), ".hasset", StringComparison.OrdinalIgnoreCase) ||
                string.Equals(Path.GetExtension(sourceAssetPath), ".hetex", StringComparison.OrdinalIgnoreCase)) {
                return SourceTextureDecoder.Decode(sourceAssetPath);
            }

        return SourceFontAtlasTextureImporter.Import(sourceAssetPath);
    }
}
