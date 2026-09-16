using helengine;
using helengine.baseplatform.Builders;
using helengine.baseplatform.Reporting;
using helengine.files;
using System.Globalization;

namespace helengine.ds.builder;

/// <summary>
/// Reports staged Nintendo DS scene state that the current runtime cannot render faithfully.
/// </summary>
public sealed class NintendoDsSceneAssetCompatibilityReporter {
    /// <summary>
    /// Stable diagnostic code for a SpriteComponent source rectangle that the Nintendo DS OBJ path cannot sample.
    /// </summary>
    const string UnsupportedSpriteSourceRectDiagnosticCode = "DS2D001";

    /// <summary>
    /// Stable diagnostic code for a sprite texture that exceeds the Nintendo DS bottom-screen OBJ palette limit.
    /// </summary>
    const string UnsupportedBottomScreenSpritePaletteDiagnosticCode = "DS2D002";

    /// <summary>
    /// Maximum opaque colors available after reserving palette index zero for transparency in a 4bpp DS OBJ palette.
    /// </summary>
    const int BottomScreenOpaquePaletteColorLimit = 15;

    /// <summary>
    /// Stable packaged component type identifier used by SpriteComponent records.
    /// </summary>
    const string SpriteComponentTypeId = "helengine.SpriteComponent";

    /// <summary>
    /// Inspects staged scene assets and reports permitted but unsupported Nintendo DS runtime state.
    /// </summary>
    /// <param name="nitroFsRootPath">NitroFS root containing the staged cooked scene assets.</param>
    /// <param name="diagnosticReporter">Build diagnostic stream that receives compatibility warnings.</param>
    public void ReportStagedSceneCompatibility(
        string nitroFsRootPath,
        IPlatformBuildDiagnosticReporter diagnosticReporter) {
        if (string.IsNullOrWhiteSpace(nitroFsRootPath)) {
            throw new ArgumentException("NitroFS root path must be provided.", nameof(nitroFsRootPath));
        } else if (diagnosticReporter == null) {
            throw new ArgumentNullException(nameof(diagnosticReporter));
        }

        string sceneRootPath = Path.Combine(nitroFsRootPath, "cooked", "scenes");
        if (!Directory.Exists(sceneRootPath)) {
            return;
        }

        string[] sceneFilePaths = Directory.GetFiles(sceneRootPath, "*.hasset", SearchOption.AllDirectories);
        Array.Sort(sceneFilePaths, StringComparer.OrdinalIgnoreCase);
        for (int sceneIndex = 0; sceneIndex < sceneFilePaths.Length; sceneIndex++) {
            ReportSceneCompatibility(nitroFsRootPath, sceneFilePaths[sceneIndex], diagnosticReporter);
        }
    }

    /// <summary>
    /// Deserializes one staged scene and reports unsupported sprite state from every entity hierarchy level.
    /// </summary>
    /// <param name="nitroFsRootPath">NitroFS root used to resolve packaged texture references.</param>
    /// <param name="sceneFilePath">Absolute staged scene path to inspect.</param>
    /// <param name="diagnosticReporter">Build diagnostic stream receiving compatibility warnings.</param>
    static void ReportSceneCompatibility(
        string nitroFsRootPath,
        string sceneFilePath,
        IPlatformBuildDiagnosticReporter diagnosticReporter) {
        SceneAsset sceneAsset;
        using (FileStream stream = File.OpenRead(sceneFilePath)) {
            sceneAsset = helengine.files.AssetSerializer.Deserialize(stream) as SceneAsset
                ?? throw new InvalidOperationException($"Nintendo DS staged scene asset '{sceneFilePath}' did not deserialize into a SceneAsset.");
        }

        if (sceneAsset.RootEntities == null) {
            throw new InvalidOperationException($"Nintendo DS staged scene asset '{sceneFilePath}' must provide its root entities.");
        }

        for (int entityIndex = 0; entityIndex < sceneAsset.RootEntities.Length; entityIndex++) {
            ReportEntityCompatibility(nitroFsRootPath, sceneAsset, sceneAsset.RootEntities[entityIndex], string.Empty, diagnosticReporter);
        }
    }

