# Helengine Nintendo DS Geometry-First Renderer Design

## Goal

Create the first visible Nintendo DS 3D renderer slice for Helengine. The milestone should move the DS backend from a no-op draw path to a visible `cube_test` scene progression:

1. top-screen clear driven by the first extracted camera
2. one visible opaque geometry path
3. visible world-transform rotation from the authored cube scene

The final success condition for this slice is narrow and concrete: `cube_test` boots through the real editor-owned DS build path and shows a rotating cube on the top screen in `melonDS`.

## Scope

This slice covers only the first Nintendo DS 3D geometry path.

Included:
- DS-side runtime model and material carriers for authored cube-scene assets
- `NintendoDsRenderManager3D::Draw()` as a real render entry point
- camera-driven top-screen clear
- render-frame extraction from engine-owned cameras and 3D drawables
- one main-screen 3D viewport
- opaque triangle submission for the cube scene
- entity/world transform application so the cube visibly rotates
- one fixed unlit color path
- visible verification that the scene still loads and now presents geometry

Excluded:
- 2D composition and UI rendering
- lighting and shadow submission
- texture sampling
- transparent materials
- multiple cameras
- render targets beyond existing API compatibility
- performance tuning, batching, and VRAM optimization
- broad material compatibility beyond the rotating cube scene

## Repository Context

`helengine-ds` already proves the packaged runtime contract. The DS builder can stage cooked assets into NitroFS, the generated core can initialize successfully, and the `cube_test` scene can deserialize, materialize, update, and draw without crashing. The real editor-owned DS build path is now wired end to end and produces a `.nds` artifact from the city project.

The current bottleneck is renderer behavior. `NintendoDsRenderManager3D` now preserves DS-owned runtime model and material carriers again, but `Draw()` is still a stub. That means the runtime reaches the first draw without crashing, yet the top screen does not render scene geometry.

The generated core already owns the render graph needed for this slice:

- `Core::Draw()` delegates to `RenderManager3D::Draw()`
- `ObjectManager` stores cameras and `IDrawable3D` instances
- `MeshComponent` exposes runtime model and runtime material data through `IDrawable3D`
- `RenderFrameExtractionService` and related classifiers build drawable submissions per camera

The DS renderer should attach to that existing flow instead of introducing a scene-specific special case.

## Recommended Approach

Implement the first visible DS renderer slice directly in `NintendoDsRenderManager3D`.

This is the recommended direction because it preserves the real engine flow:

`scene load -> runtime asset resolution -> object registration -> frame extraction -> DS submission`

It also supports the milestone ladder already approved:

1. camera-driven clear color
2. visible triangle submission
3. visible cube rotation

Alternative approaches rejected for this slice:

- Scene-specific hardcoded cube rendering: faster to hack in, but it bypasses the engine contracts and creates throwaway code
- Broad renderer support immediately: wider than necessary for the first visible milestone and more likely to stall progress
- Software raster first: does not move the real DS hardware path forward as directly as using the DS 3D hardware

## Architecture

The first renderer slice should keep all DS-specific behavior inside `NintendoDsRenderManager3D`.

### Runtime asset carriers

`BuildModelFromRaw(ModelAsset*)` should preserve the authored data needed for geometry submission in `NintendoDsRuntimeModel`:

- positions
- index buffers
- bounds metadata
- runtime submesh ranges

`BuildMaterialFromRaw(MaterialAsset*, ShaderAsset*)` should remain intentionally narrow. For this slice it only needs to determine:

- whether the material is eligible for the first fixed opaque geometry path
- what flat color should be used for visible output

It should not claim shader, texture, lighting, or transparency support that the DS backend does not yet implement.

### Draw flow

`NintendoDsRenderManager3D::Draw()` should become the DS 3D entry point.

Per frame it should:

1. resolve `Core::Instance`
2. gather engine-owned cameras and drawables through the generated-core extraction path
3. select the first extracted camera only
4. clear the top screen from that camera
5. iterate extracted opaque drawable submissions
6. accept only DS runtime model and material pairs supported by this slice
7. submit triangles for those supported pairs

