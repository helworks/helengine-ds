using helengine;
using helengine.files;

namespace helengine.ds.builder;

/// <summary>
/// Rewrites staged Nintendo DS runtime assets so unsupported scene components are removed and imported textures are validated before native packaging.
/// </summary>
public sealed class NintendoDsSceneAssetSanitizer {
    /// <summary>
    /// Stores the city demo-disc return-to-menu component type id that Nintendo DS startup scenes must not carry.
    /// </summary>
    const string UnsupportedReturnToMenuComponentTypeId = "city.menu.DemoDiscReturnToMenuComponent, gameplay";

    /// <summary>
    /// Rewrites staged Nintendo DS runtime assets inside NitroFS so unsupported runtime-only components are removed and imported textures are validated before native packaging.
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
    /// Rewrites one staged scene-asset file when it contains unsupported Nintendo DS components.
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

        if (!RemoveUnsupportedComponents(sceneAsset.RootEntities)) {
            return;
        }

        File.WriteAllBytes(sceneFilePath, helengine.files.AssetSerializer.SerializeToBytes(sceneAsset));
    }

    /// <summary>
    /// Removes unsupported Nintendo DS components from the supplied entity tree.
    /// </summary>
    /// <param name="entities">Scene entities to inspect and rewrite.</param>
    /// <returns>True when at least one unsupported component was removed.</returns>
    static bool RemoveUnsupportedComponents(SceneEntityAsset[] entities) {
        if (entities == null || entities.Length == 0) {
            return false;
        }

        bool removedAnyComponent = false;
        for (int index = 0; index < entities.Length; index++) {
            SceneEntityAsset entity = entities[index];
            if (entity == null) {
                continue;
            }

            if (RemoveUnsupportedComponents(entity)) {
                removedAnyComponent = true;
            }
        }

        return removedAnyComponent;
    }

    /// <summary>
    /// Removes unsupported Nintendo DS components from one entity and its child tree.
    /// </summary>
    /// <param name="entity">Scene entity to inspect and rewrite.</param>
    /// <returns>True when at least one unsupported component was removed.</returns>
    static bool RemoveUnsupportedComponents(SceneEntityAsset entity) {
        if (entity == null) {
            throw new ArgumentNullException(nameof(entity));
        }

        bool removedFromEntity = RemoveUnsupportedComponentsFromEntity(entity);
        bool removedFromChildren = RemoveUnsupportedComponents(entity.Children);
        return removedFromEntity || removedFromChildren;
    }

    /// <summary>
    /// Removes unsupported Nintendo DS components from one entity record.
    /// </summary>
    /// <param name="entity">Scene entity whose component list should be filtered.</param>
    /// <returns>True when the entity component list changed.</returns>
    static bool RemoveUnsupportedComponentsFromEntity(SceneEntityAsset entity) {
        if (entity == null) {
            throw new ArgumentNullException(nameof(entity));
        }

        SceneComponentAssetRecord[] components = entity.Components ?? Array.Empty<SceneComponentAssetRecord>();
        List<SceneComponentAssetRecord> filteredComponents = new List<SceneComponentAssetRecord>(components.Length);
        bool removedAnyComponent = false;
        for (int index = 0; index < components.Length; index++) {
            SceneComponentAssetRecord component = components[index];
            if (component != null && string.Equals(component.ComponentTypeId, UnsupportedReturnToMenuComponentTypeId, StringComparison.Ordinal)) {
                removedAnyComponent = true;
                continue;
            }

            filteredComponents.Add(component);
        }

        if (!removedAnyComponent) {
            return false;
        }

        entity.Components = filteredComponents.ToArray();
        return true;
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
