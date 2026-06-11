# DS Hardware Sprite First Pass Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add the first real Nintendo DS hardware-backed 2D sprite path while keeping unsupported work skipped and reducing debug unsupported-log spam to once per category per frame.

**Architecture:** Keep `NintendoDsRenderManager2D` as the hardware-only policy gate and add one narrow top-screen OBJ sprite submission path there. Store OBJ-ready sprite payload ownership on `NintendoDsRuntimeTexture2D`, reset unsupported-log state in `BeginFrame()`, and keep text unsupported for this slice.

**Tech Stack:** C++, libnds OAM/OBJ APIs, xUnit source-audit tests, PowerShell build wrappers

---

## File Structure

- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Purpose: lock the new contract for per-frame unsupported-log throttling and narrow sprite-path behavior.
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.hpp`
  Purpose: declare DS-owned sprite cache fields needed for plain OBJ submission.
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.cpp`
  Purpose: initialize new sprite cache fields safely.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Purpose: declare per-frame unsupported-log flags and narrow sprite helper methods.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Purpose: implement per-frame unsupported-log throttling, sprite cache build/release, and first-pass top-screen sprite submission.

## Verified Runtime Surface

The shared-engine and generated C++ surfaces for this plan are:

- `ISpriteDrawable2D`: `get_Texture()`, `get_Rotation()`, `get_Color()`, `get_SourceRect()`, `get_Size()`
- `IDrawable2D`: `get_Parent()`
- `Entity`: `get_Position()`, `get_Scale()`
- `RuntimeTexture`: `get_Width()`, `get_Height()`

The implementation tasks below must use those verified accessors rather than any guessed helper like `ResolveTexture()`.

### Task 1: Lock the New 2D Contract in Source Audits

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Write the failing audit for per-frame unsupported-log throttling**

Add assertions to `Source_whenRouting2dDrawables_definesHardwareOnlyHelpersAndUnsupportedMarkerFlow` that require per-frame flags and category guards.

