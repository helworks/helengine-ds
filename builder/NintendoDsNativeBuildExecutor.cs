namespace helengine.ds.builder;

/// <summary>
/// Provides the default native Nintendo DS build executor placeholder for the startup-manifest slice.
/// </summary>
public sealed class NintendoDsNativeBuildExecutor : INintendoDsNativeBuildExecutor {
    /// <summary>
    /// Runs one Docker-backed Nintendo DS packaging build and exports the resulting package.
    /// </summary>
    /// <param name="workspace">Resolved build workspace to package.</param>
    /// <param name="cancellationToken">Cancellation token used to stop the build cooperatively.</param>
    public void Build(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken) {
        if (workspace == null) {
            throw new ArgumentNullException(nameof(workspace));
        }

        Directory.CreateDirectory(workspace.OutputRootPath);
        Directory.CreateDirectory(workspace.NitroFsRootPath);
        DateTime buildStartedUtc = DateTime.UtcNow;
        RunDockerMake(workspace, cancellationToken, "clean");
        RunDockerMake(
            workspace,
            cancellationToken,
            "HELENGINE_DS_NITROFS_ROOT=" + workspace.ContainerNitroFsRootPath,
            "HELENGINE_CORE_CPP_ROOT=" + workspace.ContainerGeneratedCoreRootPath);
        NintendoDsBuildArtifactFreshnessValidator.EnsureFreshArtifactProduced(
            workspace.RepositoryPackagePath,
            buildStartedUtc,
            "Nintendo DS package output");
        File.Copy(workspace.RepositoryPackagePath, workspace.ExportPackagePath, true);
        NintendoDsBuildArtifactFreshnessValidator.EnsureFreshArtifactProduced(
            workspace.ExportPackagePath,
            buildStartedUtc,
            "Nintendo DS exported package");
    }

    /// <summary>
    /// Runs one Docker-backed <c>make</c> command inside the Nintendo DS repository workspace.
    /// </summary>
    /// <param name="workspace">Resolved build workspace used for repository and staging mounts.</param>
    /// <param name="cancellationToken">Cancellation token used to stop the build cooperatively.</param>
    /// <param name="makeArguments">Arguments forwarded to <c>make</c>.</param>
    static void RunDockerMake(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken, params string[] makeArguments) {
        System.Diagnostics.ProcessStartInfo startInfo = new() {
            FileName = "docker",
            WorkingDirectory = workspace.RepositoryRootPath,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };
        startInfo.ArgumentList.Add("run");
        startInfo.ArgumentList.Add("--rm");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(workspace.RepositoryRootPath + ":/workspace");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(workspace.WorkingRootPath + ":/workspace-staging");
        startInfo.ArgumentList.Add("-w");
        startInfo.ArgumentList.Add("/workspace");
        startInfo.ArgumentList.Add("helengine-ds");
        startInfo.ArgumentList.Add("make");
        for (int index = 0; index < makeArguments.Length; index++) {
            if (!string.IsNullOrWhiteSpace(makeArguments[index])) {
                startInfo.ArgumentList.Add(makeArguments[index]);
            }
        }

        using System.Diagnostics.Process process = System.Diagnostics.Process.Start(startInfo)
            ?? throw new InvalidOperationException("Unable to start the Nintendo DS Docker build.");
        Task<string> standardOutputTask = process.StandardOutput.ReadToEndAsync();
        Task<string> standardErrorTask = process.StandardError.ReadToEndAsync();
        while (!process.HasExited) {
            cancellationToken.ThrowIfCancellationRequested();
            process.WaitForExit(100);
        }

        process.WaitForExit();
        Task.WaitAll(standardOutputTask, standardErrorTask);
        string standardOutput = standardOutputTask.Result;
        string standardError = standardErrorTask.Result;
        if (process.ExitCode != 0) {
            throw new InvalidOperationException(
                "Nintendo DS Docker build failed:" + Environment.NewLine + standardOutput + Environment.NewLine + standardError);
        }
    }
}
