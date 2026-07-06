# DS Component Platform Member Overrides Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a generic platform-specific component member override system that stays out of `helengine.core`, then use it to ship Nintendo DS `TextComponent.BGLayer` end to end.

**Architecture:** Platform builders register typed component-member schema extensions in shared platform metadata. The editor persists per-platform component member values in detached platform override state, packaging/codegen merge those target-specific members into generated runtime component schemas, and DS runtime rendering consumes the generated `TextComponent.BGLayer` field to route text to different BG layers.

**Tech Stack:** C#, xUnit, shared platform metadata in `helengine.baseplatform`, editor scene persistence and inspector UI in `helengine.editor`, generated runtime deserializer/codegen pipeline, Nintendo DS C++ renderer in `helengine-ds`.

---

### Task 1: Add shared platform component-member schema definitions

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\PlatformComponentMemberValueKind.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\PlatformComponentMemberDefinition.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\PlatformDefinition.cs`
- Create: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformDefinitionFactoryTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsPlatformDefinitionFactory.cs`

- [ ] **Step 1: Write the failing platform-definition test**

```csharp
namespace helengine.ds.builder.tests;

public sealed class NintendoDsPlatformDefinitionFactoryTests {
    [Fact]
    public void Create_WhenCalled_RegistersDsTextComponentBgLayerMemberOverride() {
        PlatformDefinition definition = NintendoDsPlatformDefinitionFactory.Create();
        PlatformComponentMemberDefinition member = Assert.Single(
            definition.ComponentMemberDefinitions,
            entry => string.Equals(entry.ComponentTypeId, "helengine.textcomponent", StringComparison.OrdinalIgnoreCase)
                && string.Equals(entry.MemberName, "BGLayer", StringComparison.Ordinal));

        Assert.Equal(PlatformComponentMemberValueKind.Int32, member.ValueKind);
        Assert.Equal("0", member.DefaultValue);
        Assert.Equal("BG Layer", member.DisplayName);
        Assert.Equal(new[] { "0", "1" }, member.AllowedValues);
    }
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter FullyQualifiedName~NintendoDsPlatformDefinitionFactoryTests.Create_WhenCalled_RegistersDsTextComponentBgLayerMemberOverride
```

Expected: FAIL because `PlatformDefinition` does not yet expose `ComponentMemberDefinitions` and the DS definition does not register `BGLayer`.

- [ ] **Step 3: Add the shared platform metadata types and wire them into `PlatformDefinition`**

```csharp
namespace helengine.baseplatform.Definitions;

/// <summary>
/// Identifies the serialized value shape used by one platform-specific component member definition.
/// </summary>
public enum PlatformComponentMemberValueKind {
    Int32,
    Boolean,
    Float32,
    String
}
```

```csharp
namespace helengine.baseplatform.Definitions;

/// <summary>
/// Describes one typed platform-specific member exposed for one serialized component type.
/// </summary>
public sealed class PlatformComponentMemberDefinition {
    public PlatformComponentMemberDefinition(
        string componentTypeId,
        string memberName,
        string displayName,
        PlatformComponentMemberValueKind valueKind,
        string defaultValue,
        string[] allowedValues) {
        ComponentTypeId = string.IsNullOrWhiteSpace(componentTypeId) ? throw new ArgumentException("Component type id is required.", nameof(componentTypeId)) : componentTypeId;
        MemberName = string.IsNullOrWhiteSpace(memberName) ? throw new ArgumentException("Member name is required.", nameof(memberName)) : memberName;
        DisplayName = displayName ?? string.Empty;
        ValueKind = valueKind;
        DefaultValue = defaultValue ?? string.Empty;
        AllowedValues = allowedValues == null ? Array.Empty<string>() : [.. allowedValues];
    }

    public string ComponentTypeId { get; }
    public string MemberName { get; }
    public string DisplayName { get; }
    public PlatformComponentMemberValueKind ValueKind { get; }
    public string DefaultValue { get; }
    public string[] AllowedValues { get; }
}
```

