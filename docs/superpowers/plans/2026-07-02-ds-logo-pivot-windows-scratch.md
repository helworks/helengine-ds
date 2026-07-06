# DS Logo Pivot Windows Scratch Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a disposable Windows scratch app that reproduces the DS tiled-logo pivot math with a checker image so the remaining bottom-right drift can be debugged quickly off hardware.

**Architecture:** Keep the repro isolated under `scratch` as one `net9.0-windows` WinForms app plus one focused math class that can also be referenced by xUnit. The app should render a tiled checker, a global pivot marker, tile outlines, and a small legend while exposing runtime toggles for the exact assumptions we are still validating.

**Tech Stack:** C#, .NET 9 Windows Forms, System.Drawing, xUnit, `dotnet run`, `dotnet test`

---

## File Map

- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbe.csproj`
  WinForms scratch project definition for the repro app.
- Create: `scratch/DsAffinePivotProbe/Program.cs`
  Application entry point that launches the probe form.
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbeState.cs`
  Holds the active probe settings, animation state, and toggle values.
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbeTileTransform.cs`
  Immutable per-tile draw result used by both tests and the form.
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbeMath.cs`
  Contains the exact tile-placement math that mirrors the current DS runtime assumptions.
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbeForm.cs`
  Draws the checker repro, handles hotkeys, and shows the current mode legend.
- Modify: `builder.tests/helengine.ds.builder.tests.csproj`
  Adds a project reference so xUnit can test the scratch math class directly.
- Create: `builder.tests/DsAffinePivotProbeMathTests.cs`
  Focused regression tests for rigid-body tile placement behavior.

## Task 1: Scaffold the Scratch Windows Probe

**Files:**
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbe.csproj`
- Create: `scratch/DsAffinePivotProbe/Program.cs`
- Test: `scratch/DsAffinePivotProbe/DsAffinePivotProbe.csproj`

- [ ] **Step 1: Create the scratch project file**

Create `scratch/DsAffinePivotProbe/DsAffinePivotProbe.csproj`:

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>WinExe</OutputType>
    <TargetFramework>net9.0-windows</TargetFramework>
    <UseWindowsForms>true</UseWindowsForms>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>disable</Nullable>
  </PropertyGroup>
</Project>
```

- [ ] **Step 2: Create the entry point**

Create `scratch/DsAffinePivotProbe/Program.cs`:

```csharp
namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Starts the disposable Windows probe used to debug DS tiled-logo pivot math.
/// </summary>
internal static class Program {
    /// <summary>
    /// Creates and runs the probe form.
    /// </summary>
    [STAThread]
    static void Main() {
        ApplicationConfiguration.Initialize();
        Application.Run(new DsAffinePivotProbeForm(new DsAffinePivotProbeState(), new DsAffinePivotProbeMath()));
    }
}
```

- [ ] **Step 3: Build the empty shell**

Run:

```powershell
rtk dotnet build .\scratch\DsAffinePivotProbe\DsAffinePivotProbe.csproj
```

Expected:

- `Build succeeded.`

- [ ] **Step 4: Commit the scaffold**

```powershell
git add scratch/DsAffinePivotProbe/DsAffinePivotProbe.csproj scratch/DsAffinePivotProbe/Program.cs
git commit -m "feat: scaffold DS affine pivot probe"
```

## Task 2: Add the Failing Math Tests

**Files:**
- Modify: `builder.tests/helengine.ds.builder.tests.csproj`
- Create: `builder.tests/DsAffinePivotProbeMathTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Add the scratch-project reference to the DS test project**

Update `builder.tests/helengine.ds.builder.tests.csproj` by adding:

```xml
  <ItemGroup>
    <ProjectReference Include="..\scratch\DsAffinePivotProbe\DsAffinePivotProbe.csproj" SkipGetTargetFrameworkProperties="true" />
  </ItemGroup>
```

Place it alongside the existing project references.

- [ ] **Step 2: Write the failing math tests**

Create `builder.tests/DsAffinePivotProbeMathTests.cs`:

