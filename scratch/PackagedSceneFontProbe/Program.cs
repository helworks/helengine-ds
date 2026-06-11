using helengine.editor;

namespace helengine.ds.scratch;

/// <summary>
/// Reads packaged DS scene assets and prints the serialized font references used by bottom-screen text components.
/// </summary>
public static class Program {
    /// <summary>
    /// Entry point that inspects every packaged scene under the supplied build workspace.
    /// </summary>
    /// <param name="args">Command-line arguments where the first value is the DS build workspace root.</param>
    public static void Main(string[] args) {
        if (args.Length < 1 || string.IsNullOrWhiteSpace(args[0])) {
            throw new ArgumentException("Expected the DS build workspace root path as the first argument.", nameof(args));
        }

        string buildRootPath = Path.GetFullPath(args[0]);
        string scenesRootPath = Path.Combine(buildRootPath, "package", "cooked", "scenes");
        if (!Directory.Exists(scenesRootPath)) {
            throw new DirectoryNotFoundException("Packaged scenes directory was not found: " + scenesRootPath);
        }

        AutomaticScriptComponentPersistenceDescriptor descriptor = new AutomaticScriptComponentPersistenceDescriptor(new ScriptComponentReflectionSchemaBuilder());
        ProbeSceneAssetReferenceResolver resolver = new ProbeSceneAssetReferenceResolver();
        foreach (string scenePath in Directory.GetFiles(scenesRootPath, "*.hasset", SearchOption.AllDirectories).OrderBy(path => path, StringComparer.OrdinalIgnoreCase)) {
            SceneAsset sceneAsset = ReadSceneAsset(scenePath);
            Console.WriteLine("Scene={0}", Path.GetRelativePath(scenesRootPath, scenePath).Replace('\\', '/'));
            PrintFontReferences(sceneAsset.RootEntities ?? Array.Empty<SceneEntityAsset>(), descriptor, resolver);
        }
    }

    /// <summary>
    /// Reads one packaged scene asset from disk.
    /// </summary>
    /// <param name="scenePath">Absolute path to the packaged scene asset.</param>
    /// <returns>Deserialized scene asset.</returns>
    static SceneAsset ReadSceneAsset(string scenePath) {
        byte[] sceneBytes = File.ReadAllBytes(scenePath);
        return (SceneAsset)helengine.files.AssetSerializer.DeserializeFromBytes(sceneBytes);
    }

    /// <summary>
    /// Walks the supplied entity hierarchy and prints serialized font references for every debug or text component encountered.
    /// </summary>
    /// <param name="entities">Entities to inspect.</param>
    /// <param name="descriptor">Automatic-script persistence descriptor used to deserialize text components.</param>
    /// <param name="resolver">Resolver that provides placeholder runtime assets while preserving serialized references in save state.</param>
    static void PrintFontReferences(
        SceneEntityAsset[] entities,
        AutomaticScriptComponentPersistenceDescriptor descriptor,
        ProbeSceneAssetReferenceResolver resolver) {
        foreach (SceneEntityAsset entity in entities) {
            if (entity == null) {
                continue;
            }

            PrintEntityFontReferences(entity, descriptor, resolver);
            PrintFontReferences(entity.Children ?? Array.Empty<SceneEntityAsset>(), descriptor, resolver);
        }
    }

    /// <summary>
    /// Prints serialized font references for the supported component types on one entity.
    /// </summary>
    /// <param name="entity">Entity to inspect.</param>
    /// <param name="descriptor">Automatic-script persistence descriptor used to deserialize text components.</param>
    /// <param name="resolver">Resolver that provides placeholder runtime assets while preserving serialized references in save state.</param>
    static void PrintEntityFontReferences(
        SceneEntityAsset entity,
        AutomaticScriptComponentPersistenceDescriptor descriptor,
        ProbeSceneAssetReferenceResolver resolver) {
        SceneComponentAssetRecord[] components = entity.Components ?? Array.Empty<SceneComponentAssetRecord>();
        for (int index = 0; index < components.Length; index++) {
            SceneComponentAssetRecord componentRecord = components[index];
            if (componentRecord == null) {
                continue;
            }

            if (string.Equals(componentRecord.ComponentTypeId, "helengine.DebugComponent", StringComparison.Ordinal)) {
                PrintComponentFontReference(entity.Name, componentRecord, descriptor, resolver);
                continue;
            }

            if (string.Equals(componentRecord.ComponentTypeId, "helengine.TextComponent", StringComparison.Ordinal)) {
                PrintComponentFontReference(entity.Name, componentRecord, descriptor, resolver);
            }
        }
    }

