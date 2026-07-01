namespace helengine.ds.builder;

/// <summary>
/// Provides the default native Nintendo DS build executor placeholder for the startup-manifest slice.
/// </summary>
public sealed class NintendoDsNativeBuildExecutor : INintendoDsNativeBuildExecutor {
    /// <summary>
    /// File name used for the Docker-backed clean log.
    /// </summary>
    const string CleanLogFileName = "make-clean.log";

    /// <summary>
    /// File name used for the Docker-backed build log.
    /// </summary>
    const string BuildLogFileName = "make-build.log";

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
        Directory.CreateDirectory(workspace.NativeBuildLogsRootPath);
        DateTime buildStartedUtc = DateTime.UtcNow;
        RunDockerMake(workspace, cancellationToken, CleanLogFileName, "clean");
        RunDockerMake(
            workspace,
            cancellationToken,
            BuildLogFileName,
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
    /// <param name="logFileName">Log file name written beneath the mounted workspace logs root.</param>
    /// <param name="makeArguments">Arguments forwarded to <c>make</c>.</param>
    static void RunDockerMake(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken, string logFileName, params string[] makeArguments) {
        if (workspace == null) {
            throw new ArgumentNullException(nameof(workspace));
        } else if (string.IsNullOrWhiteSpace(logFileName)) {
            throw new ArgumentException("Log file name must be provided.", nameof(logFileName));
        }

        string hostLogPath = Path.Combine(workspace.NativeBuildLogsRootPath, logFileName);
        Directory.CreateDirectory(workspace.NativeBuildLogsRootPath);
        if (File.Exists(hostLogPath)) {
            File.Delete(hostLogPath);
        }

        System.Diagnostics.ProcessStartInfo startInfo = new() {
            FileName = "docker",
            WorkingDirectory = workspace.RepositoryRootPath,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };
        IReadOnlyList<string> dockerArguments = BuildDockerArguments(workspace, logFileName, makeArguments);
        for (int index = 0; index < dockerArguments.Count; index++) {
            startInfo.ArgumentList.Add(dockerArguments[index]);
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
            string nativeBuildLogContents = ReadNativeBuildLogContents(hostLogPath);
            throw new InvalidOperationException(
                "Nintendo DS Docker build failed:" + Environment.NewLine
                + nativeBuildLogContents + Environment.NewLine
                + standardOutput + Environment.NewLine
                + standardError);
        }
    }

    /// <summary>
    /// Builds the Docker command-line arguments used to execute one Nintendo DS native build and redirect the native toolchain output into a mounted log file.
    /// </summary>
    /// <param name="workspace">Resolved build workspace used for repository and staging mounts.</param>
    /// <param name="logFileName">Log file name written beneath the mounted workspace logs root.</param>
    /// <param name="makeArguments">Arguments forwarded to <c>make</c>.</param>
    /// <returns>Docker command-line arguments.</returns>
    internal static IReadOnlyList<string> BuildDockerArguments(
        NintendoDsBuildWorkspace workspace,
        string logFileName,
        IReadOnlyList<string> makeArguments) {
        if (workspace == null) {
            throw new ArgumentNullException(nameof(workspace));
        } else if (string.IsNullOrWhiteSpace(logFileName)) {
            throw new ArgumentException("Log file name must be provided.", nameof(logFileName));
        }

        List<string> arguments = [
            "run",
            "--rm",
            "-v",
            workspace.RepositoryRootPath + ":/workspace",
            "-v",
            workspace.WorkingRootPath + ":/workspace-staging",
            "-w",
            "/workspace",
            "helengine-ds",
            "sh",
            "-lc",
            BuildDockerMakeShellCommand(workspace, logFileName, makeArguments)
        ];
        return arguments;
    }

    /// <summary>
    /// Builds the shell command that runs <c>make</c> and redirects the full native toolchain output into the mounted workspace log file.
    /// </summary>
    /// <param name="workspace">Resolved build workspace used for mounted log roots.</param>
    /// <param name="logFileName">Log file name written beneath the mounted workspace logs root.</param>
    /// <param name="makeArguments">Arguments forwarded to <c>make</c>.</param>
    /// <returns>Shell command executed inside the Docker container.</returns>
    static string BuildDockerMakeShellCommand(
        NintendoDsBuildWorkspace workspace,
        string logFileName,
        IReadOnlyList<string> makeArguments) {
        if (workspace == null) {
            throw new ArgumentNullException(nameof(workspace));
        } else if (string.IsNullOrWhiteSpace(logFileName)) {
            throw new ArgumentException("Log file name must be provided.", nameof(logFileName));
        }

        string containerLogPath = workspace.ContainerNativeBuildLogsRootPath.TrimEnd('/') + "/" + logFileName;
        System.Text.StringBuilder commandBuilder = new();
        commandBuilder.Append("mkdir -p ");
        commandBuilder.Append(EscapeShellArgument(workspace.ContainerNativeBuildLogsRootPath));
        commandBuilder.Append(" && make");
        if (makeArguments != null) {
            for (int index = 0; index < makeArguments.Count; index++) {
                string makeArgument = makeArguments[index];
                if (!string.IsNullOrWhiteSpace(makeArgument)) {
                    commandBuilder.Append(' ');
                    commandBuilder.Append(EscapeShellArgument(makeArgument));
                }
            }
        }
        commandBuilder.Append(" > ");
        commandBuilder.Append(EscapeShellArgument(containerLogPath));
        commandBuilder.Append(" 2>&1");
        return commandBuilder.ToString();
    }

    /// <summary>
    /// Escapes one shell argument for use inside the Docker container shell command.
    /// </summary>
    /// <param name="value">Unescaped shell argument.</param>
    /// <returns>Shell-escaped argument wrapped in single quotes.</returns>
    static string EscapeShellArgument(string value) {
        if (string.IsNullOrWhiteSpace(value)) {
            throw new ArgumentException("Shell argument must be provided.", nameof(value));
        }

        return "'" + value.Replace("'", "'\"'\"'") + "'";
    }

    /// <summary>
    /// Reads one native build log file when it exists so Docker stream failures still preserve the underlying toolchain output.
    /// </summary>
    /// <param name="hostLogPath">Host path to the redirected native build log.</param>
    /// <returns>Native build log contents when available; otherwise an empty string.</returns>
    static string ReadNativeBuildLogContents(string hostLogPath) {
        if (string.IsNullOrWhiteSpace(hostLogPath) || !File.Exists(hostLogPath)) {
            return string.Empty;
        }

        return File.ReadAllText(hostLogPath);
    }
}