```csharp
public PlatformDefinition(
    string platformId,
    string displayName,
    PlatformBuildProfileDefinition[] buildProfiles,
    PlatformGraphicsProfileDefinition[] graphicsProfiles,
    PlatformAssetRequirementDefinition[] assetRequirements,
    PlatformMaterialSchemaDefinition[] materialSchemas,
    PlatformComponentSupportRule[] componentSupportRules,
    PlatformCodegenProfileDefinition[] codegenProfiles,
    PlatformStorageProfileDefinition[] storageProfiles,
    PlatformMediaProfileDefinition[] mediaProfiles,
    RuntimeGenerationContract runtimeGenerationContract = null,
    PlatformHostDebugCapability hostDebugCapability = null,
    PlatformAssetCookCapabilityDefinition[] assetCookCapabilities = null,
    PlatformComponentMemberDefinition[] componentMemberDefinitions = null) {
    if (componentMemberDefinitions != null && Array.Exists(componentMemberDefinitions, definition => definition == null)) {
        throw new ArgumentException("Component member definitions cannot contain null entries.", nameof(componentMemberDefinitions));
    }

    ComponentMemberDefinitions = componentMemberDefinitions == null ? Array.Empty<PlatformComponentMemberDefinition>() : [.. componentMemberDefinitions];
}

public PlatformComponentMemberDefinition[] ComponentMemberDefinitions { get; }
```

- [ ] **Step 4: Register DS `TextComponent.BGLayer` in the platform definition**

```csharp
new PlatformComponentMemberDefinition(
    "helengine.textcomponent",
    "BGLayer",
    "BG Layer",
    PlatformComponentMemberValueKind.Int32,
    "0",
    ["0", "1"])
```

Pass that array through the `PlatformDefinition` constructor from `NintendoDsPlatformDefinitionFactory.Create()`.

- [ ] **Step 5: Run test to verify it passes**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter FullyQualifiedName~NintendoDsPlatformDefinitionFactoryTests.Create_WhenCalled_RegistersDsTextComponentBgLayerMemberOverride
```

Expected: PASS

- [ ] **Step 6: Commit**

```powershell
rtk git add C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\PlatformComponentMemberValueKind.cs C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\PlatformComponentMemberDefinition.cs C:\dev\helworks\helengine\engine\helengine.baseplatform\Definitions\PlatformDefinition.cs C:\dev\helworks\helengine-ds\builder\NintendoDsPlatformDefinitionFactory.cs C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformDefinitionFactoryTests.cs
rtk git commit -m "feat: add platform component member definitions"
```

### Task 2: Persist platform-specific component member values in editor override state

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\persistence\EntityComponentPlatformOverrideState.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\ComponentPlatformOverridePayloadService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\ComponentPlatformOverridePayloadServiceTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneSaveServiceTests.cs`

- [ ] **Step 1: Write the failing wrapped-payload round-trip test**

```csharp
[Fact]
public void WrapAndReadOverrideStates_WhenPlatformMemberValuesExist_RoundTripsMemberValues() {
    ComponentPlatformOverridePayloadService service = new ComponentPlatformOverridePayloadService();
    EntityComponentSaveState saveState = new EntityComponentSaveState();
    EntityComponentPlatformOverrideState overrideState = saveState.GetOrCreatePlatformOverride("ds");
    overrideState.Payload = [1, 2, 3];
    overrideState.SetPropertyOverride("Text");
    overrideState.SetMemberValue("BGLayer", "1");

    SceneComponentAssetRecord wrapped = service.Wrap(new SceneComponentAssetRecord {
        ComponentTypeId = "helengine.TextComponent",
        ComponentIndex = 0,
        Payload = [9, 9, 9]
    }, saveState);

    EntityComponentPlatformOverrideState restored = Assert.Single(service.ReadOverrideStates(wrapped));
    Assert.True(restored.HasMemberValue("BGLayer"));
    Assert.Equal("1", restored.GetMemberValue("BGLayer"));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~ComponentPlatformOverridePayloadServiceTests.WrapAndReadOverrideStates_WhenPlatformMemberValuesExist_RoundTripsMemberValues
```

Expected: FAIL because `EntityComponentPlatformOverrideState` does not yet store named member values and the payload wrapper does not serialize them.

- [ ] **Step 3: Add named member-value storage to the platform override state**

