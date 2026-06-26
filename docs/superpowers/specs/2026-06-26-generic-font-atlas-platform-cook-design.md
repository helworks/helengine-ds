# Generic Font Atlas Platform Cook Design

## Goal

Make font atlas cooking generic across platforms instead of special-casing Nintendo DS behavior.

The editor should expose platform-scoped font-atlas texture settings for any platform that publishes a `font-atlas-texture` cook capability. The build pipeline should then use that capability at cook time to convert generated font atlases into the platform-supported runtime texture format, while keeping the resulting cooked asset as a normal texture payload.

## Problem

The current system has two separate issues:

1. Font assets already expose per-platform font rasterization size, but they do not expose per-platform font-atlas texture cook settings.
2. Packaged font atlas externalization still routes through generic texture assumptions in some paths, which leaves the DS proof font referencing a cooked atlas that remains `Rgba32/A8` and therefore cannot be consumed by the DS BG text upload path.

This is the wrong abstraction boundary. The build knows when it is cooking a font atlas. That distinction should exist in the cook contract and in editor settings, not in file-extension hacks or runtime guesswork.

## Design

### 1. Keep normal texture asset paths

Cooked font atlases remain normal cooked texture assets and keep normal texture file paths such as `.hetex`.

There will be no special font-atlas texture extension. The distinction between font atlases and ordinary textures is expressed by cook capability and work-item kind, not by filename.

### 2. Add generic font-atlas texture platform settings

Font import settings will expose two independent platform-scoped sections:

- `font`
  - owns font rasterization settings such as `PixelSize`
- `font-atlas-texture`
  - owns generated atlas texture cook settings such as color format and alpha precision

The `font-atlas-texture` section should be generic, registry-backed, and available for any platform that publishes a `font-atlas-texture` asset cook capability.

The section payload should reuse the normal texture processor settings model so platforms can constrain options through existing texture capability metadata instead of inventing a new settings shape.

### 3. Editor UI behavior

When a font source asset is selected, the asset settings UI should show:

- the existing platform-scoped font settings block
- a platform-scoped font-atlas texture settings block

The font-atlas texture block should:

- appear only when the active platform publishes a `font-atlas-texture` capability
- use that capability's texture format metadata to constrain allowed options
- behave like other platform-scoped processor settings in the editor

This must not be DS-only. Platforms without a `font-atlas-texture` capability simply do not show the block.

### 4. Packaging behavior

When packaging a font that externalizes its atlas:

- prefer the builder-owned `font-atlas-texture` capability when present
- fall back to generic `texture` capability only for platforms that do not publish `font-atlas-texture`

The packaged `.hefont` should still point to a normal cooked texture path, but that path must represent the output of the chosen font-atlas cook capability.

The shared scene packaging transform must therefore stop assuming that all externalized font atlases are generic textures.

### 5. Builder behavior

When the work-item kind is `font-atlas-texture`, the platform builder must cook the generated font atlas using the settings resolved for that capability.

For DS specifically:

- the DS builder already has a dedicated `font-atlas-texture` path
- that path must emit a DS-supported indexed texture format
- the cooked atlas must deserialize as a normal texture asset at runtime

The DS runtime should not need special path handling or extension checks for font atlases.

### 6. Runtime expectations

The runtime still loads cooked font atlases as ordinary texture assets through the font's cooked atlas texture reference.

No new runtime asset kind is introduced.

The DS text uploader remains free to rely on texture format guarantees established at cook time.

## Implementation Notes

### Editor settings model

- add one registry-backed `font-atlas-texture` settings section definition
- back it with the shared texture processor settings type
- ensure cloning, equality, serialization, and default construction behave like the existing generic section model

### Editor UI

- extend the font asset settings panel with one additional font-atlas texture block
- resolve option visibility and allowed values from the selected platform's `font-atlas-texture` capability metadata
- preserve the existing font pixel-size block

### Packaging transform

- add one helper that resolves the builder-owned font-atlas cook capability, preferring `font-atlas-texture` and falling back to `texture`
- derive cooked font-atlas output paths from the chosen capability's output extension when present, otherwise use the normal texture extension
- route generated font-atlas work items through the dedicated capability when available

### DS builder

- keep consuming `font-atlas-texture` work items through the DS font-atlas cook source processor
- ensure cooked output remains a normal texture asset payload

## Validation

Smallest required validation:

1. Editor tests for font asset settings UI showing and mutating the new font-atlas texture block.
2. Serialization round-trip tests for the new `font-atlas-texture` settings section.
3. Packaging transform tests proving that platforms with `font-atlas-texture` produce font-atlas cook work items and cooked font references through the dedicated capability.
4. DS builder test proving `font-atlas-texture` output is cooked to the DS-supported texture format.
5. One real DS build verifying the top-screen cooked-font proof no longer resolves an `Rgba32/A8` atlas.

## Non-Goals

- introducing any special font-atlas runtime file extension
- adding DS-only editor settings
- teaching the DS runtime to accept unsupported font atlas formats instead of fixing cook-time behavior
- redesigning general texture cooking outside the font-atlas path
