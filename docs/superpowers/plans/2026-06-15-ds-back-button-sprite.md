# DS Back Button Sprite Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the Nintendo DS authored back-button body with a sprite-backed visual so the lilac button appears in melonDS while preserving the existing `BACK` text and return interaction.

**Architecture:** The current DS back button already has working authored interaction through `InteractableComponent` and `NintendoDsReturnOverlayComponent`. The change stays in the city scaffold: remove the `RoundedRectComponent` body, add a `SpriteComponent` body with a file-backed texture reference, and rely on the existing DS hardware sprite path that already services authored sprites elsewhere in the city project.

**Tech Stack:** C# scene-authoring code in `city`, xUnit source-audit tests in `helengine-ds`, Nintendo DS editor-owned build flow, melonDS manual verification, PowerShell for one-off texture generation.

---

### Task 1: Lock The Authored Contract With A Failing DS Scene Audit

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\CityNintendoDsSceneSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Add a failing source-audit test for the sprite-backed DS back button**

Insert this test into `CityNintendoDsSceneSourceAuditTests` near the other Nintendo DS scaffold assertions:

```csharp
    /// <summary>
    /// Verifies the shared Nintendo DS rendering scaffold authors the bottom-screen back button as a sprite-backed control instead of a rounded rectangle.
    /// </summary>
    [Fact]
    public void Sources_whenAuthoringDsRenderingCompanionScenes_useSpriteBackButtonBody() {
        string scaffoldFactorySource = File.ReadAllText(Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs"));

        Assert.Contains("SpriteComponent spriteComponent = new SpriteComponent {", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("Size = new int2(224, 32),", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("RenderOrder2D = 230,", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("buttonEntity.AddComponent(spriteComponent);", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("ApplyTextureReference(buttonEntity, spriteComponent, \"Images/Menu/ds-back-button.png\");", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("void ApplyTextureReference(Entity entity, Component component, string texturePath)", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("TextureAssetScenePersistenceSupport.TextureReferenceName", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.Contains("SceneAssetReferenceSourceKind.FileSystem", scaffoldFactorySource, StringComparison.Ordinal);
        Assert.DoesNotContain("buttonEntity.AddComponent(new RoundedRectComponent {", scaffoldFactorySource, StringComparison.Ordinal);
    }
```

- [ ] **Step 2: Run the focused audit to verify it fails**

Run:

```powershell
$env:HELENGINE_ROOT = 'C:\dev\helworks\helengine'
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~CityNintendoDsSceneSourceAuditTests.Sources_whenAuthoringDsRenderingCompanionScenes_useSpriteBackButtonBody" --no-restore -v minimal
```

Expected: `FAIL` because `NintendoDsRenderingSceneScaffoldFactory.cs` still contains `RoundedRectComponent` and does not yet contain `ApplyTextureReference(buttonEntity, spriteComponent, "Images/Menu/ds-back-button.png");`.

- [ ] **Step 3: Commit the failing audit in `helengine-ds`**

Run:

```powershell
rtk git -C C:\dev\helworks\helengine-ds add builder.tests\CityNintendoDsSceneSourceAuditTests.cs
rtk git -C C:\dev\helworks\helengine-ds commit -m "Add failing DS back-button sprite scaffold audit"
```

Expected: a commit containing only the new audit coverage in `helengine-ds`.

### Task 2: Author The Sprite-Backed DS Back Button In The City Scaffold

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
- Create: `C:\dev\helprojs\city\assets\Images\Menu\ds-back-button.png`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\CityNintendoDsSceneSourceAuditTests.cs`

- [ ] **Step 1: Create the DS back-button texture asset**

Generate the sprite art as a fixed `224 x 32` PNG with the same lilac fill and border colors currently hard-coded in the rounded rectangle:

```powershell
Add-Type -AssemblyName System.Drawing

$texturePath = 'C:\dev\helprojs\city\assets\Images\Menu\ds-back-button.png'
$bitmap = New-Object System.Drawing.Bitmap 224, 32
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.Clear([System.Drawing.Color]::FromArgb(255, 52, 36, 76))

$borderPen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 201, 147, 255)), 2
$graphics.DrawRectangle($borderPen, 1, 1, 221, 29)

$bitmap.Save($texturePath, [System.Drawing.Imaging.ImageFormat]::Png)

$borderPen.Dispose()
$graphics.Dispose()
$bitmap.Dispose()
```

Expected: `C:\dev\helprojs\city\assets\Images\Menu\ds-back-button.png` exists and visually matches the current dark lilac body with a lighter lilac square border.

- [ ] **Step 2: Replace the rounded rectangle body with a sprite body in the scaffold**

In `NintendoDsRenderingSceneScaffoldFactory.cs`, replace the current `RoundedRectComponent` block inside `CreateDefaultBottomOverlay(...)` with this `SpriteComponent` block:

```csharp
            SpriteComponent spriteComponent = new SpriteComponent {
                Size = new int2(224, 32),
                RenderOrder2D = 230,
                LayerMask = RuntimeLayerMask
            };
            buttonEntity.AddComponent(spriteComponent);
            ApplyTextureReference(buttonEntity, spriteComponent, "Images/Menu/ds-back-button.png");
```

Remove this old block completely:

```csharp
            buttonEntity.AddComponent(new RoundedRectComponent {
                Size = new int2(224, 32),
                Radius = 0f,
                BorderThickness = 2f,
                FillColor = new byte4(52, 36, 76, 255),
                BorderColor = new byte4(201, 147, 255, 255),
                RenderOrder2D = 230,
                LayerMask = RuntimeLayerMask
            });
