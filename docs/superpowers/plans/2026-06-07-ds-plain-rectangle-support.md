# DS Plain Rectangle Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render Nintendo DS `RoundedRectComponent` drawables as plain hardware rectangles, ignoring radius and corner flags, while keeping the renderer hardware-only and leaving true rounded corners unsupported.

**Architecture:** Extend `NintendoDsRenderManager2D` with one dedicated plain-rectangle path instead of routing rectangles through the existing runtime-texture sprite path. The new path owns frame-local DS OBJ graphics for solid-color fill and border strips, frees them at frame boundaries, and falls back to the existing unsupported marker path only when the rectangle cannot be honestly expressed within the current DS hardware contract.

**Tech Stack:** C++, libnds OBJ APIs, xUnit DS source-audit tests, PowerShell build wrapper, melonDS

---

## File Structure

- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Purpose: lock the new plain-rectangle contract so `DrawRoundedRect(...)` no longer remains an unconditional unsupported path and so `Radius` stays explicitly ignored on DS.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Purpose: declare the rectangle-specific hardware helpers, frame-local OBJ allocation tracking, and any small utility methods needed to decompose fill and border strips into DS sprite tiles.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Purpose: implement the plain-rectangle hardware path, release any temporary OBJ graphics safely between frames, and keep unsupported accounting honest when rectangle submission fails.

## Working Rules

- `RoundedRectComponent` means “plain rectangle” on DS for this pass.
- `Radius` and corner flags are ignored intentionally.
- No software 2D fallback is allowed.
- Rectangle rendering must use DS OBJ hardware only.
- Any new temporary DS OBJ graphics allocated for rectangles must be released deterministically to avoid VRAM leaks.
- Unsupported rectangle cases must still increment unsupported counters and show the existing magenta marker.

## Verification Baseline

Use these commands during the plan:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests|NintendoDsRenderManager3DSourceAuditTests|NintendoDsBootHostSourceAuditTests"
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project ..\..\helprojs\city\project.heproj -Platform ds -Output ..\..\helprojs\city\ds-build -Configuration Release
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected runtime outcome after the full plan:

- the two magenta markers to the left of `BACK` disappear
- the back button renders as a plain rectangular panel
- `UR` decreases for that screen

### Task 1: Lock The Plain-Rectangle Source Contract

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Add failing audit expectations for new rectangle helper declarations**

Add assertions that require the DS renderer header to declare a rectangle entry point plus frame-local cleanup state.