```csharp
readonly Dictionary<string, string> MemberValuesByName;

public void SetMemberValue(string memberName, string value) {
    if (string.IsNullOrWhiteSpace(memberName)) {
        throw new ArgumentException("Member name must be provided.", nameof(memberName));
    }

    MemberValuesByName[memberName] = value ?? string.Empty;
}

public bool TryGetMemberValue(string memberName, out string value) {
    if (string.IsNullOrWhiteSpace(memberName)) {
        throw new ArgumentException("Member name must be provided.", nameof(memberName));
    }

    return MemberValuesByName.TryGetValue(memberName, out value);
}

public bool HasMemberValue(string memberName) {
    if (string.IsNullOrWhiteSpace(memberName)) {
        throw new ArgumentException("Member name must be provided.", nameof(memberName));
    }

    return MemberValuesByName.ContainsKey(memberName);
}

public void RemoveMemberValue(string memberName) {
    if (string.IsNullOrWhiteSpace(memberName)) {
        throw new ArgumentException("Member name must be provided.", nameof(memberName));
    }

    MemberValuesByName.Remove(memberName);
}

public IEnumerable<KeyValuePair<string, string>> EnumerateMemberValues() {
    return MemberValuesByName;
}
public bool HasAnyMemberValues => MemberValuesByName.Count > 0;
```

- [ ] **Step 4: Extend wrapped override payload versioning to include member values**

```csharp
writer.WriteInt32(memberValues.Count);
for (int index = 0; index < memberValues.Count; index++) {
    writer.WriteString(memberValues[index].Key);
    writer.WriteString(memberValues[index].Value);
}
```

```csharp
int memberValueCount = reader.ReadInt32();
for (int index = 0; index < memberValueCount; index++) {
    overrideState.SetMemberValue(reader.ReadString(), reader.ReadString());
}
```

Increment `WrappedPayloadVersion` so stale wrapped payloads fail loudly instead of misreading the new format.

- [ ] **Step 5: Add the scene save/load round-trip test**

```csharp
[Fact]
public void SaveAndLoad_WhenPlatformOverrideContainsMemberValues_RestoresDetachedPlatformMemberState() {
    SceneSaveService saveService = new SceneSaveService(projectRootPath, new ComponentPersistenceRegistry());
    Entity entity = new Entity { Name = "TextHost" };
    TextComponent textComponent = new TextComponent { Text = "HELLO" };
    EntitySaveComponent saveComponent = new EntitySaveComponent();
    entity.AddComponent(textComponent);
    entity.AddComponent(saveComponent);

    EntityComponentSaveState componentState = saveComponent.GetOrCreateComponentState(textComponent);
    EntityComponentPlatformOverrideState overrideState = componentState.GetOrCreatePlatformOverride("ds");
    overrideState.SetPropertyOverride("Text");
    overrideState.SetMemberValue("BGLayer", "1");

    saveService.Save(scenePath, new[] { entity }, new SceneEditorSettings());

    SceneLoadService loadService = new SceneLoadService(projectRootPath, new ComponentPersistenceRegistry());
    Entity[] loadedEntities = loadService.Load(scenePath);
    TextComponent loadedText = Assert.Single(loadedEntities[0].Components.OfType<TextComponent>());
    EntitySaveComponent loadedSave = Assert.Single(loadedEntities[0].Components.OfType<EntitySaveComponent>());
    EntityComponentPlatformOverrideState loadedOverride = loadedSave.GetOrCreateComponentState(loadedText).GetOrCreatePlatformOverride("ds");
    Assert.Equal("1", loadedOverride.GetMemberValue("BGLayer"));
}
```

- [ ] **Step 6: Run tests to verify they pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~ComponentPlatformOverridePayloadServiceTests|FullyQualifiedName~SceneSaveServiceTests"
```

Expected: PASS for the new member-value tests and existing override round-trip coverage.

- [ ] **Step 7: Commit**

```powershell
rtk git add C:\dev\helworks\helengine\engine\helengine.editor\components\persistence\EntityComponentPlatformOverrideState.cs C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\ComponentPlatformOverridePayloadService.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\ComponentPlatformOverridePayloadServiceTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneSaveServiceTests.cs
rtk git commit -m "feat: persist platform component member override values"
```

### Task 3: Surface platform-specific component members in the existing platform tabs

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\PlatformComponentMemberDescriptor.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\PlatformComponentMemberDescriptorResolver.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\ComponentPropertiesView.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\EditorSession.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\PropertiesPanelComponentShellTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\PropertiesPanelMutationTests.cs`

- [ ] **Step 1: Write the failing shell test for DS override rows**

