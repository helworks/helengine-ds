using helengine;
using helengine.files;

namespace helengine.ds.builder;

/// <summary>
/// Rewrites staged Nintendo DS runtime assets so imported textures are validated before native packaging.
/// </summary>
public sealed class NintendoDsSceneAssetSanitizer {
    /// <summary>
    /// Rewrites staged Nintendo DS runtime assets inside NitroFS so imported textures are validated before native packaging.
    /// </summary>
    /// <param name="nitroFsRootPath">NitroFS root that contains staged cooked scene assets.</param>
    public void SanitizeStagedSceneAssets(string nitroFsRootPath) {
        if (string.IsNullOrWhiteSpace(nitroFsRootPath)) {
            throw new ArgumentException("NitroFS root path must be provided.", nameof(nitroFsRootPath));
        }

        string sceneRootPath = Path.Combine(nitroFsRootPath, "cooked", "scenes");
        if (Directory.Exists(sceneRootPath)) {
            string[] sceneFilePaths = Directory.GetFiles(sceneRootPath, "*.hasset", SearchOption.AllDirectories);
            for (int index = 0; index < sceneFilePaths.Length; index++) {
                SanitizeSceneAssetFile(sceneFilePaths[index]);
            }
        }

        string importedTextureRootPath = Path.Combine(nitroFsRootPath, "cooked", "imported");
        if (!Directory.Exists(importedTextureRootPath)) {
            return;
        }

        string[] importedTexturePaths = Directory.GetFiles(importedTextureRootPath, "*", SearchOption.AllDirectories);
        for (int index = 0; index < importedTexturePaths.Length; index++) {
            SanitizeImportedTextureAssetFile(importedTexturePaths[index]);
        }
    }

    /// <summary>
    /// Loads one staged scene asset to verify it remains a valid packaged scene payload for Nintendo DS runtime loading.
    /// </summary>
    /// <param name="sceneFilePath">Staged scene-asset path to inspect.</param>
    static void SanitizeSceneAssetFile(string sceneFilePath) {
        if (string.IsNullOrWhiteSpace(sceneFilePath)) {
            throw new ArgumentException("Scene asset path must be provided.", nameof(sceneFilePath));
        }

        SceneAsset sceneAsset;
        using (FileStream stream = File.OpenRead(sceneFilePath)) {
            sceneAsset = helengine.files.AssetSerializer.Deserialize(stream) as SceneAsset
                ?? throw new InvalidOperationException($"Nintendo DS staged scene asset '{sceneFilePath}' did not deserialize into a SceneAsset.");
        }

        _ = sceneAsset.RootEntities;
    }

    /// <summary>
    /// Validates one staged imported texture asset while preserving the authored cooked payload.
    /// </summary>
    /// <param name="textureAssetPath">Staged imported texture asset path to inspect.</param>
    static void SanitizeImportedTextureAssetFile(string textureAssetPath) {
        if (string.IsNullOrWhiteSpace(textureAssetPath)) {
            throw new ArgumentException("Texture asset path must be provided.", nameof(textureAssetPath));
        }

        TextureAsset textureAsset;
        using (FileStream stream = File.OpenRead(textureAssetPath)) {
            textureAsset = helengine.files.AssetSerializer.Deserialize(stream) as TextureAsset;
        }

        if (textureAsset == null) {
            return;
        }

        ValidateImportedTextureAsset(textureAsset, textureAssetPath);
    }

    /// <summary>
    /// Validates one staged imported texture asset before Nintendo DS packaging preserves the authored payload.
    /// </summary>
    /// <param name="textureAsset">Imported texture asset to validate.</param>
    /// <param name="textureAssetPath">Staged texture-asset path used for diagnostics.</param>
    static void ValidateImportedTextureAsset(TextureAsset textureAsset, string textureAssetPath) {
        if (textureAsset == null) {
            throw new ArgumentNullException(nameof(textureAsset));
        } else if (string.IsNullOrWhiteSpace(textureAssetPath)) {
            throw new ArgumentException("Texture asset path must be provided.", nameof(textureAssetPath));
        } else if (textureAsset.Width < 1 || textureAsset.Height < 1) {
            throw new InvalidOperationException($"Nintendo DS staged texture asset '{textureAssetPath}' must preserve positive authored dimensions.");
        } else if (textureAsset.Colors == null) {
            throw new InvalidOperationException($"Nintendo DS staged texture asset '{textureAssetPath}' must preserve the authored cooked color payload.");
        }

        if ((textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4 || textureAsset.ColorFormat == TextureAssetColorFormat.Indexed8)
            && (textureAsset.PaletteColors == null || textureAsset.PaletteColors.Length == 0)) {
            throw new InvalidOperationException($"Nintendo DS staged indexed texture asset '{textureAssetPath}' must preserve the authored cooked palette payload.");
        }
    }

}
