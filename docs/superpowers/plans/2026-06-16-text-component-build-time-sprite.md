# TextComponent Build-Time Sprite Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add an authored `TextComponent` flag that keeps text editing intact in the editor but packages flagged text as a cooked `SpriteComponent` plus generated texture at build time.

**Architecture:** Extend `TextComponent` with a build-time conversion flag, then route flagged text records through a dedicated bake service from `SceneComponentPackagingTransformService`. The bake service will render the authored text into a transient texture, the packager will feed that texture through the normal texture cook pipeline, and the packaged scene will emit only `SpriteComponent` payloads for converted text.

**Tech Stack:** C#, helengine editor scene packaging, `SceneComponentPackagingTransformService`, `EditorExact2DPreviewCaptureService`, texture cook work items, xUnit.

---

## File Structure

- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\TextComponent.cs`
  - Add the authored `ConvertTextToSprite` flag and default it to `false`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
  - Detect flagged text, call the bake service, emit sprite payloads, and package generated texture assets.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs`
  - Construct and pass the bake service into the transform layer.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformCookWorkItemFactory.cs`
  - Add generated-texture cook work item support mirroring the existing generated font-atlas path.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\ITextComponentSpriteBakeService.cs`
  - Bake-service contract used by packaging.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeRequest.cs`
  - Immutable bake inputs derived from the authored text record.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeResult.cs`
  - Generated texture asset plus metadata needed by packaging.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeService.cs`
  - Real bake implementation that uses exact text preview capture and readback.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\rendering\IRenderTargetTextureAssetReader.cs`
  - Rendering abstraction that converts one render target into a raw `TextureAsset`.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\rendering\DirectX11RenderTargetTextureAssetReader.cs`
  - Windows editor implementation for render-target readback.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs`
  - Focused transform tests using a fake bake service.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\TextComponentSpriteBakeServiceTests.cs`
  - Bake-service tests using test render managers and a fake render-target reader.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\AutomaticScriptComponentPersistenceDescriptorTests.cs`
  - Persist and restore the new authored flag.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneSaveServiceTests.cs`
  - Verify the tagged payload includes the new member.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`
  - End-to-end package tests proving flagged text becomes sprite output with a generated texture.

### Task 1: Add the Authored Flag and Persistence Coverage

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\components\2d\TextComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\AutomaticScriptComponentPersistenceDescriptorTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneSaveServiceTests.cs`

- [ ] **Step 1: Write the failing persistence tests**

```csharp
[Fact]
public void SerializeAndDeserialize_WhenTextComponentUsesBuildTimeSpriteConversion_RoundTripsThroughAutomaticPersistence() {
    AutomaticScriptComponentPersistenceDescriptor descriptor = new AutomaticScriptComponentPersistenceDescriptor(new ScriptComponentReflectionSchemaBuilder());
    SceneAssetReference fontReference = CreateFileReference("Fonts/Demo.ttf");
    EntityComponentSaveState saveState = new EntityComponentSaveState();

    TextComponent component = new TextComponent {
        Text = "Bake me",
        Size = new int2(128, 32),
        ConvertTextToSprite = true
    };

    saveState.SetAssetReference(nameof(TextComponent.Font), fontReference);
    SceneComponentAssetRecord record = descriptor.SerializeComponent(component, 0, saveState);

    EntitySaveComponent loadedSaveComponent = new EntitySaveComponent();
    TextComponent restored = Assert.IsType<TextComponent>(descriptor.DeserializeComponent(record, loadedSaveComponent, new TestSceneAssetReferenceResolver()));

    Assert.True(restored.ConvertTextToSprite);
}

[Fact]
public void Save_WhenSceneContainsTextComponent_WritesTaggedPayloadUsingConvertTextToSpriteMember() {
    TextComponent textComponent = new TextComponent {
        Text = "Tagged",
        Size = new int2(96, 24),
        ConvertTextToSprite = true
    };

    SceneAsset savedScene = SaveSingleComponentScene(textComponent, CreateFileReference("Fonts/Demo.ttf"));
    SceneComponentAssetRecord record = Assert.Single(savedScene.RootEntities[0].Components);
    EditorTaggedSceneComponentFieldReader reader = new EditorTaggedSceneComponentFieldReader(record.Payload);

    Assert.True(reader.TryGetFieldReader(nameof(TextComponent.ConvertTextToSprite), out EngineBinaryReader fieldReader));
    using (fieldReader) {
        Assert.Equal((byte)1, fieldReader.ReadByte());
    }
}
```

- [ ] **Step 2: Run the persistence tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AutomaticScriptComponentPersistenceDescriptorTests|FullyQualifiedName~SceneSaveServiceTests" -v minimal`

