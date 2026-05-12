namespace helengine.ds.builder;

/// <summary>
/// Captures the DS workspace and emits a stub package file for command-line smoke runs.
/// </summary>
internal sealed class NintendoDsSmokeTestNativeBuildExecutor : INintendoDsNativeBuildExecutor {
    /// <summary>
    /// Gets the last workspace passed to the smoke-test executor.
    /// </summary>
    public NintendoDsBuildWorkspace Workspace { get; private set; }

    /// <summary>
    /// Captures the staged workspace and writes one stub package file to the export path.
    /// </summary>
    /// <param name="workspace">Resolved build workspace under smoke-test verification.</param>
    /// <param name="cancellationToken">Cancellation token forwarded by the caller.</param>
    public void Build(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken) {
        Workspace = workspace ?? throw new ArgumentNullException(nameof(workspace));
        string exportPackageDirectoryPath = Path.GetDirectoryName(workspace.ExportPackagePath)
            ?? throw new InvalidOperationException("Unable to resolve the Nintendo DS smoke-test export package directory.");
        Directory.CreateDirectory(exportPackageDirectoryPath);
        File.WriteAllText(workspace.ExportPackagePath, "nds");
    }
}