```csharp
namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the scratch affine-pivot probe keeps tiled checker placement rigid around one shared parent pivot.
/// </summary>
public sealed class DsAffinePivotProbeMathTests {
    /// <summary>
    /// Verifies zero rotation keeps the tiles in their authored grid positions.
    /// </summary>
    [Fact]
    public void ComputeTileTransforms_whenAngleIsZero_keepsTilesInAuthoredGrid() {
        DsAffinePivotProbeState state = new DsAffinePivotProbeState {
            GridColumns = 2,
            GridRows = 2,
            TileWidth = 32,
            TileHeight = 32,
            ParentX = 100.0,
            ParentY = 100.0,
            RotationRadians = 0.0
        };

        DsAffinePivotProbeMath math = new DsAffinePivotProbeMath();
        IReadOnlyList<DsAffinePivotProbeTileTransform> transforms = math.ComputeTileTransforms(state);

        Assert.Equal(4, transforms.Count);
        Assert.Equal(68.0f, transforms[0].DrawLeft, 3);
        Assert.Equal(68.0f, transforms[0].DrawTop, 3);
        Assert.Equal(100.0f, transforms[3].CenterX, 3);
        Assert.Equal(100.0f, transforms[3].CenterY, 3);
    }

    /// <summary>
    /// Verifies quarter-turn placement preserves each tile center radius from the shared pivot.
    /// </summary>
    [Fact]
    public void ComputeTileTransforms_whenQuarterTurn_preservesTileCenterDistanceFromPivot() {
        DsAffinePivotProbeState state = new DsAffinePivotProbeState {
            GridColumns = 2,
            GridRows = 2,
            TileWidth = 32,
            TileHeight = 32,
            ParentX = 100.0,
            ParentY = 100.0,
            RotationRadians = Math.PI / 2.0,
            UseNegativeOrbitAngle = true
        };

        DsAffinePivotProbeMath math = new DsAffinePivotProbeMath();
        IReadOnlyList<DsAffinePivotProbeTileTransform> transforms = math.ComputeTileTransforms(state);

        for (int index = 0; index < transforms.Count; index++) {
            double deltaX = transforms[index].CenterX - state.ParentX;
            double deltaY = transforms[index].CenterY - state.ParentY;
            double radius = Math.Sqrt((deltaX * deltaX) + (deltaY * deltaY));
            Assert.Equal(22.627416997969522, radius, 6);
        }
    }

    /// <summary>
    /// Verifies changing the anchor rule changes the final top-left placement without changing the logical tile centers.
    /// </summary>
    [Fact]
    public void ComputeTileTransforms_whenAnchorRuleChanges_keepsCenterButMovesTopLeft() {
        DsAffinePivotProbeState state = new DsAffinePivotProbeState {
            GridColumns = 2,
            GridRows = 2,
            TileWidth = 32,
            TileHeight = 32,
            ParentX = 100.0,
            ParentY = 100.0,
            RotationRadians = Math.PI / 6.0
        };

        DsAffinePivotProbeMath math = new DsAffinePivotProbeMath();
        IReadOnlyList<DsAffinePivotProbeTileTransform> centeredAnchor = math.ComputeTileTransforms(state);

        state.UseExpandedAnchorOffset = true;
        IReadOnlyList<DsAffinePivotProbeTileTransform> expandedAnchor = math.ComputeTileTransforms(state);

        Assert.Equal(centeredAnchor[0].CenterX, expandedAnchor[0].CenterX, 6);
        Assert.Equal(centeredAnchor[0].CenterY, expandedAnchor[0].CenterY, 6);
        Assert.NotEqual(centeredAnchor[0].DrawLeft, expandedAnchor[0].DrawLeft);
        Assert.NotEqual(centeredAnchor[0].DrawTop, expandedAnchor[0].DrawTop);
    }
}
```

- [ ] **Step 3: Run the test to verify it fails**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter DsAffinePivotProbeMathTests
```

Expected:

- FAIL
- failure should be from missing scratch probe types and/or missing `ComputeTileTransforms`

- [ ] **Step 4: Commit the red test**

```powershell
git add builder.tests/helengine.ds.builder.tests.csproj builder.tests/DsAffinePivotProbeMathTests.cs
git commit -m "test: add DS affine pivot probe math coverage"
```

## Task 3: Implement the Shared Probe Math Model

**Files:**
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbeState.cs`
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbeTileTransform.cs`
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbeMath.cs`
- Test: `builder.tests/DsAffinePivotProbeMathTests.cs`

