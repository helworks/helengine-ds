using helengine;
using helengine.baseplatform.Reporting;
using helengine.ds.builder.tests.Builders;
using helengine.files;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies Nintendo DS cook diagnostics for staged scene state that the runtime cannot render faithfully.
/// </summary>
public class NintendoDsSceneAssetCompatibilityReporterTests {
    /// <summary>
    /// Verifies a cropped sprite remains permitted while the cook reports that the DS OBJ renderer will skip it.
    /// </summary>
    /// <param name="storageBits">Target storage and arithmetic width, independent of the managed tooling host.</param>
    [Theory]
    [InlineData(32)]
    [InlineData(64)]
    public void ReportStagedSceneCompatibility_whenSpriteUsesCroppedSourceRect_emitsVisibleWarning(int storageBits) {
        NumericPrecisionPolicy numericPolicy = new(storageBits, storageBits);
        string rootPath = Path.Combine(Path.GetTempPath(), "scene-compatibility-");
        string nitroFsRootPath = Path.Combine(rootPath, "nitrofs");
        string scenePath = Path.Combine(nitroFsRootPath, "cooked", "scenes", "tilt_trial_level_01.hasset");

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(scenePath)
                ?? throw new InvalidOperationException("Unable to resolve the staged scene directory path."));
            byte[] originalSceneBytes = BuildSceneAssetBytes(new float4(0.2109375f, 0.2109375f, 0.58203125f, 0.58203125f), numericPolicy: numericPolicy);
            File.WriteAllBytes(scenePath, originalSceneBytes);
            RecordingDiagnosticReporter diagnosticReporter = new();

            new NintendoDsSceneAssetCompatibilityReporter().ReportStagedSceneCompatibility(nitroFsRootPath, diagnosticReporter, numericPolicy);

            PlatformBuildDiagnostic diagnostic = Assert.Single(diagnosticReporter.Diagnostics);
            Assert.Equal(PlatformBuildDiagnosticSeverity.Warning, diagnostic.Severity);
            Assert.Equal("DS2D001", diagnostic.Code);
            Assert.Equal("scenes/games/tilt/tilt_trial_level_01.helen", diagnostic.SceneId);
            Assert.Equal("TiltTrialStartPrompt/TiltTrialStartPromptIcon", diagnostic.SourceIdentity);
            Assert.Contains("SourceRect", diagnostic.Message, StringComparison.Ordinal);
            Assert.Contains("0.2109375", diagnostic.Message, StringComparison.Ordinal);
            Assert.Contains("Nintendo DS OBJ renderer will skip this sprite", diagnostic.Message, StringComparison.Ordinal);
            Assert.Equal(originalSceneBytes, File.ReadAllBytes(scenePath));
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies a sprite texture that exceeds the bottom OBJ palette limit remains permitted while the cook reports the exact incompatibility.
    /// </summary>
    /// <param name="storageBits">Target storage and arithmetic width used by the scene, component payload, and texture.</param>
    [Theory]
    [InlineData(32)]
    [InlineData(64)]
    public void ReportStagedSceneCompatibility_whenSpriteTextureExceedsBottomPalette_emitsVisibleWarning(int storageBits) {
        NumericPrecisionPolicy numericPolicy = new(storageBits, storageBits);
        string rootPath = Path.Combine(Path.GetTempPath(), "scene-texture-compatibility-");
        string nitroFsRootPath = Path.Combine(rootPath, "nitrofs");
        const string textureRelativePath = "cooked/imported/start-a.hetex";
        string scenePath = Path.Combine(nitroFsRootPath, "cooked", "scenes", "tilt_trial_level_01.hasset");
        string texturePath = Path.Combine(nitroFsRootPath, textureRelativePath.Replace('/', Path.DirectorySeparatorChar));

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(scenePath)
                ?? throw new InvalidOperationException("Unable to resolve the staged scene directory path."));
            Directory.CreateDirectory(Path.GetDirectoryName(texturePath)
                ?? throw new InvalidOperationException("Unable to resolve the staged texture directory path."));
            File.WriteAllBytes(
                scenePath,
                BuildSceneAssetBytes(new float4(0f, 0f, 1f, 1f), textureRelativePath, numericPolicy));
            File.WriteAllBytes(texturePath, BuildRgba4444TextureAssetBytes(16, numericPolicy));
            RecordingDiagnosticReporter diagnosticReporter = new();

            new NintendoDsSceneAssetCompatibilityReporter().ReportStagedSceneCompatibility(nitroFsRootPath, diagnosticReporter, numericPolicy);

