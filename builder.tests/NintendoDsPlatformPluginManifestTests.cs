using System.Text.Json.Nodes;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS external platform plugin manifest remains metadata-only and matches the checked-in file.
/// </summary>
public sealed class NintendoDsPlatformPluginManifestTests {
    /// <summary>
    /// Ensures the in-memory plugin manifest payload does not declare runtime payload extension metadata.
    /// </summary>
    [Fact]
    public void Create_whenSerialized_doesNotContainRuntimePayloadTypeDeclarations() {
        JsonObject manifest = NintendoDsPlatformPluginManifest.Create();

        Assert.Null(manifest["runtimePayloadTypes"]);
        Assert.Null(manifest["serializerHooks"]);
        Assert.Equal("ds", manifest["platformId"]?.GetValue<string>());
        Assert.Equal("Nintendo DS", manifest["displayName"]?.GetValue<string>());
        Assert.Equal("builder/helengine.ds.builder.dll", manifest["builderAssemblyPath"]?.GetValue<string>());
    }

    /// <summary>
    /// Ensures the checked-in plugin manifest file matches the generated metadata-only payload.
    /// </summary>
    [Fact]
    public void CheckedInPluginManifest_matchesGeneratedMetadataOnlyPayload() {
        string manifestFilePath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "platform-plugin.json"));
        string expectedJson = NintendoDsPlatformPluginManifest.Create().ToJsonString();
        string actualJson = JsonNode.Parse(File.ReadAllText(manifestFilePath))?.ToJsonString();

        Assert.Equal(expectedJson, actualJson);
    }
}