Expected: FAIL because `TextComponent` does not yet expose `ConvertTextToSprite`.

- [ ] **Step 3: Add the authored flag to `TextComponent`**

```csharp
/// <summary>
/// Gets or sets a value indicating whether scene packaging should bake this authored text into a sprite-backed runtime component.
/// </summary>
public bool ConvertTextToSprite { get; set; }

public TextComponent() {
    TextValue = "";
    SelectionAnchorPositionValue = 0;
    CursorPositionValue = 0;
    Color = new byte4(255, 255, 255, 255);
    SourceRect = new float4(0, 0, 1, 1);
    WrapText = false;
    FontScaleValue = 1f;
    Alignment = TextAlignment.Left;
    ConvertTextToSprite = false;
}
```

- [ ] **Step 4: Run the persistence tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AutomaticScriptComponentPersistenceDescriptorTests|FullyQualifiedName~SceneSaveServiceTests" -v minimal`

Expected: PASS, with the new member serialized and restored through the existing reflected scene path.

- [ ] **Step 5: Commit**

```bash
rtk git add C:\dev\helworks\helengine\engine\helengine.core\components\2d\TextComponent.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\AutomaticScriptComponentPersistenceDescriptorTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneSaveServiceTests.cs
rtk git commit -m "feat: add build-time text sprite flag"
```

### Task 2: Add the Packaging Rewrite Seam and Focused Transform Tests

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\ITextComponentSpriteBakeService.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeRequest.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeResult.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs`

- [ ] **Step 1: Write the failing transform tests**

```csharp
[Fact]
public void TryTransform_WhenTextComponentIsNotFlagged_KeepsTextComponentPayload() {
    SceneComponentPackagingTransformService service = CreateService(new StubTextComponentSpriteBakeService());
    SceneComponentAssetRecord record = CreateTextRecord(convertTextToSprite: false);

    bool transformed = service.TryTransform(record, BuildRootPath, out SceneComponentAssetRecord transformedRecord);

    Assert.True(transformed);
    Assert.Equal("helengine.TextComponent", transformedRecord.ComponentTypeId);
}

[Fact]
public void TryTransform_WhenTextComponentIsFlagged_RewritesToSpritePayloadAndCallsBakeService() {
    StubTextComponentSpriteBakeService bakeService = new StubTextComponentSpriteBakeService();
    SceneComponentPackagingTransformService service = CreateService(bakeService);
    SceneComponentAssetRecord record = CreateTextRecord(convertTextToSprite: true);

    bool transformed = service.TryTransform(record, BuildRootPath, out SceneComponentAssetRecord transformedRecord);

    Assert.True(transformed);
    Assert.Equal("helengine.SpriteComponent", transformedRecord.ComponentTypeId);
    Assert.True(bakeService.WasCalled);
    Assert.Equal(new int2(128, 32), bakeService.LastRequest.Size);
}
```

- [ ] **Step 2: Run the transform tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~SceneComponentPackagingTransformServiceTests -v minimal`

Expected: FAIL because there is no bake-service seam and flagged text still packages as `helengine.TextComponent`.

- [ ] **Step 3: Add the bake-service contract and route flagged text through it**

