namespace helengine.ds.builder;

/// <summary>Resizes decoded pixels and produces independently owned native DS image/font atlas storage.</summary>
public static class NintendoDsNativeTextureCooker {
    /// <summary>Checks published native format, alpha and maximum resolution settings before allocation.</summary>
    public static void ValidateSettings(TextureAssetProcessorSettings settings) {
        if (settings == null) throw new ArgumentNullException(nameof(settings));
        if (settings.MaxResolution < 0 || settings.MaxResolution > 1024) throw new ArgumentException("Native DS maximum resolution must be zero or 1..1024.");
        if (!NintendoDsTextureFormatCatalog.TryGetFormat(settings.ColorFormatId, out NintendoDsTextureFormat format) || !format.SupportsAlpha(settings.AlphaPrecision)) throw new ArgumentException("Unsupported native DS format or alpha precision.");
    }
    /// <summary>Preserves source identity while applying native encoding and the legacy DS center-sampled nearest resize rule.</summary>
    public static TextureAsset Apply(TextureAsset source, TextureAssetProcessorSettings settings) {
        if (source == null) throw new ArgumentNullException(nameof(source));
        ValidateSettings(settings);
        byte[] rgba = TextureAssetPixelCodec.DecodeToRgba32(source);
        int width = source.Width, height = source.Height;
        if (settings.MaxResolution > 0 && Math.Max(width, height) > settings.MaxResolution) {
            double scale = settings.MaxResolution / (double)Math.Max(width, height);
            int resizedWidth = Math.Max(1, (int)Math.Round(width * scale)), resizedHeight = Math.Max(1, (int)Math.Round(height * scale));
            byte[] resized = new byte[resizedWidth * resizedHeight * 4];
            for (int y = 0; y < resizedHeight; y++) for (int x = 0; x < resizedWidth; x++) {
                int sourceX = Math.Clamp((int)Math.Round((x + 0.5d) * width / resizedWidth - 0.5d), 0, width - 1);
                int sourceY = Math.Clamp((int)Math.Round((y + 0.5d) * height / resizedHeight - 0.5d), 0, height - 1);
                Array.Copy(rgba, (sourceY * width + sourceX) * 4, resized, (y * resizedWidth + x) * 4, 4);
            }
            rgba = resized; width = resizedWidth; height = resizedHeight;
        }
        if (width > 1024 || height > 1024) throw new ArgumentException("Cooked DS native extents must be within 1..1024.");
        using TextureAsset decoded = new TextureAsset { Id = source.Id, RuntimeAssetId = source.RuntimeAssetId, AuthoringAssetId = source.AuthoringAssetId,
            FormerAuthoringAssetIds = source.FormerAuthoringAssetIds == null ? Array.Empty<string>() : (string[])source.FormerAuthoringAssetIds.Clone(), IsEngineOwned = source.IsEngineOwned,
            Width = (ushort)width, Height = (ushort)height, ColorFormat = TextureAssetColorFormat.Rgba32, AlphaPrecision = TextureAssetAlphaPrecision.A8, Colors = rgba };
        return NintendoDsTextureCodec.Encode(decoded, settings.ColorFormatId, settings.AlphaPrecision);
    }
}