```csharp
[Fact]
public void ShowEntityProperties_WhenDsTabSelectedAndTextComponentHasRegisteredPlatformMembers_ShowsDsOverridesSection() {
    ComponentPropertiesView panel = CreatePanelWithPlatformDefinitions();
    Entity entity = CreateEntityWithTextComponent();

    panel.ShowEntityProperties(entity, new[] { "ds" });
    panel.SetActivePlatformForTests("ds");

    Assert.Contains(FindSectionTitles(panel), title => title == "DS Overrides");
    Assert.Contains(FindRowLabels(panel), label => label == "BG Layer");
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~PropertiesPanelComponentShellTests.ShowEntityProperties_WhenDsTabSelectedAndTextComponentHasRegisteredPlatformMembers_ShowsDsOverridesSection
```

Expected: FAIL because the component inspector does not yet resolve platform-specific member descriptors from `PlatformDefinition`.

- [ ] **Step 3: Add a resolver that maps `PlatformDefinition.ComponentMemberDefinitions` to per-component UI descriptors**

```csharp
public sealed class PlatformComponentMemberDescriptor {
    public PlatformComponentMemberDescriptor(string platformId, string componentTypeId, string memberName, string displayName, PlatformComponentMemberValueKind valueKind, string defaultValue, string[] allowedValues) {
        PlatformId = platformId ?? throw new ArgumentNullException(nameof(platformId));
        ComponentTypeId = componentTypeId ?? throw new ArgumentNullException(nameof(componentTypeId));
        MemberName = memberName ?? throw new ArgumentNullException(nameof(memberName));
        DisplayName = displayName ?? string.Empty;
        ValueKind = valueKind;
        DefaultValue = defaultValue ?? string.Empty;
        AllowedValues = allowedValues == null ? Array.Empty<string>() : [.. allowedValues];
    }

    public string PlatformId { get; }
    public string ComponentTypeId { get; }
    public string MemberName { get; }
    public string DisplayName { get; }
    public PlatformComponentMemberValueKind ValueKind { get; }
    public string DefaultValue { get; }
    public string[] AllowedValues { get; }
}
```

```csharp
public sealed class PlatformComponentMemberDescriptorResolver {
    public IReadOnlyList<PlatformComponentMemberDescriptor> Resolve(PlatformDefinition definition, Component component) {
        string componentTypeId = AutomaticScriptComponentPersistenceDescriptor.BuildComponentTypeId(component.GetType());
        return definition.ComponentMemberDefinitions
            .Where(entry => string.Equals(entry.ComponentTypeId, componentTypeId, StringComparison.OrdinalIgnoreCase))
            .Select(entry => new PlatformComponentMemberDescriptor(definition.PlatformId, entry.ComponentTypeId, entry.MemberName, entry.DisplayName, entry.ValueKind, entry.DefaultValue, entry.AllowedValues))
            .ToArray();
    }
}
```

- [ ] **Step 4: Render DS override rows and persist edits through `EntityComponentPlatformOverrideState`**

```csharp
foreach (PlatformComponentMemberDescriptor descriptor in platformMemberDescriptors) {
    ComponentPropertyRow row = AcquireRow();
    row.Label.Text = descriptor.DisplayName;
    row.Tag = descriptor;
    row.TextField.Text = ReadPlatformMemberValue(commonComponent, saveComponent, CurrentPlatformId, descriptor);
    row.TextField.TextCommitted += _ => {
        WritePlatformMemberValue(commonComponent, saveComponent, CurrentPlatformId, descriptor, row.TextField.Text);
        ShowComponents(CurrentEntity, CurrentPlatformId);
    };
}
```

Use the existing platform tab and revert affordances. When a value matches the descriptor default, remove the explicit member override instead of persisting redundant state.

- [ ] **Step 5: Add the mutation test for editing DS `BG Layer`**

```csharp
[Fact]
public void EditPlatformMemberValue_WhenDsBgLayerChanges_PersistsDetachedMemberOverride() {
    ComponentPropertiesView panel = CreatePanelWithPlatformDefinitions();
    Entity entity = CreateEntityWithTextComponent();
    EntitySaveComponent saveComponent = entity.GetComponent<EntitySaveComponent>();

    panel.ShowEntityProperties(entity, new[] { "ds" });
    panel.SetActivePlatformForTests("ds");
    SetPlatformMemberText(panel, "BG Layer", "1");

    EntityComponentPlatformOverrideState overrideState = saveComponent.GetOrCreateComponentState(entity.GetComponent<TextComponent>()).GetOrCreatePlatformOverride("ds");
    Assert.Equal("1", overrideState.GetMemberValue("BGLayer"));
}
```