```csharp
public interface ITextComponentSpriteBakeService {
    TextComponentSpriteBakeResult Bake(TextComponentSpriteBakeRequest request);
}

public sealed class TextComponentSpriteBakeRequest {
    public TextComponentSpriteBakeRequest(
        string sceneId,
        int componentIndex,
        string targetPlatformId,
        string text,
        int2 size,
        byte4 color,
        bool wrapText,
        FontAsset font,
        float fontScale,
        TextAlignment alignment,
        float rotation,
        byte renderOrder2D,
        byte layerMask) {
        SceneId = sceneId;
        ComponentIndex = componentIndex;
        TargetPlatformId = targetPlatformId;
        Text = text;
        Size = size;
        Color = color;
        WrapText = wrapText;
        Font = font;
        FontScale = fontScale;
        Alignment = alignment;
        Rotation = rotation;
        RenderOrder2D = renderOrder2D;
        LayerMask = layerMask;
    }

    public string SceneId { get; }
    public int ComponentIndex { get; }
    public string TargetPlatformId { get; }
    public string Text { get; }
    public int2 Size { get; }
    public byte4 Color { get; }
    public bool WrapText { get; }
    public FontAsset Font { get; }
    public float FontScale { get; }
    public TextAlignment Alignment { get; }
    public float Rotation { get; }
    public byte RenderOrder2D { get; }
    public byte LayerMask { get; }
}

public sealed class TextComponentSpriteBakeResult {
    public TextComponentSpriteBakeResult(TextureAsset textureAsset, TextureAssetProcessorSettings processorSettings, string stableKey) {
        TextureAsset = textureAsset ?? throw new ArgumentNullException(nameof(textureAsset));
        ProcessorSettings = processorSettings ?? throw new ArgumentNullException(nameof(processorSettings));
        StableKey = string.IsNullOrWhiteSpace(stableKey) ? throw new ArgumentException("Stable key must be provided.", nameof(stableKey)) : stableKey;
    }

    public TextureAsset TextureAsset { get; }
    public TextureAssetProcessorSettings ProcessorSettings { get; }
    public string StableKey { get; }
}

if (convertTextToSprite) {
    return RewriteTextComponentAsSpriteRecord(record, buildRootPath, fontReference, text, wrapText, size, color, sourceRect, rotation, renderOrder2D, layerMask);
}
```

- [ ] **Step 4: Run the transform tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~SceneComponentPackagingTransformServiceTests -v minimal`

Expected: PASS, with the flagged branch emitting `helengine.SpriteComponent` and unflagged text staying unchanged.

- [ ] **Step 5: Commit**

```bash
rtk git add C:\dev\helworks\helengine\engine\helengine.editor\managers\project\ITextComponentSpriteBakeService.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeRequest.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeResult.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs
rtk git commit -m "feat: add text sprite packaging seam"
```

### Task 3: Add Generated Texture Packaging Support for Baked Text

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformCookWorkItemFactory.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs`

- [ ] **Step 1: Write the failing packaging-output tests**

```csharp
[Fact]
public void TryTransform_WhenTextComponentIsFlagged_WritesGeneratedTextureAssetToCookedOutput() {
    StubTextComponentSpriteBakeService bakeService = new StubTextComponentSpriteBakeService();
    SceneComponentPackagingTransformService service = CreateService(bakeService);
    SceneComponentAssetRecord record = CreateTextRecord(convertTextToSprite: true);

    service.TryTransform(record, BuildRootPath, out SceneComponentAssetRecord transformedRecord);
    SceneAssetReference textureReference = ReadSpriteTextureReference(transformedRecord);
    string outputPath = Path.Combine(BuildRootPath, textureReference.RelativePath.Replace('/', Path.DirectorySeparatorChar));

    Assert.True(File.Exists(outputPath));
}

[Fact]
public void TryTransform_WhenBuilderOwnedTextureCookIsEnabled_EnqueuesGeneratedTextureCookWorkItem() {
    List<PlatformCookWorkItem> workItems = new List<PlatformCookWorkItem>();
    SceneComponentPackagingTransformService service = CreateBuilderOwnedTextureService(workItems, new StubTextComponentSpriteBakeService());

    service.TryTransform(CreateTextRecord(convertTextToSprite: true), BuildRootPath, out _);

    PlatformCookWorkItem workItem = Assert.Single(workItems);
    Assert.Equal("texture", workItem.SourceAssetKind);
    Assert.Contains("generated/text-sprites/", workItem.OutputRelativePath);
}
```

