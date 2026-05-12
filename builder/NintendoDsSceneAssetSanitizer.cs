using helengine;
using helengine.files;

namespace helengine.ds.builder;

/// <summary>
/// Rewrites staged Nintendo DS scene assets so unsupported runtime-only gameplay helpers do not block startup scene materialization.
/// </summary>
public sealed class NintendoDsSceneAssetSanitizer {
    /// <summary>
    /// Stores the city demo-disc return-to-menu component type id that Nintendo DS startup scenes must not carry.
    /// </summary>
    const string UnsupportedReturnToMenuComponentTypeId = "city.menu.DemoDiscReturnToMenuComponent, gameplay";

    /// <summary>
    /// Rewrites staged Nintendo DS scene assets inside NitroFS so unsupported runtime-only components are removed before native packaging.
    /// </summary>
    /// <param name="nitroFsRootPath">NitroFS root that contains staged cooked scene assets.</param>
    public void SanitizeStagedSceneAssets(string nitroFsRootPath) {
        if (string.IsNullOrWhiteSpace(nitroFsRootPath)) {
            throw new ArgumentException("NitroFS root path must be provided.", nameof(nitroFsRootPath));
        }

        string sceneRootPath = Path.Combine(nitroFsRootPath, "cooked", "scenes");
        if (!Directory.Exists(sceneRootPath)) {
            return;
        }

        string[] sceneFilePaths = Directory.GetFiles(sceneRootPath, "*.hasset", SearchOption.AllDirectories);
        for (int index = 0; index < sceneFilePaths.Length; index++) {
            SanitizeSceneAssetFile(sceneFilePaths[index]);
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
}