- [ ] **Step 6: Run tests to verify they pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~PropertiesPanelComponentShellTests|FullyQualifiedName~PropertiesPanelMutationTests"
```

Expected: PASS for the new DS override UI tests and existing platform-tab editing tests.

- [ ] **Step 7: Commit**

```powershell
rtk git add C:\dev\helworks\helengine\engine\helengine.editor\components\ui\PlatformComponentMemberDescriptor.cs C:\dev\helworks\helengine\engine\helengine.editor\components\ui\PlatformComponentMemberDescriptorResolver.cs C:\dev\helworks\helengine\engine\helengine.editor\components\ui\ComponentPropertiesView.cs C:\dev\helworks\helengine\engine\helengine.editor\EditorSession.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\PropertiesPanelComponentShellTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\PropertiesPanelMutationTests.cs
rtk git commit -m "feat: expose platform component member overrides in inspector"
```

### Task 4: Build a platform-extended component schema for packaging and generated runtime deserializers

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\PlatformExtendedScriptComponentSchemaBuilder.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\ScriptComponentPlayerDeserializerGenerator.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\ScriptComponentPlayerDeserializerGeneratorTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`

- [ ] **Step 1: Write the failing packaging test**

```csharp
[Fact]
public void TransformComponent_WhenDsTextComponentHasPlatformMemberOverride_WritesBgLayerIntoPackagedPayload() {
    SceneComponentPackagingTransformService service = CreateServiceForPlatform("ds");
    TextComponent component = CreateTextComponent();
    EntityComponentSaveState saveState = new EntityComponentSaveState();
    saveState.GetOrCreatePlatformOverride("ds").SetMemberValue("BGLayer", "1");

    SceneComponentAssetRecord transformed = service.TransformAutomaticComponentForTests(component, saveState);

    Assert.Contains("BGLayer", DecodePackagedMemberNames(transformed.Payload));
}
```

- [ ] **Step 2: Write the failing codegen/deserializer test**

```csharp
[Fact]
public void GenerateNativeDeserializerSource_WhenDsTextComponentHasPlatformExtendedMember_EmitsBgLayerRead() {
    ScriptComponentPlayerDeserializerGenerator generator = new ScriptComponentPlayerDeserializerGenerator();
    ScriptComponentReflectionSchema schema = CreateExtendedTextSchema("BGLayer");

    string source = generator.GenerateNativeDeserializerSource(schema);

    Assert.Contains("component->set_BGLayer", source, StringComparison.Ordinal);
}
```

- [ ] **Step 3: Run the targeted tests to verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneComponentPackagingTransformServiceTests|FullyQualifiedName~ScriptComponentPlayerDeserializerGeneratorTests|FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests"
```

Expected: FAIL because the packaging/codegen schema is still built only from reflected core members.

- [ ] **Step 4: Introduce a platform-extended schema builder and use it in packaging/codegen**

```csharp
public sealed class PlatformExtendedScriptComponentSchemaBuilder {
    readonly ScriptComponentReflectionSchemaBuilder ReflectionSchemaBuilder;

