using helengine;
using helengine.files;

namespace helengine.ds.scratch;

/// <summary>
/// Repairs stale city menu component type ids inside serialized scene assets by rewriting old assembly-qualified ids to the current gameplay ids.
/// </summary>
public static class Program {
    /// <summary>
    /// Stable city desktop main-menu scene path used by the local DS workflow.
    /// </summary>
    static readonly string DesktopMainMenuScenePath = @"C:\dev\helprojs\city\assets\scenes\DemoDiscMainMenu.helen";

    /// <summary>
    /// Stable city Nintendo DS main-menu scene path used by the local DS workflow.
    /// </summary>
    static readonly string NintendoDsMainMenuScenePath = @"C:\dev\helprojs\city\assets\scenes\DemoDiscMainMenuDs.helen";

    /// <summary>
    /// Rewrites the known city menu scene assets and prints one update summary for each file.
    /// </summary>
    /// <param name="args">Optional explicit scene paths. When omitted, the city desktop and DS main-menu scenes are repaired.</param>
    /// <returns>Zero when the repair pass completes.</returns>
    public static int Main(string[] args) {
        IReadOnlyList<string> scenePaths = ResolveScenePaths(args);
        for (int index = 0; index < scenePaths.Count; index++) {
            string scenePath = scenePaths[index];
            bool changed = RepairScene(scenePath);
            Console.WriteLine((changed ? "UPDATED=" : "UNCHANGED=") + scenePath);
        }

        return 0;
    }

    /// <summary>
    /// Resolves the scene paths that should be repaired for the current run.
    /// </summary>
    /// <param name="args">Optional explicit scene paths.</param>
    /// <returns>Ordered scene paths to inspect.</returns>
    static IReadOnlyList<string> ResolveScenePaths(string[] args) {
        if (args != null && args.Length > 0) {
            return args;
        }

        return [
            DesktopMainMenuScenePath,
            NintendoDsMainMenuScenePath
        ];
    }

    /// <summary>
    /// Repairs one serialized scene asset in place when it still carries stale city menu component type ids.
    /// </summary>
    /// <param name="scenePath">Absolute scene path to repair.</param>
    /// <returns><c>true</c> when the scene changed; otherwise <c>false</c>.</returns>
    static bool RepairScene(string scenePath) {
        if (string.IsNullOrWhiteSpace(scenePath)) {
            throw new ArgumentException("Scene path must be provided.", nameof(scenePath));
        } else if (!File.Exists(scenePath)) {
            throw new FileNotFoundException("Scene asset was not found.", scenePath);
        }

        SceneAsset sceneAsset = helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(scenePath)) as SceneAsset
            ?? throw new InvalidOperationException($"Scene asset '{scenePath}' did not deserialize into a SceneAsset.");
        bool changed = RepairEntities(sceneAsset.RootEntities);
        if (!changed) {
            return false;
        }

        File.WriteAllBytes(scenePath, helengine.files.AssetSerializer.SerializeToBytes(sceneAsset));
        return true;
    }

    /// <summary>
    /// Repairs stale city menu component type ids throughout one serialized entity subtree.
    /// </summary>
    /// <param name="entities">Serialized entities to inspect.</param>
    /// <returns><c>true</c> when any component type id changed; otherwise <c>false</c>.</returns>
    static bool RepairEntities(SceneEntityAsset[] entities) {
        bool changed = false;
        if (entities == null) {
            return false;
        }

        for (int entityIndex = 0; entityIndex < entities.Length; entityIndex++) {
            SceneEntityAsset entity = entities[entityIndex];
            if (entity == null) {
                continue;
            }

            if (RepairComponents(entity.Components)) {
                changed = true;
            }

            if (RepairEntities(entity.Children)) {
                changed = true;
            }
        }

        return changed;
    }

    /// <summary>
    /// Repairs stale city menu component type ids within one serialized component array.
    /// </summary>
    /// <param name="components">Serialized component records to inspect.</param>
    /// <returns><c>true</c> when any component type id changed; otherwise <c>false</c>.</returns>
    static bool RepairComponents(SceneComponentAssetRecord[] components) {
        bool changed = false;
        if (components == null) {
            return false;
        }

        for (int componentIndex = 0; componentIndex < components.Length; componentIndex++) {
            SceneComponentAssetRecord component = components[componentIndex];
            if (component == null) {
                continue;
            }

            string rewrittenTypeId = RewriteComponentTypeId(component.ComponentTypeId);
            if (string.Equals(rewrittenTypeId, component.ComponentTypeId, StringComparison.Ordinal)) {
                continue;
            }

            component.ComponentTypeId = rewrittenTypeId;
            changed = true;
        }

        return changed;
    }

    /// <summary>
    /// Rewrites one stale city menu component type id to the canonical gameplay-backed id used by current runtime codegen.
    /// </summary>
    /// <param name="componentTypeId">Persisted component type id to rewrite.</param>
    /// <returns>Canonical component type id.</returns>
    static string RewriteComponentTypeId(string componentTypeId) {
        if (string.Equals(componentTypeId, "city.menu.MenuComponent, PhysicsSceneGeneratorHarness", StringComparison.Ordinal)) {
            return "city.menu.MenuComponent, gameplay";
        } else if (string.Equals(componentTypeId, "city.menu.MenuPanelComponent, PhysicsSceneGeneratorHarness", StringComparison.Ordinal)) {
            return "city.menu.MenuPanelComponent, gameplay";
        } else if (string.Equals(componentTypeId, "city.menu.MenuItemComponent, PhysicsSceneGeneratorHarness", StringComparison.Ordinal)) {
            return "city.menu.MenuItemComponent, gameplay";
        } else if (string.Equals(componentTypeId, "city.menu.PlatformInfoTextComponent, PhysicsSceneGeneratorHarness", StringComparison.Ordinal)) {
            return "city.menu.PlatformInfoTextComponent, gameplay";
        }

        return componentTypeId;
    }
}
