using System.Text.Json;

namespace helengine.ds.builder;

/// <summary>
/// Serializes and deserializes the generic Nintendo DS texture cook settings contract emitted by the editor build graph.
/// </summary>
public static class NintendoDsTextureCookSettingsSerializer {
    /// <summary>
    /// Serializes one Nintendo DS texture settings payload to the generic editor JSON contract.
    /// </summary>
    /// <param name="maxResolution">Maximum authored texture resolution preserved by the platform override.</param>
    /// <param name="colorFormat">Cooked texture color format selected for Nintendo DS.</param>
    /// <param name="alphaPrecision">Cooked texture alpha precision selected for Nintendo DS.</param>
    /// <returns>Serialized JSON payload consumed by builder-owned cook work items.</returns>
    public static string Serialize(int maxResolution, TextureAssetColorFormat colorFormat, TextureAssetAlphaPrecision alphaPrecision) {
        return JsonSerializer.Serialize(new Dictionary<string, object> {
            ["maxResolution"] = maxResolution,
            ["colorFormat"] = colorFormat.ToString(),
            ["alphaPrecision"] = alphaPrecision.ToString()
        });
    }
}
