# DS Dedicated Debug Font Design

## Goal

Stop feeding the Nintendo DS bottom overlay the normal editor default font. Instead, the City project should provide one dedicated DS debug font asset, cooked in the DS font atlas format and used explicitly by the DS scaffold.

## Problem

The current DS bottom overlay is using the normal editor default font. That is the wrong asset boundary for DS.

Current consequences:

- the bottom overlay font is larger than the narrow DS text path expects
- runtime text failures are caused by an editor-oriented asset, not by an intentional DS UI choice
- the DS renderer ends up compensating for the wrong font instead of receiving one authored for DS
- `BACK` and bottom debug text are still coupled to editor font decisions

Even after exposing `Indexed4` in the DS atlas cook path, the runtime is still being asked to render the wrong font for this screen.

## Decision

Introduce one dedicated DS debug font asset and use it for the DS bottom overlay.

That means:

- the City project owns one DS-specific debug font asset
- that asset is intentionally smaller than the editor default font
- the asset publishes explicit Nintendo DS platform atlas settings
- those DS atlas settings use:
  - `ColorFormat = Indexed4`
  - `AlphaPrecision = Binary`
- the DS scaffold uses that font instead of `EditorCore.DefaultFontAssetForEditor`
- runtime does not try to rescue the normal editor font for DS bottom-screen text

## Scope

In scope:

- adding or selecting one dedicated DS debug font asset in the City project
- DS-specific platform atlas settings for that font asset
- DS scaffold changes that stop resolving the editor default font for bottom overlay text
- verification that the DS build packages and uses the dedicated font asset

Out of scope:

- changing the generic editor default font
- inventing a new DS-only font artifact type
- runtime resampling or live conversion of arbitrary editor fonts
- sprite-backed text
- rounded rectangle support

## Target Behavior

The DS bottom overlay:

- uses a dedicated DS debug font asset
- keeps `FontScale == 1`
- uses that font for `BACK`
- uses that font for bottom debug text
- no longer depends on `ResolveRequiredEditorFont()` or `EditorCore.DefaultFontAssetForEditor`

The DS cook/build path:

- cooks that font atlas with explicit DS settings
- preserves the `Indexed4` palette-backed payload for the DS runtime text path

## Architecture

### 1. Dedicated DS debug font asset

The City project adds one dedicated debug font asset for Nintendo DS overlay usage.

This asset is a project-authored UI decision. The renderer should not guess a smaller size at runtime.

Requirements:

- smaller than the editor default debug font
- suitable for the DS bottom overlay
- committed as normal project content

### 2. DS atlas settings live on the font asset

The DS font format must be declared on the asset through platform settings, not inferred at runtime.

For the dedicated DS debug font:

- DS `font-atlas-texture` settings are explicit
- `Indexed4` is used
- binary alpha is used

This keeps the asset pipeline honest and makes the DS font choice visible in authored content.

### 3. DS scaffold uses the DS debug font

`NintendoDsRenderingSceneScaffoldFactory` should stop resolving the editor default font for bottom overlay text.

Instead it should resolve the dedicated DS debug font asset and assign it to:

- `DebugComponent`
- the `BACK` button `TextComponent`
- any other default bottom overlay text emitted by the scaffold

### 4. Runtime stays narrow

The DS runtime text path remains hardware-only and narrow.

This change does not ask the renderer to become more permissive. It asks the project to supply the correct font asset.

If the dedicated DS debug font fits the current DS text path, it renders. If later text still exceeds that path, it stays unsupported until the renderer is extended intentionally in a later change.

## Testing

Project/build verification should enforce:

- the DS scaffold references the dedicated DS debug font instead of the editor default font
- the dedicated DS debug font publishes DS-specific `font-atlas-texture` settings
- those DS settings use `Indexed4`
- the DS build still succeeds and packages the bottom overlay content

Runtime verification:

- the bottom screen is no longer blank because of the oversized editor default font
- `BACK` is visible using the dedicated DS debug font
- DS debug overlay text is visible with the dedicated DS debug font

## Risks

- if the dedicated DS debug font is still authored too large, the same renderer limits will remain visible
- if the scaffold keeps any hidden dependency on the editor default font, the change will be partial
- if the font asset platform settings are not committed explicitly, future asset edits may drift away from the DS format again

## Success Criteria

- the DS bottom overlay no longer uses the editor default font
- the City project owns one dedicated DS debug font asset
- that font uses DS-specific atlas settings with `Indexed4`
- the DS bottom overlay renders with the dedicated font
- runtime does not perform live rescue or conversion of the normal editor font for DS bottom-screen text
