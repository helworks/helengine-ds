namespace helengine.ds.builder;

/// <summary>
/// Stores the Nintendo DS-owned startup-scene identifiers used by the generated boot-scene flow.
/// </summary>
public static class NintendoDsStartupSceneIds {
    /// <summary>
    /// Scene id of the generated boot scene that installs scene mappings before startup redirects run.
    /// </summary>
    public const string GeneratedBootSceneId = "GeneratedBootScene";

    /// <summary>
    /// Cooked runtime-relative payload path of the generated boot scene.
    /// </summary>
    public const string GeneratedBootSceneCookedRelativePath = "cooked/scenes/GeneratedBootScene.hasset";
}
