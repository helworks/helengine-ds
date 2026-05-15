# DS Runtime 60 FPS Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reduce Nintendo DS renderer CPU overhead toward a stable 60 fps for the current colored cube scene without changing scene content and while preserving the current look.

**Architecture:** Move lighting discovery to frame scope, move transform and drawable setup to drawable scope, and keep the existing face-normal lighting model intact. Use the runtime-managed light collections already owned by `ObjectManager` as the authoritative per-frame light source instead of rescanning the full scene graph inside the renderer hot path.

**Tech Stack:** C++ Nintendo DS runtime, generated-core C++ interfaces, C# xUnit source-audit tests, `dotnet` build/test, `melonDS`

---

### Task 1: Lock the Frame-Lighting Contract with Source Audits

**Files:**
- Create: `builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Write the failing audit test for frame-scoped lighting**

Create `builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs` with:

```csharp
namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS renderer source so performance-sensitive draw-path contracts stay intact.
/// </summary>
public class NintendoDsRenderManager3DPerformanceSourceAuditTests {
    /// <summary>
    /// Verifies frame lighting is resolved once per draw from runtime-managed light collections.
    /// </summary>
    [Fact]
    public void Source_whenResolvingFrameLighting_usesObjectManagerLightCollectionsOutsideDrawableLoop() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("float3 FrameLightDirection;", headerSource, StringComparison.Ordinal);
        Assert.Contains("float3 FrameDirectionalRadiance;", headerSource, StringComparison.Ordinal);
        Assert.Contains("float3 FrameAmbientRadiance;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ResolveFrameLighting(ObjectManager* objectManager);", headerSource, StringComparison.Ordinal);
        Assert.Contains("ResolveFrameLighting(objectManager);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("objectManager->get_DirectionalLights()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("objectManager->get_AmbientLights()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("ResolveSceneLighting(lightDirection, directionalRadiance, ambientRadiance);", sourceCode, StringComparison.Ordinal);
    }
}
```

- [ ] **Step 2: Run the audit test and verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager3DPerformanceSourceAuditTests.Source_whenResolvingFrameLighting_usesObjectManagerLightCollectionsOutsideDrawableLoop" --no-restore -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\bin\' -v minimal
```

Expected: FAIL because `NintendoDsRenderManager3D` does not yet define frame-light fields or `ResolveFrameLighting(ObjectManager* objectManager)`.

- [ ] **Step 3: Add the drawable-scope transform audit before implementation**

Extend `builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs` with:

```csharp
    /// <summary>
    /// Verifies drawable-scope transform inputs are cached outside the triangle loop.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingTriangles_cachesEntityTransformOutsidePerTriangleWork() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains(
            "void SubmitLitTriangle(\n"
            + "            NintendoDsRuntimeMaterial* runtimeMaterial,\n"
            + "            Array<float3>* positions,\n"
            + "            int32_t indexA,\n"
            + "            int32_t indexB,\n"
            + "            int32_t indexC,\n"
            + "            const float3& entityPosition,\n"
            + "            const float3& entityScale,\n"
            + "            const float4& entityOrientation);",
            headerSource,
            StringComparison.Ordinal);
        Assert.Contains("Entity* entity = drawable->get_Parent();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float3 entityPosition = float3::get_Zero();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float3 entityScale = float3::get_One();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float4 entityOrientation = float4::get_Identity();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TransformVertex((*positions)[indexA], entityPosition, entityScale, entityOrientation)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TransformVertex(drawable, (*positions)[indexA])", sourceCode, StringComparison.Ordinal);
    }
```

- [ ] **Step 4: Run the audit test and verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager3DPerformanceSourceAuditTests.Source_whenSubmittingTriangles_cachesEntityTransformOutsidePerTriangleWork" --no-restore -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\bin\' -v minimal
```

Expected: FAIL because `SubmitLitTriangle(...)` still takes `IDrawable3D*` and `TransformVertex(...)` still re-reads the drawable parent per triangle.

- [ ] **Step 5: Commit the red tests**

```bash
git add builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs
git commit -m "test: add DS renderer performance source audits"
```

### Task 2: Move Lighting Resolution to Frame Scope

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager3D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager3D.cpp`
- Test: `builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs`

- [ ] **Step 1: Add frame-light fields and the new resolver declaration**

Update `src/platform/ds/NintendoDsRenderManager3D.hpp` by replacing the old lighting resolver declaration and adding frame fields:

```cpp
        /// <summary>
        /// Stores the resolved frame directional-light direction used for triangle lighting during the active draw call.
        /// </summary>
        float3 FrameLightDirection;

        /// <summary>
        /// Stores the resolved frame directional-light radiance used for triangle lighting during the active draw call.
        /// </summary>
        float3 FrameDirectionalRadiance;

        /// <summary>
        /// Stores the resolved frame ambient radiance used for triangle lighting during the active draw call.
        /// </summary>
        float3 FrameAmbientRadiance;
```

```cpp
        /// <summary>
        /// Resolves frame lighting once from runtime-managed light collections before drawable submission begins.
        /// </summary>
        /// <param name="objectManager">Runtime object manager that owns registered light collections.</param>
        void ResolveFrameLighting(ObjectManager* objectManager);
```

Remove this declaration:

```cpp
        void ResolveSceneLighting(float3& lightDirection, float3& directionalRadiance, float3& ambientRadiance) const;
```

- [ ] **Step 2: Initialize frame-light state in the constructor**

Update `src/platform/ds/NintendoDsRenderManager3D.cpp` constructor initializer list:

```cpp
    NintendoDsRenderManager3D::NintendoDsRenderManager3D()
        : HardwareInitialized(false)
        , RenderQueueSnapshotVisitor(new NintendoDsRenderQueueSnapshotVisitor())
        , LastBuildStage("NotStarted")
        , LastBuildAssetId()
        , FrameLightDirection(0.0f, -1.0f, 0.0f)
        , FrameDirectionalRadiance(0.0f, 0.0f, 0.0f)
        , FrameAmbientRadiance(0.0f, 0.0f, 0.0f) {
    }
```

- [ ] **Step 3: Resolve frame lighting once in `Draw()`**

Update `Draw()` in `src/platform/ds/NintendoDsRenderManager3D.cpp`:

```cpp
    void NintendoDsRenderManager3D::Draw() {
        Core* core = Core::get_Instance();
        if (core == nullptr) {
            throw new InvalidOperationException("Core::Instance was not initialized.");
        }

        ObjectManager* objectManager = core->get_ObjectManager();
        if (objectManager == nullptr) {
            throw new InvalidOperationException("Object manager was not initialized.");
        }

        List<ICamera*>* cameras = objectManager->get_Cameras();
        if (cameras == nullptr || cameras->Count() <= 0) {
            return;
        }

        ResolveFrameLighting(objectManager);

        ICamera* selectedCamera = (*cameras)[0];
        EnsureHardwareInitialized();
        ClearFromCamera(selectedCamera);
        ConfigureCamera(selectedCamera);
        DrawRenderQueue(selectedCamera);
    }
```

- [ ] **Step 4: Implement `ResolveFrameLighting(ObjectManager* objectManager)` against runtime-managed light collections**

Replace the old entity/component scan with:

```cpp
    void NintendoDsRenderManager3D::ResolveFrameLighting(ObjectManager* objectManager) {
        FrameLightDirection = float3(0.0f, -1.0f, 0.0f);
        FrameDirectionalRadiance = float3(0.0f, 0.0f, 0.0f);
        FrameAmbientRadiance = float3(0.0f, 0.0f, 0.0f);

        if (objectManager == nullptr) {
            return;
        }

        List<AmbientLightComponent*>* ambientLights = objectManager->get_AmbientLights();
        if (ambientLights != nullptr) {
            for (int32_t lightIndex = 0; lightIndex < ambientLights->Count(); lightIndex++) {
                AmbientLightComponent* ambientLight = (*ambientLights)[lightIndex];
                if (ambientLight == nullptr) {
                    continue;
                }

                float4 color = ambientLight->get_Color();
                FrameAmbientRadiance = FrameAmbientRadiance + float3(
                    color.X * ambientLight->get_Intensity(),
                    color.Y * ambientLight->get_Intensity(),
                    color.Z * ambientLight->get_Intensity());
            }
        }

        List<DirectionalLightComponent*>* directionalLights = objectManager->get_DirectionalLights();
        if (directionalLights == nullptr || directionalLights->Count() <= 0) {
            return;
        }

        DirectionalLightComponent* directionalLight = (*directionalLights)[0];
        if (directionalLight == nullptr) {
            return;
        }

        float4 color = directionalLight->get_Color();
        FrameLightDirection = LightDirectionUtility::GetLightDirection(directionalLight);
        FrameDirectionalRadiance = float3(
            color.X * directionalLight->get_Intensity(),
            color.Y * directionalLight->get_Intensity(),
            color.Z * directionalLight->get_Intensity());
    }
```

- [ ] **Step 5: Run the first audit and verify it passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager3DPerformanceSourceAuditTests.Source_whenResolvingFrameLighting_usesObjectManagerLightCollectionsOutsideDrawableLoop" --no-restore -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\bin\' -v minimal
```

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add src/platform/ds/NintendoDsRenderManager3D.hpp src/platform/ds/NintendoDsRenderManager3D.cpp builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs
git commit -m "refactor: cache DS lighting at frame scope"
```

### Task 3: Move Transform Setup to Drawable Scope

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager3D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager3D.cpp`
- Test: `builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs`

- [ ] **Step 1: Change the triangle and transform signatures in the header**

Update `src/platform/ds/NintendoDsRenderManager3D.hpp`:

```cpp
        void SubmitLitTriangle(
            NintendoDsRuntimeMaterial* runtimeMaterial,
            Array<float3>* positions,
            int32_t indexA,
            int32_t indexB,
            int32_t indexC,
            const float3& entityPosition,
            const float3& entityScale,
            const float4& entityOrientation);
```

```cpp
        float3 TransformVertex(
            float3 modelVertex,
            const float3& entityPosition,
            const float3& entityScale,
            const float4& entityOrientation);
```

Remove the old signatures:

```cpp
        void SubmitLitTriangle(
            IDrawable3D* drawable,
            NintendoDsRuntimeMaterial* runtimeMaterial,
            Array<float3>* positions,
            int32_t indexA,
            int32_t indexB,
            int32_t indexC,
            float3 lightDirection,
            float3 directionalRadiance,
            float3 ambientRadiance);
```

```cpp
        float3 TransformVertex(IDrawable3D* drawable, float3 modelVertex);
```

- [ ] **Step 2: Cache entity transform once in `SubmitOpaqueDrawable(...)`**

Update the start of `SubmitOpaqueDrawable(...)` in `src/platform/ds/NintendoDsRenderManager3D.cpp`:

```cpp
        Entity* entity = drawable->get_Parent();
        float3 entityPosition = float3::get_Zero();
        float3 entityScale = float3::get_One();
        float4 entityOrientation = float4::get_Identity();
        if (entity != nullptr) {
            entityPosition = entity->get_Position();
            entityScale = entity->get_Scale();
            entityOrientation = entity->get_Orientation();
        }

        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
        glBegin(GL_TRIANGLES);
```

- [ ] **Step 3: Remove per-drawable light discovery and pass cached transform state into every triangle submission**

Update the triangle-submission calls in `SubmitOpaqueDrawable(...)`:

```cpp
                SubmitLitTriangle(
                    runtimeMaterial,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices32)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 2]),
                    entityPosition,
                    entityScale,
                    entityOrientation);
