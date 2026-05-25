using System.Text.Json.Nodes;

namespace helengine.ds.builder;

/// <summary>
/// Creates the metadata-only external platform plugin manifest exposed by the Nintendo DS repository.
/// </summary>
public static class NintendoDsPlatformPluginManifest {
    /// <summary>
    /// Creates the metadata-only Nintendo DS platform plugin manifest payload.
    /// </summary>
    /// <returns>Manifest object that contains only generic editor-consumable platform metadata.</returns>
    public static JsonObject Create() {
        return new JsonObject {
            ["platformId"] = "ds",
            ["displayName"] = "Nintendo DS",
            ["builderAssemblyPath"] = "builder/helengine.ds.builder.dll",
            ["definitionFactoryType"] = "helengine.ds.builder.NintendoDsPlatformDefinitionFactory"
        };
    }
}
