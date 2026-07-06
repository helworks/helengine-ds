namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Draws the tiled-checker pivot repro and exposes hotkeys for the current DS math assumptions.
/// </summary>
public sealed class DsAffinePivotProbeForm : Form {
    /// <summary>
    /// Holds the mutable probe state.
    /// </summary>
    readonly DsAffinePivotProbeState State;

    /// <summary>
    /// Holds the math helper used by the probe.
    /// </summary>
    readonly DsAffinePivotProbeMath Math;

    /// <summary>
    /// Drives the continuous rotation preview.
    /// </summary>
    readonly System.Windows.Forms.Timer FrameTimer;

    /// <summary>
    /// Creates the form.
    /// </summary>
    /// <param name="state">Mutable probe state.</param>
    /// <param name="math">Math helper used by the probe.</param>
    public DsAffinePivotProbeForm(DsAffinePivotProbeState state, DsAffinePivotProbeMath math) {
        State = state ?? throw new ArgumentNullException(nameof(state));
        Math = math ?? throw new ArgumentNullException(nameof(math));
        KeyPreview = true;
        Text = "DS Affine Pivot Probe";
        Width = 860;
        Height = 640;
        StartPosition = FormStartPosition.CenterScreen;
        DoubleBuffered = true;

        FrameTimer = new System.Windows.Forms.Timer();
        FrameTimer.Interval = 16;
        FrameTimer.Tick += HandleFrameTimerTick;
        FrameTimer.Start();
    }

    /// <summary>
    /// Advances the rotation and redraws the window.
    /// </summary>
    /// <param name="sender">Timer source.</param>
    /// <param name="e">Unused event data.</param>
    void HandleFrameTimerTick(object sender, EventArgs e) {
        State.RotationRadians += System.Math.PI / 180.0;
        Invalidate();
    }

    /// <summary>
    /// Handles the small set of math toggles used during debugging.
    /// </summary>
    /// <param name="e">Key data for the current key press.</param>
    protected override void OnKeyDown(KeyEventArgs e) {
        base.OnKeyDown(e);

        if (e.KeyCode == Keys.A) {
            State.UseExpandedAnchorOffset = !State.UseExpandedAnchorOffset;
        } else if (e.KeyCode == Keys.S) {
            State.UseNegativeOrbitAngle = !State.UseNegativeOrbitAngle;
        } else if (e.KeyCode == Keys.R) {
            State.RoundParentCenter = !State.RoundParentCenter;
        } else if (e.KeyCode == Keys.G) {
            if (State.GridColumns == 5) {
                State.GridColumns = 2;
                State.GridRows = 2;
            } else if (State.GridColumns == 2) {
                State.GridColumns = 3;
                State.GridRows = 3;
            } else {
                State.GridColumns = 5;
                State.GridRows = 5;
            }
        } else if (e.KeyCode == Keys.Space) {
            FrameTimer.Enabled = !FrameTimer.Enabled;
        }

        Invalidate();
    }

    /// <summary>
    /// Draws the checker tiles, pivot marker, and current mode legend.
    /// </summary>
    /// <param name="e">Paint event data for the form.</param>
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
    /// <param name="graphics">Graphics surface that receives the whole-image bounds.</param>
    void DrawWholeBounds(Graphics graphics) {
        float totalWidth = State.GridColumns * State.TileWidth;
        float totalHeight = State.GridRows * State.TileHeight;
        graphics.DrawRectangle(Pens.DimGray, (float)(State.ParentX - (totalWidth * 0.5)), (float)(State.ParentY - (totalHeight * 0.5)), totalWidth, totalHeight);
    }

    /// <summary>
    /// Draws the parent pivot marker.
    /// </summary>
    /// <param name="graphics">Graphics surface that receives the pivot marker.</param>
    void DrawPivot(Graphics graphics) {
        graphics.FillEllipse(Brushes.HotPink, (float)State.ParentX - 4.0f, (float)State.ParentY - 4.0f, 8.0f, 8.0f);
    }

    /// <summary>
    /// Draws the transformed checker tiles and tile-center markers.
    /// </summary>
    /// <param name="graphics">Graphics surface that receives the tile drawing.</param>
    /// <param name="transforms">Per-tile transforms computed from the probe math.</param>
    void DrawTiles(Graphics graphics, IReadOnlyList<DsAffinePivotProbeTileTransform> transforms) {
        for (int index = 0; index < transforms.Count; index++) {
            DsAffinePivotProbeTileTransform transform = transforms[index];
            Color fillColor = ((transform.TileColumn + transform.TileRow) & 1) == 0 ? Color.CornflowerBlue : Color.Goldenrod;

            using SolidBrush fillBrush = new SolidBrush(fillColor);
            System.Drawing.Drawing2D.GraphicsState previousState = graphics.Save();
            graphics.TranslateTransform(transform.DrawLeft + (State.TileWidth * 0.5f), transform.DrawTop + (State.TileHeight * 0.5f));
            graphics.RotateTransform(transform.RotationDegrees);
            graphics.FillRectangle(fillBrush, -State.TileWidth * 0.5f, -State.TileHeight * 0.5f, State.TileWidth, State.TileHeight);
            graphics.DrawRectangle(Pens.White, -State.TileWidth * 0.5f, -State.TileHeight * 0.5f, State.TileWidth, State.TileHeight);
            graphics.FillEllipse(Brushes.Lime, -3.0f, -3.0f, 6.0f, 6.0f);
            graphics.Restore(previousState);
        }
    }

    /// <summary>
    /// Draws the current toggle state legend.
    /// </summary>
    /// <param name="graphics">Graphics surface that receives the legend text.</param>
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