            PlatformBuildDiagnostic diagnostic = Assert.Single(diagnosticReporter.Diagnostics);
            Assert.Equal(PlatformBuildDiagnosticSeverity.Warning, diagnostic.Severity);
            Assert.Equal("DS2D002", diagnostic.Code);
            Assert.Equal("scenes/games/tilt/tilt_trial_level_01.helen", diagnostic.SceneId);
            Assert.Equal("TiltTrialStartPrompt/TiltTrialStartPromptIcon", diagnostic.SourceIdentity);
            Assert.Contains(textureRelativePath, diagnostic.Message, StringComparison.Ordinal);
            Assert.Contains("16 opaque colors", diagnostic.Message, StringComparison.Ordinal);
            Assert.Contains("15", diagnostic.Message, StringComparison.Ordinal);
            Assert.Contains("bottom-screen", diagnostic.Message, StringComparison.OrdinalIgnoreCase);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Builds one serialized staged scene containing a SpriteComponent automatic ordinal payload.
    /// </summary>
    /// <param name="sourceRect">Authored normalized sprite source rectangle.</param>
    /// <param name="textureRelativePath">Optional staged texture path referenced by the sprite.</param>
    /// <param name="numericPolicy">Target numeric policy written into the scene and its component payload.</param>
    /// <returns>Serialized scene bytes containing the supplied sprite state.</returns>
    static byte[] BuildSceneAssetBytes(float4 sourceRect, string textureRelativePath = "", NumericPrecisionPolicy numericPolicy = null) {
        SceneAsset sceneAsset = new() {
            Id = "scenes/games/tilt/tilt_trial_level_01.helen",
            AssetReferences = string.IsNullOrWhiteSpace(textureRelativePath)
                ? Array.Empty<SceneAssetReference>()
                : [SceneAssetReferenceFactory.CreateFileSystemTexture(textureRelativePath)],
            RootEntities = [
                new SceneEntityAsset {
                    Id = 1,
                    Name = "TiltTrialStartPrompt",
                    LocalPosition = float3.Zero,
                    LocalScale = float3.One,
                    LocalOrientation = float4.Identity,
                    Components = Array.Empty<SceneComponentAssetRecord>(),
                    Children = [
                        new SceneEntityAsset {
                            Id = 2,
                            Name = "TiltTrialStartPromptIcon",
                            LocalPosition = float3.Zero,
                            LocalScale = float3.One,
                            LocalOrientation = float4.Identity,
                            Components = [
                                new SceneComponentAssetRecord {
                                    ComponentKey = "sprite-component",
                                    ComponentTypeId = "helengine.SpriteComponent",
                                    ComponentIndex = 0,
                                    Payload = BuildSpriteComponentPayload(sourceRect, textureRelativePath, numericPolicy)
                                }
                            ],
                            Children = Array.Empty<SceneEntityAsset>()
                        }
                    ]
                }
            ]
        };
        return helengine.files.AssetSerializer.SerializeToBytes(sceneAsset, numericPolicy ?? RealRepresentation.Policy);
    }

    /// <summary>
    /// Builds the stable five-member automatic SpriteComponent payload used by packaged scenes.
    /// </summary>
    /// <param name="sourceRect">Authored normalized sprite source rectangle.</param>
    /// <param name="textureRelativePath">Optional staged texture path referenced by the sprite.</param>
    /// <param name="numericPolicy">Target numeric policy written into numeric component members.</param>
    /// <returns>Little-endian automatic component payload bytes.</returns>
    static byte[] BuildSpriteComponentPayload(float4 sourceRect, string textureRelativePath, NumericPrecisionPolicy numericPolicy) {
        using MemoryStream stream = new();
        using (EngineBinaryWriter writer = EngineBinaryWriter.Create(stream, EngineBinaryEndianness.LittleEndian)) {
            writer.NumericPolicy = numericPolicy ?? RealRepresentation.Policy;
            writer.WriteByte(AutomaticScriptComponentRuntimeDeserializer.CurrentVersion);
            writer.WriteInt32(5);
            writer.WriteByte(255);
            writer.WriteByte(255);
            writer.WriteByte(255);
            writer.WriteByte(255);
            writer.WriteByte(220);
            writer.WriteInt2(new int2(32, 32));
            writer.WriteFloat4(sourceRect);
            if (string.IsNullOrWhiteSpace(textureRelativePath)) {
                writer.WriteByte(0);
            } else {
                SceneAssetReference reference = SceneAssetReferenceFactory.CreateFileSystemTexture(textureRelativePath);
                writer.WriteByte(1);
                writer.WriteInt32((int)reference.SourceKind);
                writer.WriteString(reference.RelativePath);
                writer.WriteString(reference.ProviderId);
                writer.WriteString(reference.AssetId);
                writer.WriteString(reference.ContentHash);
            }
        }

        return stream.ToArray();
    }

    /// <summary>
    /// Builds one valid 8 by 8 RGBA4444 texture containing the requested number of opaque DS RGB15 colors.
    /// </summary>
    /// <param name="opaqueColorCount">Number of distinct opaque colors to encode.</param>
    /// <param name="numericPolicy">Target numeric policy stored in the packaged texture container.</param>
    /// <returns>Serialized packaged texture bytes.</returns>
    static byte[] BuildRgba4444TextureAssetBytes(int opaqueColorCount, NumericPrecisionPolicy numericPolicy) {
        if (opaqueColorCount < 1 || opaqueColorCount > 16) {
            throw new ArgumentOutOfRangeException(nameof(opaqueColorCount));
        }

        byte[] colors = new byte[8 * 8 * 2];
        for (int pixelIndex = 0; pixelIndex < 8 * 8; pixelIndex++) {
            ushort packedColor = (ushort)(0xF000 | (pixelIndex % opaqueColorCount));
            colors[pixelIndex * 2] = (byte)(packedColor & 255);
            colors[(pixelIndex * 2) + 1] = (byte)(packedColor >> 8);
        }

        return helengine.files.AssetSerializer.SerializeToBytes(new TextureAsset {
            Id = "test-start-a",
            Width = 8,
            Height = 8,
            ColorFormat = TextureAssetColorFormat.Rgba4444,
            AlphaPrecision = TextureAssetAlphaPrecision.A4,
            Colors = colors
        }, numericPolicy);
    }
}
