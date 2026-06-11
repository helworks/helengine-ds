# DS Background Text Without Console Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make Nintendo DS bottom-screen `TextComponent` rendering consume cooked `Indexed4` font atlases, repack supported glyphs into BG text tiles, and write the BG map directly without `iprintf` or console semantics.

**Architecture:** Keep the existing hardware-only DS text-background route, but replace the fake ASCII tile assumption with a font-aware glyph cache backed by the cooked atlas referenced by `FontAsset`. Runtime owns only DS tile packing and BG upload; build-time owns atlas color-format conversion, and the DS default font-atlas cook contract stays `Indexed4`.

**Tech Stack:** C++, libnds BG APIs, generated-core `AssetSerializer`, DS source-audit tests, xUnit builder tests, PowerShell build wrapper.

---

## File Map

- `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Source contract for cooked atlas materialization and font-aware DS text tile lookup.
- `src/platform/ds/NintendoDsRuntimeTexture2D.hpp`
  Runtime texture shape already used by DS sprite and texture paths; reused for cooked font atlas payloads.
- `src/platform/ds/NintendoDsRenderManager2D.hpp`
  DS bottom-screen text-background state, cooked-font glyph-cache state, and helper declarations.
- `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Cooked atlas deserialization, glyph repack/upload, and direct BG map text submission.
- `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`
  Builder metadata lock for the DS default `font-atlas-texture` cook contract. This is already green and should stay green through verification.

### Task 1: Lock The Cooked-Font Runtime Contract

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Write the failing source-audit assertions for cooked atlas consumption**

Add assertions like these to `Source_whenRouting2dDrawables_definesHardwareOnlyHelpersAndSilentUnsupportedSkip()`:

```csharp
Assert.Contains("void EnsureBottomScreenFontGlyphTilesReady(FontAsset* font);", headerSource, StringComparison.Ordinal);
Assert.Contains("bool TryResolveBottomScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex);", headerSource, StringComparison.Ordinal);
Assert.Contains("FontAsset* BottomScreenTextGlyphCacheFont;", headerSource, StringComparison.Ordinal);
Assert.Contains("std::array<uint16_t, 95> BottomScreenTextGlyphTileIndices;", headerSource, StringComparison.Ordinal);
Assert.Contains("bool BottomScreenTextGlyphTilesUploaded;", headerSource, StringComparison.Ordinal);

Assert.Contains("LastTextureBuildStage = \"BuildTextureFromCookedOpened\";", sourceCode, StringComparison.Ordinal);
Assert.Contains("asset = ::AssetSerializer::Deserialize(stream);", sourceCode, StringComparison.Ordinal);
Assert.Contains("::TextureAsset* textureAsset = he_cpp_try_cast<TextureAsset>(asset);", sourceCode, StringComparison.Ordinal);
Assert.Contains("runtimeTexture->ColorFormat = textureAsset->ColorFormat;", sourceCode, StringComparison.Ordinal);
Assert.Contains("runtimeTexture->PaletteColors = textureAsset->PaletteColors;", sourceCode, StringComparison.Ordinal);
Assert.Contains("EnsureBottomScreenFontGlyphTilesReady(font);", sourceCode, StringComparison.Ordinal);
Assert.Contains("TryResolveBottomScreenGlyphTileIndex(font, character, tileIndex)", sourceCode, StringComparison.Ordinal);

Assert.DoesNotContain("return static_cast<uint16_t>(character - 32);", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("runtimeTexture->set_Width(0);", sourceCode, StringComparison.Ordinal);
Assert.DoesNotContain("runtimeTexture->set_Height(0);", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the focused audit to verify it fails for the right reason**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: FAIL because `BuildTextureFromCooked(...)` still returns an empty shell and `ResolveBottomScreenGlyphTileIndex(...)` still assumes `character - 32`.

- [ ] **Step 3: Commit the failing audit-only state**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "test: require DS cooked font atlas text path"
```

### Task 2: Materialize Cooked Font Atlas Payloads In `BuildTextureFromCooked`

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Replace the empty cooked-texture stub with the shared asset-deserialization path**

Use the same file-open and typed-asset pattern already used in `NintendoDsRenderManager3D::BuildMaterialFromCooked(...)` and `BuildModelFromCooked(...)`:

