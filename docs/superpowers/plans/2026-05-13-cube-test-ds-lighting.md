# Cube Test DS Lighting Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a first-pass Nintendo DS lighting path for `cube_test` so the rotating cube is shaded by the existing scene directional light with a small ambient floor.

**Architecture:** Keep the current DS immediate-mode geometry path and add one narrow lighting layer inside the native renderer. Resolve the first active directional light from the loaded runtime scene, compute one face normal per submitted triangle, evaluate `ambient + diffuse`, and convert the intensity into DS polygon color without introducing full material or per-vertex normal support.

**Tech Stack:** C++, libnds DS 3D API, generated-core runtime scene/components, melonDS manual verification

---

### Task 1: Add Testable DS Lighting Math Helpers

**Files:**
- Create: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsLightingMath.hpp`
- Create: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsLightingMath.cpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
- Test: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsLightingMath.hpp` through temporary compile use in renderer

- [ ] **Step 1: Write the failing helper integration**

Add includes and temporary calls in `NintendoDsRenderManager3D.cpp` that reference these new helper signatures before the helpers exist:

```cpp
#include "platform/ds/NintendoDsLightingMath.hpp"

float faceLighting = NintendoDsLightingMath::EvaluateDirectionalDiffuseWithAmbient(
    triangleNormal,
    lightDirection,
    0.2f);
uint16_t litColor = NintendoDsLightingMath::ScalePackedGreyscale(faceLighting);
```

Expected compile result before helper creation: missing include / missing symbol errors for `NintendoDsLightingMath`.

- [ ] **Step 2: Run build to verify the integration fails**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath=C:\dev\helworks\helengine\.codex-build\bin\ -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: FAIL with unresolved `NintendoDsLightingMath` include or symbol errors.

- [ ] **Step 3: Write the minimal helper implementation**

Create `NintendoDsLightingMath.hpp` with:

```cpp
#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdint>

#include "float3.hpp"

namespace helengine::ds {
    class NintendoDsLightingMath {
    public:
        static float EvaluateDirectionalDiffuseWithAmbient(const float3& normal, const float3& lightDirection, float ambientFloor);
        static uint16_t ScalePackedGreyscale(float intensity);
        static float3 ComputeTriangleNormal(const float3& a, const float3& b, const float3& c);
    };
}
#endif
```

Create `NintendoDsLightingMath.cpp` with:

```cpp
#include "platform/ds/NintendoDsLightingMath.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <algorithm>

namespace helengine::ds {
    float NintendoDsLightingMath::EvaluateDirectionalDiffuseWithAmbient(const float3& normal, const float3& lightDirection, float ambientFloor) {
        float3 normalizedNormal = normal;
        normalizedNormal.Normalize();

        float3 normalizedLightDirection = lightDirection;
        normalizedLightDirection.Normalize();

        float diffuse = float3::Dot(normalizedNormal, normalizedLightDirection * -1.0f);
        if (diffuse < 0.0f) {
            diffuse = 0.0f;
        }

        float intensity = ambientFloor + ((1.0f - ambientFloor) * diffuse);
        return std::clamp(intensity, 0.0f, 1.0f);
    }

    uint16_t NintendoDsLightingMath::ScalePackedGreyscale(float intensity) {
        float clamped = std::clamp(intensity, 0.0f, 1.0f);
        uint16_t channel = static_cast<uint16_t>(clamped * 31.0f + 0.5f);
        return RGB15(channel, channel, channel);
    }

    float3 NintendoDsLightingMath::ComputeTriangleNormal(const float3& a, const float3& b, const float3& c) {
        float3 ab = b - a;
        float3 ac = c - a;
        float3 normal = float3::Cross(ab, ac);
        normal.Normalize();
        return normal;
    }
}
#endif
```

- [ ] **Step 4: Update the renderer to use the helpers minimally**

Modify `NintendoDsRenderManager3D.cpp` so triangle submission computes one face normal and one lit polygon color before each emitted triangle batch.

Use this shape inside the indexed submission loops:

