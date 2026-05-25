using System.Text.Json;

namespace helengine.ds.builder;

/// <summary>
/// Serializes and deserializes the generic Nintendo DS texture cook settings contract emitted by the editor build graph.
/// </summary>
public static class NintendoDsTextureCookSettingsSerializer {
    /// <summary>
    /// Serializes one Nintendo DS texture settings payload to the generic editor JSON contract.
    /// </summary>
    /// <param name="settings">Texture settings to serialize.</param>
    /// <returns>Serialized JSON payload consumed by builder-owned cook work items.</returns>
    public static string Serialize(TextureAssetProcessorSettings settings) {
        if (settings == null) {
            throw new ArgumentNullException(nameof(settings));
        }

        return JsonSerializer.Serialize(new {
            maxResolution = settings.MaxResolution,
            colorFormat = settings.ColorFormatId,
            alphaPrecision = settings.AlphaPrecision.ToString()
        });
    }

    /// <summary>
    /// Serializes one Nintendo DS texture settings payload to the generic editor JSON contract.
    /// </summary>
    /// <param name="maxResolution">Maximum authored texture resolution preserved by the platform override.</param>
    /// <param name="colorFormat">Cooked texture color format selected for Nintendo DS.</param>
    /// <param name="alphaPrecision">Cooked texture alpha precision selected for Nintendo DS.</param>
    /// <returns>Serialized JSON payload consumed by builder-owned cook work items.</returns>
    public static string Serialize(int maxResolution, TextureAssetColorFormat colorFormat, TextureAssetAlphaPrecision alphaPrecision) {
        return Serialize(new TextureAssetProcessorSettings {
            MaxResolution = maxResolution,
            ColorFormatId = colorFormat.ToString(),
            AlphaPrecision = alphaPrecision
        });
    }

    /// <summary>
    /// Deserializes one Nintendo DS texture settings payload from the generic editor JSON contract.
    /// </summary>
    /// <param name="serializedSettings">Serialized JSON payload emitted by the editor build graph.</param>
    /// <returns>Resolved Nintendo DS texture processor settings.</returns>
    public static TextureAssetProcessorSettings Deserialize(string serializedSettings) {
        if (string.IsNullOrWhiteSpace(serializedSettings)) {
            throw new ArgumentException("Serialized Nintendo DS texture settings must be provided.", nameof(serializedSettings));
        }

        using JsonDocument document = JsonDocument.Parse(serializedSettings);
        JsonElement root = document.RootElement;
        int maxResolution = root.TryGetProperty("maxResolution", out JsonElement maxResolutionElement)
            ? maxResolutionElement.GetInt32()
            : 0;
        string colorFormatId = root.TryGetProperty("colorFormat", out JsonElement colorFormatElement)
            ? colorFormatElement.GetString() ?? TextureAssetColorFormat.Rgba32.ToString()
            : TextureAssetColorFormat.Rgba32.ToString();
        string alphaPrecisionName = root.TryGetProperty("alphaPrecision", out JsonElement alphaPrecisionElement)
            ? alphaPrecisionElement.GetString() ?? TextureAssetAlphaPrecision.A8.ToString()
            : TextureAssetAlphaPrecision.A8.ToString();

        if (!Enum.TryParse(alphaPrecisionName, true, out TextureAssetAlphaPrecision alphaPrecision)) {
            throw new InvalidOperationException($"Unsupported Nintendo DS texture alpha precision '{alphaPrecisionName}'.");
        }

        return new TextureAssetProcessorSettings {
            MaxResolution = maxResolution,
            ColorFormatId = colorFormatId,
            AlphaPrecision = alphaPrecision
        };
    }
}