    public ScriptComponentReflectionSchema Build(Type componentType, PlatformDefinition platformDefinition) {
        ScriptComponentReflectionSchema baseSchema = ReflectionSchemaBuilder.Build(componentType);
        if (platformDefinition == null) {
            return baseSchema;
        }

        List<ScriptComponentReflectionMember> members = new List<ScriptComponentReflectionMember>(baseSchema.Members);
        foreach (PlatformComponentMemberDefinition extension in platformDefinition.ComponentMemberDefinitions.Where(entry => Matches(componentType, entry))) {
            members.Add(CreateSyntheticMember(extension));
        }

        return new ScriptComponentReflectionSchema(baseSchema.ComponentType, members);
    }
}
```

Use this builder in:

- `SceneComponentPackagingTransformService` when serializing packaged runtime payloads
- `EditorGeneratedCoreRegenerationService` when emitting cooked-scene automatic runtime deserializers
- `ScriptComponentPlayerDeserializerGenerator` input schemas

- [ ] **Step 5: Write member values into the packaged payload from platform override state or platform default**

```csharp
for (int index = 0; index < schema.Members.Count; index++) {
    ScriptComponentReflectionMember member = schema.Members[index];
    if (member is PlatformSyntheticScriptComponentMember syntheticMember) {
        string persistedValue = ResolvePlatformMemberValue(saveState, PlatformDefinition.PlatformId, syntheticMember.Definition);
        WriteSyntheticPlatformMemberValue(writer, syntheticMember, persistedValue);
        continue;
    }

    AutomaticScriptComponentPersistenceDescriptor.WriteSupportedMemberValue(writer, member, component, rewrittenSaveState);
}
```

- [ ] **Step 6: Run tests to verify they pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneComponentPackagingTransformServiceTests|FullyQualifiedName~ScriptComponentPlayerDeserializerGeneratorTests|FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests"
```

Expected: PASS, and generated DS deserializers should now include typed reads for synthetic platform members like `BGLayer`.

- [ ] **Step 7: Commit**

```powershell
rtk git add C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\PlatformExtendedScriptComponentSchemaBuilder.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\ScriptComponentPlayerDeserializerGenerator.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\ScriptComponentPlayerDeserializerGeneratorTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs
rtk git commit -m "feat: generate platform-extended component schemas"
```

