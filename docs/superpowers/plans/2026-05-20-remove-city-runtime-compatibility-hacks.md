# Remove City Runtime Compatibility Hacks Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove all engine/editor runtime compatibility hacks for `city.*` components, regenerate fresh scripted-component support, and verify the DS build runs only on the clean contract.

**Architecture:** Engine and editor will stop treating city component ids as built-in compatibility types. Fresh generated scripted-component payloads and deserializers will own `city.menu.DemoDiscReturnToMenuComponent`, `city.menu.PlatformInfoTextComponent`, and `city.menu.NintendoDsReturnOverlayComponent` end-to-end.

**Tech Stack:** C#, helengine editor/runtime codegen pipeline, city gameplay components, DS native build pipeline, xUnit editor/runtime tests.

---

## File Structure

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs`
  - Remove the city-specific engine-owned runtime compatibility filter.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimeComponentRegistry.cs`
  - Remove registration of built-in city deserializers.
- Delete: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimeDemoDiscReturnToMenuComponentDeserializer.cs`
  - Remove the engine-owned demo-disc return compatibility deserializer.
- Delete: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimePlatformInfoTextComponentDeserializer.cs`
  - Remove the engine-owned platform-info compatibility deserializer.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs`
  - Remove legacy city packaging assumptions that only exist for compatibility hacks.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
  - Remove legacy city packaging assumptions that only exist for compatibility hacks.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
  - Assert city components are no longer excluded from automatic runtime support generation.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`
  - Assert fresh packaged city scripted-component payloads use the generic automatic format.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs`
  - Verify fresh return/menu scenes still load correctly after compatibility removal.

### Task 1: Add the failing generated-core regression for city component filtering

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`

- [ ] **Step 1: Add the failing test that proves city components should no longer be filtered out**

```csharp
[Fact]
public void Generate_WhenSceneUsesCityMenuComponents_DoesNotTreatThemAsEngineOwnedCompatibilityTypes() {
    string scenePath = CreateCookedScene(
        "city-components-scene.hasset",
        "city.menu.DemoDiscReturnToMenuComponent, gameplay",
        "city.menu.PlatformInfoTextComponent, gameplay");

    DictionaryScriptTypeResolver scriptTypeResolver = new DictionaryScriptTypeResolver();
    scriptTypeResolver.Register("city.menu.DemoDiscReturnToMenuComponent, gameplay", typeof(TestUpdateOnlyScriptComponent));
    scriptTypeResolver.Register("city.menu.PlatformInfoTextComponent, gameplay", typeof(TestUpdateOnlyScriptComponent));

    RunGeneratedCoreRegeneration(scenePath, scriptTypeResolver);

    string generatedSource = ReadGeneratedSource("GeneratedRuntimeComponentRegistry.cpp");
    Assert.Contains("city.menu.DemoDiscReturnToMenuComponent, gameplay", generatedSource, StringComparison.Ordinal);
    Assert.Contains("city.menu.PlatformInfoTextComponent, gameplay", generatedSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests.Generate_WhenSceneUsesCityMenuComponents_DoesNotTreatThemAsEngineOwnedCompatibilityTypes" -v minimal
```

Expected: FAIL because the current generated-core path still filters city compatibility component ids out.

- [ ] **Step 3: Commit the failing test**

```powershell
git -C C:\dev\helworks\helengine add engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs
git -C C:\dev\helworks\helengine commit -m "Add failing city compatibility filter regression"
```

### Task 2: Remove engine-owned city compatibility filtering and runtime deserializers

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimeComponentRegistry.cs`
- Delete: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimeDemoDiscReturnToMenuComponentDeserializer.cs`
- Delete: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimePlatformInfoTextComponentDeserializer.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorGeneratedCoreRegenerationServiceTests.cs`

- [ ] **Step 1: Remove the city compatibility-type filter from generated-core regeneration**

```csharp
static bool IsEngineOwnedRuntimeCompatibilityComponentTypeId(string componentTypeId) {
    if (string.IsNullOrWhiteSpace(componentTypeId)) {
        return false;
    }

    return false;
}
```

- [ ] **Step 2: Remove built-in city deserializer registration from the runtime registry**

```csharp
public static RuntimeComponentRegistry CreateDefault() {
    RuntimeComponentRegistry registry = new RuntimeComponentRegistry();
    registry.Register(new RuntimeCameraComponentDeserializer());
    registry.Register(new RuntimeMeshComponentDeserializer());
    registry.Register(new RuntimeTextComponentDeserializer());
    registry.Register(new RuntimeSpriteComponentDeserializer());
    registry.Register(new RuntimeRoundedRectComponentDeserializer());
    registry.Register(new RuntimeDebugComponentDeserializer());
    registry.Register(new RuntimeFPSComponentDeserializer());
    registry.Register(new RuntimeSceneMapComponentDeserializer());
    // city-specific deserializers removed here
    return registry;
}
```

- [ ] **Step 3: Delete the built-in city deserializer files**

Delete:

```text
C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimeDemoDiscReturnToMenuComponentDeserializer.cs
C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimePlatformInfoTextComponentDeserializer.cs
```

- [ ] **Step 4: Run the focused generated-core regression**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorGeneratedCoreRegenerationServiceTests.Generate_WhenSceneUsesCityMenuComponents_DoesNotTreatThemAsEngineOwnedCompatibilityTypes" -v minimal
```