- [ ] **Step 1: Create the state model**

Create `scratch/DsAffinePivotProbe/DsAffinePivotProbeState.cs`:

```csharp
namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Holds the active scratch-probe options and animation parameters.
/// </summary>
public sealed class DsAffinePivotProbeState {
    /// <summary>
    /// Number of tile columns in the checker grid.
    /// </summary>
    public int GridColumns { get; set; } = 2;

    /// <summary>
    /// Number of tile rows in the checker grid.
    /// </summary>
    public int GridRows { get; set; } = 2;

    /// <summary>
    /// Width of one logical tile in pixels.
    /// </summary>
    public int TileWidth { get; set; } = 32;

    /// <summary>
    /// Height of one logical tile in pixels.
    /// </summary>
    public int TileHeight { get; set; } = 32;

    /// <summary>
    /// Parent pivot X coordinate in client-space pixels.
    /// </summary>
    public double ParentX { get; set; } = 220.0;

    /// <summary>
    /// Parent pivot Y coordinate in client-space pixels.
    /// </summary>
    public double ParentY { get; set; } = 180.0;

    /// <summary>
    /// Shared rotation angle in radians.
    /// </summary>
    public double RotationRadians { get; set; }

    /// <summary>
    /// Whether the orbit math uses the negative angle convention that currently best matches DS behavior.
    /// </summary>
    public bool UseNegativeOrbitAngle { get; set; } = true;

    /// <summary>
    /// Whether the parent center should be rounded before orbit math is applied.
    /// </summary>
    public bool RoundParentCenter { get; set; }

    /// <summary>
    /// Whether the tile anchor offset should use one expanded full-width/full-height rule.
    /// </summary>
    public bool UseExpandedAnchorOffset { get; set; }
}
```

- [ ] **Step 2: Create the immutable tile-transform result**

Create `scratch/DsAffinePivotProbe/DsAffinePivotProbeTileTransform.cs`:

```csharp
namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Stores the draw placement and visual rotation for one logical checker tile.
/// </summary>
public sealed class DsAffinePivotProbeTileTransform {
    /// <summary>
    /// Creates one tile-transform result.
    /// </summary>
    public DsAffinePivotProbeTileTransform(int tileColumn, int tileRow, float centerX, float centerY, float drawLeft, float drawTop, float rotationDegrees) {
        TileColumn = tileColumn;
        TileRow = tileRow;
        CenterX = centerX;
        CenterY = centerY;
        DrawLeft = drawLeft;
        DrawTop = drawTop;
        RotationDegrees = rotationDegrees;
    }

    /// <summary>
    /// Tile column index inside the logical checker grid.
    /// </summary>
    public int TileColumn { get; }

    /// <summary>
    /// Tile row index inside the logical checker grid.
    /// </summary>
    public int TileRow { get; }

    /// <summary>
    /// Final client-space tile center X coordinate.
    /// </summary>
    public float CenterX { get; }

    /// <summary>
    /// Final client-space tile center Y coordinate.
    /// </summary>
    public float CenterY { get; }

    /// <summary>
    /// Final client-space tile top-left X coordinate.
    /// </summary>
    public float DrawLeft { get; }

    /// <summary>
    /// Final client-space tile top-left Y coordinate.
    /// </summary>
    public float DrawTop { get; }

    /// <summary>
    /// Shared visual rotation in degrees.
    /// </summary>
    public float RotationDegrees { get; }
}
```

- [ ] **Step 3: Implement the tile-placement math**

Create `scratch/DsAffinePivotProbe/DsAffinePivotProbeMath.cs`:

