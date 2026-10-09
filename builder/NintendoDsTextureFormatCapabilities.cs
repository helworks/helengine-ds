using helengine.baseplatform.Definitions;

namespace helengine.ds.builder;

/// <summary>Publishes all native DS texture and tile formats alongside the existing image/font defaults.</summary>
public static class NintendoDsTextureFormatCapabilities {
    /// <summary>Creates exact native alpha combinations while retaining the legacy DS cook contract.</summary>
    public static PlatformTextureFormatCapabilityDefinition Create() {
        List<string> formats = new List<string> { "Rgba4444", "Indexed4", "Indexed8" };
        List<PlatformTextureFormatCombinationDefinition> combinations = new List<PlatformTextureFormatCombinationDefinition> {
            new PlatformTextureFormatCombinationDefinition("Rgba4444", TextureAssetAlphaPrecision.A4),
            new PlatformTextureFormatCombinationDefinition("Indexed4", TextureAssetAlphaPrecision.Binary),
            new PlatformTextureFormatCombinationDefinition("Indexed8", TextureAssetAlphaPrecision.A8)
        };
        foreach (NintendoDsTextureFormat format in NintendoDsTextureFormatCatalog.Formats) {
            formats.Add(format.Id); combinations.Add(new PlatformTextureFormatCombinationDefinition(format.Id, TextureAssetAlphaPrecision.Opaque));
            combinations.Add(new PlatformTextureFormatCombinationDefinition(format.Id, format.Alpha));
        }
        return new PlatformTextureFormatCapabilityDefinition(formats.ToArray(), new[] { TextureAssetAlphaPrecision.Opaque, TextureAssetAlphaPrecision.Binary, TextureAssetAlphaPrecision.A3, TextureAssetAlphaPrecision.A4, TextureAssetAlphaPrecision.A5, TextureAssetAlphaPrecision.A8 }, combinations.ToArray());
    }
}
