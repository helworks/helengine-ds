# Helengine Nintendo DS Geometry-First Renderer Design

## Goal

Create the first visible Nintendo DS 3D renderer slice for Helengine. The milestone should render the rotating cube startup scene as one visible rotating cube on the top screen while preserving the current generated-core scene load path and engine-owned material/model contracts.

## Scope

This milestone covers only the first Nintendo DS 3D geometry path.

Included:
- DS-side runtime model and material carriers for authored cube-scene assets
- `NintendoDsRenderManager3D::Draw()` as a real render entry point
- Render-frame extraction from engine-owned cameras and 3D drawables
- One main-screen 3D viewport
- Opaque triangle submission for the rotating cube scene
- Entity/world transform application so the cube visibly rotates
- One fixed unlit color path
- Visible verification that the scene still loads and now presents geometry

Excluded:
- 2D composition and UI rendering
- Lighting and shadow submission
- Texture sampling
- Transparent materials
- Multiple cameras
- Render targets beyond existing API compatibility
- Performance tuning, batching, and aggressive VRAM optimization
- Broad material compatibility beyond the rotating cube scene

## Repository Context

`helengine-ds` already proves the packaged runtime contract. The DS builder can stage cooked assets into NitroFS, the generated core can initialize successfully, and the rotating cube scene can deserialize, materialize, update, and draw without crashing. The current visible runtime checkpoints reach `first draw complete`, but the top screen stays green because the DS render managers still behave as placeholders.

`NintendoDsRenderManager2D` currently discards all 2D draw calls. `NintendoDsRenderManager3D` currently returns placeholder runtime materials, placeholder runtime models, and a placeholder render target, while the inherited `RenderManager3D::Draw()` implementation remains a no-op. That means the current bottleneck is no longer scene load; it is the absence of a DS-side translation from generated-core drawables into DS geometry commands.

The generated core already owns the runtime render graph needed for this slice:

- `ObjectManager` keeps registered cameras and `IDrawable3D` instances
- `MeshComponent` exposes `RuntimeModel` and `RuntimeMaterial` through `IDrawable3D`
- `RenderFrameExtractionService` and `RenderFrameDrawableClassifier` build per-camera drawable submissions
- `Core::Draw()` already delegates to `RenderManager3D::Draw()`

The DS renderer should attach to that existing contract instead of introducing a scene-specific special case.

## Recommended Approach

Use the existing engine render contracts end to end and implement the first visible DS geometry path inside `NintendoDsRenderManager3D`.

This is the recommended direction because it keeps the milestone narrow without creating a dead-end renderer. The DS backend should gather engine-owned cameras and drawables from `ObjectManager`, classify them through the generated-core render-frame path, and translate the supported submissions into DS geometry commands. That preserves the true runtime flow:

`scene load -> runtime asset resolution -> object registration -> frame extraction -> DS geometry submission`

Alternative approaches rejected for this slice:

- Scene-specific cube rendering: faster to hack in, but it would bypass the engine render path and create code that would need to be deleted almost immediately
- Software raster first: accurate in theory, but it delays first visible hardware output and does not move the DS target forward
- Broad material support immediately: wider than needed for the first pixel milestone and likely to slow down visible progress

## Architecture

The first renderer slice should add three focused DS responsibilities.

### Runtime asset carriers

`NintendoDsRenderManager3D::BuildModelFromRaw(ModelAsset*)` should create a DS-owned runtime model that preserves the authored data required for visible geometry:

- positions
- index buffers
- submesh ranges
- bounds metadata

The DS runtime model should remain compatible with the generated-core `RuntimeModel` contract, but it may add DS-specific cached data needed by the submission path.

`NintendoDsRenderManager3D::BuildMaterialFromRaw(MaterialAsset*, ShaderAsset*)` should remain intentionally narrow. For this milestone it only needs to preserve enough information to choose one visible opaque unlit draw path for the rotating cube scene. It should not pretend to support shaders, lights, or textured material features that the DS backend does not yet implement.

### Frame extraction and submission

`NintendoDsRenderManager3D::Draw()` should become the DS render entry point.

Per frame it should:

1. Get `Core::get_Instance()`
2. Read the registered cameras and `IDrawable3D` instances from `ObjectManager`
3. Build a frame extraction result using `RenderFrameExtractionService`
4. Select the first camera
5. Iterate the camera's drawable submissions
6. Accept only supported opaque mesh submissions
7. Translate each accepted submission into DS geometry commands

Unsupported submissions should be handled explicitly in one place. The renderer may skip them or map them to a single fixed fallback path, but it should not silently claim support for transparency, lighting, or other unimplemented features.