```cpp
float3 a = TransformVertex(drawable, (*positions)[indexA]);
float3 b = TransformVertex(drawable, (*positions)[indexB]);
float3 c = TransformVertex(drawable, (*positions)[indexC]);

float3 faceNormal = NintendoDsLightingMath::ComputeTriangleNormal(a, b, c);
float intensity = NintendoDsLightingMath::EvaluateDirectionalDiffuseWithAmbient(faceNormal, resolvedLightDirection, 0.2f);
glColor(NintendoDsLightingMath::ScalePackedGreyscale(intensity));
glVertex3v16(floattov16(a.X), floattov16(a.Y), floattov16(a.Z));
glVertex3v16(floattov16(b.X), floattov16(b.Y), floattov16(b.Z));
glVertex3v16(floattov16(c.X), floattov16(c.Y), floattov16(c.Z));
```

- [ ] **Step 5: Run build to verify it passes**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath=C:\dev\helworks\helengine\.codex-build\bin\ -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: PASS, DS ROM produced.

- [ ] **Step 6: Commit**

```bash
git -C C:\dev\helworks\helengine-ds add src/platform/ds/NintendoDsLightingMath.hpp src/platform/ds/NintendoDsLightingMath.cpp src/platform/ds/NintendoDsRenderManager3D.cpp
git -C C:\dev\helworks\helengine-ds commit -m "Add DS face-lighting math helpers"
```

