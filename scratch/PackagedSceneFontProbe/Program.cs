using helengine.editor;
using helengine.ds.builder;
using helengine.baseplatform.Manifest;
using helengine.directx11;
using System.Reflection;

namespace helengine.ds.scratch;

/// <summary>
/// Reads packaged DS scene assets and prints serialized font references or emitted packager font work items.
/// </summary>
public static class Program {
    /// <summary>
    /// Stable probe scene id used by the temporary text-component packaging repro.
    /// </summary>
    const string TemporaryTextSceneId = "Scenes/TextScene.helen";

    /// <summary>
    /// Relative font path used inside the temporary probe project.
    /// </summary>
    const string TemporaryFontRelativePath = "Fonts/DemoDiscBody.ttf";

    /// <summary>
    /// Entry point that inspects packaged scenes or one authored scene-packaging pass.
    /// </summary>
    /// <param name="args">Command-line arguments that select the probe mode.</param>
    public static void Main(string[] args) {
        if (args.Length >= 1 && string.Equals(args[0], "--temp-project-work-items", StringComparison.OrdinalIgnoreCase)) {
            PrintTemporaryProjectWorkItems(args);
            return;
        }

        if (args.Length >= 1 && string.Equals(args[0], "--packager-work-items", StringComparison.OrdinalIgnoreCase)) {
            PrintPackagerWorkItems(args);
            return;
        }

        if (args.Length >= 1 && string.Equals(args[0], "--sprite-info", StringComparison.OrdinalIgnoreCase)) {
            PrintSpriteInfo(args);
            return;
        }

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
    /// Reads one scene asset from disk and prints serialized sprite component sizes plus texture references.
    /// </summary>
    /// <param name="args">Command-line arguments where the second value is the absolute scene path.</param>
    static void PrintSpriteInfo(string[] args) {
        if (args.Length < 2 || string.IsNullOrWhiteSpace(args[1])) {
            throw new ArgumentException("Expected '--sprite-info <scene-path>'.", nameof(args));
        }

        string scenePath = Path.GetFullPath(args[1]);
        if (!File.Exists(scenePath)) {
            throw new FileNotFoundException("Scene asset was not found.", scenePath);
        }

        AutomaticScriptComponentPersistenceDescriptor descriptor = new AutomaticScriptComponentPersistenceDescriptor(new ScriptComponentReflectionSchemaBuilder());
        ProbeSceneAssetReferenceResolver resolver = new ProbeSceneAssetReferenceResolver();
        SceneAsset sceneAsset = ReadSceneAsset(scenePath);
        PrintSpriteComponents(sceneAsset.RootEntities ?? Array.Empty<SceneEntityAsset>(), descriptor, resolver);
    }

    /// <summary>
    /// Packages one authored scene through the real editor packager and prints the emitted font-atlas cook work items.
    /// </summary>
    /// <param name="args">Command-line arguments where the second value is the project root, the third is the scene path, and the fourth is the temporary build root.</param>
    static void PrintPackagerWorkItems(string[] args) {
        if (args.Length < 4) {
            throw new ArgumentException("Expected '--packager-work-items <project-root> <scene-relative-path> <build-root>'.", nameof(args));
        }

        string projectRootPath = Path.GetFullPath(args[1]);
        string sceneRelativePath = args[2].Replace('\\', '/');
        string buildRootPath = Path.GetFullPath(args[3]);

        if (Directory.Exists(buildRootPath)) {
            Directory.Delete(buildRootPath, true);
        }

        Directory.CreateDirectory(buildRootPath);
        ConfigureShaderBackends();

        EditorPlatformBuildScenePackager packager = new(
            projectRootPath,
            LoadEditorHostImporters(),
            NintendoDsPlatformDefinitionFactory.Create());
        EditorPlatformBuildScenePackagerResult result = packager.Package([sceneRelativePath], buildRootPath);

        foreach (PlatformCookWorkItem workItem in result.PlatformCookWorkItems.OrderBy(item => item.OutputRelativePath, StringComparer.OrdinalIgnoreCase)) {
            if (!string.Equals(workItem.SourceAssetKind, "font-atlas-texture", StringComparison.OrdinalIgnoreCase)) {
                continue;
            }

            Console.WriteLine(
                "{0} | {1} | {2}",
                workItem.OutputRelativePath ?? "<null>",
                workItem.SourceAssetKind ?? "<null>",
                workItem.SourceAssetPath ?? "<null>");
        }
    }

    /// <summary>
    /// Creates one temporary project with a single file-backed text component, packages it, and prints emitted font-atlas work items.
    /// </summary>
    /// <param name="args">Command-line arguments where the second value is the source font path, the third is the temporary project root, and the fourth indicates whether ConvertTextToSprite should be enabled.</param>
    static void PrintTemporaryProjectWorkItems(string[] args) {
        if (args.Length < 5) {
            throw new ArgumentException("Expected '--temp-project-work-items <source-font-path> <project-root> <build-root> <convert-text-to-sprite>'.", nameof(args));
        }

        string sourceFontPath = Path.GetFullPath(args[1]);
        string projectRootPath = Path.GetFullPath(args[2]);
        string buildRootPath = Path.GetFullPath(args[3]);
        bool convertTextToSprite = bool.Parse(args[4]);

        if (!File.Exists(sourceFontPath)) {
            throw new FileNotFoundException("Source font file was not found.", sourceFontPath);
        }

        if (Directory.Exists(projectRootPath)) {
            Directory.Delete(projectRootPath, true);
        }
        if (Directory.Exists(buildRootPath)) {
            Directory.Delete(buildRootPath, true);
        }

        Directory.CreateDirectory(projectRootPath);
        Directory.CreateDirectory(buildRootPath);
        WriteTemporaryProjectFont(sourceFontPath, projectRootPath);
        WriteTemporaryProjectScene(projectRootPath, convertTextToSprite);
        ConfigureShaderBackends();

        EditorPlatformBuildScenePackager packager = new(
            projectRootPath,
            LoadEditorHostImporters(),
            NintendoDsPlatformDefinitionFactory.Create());
        EditorPlatformBuildScenePackagerResult result = packager.Package([TemporaryTextSceneId], buildRootPath);

        foreach (PlatformCookWorkItem workItem in result.PlatformCookWorkItems.OrderBy(item => item.OutputRelativePath, StringComparer.OrdinalIgnoreCase)) {
            Console.WriteLine(
                "{0} | {1} | {2}",
                workItem.OutputRelativePath ?? "<null>",
                workItem.SourceAssetKind ?? "<null>",
                workItem.SourceAssetPath ?? "<null>");
        }
    }

    /// <summary>
    /// Configures the minimal built-in shader backend registry required by the editor scene packager.
    /// </summary>
    static void ConfigureShaderBackends() {
        ShaderBackendRegistry shaderBackendRegistry = new ShaderBackendRegistry();
        shaderBackendRegistry.Register(new DirectX11ShaderBackend());
        EditorBuiltInShaderAssetLibrary.ConfigureShaderBackends(shaderBackendRegistry);
    }

    /// <summary>
    /// Loads the editor host's default importer registrations so the scratch probe matches the real packaging path.
    /// </summary>
    /// <returns>Importer registrations published by the editor host.</returns>
    static IReadOnlyList<IAssetImporterRegistration> LoadEditorHostImporters() {
        string appAssemblyPath = @"C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\bin\Debug\net9.0-windows\helengine.editor.app.dll";
        Assembly appAssembly = Assembly.LoadFrom(appAssemblyPath);
        Type importerFactoryType = appAssembly.GetType("helengine.editor.app.EditorHostImporterFactory", throwOnError: true)
            ?? throw new InvalidOperationException("EditorHostImporterFactory type was not found.");
        MethodInfo createDefaultMethod = importerFactoryType.GetMethod(
            "CreateDefault",
            BindingFlags.Public | BindingFlags.Static)
            ?? throw new InvalidOperationException("Editor host importer factory did not expose CreateDefault.");
        object result = createDefaultMethod.Invoke(null, null)
            ?? throw new InvalidOperationException("Editor host importer factory returned null.");
        return (IReadOnlyList<IAssetImporterRegistration>)result;
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
    /// Copies the authored source font into the temporary probe project.
    /// </summary>
    /// <param name="sourceFontPath">Absolute path to the source font file that should be copied.</param>
    /// <param name="projectRootPath">Temporary project root that should receive the copied font.</param>
    static void WriteTemporaryProjectFont(string sourceFontPath, string projectRootPath) {
        string fontFullPath = Path.Combine(projectRootPath, "assets", TemporaryFontRelativePath.Replace('/', Path.DirectorySeparatorChar));
        string fontDirectoryPath = Path.GetDirectoryName(fontFullPath);
        if (string.IsNullOrWhiteSpace(fontDirectoryPath)) {
            throw new InvalidOperationException("Temporary font directory path could not be resolved.");
        }

        Directory.CreateDirectory(fontDirectoryPath);
        File.Copy(sourceFontPath, fontFullPath, true);
    }

    /// <summary>
    /// Writes one minimal authored scene that contains a single text component with a file-backed font reference.
    /// </summary>
    /// <param name="projectRootPath">Temporary project root that should receive the scene asset.</param>
    /// <param name="convertTextToSprite">True when the authored text should request build-time sprite conversion.</param>
    static void WriteTemporaryProjectScene(string projectRootPath, bool convertTextToSprite) {
        string sceneFullPath = Path.Combine(projectRootPath, "assets", TemporaryTextSceneId.Replace('/', Path.DirectorySeparatorChar));
        string sceneDirectoryPath = Path.GetDirectoryName(sceneFullPath);
        if (string.IsNullOrWhiteSpace(sceneDirectoryPath)) {
            throw new InvalidOperationException("Temporary scene directory path could not be resolved.");
        }

        Directory.CreateDirectory(sceneDirectoryPath);

        SceneAssetReference fontReference = CreateFileFontReference(TemporaryFontRelativePath);
        SceneAsset sceneAsset = new SceneAsset {
            Id = TemporaryTextSceneId,
            AssetReferences = [fontReference],
            RootEntities = [
                new SceneEntityAsset {
                    Id = 1u,
                    Name = "Root",
                    LocalPosition = float3.Zero,
                    LocalScale = float3.One,
                    LocalOrientation = float4.Identity,
                    Components = [
                        new SceneComponentAssetRecord {
                            ComponentTypeId = "helengine.TextComponent",
                            ComponentIndex = 0,
                            Payload = CreateTextComponentPayload(fontReference, convertTextToSprite)
                        }
                    ],
                    Children = Array.Empty<SceneEntityAsset>()
                }
            ]
        };

        using FileStream stream = new FileStream(sceneFullPath, FileMode.Create, FileAccess.Write, FileShare.None);
        global::helengine.editor.AssetSerializer.Serialize(stream, sceneAsset);
    }

    /// <summary>
    /// Creates one serialized text-component payload that mirrors authored scene text records with a file-backed font reference.
    /// </summary>
    /// <param name="fontReference">File-backed font reference stored in the component save state.</param>
    /// <param name="convertTextToSprite">True when the authored text should request build-time sprite conversion.</param>
    /// <returns>Serialized automatic reflected text-component payload.</returns>
    static byte[] CreateTextComponentPayload(SceneAssetReference fontReference, bool convertTextToSprite) {
        if (fontReference == null) {
            throw new ArgumentNullException(nameof(fontReference));
        }

        AutomaticScriptComponentPersistenceDescriptor descriptor = new AutomaticScriptComponentPersistenceDescriptor(new ScriptComponentReflectionSchemaBuilder());
        TextComponent textComponent = new TextComponent {
            Font = CreatePackagedFontAsset(),
            Text = "Hello world",
            WrapText = true,
            Size = new int2(320, 64),
            Color = new byte4(12, 34, 56, 255),
            SourceRect = new float4(0f, 0f, 1f, 1f),
            Rotation = 0.25f,
            FontScale = 2f,
            RenderOrder2D = 19,
            LayerMask = 7,
            SelectionEnabled = true,
            ConvertTextToSprite = convertTextToSprite,
            Alignment = TextAlignment.Center
        };
        EntityComponentSaveState saveState = new EntityComponentSaveState();
        saveState.SetAssetReference(nameof(TextComponent.Font), fontReference);

        SceneComponentAssetRecord record = descriptor.SerializeComponent(textComponent, 0, saveState);
        return record.Payload;
    }

    /// <summary>
    /// Creates one file-backed font reference for the temporary probe project.
    /// </summary>
    /// <param name="relativePath">Project-relative font path stored by the scene reference.</param>
    /// <returns>File-backed scene font reference.</returns>
    static SceneAssetReference CreateFileFontReference(string relativePath) {
        return SceneAssetReferenceFactory.CreateFileSystemFont(relativePath);
    }

    /// <summary>
    /// Creates one minimal packaged font asset used only to satisfy automatic text-component serialization.
    /// </summary>
    /// <returns>Minimal packaged font asset with an embedded source texture asset.</returns>
    static FontAsset CreatePackagedFontAsset() {
        return new FontAsset(
            new FontInfo("Demo", 16, 8f),
            null,
            new Dictionary<char, FontChar>(),
            16f,
            64,
            64) {
                SourceTextureAsset = new TextureAsset {
                    Id = "fonts/demo-source",
                    Width = 64,
                    Height = 64,
                    ColorFormat = TextureAssetColorFormat.Rgba32,
                    AlphaPrecision = TextureAssetAlphaPrecision.A8,
                    Colors = new byte[64 * 64 * 4]
                }
            };
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
    /// Walks the supplied entity hierarchy and prints serialized sprite component details for every sprite encountered.
    /// </summary>
    /// <param name="entities">Entities to inspect.</param>
    /// <param name="descriptor">Automatic-script persistence descriptor used to deserialize sprite components.</param>
    /// <param name="resolver">Resolver that provides placeholder runtime assets while preserving serialized references in save state.</param>
    static void PrintSpriteComponents(
        SceneEntityAsset[] entities,
        AutomaticScriptComponentPersistenceDescriptor descriptor,
        ProbeSceneAssetReferenceResolver resolver) {
        foreach (SceneEntityAsset entity in entities) {
            if (entity == null) {
                continue;
            }

            PrintEntitySpriteComponents(entity, descriptor, resolver);
            PrintSpriteComponents(entity.Children ?? Array.Empty<SceneEntityAsset>(), descriptor, resolver);
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
    /// Prints serialized sprite component details for the supported component types on one entity.
    /// </summary>
    /// <param name="entity">Entity to inspect.</param>
    /// <param name="descriptor">Automatic-script persistence descriptor used to deserialize sprite components.</param>
    /// <param name="resolver">Resolver that provides placeholder runtime assets while preserving serialized references in save state.</param>
    static void PrintEntitySpriteComponents(
        SceneEntityAsset entity,
        AutomaticScriptComponentPersistenceDescriptor descriptor,
        ProbeSceneAssetReferenceResolver resolver) {
        SceneComponentAssetRecord[] components = entity.Components ?? Array.Empty<SceneComponentAssetRecord>();
        for (int index = 0; index < components.Length; index++) {
            SceneComponentAssetRecord componentRecord = components[index];
            if (componentRecord == null) {
                continue;
            }

            if (!string.Equals(componentRecord.ComponentTypeId, "helengine.SpriteComponent", StringComparison.Ordinal)) {
                continue;
            }

            PrintSpriteComponentInfo(entity.Name, componentRecord, descriptor, resolver);
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

    /// <summary>
    /// Deserializes one sprite component record and prints its serialized size plus preserved texture reference.
    /// </summary>
    /// <param name="entityName">Owning entity name used for diagnostics.</param>
    /// <param name="componentRecord">Serialized component record to inspect.</param>
    /// <param name="descriptor">Automatic-script persistence descriptor used to deserialize the record.</param>
    /// <param name="resolver">Resolver that provides placeholder runtime assets while preserving serialized references in save state.</param>
    static void PrintSpriteComponentInfo(
        string entityName,
        SceneComponentAssetRecord componentRecord,
        AutomaticScriptComponentPersistenceDescriptor descriptor,
        ProbeSceneAssetReferenceResolver resolver) {
        EntitySaveComponent saveComponent = new EntitySaveComponent();
        Component component = descriptor.DeserializeComponent(componentRecord, saveComponent, resolver);
        if (component is not SpriteComponent spriteComponent) {
            return;
        }

        string textureReferenceText = "<none>";
        if (saveComponent.TryGetComponentState(spriteComponent, out EntityComponentSaveState componentState)
            && componentState.TryGetAssetReference("Texture", out SceneAssetReference textureReference)) {
            textureReferenceText = string.Join(
                "|",
                textureReference.ProviderId ?? "<null>",
                textureReference.AssetId ?? "<null>",
                textureReference.RelativePath ?? "<null>");
        }

        Console.WriteLine(
            "Entity={0} Size={1}x{2} RenderOrder={3} TextureRef={4}",
            entityName,
            spriteComponent.Size.X,
            spriteComponent.Size.Y,
            spriteComponent.RenderOrder2D,
            textureReferenceText);
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
        return new ProbeRuntimeTexture();
    }

    /// <summary>
    /// Resolves one animation clip asset reference.
    /// </summary>
    /// <param name="reference">Serialized animation clip reference.</param>
    /// <returns>Placeholder animation clip asset.</returns>
    public AnimationClipAsset ResolveAnimationClip(SceneAssetReference reference) {
        return new AnimationClipAsset();
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

/// <summary>
/// Minimal runtime texture used only to satisfy automatic sprite-component deserialization inside the scene probe.
/// </summary>
sealed class ProbeRuntimeTexture : RuntimeTexture {
}