```csharp
Assert.Contains("bool UnsupportedSpriteLoggedThisFrame;", headerSource, StringComparison.Ordinal);
Assert.Contains("bool UnsupportedTextLoggedThisFrame;", headerSource, StringComparison.Ordinal);
Assert.Contains("bool UnsupportedRoundedRectLoggedThisFrame;", headerSource, StringComparison.Ordinal);
Assert.Contains("UnsupportedSpriteLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
Assert.Contains("UnsupportedTextLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
Assert.Contains("UnsupportedRoundedRectLoggedThisFrame = false;", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 2: Write the failing audit for the narrow sprite path**

Add assertions that `TryDrawHardwareSprite(...)` is no longer an unconditional `false`, that it reads the verified sprite accessors, and that text remains explicitly unsupported.

```csharp
Assert.DoesNotContain("bool NintendoDsRenderManager2D::TryDrawHardwareSprite(ISpriteDrawable2D* sprite) {\r\n        if (sprite == nullptr) {\r\n            return false;\r\n        }\r\n\r\n        return false;\r\n    }", sourceCode, StringComparison.Ordinal);
Assert.Contains("sprite->get_Texture()", sourceCode, StringComparison.Ordinal);
Assert.Contains("sprite->get_Rotation()", sourceCode, StringComparison.Ordinal);
Assert.Contains("sprite->get_Color()", sourceCode, StringComparison.Ordinal);
Assert.Contains("sprite->get_SourceRect()", sourceCode, StringComparison.Ordinal);
Assert.Contains("sprite->get_Size()", sourceCode, StringComparison.Ordinal);
Assert.Contains("sprite->get_Parent()", sourceCode, StringComparison.Ordinal);
Assert.Contains("bool NintendoDsRenderManager2D::TryDrawHardwareText(ITextDrawable2D* text)", sourceCode, StringComparison.Ordinal);
Assert.Contains("return false;", sourceCode, StringComparison.Ordinal);
Assert.Contains("oamSet(", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 3: Run the focused audit to verify it fails**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj --no-restore --no-build --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: FAIL because the new throttling fields and sprite-path source markers are not present yet.

- [ ] **Step 4: Commit the failing test change**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "test: lock DS sprite first-pass source contract"
```

### Task 2: Extend Runtime Texture State for DS OBJ Sprite Ownership

**Files:**
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.hpp`
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.cpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Add the minimal sprite cache fields to the runtime texture type**

Add only ownership and readiness state required for the first-pass OBJ path.

```cpp
        /// DS OBJ graphics payload allocated for plain top-screen sprite submission.
        void* HardwareSpriteGraphics;

        /// True once the plain OBJ sprite payload has been prepared for DS submission.
        bool HardwareSpritePrepared;

        /// Width of the prepared DS OBJ sprite payload in pixels.
        int32_t HardwareSpriteWidth;

        /// Height of the prepared DS OBJ sprite payload in pixels.
        int32_t HardwareSpriteHeight;
```

- [ ] **Step 2: Initialize the new cache fields in the runtime texture constructor**

```cpp
        , HardwareSpriteGraphics(nullptr)
        , HardwareSpritePrepared(false)
        , HardwareSpriteWidth(0)
        , HardwareSpriteHeight(0)
```

- [ ] **Step 3: Release the new cache fields from `ReleaseTexture(...)`**

Extend the DS texture release path so any prepared OBJ sprite memory is released with the runtime texture.

```cpp
        if (dsTexture->HardwareSpriteGraphics != nullptr) {
            oamFreeGfx(&oamMain, dsTexture->HardwareSpriteGraphics);
            dsTexture->HardwareSpriteGraphics = nullptr;
        }
        dsTexture->HardwareSpritePrepared = false;
        dsTexture->HardwareSpriteWidth = 0;
        dsTexture->HardwareSpriteHeight = 0;
```

- [ ] **Step 4: Run the source audit to verify the cache fields satisfy the new contract**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj --no-restore --no-build --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: still FAIL, but no longer on missing runtime texture state once the next tasks land.

- [ ] **Step 5: Commit the runtime texture state change**

```bash
git add src/platform/ds/NintendoDsRuntimeTexture2D.hpp src/platform/ds/NintendoDsRuntimeTexture2D.cpp src/platform/ds/NintendoDsRenderManager2D.cpp
git commit -m "feat: add DS runtime sprite cache state"
```

### Task 3: Throttle Unsupported Diagnostics to Once Per Category Per Frame

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Add per-frame unsupported-log flags to the renderer header**

```cpp
        /// True once one unsupported sprite diagnostic has already been logged during the active frame.
        bool UnsupportedSpriteLoggedThisFrame;

        /// True once one unsupported text diagnostic has already been logged during the active frame.
        bool UnsupportedTextLoggedThisFrame;

        /// True once one unsupported rounded-rectangle diagnostic has already been logged during the active frame.
        bool UnsupportedRoundedRectLoggedThisFrame;
```

- [ ] **Step 2: Initialize and reset the flags in the constructor and `BeginFrame()`**

```cpp
        , UnsupportedSpriteLoggedThisFrame(false)
        , UnsupportedTextLoggedThisFrame(false)
        , UnsupportedRoundedRectLoggedThisFrame(false)
```

```cpp
        UnsupportedSpriteLoggedThisFrame = false;
        UnsupportedTextLoggedThisFrame = false;
        UnsupportedRoundedRectLoggedThisFrame = false;
```

- [ ] **Step 3: Gate `LogUnsupportedDrawable(...)` by category**

Keep the existing log text, but return early once the frame has already logged that category.

```cpp
        if (std::strcmp(safeCategory, "Sprite") == 0) {
            if (UnsupportedSpriteLoggedThisFrame) {
                return;
            }

            UnsupportedSpriteLoggedThisFrame = true;
        } else if (std::strcmp(safeCategory, "Text") == 0) {
            if (UnsupportedTextLoggedThisFrame) {
                return;
            }

            UnsupportedTextLoggedThisFrame = true;
        } else if (std::strcmp(safeCategory, "RoundedRect") == 0) {
            if (UnsupportedRoundedRectLoggedThisFrame) {
                return;
            }

            UnsupportedRoundedRectLoggedThisFrame = true;
        }
```

- [ ] **Step 4: Run the focused audit to verify the throttling path passes**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj --no-restore --no-build --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: fewer or no failures related to throttling; sprite-path assertions may still fail until Task 4 is complete.

- [ ] **Step 5: Commit the unsupported-log throttling change**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp
git commit -m "feat: throttle DS unsupported 2D diagnostics"
```

### Task 4: Implement Narrow Top-Screen Plain Sprite Submission

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.hpp`
- Modify: `src/platform/ds/NintendoDsRuntimeTexture2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Declare helper methods for the narrow sprite path**

Add helpers instead of expanding `TryDrawHardwareSprite(...)` into one large block.

```cpp
        bool TryPrepareHardwareSpriteGraphics(NintendoDsRuntimeTexture2D* runtimeTexture, SpriteSize& spriteSize, SpriteColorFormat& colorFormat);
        bool IsHardwareSpriteFormatSupported(NintendoDsRuntimeTexture2D* runtimeTexture) const;
        bool TryResolveHardwareSpriteSize(const ::int2& drawableSize, SpriteSize& spriteSize, SpriteColorFormat& colorFormat) const;
        std::vector<uint16_t> BuildHardwareSpritePixels(NintendoDsRuntimeTexture2D* runtimeTexture) const;
```

- [ ] **Step 2: Validate the first-pass contract inside `TryDrawHardwareSprite(...)`**

Reject anything that is not top-screen, missing a parent, missing a texture, rotated, tinted, cropped, zero-sized, or not one-OBJ sized.

```cpp
        if (sprite == nullptr || ActiveViewportTargetsBottomScreen) {
            return false;
        }

        Entity* parent = sprite->get_Parent();
        if (parent == nullptr) {
            return false;
        }

        if (sprite->get_Rotation() != 0.0f) {
            return false;
        }

        byte4 color = sprite->get_Color();
        if (color.X != 255 || color.Y != 255 || color.Z != 255 || color.W != 255) {
            return false;
        }

        float4 sourceRect = sprite->get_SourceRect();
        if (sourceRect.X != 0.0f || sourceRect.Y != 0.0f || sourceRect.Z != 1.0f || sourceRect.W != 1.0f) {
            return false;
        }

        int2 drawableSize = sprite->get_Size();
        if (drawableSize.X <= 0 || drawableSize.Y <= 0) {
            return false;
        }

        RuntimeTexture* runtimeTextureBase = sprite->get_Texture();
        NintendoDsRuntimeTexture2D* runtimeTexture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(runtimeTextureBase);
        if (runtimeTexture == nullptr) {
            return false;
        }

        if (!TryPrepareHardwareSpriteGraphics(runtimeTexture, spriteSize, colorFormat)) {
            return false;
        }
```

- [ ] **Step 3: Build OBJ-ready graphics only for accepted texture formats and sizes**

Restrict the first pass to formats already published by the DS pipeline and to one-OBJ dimensions.

```cpp
        if (runtimeTexture->ColorFormat != TextureAssetColorFormat::Rgba4444
            && runtimeTexture->ColorFormat != TextureAssetColorFormat::Indexed4
            && runtimeTexture->ColorFormat != TextureAssetColorFormat::Indexed8) {
            return false;
        }

        if (runtimeTexture->get_Width() != drawableSize.X || runtimeTexture->get_Height() != drawableSize.Y) {
            return false;
        }

        if (!TryResolveHardwareSpriteSize(drawableSize, spriteSize, colorFormat)) {
            return false;
        }
```

- [ ] **Step 4: Submit one main-screen OBJ entry**

Reuse the same OAM family already used by the debug markers, but allocate sprite ids from the normal main-screen counter and place the sprite from its parent entity world position.

```cpp
        float3 parentPosition = parent->get_Position();
        int32_t clampedX = std::clamp(static_cast<int32_t>(std::round(parentPosition.X)) + ActiveViewportOffsetX, static_cast<int32_t>(0), static_cast<int32_t>(FrameBufferWidth - drawableSize.X));
        int32_t clampedY = std::clamp(static_cast<int32_t>(std::round(parentPosition.Y)) + ActiveViewportOffsetY, static_cast<int32_t>(0), static_cast<int32_t>(VisibleScreenHeight - drawableSize.Y));
        oamSet(
            &oamMain,
            NextMainDebugMarkerSpriteId,
            clampedX,
            clampedY,
            0,
            0,
            spriteSize,
            colorFormat,
            runtimeTexture->HardwareSpriteGraphics,
            -1,
            false,
            false,
            false,
            false,
            false);
        NextMainDebugMarkerSpriteId++;
        oamUpdate(&oamMain);
        return true;
```

- [ ] **Step 5: Keep text explicitly unsupported in this slice**

Do not broaden scope during sprite work.

```cpp
    bool NintendoDsRenderManager2D::TryDrawHardwareText(ITextDrawable2D* text) {
        if (text == nullptr) {
            return false;
        }

        return false;
    }
```

- [ ] **Step 6: Run the focused audit to verify the 2D contract passes**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj --no-restore --no-build --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: PASS.

- [ ] **Step 7: Commit the hardware sprite path**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp src/platform/ds/NintendoDsRuntimeTexture2D.hpp src/platform/ds/NintendoDsRuntimeTexture2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "feat: add DS hardware sprite first pass"
```

### Task 5: Run Focused Verification and Build a DS ROM

**Files:**
- Modify: none
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Run the focused DS renderer audit suite**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj --no-restore --no-build --filter "NintendoDsRenderManager2DSourceAuditTests|NintendoDsRenderManager3DSourceAuditTests|NintendoDsBootHostSourceAuditTests"
```

Expected: PASS with the DS 2D, 3D, and boot-host source audits green.

- [ ] **Step 2: Rebuild the DS ROM**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\..\helprojs\city\project.heproj `
  -Platform ds `
  -Output ..\..\helprojs\city\ds-build `
  -Configuration Release
```

Expected: build completes and updates `..\..\helprojs\city\ds-build\helengine_ds.nds`.

- [ ] **Step 3: Launch the rebuilt ROM for manual verification**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected: melonDS launches the rebuilt ROM so the user can confirm that some plain sprites now render and unsupported spam is reduced.

Expected user-visible constraints:

- plain top-screen sprites may appear
- text still skips
- rotated, tinted, cropped, bottom-screen, or oversize sprites still skip
- unsupported log spam is reduced to once per category per frame

- [ ] **Step 4: Commit verification-only follow-ups if needed**

```bash
git status --short
```

Expected: no new source changes unless a tiny verification follow-up was intentionally made.
