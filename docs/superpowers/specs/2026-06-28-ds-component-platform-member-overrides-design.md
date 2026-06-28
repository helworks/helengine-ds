# DS Component Platform Member Overrides Design

## Goal

Add a generic platform-specific component member override system that:

- keeps `helengine.core` platform-agnostic
- uses the existing editor entity platform tabs for authoring
- persists platform-specific component member values in detached platform override state
- lets platform builders extend component schemas with extra typed members
- lets generated runtime code include those members only for the target platform

The first exercised case is Nintendo DS `TextComponent.BGLayer`, so DS text can target different background layers without introducing platform-specific properties into the shared core component type.

## Why This Exists

The current DS text path can render on one text background layer, but there is no generic way for a platform to add authorable per-component fields that exist only for that platform. The earlier attribute idea would have leaked platform concerns into `helengine.core`, which violates the architecture constraint that the main engine does not know about platforms.

The editor already has platform tabs and detached platform component override plumbing. The missing piece is member-level overrides plus builder-owned schema extensions that flow from editor authoring through persistence, packaging, codegen, deserialization, and runtime use.

## Requirements

- The base component types in `helengine.core` must remain platform-agnostic.
- Platform-specific component members must be authorable in the existing platform tabs.
- Prefab component defaults and scene component instances must both support the same platform-specific member override model.
- The system must be generic for any component member, not a DS-only text hack.
- Platform-specific members must be defined by platform builder metadata, not by ad-hoc UI code.
- Codegen must remain target-driven and schema-driven, not hardcoded to DS-specific component rules.
- Non-target platforms must not carry runtime fields for platform-specific members they do not use.

## Non-Goals

- Adding platform-specific members directly to shared core component classes.
- Building a free-form string/object property bag for runtime use.
- Implementing arbitrary per-platform behavior in codegen without schema ownership in the builder.
- Solving every DS layer/rendering feature in the first slice beyond the concrete `TextComponent.BGLayer` path.

## Existing Foundation

The current codebase already provides the right architectural starting points:

- editor entity platform tabs already exist in `ComponentPropertiesView`
- detached platform component override state already exists in `ComponentPlatformEditingService`
- scene persistence already saves and loads platform component overrides separately from the base component payload
- automatic component serialization and generated runtime deserialization already have central schema-driven paths

Because of that, the correct solution is to extend the existing override/persistence/codegen stack instead of inventing a second platform-property mechanism.

## Architecture

The system has five layers:

1. Platform component schema extension registration
2. Editor authoring and detached platform component member override persistence
3. Scene packaging and platform payload rewriting
4. Target-specific generated runtime component schema
5. Target runtime deserialization and renderer consumption

The base component remains unchanged in shared engine code. Platform builders register additional typed members for specific component types. The editor shows those extra members only in the active platform tab. Scene save/load persists those platform member values as detached platform override data. During packaging/codegen for a target platform, the platform-specific members become part of the generated runtime component shape for that target only.

## Data Model

### Base Principle

Every component continues to have one common/base definition. Platform-specific member values live only in detached per-platform override state.

### Platform Member Override Record

Each platform component override record must be able to represent:

- platform id
- target component identity within the entity save state
- component type id
- added/removed/replaced platform component state when applicable
- typed member override values for a matching base component instance

The new member override portion must be structured and typed, not a raw dictionary exposed to users. Internally, the persistence layer may serialize values generically, but the editor/runtime contract must still be driven by typed schema definitions.

### Schema Extension Definition

Platform builders register component schema extensions with:

- component type id
- member id / runtime member name
- member data type
- default value
- editor label
- editor ordering
- optional enum/options metadata
- optional validation constraints

For the first DS slice, the builder registers:

- component: `helengine.TextComponent`
- member: `BGLayer`
- type: integer
- default: `0`
- allowed values: `0`, `1`
- editor label: `BG Layer`

## Editor UX

### Placement

The UI must reuse the existing entity platform tabs. No new global platform UI should be invented for this.

When the user selects the common tab:

- only normal shared component members appear

When the user selects `DS`:

- each component that has DS-specific schema extensions may show a `DS Overrides` section
- only DS-specific members for that component appear in that section
- components with no DS-specific members show no DS override section

### Behavior

- The UI should feel like editing a normal typed property, not a custom override bag.
- Editing a DS-specific member creates or updates the detached DS component override state for that component.
- Reverting a property to its platform default should clear the explicit override value instead of persisting redundant state.
- Prefab defaults and scene instances should use the same editing surface and persistence path.

### Why This UX

This matches user expectation:

