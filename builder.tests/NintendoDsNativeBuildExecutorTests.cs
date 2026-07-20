namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS native build executor constructs Docker commands that stream native build output.
/// </summary>
public sealed class NintendoDsNativeBuildExecutorTests {
    /// <summary>
    /// Ensures Docker commands use a shell wrapper that leaves make output attached to Docker's streams.
    /// </summary>
    [Fact]
    public void BuildDockerArguments_keeps_make_output_on_docker_streams() {
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
            argument => argument.Contains("mkdir -p '/workspace-staging/ds/logs' && make 'HELENGINE_DS_NITROFS_ROOT=/workspace-staging/ds/nitrofs' 'HELENGINE_CORE_CPP_ROOT=/workspace-staging/ds/generated-core'", StringComparison.Ordinal));
        Assert.DoesNotContain(
            arguments,
            argument => argument.Contains("make-build.log' 2>&1", StringComparison.Ordinal));
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

    /// <summary>
    /// Ensures Docker build commands propagate the fatal-error console toggle into the native make invocation.
    /// </summary>
    [Fact]
    public void BuildDockerArguments_includes_fatal_error_console_make_argument() {
        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRootPath: @"C:\repo\helengine-ds",
            workingRootPath: @"C:\temp\workspace",
            outputRootPath: @"C:\temp\out",
            generatedCoreRootPath: @"C:\temp\generated-core",
            enableRuntimeDiagnostics: false,
            disabledRuntimeFeatures: string.Empty,
            enableFatalErrorConsole: false);

        IReadOnlyList<string> arguments = NintendoDsNativeBuildExecutor.BuildDockerArguments(
            workspace,
            "make-build.log",
            [
                "HELENGINE_DS_NITROFS_ROOT=/workspace-staging/ds/nitrofs",
                "HELENGINE_CORE_CPP_ROOT=/workspace-staging/ds/generated-core",
                "HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE=0"
            ]);

        Assert.Contains(
            arguments,
            argument => argument.Contains("HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE=0", StringComparison.Ordinal));
    }
}