```

```cpp
                SubmitLitTriangle(
                    runtimeMaterial,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices16)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 2]),
                    entityPosition,
                    entityScale,
                    entityOrientation);
```

```cpp
                SubmitLitTriangle(
                    runtimeMaterial,
                    positions,
                    index,
                    index + 1,
                    index + 2,
                    entityPosition,
                    entityScale,
                    entityOrientation);
```

Delete these lines from `SubmitOpaqueDrawable(...)`:

```cpp
        float3 lightDirection(0.0f, -1.0f, 0.0f);
        float3 directionalRadiance(0.0f, 0.0f, 0.0f);
        float3 ambientRadiance(0.0f, 0.0f, 0.0f);
        ResolveSceneLighting(lightDirection, directionalRadiance, ambientRadiance);
```

- [ ] **Step 4: Rewrite `SubmitLitTriangle(...)` and `TransformVertex(...)` to use cached frame and drawable state**

Replace both implementations in `src/platform/ds/NintendoDsRenderManager3D.cpp` with:

```cpp
    void NintendoDsRenderManager3D::SubmitLitTriangle(
        NintendoDsRuntimeMaterial* runtimeMaterial,
        Array<float3>* positions,
        int32_t indexA,
        int32_t indexB,
        int32_t indexC,
        const float3& entityPosition,
        const float3& entityScale,
        const float4& entityOrientation) {
        if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
        } else if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (indexA < 0 || indexB < 0 || indexC < 0) {
            return;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length) {
            return;
        }

        float3 vertexA = TransformVertex((*positions)[indexA], entityPosition, entityScale, entityOrientation);
        float3 vertexB = TransformVertex((*positions)[indexB], entityPosition, entityScale, entityOrientation);
        float3 vertexC = TransformVertex((*positions)[indexC], entityPosition, entityScale, entityOrientation);
        float3 faceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexB, vertexC);
        float diffuse = NintendoDsLightingMath::EvaluateDirectionalDiffuse(faceNormal, FrameLightDirection);
        float3 lighting = FrameAmbientRadiance + (FrameDirectionalRadiance * diffuse);
        lighting = NintendoDsLightingMath::ClampColor(lighting);

        float3 shapedLighting = NintendoDsLightingMath::ApplyDisplayContrastCurve(lighting);
        float3 finalColor = NintendoDsLightingMath::MultiplyColor(runtimeMaterial->BaseColor, shapedLighting);
        finalColor = NintendoDsLightingMath::ClampColor(finalColor);

        glColor(NintendoDsColorPacker::PackOpaqueColor(finalColor));
        glVertex3v16(floattov16(vertexA.X), floattov16(vertexA.Y), floattov16(vertexA.Z));
        glVertex3v16(floattov16(vertexB.X), floattov16(vertexB.Y), floattov16(vertexB.Z));
        glVertex3v16(floattov16(vertexC.X), floattov16(vertexC.Y), floattov16(vertexC.Z));
    }