```csharp
Assert.Contains("bool TryDrawHardwareRectangle(IRoundedRectDrawable2D* shape);", headerSource, StringComparison.Ordinal);
Assert.Contains("bool TryDrawSolidHardwareRectangle(int32_t x, int32_t y, int32_t width, int32_t height, const byte4& color);", headerSource, StringComparison.Ordinal);
Assert.Contains("void ReleaseFrameLocalRectangleGraphics();", headerSource, StringComparison.Ordinal);
Assert.Contains("std::vector<void*> FrameLocalMainRectangleGraphics;", headerSource, StringComparison.Ordinal);
Assert.Contains("std::vector<void*> FrameLocalSubRectangleGraphics;", headerSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Add failing audit expectations that `DrawRoundedRect(...)` no longer hard-fails immediately**

Require the source to attempt hardware rectangle drawing before unsupported fallback.

```csharp
Assert.Contains("if (!TryDrawHardwareRectangle(shape))", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain(
    "ProfileUnsupportedPrimitiveCount++;\r\n        ProfileUnsupportedRoundedRectPrimitiveCount++;\r\n        LogUnsupportedDrawable(\"RoundedRect\", shape);",
    sourceCode,
    StringComparison.Ordinal);
```

- [ ] **Step 3: Add failing audit expectations that DS explicitly ignores radius**

Lock the “flatten to plain rectangle” behavior into the source contract.

```csharp
Assert.Contains("(void)shape->get_Radius();", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("shape->get_Radius() > 0", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("RoundedRectCorners", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 4: Add failing audit expectations for frame-local cleanup and solid-color tile generation**

Require the source to free temporary rectangle OBJ graphics and to generate solid-color tile payloads instead of creating a software framebuffer.

```csharp
Assert.Contains("ReleaseFrameLocalRectangleGraphics();", sourceCode, StringComparison.Ordinal);
Assert.Contains("oamFreeGfx(", sourceCode, StringComparison.Ordinal);
Assert.Contains("BuildSolidRectangleTilePixels(", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("TopCpuFrameBuffer", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 5: Run the focused audit to verify it fails**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: FAIL because the rectangle helper declarations and draw path do not exist yet.

- [ ] **Step 6: Commit the failing audit**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "test: lock DS plain rectangle contract"
```

### Task 2: Add A Frame-Local DS Hardware Rectangle Path

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Declare the rectangle helpers and frame-local allocation lists**

Add dedicated helpers to the header so the rectangle path does not pretend rectangles are runtime textures.

```cpp
        bool TryDrawHardwareRectangle(IRoundedRectDrawable2D* shape);
        bool TryDrawSolidHardwareRectangle(int32_t x, int32_t y, int32_t width, int32_t height, const byte4& color);
        void ReleaseFrameLocalRectangleGraphics();
        std::vector<uint16_t> BuildSolidRectangleTilePixels(int32_t tileWidth, int32_t tileHeight, int32_t filledWidth, int32_t filledHeight, uint16_t packedColor) const;
        std::vector<void*> FrameLocalMainRectangleGraphics;
        std::vector<void*> FrameLocalSubRectangleGraphics;
```

- [ ] **Step 2: Initialize the new frame-local storage and release it at frame start**

Extend the constructor and `BeginFrame()` so temporary rectangle graphics never leak across frames.

```cpp
        , FrameLocalMainRectangleGraphics()
        , FrameLocalSubRectangleGraphics()
```

```cpp
        ReleaseFrameLocalRectangleGraphics();
```

The cleanup helper should follow this shape:

```cpp
    void NintendoDsRenderManager2D::ReleaseFrameLocalRectangleGraphics() {
        for (void* graphics : FrameLocalMainRectangleGraphics) {
            if (graphics != nullptr) {
                oamFreeGfx(&oamMain, graphics);
            }
        }

        for (void* graphics : FrameLocalSubRectangleGraphics) {
            if (graphics != nullptr) {
                oamFreeGfx(&oamSub, graphics);
            }
        }

        FrameLocalMainRectangleGraphics.clear();
        FrameLocalSubRectangleGraphics.clear();
    }
```

- [ ] **Step 3: Rework `DrawRoundedRect(...)` to attempt hardware rectangle rendering first**

Change the existing unconditional skip path into the same supported-then-unsupported shape already used by text and sprites.

```cpp
    void NintendoDsRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
        uint32_t timingStartTicks = cpuGetTiming();
        ProfileRoundedRectPrimitiveCount++;
        if (!TryDrawHardwareRectangle(shape)) {
            ProfileUnsupportedPrimitiveCount++;
            ProfileUnsupportedRoundedRectPrimitiveCount++;
            LogUnsupportedDrawable("RoundedRect", shape);
            int2 markerPosition = ResolveUnsupportedDrawableMarkerPosition(shape);
            DrawUnsupportedDrawableMarker(markerPosition.X, markerPosition.Y, ActiveViewportTargetsBottomScreen ? NintendoDsScreenTarget::Bottom : NintendoDsScreenTarget::Top);
        }

        ProfileRoundedRectMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - timingStartTicks);
    }
```

- [ ] **Step 4: Implement `TryDrawHardwareRectangle(...)` with fill-plus-border strip decomposition**

Use the rounded-rect component as a plain rectangle and ignore radius deliberately.

```cpp
    bool NintendoDsRenderManager2D::TryDrawHardwareRectangle(IRoundedRectDrawable2D* shape) {
        if (shape == nullptr) {
            return false;
        }

        Entity* parent = shape->get_Parent();
        if (parent == nullptr) {
            return false;
        }

        (void)shape->get_Radius();
        int2 size = shape->get_Size();
        if (size.X <= 0 || size.Y <= 0) {
            return false;
        }

        float3 parentPosition = parent->get_Position();
        int32_t screenX = static_cast<int32_t>(std::round(parentPosition.X)) + ActiveViewportOffsetX;
        int32_t screenY = static_cast<int32_t>(std::round(parentPosition.Y)) + ActiveViewportOffsetY;
        int32_t borderThickness = std::max(static_cast<int32_t>(0), static_cast<int32_t>(std::round(shape->get_BorderThickness())));
        byte4 fillColor = shape->get_FillColor();
        byte4 borderColor = shape->get_BorderColor();

        bool drewAnyPixels = false;
        if (fillColor.W == 255) {
            int32_t inset = std::min(borderThickness, std::min(size.X / 2, size.Y / 2));
            int32_t fillX = screenX + inset;
            int32_t fillY = screenY + inset;
            int32_t fillWidth = size.X - (inset * 2);
            int32_t fillHeight = size.Y - (inset * 2);
            if (fillWidth > 0 && fillHeight > 0) {
                drewAnyPixels = TryDrawSolidHardwareRectangle(fillX, fillY, fillWidth, fillHeight, fillColor) || drewAnyPixels;
            }
        } else if (fillColor.W != 0) {
            return false;
        }

        if (borderThickness > 0) {
            if (borderColor.W != 255) {
                return false;
            }

            drewAnyPixels = TryDrawSolidHardwareRectangle(screenX, screenY, size.X, borderThickness, borderColor) || drewAnyPixels;
            drewAnyPixels = TryDrawSolidHardwareRectangle(screenX, screenY + size.Y - borderThickness, size.X, borderThickness, borderColor) || drewAnyPixels;
            drewAnyPixels = TryDrawSolidHardwareRectangle(screenX, screenY + borderThickness, borderThickness, size.Y - (borderThickness * 2), borderColor) || drewAnyPixels;
            drewAnyPixels = TryDrawSolidHardwareRectangle(screenX + size.X - borderThickness, screenY + borderThickness, borderThickness, size.Y - (borderThickness * 2), borderColor) || drewAnyPixels;
        } else if (borderColor.W != 0 && borderColor.W != 255) {
            return false;
        }

        return drewAnyPixels;
    }
```

- [ ] **Step 5: Implement `TryDrawSolidHardwareRectangle(...)` with DS tile-span decomposition**

Build per-tile solid-color OBJ payloads directly, reuse the existing span logic, and register every temporary allocation in the per-frame list for cleanup.

```cpp
    bool NintendoDsRenderManager2D::TryDrawSolidHardwareRectangle(int32_t x, int32_t y, int32_t width, int32_t height, const byte4& color) {
        if (width <= 0 || height <= 0 || color.W != 255) {
            return false;
        }

        std::vector<int32_t> tileWidths;
        std::vector<int32_t> tileHeights;
        BuildHardwareSpriteTileSpans(width, tileWidths);
        BuildHardwareSpriteTileSpans(height, tileHeights);
        if (tileWidths.empty() || tileHeights.empty()) {
            return false;
        }

        bool targetBottomScreen = ActiveViewportTargetsBottomScreen;
        OamState* oamState = targetBottomScreen ? &oamSub : &oamMain;
        std::vector<void*>& frameLocalGraphics = targetBottomScreen ? FrameLocalSubRectangleGraphics : FrameLocalMainRectangleGraphics;
        if (targetBottomScreen) {
            if (!SubSpriteEngineInitialized) {
                vramSetBankI(VRAM_I_SUB_SPRITE);
                oamInit(&oamSub, SpriteMapping_1D_32, false);
                oamClear(&oamSub, 0, 128);
                SubSpriteEngineInitialized = true;
            }
        } else if (!MainSpriteEngineInitialized) {
            vramSetBankG(VRAM_G_MAIN_SPRITE);
            oamInit(&oamMain, SpriteMapping_1D_32, false);
            oamClear(&oamMain, 0, 128);
            MainSpriteEngineInitialized = true;
        }

        int32_t nextSpriteId = targetBottomScreen ? NextSubDebugMarkerSpriteId : NextMainDebugMarkerSpriteId;
        int32_t tileCount = static_cast<int32_t>(tileWidths.size() * tileHeights.size());
        if (nextSpriteId + tileCount > 128) {
            return false;
        }

        uint16_t packedColor = static_cast<uint16_t>(
            BIT(15)
            | ((color.X >> 3) & 31)
            | (((color.Y >> 3) & 31) << 5)
            | (((color.Z >> 3) & 31) << 10));

        int32_t remainingHeight = height;
        int32_t drawY = y;
        for (int32_t tileHeight : tileHeights) {
            int32_t filledHeight = std::min(tileHeight, remainingHeight);
            int32_t remainingWidth = width;
            int32_t drawX = x;
            for (int32_t tileWidth : tileWidths) {
                int32_t filledWidth = std::min(tileWidth, remainingWidth);
                SpriteSize spriteSize = SpriteSize_8x8;
                if (tileWidth == 8 && tileHeight == 16) {
                    spriteSize = SpriteSize_8x16;
                } else if (tileWidth == 8 && tileHeight == 32) {
                    spriteSize = SpriteSize_8x32;
                } else if (tileWidth == 16 && tileHeight == 8) {
                    spriteSize = SpriteSize_16x8;
                } else if (tileWidth == 16 && tileHeight == 16) {
                    spriteSize = SpriteSize_16x16;
                } else if (tileWidth == 16 && tileHeight == 32) {
                    spriteSize = SpriteSize_16x32;
                } else if (tileWidth == 32 && tileHeight == 8) {
                    spriteSize = SpriteSize_32x8;
                } else if (tileWidth == 32 && tileHeight == 16) {
                    spriteSize = SpriteSize_32x16;
                } else if (tileWidth == 32 && tileHeight == 32) {
                    spriteSize = SpriteSize_32x32;
                }

                void* tileGraphics = oamAllocateGfx(oamState, spriteSize, SpriteColorFormat_Bmp);
                if (tileGraphics == nullptr) {
                    return false;
                }

                std::vector<uint16_t> tilePixels = BuildSolidRectangleTilePixels(tileWidth, tileHeight, filledWidth, filledHeight, packedColor);
                std::memcpy(tileGraphics, tilePixels.data(), tilePixels.size() * sizeof(uint16_t));
                frameLocalGraphics.push_back(tileGraphics);

                oamSet(oamState, nextSpriteId++, drawX, drawY, 0, 0, spriteSize, SpriteColorFormat_Bmp, tileGraphics, -1, false, false, false, false, false);
                drawX += tileWidth;
                remainingWidth -= filledWidth;
            }

            drawY += tileHeight;
            remainingHeight -= filledHeight;
        }

        if (targetBottomScreen) {
            NextSubDebugMarkerSpriteId = nextSpriteId;
        } else {
            NextMainDebugMarkerSpriteId = nextSpriteId;
        }

        oamUpdate(oamState);
        return true;
    }
```

- [ ] **Step 6: Implement `BuildSolidRectangleTilePixels(...)`**

Generate a padded tile buffer where only the requested fill region is opaque and the rest stays transparent.

```cpp
    std::vector<uint16_t> NintendoDsRenderManager2D::BuildSolidRectangleTilePixels(int32_t tileWidth, int32_t tileHeight, int32_t filledWidth, int32_t filledHeight, uint16_t packedColor) const {
        std::vector<uint16_t> tilePixels(static_cast<std::size_t>(tileWidth * tileHeight), 0);
        for (int32_t pixelY = 0; pixelY < filledHeight; pixelY++) {
            for (int32_t pixelX = 0; pixelX < filledWidth; pixelX++) {
                tilePixels[static_cast<std::size_t>(pixelY * tileWidth + pixelX)] = packedColor;
            }
        }

        return tilePixels;
    }
```

- [ ] **Step 7: Run the focused source audit to verify the rectangle contract passes**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: PASS.

- [ ] **Step 8: Commit the rectangle path**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "feat: add DS plain rectangle path"
```

### Task 3: Verify The DS Overlay Panel End-To-End

**Files:**
- Test only: runtime build and emulator verification

- [ ] **Step 1: Run the focused DS audit set**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests|NintendoDsRenderManager3DSourceAuditTests|NintendoDsBootHostSourceAuditTests"
```

Expected: PASS.

- [ ] **Step 2: Build the release DS ROM with the shared wrapper**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 -Project ..\..\helprojs\city\project.heproj -Platform ds -Output ..\..\helprojs\city\ds-build -Configuration Release
```

Expected: PASS and fresh `helengine_ds.nds` output.

- [ ] **Step 3: Launch the rebuilt ROM in melonDS**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected: melonDS launches the rebuilt ROM.

- [ ] **Step 4: Verify the visual result**

Check these exact runtime conditions:

```text
- no magenta markers remain to the left of BACK
- BACK still renders as text
- the button panel is visible as a plain rectangle
- the panel is not rounded, and that is acceptable
- UR is lower than before on the bottom screen
```

- [ ] **Step 5: Commit the verified state**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp
git commit -m "fix: render DS overlay panels as plain rectangles"
```