    /// <summary>
    /// Deserializes one component record and prints its preserved serialized font reference.
    /// </summary>
    /// <param name="entityName">Owning entity name used for diagnostics.</param>
    /// <param name="componentRecord">Serialized component record to inspect.</param>
    /// <param name="descriptor">Automatic-script persistence descriptor used to deserialize the record.</param>
    /// <param name="resolver">Resolver that provides placeholder runtime assets while preserving serialized references in save state.</param>
    static void PrintComponentFontReference(
        string entityName,
        SceneComponentAssetRecord componentRecord,
        AutomaticScriptComponentPersistenceDescriptor descriptor,
        ProbeSceneAssetReferenceResolver resolver) {
        EntitySaveComponent saveComponent = new EntitySaveComponent();
        Component component = descriptor.DeserializeComponent(componentRecord, saveComponent, resolver);
        if (component == null) {
            return;
        }

        if (!saveComponent.TryGetComponentState(component, out EntityComponentSaveState componentState)) {
            Console.WriteLine("  Entity={0} Component={1} FontRef=<missing-save-state>", entityName, componentRecord.ComponentTypeId);
            return;
        }

        if (!componentState.TryGetAssetReference("Font", out SceneAssetReference fontReference)) {
            Console.WriteLine("  Entity={0} Component={1} FontRef=<none>", entityName, componentRecord.ComponentTypeId);
            return;
        }

        Console.WriteLine(
            "  Entity={0} Component={1} FontRef={2}|{3}|{4}",
            entityName,
            componentRecord.ComponentTypeId,
            fontReference.ProviderId ?? "<null>",
            fontReference.AssetId ?? "<null>",
            fontReference.RelativePath ?? "<null>");
    }
}

/// <summary>
/// Minimal scene-asset resolver that returns placeholder runtime assets while preserving serialized font references in component save state.
/// </summary>
sealed class ProbeSceneAssetReferenceResolver : ISceneAssetReferenceResolver {
    /// <summary>
    /// Resolves one font asset reference into a placeholder runtime font so automatic component deserialization can proceed.
    /// </summary>
    /// <param name="fontReference">Serialized font reference being resolved.</param>
    /// <returns>Placeholder runtime font asset.</returns>
    public FontAsset ResolveFont(SceneAssetReference fontReference) {
        FontInfo fontInfo = new FontInfo(fontReference?.AssetId ?? "probe-font", 8, 4f);
        return new FontAsset(fontInfo, null, new Dictionary<char, FontChar>(), 8f, 8, 8);
    }

    /// <summary>
    /// Resolves one texture asset reference.
    /// </summary>
    /// <param name="textureReference">Serialized texture reference.</param>
    /// <returns>This probe does not resolve textures.</returns>
    public RuntimeTexture ResolveTexture(SceneAssetReference textureReference) {
        throw new InvalidOperationException("Texture resolution is not required by the packaged scene font probe.");
    }

    /// <summary>
    /// Resolves one mesh asset reference.
    /// </summary>
    /// <param name="meshReference">Serialized mesh reference.</param>
    /// <returns>This probe does not resolve meshes.</returns>
    public RuntimeMaterial ResolveMaterial(SceneAssetReference materialReference) {
        throw new InvalidOperationException("Material resolution is not required by the packaged scene font probe.");
    }

    /// <summary>
    /// Resolves one model asset reference.
    /// </summary>
    /// <param name="reference">Serialized model reference.</param>
    /// <returns>This probe does not resolve models.</returns>
    public RuntimeModel ResolveModel(SceneAssetReference reference) {
        throw new InvalidOperationException("Model resolution is not required by the packaged scene font probe.");
    }
}