- [ ] **Step 2: Run the packaging-output tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~SceneComponentPackagingTransformServiceTests -v minimal`

Expected: FAIL because generated text textures are not yet written or queued for platform cooking.

- [ ] **Step 3: Add generated texture output helpers and the cook-work-item factory path**

```csharp
public static PlatformCookWorkItem CreateGeneratedTextureWorkItem(
    PlatformDefinition platformDefinition,
    string targetPlatformId,
    string sourceAssetPath,
    string outputRelativePath,
    string sourceAssetId,
    TextureAssetProcessorSettings processorSettings,
    AssetFileHasher fileHasher) {
    PlatformAssetCookCapabilityDefinition capability = ResolveBuilderOwnedCapability(platformDefinition, "texture");
    if (capability == null) {
        return null;
    }

    string fullSourcePath = Path.GetFullPath(sourceAssetPath);
    string serializedSettings = SerializeTextureSettings(processorSettings);
    string settingsHash = ComputeStringHash(fileHasher, serializedSettings);
    string sourceHash = fileHasher.ComputeHash(fullSourcePath);
    string normalizedOutputRelativePath = outputRelativePath.Replace('\\', '/');

    return new PlatformCookWorkItem(
        string.Concat(targetPlatformId, ":texture:", normalizedOutputRelativePath),
        fullSourcePath,
        "texture",
        targetPlatformId,
        capability.TargetArtifactKind,
        normalizedOutputRelativePath,
        string.Concat(capability.TargetArtifactKind, ":", normalizedOutputRelativePath),
        sourceHash,
        settingsHash,
        serializedSettings,
        [
            new PlatformCookWorkItemMetadata("source-asset-id", sourceAssetId),
            new PlatformCookWorkItemMetadata("settings-contract-id", capability.SettingsContractId)
        ]);
}

string sourceRelativePath = string.Concat("generated/text-sprites/", bakeResult.StableKey, ".hasset");
string cookedRelativePath = string.Concat("cooked/generated/text-sprites/", bakeResult.StableKey, ".hasset");
string sourceFullPath = Path.Combine(buildRootPath, sourceRelativePath.Replace('/', Path.DirectorySeparatorChar));
WriteAsset(sourceFullPath, bakeResult.TextureAsset);

if (SupportsBuilderOwnedPlatformCookKind("texture")) {
    PlatformCookWorkItem workItem = EditorPlatformCookWorkItemFactory.CreateGeneratedTextureWorkItem(
        PlatformDefinition,
        TargetPlatformId,
        sourceFullPath,
        cookedRelativePath,
        bakeResult.TextureAsset.Id,
        bakeResult.ProcessorSettings,
        FileHasher);
    PlatformCookWorkItemSink?.Invoke(workItem);
} else {
    WriteAsset(Path.Combine(buildRootPath, cookedRelativePath.Replace('/', Path.DirectorySeparatorChar)), bakeResult.TextureAsset);
}
```

- [ ] **Step 4: Run the packaging-output tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~SceneComponentPackagingTransformServiceTests -v minimal`

Expected: PASS, with generated texture assets written to build output or queued for builder-owned platform cooking.

- [ ] **Step 5: Commit**

```bash
rtk git add C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformCookWorkItemFactory.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs
rtk git commit -m "feat: package generated text sprites"
```

### Task 4: Implement the Real Bake Service with Exact Preview Capture

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\rendering\IRenderTargetTextureAssetReader.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\rendering\DirectX11RenderTargetTextureAssetReader.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeService.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\TextComponentSpriteBakeServiceTests.cs`

- [ ] **Step 1: Write the failing bake-service tests**

```csharp
[Fact]
public void Bake_WhenRequested_UsesTheAuthoredTextBoxSizeForTheGeneratedTexture() {
    FakeRenderTargetTextureAssetReader reader = new FakeRenderTargetTextureAssetReader(new TextureAsset {
        Id = "generated:text",
        Width = 128,
        Height = 32,
        ColorFormat = TextureAssetColorFormat.Rgba32,
        AlphaPrecision = TextureAssetAlphaPrecision.A8
    });
    TextComponentSpriteBakeService service = CreateBakeService(reader);

    TextComponentSpriteBakeResult result = service.Bake(new TextComponentSpriteBakeRequest(
        "Scenes/Test.helen",
        0,
        "ds",
        "BACK",
        new int2(128, 32),
        new byte4(255, 255, 255, 255),
        false,
        CreateFont(),
        1f,
        TextAlignment.Center,
        0f,
        12,
        1));

    Assert.Equal((ushort)128, result.TextureAsset.Width);
    Assert.Equal((ushort)32, result.TextureAsset.Height);
}

