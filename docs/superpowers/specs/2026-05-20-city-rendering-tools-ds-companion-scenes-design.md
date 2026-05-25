# City Rendering.Tools DS Companion Scenes Design

## Goal

Teach the `city` project scene generators in `assets/codebase/rendering.tools` to emit Nintendo DS companion scenes for generated scenes, with top-screen 3D content and a bottom-screen overlay that defaults to `DebugComponent` plus a back button.

## Scope

This design is intentionally project-local.

- It does include `city/assets/codebase/rendering.tools`.
- It does include DS companion scene ids and generated scene outputs.
- It does include SceneMap-based remapping from default scene ids to DS companion scene ids.
- It does include a default bottom-screen overlay contract.
- It does allow generators to opt into custom bottom-screen content later.
- It does not turn this into a generic engine/editor feature for all projects.
- It does not change menu providers to point at DS-specific scene ids.

## Current State

The `city` project already has:

- generated scene factories in `assets/codebase/rendering.tools`
- normal generated scene writing services
- a DS boot/menu path that already uses `SceneMapComponent` to redirect default ids to DS ids
- a DS menu scene that is explicitly authored as a dual-screen scene

Non-menu generated scenes do not currently have a consistent DS companion-scene contract.

## Design Decision

Each generated scene in `rendering.tools` will keep its existing default scene output and gain a DS companion scene output.

The default scene id remains the canonical scene id used by providers and menu definitions.

The DS platform will reach the DS companion scene through `SceneMapComponent` remapping rather than by teaching providers to point at DS-specific ids.

## DS Companion Scene Contract

Every generated DS companion scene will have two screens:

- top screen: primary 3D scene presentation
- bottom screen: overlay presentation

### Top screen

The top screen is the main presentation target for the generated scene’s authored 3D content.

Default rule:

- the scene’s existing authored content is placed on the top-screen scene root/camera

### Bottom screen

The bottom screen defaults to a lightweight DS overlay containing:

- `DebugComponent`
- a back button that returns to the menu

This is the default for most generated scenes.

### Custom bottom-screen content

Some generators will be allowed to provide their own bottom-screen content.

That will be an explicit generator-owned opt-in, not a runtime heuristic.

Default rule:

- if a generator does not opt into custom bottom content, the DS companion scene emits the standard debug/back overlay
- if a generator opts in, it supplies the bottom-screen entities instead

## Scene Id and Path Rules

Default scene ids remain unchanged.

Each DS companion scene gets a stable DS-specific id derived from the default scene id, using a deterministic suffix such as `_ds`.

Examples:

- `cube_test` -> `cube_test_ds`
- `axis_test` -> `axis_test_ds`

The same rule applies to generated scene asset paths.

The important constraint is stability: the DS id must be deterministic so SceneMap entries, tests, and return/menu flows stay predictable.

## Loading and Remapping Rules

Menu providers continue to point at default scene ids.

They are not made DS-aware.

The DS generated boot scene’s `SceneMapComponent` will contain remaps from default generated scene ids to DS companion scene ids.

Examples:

- `cube_test -> cube_test_ds`
- `axis_test -> axis_test_ds`
- `spotlight_street_slice -> spotlight_street_slice_ds`

This keeps platform-specific load redirection in one place:

- providers emit default ids
- runtime asks `SceneMapComponent.ResolveSceneId(...)`
- DS resolves default ids to DS ids

`DemoDiscReturnToMenuComponent` continues loading the normal main-menu id and relies on the active scene map to resolve it to the DS menu scene.

## Architecture

### Generator ownership

The `rendering.tools` layer remains responsible for generated-scene authoring.

This design does not push project-specific scene-generation policy into the engine.

### Definition extension

`GeneratedAuthoringSceneDefinition` will be extended so generators can describe DS companion-scene data alongside the existing normal scene data.

The definition needs enough information to express:

- default scene id/path
- DS companion scene id/path
- whether the generator uses the default DS bottom overlay
- optional custom bottom-screen entities when the generator opts out of the default overlay

### Shared DS scaffold helper

The DS scene scaffold should be centralized in one rendering-tools helper/service so every generator does not manually rebuild the same dual-screen camera setup.

That helper is responsible for:

- creating the DS top camera and viewport
- creating the DS bottom camera and viewport
- creating the default bottom overlay when requested
- attaching the back button
- attaching the debug component

### Per-generator responsibility

Individual generated-scene factories remain responsible for their scene content.

They should continue to own:

- scene-specific entities
- scene-specific components
- scene-specific materials/models
- any future custom bottom-screen entities they choose to provide

The shared helper only provides the DS scaffold and default overlay.

## Back Button Contract

The bottom-screen back button is part of the standard DS overlay.

Its role is:

- return from generated scenes to the menu
- use the same normal menu scene id the rest of the project uses
- let SceneMap resolve the DS-specific destination

This preserves the current “default id first, platform remap later” model.

## Debug Overlay Contract

The bottom-screen debug overlay uses `DebugComponent` by default.

It is a scene-authored DS overlay element, not a DS boot-host console.

That means:

- normal scene rendering remains active on both screens
- diagnostics live in-scene
- the overlay is part of the generated DS scene contract

## File Impact

### `city`

Expected touch points are within:

- `assets/codebase/rendering.tools/GeneratedAuthoringSceneDefinition.cs`
- `assets/codebase/rendering.tools/GeneratedAuthoringSceneWriteService.cs`
- `assets/codebase/rendering.tools/RenderingSceneGenerator.cs`
- the individual generated-scene factories that need DS companion output

### Shared engine/editor

Only minimal integration should be touched if required to serialize the already-existing scene/component contracts.

This feature should not become a generic editor-wide DS scene-generation system.

## Testing Strategy

Testing should verify three levels:

### Rendering-tools authoring tests

Verify generated scene definitions and written scene assets contain:

- default scene outputs
- DS companion scene outputs
- stable DS companion ids
- top/bottom camera structure
- default debug/back overlay when no custom bottom content is supplied

### Scene map tests

Verify the DS startup/generated boot scene contains remaps from default generated scene ids to DS companion ids.

### Runtime smoke validation

Verify on DS that:

- selecting a generated scene from the menu loads the DS companion scene through SceneMap
- top screen shows the scene content
- bottom screen shows debug + back button by default
- pressing the back button returns to the DS menu

## Success Criteria

The feature is complete when all of the following are true:

1. `rendering.tools` generators emit DS companion scenes in addition to their normal generated scenes.
2. Menu providers continue using default scene ids.
3. DS boot-time scene mapping redirects default generated scene ids to DS companion ids.
4. Generated DS scenes show scene content on the top screen.
5. Generated DS scenes show default `DebugComponent` + back button on the bottom screen unless the generator explicitly opts into custom bottom content.
6. Returning to the menu from those scenes continues to work through the existing SceneMap/menu-resolution path.

## Risks

### Scope creep into a generic engine feature

The main architectural risk is moving this into shared engine/editor code. That would make a project-local rendering-tools policy harder to reason about and harder to evolve.

### Generator duplication

If each factory manually authors its own DS cameras and bottom overlay, the contract will drift. The shared DS scaffold helper exists to prevent that.

### SceneMap incompleteness

If a generated scene gets a DS companion scene but no SceneMap entry, DS will still load the default scene id. The generation pipeline must treat scene emission and scene-map remap emission as one contract.

### Back-button drift

If the back button starts loading DS scene ids directly instead of using the normal menu id plus SceneMap, scene loading behavior will become inconsistent. The design explicitly avoids that.