### DS geometry backend

The low-level DS geometry path should stay small and deterministic.

It should:

- initialize the DS main-screen 3D hardware state during renderer startup
- clear the top-screen frame each draw
- build projection, view, and model transforms from the engine camera and drawable parent transform
- submit triangles for one submesh at a time
- present one visible cube even if all shading is flat

This slice should not attempt full renderer architecture up front. It only needs enough DS hardware control to prove that the engine-authored rotating cube can reach the screen through the normal material/model pipeline.

## Runtime Behavior

The visible runtime goal is straightforward:

- the bottom-screen diagnostic checkpoints continue to prove startup progress
- the top screen no longer remains a flat bootstrap color
- the rotating cube scene produces one visible rotating cube

The renderer should use one fixed unlit visible color path. If the authored material exposes a directly usable diffuse color, the DS backend may use it. If not, the renderer may use one explicit fallback color owned by the DS backend for this milestone.

The runtime should not fabricate broader support. If the scene requests unsupported material features, those features should be ignored or downgraded through one explicit DS-first rule rather than hidden behind fake compatibility.

## Data And Control Flow

The render flow for this milestone should remain narrow and traceable:

1. `main()` constructs `NintendoDsBootHost`
2. `NintendoDsBootHost::Run()` initializes generated core and loads the startup scene
3. `Core::Update()` advances the rotating cube behavior
4. `Core::Draw()` calls `NintendoDsRenderManager3D::Draw()`
5. `NintendoDsRenderManager3D::Draw()` gathers cameras and drawables from `ObjectManager`
6. `RenderFrameExtractionService` classifies drawables into drawable submissions
7. The DS renderer filters to supported opaque geometry submissions
8. The DS renderer resolves runtime model data and transforms
9. The DS renderer emits DS geometry commands for the cube
10. The top screen presents the rotating cube

This preserves the engine-owned render flow while keeping the DS-specific responsibilities local to the DS renderer.

## Error Handling

This slice should preserve strict, explicit behavior.

Builder-side behavior is already validated by the current generated-core and NitroFS pipeline. Runtime-side rendering should follow the same rule against hidden best-effort patches:

- null runtime model or material data should fail in a clear DS-owned code path
- unsupported material features should be handled through one explicit downgrade or skip rule
- unsupported cameras beyond the first should be ignored intentionally, not half-rendered
- the renderer should not claim shader, lighting, or texture support when those features are still unimplemented

If the scene still loads but no supported geometry submissions are found, the runtime should remain alive and preserve visible diagnostics rather than crashing.

## Verification Plan

The first renderer slice should be verified at three levels.

### Builder verification

The DS builder tests should continue to pass so the scene-load and generated-core staging seams remain stable.

### Native build verification

The native package should rebuild successfully into `build/helengine_ds.nds` using the same NitroFS payload flow that already loads the rotating cube scene.

### Emulator verification

Manual emulator verification in `melonDS` should confirm:

1. the ROM still boots without immediate shutdown
2. the bottom screen still reaches `first draw complete`
3. the top screen shows a rotating cube instead of a flat green fill

One additional visible checkpoint may be added after the first supported geometry submission if it helps isolate render bring-up without weakening the current diagnostics.

## Testing Strategy

Automated tests should remain focused on the parts this repository can verify reliably:

- DS builder tests remain the regression safety net for staging and packaging
- any DS-owned pure data translation helpers introduced for runtime model or material conversion should receive targeted unit coverage when practical

Manual verification remains necessary for the visible hardware result:

- rebuild the ROM
- launch it in `melonDS`
- confirm one rotating cube is visible on the top screen

This keeps the milestone honest. The first visible result is hardware-facing and should be verified as such.

## Constraints

- Keep the engine-owned material/model pipeline intact
- Implement only the geometry needed for the rotating cube scene
- No shader support on Nintendo DS
- No fake feature detection for unsupported rendering features
- Do not widen scope into 2D, lighting, or full material parity before one visible cube exists

## Recommended Implementation Direction

Implement the first DS renderer slice directly in `NintendoDsRenderManager3D` using the generated-core `ObjectManager`, `RenderFrameExtractionService`, and `RuntimeModel`/`RuntimeMaterial` contracts that already exist. Add DS-owned runtime carriers for model and material data, make `Draw()` gather the first camera's opaque submissions, and translate those submissions into one fixed unlit DS geometry path. The success condition is narrow and visible: the rotating cube scene still loads, and one rotating cube appears on the top screen.