```

```cpp
    float3 NintendoDsRenderManager3D::TransformVertex(
        float3 modelVertex,
        const float3& entityPosition,
        const float3& entityScale,
        const float4& entityOrientation) {
        float3 scaledVertex = modelVertex * entityScale;
        float3 rotatedVertex = float4::RotateVector(scaledVertex, entityOrientation);
        return rotatedVertex + entityPosition;
    }
```

- [ ] **Step 5: Run the second audit and verify it passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager3DPerformanceSourceAuditTests.Source_whenSubmittingTriangles_cachesEntityTransformOutsidePerTriangleWork" --no-restore -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\bin\' -v minimal
```

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add src/platform/ds/NintendoDsRenderManager3D.hpp src/platform/ds/NintendoDsRenderManager3D.cpp builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs
git commit -m "refactor: cache DS drawable transform state"
```

### Task 4: Run Focused Regression and Full DS Validation

**Files:**
- Modify: none
- Test: `builder.tests/helengine.ds.builder.tests.csproj`
- Verify build: `C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj`

- [ ] **Step 1: Run the focused DS regression suite**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsBootHostSourceAuditTests|FullyQualifiedName~NintendoDsRenderManager3DSourceAuditTests|FullyQualifiedName~NintendoDsRenderManager3DPerformanceSourceAuditTests|FullyQualifiedName~NintendoDsGeneratedCoreStagerTests.Stage_whenGeneratedMaterialResolverLoadsShaderPackages_rewritesToCookedPlatformOwnedMaterialPath" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\bin\' -v minimal
```

