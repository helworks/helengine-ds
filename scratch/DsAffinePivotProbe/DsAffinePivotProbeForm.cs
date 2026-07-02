namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Hosts the disposable Windows probe used to debug DS tiled-logo pivot math.
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
    /// Creates the scratch probe form.
    /// </summary>
    /// <param name="state">Mutable probe state.</param>
    /// <param name="math">Math helper used by the probe.</param>
    public DsAffinePivotProbeForm(DsAffinePivotProbeState state, DsAffinePivotProbeMath math) {
        State = state ?? throw new ArgumentNullException(nameof(state));
        Math = math ?? throw new ArgumentNullException(nameof(math));
        Text = "DS Affine Pivot Probe";
        Width = 860;
        Height = 640;
        StartPosition = FormStartPosition.CenterScreen;
        DoubleBuffered = true;
    }
}
