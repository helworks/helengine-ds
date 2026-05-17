using helengine;
using helengine.files;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies Nintendo DS scene-asset sanitization rules applied before native packaging.
/// </summary>
public class NintendoDsSceneAssetSanitizerTests {
    /// <summary>
    /// Verifies the sanitizer preserves the demo-disc return-to-menu runtime component so packaged gameplay scenes can navigate back to the main menu.
    /// </summary>
    [Fact]
    public void SanitizeStagedSceneAssets_preserves_demo_disc_return_to_menu_component() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-scene-sanitizer-" + Guid.NewGuid().ToString("N"));
        string nitroFsRootPath = Path.Combine(rootPath, "nitrofs");
        string scenePath = Path.Combine(nitroFsRootPath, "cooked", "scenes", "rendering", "cube_test.hasset");

        try {
            Directory.CreateDirectory(Path.GetDirectoryName(scenePath)
                ?? throw new InvalidOperationException("Unable to resolve the staged scene directory path."));
            File.WriteAllBytes(scenePath, BuildSceneAssetBytes());

            NintendoDsSceneAssetSanitizer sanitizer = new();
            sanitizer.SanitizeStagedSceneAssets(nitroFsRootPath);

            SceneAsset sanitizedSceneAsset = Assert.IsType<SceneAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(scenePath)));
            SceneEntityAsset rootEntity = Assert.Single(sanitizedSceneAsset.RootEntities);
            Assert.Contains(
                rootEntity.Components,
                component => string.Equals(component.ComponentTypeId, "city.menu.DemoDiscReturnToMenuComponent, gameplay", StringComparison.Ordinal));
            Assert.Contains(
                rootEntity.Components,
                component => string.Equals(component.ComponentTypeId, "helengine.MeshComponent", StringComparison.Ordinal));
            Assert.Contains(
                rootEntity.Components,
                component => string.Equals(component.ComponentTypeId, "helengine.DirectionalShadowTowerSpinComponent", StringComparison.Ordinal));
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the sanitizer preserves authored imported-texture dimensions instead of acting as the primary DS texture downscaler.
    /// </summary>
    [Fact]
    public void SanitizeStagedSceneAssets_preserves_imported_texture_dimensions() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-scene-sanitizer-" + Guid.NewGuid().ToString("N"));
        string nitroFsRootPath = Path.Combine(rootPath, "nitrofs");
        string importedRootPath = Path.Combine(nitroFsRootPath, "cooked", "imported");
        string oversizedTexturePath = Path.Combine(importedRootPath, "oversized-logo.hasset");
        string smallTexturePath = Path.Combine(importedRootPath, "small-icon.hasset");

        try {
            Directory.CreateDirectory(importedRootPath);
            File.WriteAllBytes(oversizedTexturePath, helengine.files.AssetSerializer.SerializeToBytes(CreateTextureAsset(512, 512)));
            File.WriteAllBytes(smallTexturePath, helengine.files.AssetSerializer.SerializeToBytes(CreateTextureAsset(64, 64)));

            NintendoDsSceneAssetSanitizer sanitizer = new();
            sanitizer.SanitizeStagedSceneAssets(nitroFsRootPath);

            TextureAsset sanitizedOversizedTexture = Assert.IsType<TextureAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(oversizedTexturePath)));
            Assert.Equal((ushort)512, sanitizedOversizedTexture.Width);
            Assert.Equal((ushort)512, sanitizedOversizedTexture.Height);
            Assert.Equal(512 * 512 * 4, sanitizedOversizedTexture.Colors.Length);

            TextureAsset sanitizedSmallTexture = Assert.IsType<TextureAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(smallTexturePath)));
            Assert.Equal((ushort)64, sanitizedSmallTexture.Width);
            Assert.Equal((ushort)64, sanitizedSmallTexture.Height);
            Assert.Equal(64 * 64 * 4, sanitizedSmallTexture.Colors.Length);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the Nintendo DS scene sanitizer no longer downsizes cooked imported textures as the normal DS texture-policy path.
    /// </summary>
    [Fact]
    public void SanitizeStagedSceneAssets_whenImportedTextureExceedsOldClamp_preservesAuthoredDimensions() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-scene-sanitizer-" + Guid.NewGuid().ToString("N"));
        string nitroFsRootPath = Path.Combine(rootPath, "nitrofs");
        string importedRootPath = Path.Combine(nitroFsRootPath, "cooked", "imported");
        string texturePath = Path.Combine(importedRootPath, "authored-font-atlas.hasset");

        try {
            Directory.CreateDirectory(importedRootPath);
            File.WriteAllBytes(texturePath, helengine.files.AssetSerializer.SerializeToBytes(CreateTextureAsset(256, 256)));

            new NintendoDsSceneAssetSanitizer().SanitizeStagedSceneAssets(nitroFsRootPath);

            TextureAsset sanitizedTexture = Assert.IsType<TextureAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(texturePath)));
            Assert.Equal((ushort)256, sanitizedTexture.Width);
            Assert.Equal((ushort)256, sanitizedTexture.Height);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Builds one serialized scene asset that includes the unsupported return-to-menu component and supported cube-scene records.
    /// </summary>
    /// <returns>Serialized scene asset bytes.</returns>
    static byte[] BuildSceneAssetBytes() {
        SceneAsset sceneAsset = new SceneAsset {
            Id = "scenes/rendering/cube_test.helen",
            RootEntities = [
                new SceneEntityAsset {
                    Id = 1,
                    Name = "CubeRoot",
                    LocalPosition = float3.Zero,
                    LocalScale = float3.One,
                    LocalOrientation = float4.Identity,
                    Components = [
                        new SceneComponentAssetRecord {
                            ComponentTypeId = "city.menu.DemoDiscReturnToMenuComponent, gameplay",
                            ComponentIndex = 0,
                            Payload = []
                        },
                        new SceneComponentAssetRecord {
                            ComponentTypeId = "helengine.MeshComponent",
                            ComponentIndex = 1,
                            Payload = [1, 2, 3]
                        },
                        new SceneComponentAssetRecord {
                            ComponentTypeId = "helengine.DirectionalShadowTowerSpinComponent",
                            ComponentIndex = 2,
                            Payload = [4, 5, 6]
                        }
                    ],
                    Children = Array.Empty<SceneEntityAsset>()
                }
            ]
        };
        return helengine.files.AssetSerializer.SerializeToBytes(sceneAsset);
    }

    /// <summary>
    /// Creates one deterministic texture asset payload for imported-texture sanitization tests.
    /// </summary>
    /// <param name="width">Authored texture width.</param>
    /// <param name="height">Authored texture height.</param>
    /// <returns>Texture asset with opaque deterministic RGBA pixels.</returns>
    static TextureAsset CreateTextureAsset(ushort width, ushort height) {
        byte[] colors = new byte[width * height * 4];
        for (int index = 0; index < colors.Length; index += 4) {
            colors[index] = 10;
            colors[index + 1] = 20;
            colors[index + 2] = 30;
            colors[index + 3] = 255;
        }

        return new TextureAsset {
            Id = "test-texture",
            Width = width,
            Height = height,
            Colors = colors
        };
    }
}