```cpp
RuntimeTexture* NintendoDsRenderManager2D::BuildTextureFromCooked(std::string cookedAssetPath) {
    LastTextureBuildStage = "BuildTextureFromCookedBegin";
    LastTextureAssetId = cookedAssetPath;
    if (cookedAssetPath.empty()) {
        throw new ArgumentException("Cooked texture asset path must be provided.", "cookedAssetPath");
    }

    ::FileStream* stream = nullptr;
    ::Asset* asset = nullptr;
    try {
        stream = ::File::OpenRead(cookedAssetPath);
        LastTextureBuildStage = "BuildTextureFromCookedOpened";
        asset = ::AssetSerializer::Deserialize(stream);
        LastTextureBuildStage = "BuildTextureFromCookedDeserialized";
        delete stream;
        stream = nullptr;

        ::TextureAsset* textureAsset = he_cpp_try_cast<TextureAsset>(asset);
        if (textureAsset == nullptr) {
            throw new InvalidOperationException("Nintendo DS cooked texture payloads must deserialize as TextureAsset.");
        }

        NintendoDsRuntimeTexture2D* runtimeTexture = new NintendoDsRuntimeTexture2D();
        runtimeTexture->set_Width(textureAsset->Width);
        runtimeTexture->set_Height(textureAsset->Height);
        runtimeTexture->ColorFormat = textureAsset->ColorFormat;
        runtimeTexture->AlphaPrecision = textureAsset->AlphaPrecision;
        runtimeTexture->Colors = textureAsset->Colors;
        runtimeTexture->PaletteColors = textureAsset->PaletteColors;
        runtimeTexture->HardwareTextureId = -1;
        runtimeTexture->HardwareTextureUploaded = false;

        textureAsset->Colors = Array<uint8_t>::Empty();
        textureAsset->PaletteColors = Array<uint8_t>::Empty();
        delete textureAsset;
        LastTextureBuildStage = "BuildTextureFromCookedComplete";
        return runtimeTexture;
    } catch (...) {
        if (stream != nullptr) {
            delete stream;
        }
        if (asset != nullptr) {
            delete asset;
        }

        throw;
    }
}
```

- [ ] **Step 2: Add the missing includes required by the cooked-texture load path**

At the top of `NintendoDsRenderManager2D.cpp`, add the same serializer and file includes used by the 3D cooked paths:

```cpp
#include "Asset.hpp"
#include "AssetSerializer.hpp"
#include "system/io/file.hpp"
```

- [ ] **Step 3: Run the focused audit**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: still FAIL because the text path still uses the fake ASCII tile mapping and there is no cooked-font glyph cache yet.

- [ ] **Step 4: Commit**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "feat: load cooked DS font atlas textures"
```

### Task 3: Add Font-Aware BG Glyph Cache State

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Add explicit font-glyph cache state to the DS 2D renderer**

Add these fields in `NintendoDsRenderManager2D.hpp` near the other bottom-screen text state:

```cpp
FontAsset* BottomScreenTextGlyphCacheFont;
std::array<uint16_t, 95> BottomScreenTextGlyphTileIndices;
bool BottomScreenTextGlyphTilesUploaded;
```

Initialize them in the constructor:

```cpp
, BottomScreenTextGlyphCacheFont(nullptr)
, BottomScreenTextGlyphTileIndices()
, BottomScreenTextGlyphTilesUploaded(false)
```

And then:

```cpp
BottomScreenTextGlyphTileIndices.fill(static_cast<uint16_t>(0));
```

- [ ] **Step 2: Declare the font-aware glyph helpers**

Add these helper declarations to `NintendoDsRenderManager2D.hpp`:

```cpp
void EnsureBottomScreenFontGlyphTilesReady(FontAsset* font);
bool TryResolveBottomScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex);
```

- [ ] **Step 3: Implement the smallest cache reset behavior**

In `EnsureBottomScreenTextBackgroundReady()` or the constructor path, keep the cache invalid until a real font has been uploaded:

```cpp
BottomScreenTextGlyphCacheFont = nullptr;
BottomScreenTextGlyphTileIndices.fill(static_cast<uint16_t>(0));
BottomScreenTextGlyphTilesUploaded = false;
```

In `EnsureBottomScreenFontGlyphTilesReady(FontAsset* font)`, start with this guard shape:

```cpp
void NintendoDsRenderManager2D::EnsureBottomScreenFontGlyphTilesReady(FontAsset* font) {
    if (font == nullptr) {
        throw new ArgumentNullException("font");
    }
    if (BottomScreenTextGlyphTilesUploaded && BottomScreenTextGlyphCacheFont == font) {
        return;
    }

    BottomScreenTextGlyphTileIndices.fill(static_cast<uint16_t>(0));
    BottomScreenTextGlyphCacheFont = font;
    BottomScreenTextGlyphTilesUploaded = false;
}
```

- [ ] **Step 4: Run the focused audit**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: still FAIL because no glyphs are uploaded yet and the text path still calls the old ASCII resolver.

- [ ] **Step 5: Commit**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "refactor: add DS background text glyph cache state"
```

### Task 4: Repack Cooked Glyphs Into BG Tiles And Use Them For Text

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Implement `TryResolveBottomScreenGlyphTileIndex(...)` as a font-aware lookup**

Replace the old `ResolveBottomScreenGlyphTileIndex(char)` behavior with:

