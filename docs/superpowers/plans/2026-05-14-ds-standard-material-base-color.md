# DS Standard Material Base Color Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the Nintendo DS renderer honor standard-material `BaseColorBuffer` payloads so any standard material renders with lit authored base color instead of grayscale.

**Architecture:** Keep the DS runtime aligned with the existing engine material contract. Decode `BaseColorBuffer` from cooked `MaterialAsset::ConstantBuffers` into `NintendoDsRuntimeMaterial`, then multiply that base color by the current DS ambient + directional face-lighting result during triangle submission. There is no native DS unit-test harness in this repo today, so verification relies on focused build/export checks plus visual validation in `melonDS`.

**Tech Stack:** C++17 Nintendo DS runtime, libnds GL immediate mode, C# builder/export pipeline, xUnit for DS builder-side regression coverage, `rtk dotnet`, `melonDS`

---

## File Map

### Runtime Files

- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRuntimeMaterial.hpp`
  - store normalized authored base color for DS runtime materials
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRuntimeMaterial.cpp`
  - initialize DS runtime material base color to white
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.hpp`
  - declare base-color decoding helpers used by DS runtime material creation
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
  - parse `BaseColorBuffer`
  - replace grayscale final color output with base-color-modulated lighting
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsLightingMath.hpp`
  - add channel-wise color-shaping helpers
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsLightingMath.cpp`
  - implement channel-wise display-curve and color modulation helpers
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsColorPacker.hpp`
  - add float3-to-packed-color convenience overload if needed
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsColorPacker.cpp`
  - implement float3 packing if needed

### Verification / Support Files

- Modify temporarily for visual verification only: `C:\tmp\helengine-ds-city-cube-project\city\user_settings\build_config.json`
  - switch DS selected scene from `cube_test` to `colored_cube_grid`, then restore after verification
- Optional note-only reference: `C:\tmp\helengine-ds-city-cube-project\city\assets\codebase\rendering.tools\ColoredCubeGridSceneFactory.cs`
  - confirms the authored standard-material `base-color` content already exists

### Documentation

- Create: `C:\dev\helworks\helengine-ds\docs\superpowers\plans\2026-05-14-ds-standard-material-base-color.md`

---

### Task 1: Add DS Runtime Material Base Color Storage And Decode Path

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRuntimeMaterial.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRuntimeMaterial.cpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`

- [ ] **Step 1: Add runtime material base-color storage**

```cpp
class NintendoDsRuntimeMaterial : public RuntimeMaterial {
public:
    NintendoDsRuntimeMaterial();

    uint16_t PackedDiffuseColor;
    float3 BaseColor;
    bool SupportsGeometrySubmission;
};
```

- [ ] **Step 2: Initialize white fallback in the runtime material constructor**

```cpp
NintendoDsRuntimeMaterial::NintendoDsRuntimeMaterial()
    : PackedDiffuseColor(0xFFFF)
    , BaseColor(1.0f, 1.0f, 1.0f)
    , SupportsGeometrySubmission(false) {
}
```

- [ ] **Step 3: Declare focused decode helpers on the DS render manager**

```cpp
private:
    static constexpr const char* StandardMaterialBaseColorBufferName = "BaseColorBuffer";

    bool TryResolveStandardMaterialBaseColor(MaterialAsset* materialAsset, float3& resolvedColor) const;
    static bool TryDecodeFloat4ConstantBuffer(const Array<uint8_t>* data, float4& decodedColor);
```

- [ ] **Step 4: Implement `BaseColorBuffer` discovery and float decoding**

```cpp
bool NintendoDsRenderManager3D::TryResolveStandardMaterialBaseColor(MaterialAsset* materialAsset, float3& resolvedColor) const {
    if (materialAsset == nullptr || materialAsset->ConstantBuffers == nullptr) {
        return false;
    }

    for (int32_t index = 0; index < materialAsset->ConstantBuffers->Length; index++) {
        MaterialConstantBufferAsset* constantBuffer = (*materialAsset->ConstantBuffers)[index];
        if (constantBuffer == nullptr || constantBuffer->Name != StandardMaterialBaseColorBufferName) {
            continue;
        }

        float4 decodedColor;
        if (!TryDecodeFloat4ConstantBuffer(constantBuffer->Data, decodedColor)) {
            return false;
        }

        resolvedColor = float3(
            std::clamp(decodedColor.X, 0.0f, 1.0f),
            std::clamp(decodedColor.Y, 0.0f, 1.0f),
            std::clamp(decodedColor.Z, 0.0f, 1.0f));
        return true;
    }

    return false;
}
```

