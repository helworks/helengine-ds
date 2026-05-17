# SceneManager-Only Public Scene Transitions

## Summary

Runtime scene transitions should have exactly one public path: `SceneManager`.

The current architecture still exposes `Core.SceneLoadService` to shared runtime callers, and some code paths still call it directly. That creates inconsistent lifecycle behavior because `SceneLoadService` materializes scenes, while `SceneManager` additionally owns scene identity, transition queuing, trace state, and scene-owned asset tracking. The DS startup menu leak demonstrated the problem directly: the initial menu was loaded through a bypass path, while later menu transitions were loaded through `SceneManager`, so the same logical scene had different ownership behavior.

This slice makes `SceneManager` the only supported public runtime scene-transition API. `RuntimeSceneLoadService` remains in the system, but only as a private collaborator used by `SceneManager`.

## Goals

- Ensure all runtime scene transitions enter through `SceneManager`
- Remove public/shared runtime use of `Core.SceneLoadService`
- Enforce the rule that every runtime scene transition targets a valid scene id
- Preserve the current scene materialization logic by keeping `RuntimeSceneLoadService` as an internal helper
- Eliminate transition-path inconsistencies between startup, menu navigation, return-to-menu, and other runtime scene loads

## Non-Goals

- Fully merge `RuntimeSceneLoadService` into `SceneManager`
- Redesign scene catalog storage or scene manifest formats
- Introduce new public raw-`SceneAsset` runtime load APIs
- Support runtime transitions that do not have a valid scene id

## Problem Statement

The runtime currently has two conceptually different scene-loading paths:

- `SceneManager.LoadScene(...)`
- `Core.SceneLoadService.Load(...)`

Those paths are not equivalent. `SceneManager` owns:

- scene-id-based resolution
- pending-operation sequencing
- unload/reload policy
- loaded-scene record management
- scene-owned asset tracking and release
- scene transition trace state

`RuntimeSceneLoadService` owns:

- scene-asset materialization
- entity tree construction
- component instantiation
- low-level asset reference resolution during scene load
- low-level scene-load diagnostics

When public runtime code calls `SceneLoadService` directly, it bypasses `SceneManager` state and ownership handling. That is exactly the bug class that caused startup menu memory to behave differently from return-to-menu transitions on DS.

## Required Runtime Rule

Every runtime scene transition must target a valid scene id.

If a runtime caller cannot provide a scene id, that is a bug. The runtime should fail clearly instead of silently materializing a scene from a raw `SceneAsset` through a bypass path.

This rule applies to:

- startup scene entry
- menu navigation
- return-to-menu
- gameplay scene switches
- any future runtime-authored scene transitions

## Architecture

### Public Boundary

`SceneManager` becomes the single public scene-transition surface for runtime code.

Public/runtime callers may:

- request a scene transition by scene id
- inspect loaded/pending scene state through `SceneManager`

Public/runtime callers may not:

- call `Core.SceneLoadService` directly
- load scenes from raw `SceneAsset` values
- bypass `SceneManager` ownership and transition tracking

### Internal Boundary

`RuntimeSceneLoadService` remains in place for this slice, but only as a private `SceneManager` collaborator.

Its role becomes explicitly internal:

- given a `SceneAsset`, materialize entities/components/assets
- return tracked load results to `SceneManager`
- emit low-level load diagnostics for failure analysis

It is no longer part of the intended public runtime scene-transition API.

### Ownership Model

The invariant after this refactor is:

- if a scene is active at runtime, it entered through `SceneManager`
- if a scene entered through `SceneManager`, it participates in the same loaded-scene record and owned-asset lifecycle

That invariant eliminates path-dependent release behavior.

## Caller Migration

### Shared Runtime Callers

Shared runtime callers that currently use `Core.SceneLoadService` directly must be migrated to `SceneManager`.

Primary examples include:

- `MenuComponent`
- return-to-menu flows
- any gameplay/runtime helper that currently loads a scene from a `SceneAsset`

These callers must operate on scene ids only.

### Startup Scene

Startup may still begin from a manifest-owned cooked relative path, but it must resolve that path to a scene id before initiating the transition.

That DS startup fix becomes the model for all runtime startup paths:

1. read startup manifest entry
2. resolve cooked scene path to scene id via runtime scene catalog
3. call `SceneManager.LoadScene(sceneId, ...)`

### Generated and Native Callers

Generated/native gameplay code that still emits `SceneLoadService->Load(sceneAsset)` must be migrated so it resolves a scene id and calls `SceneManager`.

The important requirement is not whether the caller is shared C#, generated native code, or handwritten platform code. The requirement is that every runtime transition enters through `SceneManager`.

## Error Handling

The runtime should fail clearly when a scene id cannot be resolved.

Examples:

- startup manifest points at a cooked scene path not present in the runtime scene catalog
- gameplay code requests a transition to a missing scene id
- generated menu/navigation code cannot resolve its target scene id

These are correctness failures and should remain loud. The runtime should not invent fallback raw-scene loads to “keep going.”

## Testing

This slice should verify:

- `Core.SceneLoadService` is no longer used directly by public/shared runtime callers
- startup scene loading routes through `SceneManager`
- menu navigation routes through `SceneManager`
- return-to-menu routes through `SceneManager`
- transitions without scene ids fail clearly
- scene-owned asset tracking remains consistent across startup and subsequent scene returns

DS remains the motivating regression environment, but the rule is shared-engine behavior and should be validated at the shared runtime layer where possible.

## Success Criteria

- No public/shared runtime caller uses `Core.SceneLoadService` directly
- `SceneManager` is the only public runtime scene-transition API
- `RuntimeSceneLoadService` remains only as an internal `SceneManager` helper
- All runtime transitions use valid scene ids
- Startup, menu navigation, and return-to-menu all follow the same tracked lifecycle path

## Follow-Up

If this refactor lands cleanly and no remaining callers need direct access, the next slice can evaluate whether `RuntimeSceneLoadService` still deserves to exist as a separate internal unit or whether its materialization logic should be merged into `SceneManager`.