    /// <summary>
    /// Reports unsupported sprite state from one staged entity and recursively inspects its children.
    /// </summary>
    /// <param name="nitroFsRootPath">NitroFS root used to resolve packaged texture references.</param>
    /// <param name="sceneAsset">Scene that owns the entity being inspected.</param>
    /// <param name="entityAsset">Staged entity whose component payloads should be inspected.</param>
    /// <param name="parentEntityPath">Slash-separated path of the staged parent entity.</param>
    /// <param name="diagnosticReporter">Build diagnostic stream receiving compatibility warnings.</param>
    static void ReportEntityCompatibility(
        string nitroFsRootPath,
        SceneAsset sceneAsset,
        SceneEntityAsset entityAsset,
        string parentEntityPath,
        IPlatformBuildDiagnosticReporter diagnosticReporter) {
        if (entityAsset == null) {
            throw new InvalidOperationException("Nintendo DS staged scene entities cannot contain null entries.");
        } else if (entityAsset.Components == null) {
            throw new InvalidOperationException("Nintendo DS staged scene entities must provide their component records.");
        } else if (entityAsset.Children == null) {
            throw new InvalidOperationException("Nintendo DS staged scene entities must provide their child entities.");
        }

        string entityName = string.IsNullOrWhiteSpace(entityAsset.Name)
            ? "entity-" + entityAsset.Id.ToString(CultureInfo.InvariantCulture)
            : entityAsset.Name;
        string entityPath = string.IsNullOrWhiteSpace(parentEntityPath)
            ? entityName
            : parentEntityPath + "/" + entityName;
        for (int componentIndex = 0; componentIndex < entityAsset.Components.Length; componentIndex++) {
            SceneComponentAssetRecord componentRecord = entityAsset.Components[componentIndex]
                ?? throw new InvalidOperationException($"Nintendo DS staged entity '{entityPath}' cannot contain null component records.");
            if (!IsSpriteComponent(componentRecord.ComponentTypeId)) {
                continue;
            }

            string componentIdentity = string.IsNullOrWhiteSpace(componentRecord.ComponentKey)
                ? "component index " + componentRecord.ComponentIndex.ToString(CultureInfo.InvariantCulture)
                : "component '" + componentRecord.ComponentKey + "'";
            ReadSpriteState(componentRecord, entityPath, out float4 sourceRect, out SceneAssetReference textureReference);
            if (!IsFullSourceRect(sourceRect)) {
                string message = string.Format(
                    CultureInfo.InvariantCulture,
                    "Nintendo DS sprite {0} on entity '{1}' uses unsupported SourceRect ({2}, {3}, {4}, {5}). "
                        + "The Nintendo DS OBJ renderer will skip this sprite; it currently requires SourceRect (0, 0, 1, 1).",
                    componentIdentity,
                    entityPath,
                    sourceRect.X,
                    sourceRect.Y,
                    sourceRect.Z,
                    sourceRect.W);
                diagnosticReporter.Report(new PlatformBuildDiagnostic(
                    PlatformBuildDiagnosticSeverity.Warning,
                    UnsupportedSpriteSourceRectDiagnosticCode,
                    message,
                    sceneAsset.Id ?? string.Empty,
                    string.Empty,
                    entityPath));
            }

            ReportBottomScreenPaletteCompatibility(
                nitroFsRootPath,
                sceneAsset,
                entityPath,
                componentIdentity,
                textureReference,
                diagnosticReporter);
        }

        for (int childIndex = 0; childIndex < entityAsset.Children.Length; childIndex++) {
            ReportEntityCompatibility(nitroFsRootPath, sceneAsset, entityAsset.Children[childIndex], entityPath, diagnosticReporter);
        }
    }