- [ ] **Step 5: Wire decoded base color into `BuildMaterialFromRaw(...)` with white fallback**

```cpp
RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) {
    NintendoDsRuntimeMaterial* runtimeMaterial = new NintendoDsRuntimeMaterial();
    runtimeMaterial->set_Id(materialAsset->get_Id());

    float3 resolvedBaseColor(1.0f, 1.0f, 1.0f);
    if (TryResolveStandardMaterialBaseColor(materialAsset, resolvedBaseColor)) {
        runtimeMaterial->BaseColor = resolvedBaseColor;
    }

    runtimeMaterial->PackedDiffuseColor = NintendoDsColorPacker::PackOpaqueWhite();
    runtimeMaterial->SupportsGeometrySubmission = true;
    if (materialAsset->RenderState != nullptr) {
        runtimeMaterial->SetRenderState(materialAsset->RenderState);
    }

    return runtimeMaterial;
}
```

- [ ] **Step 6: Build the DS builder and confirm the runtime compiles before lighting changes**

Run:

```powershell
rtk dotnet build C:\dev\helworks\helengine-ds\builder\helengine.ds.builder.csproj -p:HelengineRoot=C:\dev\helworks\helengine
```

Expected: `Build succeeded.`

- [ ] **Step 7: Commit the decode-path slice**

```bash
git add src/platform/ds/NintendoDsRuntimeMaterial.hpp src/platform/ds/NintendoDsRuntimeMaterial.cpp src/platform/ds/NintendoDsRenderManager3D.hpp src/platform/ds/NintendoDsRenderManager3D.cpp
git commit -m "Decode DS standard material base color"
```

---

### Task 2: Replace Grayscale Output With Base-Color-Modulated Lighting

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsLightingMath.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsLightingMath.cpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsColorPacker.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsColorPacker.cpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`

- [ ] **Step 1: Add channel-wise lighting helpers**

```cpp
class NintendoDsLightingMath {
public:
    static float EvaluateDirectionalDiffuse(const float3& normal, const float3& lightDirection);
    static float ComputeLuminance(const float3& radiance);
    static float ApplyDisplayContrastCurve(float intensity);
    static float3 ApplyDisplayContrastCurve(const float3& color);
    static float3 ClampColor(const float3& color);
    static float3 MultiplyColor(const float3& left, const float3& right);
    static uint16_t ScalePackedGreyscale(float intensity);
    static float3 ComputeTriangleNormal(const float3& a, const float3& b, const float3& c);
};
```

- [ ] **Step 2: Implement channel-wise clamp, curve, and multiply helpers**

```cpp
float3 NintendoDsLightingMath::ClampColor(const float3& color) {
    return float3(
        std::clamp(color.X, 0.0f, 1.0f),
        std::clamp(color.Y, 0.0f, 1.0f),
        std::clamp(color.Z, 0.0f, 1.0f));
}

float3 NintendoDsLightingMath::ApplyDisplayContrastCurve(const float3& color) {
    return float3(
        ApplyDisplayContrastCurve(color.X),
        ApplyDisplayContrastCurve(color.Y),
        ApplyDisplayContrastCurve(color.Z));
}

float3 NintendoDsLightingMath::MultiplyColor(const float3& left, const float3& right) {
    return float3(left.X * right.X, left.Y * right.Y, left.Z * right.Z);
}
```

- [ ] **Step 3: Add a direct RGB packer if the existing API is awkward at the callsite**

```cpp
static uint16_t PackOpaqueColor(const float3& color);
```

```cpp
uint16_t NintendoDsColorPacker::PackOpaqueColor(const float3& color) {
    return PackOpaqueColor(color.X, color.Y, color.Z);
}
```

- [ ] **Step 4: Replace grayscale final color submission in `SubmitLitTriangle(...)`**

```cpp
float3 faceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexB, vertexC);
float diffuse = NintendoDsLightingMath::EvaluateDirectionalDiffuse(faceNormal, lightDirection);
float3 lighting = ambientRadiance + (directionalRadiance * diffuse);
lighting = NintendoDsLightingMath::ClampColor(lighting);