### Task 2: Resolve the Runtime Directional Light in the DS Renderer

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.hpp`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
- Reference: `C:\dev\helworks\helengine\engine\helengine.core\utils\LightDirectionUtility.cs`
- Reference: `C:\dev\helworks\helengine\engine\helengine.core\components\DirectionalLightComponent.cs`

- [ ] **Step 1: Write the failing renderer integration**

Add new private renderer helpers before implementing them:

```cpp
bool TryResolveDirectionalLight(float3& lightDirection, float& lightIntensity) const;
```

and call them from `DrawRenderQueue(...)` or `SubmitOpaqueDrawable(...)`:

```cpp
float3 lightDirection = float3(0.0f, -1.0f, 0.0f);
float lightIntensity = 1.0f;
TryResolveDirectionalLight(lightDirection, lightIntensity);
```

Expected compile result before implementation: missing method definition.

- [ ] **Step 2: Run build to verify it fails**

Run the same editor DS build command.

Expected: FAIL with unresolved `TryResolveDirectionalLight`.

- [ ] **Step 3: Implement the minimal light resolution**

Add one helper that walks runtime scene objects through `Core::Instance->get_ObjectManager()`, finds the first `DirectionalLightComponent`, resolves its parent entity orientation, and derives a forward light direction matching the engine’s directional-light convention.

Implementation shape:

```cpp
bool NintendoDsRenderManager3D::TryResolveDirectionalLight(float3& lightDirection, float& lightIntensity) const {
    Core* core = Core::get_Instance();
    if (core == nullptr || core->get_ObjectManager() == nullptr) {
        return false;
    }

    List<LightComponent*>* lights = core->get_ObjectManager()->get_Lights();
    if (lights == nullptr) {
        return false;
    }

    for (int32_t index = 0; index < lights->Count(); index++) {
        LightComponent* light = (*lights)[index];
        DirectionalLightComponent* directional = dynamic_cast<DirectionalLightComponent*>(light);
        if (directional == nullptr) {
            continue;
        }

        Entity* entity = directional->get_Parent();
        float4 orientation = entity != nullptr ? entity->get_Orientation() : float4::get_Identity();
        lightDirection = float4::RotateVector(float3(0.0f, 0.0f, -1.0f), orientation);
        lightIntensity = directional->get_Intensity();
        return true;
    }

    return false;
}
```

- [ ] **Step 4: Clamp the resolved light into the face-lighting path**

Scale the evaluated intensity by the resolved light intensity, but keep the ambient floor:

```cpp
float lit = NintendoDsLightingMath::EvaluateDirectionalDiffuseWithAmbient(faceNormal, lightDirection, 0.2f);
lit *= lightIntensity;
if (lit > 1.0f) {
    lit = 1.0f;
}
```

- [ ] **Step 5: Run build to verify it passes**

Run the same editor DS build command.

Expected: PASS, DS ROM produced.

- [ ] **Step 6: Commit**

```bash
git -C C:\dev\helworks\helengine-ds add src/platform/ds/NintendoDsRenderManager3D.hpp src/platform/ds/NintendoDsRenderManager3D.cpp
git -C C:\dev\helworks\helengine-ds commit -m "Resolve scene directional light in DS renderer"
```

### Task 3: Restructure Triangle Submission for Per-Face Lit Polygons

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
- Test: `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds` in melonDS

- [ ] **Step 1: Write the failing triangle-path change**

Change `SubmitOpaqueDrawable(...)` temporarily to require triangle-grouped indices explicitly:

```cpp
if ((runtimeModel->Indices16 != nullptr) && (runtimeModel->Indices16->Length % 3 != 0)) {
    throw new InvalidOperationException("DS triangle lighting requires triangle-grouped index buffers.");
}
```

Expected result before loop refactor: the build passes, but this highlights the assumption we are about to depend on.

- [ ] **Step 2: Run ROM in melonDS to verify current visual baseline**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -Command "`$melonDsPath = 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe'; `$romPath = 'C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'; Start-Process -FilePath `$melonDsPath -ArgumentList `$romPath"
```

Expected: cube still renders with flat white polygons before the lighting loop rewrite is finished.

- [ ] **Step 3: Rewrite the draw loops around triangle batches**

Convert each index loop into `index += 3` batches so one face normal and one DS polygon color are computed per triangle:

```cpp
for (int32_t index = 0; index + 2 < runtimeModel->Indices16->Length; index += 3) {
    uint16_t indexA = (*runtimeModel->Indices16)[index];
    uint16_t indexB = (*runtimeModel->Indices16)[index + 1];
    uint16_t indexC = (*runtimeModel->Indices16)[index + 2];
    SubmitLitTriangle(drawable, positions, indexA, indexB, indexC, lightDirection, lightIntensity);
}
```

Add one local helper or small private method to keep the per-triangle path readable.

- [ ] **Step 4: Run build to verify it passes**

Run the same editor DS build command.

Expected: PASS, DS ROM produced.

- [ ] **Step 5: Verify in melonDS**

Run melonDS with the rebuilt ROM.

Expected visual result:
- cube still rotates continuously
- cube no longer renders uniformly white
- visible faces brighten/dim as they rotate under the authored directional light
- no return of black-frame flicker or shutdown

- [ ] **Step 6: Commit**

```bash
git -C C:\dev\helworks\helengine-ds add src/platform/ds/NintendoDsRenderManager3D.cpp
git -C C:\dev\helworks\helengine-ds commit -m "Shade DS cube faces from directional light"
```

### Task 4: Final Verification and Cleanup

**Files:**
- Verify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager3D.cpp`
- Verify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsLightingMath.cpp`
- Verify: `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`

- [ ] **Step 1: Run the full focused DS build one more time**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath=C:\dev\helworks\helengine\.codex-build\bin\ -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: PASS, DS ROM produced.

- [ ] **Step 2: Verify visually in melonDS**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -Command "`$melonDsPath = 'C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe'; `$romPath = 'C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds'; Start-Process -FilePath `$melonDsPath -ArgumentList `$romPath"
```

Expected:
- boot succeeds
- cube rotates continuously
- lighting reads clearly on the cube

- [ ] **Step 3: Check git state**

Run:

```bash
git -C C:\dev\helworks\helengine-ds status --short
```

Expected: only intended DS lighting files are modified.

- [ ] **Step 4: Commit final cleanup if needed**

```bash
git -C C:\dev\helworks\helengine-ds add src/platform/ds/NintendoDsLightingMath.hpp src/platform/ds/NintendoDsLightingMath.cpp src/platform/ds/NintendoDsRenderManager3D.cpp src/platform/ds/NintendoDsRenderManager3D.hpp
git -C C:\dev\helworks\helengine-ds commit -m "Finalize DS cube-test lighting pass"
```