[Fact]
public void Bake_WhenRequested_UsesDSFriendlyDefaultProcessorSettings() {
    TextComponentSpriteBakeService service = CreateBakeService(new FakeRenderTargetTextureAssetReader(CreateTextureAsset(96, 24)));

    TextComponentSpriteBakeResult result = service.Bake(CreateRequest("ds"));

    Assert.Equal(TextureAssetColorFormat.Indexed8.ToString(), result.ProcessorSettings.ColorFormatId);
    Assert.Equal(TextureAssetAlphaPrecision.A4, result.ProcessorSettings.AlphaPrecision);
}
```

- [ ] **Step 2: Run the bake-service tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~TextComponentSpriteBakeServiceTests -v minimal`

Expected: FAIL because the bake service and render-target readback abstraction do not yet exist.

- [ ] **Step 3: Implement the bake service and readback abstraction**

```csharp
public interface IRenderTargetTextureAssetReader {
    TextureAsset ReadTextureAsset(RenderTarget renderTarget, string assetId);
}

public sealed class TextComponentSpriteBakeService : ITextComponentSpriteBakeService {
    readonly RenderManager3D RenderManager3D;
    readonly IRenderTargetTextureAssetReader RenderTargetTextureAssetReader;

    public TextComponentSpriteBakeService(RenderManager3D renderManager3D, IRenderTargetTextureAssetReader renderTargetTextureAssetReader) {
        RenderManager3D = renderManager3D ?? throw new ArgumentNullException(nameof(renderManager3D));
        RenderTargetTextureAssetReader = renderTargetTextureAssetReader ?? throw new ArgumentNullException(nameof(renderTargetTextureAssetReader));
    }

    public TextComponentSpriteBakeResult Bake(TextComponentSpriteBakeRequest request) {
        using EditorExact2DPreviewCaptureService captureService = new EditorExact2DPreviewCaptureService(RenderManager3D);
        Entity sourceEntity = new Entity();
        sourceEntity.InitComponents();
        sourceEntity.InitChildren();

        TextComponent sourceComponent = new TextComponent {
            Text = request.Text,
            Size = request.Size,
            Color = request.Color,
            WrapText = request.WrapText,
            Font = request.Font,
            FontScale = request.FontScale,
            Alignment = request.Alignment,
            Rotation = request.Rotation,
            RenderOrder2D = request.RenderOrder2D,
            LayerMask = request.LayerMask
        };
        sourceEntity.AddComponent(sourceComponent);
        captureService.CaptureTextPreview(sourceEntity, sourceComponent, request.Size);

        string stableKey = ComputeStableKey(request);
        TextureAsset textureAsset = RenderTargetTextureAssetReader.ReadTextureAsset(captureService.PreviewRenderTarget, string.Concat("generated:text:", stableKey));
        TextureAssetProcessorSettings processorSettings = ResolveProcessorSettings(request.TargetPlatformId);
        return new TextComponentSpriteBakeResult(textureAsset, processorSettings, stableKey);
    }
}
```

- [ ] **Step 4: Run the bake-service tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextComponentSpriteBakeServiceTests|FullyQualifiedName~EditorExact2DPreviewCaptureServiceTests" -v minimal`

Expected: PASS, with the bake service preserving authored box size and producing platform-appropriate processor settings.

- [ ] **Step 5: Commit**

```bash
rtk git add C:\dev\helworks\helengine\engine\helengine.editor\managers\rendering\IRenderTargetTextureAssetReader.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\rendering\DirectX11RenderTargetTextureAssetReader.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\TextComponentSpriteBakeService.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\TextComponentSpriteBakeServiceTests.cs
rtk git commit -m "feat: bake text components into sprite textures"
```

### Task 5: Wire the Packager and Prove the End-to-End Build Output

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`

- [ ] **Step 1: Write the failing end-to-end packager tests**

```csharp
[Fact]
public void Package_WhenSceneContainsFlaggedTextComponent_WritesSpriteComponentIntoPackagedScene() {
    string sceneId = "Scenes/BakedTextScene.helen";
    WriteSceneAsset(sceneId, "helengine.TextComponent", WriteTextComponentPayload(convertTextToSprite: true), new[] { CreateEditorFontReference() });

    EditorPlatformBuildScenePackager packager = CreatePackager(new StubTextComponentSpriteBakeService());
    packager.Package(new[] { sceneId }, BuildRootPath);

    SceneAsset packagedScene;
    using (FileStream stream = File.OpenRead(GetPackagedScenePath(BuildRootPath, sceneId))) {
        packagedScene = Assert.IsType<SceneAsset>(AssetSerializer.Deserialize(stream));
    }

    Assert.Equal("helengine.SpriteComponent", packagedScene.RootEntities[0].Components[0].ComponentTypeId);
}