Expected: PASS.

- [ ] **Step 5: Commit the compatibility-removal core changes**

```powershell
git -C C:\dev\helworks\helengine add engine\helengine.editor\managers\project\EditorGeneratedCoreRegenerationService.cs engine\helengine.core\scene\runtime\RuntimeComponentRegistry.cs
git -C C:\dev\helworks\helengine rm engine\helengine.core\scene\runtime\RuntimeDemoDiscReturnToMenuComponentDeserializer.cs engine\helengine.core\scene\runtime\RuntimePlatformInfoTextComponentDeserializer.cs
git -C C:\dev\helworks\helengine commit -m "Remove city runtime compatibility deserializers"
```

### Task 3: Remove legacy city packaging assumptions

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`

- [ ] **Step 1: Remove city-specific legacy constants and rewrite branches that only preserve compatibility hacks**

```csharp
// Remove:
const string LegacyPlatformInfoTextComponentTypeId = "city.menu.PlatformInfoTextComponent, gameplay";

// Remove any branch shaped like:
if (string.Equals(componentRecord.ComponentTypeId, LegacyPlatformInfoTextComponentTypeId, StringComparison.Ordinal)) {
    ...
}
```

- [ ] **Step 2: Add a packaging regression that expects automatic scripted payload handling**

```csharp
[Fact]
public void Package_WhenSceneUsesCityMenuComponents_WritesAutomaticScriptPayloadWithoutLegacyCompatibilityRewrite() {
    // create scene component records for city.menu.DemoDiscReturnToMenuComponent and city.menu.PlatformInfoTextComponent
    // package them
    // assert payload version == AutomaticScriptComponentRuntimeDeserializer.CurrentVersion
    // assert payload member count matches the live reflected member count, not a legacy custom format
}
```

- [ ] **Step 3: Run the focused packaging regression slice**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorWindowsBuildScenePackagerTests.Package_WhenSceneUsesCityMenuComponents_WritesAutomaticScriptPayloadWithoutLegacyCompatibilityRewrite" -v minimal
```

Expected: PASS.

- [ ] **Step 4: Commit the packaging cleanup**

```powershell
git -C C:\dev\helworks\helengine add engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs
git -C C:\dev\helworks\helengine commit -m "Remove city packaging compatibility rewrites"
```

### Task 4: Regenerate and verify fresh city runtime component flow

**Files:**
- Verify: `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscReturnToMenuComponent.cs`
- Verify: `C:\dev\helprojs\city\assets\codebase\menu\PlatformInfoTextComponent.cs`
- Verify: `C:\dev\helprojs\city\assets\codebase\menu\NintendoDsReturnOverlayComponent.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneMapServiceTests.cs` if fresh-contract regressions need adjustment

- [ ] **Step 1: Run the focused scene/runtime regression slice on fresh automatic city components**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneMapServiceTests.Update_WhenReturnToMenuPointerClickIsTriggeredAndMappingExists_LoadsMappedSceneId|FullyQualifiedName~SceneMapServiceTests.Update_WhenReturnToMenuPointerClickRepeatsAfterSuccessfulLoad_DoesNotReloadTargetScene|FullyQualifiedName~SceneMapServiceTests.LoadScene_WhenPersistentBootSceneRoutesRepeatedCubeReturns_PreservesMappedMenuAcrossMultipleCycles" -v minimal
```

Expected: PASS on fresh output only.

- [ ] **Step 2: Regenerate fresh DS project outputs by rebuilding the ROM**

Run:

```powershell
dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath=C:\dev\helworks\helengine\.codex-build-ds-editor-font\bin\ -- --project C:\dev\helprojs\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: build succeeds and regenerates fresh scripted-component runtime support for city menu components.

- [ ] **Step 3: Relaunch melonDS on the fresh ROM**

Run:

```powershell
Get-Process melonDS -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList 'C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'
```

Expected: melonDS opens the fresh ROM.

- [ ] **Step 4: Perform the manual smoke check on fresh output**

Check:

```text
1. Boot into the DS menu.
2. Confirm there is no "serialized member" exception.
3. Open cube_test.
4. Tap the bottom-screen Back button.
5. Press B to return from cube_test.
6. Confirm both paths return to the menu on fresh generated output.
```

- [ ] **Step 5: Commit any final fresh-contract adjustments**

```powershell
git -C C:\dev\helworks\helengine status --short
git -C C:\dev\helprojs\city status --short
git -C C:\dev\helworks\helengine-ds status --short
```

Commit only if code changed during this verification task. If no code changed, skip this step.
