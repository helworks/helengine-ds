namespace helengine.ds.builder;

/// <summary>
/// Writes the builder-owned Nintendo DS startup manifest into a staged NitroFS root.
/// </summary>
public sealed class NintendoDsStartupManifestWriter {
    /// <summary>
    /// Stores the stable runtime-relative NitroFS path used for the startup manifest payload.
    /// </summary>
    const string ManifestRelativePath = "runtime/ds_startup_manifest.bin";

    /// <summary>
    /// Writes one startup manifest into the staged NitroFS root.
    /// </summary>
    /// <param name="nitroFsRootPath">NitroFS root directory that receives the manifest.</param>
    /// <param name="topScreenColorHex">Top-screen color in <c>#RRGGBB</c> form.</param>
    /// <param name="bottomScreenColorHex">Bottom-screen color in <c>#RRGGBB</c> form.</param>
    /// <returns>Full path to the emitted startup-manifest file.</returns>
    public string Write(string nitroFsRootPath, string topScreenColorHex, string bottomScreenColorHex) {
        if (string.IsNullOrWhiteSpace(nitroFsRootPath)) {
            throw new ArgumentException("NitroFS root path must be provided.", nameof(nitroFsRootPath));
        }

        NintendoDsStartupManifest manifest = new(
            ParseColor(topScreenColorHex, "startup top screen color"),
            ParseColor(bottomScreenColorHex, "startup bottom screen color"));

        string manifestPath = Path.Combine(nitroFsRootPath, ManifestRelativePath.Replace('/', Path.DirectorySeparatorChar));
        string manifestDirectoryPath = Path.GetDirectoryName(manifestPath)
            ?? throw new InvalidOperationException("Unable to resolve the Nintendo DS startup manifest directory.");
        Directory.CreateDirectory(manifestDirectoryPath);

        using FileStream stream = File.Create(manifestPath);
        using BinaryWriter writer = new(stream);
        writer.Write(new byte[] { 0x48, 0x44, 0x53, 0x50 });
        writer.Write((ushort)1);
        writer.Write((ushort)4);
        writer.Write(manifest.TopScreenColor);
        writer.Write(manifest.BottomScreenColor);

        return manifestPath;
    }

    /// <summary>
    /// Parses one authored RGB color string into the packed Nintendo DS 15-bit color format with the visibility bit enabled.
    /// </summary>
    /// <param name="colorText">Authored color string in <c>#RRGGBB</c> form.</param>
    /// <param name="fieldName">Human-readable field name used in validation errors.</param>
    /// <returns>Packed Nintendo DS color value.</returns>
    static ushort ParseColor(string colorText, string fieldName) {
        if (string.IsNullOrWhiteSpace(colorText)) {
            throw new InvalidOperationException($"The {fieldName} build setting is required.");
        }

        string normalized = colorText.Trim();
        if (normalized.StartsWith("#", StringComparison.Ordinal)) {
            normalized = normalized.Substring(1);
        }

        if (normalized.Length != 6) {
            throw new InvalidOperationException($"The {fieldName} build setting must use #RRGGBB format.");
        }

        try {
            byte red = Convert.ToByte(normalized.Substring(0, 2), 16);
            byte green = Convert.ToByte(normalized.Substring(2, 2), 16);
            byte blue = Convert.ToByte(normalized.Substring(4, 2), 16);

            ushort packedRed = (ushort)((red * 31) / 255);
            ushort packedGreen = (ushort)(((green * 31) / 255) << 5);
            ushort packedBlue = (ushort)(((blue * 31) / 255) << 10);
            return (ushort)(0x8000 | packedRed | packedGreen | packedBlue);
        } catch (FormatException ex) {
            throw new InvalidOperationException($"The {fieldName} build setting must use #RRGGBB format.", ex);
        } catch (OverflowException ex) {
            throw new InvalidOperationException($"The {fieldName} build setting must use #RRGGBB format.", ex);
        }
    }
}
