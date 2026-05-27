using System.Text;

namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the city-owned authored physics scene generation sources and generated scene assets.
/// </summary>
public sealed class CityPhysicsSceneOwnershipAuditTests {
    /// <summary>
    /// Absolute path of the city project used by the local physics-scene integration workflow.
    /// </summary>
    const string CityProjectRootPath = @"C:\dev\helprojs\city";

    /// <summary>
    /// Verifies the city project owns the authored physics scene generator sources instead of delegating scene authoring to helengine.
    /// </summary>
    [Fact]
    public void Sources_whenCityOwnsPhysicsSceneGeneration_includeCityPhysicsToolsAndCommands() {
        string demoMenuItemProviderSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "DemoMenuItemProvider.cs"));

        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "PhysicsSceneCatalog.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "PhysicsSceneFactory.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "PhysicsSceneGenerator.cs")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "code.module.json")));
        Assert.True(File.Exists(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "GeneratePhysicsScenesCommand.cs")));

        string physicsSceneCatalogSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "PhysicsSceneCatalog.cs"));
        string physicsSceneFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "PhysicsSceneFactory.cs"));
        string physicsSceneGeneratorSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "PhysicsSceneGenerator.cs"));
        string physicsToolsModuleSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "physics.tools", "code.module.json"));
        string generatePhysicsScenesCommandSource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "menu.tools", "GeneratePhysicsScenesCommand.cs"));

        Assert.Contains("public static class PhysicsSceneCatalog", physicsSceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("test_scene_dynamic_stack_boxes.helen", physicsSceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("test_scene_dynamic_sphere_stack.helen", physicsSceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("test_scene_dynamic_mixed_stack.helen", physicsSceneCatalogSource, StringComparison.Ordinal);
        Assert.Contains("public sealed class PhysicsSceneFactory", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("public SceneAsset CreateSceneAsset(string sceneId)", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("public void WriteScenes(string projectRootPath)", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("PhysicsValidationSceneFactory", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("PhysicsValidationSceneCatalog", physicsSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("public sealed class PhysicsSceneGenerator", physicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("PhysicsSceneFactory factory = new PhysicsSceneFactory();", physicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("factory.WriteScenes(projectRootPath);", physicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.DoesNotContain("PhysicsValidationSceneFactory", physicsSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("\"moduleId\": \"physics.tools\"", physicsToolsModuleSource, StringComparison.Ordinal);
        Assert.Contains("\"moduleKind\": \"editor\"", physicsToolsModuleSource, StringComparison.Ordinal);
        Assert.Contains("public sealed class GeneratePhysicsScenesCommand", generatePhysicsScenesCommandSource, StringComparison.Ordinal);
        Assert.Contains("menu.generate-physics-scenes", generatePhysicsScenesCommandSource, StringComparison.Ordinal);
        Assert.Contains("PhysicsSceneGenerator generator = new PhysicsSceneGenerator();", generatePhysicsScenesCommandSource, StringComparison.Ordinal);
        Assert.Contains("Generate Physics Scenes", demoMenuItemProviderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the committed city stacked-boxes authored physics scene points its camera toward the box stack volume.
    /// </summary>
    [Fact]
    public void Asset_whenDynamicStackBoxesSceneIsAuthored_cameraFacesStackVolume() {
        SceneAsset sceneAsset = ReadSceneAsset("test_scene_dynamic_stack_boxes.helen");
        SceneEntityAsset cameraEntity = FindRootEntity(sceneAsset, "Camera");
        SceneEntityAsset scenarioEntity = FindRootEntity(sceneAsset, "Scenario");
        SceneEntityAsset firstStackBoxEntity = FindChildEntity(scenarioEntity, "StackBox01");
        SceneEntityAsset fourthStackBoxEntity = FindChildEntity(scenarioEntity, "StackBox04");
        float3 stackCenter = (firstStackBoxEntity.LocalPosition + fourthStackBoxEntity.LocalPosition) * 0.5f;
        float3 cameraForward = float4.RotateVector(new float3(0f, 0f, -1f), cameraEntity.LocalOrientation);
        float3 cameraToStack = float3.Normalize(stackCenter - cameraEntity.LocalPosition);
        float facingDot = float3.Dot(cameraForward, cameraToStack);

        Assert.True(facingDot > 0.9f, $"The stacked-boxes camera should face the stack volume, but dot={facingDot}.");
    }

    /// <summary>
    /// Reads one authored city physics scene asset from the committed project scene folder.
    /// </summary>
    /// <param name="sceneFileName">Committed authored physics scene file name.</param>
    /// <returns>Deserialized scene asset.</returns>
    static SceneAsset ReadSceneAsset(string sceneFileName) {
        string scenePath = Path.Combine(CityProjectRootPath, "assets", "scenes", "physics", sceneFileName);
        Assert.True(File.Exists(scenePath));

        byte[] sceneBytes = File.ReadAllBytes(scenePath);
        return Assert.IsType<SceneAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(sceneBytes));
    }

    /// <summary>
    /// Finds one root scene entity with the supplied stable name.
    /// </summary>
    /// <param name="sceneAsset">Scene asset being inspected.</param>
    /// <param name="entityName">Stable entity name to resolve.</param>
    /// <returns>Resolved root scene entity.</returns>
    static SceneEntityAsset FindRootEntity(SceneAsset sceneAsset, string entityName) {
        if (sceneAsset == null) {
            throw new ArgumentNullException(nameof(sceneAsset));
        } else if (string.IsNullOrWhiteSpace(entityName)) {
            throw new ArgumentException("Entity name must be provided.", nameof(entityName));
        }

        SceneEntityAsset rootEntity = Assert.Single(
            sceneAsset.RootEntities,
            entity => string.Equals(entity.Name, entityName, StringComparison.Ordinal));
        return rootEntity;
    }

    /// <summary>
    /// Finds one direct child entity with the supplied stable name.
    /// </summary>
    /// <param name="parentEntity">Parent entity that owns the child.</param>
    /// <param name="entityName">Stable child entity name to resolve.</param>
    /// <returns>Resolved child entity.</returns>
    static SceneEntityAsset FindChildEntity(SceneEntityAsset parentEntity, string entityName) {
        if (parentEntity == null) {
            throw new ArgumentNullException(nameof(parentEntity));
        } else if (string.IsNullOrWhiteSpace(entityName)) {
            throw new ArgumentException("Entity name must be provided.", nameof(entityName));
        }

        SceneEntityAsset childEntity = Assert.Single(
            parentEntity.Children,
            entity => string.Equals(entity.Name, entityName, StringComparison.Ordinal));
        return childEntity;
    }
}