```csharp
namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Reproduces the current DS tiled-sprite orbit and anchor math so it can be debugged quickly on Windows.
/// </summary>
public sealed class DsAffinePivotProbeMath {
    /// <summary>
    /// Computes the final draw transforms for one logical checker grid.
    /// </summary>
    /// <param name="state">Current probe settings.</param>
    /// <returns>Ordered per-tile draw transforms.</returns>
    public IReadOnlyList<DsAffinePivotProbeTileTransform> ComputeTileTransforms(DsAffinePivotProbeState state) {
        if (state == null) {
            throw new ArgumentNullException(nameof(state));
        }

        if (state.GridColumns <= 0 || state.GridRows <= 0 || state.TileWidth <= 0 || state.TileHeight <= 0) {
            throw new InvalidOperationException("Grid and tile sizes must be positive.");
        }

        double parentX = state.RoundParentCenter ? Math.Round(state.ParentX) : state.ParentX;
        double parentY = state.RoundParentCenter ? Math.Round(state.ParentY) : state.ParentY;
        double totalWidth = state.GridColumns * state.TileWidth;
        double totalHeight = state.GridRows * state.TileHeight;
        double orbitAngle = state.UseNegativeOrbitAngle ? -state.RotationRadians : state.RotationRadians;
        double sine = Math.Sin(orbitAngle);
        double cosine = Math.Cos(orbitAngle);
        double rotationDegrees = state.RotationRadians * (180.0 / Math.PI);

        List<DsAffinePivotProbeTileTransform> transforms = new List<DsAffinePivotProbeTileTransform>(state.GridColumns * state.GridRows);
        for (int row = 0; row < state.GridRows; row++) {
            for (int column = 0; column < state.GridColumns; column++) {
                double localCenterX = ((column * state.TileWidth) + (state.TileWidth * 0.5)) - (totalWidth * 0.5);
                double localCenterY = ((row * state.TileHeight) + (state.TileHeight * 0.5)) - (totalHeight * 0.5);
                double rotatedCenterX = (localCenterX * cosine) - (localCenterY * sine);
                double rotatedCenterY = (localCenterX * sine) + (localCenterY * cosine);
                double centerX = parentX + rotatedCenterX;
                double centerY = parentY + rotatedCenterY;
                double anchorOffsetX = state.UseExpandedAnchorOffset ? state.TileWidth : (state.TileWidth * 0.5);
                double anchorOffsetY = state.UseExpandedAnchorOffset ? state.TileHeight : (state.TileHeight * 0.5);
                double drawLeft = Math.Round(centerX - anchorOffsetX);
                double drawTop = Math.Round(centerY - anchorOffsetY);

                transforms.Add(new DsAffinePivotProbeTileTransform(
                    column,
                    row,
                    (float)centerX,
                    (float)centerY,
                    (float)drawLeft,
                    (float)drawTop,
                    (float)rotationDegrees));
            }
        }

        return transforms;
    }
}
```

- [ ] **Step 4: Run the tests to verify they pass**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter DsAffinePivotProbeMathTests
```

Expected:

- PASS

- [ ] **Step 5: Commit the math model**

```powershell
git add scratch/DsAffinePivotProbe/DsAffinePivotProbeState.cs scratch/DsAffinePivotProbe/DsAffinePivotProbeTileTransform.cs scratch/DsAffinePivotProbe/DsAffinePivotProbeMath.cs builder.tests/DsAffinePivotProbeMathTests.cs
git commit -m "feat: add DS affine pivot probe math"
```

## Task 4: Build the Visual Windows Probe

**Files:**
- Create: `scratch/DsAffinePivotProbe/DsAffinePivotProbeForm.cs`
- Modify: `scratch/DsAffinePivotProbe/Program.cs`
- Test: `scratch/DsAffinePivotProbe/DsAffinePivotProbe.csproj`

- [ ] **Step 1: Create the WinForms probe UI**

Create `scratch/DsAffinePivotProbe/DsAffinePivotProbeForm.cs`:

```csharp
namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Draws the tiled-checker pivot repro and exposes hotkeys for the current DS math assumptions.
/// </summary>
public sealed class DsAffinePivotProbeForm : Form {
    /// <summary>
    /// Shared mutable probe state.
    /// </summary>
    readonly DsAffinePivotProbeState State;

    /// <summary>
    /// Shared math helper used by both tests and the visual repro.
    /// </summary>
    readonly DsAffinePivotProbeMath Math;

    /// <summary>
    /// Drives the continuous rotation preview.
    /// </summary>
    readonly Timer FrameTimer;

