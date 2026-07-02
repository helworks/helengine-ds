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
