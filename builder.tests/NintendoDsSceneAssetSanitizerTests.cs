using helengine;
using helengine.files;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies Nintendo DS scene-asset sanitization rules applied before native packaging.
/// </summary>
public class NintendoDsSceneAssetSanitizerTests {
    /// <summary>
    /// Verifies the sanitizer removes the unsupported demo-disc return-to-menu runtime component while preserving supported scene content.
    /// </summary>
    [Fact]
    public void SanitizeStagedSceneAssets_removes_unsupported_demo_disc_return_to_menu_component() {
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
            Assert.DoesNotContain(
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
    /// Builds one serialized scene asset that includes the unsupported return-to-menu component and supported cube-scene records.
    /// </summary>
    /// <returns>Serialized scene asset bytes.</returns>
    static byte[] BuildSceneAssetBytes() {
        SceneAsset sceneAsset = new SceneAsset {
            Id = "scenes/rendering/cube_test.helen",
            RootEntities = [
                new SceneEntityAsset {
                    Id = "cube-root",
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
}
