# Generic Platform Settings Registry Design

## Goal

Replace the hardcoded per-platform processor-settings container with a generic registry-based system, and use that system to add per-platform font settings that affect every platform-resolved font build path.

The current platform settings model is structurally wrong for this job. `AssetPlatformProcessorSettings` hardcodes `Texture`, `Model`, and `Material` as fields on one central class. That means every new per-platform asset setting requires changing the engine-wide container, serialization, cloning, equality checks, and editor UI by hand. Font size would work if implemented the same way, but it would repeat the same design mistake.

The new design makes per-platform settings section-based instead of field-based. Existing texture, model, and material settings become registered sections in the same mechanism, and font settings become another section rather than a special-case extension.

## Non-Goals

- No compatibility reader for the old hardcoded `.hasset` processor-settings layout.
- No migration or automatic upgrade path for existing `.hasset` files.
- No DS-only or font-only shortcut path.
- No stringly typed ad hoc property bags.
- No fallback behavior that silently ignores unknown or invalid section data.

We are the only users of the engine right now. Breaking and regenerating authored settings is acceptable and preferred.

## Problem Statement

Today the per-platform settings shape is effectively:

```csharp
public class AssetPlatformProcessorSettings {
    public TextureAssetProcessorSettings Texture { get; set; }
    public ModelAssetProcessorSettings Model { get; set; }
    public MaterialAssetProcessorSettings Material { get; set; }
}
```

That has several problems:

- the engine core must know every section type up front
- new platform settings force edits to a central "everything bag"
- editor rendering is coupled to hardcoded fields
- cloning and equality logic are hand-maintained and easy to miss during extension
- font rasterization cannot be configured per platform because no font section exists

The immediate symptom is that `GdiFontImporter` hardcodes `32f` pixels when building a font asset, so per-platform font size has no place to come from. The broader issue is that the platform settings system is not generic even though it presents itself as one.

## Design Principles

- Per-platform settings must be section-based, not field-based.
- New section types must register into the system instead of modifying a central container.
- Sections must remain strongly typed at use sites.
- Serialization, cloning, equality, and editor rendering must all flow through the same registered section metadata.
- Missing, unknown, or mistyped section usage must fail hard.
- Font settings must affect every platform-resolved font build path, including editor preview/cache generation and cooked runtime outputs.

## Proposed Architecture

### 1. Replace the hardcoded platform-settings container with a section registry

`AssetPlatformProcessorSettings` stops owning hardcoded `Texture`, `Model`, and `Material` properties. Instead it owns a dictionary of registered settings sections keyed by section id.

Conceptually:

```csharp
public class AssetPlatformProcessorSettings {
    public Dictionary<string, AssetPlatformSettingsSection> Sections { get; set; }
}
```

`AssetPlatformSettingsSection` stores:

- section id
- typed payload object

This keeps the stored structure generic while still allowing typed retrieval through the registry.

### 2. Add a registry that owns section behavior

Introduce an `AssetPlatformSettingsSectionRegistry` that registers every built-in section. Each registered section owns the behavior that is currently scattered across hardcoded engine code.

Each registered section must define:

- section id
- payload type
- default payload creation
- payload clone behavior
- payload equality behavior
- binary serialize behavior
- binary deserialize behavior
- editor applicability rules
- editor binding/rendering behavior

The registry becomes the single source of truth for how a section exists in memory, on disk, and in the editor.

### 3. Adapt existing built-in settings behind the registry

The current built-in settings become registered sections rather than fields:

- `TextureAssetProcessorSettings` becomes section id `texture`
- `ModelAssetProcessorSettings` becomes section id `model`
- `MaterialAssetProcessorSettings` becomes section id `material`
- new `FontAssetProcessorSettings` becomes section id `font`

This is a full structural refactor, not an adapter layered on top of the old shape.

### 4. Add a real font section

Introduce `FontAssetProcessorSettings` as the first new section that proves the generic system works for a new asset setting type.

Initial payload:

- `PixelSize`

Even though the initial payload is small, it must live in a dedicated `font` section so future font-specific platform settings can be added without touching central infrastructure again.

## Data Flow

### Editor and `.hasset`

Per-platform settings authoring flow becomes:

1. The editor resolves supported platforms.
2. For each platform, `AssetPlatformProcessorSettings` holds a section map.
3. `AssetImportSettingsView` enumerates registered sections that apply to the current asset kind.
4. The matching section editor adapters render and bind their controls dynamically.
5. Saving writes the section-based `.hasset` binary payload.

This removes the hardcoded texture/model/material editor branches as the ownership model for platform settings.

