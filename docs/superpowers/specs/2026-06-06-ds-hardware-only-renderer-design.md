# Nintendo DS Hardware-Only Renderer Design

## Summary

The Nintendo DS backend must only render through real Nintendo DS hardware capabilities. The backend must not keep CPU-composited 2D framebuffers, software raster fallback paths, or placeholder capability implementations that claim DS support where none exists.

Unsupported rendering work must be skipped instead of changing execution paths. In debug builds, unsupported drawables must emit a visible hardware-rendered magenta diagnostic marker so developers can immediately see content that does not map to DS hardware. In release builds, unsupported rendering work must be skipped silently.

## Goals

- Remove the CPU 2D compositor from the DS backend entirely.
- Eliminate software fallback behavior from DS rendering.
- Preserve only rendering paths that map to actual DS hardware features.
- Make unsupported rendering obvious in debug builds without inventing fake support.
- Reduce DS runtime RAM usage by removing software framebuffers and software draw caches that exist only to sustain the forbidden path.

## Non-Goals

- Recreate unsupported visuals through approximation or emulation.
- Preserve existing DS 2D behavior when it depends on CPU composition.
- Keep placeholder DS implementations for unsupported APIs such as fake off-screen render targets.

## Policy

### Rendering Policy

The DS backend must satisfy all of the following rules:

1. A drawable may render only when the backend can express it through real DS hardware.
2. The backend must not switch to CPU composition, software sampling, or software rasterization when a hardware path is unavailable.
3. Unsupported drawables must be skipped individually so one unsupported request does not fail the entire frame.
4. Debug builds must surface unsupported drawables through explicit diagnostics and a visible magenta hardware marker.
5. Release builds must skip unsupported drawables without the debug marker.

### Capability Honesty

If the DS backend cannot implement a runtime capability through real hardware, it must not fabricate support. The backend must reject that capability at the platform boundary instead of returning placeholder objects or silently downgrading behavior.

## Current Problem

The existing DS backend still contains a CPU-composited 2D renderer:

- `NintendoDsRenderManager2D` owns CPU-side top and bottom framebuffers.
- `PresentFrame()` copies those CPU buffers into visible DS bitmap VRAM.
- Text rendering still includes cached bitmap and fallback-oriented paths.
- `NintendoDsRenderManager3D::CreateRenderTarget(...)` currently returns a placeholder `RenderTarget` even though the backend does not provide a real DS off-screen render target implementation.

This violates the intended DS platform contract and wastes RAM on paths that should not exist.

## Design

### 1. Replace DS 2D Software Composition With Hardware Routing

`NintendoDsRenderManager2D` will remain the engine-facing 2D render manager for DS, but its role changes completely. It will no longer compose software framebuffers or rasterize pixels on the CPU. Instead, it will:

- inspect drawables,
- determine whether each drawable maps to DS hardware,
- submit supported drawables through hardware-only DS paths,
- skip unsupported drawables,
- emit debug diagnostics for unsupported drawables in debug builds.

The following software-backed state and behavior must be removed:

- CPU top and bottom framebuffers,
- CPU-side frame presentation via DMA copies,
- software clear of CPU 2D backbuffers,
- cached bitmap text surfaces,
- cached rounded-rectangle rasters,
- generic textured-quad fallback for 2D drawing,
- profiling counters that only exist to measure the removed software path.

### 2. Define DS 2D Support Strictly

The DS backend will categorize 2D drawables by hardware support:

Supported categories:

- text that fits the DS hardware text sprite path,
- sprites that fit DS hardware sprite and texture constraints.

Unsupported categories:

- rounded rectangles that depend on CPU rasterization,
- text that requires cached bitmap blits or generic textured glyph fallback,
- sprites that require unsupported texture formats or unsupported CPU-side sampling/composition behavior,
- any 2D draw mode that depends on the removed software framebuffer path.

This support classification must live in the DS renderer rather than being hidden behind fallback helpers.

### 3. Debug-Only Magenta Marker

In debug builds, each unsupported drawable must produce a visible hardware-rendered magenta marker. The marker must not rely on software composition.

Recommended shape:

- a small fixed OBJ sprite or tile-aligned quad,
- colored with a strong magenta DS-compatible packed color,
- positioned at the drawable anchor or top-left bound used for the rejected request.

This does not attempt to recreate the original unsupported primitive. Its only purpose is to make the unsupported request visible on device or emulator while preserving the hardware-only rule.

### 4. Release Behavior

In release builds, unsupported drawables are skipped. No marker is emitted. No alternate rendering path is used.

### 5. Remove Placeholder DS Capability Lies

`NintendoDsRenderManager3D::CreateRenderTarget(int32_t width, int32_t height)` must stop returning a placeholder object. Because the DS backend does not support real off-screen render targets in this renderer design, the method must reject the request explicitly at the DS boundary.

This keeps the API honest and prevents higher layers from assuming a supported DS feature that does not exist.

## Logging And Diagnostics

Debug diagnostics should be explicit and category-based:

- unsupported rounded rectangle,
- unsupported text fallback request,
- unsupported sprite format or mode,
- unsupported render-target creation.

The backend should avoid log spam by using category-aware or signature-aware throttling if repeated unsupported drawables would flood the debug console. The logging policy must not change rendering behavior.

## Memory Impact

Expected RAM savings come from removing:

- top-screen CPU 2D framebuffer,
- bottom-screen CPU 2D framebuffer,
- software text bitmap caches,
- software rounded-rectangle caches,
- associated bookkeeping that only exists to sustain software rendering.

Debug-only unsupported markers should use small fixed hardware assets or transient DS hardware state so they do not reintroduce broad software caches.

## Testing Impact

Existing source-audit tests that assert the CPU 2D compositor must be rewritten. New tests should assert the opposite contract:

- `NintendoDsRenderManager2D` no longer declares CPU framebuffers.
- `NintendoDsRenderManager2D` no longer declares or uses CPU `PresentFrame()` DMA presentation.
- software text bitmap cache and fallback glyph paths are removed.
- unsupported drawables route through skip and debug-marker handling.
- `NintendoDsRenderManager3D::CreateRenderTarget(...)` no longer fabricates placeholder support.

Behavioral validation for this change should stay narrowly scoped to DS renderer source audits and the smallest build or test coverage needed to prove the contract.

## Risks

- Existing DS menu or HUD content may disappear immediately if it depended on the CPU compositor.
- Existing tests assume the current software path and will fail until updated.
- Some engine-facing 2D APIs may currently over-promise what DS can render and will need explicit unsupported handling.

These risks are acceptable because the current path is intentionally being removed as invalid platform behavior.

## Acceptance Criteria

- No DS CPU 2D compositor remains in renderer state or frame flow.
- DS rendering does not switch to software fallback under any circumstance.
- Unsupported drawables are skipped individually.
- Debug builds show a visible magenta hardware marker for unsupported drawables.
- Release builds skip unsupported drawables without marker rendering.
- DS render-target creation no longer returns placeholder objects.