    /// <summary>
    /// Creates the form.
    /// </summary>
    public DsAffinePivotProbeForm(DsAffinePivotProbeState state, DsAffinePivotProbeMath math) {
        State = state ?? throw new ArgumentNullException(nameof(state));
        Math = math ?? throw new ArgumentNullException(nameof(math));
        DoubleBuffered = true;
        Width = 860;
        Height = 640;
        KeyPreview = true;
        Text = "DS Affine Pivot Probe";

        FrameTimer = new Timer();
        FrameTimer.Interval = 16;
        FrameTimer.Tick += HandleFrameTimerTick;
        FrameTimer.Start();
    }

    /// <summary>
    /// Advances the rotation and redraws the window.
    /// </summary>
    void HandleFrameTimerTick(object sender, EventArgs e) {
        State.RotationRadians += Math.PI / 180.0;
        Invalidate();
    }

    /// <summary>
    /// Handles the small set of math toggles used during debugging.
    /// </summary>
    protected override void OnKeyDown(KeyEventArgs e) {
        base.OnKeyDown(e);

        if (e.KeyCode == Keys.A) {
            State.UseExpandedAnchorOffset = !State.UseExpandedAnchorOffset;
        } else if (e.KeyCode == Keys.S) {
            State.UseNegativeOrbitAngle = !State.UseNegativeOrbitAngle;
        } else if (e.KeyCode == Keys.R) {
            State.RoundParentCenter = !State.RoundParentCenter;
        } else if (e.KeyCode == Keys.G) {
            bool useThreeByThree = State.GridColumns == 2;
            State.GridColumns = useThreeByThree ? 3 : 2;
            State.GridRows = useThreeByThree ? 3 : 2;
        } else if (e.KeyCode == Keys.Space) {
            FrameTimer.Enabled = !FrameTimer.Enabled;
        }

        Invalidate();
    }

    /// <summary>
    /// Draws the checker tiles, pivot marker, and current mode legend.
    /// </summary>
    protected override void OnPaint(PaintEventArgs e) {
        base.OnPaint(e);
        e.Graphics.Clear(Color.FromArgb(20, 24, 30));
        e.Graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

        IReadOnlyList<DsAffinePivotProbeTileTransform> transforms = Math.ComputeTileTransforms(State);
        DrawWholeBounds(e.Graphics);
        DrawPivot(e.Graphics);
        DrawTiles(e.Graphics, transforms);
        DrawLegend(e.Graphics);
    }

    /// <summary>
    /// Draws the whole logical image bounds around the current pivot.
    /// </summary>
    void DrawWholeBounds(Graphics graphics) {
        float totalWidth = State.GridColumns * State.TileWidth;
        float totalHeight = State.GridRows * State.TileHeight;
        graphics.DrawRectangle(Pens.DimGray, (float)(State.ParentX - (totalWidth * 0.5)), (float)(State.ParentY - (totalHeight * 0.5)), totalWidth, totalHeight);
    }

    /// <summary>
    /// Draws the parent pivot marker.
    /// </summary>
    void DrawPivot(Graphics graphics) {
        graphics.FillEllipse(Brushes.HotPink, (float)State.ParentX - 4.0f, (float)State.ParentY - 4.0f, 8.0f, 8.0f);
    }

    /// <summary>
    /// Draws the transformed checker tiles and tile-center markers.
    /// </summary>
    void DrawTiles(Graphics graphics, IReadOnlyList<DsAffinePivotProbeTileTransform> transforms) {
        for (int index = 0; index < transforms.Count; index++) {
            DsAffinePivotProbeTileTransform transform = transforms[index];
            Color fillColor = ((transform.TileColumn + transform.TileRow) & 1) == 0 ? Color.CornflowerBlue : Color.Goldenrod;

            GraphicsState previousState = graphics.Save();
            graphics.TranslateTransform(transform.CenterX, transform.CenterY);
            graphics.RotateTransform(transform.RotationDegrees);
            graphics.FillRectangle(new SolidBrush(fillColor), (-State.TileWidth * 0.5f), (-State.TileHeight * 0.5f), State.TileWidth, State.TileHeight);
            graphics.DrawRectangle(Pens.White, (-State.TileWidth * 0.5f), (-State.TileHeight * 0.5f), State.TileWidth, State.TileHeight);
            graphics.FillEllipse(Brushes.Lime, -3.0f, -3.0f, 6.0f, 6.0f);
            graphics.Restore(previousState);
        }
    }

