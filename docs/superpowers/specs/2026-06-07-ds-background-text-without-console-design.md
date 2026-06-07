# DS Background Text Without Console Design

## Goal

Keep Nintendo DS text rendering on the hardware text background, but remove the libnds console layer entirely. `TextComponent` should no longer depend on `iprintf`, console row clearing, or console cursor semantics.

## Problem

The current DS `TextComponent` path uses the bottom-screen text background through libnds console APIs. That is still a console renderer, not a direct background-text renderer.

Current consequences:

- text rendering behaves like console rows instead of authored UI text
- text updates can erase and rewrite rows in ways that cause visible tearing
- authored UI like `BACK` is coupled to the same console machinery as debug overlay text
- “background text” is conceptually muddied by a console abstraction the project does not want

## Decision

Replace the DS `iprintf`-based text path with direct hardware text-background submission.

That means:

- keep the DS text background as the hardware target
- stop using `consoleInit`, `iprintf`, and console-row persistence logic for runtime UI text
- write glyph/tile indices directly into the bottom-screen text background map
- keep unsupported text silently skipped while still counting `UT`

## Scope

In scope:

- `NintendoDsRenderManager2D` text rendering path
- DS boot/runtime setup needed to expose a bottom-screen text background without console semantics
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

Unsupported text:

- is silently skipped
- still increments unsupported counters
- does not draw markers
- does not write to a console

## Architecture

### 1. Remove console semantics from runtime text

`NintendoDsRenderManager2D::TryDrawHardwareText(...)` stops calling `iprintf`.

Instead it:

- resolves the target bottom-screen tile cell range
- translates supported characters into DS text-background tile indices
- writes those indices directly into the active background map
- writes blank tiles into only the covered region when text shrinks or changes

### 2. Track authored text regions, not console rows

The renderer should stop thinking in “console rows that persist for N frames.”

Instead it should track per-draw text regions sufficient to clear stale characters when a supported text drawable changes. The tracked state is there to maintain authored text correctly, not to emulate a terminal.

### 3. Keep metrics honest

The DS overlay counters remain published:

- supported text still contributes to `Tx`
- unsupported text still contributes to `UT`

The bottom-screen debug overlay may disappear temporarily if it still depends on cases outside the supported direct-background text slice. That is acceptable; correctness of the renderer contract is more important than preserving console-style diagnostics.

### 4. Boot/runtime setup

If the current DS boot path initializes the bottom text layer through console helpers, it should be changed so the runtime owns the text background directly instead of through the console abstraction.

The key rule is:

- text background yes
- console runtime no

## Testing

Source audits should enforce:

- no `iprintf` in the DS runtime text path
- no console-row persistence state in the DS renderer
- `TryDrawHardwareText(...)` still exists and remains hardware-only
- unsupported text still increments counters while skipping visibly

Runtime verification:

- `BACK` should render without the current scanline-style tearing
- debug text behavior should be re-evaluated after the direct background path lands

## Risks

- direct tile-map text submission may require explicit glyph/tile ownership that the console layer previously hid
- some current bottom-screen text may disappear temporarily if it relied on console-specific behavior
- debug overlay text may need follow-up adaptation once the direct path is in place

## Success Criteria

- `TextComponent` on DS no longer uses `iprintf` or console semantics
- `BACK` no longer tears because of console row rewrites
- DS text remains hardware-background based, not sprite based
- unsupported text is silently skipped and still counted