### Task 5: Verify DS platform registration and generated `TextComponent.BGLayer` output in the DS builder

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsGeneratedCoreStagerTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsGeneratedCoreStagerSourceAuditTests.cs` (create if a dedicated source-audit file is clearer)

- [ ] **Step 1: Write the failing DS generated-core test**

```csharp
[Fact]
public void Stage_WhenDsGeneratedRuntimeTextComponentIsExtended_EmitsBgLayerAccessors() {
    string generatedCoreRootPath = CreateGeneratedCoreRoot();

    EditorGeneratedCoreRegenerationService.EmitCookedSceneAutomaticRuntimeComponentDeserializers(
        generatedCoreRootPath,
        BuildDsPlatformDefinition(),
        CreateCookedSceneManifestWithTextComponent());

    string generatedTextHeader = File.ReadAllText(Path.Combine(generatedCoreRootPath, "TextComponent.hpp"));
    string generatedTextSource = File.ReadAllText(Path.Combine(generatedCoreRootPath, "TextComponent.cpp"));
    Assert.Contains("int32_t get_BGLayer() const;", generatedTextHeader, StringComparison.Ordinal);
    Assert.Contains("void set_BGLayer(int32_t value);", generatedTextHeader, StringComparison.Ordinal);
    Assert.Contains("BGLayer(", generatedTextSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the DS generated-core test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests"
```

Expected: FAIL because the staged/generated DS runtime still mirrors only core `TextComponent` members.

- [ ] **Step 3: Extend the DS builder test harness expectations to include `BGLayer`**

```csharp
Assert.Contains("int32_t get_BGLayer() const;", generatedTextHeader, StringComparison.Ordinal);
Assert.Contains("void set_BGLayer(int32_t value);", generatedTextHeader, StringComparison.Ordinal);
Assert.Contains("component->set_BGLayer(reader->ReadInt32());", generatedTextDeserializerSource, StringComparison.Ordinal);
```

- [ ] **Step 4: Update the DS builder/stager expectations to consume the platform-extended generated runtime output**

Keep the builder itself thin here. The heavy lifting should already have happened in the editor-side generation pipeline from Task 4. This task is the DS-side verification that the generated core staged into the DS workspace now contains the extended text component member.

- [ ] **Step 5: Run the DS builder tests to verify they pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests"
```

Expected: PASS with DS generated-core tests proving `BGLayer` survives staging.

- [ ] **Step 6: Commit**

```powershell
rtk git add C:\dev\helworks\helengine-ds\builder.tests\NintendoDsGeneratedCoreStagerTests.cs C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs
rtk git commit -m "test: verify DS generated core includes text bg layer"
```

### Task 6: Route DS text to BG0 or BG1 using generated `TextComponent.BGLayer`

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.cpp`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Write the failing DS source-audit test**

```csharp
[Fact]
public void Source_whenTextDrawableUsesBgLayerOne_routesTextToFrontBackgroundLayer() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("int32_t bgLayer = ResolveTextBackgroundLayer(text);", sourceCode, StringComparison.Ordinal);
    Assert.Contains("EnsureScreenTextBackgroundReady(targetScreen, bgLayer);", sourceCode, StringComparison.Ordinal);
    Assert.Contains("if (bgLayer == 1)", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the source-audit test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests.Source_whenTextDrawableUsesBgLayerOne_routesTextToFrontBackgroundLayer
```

Expected: FAIL because the DS text renderer only knows one BG text layer per screen.

- [ ] **Step 3: Add dual text-background state and BG-layer resolution**

```cpp
int32_t NintendoDsRenderManager2D::ResolveTextBackgroundLayer(ITextDrawable2D* text) const {
    if (text == nullptr) {
        throw new ArgumentNullException("text");
    }

    ::TextComponent* typedText = he_cpp_try_cast<TextComponent>(text);
    if (typedText == nullptr) {
        return 0;
    }

    return typedText->get_BGLayer() <= 0 ? 0 : 1;
}
```

```cpp
void NintendoDsRenderManager2D::EnsureScreenTextBackgroundReady(NintendoDsScreenTarget targetScreen, int32_t bgLayer) {
    int32_t backgroundSlot = bgLayer <= 0 ? 0 : 1;
    int32_t backgroundPriority = bgLayer <= 0 ? 2 : 0;
    ScreenTextBackgroundState& state = GetScreenTextBackgroundState(targetScreen, backgroundSlot);
    if (!state.IsInitialized) {
        state.BackgroundId = targetScreen == NintendoDsScreenTarget::Main
            ? bgInit(backgroundSlot, BgType_Text8bpp, BgSize_T_256x256, 0, backgroundSlot)
            : bgInitSub(backgroundSlot, BgType_Text8bpp, BgSize_T_256x256, 0, backgroundSlot);
        state.IsInitialized = true;
    }

    bgSetPriority(state.BackgroundId, backgroundPriority);
}
```

- [ ] **Step 4: Route text writes and glyph cache state through the resolved BG layer**

```cpp
int32_t bgLayer = ResolveTextBackgroundLayer(text);
EnsureScreenTextBackgroundReady(targetScreen, bgLayer);
WriteScreenTextLine(targetScreen, bgLayer, targetRow, startColumn, visibleLine, writableColumnCount);
```

Keep the first slice intentionally narrow:

- `BGLayer = 0` -> current background path
- `BGLayer = 1` -> front text background path

- [ ] **Step 5: Run the DS source-audit tests to verify they pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests|FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests"
```

Expected: PASS, with text-path audits proving the renderer now distinguishes background layers and still preserves the extended-palette mode on top-screen fallback.

- [ ] **Step 6: Build and launch the DS ROM for visual verification**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city\project.heproj -Platform ds -Output C:\dev\helprojs\city\output\ds
rtk powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\launch_in_emulator.ps1 -ArtifactPath C:\dev\helprojs\city\output\ds\helengine_ds.nds
```

Expected: the current DS proof build still renders top-screen sprite/logo, and authored bottom-screen text with `BGLayer = 1` can appear in front of sprite content.

- [ ] **Step 7: Commit**

```powershell
rtk git add C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.hpp C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.cpp C:\dev\helworks\helengine-ds\builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs
rtk git commit -m "feat: route DS text through selectable bg layers"
```

## Self-Review

### Spec coverage

- Generic platform component member schema definitions: covered by Task 1.
- Detached platform member persistence for scene/prefab/component override state: covered by Task 2.
- Existing platform tabs plus `DS Overrides` UX: covered by Task 3.
- Builder/codegen/runtime schema extension flow: covered by Tasks 4 and 5.
- Concrete DS `TextComponent.BGLayer` runtime behavior: covered by Task 6.

No spec gaps remain.

### Placeholder scan

- No `TODO`, `TBD`, or “implement later” placeholders remain.
- Every task names exact files and runnable commands.
- All testing steps include concrete commands and expected outcomes.

### Type consistency

- Shared metadata names are kept consistent as `PlatformComponentMemberDefinition` and `PlatformComponentMemberValueKind`.
- Persisted member value API is consistently `SetMemberValue` / `TryGetMemberValue` / `GetMemberValue`.
- The first DS member is consistently named `BGLayer`.