- common data stays common
- DS-only data appears only in the DS tab
- the user never needs to understand packaging, codegen, or runtime schema generation

## Persistence

### Editor Save

Scene and prefab save paths must persist:

- base component payload as today
- detached platform component override payloads as today
- plus the new typed member override values inside the detached platform override payload

The base component payload must never contain platform-specific members like `BGLayer`.

### Editor Load

When loading scenes and prefabs:

- base components load as today
- detached platform override payloads load as today
- typed platform member override values must be rehydrated into the platform editing state so the DS tab shows the authored values

## Packaging

The platform packager must read detached platform component member overrides for the active target and merge them into the target runtime component payload shape.

For DS packaging:

- if a `TextComponent` has a DS `BGLayer` override, the packaged DS text payload must include it
- if no DS override exists, the packaged DS payload must still receive the builder-defined default value

Non-DS packagers must ignore DS component member extensions entirely.

## Codegen

### Ownership

Codegen does not hardcode DS-specific component behavior. The platform builder owns the schema extension list, and codegen consumes that list for the active target.

### Generated Result

For a DS build, generated runtime `TextComponent` gains:

- a real runtime field/property `BGLayer`

For a non-DS build:

- no `BGLayer` runtime member exists

This preserves platform isolation while still giving DS runtime code a typed field to read.

## Runtime Deserialization

The generated DS runtime deserializer must read `BGLayer` from the packaged DS `TextComponent` payload and populate the generated DS runtime component member.

The shared runtime deserializer path should remain schema-driven. The DS-specific member exists because the generated DS runtime component schema includes it, not because shared runtime code learned about DS.

## DS Runtime Behavior

### First Concrete Use Case

The first runtime consumer is DS text rendering:

- `BGLayer = 0` routes the text to the current DS text background path
- `BGLayer = 1` routes the text to a second DS text background layer intended to appear in front of sprites

### Renderer Changes

The DS renderer must add support for a second text background layer on the relevant screen and respect `TextComponent.BGLayer` when choosing the target background map/glyph cache state.

The first implementation only needs to support the BG layer choices required for DS text overlay ordering. It does not need to solve arbitrary DS layer composition in one pass.

## Validation Rules

- Platform builders must reject duplicate schema extension registrations for the same component member on the same platform.
- Member values must be validated against the registered data type and allowed range/options.
- Editor UI must refuse to edit platform-specific members when no matching platform schema extension exists.
- Packaging must fail loudly if a persisted platform member override exists but the active platform schema no longer defines that member.

## Testing Strategy

### Editor Persistence

- scene save/load round-trips DS component member overrides
- prefab save/load round-trips DS component member overrides
- clearing an override removes redundant persisted state

### Editor UX/State

- DS tab shows `DS Overrides` for `TextComponent` when DS registers `BGLayer`
- common tab does not show `BGLayer`
- components without DS schema extensions show no DS override section

### Builder and Schema

- DS platform registers `TextComponent.BGLayer`
- duplicate registrations fail
- unsupported type/value combinations fail

### Codegen

- DS generated runtime `TextComponent` includes `BGLayer`
- non-DS generated runtime `TextComponent` does not include `BGLayer`

### Packaging and Runtime

- DS packaged `TextComponent` payload includes `BGLayer`
- non-DS packaged payloads do not include DS-only members
- DS runtime deserialization restores `BGLayer`
- DS renderer routes `BGLayer = 1` text to the front background layer

## First Delivery Slice

The first implementation slice should deliver:

- generic platform component member schema extension registration
- generic detached platform component member override persistence
- generic editor rendering for platform-specific component members in the existing platform tabs
- generic packaging/codegen/deserialization flow for platform component members
- one concrete DS extension: `TextComponent.BGLayer`
- DS renderer support for at least two text BG layers so `BGLayer` visibly changes composition

## Risks

- The existing detached platform component override model may currently assume whole-component replacement more strongly than member override merging; that boundary must be cleaned up rather than patched.
- Editor property rendering may need a generic typed-property descriptor path for platform extensions instead of one-off control creation.
- Generated runtime component schema and deserializer generation must stay synchronized; any split ownership there will produce drift.
- DS renderer BG memory/layout limits must be respected when adding the second text background path.

## Recommended Implementation Direction

Build the generic infrastructure first, but keep the exercised vertical slice small:

- one platform: DS
- one component: `TextComponent`
- one field: `BGLayer`

That is enough to prove the stack end-to-end without turning the first pass into a giant speculative framework. The system remains generic because every layer is built in terms of platform-registered component member schema extensions rather than DS-specific hardcoding.
