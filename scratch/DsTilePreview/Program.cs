namespace helengine.ds.scratch.tilepreview;

/// <summary>
/// Starts the DS tile preview desktop tool so cooked glyph packing can be debugged without the emulator.
/// </summary>
public static class Program {
    /// <summary>
    /// Creates the WinForms preview window and optionally seeds it with one build workspace path.
    /// </summary>
    /// <param name="args">Optional command-line arguments where the first value is one DS build workspace or font path.</param>
    [STAThread]
    public static void Main(string[] args) {
        ApplicationConfiguration.Initialize();
        string initialPath = args.Length > 0 ? args[0] : LatestDsWorkspaceLocator.ResolveInitialPath();
        Application.Run(new DsTilePreviewForm(initialPath));
    }
}
