# Generated Boot Scene With SceneMapComponent

## Summary

Introduce a generated boot scene for platforms that want startup scene remapping. The boot scene contains a minimal helper entity with `SceneMapComponent`. `SceneMapComponent` remains an optional helper, but it gains an optional `InitialSceneId` so it can both remap later menu scene loads and redirect the initial startup scene through the same mechanism.

This removes the need for platform-specific startup scene path rewrites as the primary routing mechanism. Startup becomes a normal shared scene transition through `SceneManager`.

## Goals

- Generate a minimal boot scene through the editor instead of requiring projects to author it by hand.
- Keep `SceneMapComponent` optional and helper-oriented rather than introducing a new engine service.
- Let `SceneMapComponent` optionally load the first runtime scene through `InitialSceneId`.
- Make startup use the same remap mechanism as later menu scene loads.
- Move DS startup routing away from DS-specific generated-core startup path rewriting.

## Non-Goals

- Making `SceneMapComponent` mandatory for all projects.
- Adding a new `Core` service or `SceneManager`-owned scene-map system.
- Adding DS-only logic inside gameplay components for menu routing.
- Broadly redesigning scene manifests beyond startup scene selection.

## Runtime Design

`SceneMapComponent` becomes a strict singleton helper component with these responsibilities:

- enforce a single active instance
- expose `ResolveSceneId(string sceneId)`
- optionally own `InitialSceneId`
- optionally perform a one-shot startup redirect when `InitialSceneId` is authored

### Singleton Behavior

- On activation/registration, if `Instance` is null, assign `this`.
- If a second active `SceneMapComponent` appears, throw immediately.
- On deactivation/unload/dispose, clear `Instance` if `Instance == this`.

### Scene Resolution

- `ResolveSceneId(string sceneId)` returns the mapped scene id when the singleton exists and a mapping is present.
- If no singleton exists or no mapping exists for the requested id, the original id is returned unchanged.

### Optional Startup Redirect

`SceneMapComponent` gains `InitialSceneId`.

When the component becomes active:

1. register the singleton
2. if `InitialSceneId` is not empty and the startup redirect has not already run:
   - resolve the authored `InitialSceneId` through `ResolveSceneId(...)`
   - call `Core.Instance.SceneManager.LoadScene(resolvedSceneId, SceneLoadMode.Single)`
   - mark the redirect as executed

The startup redirect is opt-in. Projects that use `SceneMapComponent` only for later remapping can leave `InitialSceneId` empty.

## Editor Generation Design

The editor should generate a boot scene for platforms that opt into startup remapping. This scene should be minimal:

- one root entity
- one `SceneMapComponent`
- authored `Mappings`
- optional `InitialSceneId`

The boot scene is generated alongside other generated menu/platform scene assets. It should not require hand-authored project scene files.

### Generated Boot Scene Content

For DS, the generated boot scene should contain:

- mapping: `DemoDiscMainMenu -> DemoDiscMainMenuDs`
- `InitialSceneId = DemoDiscMainMenu`

Startup flow then becomes:

1. platform startup manifest points to generated boot scene
2. boot scene loads
3. `SceneMapComponent` registers its singleton
4. `SceneMapComponent` resolves `DemoDiscMainMenu` to `DemoDiscMainMenuDs`
5. `SceneManager.LoadScene(...)` loads the DS menu scene

For platforms that do not need remapping:

- the boot scene can still be used with an empty or identity mapping
- or the feature can remain disabled entirely until those platforms opt in

The preferred direction is one shared boot-scene pattern for any platform that wants it.

## Menu Integration

All menu-driven scene loads should resolve logical scene ids through `SceneMapComponent.ResolveSceneId(...)` before calling `SceneManager.LoadScene(...)`.

That includes:

- shared `MenuComponent`
- return-to-menu runtime component
- project gameplay return-to-menu component code

This keeps startup and later menu transitions on one consistent routing model.

## Build And Startup Routing

The build pipeline should point startup to the generated boot scene when a platform uses startup remapping.

Expected editor seams:

- generated boot-scene preparation service/factory near the existing generated menu preparation path
- build queue startup-scene override logic updated to select the generated boot scene

Expected DS cleanup:

- DS-specific generated-core startup scene path rewriting should be reduced or removed where the boot-scene manifest entry now provides the correct startup route directly

The desired end state is that DS startup is correct because the cooked manifest names the generated boot scene, not because generated native source is later patched.

## Error Handling

- Duplicate active `SceneMapComponent` instances must throw immediately with a clear error.
- If `InitialSceneId` is empty, startup redirect does nothing.
- If `InitialSceneId` does not have a mapping, the original id is used unchanged.
- If the resolved startup scene id is invalid or missing, the normal `SceneManager` load failure path should surface the problem.

## Testing

### Shared Runtime

- `ResolveSceneId(...)` returns original id when no singleton exists.
- `ResolveSceneId(...)` returns mapped id when mapping exists.
- activating a second `SceneMapComponent` throws.
- unloading or disposing the singleton clears `Instance`.
- `InitialSceneId` performs a one-shot startup redirect.
- menu scene loads route through `ResolveSceneId(...)`.

### Editor

- generated boot-scene preparation writes the expected minimal helper scene
- generated boot scene contains the expected `SceneMapComponent` payload
- build queue startup override selects the generated boot scene for platforms using the feature

### DS Builder

- startup scene in the cooked/build manifest is the generated boot scene
- DS boot-scene mapping resolves `DemoDiscMainMenu -> DemoDiscMainMenuDs`
- DS no longer depends on startup scene native path rewriting as the primary startup routing mechanism

## Implementation Notes

- `SceneMapComponent` remains a helper in shared code, not a `Core` service.
- No new engine interface should be added for scene mapping.
- The feature should be removable for source-code users who do not want it.
- The singleton model is intentionally strict and fail-fast.

## Success Criteria

- A generated minimal boot scene exists for opted-in platforms.
- `SceneMapComponent` can optionally load `InitialSceneId`.
- Startup and menu scene transitions use the same remap helper.
- DS startup reaches `DemoDiscMainMenuDs` through the boot scene.
- DS-specific startup path rewrites are no longer the main mechanism for startup routing.