[Fact]
public void Package_WhenSceneContainsFlaggedTextComponent_WritesPackagedTextureReferencedByTheSpritePayload() {
    string sceneId = "Scenes/BakedTextScene.helen";
    WriteSceneAsset(sceneId, "helengine.TextComponent", WriteTextComponentPayload(convertTextToSprite: true), new[] { CreateEditorFontReference() });

    EditorPlatformBuildScenePackager packager = CreatePackager(new StubTextComponentSpriteBakeService());
    packager.Package(new[] { sceneId }, BuildRootPath);

    SceneAsset packagedScene = LoadPackagedScene(sceneId);
    SceneAssetReference textureReference = ReadSpriteTextureReference(packagedScene.RootEntities[0].Components[0]);
    string packagedTexturePath = Path.Combine(BuildRootPath, textureReference.RelativePath.Replace('/', Path.DirectorySeparatorChar));

    Assert.True(File.Exists(packagedTexturePath));
}
```

- [ ] **Step 2: Run the end-to-end packager tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~EditorPlatformBuildScenePackagerTests -v minimal`

Expected: FAIL because the packager does not yet inject the bake service into `SceneComponentPackagingTransformService`.

- [ ] **Step 3: Wire the bake service through `EditorWindowsBuildScenePackager`**

```csharp
readonly ITextComponentSpriteBakeService TextComponentSpriteBakeService;

public EditorPlatformBuildScenePackager(
    string projectRootPath,
    IReadOnlyList<IAssetImporterRegistration> importerRegistrations = null,
    PlatformDefinition platformDefinition = null,
    IBuildStatusReporter statusReporter = null,
    IPlatformAssetBuilder builder = null,
    string selectedBuildProfileId = "",
    string selectedGraphicsProfileId = "",
    IScriptTypeResolver scriptTypeResolver = null,
    ITextComponentSpriteBakeService textComponentSpriteBakeService = null) {
    TextComponentSpriteBakeService = textComponentSpriteBakeService
        ?? new TextComponentSpriteBakeService(
            Core.Instance?.RenderManager3D ?? throw new InvalidOperationException("Text sprite baking requires an initialized RenderManager3D."),
            new DirectX11RenderTargetTextureAssetReader());

    TransformService = new SceneComponentPackagingTransformService(
        assetsRootPath,
        projectContentManager,
        assetImportManager,
        fileSystemModelResolver,
        referencedShaderAssetIds,
        referencedShaderAssetIdsSet,
        targetPlatformId,
        builder,
        selectedBuildProfileId,
        selectedGraphicsProfileId,
        scriptTypeResolver,
        RememberPlatformCookWorkItem,
        platformDefinition,
        TextComponentSpriteBakeService);
}
```

- [ ] **Step 4: Run the end-to-end packager tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorPlatformBuildScenePackagerTests|FullyQualifiedName~SceneComponentPackagingTransformServiceTests|FullyQualifiedName~TextComponentSpriteBakeServiceTests" -v minimal`

Expected: PASS, with packaged flagged text rewritten to sprite output and a generated texture present in the build output.

- [ ] **Step 5: Commit**

```bash
rtk git add C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs
rtk git commit -m "feat: package flagged text as sprites"
```

## Self-Review

- Spec coverage:
  - authored flag: Task 1
  - packaging rewrite to sprite-only runtime output: Tasks 2 and 5
  - generated build-owned texture only during packaging: Tasks 3 and 5
  - authored size-box preservation: Task 4 bake-service tests
  - no silent fallback on bake failure: Tasks 2 and 5 require hard failure paths
- Placeholder scan:
  - no `TODO`, `TBD`, or “handle later” language remains
  - every task names exact files, test targets, and commands
- Type consistency:
  - `ConvertTextToSprite`, `ITextComponentSpriteBakeService`, `TextComponentSpriteBakeRequest`, `TextComponentSpriteBakeResult`, and `IRenderTargetTextureAssetReader` are used consistently across tasks
