# DS Background Text Without Console Design

## Goal

Keep Nintendo DS text rendering on the hardware text background, but remove the libnds console layer entirely. `TextComponent` should no longer depend on `iprintf`, console row clearing, or console cursor semantics, and DS font atlases must arrive at runtime already cooked as `Indexed4` payloads.

## Problem

The current DS `TextComponent` path uses the bottom-screen text background through libnds console APIs. That is still a console renderer, not a direct background-text renderer.

Current consequences:

- text rendering behaves like console rows instead of authored UI text
- text updates can erase and rewrite rows in ways that cause visible tearing
- authored UI like `BACK` is coupled to the same console machinery as debug overlay text
- "background text" is conceptually muddied by a console abstraction the project does not want

The first attempt at direct map writes also proved another missing piece: writing tile indices into the background map is not enough by itself. `consoleInit` had been hiding font glyph tile and palette upload. Once console ownership was removed, the bottom screen went blank because the renderer had no DS-owned glyph tiles to point at.

## Decision

Replace the DS `iprintf`-based text path with direct hardware text-background submission.

That means:

- keep the DS text background as the hardware target
- stop using `consoleInit`, `iprintf`, and console-row persistence logic for runtime UI text
- keep DS font atlases pre-cooked as `Indexed4` payloads during the build pipeline
- do not convert font atlases between color formats at runtime
- repack supported glyphs from the cooked atlas into DS text-background character tiles at runtime
- write glyph and tile indices directly into the bottom-screen text background map
- keep unsupported text silently skipped while still counting `UT`

## Scope

In scope:

- `NintendoDsRenderManager2D` text rendering path
- DS boot and runtime setup needed to expose a bottom-screen text background without console semantics
- DS default font-atlas cook settings for builder-owned Nintendo DS font atlas artifacts
- DS source audits covering the new contract

Out of scope:

- sprite-backed text
- top-screen text
- non-`1x` text scale
- wrapped text
- rotated text
- tinted or translucent text
- rounded rectangle support

## Target Behavior

Supported first-pass text remains narrow:

- `TextComponent`
- bottom screen only
- `FontScale == 1`
- opaque white
- no wrap
- current alignment support rules stay explicit in code
- printable ASCII plus newline only
- only glyphs that can be repacked into the first-pass DS text-background tile layout are supported

Unsupported text:

- is silently skipped
- still increments unsupported counters
- does not draw markers
- does not write to a console

## Architecture

### 1. DS font atlases are build-time `Indexed4`

Nintendo DS font atlases are builder-owned cooked artifacts and should default to `Indexed4` with binary alpha.

That keeps the asset boundary honest:

- build pipeline owns color-format conversion
- runtime consumes cooked DS atlas bytes only
- runtime does not invent a new generic font-processing path

### 2. Remove console semantics from runtime text

`NintendoDsRenderManager2D::TryDrawHardwareText(...)` stops calling `iprintf`.

Instead it:

- ensures one DS-owned text-background glyph cache exists for the active `FontAsset`
- resolves the target bottom-screen tile cell range
- translates supported characters into DS text-background tile indices through that cache
- writes those indices directly into the active background map
- writes blank tiles into only the covered region when text shrinks or changes

### 3. Repack cooked atlas glyphs into DS text-background tiles

The cooked `Indexed4` atlas remains one standard glyph atlas. It is not already a DS text-background tile set.

So the runtime still owns one DS-specific repack step:

- deserialize the cooked atlas payload through `BuildTextureFromCooked(...)`
- use `FontAsset::Characters` glyph rectangles to locate supported glyphs in that atlas
- read indexed texels and the cooked palette from the DS runtime texture
- repack each supported glyph into DS text-background character tiles
- upload those tiles and one 16-color palette into sub-screen background VRAM
- build one renderer-owned character-to-tile lookup for map writes

This is DS tile packing and upload, not generic color-format conversion.

### 4. Track authored text regions, not console rows

The renderer should stop thinking in "console rows that persist for N frames."

Instead it should track per-draw text regions sufficient to clear stale characters when a supported text drawable changes. The tracked state is there to maintain authored text correctly, not to emulate a terminal.

### 5. Keep metrics honest

The DS overlay counters remain published:

- supported text still contributes to `Tx`
- unsupported text still contributes to `UT`

The bottom-screen debug overlay may disappear temporarily if it still depends on cases outside the supported direct-background text slice. That is acceptable; correctness of the renderer contract is more important than preserving console-style diagnostics.

### 6. Boot and runtime setup

If the current DS boot path initializes the bottom text layer through console helpers, it should be changed so the runtime owns the text background directly instead of through the console abstraction.

The key rule is:

- text background yes
- console runtime no

## Testing

Source audits should enforce:

- no `iprintf` in the DS runtime text path
- no console-row persistence state in the DS renderer
- `TryDrawHardwareText(...)` still exists and remains hardware-only
- `ResolveBottomScreenGlyphTileIndex(...)` no longer assumes `character - 32`
- `BuildTextureFromCooked(...)` must materialize the cooked atlas payload instead of returning an empty texture shell
- unsupported text still increments counters while skipping visibly

Builder verification should enforce:

- the Nintendo DS default `font-atlas-texture` cook contract is `Indexed4`
- the published DS font-atlas default no longer claims `Indexed8`

Runtime verification:

- `BACK` should render without the current scanline-style tearing
- bottom-screen text should no longer disappear because of missing glyph-tile upload
- debug text behavior should be re-evaluated after the direct background path lands

## Risks

- direct tile-map text submission requires explicit glyph and tile ownership that the console layer previously hid
- some glyphs may not fit the first-pass one-cell repack rules and will remain unsupported until later work expands that contract
- some current bottom-screen text may disappear temporarily if it relied on console-specific behavior
- debug overlay text may need follow-up adaptation once the direct path is in place

## Success Criteria

- `TextComponent` on DS no longer uses `iprintf` or console semantics
- DS font atlases default to builder-cooked `Indexed4`
- runtime does not perform generic font color-format conversion
- `BACK` no longer tears because of console row rewrites
- DS text remains hardware-background based, not sprite based
- unsupported text is silently skipped and still counted