Unsupported submissions should be handled explicitly in one place. The renderer may skip them for now, but it should not silently pretend to support unimplemented features.

### DS-local helper state

This first implementation should stay small. `NintendoDsRenderManager3D` may own only the minimal helper state required for bring-up:

- one `RenderFrameExtractionService`
- one "3D hardware initialized" guard
- one fallback opaque color

`NintendoDsBootHost` should remain unchanged unless DS video-mode setup proves impossible without host-side changes.

## Runtime Behavior

The visible runtime goal is incremental and honest.

### Checkpoint 1: camera clear

The top screen should stop remaining a static bootstrap color. `Draw()` should clear from the first extracted camera so the DS backend proves it is participating in the real frame.

### Checkpoint 2: visible geometry

After clear works, `Draw()` should submit one supported opaque triangle path so geometry appears on the top screen.

### Checkpoint 3: visible rotation

The same geometry path should apply the drawable world transform so the authored cube in `cube_test` visibly rotates.

For this milestone, flat unlit output is sufficient. If authored material color is directly usable, the DS backend may use it. Otherwise it may use one explicit DS-owned fallback color.

## Data And Control Flow

The intended render flow remains narrow and traceable:

1. `main()` constructs `NintendoDsBootHost`
2. `NintendoDsBootHost::Run()` initializes generated core and loads the startup scene
3. `Core::Update()` advances the cube behavior
4. `Core::Draw()` calls `NintendoDsRenderManager3D::Draw()`
5. `NintendoDsRenderManager3D::Draw()` extracts the first camera frame
6. the DS renderer clears from the selected camera
7. the DS renderer filters to supported opaque mesh submissions
8. the DS renderer resolves DS runtime model and material data
9. the DS renderer emits DS geometry commands
10. the top screen presents a rotating cube

## Error Handling

This slice should preserve strict behavior rather than hiding failures behind best-effort patches.

- null DS runtime model or material data should fail in a clear DS-owned code path
- unsupported materials should be skipped or downgraded through one explicit rule
- cameras beyond the first should be ignored intentionally, not half-rendered
- unsupported renderer features should remain unsupported rather than faked

If the scene loads but no supported geometry submissions are found, the runtime should stay alive and preserve visible diagnostics instead of crashing.

## Verification Plan

Verification should remain staged.

### Builder and editor verification

The DS builder and editor-owned handoff path should remain unchanged and continue producing a valid `.nds` for `cube_test`.

### Native build verification

The DS native package should rebuild successfully through the same editor-owned path already proven for `cube_test`.

### Emulator verification

Manual `melonDS` verification should confirm the milestone ladder in order:

1. ROM boots without immediate shutdown
2. bottom screen still reaches `first draw complete`
3. top screen responds to camera clear
4. top screen shows visible geometry
5. top screen shows visible rotation

## Testing Strategy

Automated tests should stay focused on repository-owned seams that can be verified reliably:

- DS builder tests remain the regression safety net for staging and packaging
- any pure DS-owned data translation helpers added for model or material conversion should receive targeted unit coverage when practical

Manual verification remains necessary for the first visible hardware-facing result:

- rebuild the ROM through the editor-owned path
- launch it in `melonDS`
- confirm the milestone ladder visually

## Constraints

- Keep the engine-owned model and material pipeline intact
- Keep DS-specific logic inside `NintendoDsRenderManager3D`
- Do not widen scope into textures, lighting, transparency, or multi-camera rendering before one rotating cube exists
- Prefer explicit unsupported behavior over fake compatibility

## Recommended Implementation Direction

Implement the renderer slice in milestone order inside `NintendoDsRenderManager3D`:

1. make `Draw()` perform the first extracted-camera clear
2. make `Draw()` submit one visible opaque triangle path from extracted mesh submissions
3. apply drawable transforms so `cube_test` visibly rotates

This keeps the work narrow, visible, and aligned with the real Helengine runtime flow.