### Asset Import and Platform Builds

Processing flow becomes:

1. `AssetImportManager` loads `.hasset`.
2. It resolves the active platform id.
3. It requests the typed section it needs from the section registry for that platform.
4. If the section is absent, the registry creates the registered default payload for that section.
5. The importer or processor consumes the typed payload.

Examples:

- texture processing requests the `texture` section
- model processing requests the `model` section
- material generation requests the `material` section
- font import requests the `font` section

### Font Build Path

The font path must stop free-running with a hardcoded size.

Required behavior:

1. `AssetImportManager.BuildFontAssetForPlatform(...)` resolves the active platform `font` section.
2. The importer contract receives `FontAssetProcessorSettings`.
3. `GdiFontImporter` creates the `System.Drawing.Font` using `PixelSize` instead of hardcoded `32f`.
4. Editor platform font variant caching continues to key off serialized import settings, so different platform font settings generate distinct cached outputs.

This must apply equally to:

- direct editor font import
- platform-specific editor variant generation
- downstream cooked output generation

## Section API Shape

The exact type names can change if repository naming constraints demand it, but the system needs these responsibilities:

- one runtime section container for a saved platform payload
- one registry for section metadata and typed operations
- one serializer contract per section type
- one editor adapter contract per section type

Typed retrieval must look like engine-owned code, not raw casts scattered everywhere. The important rule is that callers ask for a section id and expected payload type and either get the correct typed payload or fail immediately.

## Serialization

`AssetImportSettingsBinarySerializer` must switch directly to a section-based processor payload format.

Required behavior:

- bump the format version
- write each platform id
- write the number of registered section payloads present for that platform
- for each section, write section id and delegate payload serialization to the matching registered serializer

Deserialization behavior:

- read platform id
- read section count
- for each section id, resolve a registered serializer
- deserialize the typed payload
- fail immediately if the section id is unknown

There is intentionally no reader path for the old layout.

## Editor UI

`AssetImportSettingsView` must stop treating per-platform settings as a hardcoded set of central fields.

Required behavior:

- enumerate applicable registered sections for the current asset kind and current platform
- let each section adapter own its controls and synchronization behavior
- preserve the platform-tab workflow already in place
- keep texture settings available for font assets because font atlases still need texture processing settings
- add a dedicated `font` section for font assets with a `Pixel Size` control

The important ownership change is that the editor becomes a host for registered sections rather than the place where every section is hardcoded by name.

## Validation Rules

- section ids must be unique in the registry
- section ids must be unique within one platform payload
- typed retrieval with the wrong payload type is a hard failure
- missing serializer or editor adapter for a registered section is a hard failure
- saved section ids that are not registered are a hard failure
- `font.pixelSize` must be greater than zero

Defaults:

- when a requested section is absent for a platform, the registry creates that section's default payload
- the default `font.pixelSize` should preserve the current effective behavior, which means `32`

## Failure Behavior

- No silent section drops.
- No best-effort coercion from invalid data.
- No fallback to hardcoded `32f` when font settings are present but invalid.
- No hidden substitution of one section type for another.

Examples of expected failures:

- unknown asset-platform settings section id `font`
- asset-platform settings section `font` requested as `TextureAssetProcessorSettings`
- font pixel size must be greater than zero
- no serializer registered for section `material`

The engine should fail early and explicitly when the registry configuration or saved data is inconsistent.

## Testing

### Serialization

- section-based `.hasset` roundtrip test covering `texture`, `model`, `material`, and `font`
- load failure test for unknown section ids
- load failure test for duplicate or invalid section data where applicable

### Registry

- default payload creation test per built-in section
- typed retrieval test
- wrong-type retrieval failure test
- duplicate registration rejection test
- clone and equality tests across built-in sections

### Editor

- `AssetImportSettingsView` tests proving sections are enumerated dynamically
- regression tests proving font assets show both texture and font sections
- regression tests proving font `Pixel Size` edits are stored under the selected platform only

### Asset Import

- `AssetImportManager` tests proving font builds consume the active platform `font` section
- regression tests proving texture/model/material still resolve correctly through the registry after the refactor
- cache-key regression test proving platform font settings change editor font variant cache identity

### Importer

- `GdiFontImporter` tests proving the requested pixel size is used
- regression test proving the hardcoded `32f` path is gone

## Scope

This is an engine-structure fix, not a DS-only adjustment.

The first visible user-facing outcome is per-platform font pixel size, but the real deliverable is a generic per-platform settings system that stops requiring central-class edits every time a new asset setting type appears.
