# DS Bottom Console Render Profiler Design

## Summary

The current Nintendo DS main menu is interactive only in theory because the top-screen 2D renderer is running at roughly `0.6` update FPS in `melonDS`. We need a profiling setup that measures the menu workload without using the same engine 2D text path to display the measurements.

This design moves the interactive menu to the top screen and repurposes the bottom screen as a native Nintendo DS text console. The native bottom console will display aggregated 2D renderer timing and primitive-count diagnostics collected from `NintendoDsRenderManager2D`.

## Goals

- Render the interactive DS menu on the top screen.
- Use the bottom screen as a native DS debug console.
- Measure DS 2D renderer cost without relying on the engine 2D renderer for the diagnostic display.
- Report stable, rolling timing and primitive-count data that can guide the next optimization pass.

## Non-Goals

- No renderer optimization is included in this slice.
- No shared cross-platform debug overlay work is included.
- No attempt is made to preserve the current branding-only top-screen layout during profiling.
- No per-draw-call logging or trace capture is included.

## Approach Options

### Option 1: Top-screen engine overlay profiler

Keep the menu where it is and render timing text through `FPSComponent` or another engine 2D overlay.

Pros:

- Smallest scene-layout change.
- Reuses existing text pipeline.

Cons:

- Pollutes the workload being measured.
- Can hide whether text rendering itself is the dominant cost.
- Depends on the same slow path that is currently under suspicion.

### Option 2: Bottom-screen native console profiler

Move the interactive menu to the top screen and show diagnostics on a native libnds console on the bottom screen.

Pros:

- Keeps measurements outside the engine 2D renderer.
- Makes the top screen represent the real workload we care about.
- Fits the DS dual-screen model cleanly.

Cons:

- Requires temporary DS menu-scene remapping.
- Adds a DS-only diagnostic seam in the boot host.

### Option 3: External logging only

Collect the same timing data but emit it only to a terminal, file, or debugger channel.

Pros:

- Lowest on-screen intrusion.
- Easy to expand later.

Cons:

- Not practical in the current `melonDS` workflow because there is no visible terminal for the user.
- Slower feedback loop than an on-device console.

### Recommendation

Use Option 2. It gives a clean measurement boundary and matches the current debugging environment.

## Architecture

### DS Menu Layout During Profiling

The DS-specific generated menu scene should be temporarily remapped:

- Top screen viewport: interactive menu content.
- Bottom screen: native DS console, not engine-rendered UI.

The current top-screen branding-only viewport is removed or disabled for this profiling pass. The goal is not to preserve the branded presentation; the goal is to run the real menu workload on the screen we are profiling.

### Renderer-Owned Profiling Data

`NintendoDsRenderManager2D` should own frame-local profiling data for the DS 2D path. The renderer already defines the draw boundaries we care about, so it is the right place to measure:

- total 2D frame time
- text raster time and text primitive count
- sprite raster time and sprite primitive count
- rounded-rectangle raster time and rounded-rectangle primitive count

The data should be reset at frame start, accumulated during camera traversal, and exposed through a compact read-only snapshot API or equivalent getters that `NintendoDsBootHost` can poll after rendering.

### Native Console Output

`NintendoDsBootHost` should own the bottom-screen debug console lifecycle:

- initialize the native DS bottom-screen console
- clear and redraw the console at a throttled cadence
- read the current 2D profiling snapshot from `NintendoDsRenderManager2D`
- print a compact rolling report

The output should be aggregated per frame, not emitted per primitive or per callsite. That keeps the measurements readable and avoids the console itself becoming a major runtime cost.

## Data Report Format

The bottom-screen native log should show a compact report such as:

- frame number
- update FPS
- total 2D milliseconds
- text milliseconds and count
- sprite milliseconds and count
- rounded-rect milliseconds and count

The exact wording can vary, but the categories must stay stable so repeated builds can be compared.

## Timing Collection Rules

- Profiling must measure the DS renderer hot path only.
- Console output must not contribute to the top-screen render cost being investigated.
- Measurements should be aggregated over a rolling window or refreshed every `15` to `30` frames to reduce noise.
- The data model should make it easy to compare before/after optimizations later.

## Failure Handling

- If the native console cannot initialize, DS startup should fail clearly rather than silently dropping diagnostics.
- If profiling data is unavailable for a frame, the console should print a clear placeholder instead of stale values.
- If the DS menu scene generation fails to remap the viewports, tests should fail before runtime.

## Testing Strategy

Add or update focused tests to cover:

- DS menu generation maps the interactive menu viewport to the top screen for the profiling build shape.
- DS boot host source audits verify native console initialization and diagnostic printing.
- DS renderer source audits verify timing buckets exist for total 2D, text, sprites, and rounded rects.

Verification should also include a real DS export and manual `melonDS` validation:

- top screen shows the interactive menu
- bottom screen shows a native DS log
- the log updates with timing and primitive-count diagnostics

## Success Criteria

- The interactive DS menu renders on the top screen.
- The bottom screen shows a native DS debug console.
- The debug console reports rolling renderer timings and primitive counts.
- The measurements do not rely on the engine 2D text path for their display.
- The resulting data is sufficient to identify which 2D primitive bucket dominates the current `0.6` FPS behavior.