```cpp
bool NintendoDsRenderManager2D::TryResolveBottomScreenGlyphTileIndex(FontAsset* font, char character, uint16_t& tileIndex) {
    tileIndex = 0;
    if (font == nullptr || character < 32 || character > 126) {
        return false;
    }

    EnsureBottomScreenFontGlyphTilesReady(font);
    uint16_t resolvedTileIndex = BottomScreenTextGlyphTileIndices[static_cast<std::size_t>(character - 32)];
    if (resolvedTileIndex == 0) {
        return false;
    }

    tileIndex = resolvedTileIndex;
    return true;
}
```

- [ ] **Step 2: Implement first-pass glyph upload from the cooked atlas**

In `EnsureBottomScreenFontGlyphTilesReady(FontAsset* font)`, add the real repack path:

```cpp
NintendoDsRuntimeTexture2D* runtimeTexture = he_cpp_try_cast<NintendoDsRuntimeTexture2D>(font->Texture);
if (runtimeTexture == nullptr || runtimeTexture->ColorFormat != TextureAssetColorFormat::Indexed4) {
    return;
}
if (runtimeTexture->Colors == nullptr || runtimeTexture->PaletteColors == nullptr) {
    return;
}

BG_PALETTE_SUB[0] = 0;
for (int32_t paletteIndex = 1; paletteIndex < 16; paletteIndex++) {
    int32_t paletteOffset = paletteIndex * 4;
    uint8_t red = runtimeTexture->PaletteColors->Data[paletteOffset];
    uint8_t green = runtimeTexture->PaletteColors->Data[paletteOffset + 1];
    uint8_t blue = runtimeTexture->PaletteColors->Data[paletteOffset + 2];
    BG_PALETTE_SUB[paletteIndex] = RGB15(red >> 3, green >> 3, blue >> 3);
}
```

Then iterate the supported characters from `font->Characters`, repack one `8x8` tile per supported glyph into the BG char base, and record the uploaded tile id in `BottomScreenTextGlyphTileIndices[character - 32]`.

The first-pass upload rules should stay narrow and explicit:

- skip any character outside ASCII `32..126`
- skip glyphs whose source rect cannot fit one `8x8` tile cleanly
- skip glyphs when the atlas payload or palette is missing
- leave `BottomScreenTextGlyphTileIndices[...] == 0` for unsupported glyphs

- [ ] **Step 3: Rewrite `WriteBottomScreenTextLine(...)` to use the font-aware lookup**

Change the tile resolution block to:

```cpp
uint16_t tileIndex = 0;
if (index < static_cast<int32_t>(line.size())) {
    char character = line[static_cast<std::size_t>(index)];
    if (!TryResolveBottomScreenGlyphTileIndex(BottomScreenTextGlyphCacheFont, character, tileIndex)) {
        tileIndex = 0;
    }
}
```

And in `TryDrawHardwareText(...)`, ensure the active font cache is prepared before writing any line:

```cpp
EnsureBottomScreenTextBackgroundReady();
EnsureBottomScreenFontGlyphTilesReady(font);
```

- [ ] **Step 4: Update `TryDrawHardwareText(...)` so unsupported glyphs fail honestly**

Before writing a line, verify each non-space printable character resolves:

```cpp
uint16_t tileIndex = 0;
if (character != ' ' && !TryResolveBottomScreenGlyphTileIndex(font, character, tileIndex)) {
    TraceUnsupportedTextDrawable(text, "glyph");
    return false;
}
```

That keeps the hardware-only policy intact: if the cooked atlas cannot be expressed through the first-pass BG tile rules, the whole text drawable stays unsupported.

- [ ] **Step 5: Run the focused audit**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsRenderManager2DSourceAuditTests"
```

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "feat: render DS background text from cooked indexed4 fonts"
```

### Task 5: Verify Builder Contract And Native DS Build

**Files:**
- Modify: none unless verification reveals a regression
- Test: `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Run the focused builder and source-audit suite**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsPlatformAssetBuilderTests.Descriptor_and_definition_expose_ds_texture_format_capabilities|NintendoDsPlatformAssetBuilderTests.Descriptor_and_definition_expose_ds_metadata|NintendoDsRenderManager2DSourceAuditTests"
```

Expected: PASS

- [ ] **Step 2: Build the DS ROM through the shared wrapper**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\..\helprojs\city\project.heproj `
  -Platform ds `
  -Output ..\..\helprojs\city\ds-build `
  -Configuration Release
```

Expected: PASS with an updated `..\..\helprojs\city\ds-build\helengine_ds.nds`

- [ ] **Step 3: Launch the rebuilt ROM in melonDS**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 `
  -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected runtime check:

- `BACK` is visible on the bottom screen
- bottom-screen text is no longer blank
- bottom-screen text no longer tears because of `iprintf`
- unsupported text still increments `UT` when a glyph cannot be repacked

- [ ] **Step 4: Commit the verification-complete runtime path**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs builder.tests/NintendoDsPlatformAssetBuilderTests.cs
git commit -m "feat: consume cooked indexed4 fonts for DS background text"
```