float3 shapedLighting = NintendoDsLightingMath::ApplyDisplayContrastCurve(lighting);
float3 finalColor = NintendoDsLightingMath::MultiplyColor(runtimeMaterial->BaseColor, shapedLighting);
finalColor = NintendoDsLightingMath::ClampColor(finalColor);

glColor(NintendoDsColorPacker::PackOpaqueColor(finalColor));
```

- [ ] **Step 5: Thread `runtimeMaterial` through `SubmitLitTriangle(...)`**

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

- [ ] **Step 6: Rebuild the editor-driven DS ROM and confirm export succeeds**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: DS export completes and writes a fresh `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`

- [ ] **Step 7: Commit the colored-lighting slice**

```bash
git add src/platform/ds/NintendoDsLightingMath.hpp src/platform/ds/NintendoDsLightingMath.cpp src/platform/ds/NintendoDsColorPacker.hpp src/platform/ds/NintendoDsColorPacker.cpp src/platform/ds/NintendoDsRenderManager3D.cpp
git commit -m "Render DS standard materials with lit base color"
```

---

### Task 3: Verify `colored_cube_grid` In `melonDS` And Preserve `cube_test`

**Files:**
- Modify temporarily: `C:\tmp\helengine-ds-city-cube-project\city\user_settings\build_config.json`
- Modify if needed: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`

- [ ] **Step 1: Switch DS scene selection to `colored_cube_grid` for runtime verification**

```json
{
  "platformId": "ds",
  "selectedSceneIds": [
    "colored_cube_grid"
  ],
  "sceneOrders": [
    {
      "orderNumber": 1,
      "sceneId": "colored_cube_grid"
    }
  ]
}
```

- [ ] **Step 2: Rebuild the DS export with `colored_cube_grid` as the selected DS scene**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: fresh ROM generated for the colored cube grid scene

- [ ] **Step 3: Launch the fresh ROM in `melonDS` and verify colored lit cubes**

Run:

```powershell
Start-Process -FilePath 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe' -ArgumentList 'C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds' -WindowStyle Hidden
```

Expected:

- distinct colored cubes are visible
- lighting changes as cubes rotate
- camera clear color remains correct
- no black-frame flicker
- no shutdown regression

- [ ] **Step 4: Restore the DS scene selection back to `cube_test`**

```json
{
  "platformId": "ds",
  "selectedSceneIds": [
    "cube_test"
  ],
  "sceneOrders": [
    {
      "orderNumber": 1,
      "sceneId": "cube_test"
    }
  ]
}
```

- [ ] **Step 5: Rebuild and verify `cube_test` still renders white-lit and rotating**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected:

- `cube_test` boots
- cube still rotates continuously
- no flicker
- no clear-color regression

- [ ] **Step 6: Commit any last verification-driven DS renderer adjustments**

```bash
git add src/platform/ds/NintendoDsRenderManager3D.cpp C:/tmp/helengine-ds-city-cube-project/city/user_settings/build_config.json
git commit -m "Verify DS standard material base color in colored cube grid"
```

---

## Self-Review

### Spec Coverage

- `BaseColorBuffer` decode path: Task 1
- DS runtime material base RGB storage: Task 1
- Replace grayscale final output with lit base color: Task 2
- White fallback for missing malformed color data: Task 1
- `colored_cube_grid` visual verification: Task 3
- `cube_test` regression check: Task 3

### Placeholder Scan

- No `TODO` or `TBD` placeholders remain.
- Commands, file paths, and target runtime files are explicit.

### Type Consistency

- `NintendoDsRuntimeMaterial::BaseColor` is introduced in Task 1 and consumed in Task 2.
- `TryResolveStandardMaterialBaseColor(...)` and `TryDecodeFloat4ConstantBuffer(...)` are declared before being used.
- `SubmitLitTriangle(...)` signature change in Task 2 matches the `runtimeMaterial` usage at the callsite.