    /// <summary>
    /// Draws the current toggle state legend.
    /// </summary>
    void DrawLegend(Graphics graphics) {
        string legend = string.Join(
            Environment.NewLine,
            new[] {
                $"A anchor expanded: {State.UseExpandedAnchorOffset}",
                $"S negative orbit sign: {State.UseNegativeOrbitAngle}",
                $"R rounded parent center: {State.RoundParentCenter}",
                $"G grid: {State.GridColumns}x{State.GridRows}",
                $"Space animation running: {FrameTimer.Enabled}"
            });

        graphics.DrawString(legend, Font, Brushes.White, 16.0f, 16.0f);
    }
}
```

- [ ] **Step 2: Build the scratch app**

Run:

```powershell
rtk dotnet build .\scratch\DsAffinePivotProbe\DsAffinePivotProbe.csproj
```

Expected:

- `Build succeeded.`

- [ ] **Step 3: Run the visual probe**

Run:

```powershell
rtk dotnet run --project .\scratch\DsAffinePivotProbe\DsAffinePivotProbe.csproj
```

Expected:

- one window opens titled `DS Affine Pivot Probe`
- the window shows a rotating tiled checker, a hot-pink pivot marker, white tile outlines, green tile centers, and a legend

- [ ] **Step 4: Manually verify the debugging controls**

Verify:

- `A` toggles the anchor rule
- `S` toggles orbit-angle sign
- `R` toggles raw versus rounded parent center
- `G` swaps `2x2` and `3x3`
- `Space` pauses and resumes rotation

- [ ] **Step 5: Commit the visual probe**

```powershell
git add scratch/DsAffinePivotProbe/Program.cs scratch/DsAffinePivotProbe/DsAffinePivotProbeForm.cs
git commit -m "feat: add DS affine pivot probe window"
```

## Task 5: Final Verification and DS Handoff

**Files:**
- Verify: `builder.tests/DsAffinePivotProbeMathTests.cs`
- Verify: `scratch/DsAffinePivotProbe/DsAffinePivotProbeForm.cs`
- Verify output: `scratch/DsAffinePivotProbe/DsAffinePivotProbe.csproj`

- [ ] **Step 1: Re-run the focused math tests**

Run:

```powershell
rtk dotnet test .\builder.tests\helengine.ds.builder.tests.csproj --filter DsAffinePivotProbeMathTests
```

Expected:

- PASS

- [ ] **Step 2: Re-run the scratch app**

Run:

```powershell
rtk dotnet run --project .\scratch\DsAffinePivotProbe\DsAffinePivotProbe.csproj
```

Expected:

- the window opens and redraws continuously

- [ ] **Step 3: Capture the useful debugging conclusion**

Verify manually:

- the baseline mode reproduces the same class of pivot drift seen on DS
- at least one toggle combination reduces or removes the drift
- the winning toggle combination can be translated back into the DS runtime math

- [ ] **Step 4: Commit any final polish if needed**

```powershell
git add builder.tests/DsAffinePivotProbeMathTests.cs scratch/DsAffinePivotProbe/DsAffinePivotProbe.csproj scratch/DsAffinePivotProbe/Program.cs scratch/DsAffinePivotProbe/DsAffinePivotProbeState.cs scratch/DsAffinePivotProbe/DsAffinePivotProbeTileTransform.cs scratch/DsAffinePivotProbe/DsAffinePivotProbeMath.cs scratch/DsAffinePivotProbe/DsAffinePivotProbeForm.cs
git commit -m "test: verify DS affine pivot scratch repro"
```

## Self-Review

- Spec coverage: the scratch app, checker rendering, visual markers, hotkey toggles, and fast Windows-only validation loop are all covered by Tasks 1 through 5.
- Placeholder scan: no `TODO`, `TBD`, or “similar to previous task” shortcuts remain.
- Type consistency: all tasks use the same scratch namespace and the same `DsAffinePivotProbe*` type names across tests, app bootstrap, state, math, and UI.
