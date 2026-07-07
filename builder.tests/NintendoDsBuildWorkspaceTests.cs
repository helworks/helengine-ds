namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the resolved filesystem layout used for one Nintendo DS build.
/// </summary>
public class NintendoDsBuildWorkspaceTests {
    /// <summary>
    /// Verifies the workspace builds the expected NitroFS staging and package-export paths.
    /// </summary>
    [Fact]
    public void Create_builds_expected_staging_and_output_paths() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-work-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");

        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRoot,
            workingRoot,
            outputRoot,
            generatedCoreRoot);

        Assert.Equal(Path.GetFullPath(repositoryRoot), workspace.RepositoryRootPath);
        Assert.Equal(Path.GetFullPath(Path.Combine(workingRoot, "ds", "nitrofs")), workspace.NitroFsRootPath);
        Assert.Equal(Path.GetFullPath(Path.Combine(workingRoot, "ds", "generated-core")), workspace.StagedGeneratedCoreRootPath);
        Assert.Equal(Path.GetFullPath(Path.Combine(workingRoot, "ds", "logs")), workspace.NativeBuildLogsRootPath);
        Assert.Equal("/workspace-staging/ds/nitrofs", workspace.ContainerNitroFsRootPath);
        Assert.Equal("/workspace-staging/ds/generated-core", workspace.ContainerGeneratedCoreRootPath);
        Assert.Equal("/workspace-staging/ds/logs", workspace.ContainerNativeBuildLogsRootPath);
        Assert.Equal(Path.GetFullPath(Path.Combine(repositoryRoot, "build", "helengine_ds.nds")), workspace.RepositoryPackagePath);
        Assert.Equal(Path.GetFullPath(Path.Combine(outputRoot, "helengine_ds.nds")), workspace.ExportPackagePath);
    }

    /// <summary>
    /// Verifies the workspace preserves the raw disabled runtime feature string for downstream native-build consumers.
    /// </summary>
    [Fact]
    public void Create_preserves_disabled_runtime_features() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-work-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");

        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRoot,
            workingRoot,
            outputRoot,
            generatedCoreRoot,
            enableRuntimeDiagnostics: false,
            disabledRuntimeFeatures: "debug_overlay;physics3d.box_box_contact");

        Assert.Equal("debug_overlay;physics3d.box_box_contact", workspace.DisabledRuntimeFeatures);
    }
}