Expected: PASS

- [ ] **Step 2: Rebuild the DS city project with an isolated app output path**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-build-ds-mainloop\bin\' -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: `Build completed for platform 'ds': C:\tmp\helengine-ds-city-cube-project\output\ds`

- [ ] **Step 3: Relaunch `melonDS` with the rebuilt ROM**

Run:

```powershell
rtk proxy powershell -NoProfile -ExecutionPolicy Bypass -Command "Get-Process melonDS -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue; Start-Sleep -Milliseconds 500; Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList 'C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'"
```

Expected: `melonDS` opens with the colored cube scene still rendering.

- [ ] **Step 4: Record the validation result**

Capture these outcomes in the implementation notes or handoff message:

```text
- startup scene still loads successfully
- top screen still renders the colored cube scene
- bottom screen no longer stalls in the smoke-test VBlank loop
- focused DS regressions pass
- full DS build succeeds
```

- [ ] **Step 5: Commit**

```bash
git add builder.tests/NintendoDsRenderManager3DPerformanceSourceAuditTests.cs src/platform/ds/NintendoDsRenderManager3D.hpp src/platform/ds/NintendoDsRenderManager3D.cpp
git commit -m "perf: reduce DS renderer hot-path overhead"
```

## Self-Review

### Spec Coverage

- Frame-scope lighting cache: covered by Task 1 and Task 2.
- Runtime-managed light ownership: covered by Task 2 via `ObjectManager` light collections.
- Drawable-scope setup: covered by Task 3.
- Preserve current look: covered by Task 3 by keeping the same face-normal lighting and contrast shaping.
- Boot and cooked-material stability: covered by Task 4 regression suite.
- DS validation: covered by Task 4 build and emulator steps.

### Placeholder Scan

- No `TODO`, `TBD`, or deferred implementation markers remain.
- Conditional future stage work from the design spec is intentionally not part of this plan. This plan implements the first performance pass only: frame-scope lighting and drawable-scope transform caching. If 60 fps is still not close enough after this pass, write a follow-up plan for the DS-native submission-path upgrade.

### Type Consistency

- `ResolveFrameLighting(ObjectManager* objectManager)` is used consistently in tests and runtime changes.
- `FrameLightDirection`, `FrameDirectionalRadiance`, and `FrameAmbientRadiance` are the same field names across tasks.
- `SubmitLitTriangle(...)` and `TransformVertex(...)` signatures are aligned between header and source updates.
