using helengine;
using helengine.files;

namespace helengine.ds.builder;

/// <summary>
/// Rewrites staged Nintendo DS runtime assets so unsupported audio content is removed and imported textures are validated before native packaging.
/// </summary>
public sealed class NintendoDsSceneAssetSanitizer {
    /// <summary>
    /// Rewrites staged Nintendo DS runtime assets inside NitroFS so audio content is removed and imported textures are validated before native packaging.
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

        RemoveAudioContent(sceneAsset);
        File.WriteAllBytes(sceneFilePath, helengine.files.AssetSerializer.SerializeToBytes(sceneAsset));
    }

    /// <summary>
    /// Removes audio source components and their cooked audio references from one staged scene.
    /// </summary>
    /// <param name="sceneAsset">Staged scene asset to rewrite for Nintendo DS packaging.</param>
    static void RemoveAudioContent(SceneAsset sceneAsset) {
        if (sceneAsset == null) {
            throw new ArgumentNullException(nameof(sceneAsset));
        } else if (sceneAsset.AssetReferences == null) {
            throw new InvalidOperationException("Nintendo DS staged scenes must provide their asset references.");
        } else if (sceneAsset.RootEntities == null) {
            throw new InvalidOperationException("Nintendo DS staged scenes must provide their root entities.");
        }

        sceneAsset.AssetReferences = sceneAsset.AssetReferences
            .Where(assetReference => !IsCookedAudioAssetReference(assetReference))
            .ToArray();

        for (int index = 0; index < sceneAsset.RootEntities.Length; index++) {
            RemoveAudioContent(sceneAsset.RootEntities[index]);
        }
    }

    /// <summary>
    /// Removes audio source component records from one staged entity and every descendant entity.
    /// </summary>
    /// <param name="entityAsset">Staged entity asset to rewrite for Nintendo DS packaging.</param>
    static void RemoveAudioContent(SceneEntityAsset entityAsset) {
        if (entityAsset == null) {
            throw new ArgumentNullException(nameof(entityAsset));
        } else if (entityAsset.Components == null) {
            throw new InvalidOperationException("Nintendo DS staged entities must provide their component records.");
        } else if (entityAsset.Children == null) {
            throw new InvalidOperationException("Nintendo DS staged entities must provide their child entities.");
        }

        entityAsset.Components = entityAsset.Components
            .Where(component => !IsAudioSourceComponent(component))
            .ToArray();
        RemoveAudioPlatformOverrides(entityAsset);

        for (int index = 0; index < entityAsset.Children.Length; index++) {
            RemoveAudioContent(entityAsset.Children[index]);
        }
    }

    /// <summary>
    /// Removes platform-only audio source component additions from one staged entity.
    /// </summary>
    /// <param name="entityAsset">Staged entity whose platform component overrides should be rewritten.</param>
    static void RemoveAudioPlatformOverrides(SceneEntityAsset entityAsset) {
        if (entityAsset == null) {
            throw new ArgumentNullException(nameof(entityAsset));
        } else if (entityAsset.PlatformComponentOverrides == null) {
            throw new InvalidOperationException("Nintendo DS staged entities must provide their platform component overrides.");
        }

        for (int index = 0; index < entityAsset.PlatformComponentOverrides.Length; index++) {
            SceneEntityPlatformComponentOverrideAsset componentOverride = entityAsset.PlatformComponentOverrides[index]
                ?? throw new InvalidOperationException("Nintendo DS staged platform component overrides cannot contain null entries.");
            if (componentOverride.AddedComponents == null) {
                throw new InvalidOperationException("Nintendo DS staged platform component overrides must provide their added components.");
            }

            componentOverride.AddedComponents = componentOverride.AddedComponents
                .Where(addedComponent => !IsAudioSourceComponent(addedComponent))
                .ToArray();
        }
    }

    /// <summary>
    /// Determines whether one platform-only component addition is an authored audio source.
    /// </summary>
    /// <param name="addedComponent">Platform-only component addition to inspect.</param>
    /// <returns><c>true</c> when the added component is an audio source; otherwise <c>false</c>.</returns>
    static bool IsAudioSourceComponent(SceneEntityPlatformAddedComponentAsset addedComponent) {
        if (addedComponent == null) {
            throw new InvalidOperationException("Nintendo DS staged platform component additions cannot contain null entries.");
        }

        return IsAudioSourceComponent(addedComponent.Component);
    }

    /// <summary>
    /// Determines whether one staged scene asset reference resolves to a cooked audio payload.
    /// </summary>
    /// <param name="assetReference">Staged asset reference to inspect.</param>
    /// <returns><c>true</c> when the reference identifies a cooked audio payload; otherwise <c>false</c>.</returns>
    static bool IsCookedAudioAssetReference(SceneAssetReference assetReference) {
        if (assetReference == null) {
            throw new InvalidOperationException("Nintendo DS staged scene asset references cannot contain null entries.");
        }

        return assetReference.RelativePath.StartsWith("audio/", StringComparison.OrdinalIgnoreCase)
            || assetReference.RelativePath.StartsWith("cooked/audio/", StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// Determines whether one staged component record is an authored audio source.
    /// </summary>
    /// <param name="component">Staged component record to inspect.</param>
    /// <returns><c>true</c> when the component is an audio source; otherwise <c>false</c>.</returns>
    static bool IsAudioSourceComponent(SceneComponentAssetRecord component) {
        if (component == null) {
            throw new InvalidOperationException("Nintendo DS staged component records cannot contain null entries.");
        }

        return string.Equals(component.ComponentTypeId, "helengine.AudioSourceComponent", StringComparison.Ordinal);
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
