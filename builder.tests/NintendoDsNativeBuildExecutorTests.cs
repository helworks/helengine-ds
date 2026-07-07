namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS native build executor constructs Docker commands that write native build output into mounted workspace log files.
/// </summary>
public sealed class NintendoDsNativeBuildExecutorTests {
    /// <summary>
    /// Ensures Docker commands use a shell wrapper that redirects make output into the mounted workspace log path.
    /// </summary>
    [Fact]
    public void BuildDockerArguments_redirects_make_output_into_workspace_log_file() {
        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRootPath: @"C:\repo\helengine-ds",
            workingRootPath: @"C:\temp\workspace",
            outputRootPath: @"C:\temp\out",
            generatedCoreRootPath: @"C:\temp\generated-core");

        IReadOnlyList<string> arguments = NintendoDsNativeBuildExecutor.BuildDockerArguments(
            workspace,
            "make-build.log",
            [
                "HELENGINE_DS_NITROFS_ROOT=/workspace-staging/ds/nitrofs",
                "HELENGINE_CORE_CPP_ROOT=/workspace-staging/ds/generated-core"
            ]);

        Assert.Equal("run", arguments[0]);
        Assert.Contains("helengine-ds", arguments);
        Assert.Contains("sh", arguments);
        Assert.Contains("-lc", arguments);
        Assert.Contains(
            arguments,
            argument => argument.Contains("make 'HELENGINE_DS_NITROFS_ROOT=/workspace-staging/ds/nitrofs' 'HELENGINE_CORE_CPP_ROOT=/workspace-staging/ds/generated-core' > '/workspace-staging/ds/logs/make-build.log' 2>&1", StringComparison.Ordinal));
    }

    /// <summary>
    /// Ensures Docker build commands propagate the runtime diagnostics toggle into the native make invocation.
    /// </summary>
    [Fact]
    public void BuildDockerArguments_includes_runtime_diagnostics_make_argument() {
        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRootPath: @"C:\repo\helengine-ds",
            workingRootPath: @"C:\temp\workspace",
            outputRootPath: @"C:\temp\out",
            generatedCoreRootPath: @"C:\temp\generated-core",
            enableRuntimeDiagnostics: false);

        IReadOnlyList<string> arguments = NintendoDsNativeBuildExecutor.BuildDockerArguments(
            workspace,
            "make-build.log",
            [
                "HELENGINE_DS_NITROFS_ROOT=/workspace-staging/ds/nitrofs",
                "HELENGINE_CORE_CPP_ROOT=/workspace-staging/ds/generated-core",
                "HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS=0"
            ]);

        Assert.Contains(
            arguments,
            argument => argument.Contains("HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS=0", StringComparison.Ordinal));
    }

    /// <summary>
    /// Ensures Docker build commands propagate the raw disabled runtime feature string into the native make invocation.
    /// </summary>
    [Fact]
    public void BuildDockerArguments_includes_disabled_runtime_features_make_argument() {
        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRootPath: @"C:\repo\helengine-ds",
            workingRootPath: @"C:\temp\workspace",
            outputRootPath: @"C:\temp\out",
            generatedCoreRootPath: @"C:\temp\generated-core",
            enableRuntimeDiagnostics: false,
            disabledRuntimeFeatures: "debug_overlay;physics3d.box_box_contact");

        IReadOnlyList<string> arguments = NintendoDsNativeBuildExecutor.BuildDockerArguments(
            workspace,
            "make-build.log",
            [
                "HELENGINE_DS_NITROFS_ROOT=/workspace-staging/ds/nitrofs",
                "HELENGINE_CORE_CPP_ROOT=/workspace-staging/ds/generated-core",
                "HELENGINE_DS_DISABLED_RUNTIME_FEATURES=debug_overlay;physics3d.box_box_contact"
            ]);

        Assert.Contains(
            arguments,
            argument => argument.Contains("HELENGINE_DS_DISABLED_RUNTIME_FEATURES=debug_overlay;physics3d.box_box_contact", StringComparison.Ordinal));
    }
}