```

Do not change:

- `InteractableComponent.Size = new int2(224, 32)`
- `NintendoDsReturnOverlayComponent`
- the child `BACK` `TextComponent`

- [ ] **Step 3: Add the texture-reference helper methods to the scaffold factory**

Copy the existing city pattern used by `DemoSceneInstructionOverlayFactory` into `NintendoDsRenderingSceneScaffoldFactory.cs` by adding these helpers near the existing font-reference helpers:

```csharp
        /// <summary>
        /// Stores the supplied texture reference on the generated scene save state for the given sprite component.
        /// </summary>
        /// <param name="entity">Entity that owns the component.</param>
        /// <param name="component">Component whose texture reference should be stored.</param>
        /// <param name="texturePath">Project-relative texture path.</param>
        void ApplyTextureReference(Entity entity, Component component, string texturePath) {
            if (entity == null) {
                throw new ArgumentNullException(nameof(entity));
            } else if (component == null) {
                throw new ArgumentNullException(nameof(component));
            } else if (string.IsNullOrWhiteSpace(texturePath)) {
                throw new ArgumentException("Texture path must be provided.", nameof(texturePath));
            }

            EntitySaveComponent saveComponent = FindRequiredEntitySaveComponent(entity);
            saveComponent.SetAssetReference(component, TextureAssetScenePersistenceSupport.TextureReferenceName, BuildFileReference(texturePath));
        }

        /// <summary>
        /// Builds one stable file-backed scene asset reference.
        /// </summary>
        /// <param name="relativePath">Project-relative asset path.</param>
        /// <returns>Stable file-backed scene asset reference.</returns>
        SceneAssetReference BuildFileReference(string relativePath) {
            if (string.IsNullOrWhiteSpace(relativePath)) {
                throw new ArgumentException("Relative path must be provided.", nameof(relativePath));
            }

            return new SceneAssetReference {
                SourceKind = SceneAssetReferenceSourceKind.FileSystem,
                RelativePath = relativePath
            };
        }
```

Keep the existing `ApplyFontReference(...)`, `FindRequiredEntitySaveComponent(...)`, and `BuildNintendoDsDebugFontReference()` helpers unchanged.

- [ ] **Step 4: Run the focused audit again to verify the authored contract now passes**

Run:

```powershell
$env:HELENGINE_ROOT = 'C:\dev\helworks\helengine'
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~CityNintendoDsSceneSourceAuditTests.Sources_whenAuthoringDsRenderingCompanionScenes_useSpriteBackButtonBody" --no-restore -v minimal
```

Expected: `PASS`.

### Task 3: Rebuild The DS Target And Verify The Sprite In melonDS

**Files:**
- Verify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
- Verify: `C:\dev\helprojs\city\assets\Images\Menu\ds-back-button.png`
- Verify: `C:\dev\helprojs\city\ds-build\helengine_ds.nds`

- [ ] **Step 1: Run the broader DS source-audit subset**

Run:

```powershell
$env:HELENGINE_ROOT = 'C:\dev\helworks\helengine'
rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~CityNintendoDsSceneSourceAuditTests|FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests|FullyQualifiedName~NintendoDsInputBackendSourceAuditTests" --no-restore -v minimal
```

Expected: `PASS`.

- [ ] **Step 2: Rebuild the city DS target through the normal editor-owned build flow**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Release -- --project C:\dev\helprojs\city\project.heproj --build ds --output C:\dev\helprojs\city\ds-build
```

Expected: output includes:

```text
Build completed for platform 'ds': C:\dev\helprojs\city\ds-build
```

- [ ] **Step 3: Launch the rebuilt ROM in melonDS**

Run:

```powershell
rtk proxy powershell.exe -NoProfile -Command '$emulatorPath = ''C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe''; $romPath = ''C:\dev\helprojs\city\ds-build\helengine_ds.nds''; Start-Process -FilePath $emulatorPath -ArgumentList $romPath -WorkingDirectory (Split-Path -Parent $emulatorPath)'
```

Expected: melonDS opens with the rebuilt `helengine_ds.nds`.

- [ ] **Step 4: Ask the user for the visual result and interaction result**

Use this exact check:

```text
Please tell me whether the lilac back button is visible on the bottom screen and whether tapping it returns to the DS menu.
```

Expected: the user reports that the button is visible, the `BACK` text is still present, and one tap returns to the menu. Do not use capture scripts or screenshots for this verification.

- [ ] **Step 5: Commit the city implementation once the build and manual verification pass**

Run:

```powershell
rtk git -C C:\dev\helprojs\city add assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs assets\Images\Menu\ds-back-button.png
rtk git -C C:\dev\helprojs\city add assets\Images\Menu\ds-back-button.png.hasset
rtk git -C C:\dev\helprojs\city commit -m "Author DS back button as sprite"
```

Expected: the city repo commit contains the scaffold swap and the new button texture asset. If `assets\Images\Menu\ds-back-button.png.hasset` was not emitted during local authoring, omit it from the `git add` command and keep the commit limited to the files that actually exist.

## Self-Review Notes

- **Spec coverage:** The plan covers the authored back-button sprite swap, the dedicated asset, the unchanged interaction/text ownership, the DS build, and the user-reported melonDS verification flow. The design’s optional DS runtime tightening is intentionally left out of the initial implementation path because current evidence shows authored sprites already exist in city scenes; if Task 3 proves otherwise, that is a new concrete renderer bug and should get a separate debug-first follow-up plan.
- **Placeholder scan:** No `TODO`/`TBD` placeholders remain. Each file path, test command, build command, and texture path is explicit.
- **Type consistency:** The plan uses the existing names already present in the codebase: `NintendoDsRenderingSceneScaffoldFactory`, `SpriteComponent`, `InteractableComponent`, `NintendoDsReturnOverlayComponent`, `TextureAssetScenePersistenceSupport.TextureReferenceName`, and `SceneAssetReferenceSourceKind.FileSystem`.
