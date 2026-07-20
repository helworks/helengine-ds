using helengine.baseplatform.Builders;

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
    /// Stores the writer that emits one human-readable native binary size report after successful packaging.
    /// </summary>
    readonly NativeBinarySizeReportWriter NativeBinarySizeReportWriter = new();

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
        RunDockerMake(workspace, cancellationToken, CleanLogFileName, "clean");
        RunDockerMake(
            workspace,
            cancellationToken,
            BuildLogFileName,
            "HELENGINE_DS_NITROFS_ROOT=" + workspace.ContainerNitroFsRootPath,
            "HELENGINE_CORE_CPP_ROOT=" + workspace.ContainerGeneratedCoreRootPath,
            "HELENGINE_DS_DISABLED_RUNTIME_FEATURES=" + workspace.DisabledRuntimeFeatures,
            "HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS=" + (workspace.EnableRuntimeDiagnostics ? "1" : "0"),
            "HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE=" + (workspace.EnableFatalErrorConsole ? "1" : "0"));
        NintendoDsBuildArtifactValidator.EnsureArtifactProduced(
            workspace.RepositoryPackagePath,
            "Nintendo DS package output");
        NintendoDsBuildArtifactValidator.EnsureArtifactProduced(
            workspace.RepositoryMapPath,
            "Nintendo DS linker map output");
        NintendoDsBuildArtifactValidator.EnsureArtifactProduced(
            workspace.RepositoryElfPath,
            "Nintendo DS linked ELF output");
        File.Copy(workspace.RepositoryPackagePath, workspace.ExportPackagePath, true);
        NintendoDsBuildArtifactValidator.EnsureArtifactProduced(
            workspace.ExportPackagePath,
            "Nintendo DS exported package");
        NativeBinarySizeReportWriter.WriteReport(
            workspace.RepositoryMapPath,
            workspace.RepositoryElfPath,
            workspace.ExportPackagePath,
            workspace.ExportNativeBinarySizeReportPath);
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

        using StreamWriter logWriter = new(hostLogPath, false, new System.Text.UTF8Encoding(false));
        NativeProcessRunResult result = new NativeProcessRunner().Run(
            startInfo,
            cancellationToken,
            (line, isError) => StreamDockerOutputLine(line, logWriter, isError));
        if (result.ExitCode != 0) {
            string nativeBuildLogContents = ReadNativeBuildLogContents(hostLogPath);
            throw new InvalidOperationException(
                "Nintendo DS Docker build failed:" + Environment.NewLine
                + nativeBuildLogContents);
        }
    }

    /// <summary>
    /// Writes one Docker output line to the live console and the persistent native build log.
    /// </summary>
    /// <param name="line">Output line received from Docker.</param>
    /// <param name="logWriter">Writer for the mounted native build log.</param>
    /// <param name="isError">Whether the line came from Docker's error stream.</param>
    static void StreamDockerOutputLine(string line, StreamWriter logWriter, bool isError) {
        if (line == null) {
            return;
        }

        logWriter.WriteLine(line);
        logWriter.Flush();
        if (isError) {
            Console.Error.WriteLine(line);
        } else {
            Console.WriteLine(line);
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
    /// Builds the shell command that runs <c>make</c> while allowing the Docker process to stream native toolchain output.
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