    /// <summary>
    /// Determines whether one packaged component type identifier represents SpriteComponent.
    /// </summary>
    /// <param name="componentTypeId">Packaged component type identifier to inspect.</param>
    /// <returns><c>true</c> when the component is a current or legacy SpriteComponent record.</returns>
    static bool IsSpriteComponent(string componentTypeId) {
        return string.Equals(componentTypeId, SpriteComponentTypeId, StringComparison.OrdinalIgnoreCase)
            || string.Equals(componentTypeId, "helengine.SpriteComponent, helengine.core", StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// Reads the stable size, SourceRect, and texture members from one automatic ordinal SpriteComponent payload.
    /// </summary>
    /// <param name="componentRecord">Packaged SpriteComponent record to decode.</param>
    /// <param name="entityPath">Owning entity path used when reporting malformed payloads.</param>
    /// <param name="sourceRect">Receives the authored normalized sprite source rectangle.</param>
    /// <param name="textureReference">Receives the optional packaged texture reference.</param>
    static void ReadSpriteState(
        SceneComponentAssetRecord componentRecord,
        string entityPath,
        out float4 sourceRect,
        out SceneAssetReference textureReference) {
        using MemoryStream stream = new(componentRecord.Payload ?? Array.Empty<byte>(), writable: false);
        using EngineBinaryReader reader = EngineBinaryReader.Create(stream, EngineBinaryEndianness.LittleEndian);
        byte version = reader.ReadByte();
        if (version != AutomaticScriptComponentRuntimeDeserializer.CurrentVersion) {
            throw new InvalidOperationException(
                $"Nintendo DS staged SpriteComponent on entity '{entityPath}' uses unsupported automatic payload version '{version}'.");
        }

        int memberCount = reader.ReadInt32();
        if (memberCount < 4) {
            throw new InvalidOperationException(
                $"Nintendo DS staged SpriteComponent on entity '{entityPath}' does not contain the required SourceRect member.");
        }

        reader.ReadByte();
        reader.ReadByte();
        reader.ReadByte();
        reader.ReadByte();
        reader.ReadByte();
        reader.ReadInt2();
        sourceRect = reader.ReadFloat4();
        textureReference = memberCount >= 5
            ? SceneAssetReferenceFactory.ReadOptionalReference(reader)
            : null;
    }

    /// <summary>
    /// Reports when one packaged sprite texture cannot fit the 4bpp palette available to the DS bottom OBJ engine.
    /// </summary>
    /// <param name="nitroFsRootPath">NitroFS root used to resolve the packaged texture.</param>
    /// <param name="sceneAsset">Scene that owns the sprite.</param>
    /// <param name="entityPath">Owning entity path used by the diagnostic.</param>
    /// <param name="componentIdentity">Stable component key or index description.</param>
    /// <param name="textureReference">Optional packaged texture reference stored on the sprite.</param>
    /// <param name="diagnosticReporter">Build diagnostic stream receiving compatibility warnings.</param>
    static void ReportBottomScreenPaletteCompatibility(
        string nitroFsRootPath,
        SceneAsset sceneAsset,
        string entityPath,
        string componentIdentity,
        SceneAssetReference textureReference,
        IPlatformBuildDiagnosticReporter diagnosticReporter) {
        if (textureReference == null
            || textureReference.SourceKind != SceneAssetReferenceSourceKind.FileSystem
            || string.IsNullOrWhiteSpace(textureReference.RelativePath)) {
            return;
        }

        string rootPath = Path.GetFullPath(nitroFsRootPath);
        string texturePath = Path.GetFullPath(Path.Combine(
            rootPath,
            textureReference.RelativePath.Replace('/', Path.DirectorySeparatorChar).TrimStart(Path.DirectorySeparatorChar)));
        string rootPrefix = rootPath.EndsWith(Path.DirectorySeparatorChar)
            ? rootPath
            : rootPath + Path.DirectorySeparatorChar;
        if (!texturePath.StartsWith(rootPrefix, StringComparison.OrdinalIgnoreCase)) {
            throw new InvalidOperationException(
                $"Nintendo DS staged sprite texture '{textureReference.RelativePath}' resolves outside the NitroFS root.");
        }
        if (!File.Exists(texturePath)) {
            return;
        }

        TextureAsset textureAsset;
        using (FileStream stream = File.OpenRead(texturePath)) {
            textureAsset = helengine.files.AssetSerializer.Deserialize(stream) as TextureAsset
                ?? throw new InvalidOperationException(
                    $"Nintendo DS staged sprite texture '{textureReference.RelativePath}' did not deserialize into a TextureAsset.");
        }

        if (!TryCountOpaqueDsPaletteColors(textureAsset, out int opaqueColorCount)
            || opaqueColorCount <= BottomScreenOpaquePaletteColorLimit) {
            return;
        }

        string message = string.Format(
            CultureInfo.InvariantCulture,
            "Nintendo DS sprite {0} on entity '{1}' references texture '{2}' with {3} opaque colors after DS conversion. "
                + "The bottom-screen OBJ renderer will skip this sprite because its 4bpp palette supports at most {4} opaque colors plus transparency; "
                + "the main-screen OBJ renderer may use its 8bpp fallback.",
            componentIdentity,
            entityPath,
            textureReference.RelativePath,
            opaqueColorCount,
            BottomScreenOpaquePaletteColorLimit);
        diagnosticReporter.Report(new PlatformBuildDiagnostic(
            PlatformBuildDiagnosticSeverity.Warning,
            UnsupportedBottomScreenSpritePaletteDiagnosticCode,
            message,
            sceneAsset.Id ?? string.Empty,
            textureReference.RelativePath,
            entityPath));
    }

    /// <summary>
    /// Counts distinct opaque RGB15 colors exactly as the current DS 4bpp OBJ preparation path sees them.
    /// </summary>
    /// <param name="textureAsset">Cooked texture asset to inspect.</param>
    /// <param name="opaqueColorCount">Receives the number of distinct opaque RGB15 colors.</param>
    /// <returns><c>true</c> when the texture uses a format handled by the DS OBJ preparation path.</returns>
    static bool TryCountOpaqueDsPaletteColors(TextureAsset textureAsset, out int opaqueColorCount) {
        HashSet<ushort> colors = new HashSet<ushort>();
        int pixelCount = checked(textureAsset.Width * textureAsset.Height);
        byte[] pixelBytes = textureAsset.Colors ?? Array.Empty<byte>();
        if (textureAsset.ColorFormat == TextureAssetColorFormat.Rgba4444) {
            int requiredByteCount = checked(pixelCount * 2);
            if (pixelBytes.Length < requiredByteCount) {
                throw new InvalidOperationException(
                    $"Nintendo DS RGBA4444 texture '{textureAsset.Id}' requires {requiredByteCount} color bytes but contains {pixelBytes.Length}.");
            }

            for (int pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {
                int sourceIndex = pixelIndex * 2;
                ushort packedColor = (ushort)(pixelBytes[sourceIndex] | (pixelBytes[sourceIndex + 1] << 8));
                int alpha = ((packedColor >> 12) & 15) * 17;
                if (alpha < 128) {
                    continue;
                }

                int red = ((packedColor >> 0) & 15) * 17;
                int green = ((packedColor >> 4) & 15) * 17;
                int blue = ((packedColor >> 8) & 15) * 17;
                colors.Add(PackRgb15(red, green, blue));
            }

            opaqueColorCount = colors.Count;
            return true;
        }
        if (textureAsset.ColorFormat != TextureAssetColorFormat.Indexed4
            && textureAsset.ColorFormat != TextureAssetColorFormat.Indexed8) {
            opaqueColorCount = 0;
            return false;
        }

        byte[] paletteBytes = textureAsset.PaletteColors ?? Array.Empty<byte>();
        int requiredIndexByteCount = textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4
            ? (pixelCount + 1) / 2
            : pixelCount;
        if (pixelBytes.Length < requiredIndexByteCount) {
            throw new InvalidOperationException(
                $"Nintendo DS indexed texture '{textureAsset.Id}' requires {requiredIndexByteCount} index bytes but contains {pixelBytes.Length}.");
        }

        for (int pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {
            int paletteIndex;
            if (textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4) {
                byte packedIndices = pixelBytes[pixelIndex / 2];
                paletteIndex = (pixelIndex & 1) == 0
                    ? packedIndices & 15
                    : (packedIndices >> 4) & 15;
            } else {
                paletteIndex = pixelBytes[pixelIndex];
            }

            int paletteOffset = paletteIndex * 4;
            if (paletteOffset < 0 || paletteOffset + 3 >= paletteBytes.Length) {
                throw new InvalidOperationException(
                    $"Nintendo DS indexed texture '{textureAsset.Id}' references palette index {paletteIndex} outside its palette payload.");
            }
            if (paletteBytes[paletteOffset + 3] < 128) {
                continue;
            }

            colors.Add(PackRgb15(
                paletteBytes[paletteOffset],
                paletteBytes[paletteOffset + 1],
                paletteBytes[paletteOffset + 2]));
        }

        opaqueColorCount = colors.Count;
        return true;
    }

    /// <summary>
    /// Packs one 8-bit RGB color into the 15-bit color space used by DS OBJ palettes.
    /// </summary>
    /// <param name="red">Red channel in the range 0 through 255.</param>
    /// <param name="green">Green channel in the range 0 through 255.</param>
    /// <param name="blue">Blue channel in the range 0 through 255.</param>
    /// <returns>Packed RGB15 color without the opaque high bit.</returns>
    static ushort PackRgb15(int red, int green, int blue) {
        return (ushort)(
            ((red >> 3) & 31)
            | (((green >> 3) & 31) << 5)
            | (((blue >> 3) & 31) << 10));
    }

    /// <summary>
    /// Determines whether a normalized source rectangle samples the complete texture accepted by the current DS OBJ path.
    /// </summary>
    /// <param name="sourceRect">Normalized source rectangle to evaluate.</param>
    /// <returns><c>true</c> when the rectangle samples the complete texture within runtime tolerance.</returns>
    static bool IsFullSourceRect(float4 sourceRect) {
        return Math.Abs(sourceRect.X) <= 0.001f
            && Math.Abs(sourceRect.Y) <= 0.001f
            && Math.Abs(sourceRect.Z - 1f) <= 0.001f
            && Math.Abs(sourceRect.W - 1f) <= 0.001f;
    }
}
