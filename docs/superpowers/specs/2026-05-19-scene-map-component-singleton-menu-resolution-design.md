# SceneMapComponent Singleton Menu Resolution Design

## Goal

Remove `SceneMapService` entirely and replace it with a simpler optional runtime pattern:

- `SceneMapComponent` becomes a strict singleton helper component
- menu-driven scene loads resolve logical ids through that singleton when present
- projects that do not use scene remapping continue to behave exactly as they do now

This keeps scene remapping out of `Core` and out of `SceneManager` service ownership while still letting projects opt into runtime scene-id overrides such as:

- `DemoDiscMainMenu -> DemoDiscMainMenuDs`

## Problem

The current runtime scene remap flow is too engine-ingrained for what it actually is:

- `Core` owns `SceneMapService`
- menu code relies on that service
- the service scans loaded scenes for `SceneMapComponent`

That makes scene remapping feel like a required engine subsystem when it is really just an optional helper pattern. The user requirement is to keep scene remapping lightweight and removable in source builds.

There is also a practical DS bug behind this design:

- DS startup uses `DemoDiscMainMenuDs`
- the city return-to-menu component still requests `DemoDiscMainMenu`
- the remap needs to be available to menu code without forcing a `Core` service

## Design Summary

The runtime helper model becomes:

1. Remove `SceneMapService`
2. Remove `Core.SceneMapService`
3. Make `SceneMapComponent` maintain a strict singleton `Instance`
4. Add a static resolve helper on `SceneMapComponent`
5. Route all menu scene loads through that helper before `SceneManager.LoadScene(...)`

This preserves the optional nature of scene remapping:

- if no `SceneMapComponent` exists, the original scene id is used unchanged
- if one exists, mapped ids are used
- if more than one becomes active, runtime fails immediately

## Runtime Contract

`SceneMapComponent` becomes the only runtime entry point for scene-id mapping.

### Singleton behavior

- When one `SceneMapComponent` becomes active, it assigns `SceneMapComponent.Instance`
- If `Instance` is already assigned to a different component, runtime throws immediately
- When the active instance is unloaded or deactivated, it clears `Instance`

### Resolve behavior

`SceneMapComponent` exposes one static helper:

- `ResolveSceneId(string sceneId)`

Behavior:

- throws when `sceneId` is null, empty, or whitespace
- returns the original `sceneId` when no singleton instance exists
- returns the mapped scene id when the singleton contains a matching entry
- returns the original `sceneId` when the singleton exists but has no mapping for that id

The helper encapsulates the singleton and dictionary logic so callers do not read `Mappings` directly.

## Menu Integration

All menu-owned scene loads should resolve through the singleton helper.

### MenuComponent

Any menu item that loads a target scene should first resolve:

- logical target id from the menu item
- mapped target id from `SceneMapComponent.ResolveSceneId(...)`
- then pass the resolved id into `SceneManager.LoadScene(...)`

### DemoDiscReturnToMenuComponent

The return component keeps its logical scene id:

- `DemoDiscMainMenu`

Before loading, it resolves that id through `SceneMapComponent.ResolveSceneId(...)`.

That makes DS work through authored remap data without hardcoding platform ids into menu gameplay code.

## Ownership Boundaries

This design intentionally avoids making scene remapping an engine-owned service.

### What it is

- optional helper component
- optional runtime singleton
- explicit menu-callsite utility

### What it is not

- not a `Core` service
- not a `SceneManager` feature
- not a required engine initialization dependency
- not a mandatory authored-scene pattern for all projects

Projects can:

- use it when they want runtime scene-id remapping
- ignore it completely when they do not

## Failure Behavior

The system should fail loudly on invalid configuration.

### Duplicate scene map instances

If a second active `SceneMapComponent` appears while another singleton instance already exists:

- throw immediately
- error message should clearly explain that only one active `SceneMapComponent` is allowed

### Invalid scene id input

If `ResolveSceneId(...)` receives an invalid scene id:

- throw immediately

This keeps the helper strict and avoids masking broken calling code.

## Compatibility

Projects that do not use scene mapping should see no behavior change:

- no singleton exists
- `ResolveSceneId(...)` returns the original id
- menu scene loads continue to use the same ids they already authored

Projects that already author `SceneMapComponent` data can keep that authored data model. Only the runtime access pattern changes.

## Implementation Areas

Shared engine changes will be in `helengine`:

- `engine/helengine.core/components/SceneMapComponent.cs`
- `engine/helengine.core/scene/runtime/SceneMapService.cs` removed
- `engine/helengine.core/Core.cs`
- `engine/helengine.core/components/2d/menu/MenuComponent.cs`
- `engine/helengine.core/components/2d/menu/DemoDiscReturnToMenuRuntimeComponent.cs`

Project-owned gameplay changes may be needed where menu return components currently bypass the shared runtime helper, for example:

- `C:\dev\helprojs\city\assets\codebase\menu\DemoDiscReturnToMenuComponent.cs`

## Validation

Required tests:

1. `ResolveSceneId(...)` returns original id when no singleton instance exists
2. `ResolveSceneId(...)` returns mapped id when singleton mapping exists
3. activating a second `SceneMapComponent` throws immediately
4. unloading or deactivating the singleton clears `Instance`
5. `MenuComponent` item-triggered scene loads use remapped ids
6. `DemoDiscReturnToMenuComponent` uses the remapped main menu id

Runtime validation target:

- DS return-to-menu from gameplay scenes resolves `DemoDiscMainMenu -> DemoDiscMainMenuDs`

## Non-Goals

This slice does not:

- add new engine interfaces for scene-id remapping
- move scene-id remapping into `SceneManager`
- keep `SceneMapService` as a reduced helper
- redesign the authored `SceneMapComponent` data model
- force all projects to author a persistent scene-map scene
