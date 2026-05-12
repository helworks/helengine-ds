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
        startInfo.ArgumentList.Add("HELENGINE_DS_NITROFS_ROOT=" + workspace.ContainerNitroFsRootPath);
        startInfo.ArgumentList.Add("HELENGINE_CORE_CPP_ROOT=" + workspace.ContainerGeneratedCoreRootPath);

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

        if (!File.Exists(workspace.RepositoryPackagePath)) {
            throw new InvalidOperationException("Nintendo DS package output was not produced.");
        }

        File.Copy(workspace.RepositoryPackagePath, workspace.ExportPackagePath, true);
    }
}
