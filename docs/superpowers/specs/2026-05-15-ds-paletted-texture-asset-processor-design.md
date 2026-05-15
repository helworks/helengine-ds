# DS Paletted Texture Asset Processor Design

## Goal

Add DS-authored per-platform texture processor options for paletted cooking and alpha precision so DS texture storage decisions are made in the editor, not by DS runtime defaults.

## Constraints

- The change is DS-only.
- Texture resolution policy must be authored in the editor per asset.
- The DS runtime should decode cooked texture payloads, not invent platform policy at load time.
- Paletted texture support must be available for all textures, including font atlases.
- Existing non-DS texture behavior must remain unchanged.

## Current Problem

The first DS menu pass exposed a memory problem in the current texture pipeline.

The DS runtime was still consuming raw-style cooked texture payloads that were too large for menu assets such as fonts and sprites. A temporary DS fallback then forced textures down to `128` max resolution, which avoided some `bad_alloc` failures but also made menu text unreadably small because font atlases were being shrunk like regular UI images.

That policy is in the wrong place. The editor should own the DS tradeoff between size, quality, and format on a per-asset basis. The DS runtime should only decode the cooked representation it receives.

## Decisions

- DS per-platform texture settings will expose `MaxResolution`, `ColorFormat`, and `AlphaPrecision`.
- `ColorFormat` options are `Rgba32`, `Rgba4444`, `Indexed4`, and `Indexed8`.
- `AlphaPrecision` options are `Opaque`, `Binary`, `A4`, and `A8`.
- Paletted formats are valid for all textures, including font atlases.
- The temporary blanket DS `MaxResolution = 128` behavior will be removed as the normal policy path.
- The logo asset will use an explicit DS override of `MaxResolution = 128`.
- Menu fonts will use explicit DS-authored settings instead of inheriting generic texture limits.

## Design

### 1. Editor Data Model

The DS per-platform texture settings should become the authoritative authored policy for DS texture cooking. Each DS texture override will carry:

- `MaxResolution`
- `ColorFormat`
- `AlphaPrecision`

Those values must participate in the existing editor-side asset flow:

- typed and generic import-setting serialization
- asset-id hashing so format changes invalidate cached outputs
- editor apply/update behavior
- `AssetImportSettingsView` clone, compare, and persistence behavior

This keeps DS policy explicit per asset. Fonts and regular menu textures can then diverge cleanly without hidden runtime defaults or special-case fallback logic.

### 2. Cooked Texture Payload Contract

Cooked `TextureAsset` payloads need enough structure to represent indexed DS textures instead of assuming a single raw-color byte layout.

The cooked payload should carry:

- texture dimensions
- `ColorFormat`
- `AlphaPrecision`
- main pixel payload
- optional palette payload for indexed formats

Format behavior:

- `Indexed4` supports up to 16 palette entries
- `Indexed8` supports up to 256 palette entries
- `Opaque` forces opaque output
- `Binary` applies thresholded transparency
- `A4` preserves higher alpha fidelity in a compact form
- `A8` preserves full authored alpha fidelity when selected

The editor cook step owns quantization and alpha packing. The DS runtime only decodes the cooked representation and does not reinterpret the texture as a different policy at load time.

### 3. DS Runtime and Asset Policy

The DS runtime should decode the authored cooked payloads for:

- `Rgba32`
- `Rgba4444`
- `Indexed4`
- `Indexed8`

The current temporary DS default `MaxResolution = 128` behavior should be removed from the normal import-policy path. The existing DS scene or asset sanitizer may remain as a safety net for obviously oversized inputs, but it should no longer be the primary way textures get shaped for DS.

Per-asset policy after this change:

- `helengine-logo.png` gets an explicit DS override with `MaxResolution = 128`
- menu fonts get explicit DS overrides with larger atlas resolution and a paletted format
- other textures stay unchanged unless authored otherwise

This keeps responsibilities clean:

- the editor decides quality and storage tradeoffs
- the cook step materializes those decisions into a DS-friendly payload
- the runtime decodes and renders exactly what was authored

### 4. Implementation Shape

The implementation should follow this shape:

- extend the core/editor texture asset schema with `AlphaPrecision` and optional palette storage
- extend import-setting serializers and asset-id hashing
- extend `TextureAssetProcessor` with palette quantization and alpha packing for `Indexed4` and `Indexed8`
- update `AssetImportSettingsView` and related editor flows so DS settings are editable and persist correctly
- extend DS runtime texture decode in `NintendoDsRenderManager2D`
- remove the temporary blanket DS `MaxResolution = 128` fallback
- update project asset sidecars so the logo and fonts carry explicit DS-authored settings

Recommended alpha storage policy for the first pass:

- store alpha with the palette-entry-driven representation first
- ensure that path satisfies `Opaque`, `Binary`, and `A4`
- do not introduce a second separate alpha payload unless implementation proves it necessary

That keeps the first paletted contract smaller and simpler while still supporting the current DS menu goals.

## Verification

Verification should cover:

- serialization round-trips for the new DS texture settings
- cooked texture round-trips for indexed payloads and alpha-precision metadata
- asset-id invalidation when `ColorFormat`, `AlphaPrecision`, or `MaxResolution` changes
- DS runtime source audits for indexed decode branches
- DS export success for the city project
- `melonDS` confirmation that:
  - startup no longer fails with `bad_alloc`
  - the logo still renders
  - menu text is readable again
  - the earlier top-to-bottom refresh artifact does not regress

## Non-Goals

- adding new non-DS texture formats
- inventing more DS texture formats beyond `Rgba4444`, `Indexed4`, and `Indexed8`
- changing non-DS import defaults
- solving general 2D rendering quality beyond the texture-format and resolution contract
