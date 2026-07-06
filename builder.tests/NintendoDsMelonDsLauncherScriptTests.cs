namespace helengine.ds.builder.tests;

/// <summary>
/// Guards the canonical Nintendo DS emulator launcher contract.
/// </summary>
public sealed class NintendoDsMelonDsLauncherScriptTests {
    /// <summary>
    /// Ensures the canonical launcher requires one explicit artifact path and preserves the melonDS launch workflow.
    /// </summary>
    [Fact]
    public void Launcher_RequiresArtifactPath_AndLaunchesMelonDs() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string scriptPath = Path.Combine(repositoryRootPath, "scripts", "launch_in_emulator.ps1");

        Assert.True(File.Exists(scriptPath), "Expected scripts/launch_in_emulator.ps1 to exist.");

        string scriptSource = File.ReadAllText(scriptPath);

        Assert.Contains("[string]$ArtifactPath", scriptSource, StringComparison.Ordinal);
        Assert.Contains("[string]$LogDirectory", scriptSource, StringComparison.Ordinal);
        Assert.Contains("melonDS.exe", scriptSource, StringComparison.Ordinal);
        Assert.Contains("-RedirectStandardOutput $StdoutLogPath", scriptSource, StringComparison.Ordinal);
        Assert.Contains("-RedirectStandardError $StderrLogPath", scriptSource, StringComparison.Ordinal);
        Assert.Contains("PROCESS_ID", scriptSource, StringComparison.Ordinal);
        Assert.Contains("STDOUT_LOG", scriptSource, StringComparison.Ordinal);
        Assert.Contains("STDERR_LOG", scriptSource, StringComparison.Ordinal);
        Assert.Contains("BOOT_LOG", scriptSource, StringComparison.Ordinal);
        Assert.Contains("SCENE_TRACE_LOG", scriptSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[string]$RomPath", scriptSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the root README points users at the canonical launcher entrypoint.
    /// </summary>
    [Fact]
    public void Readme_DocumentsCanonicalLauncherWorkflow() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string readmeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "README.md"));

        Assert.Contains("launch_in_emulator.ps1", readmeSource, StringComparison.Ordinal);
        Assert.Contains("-ArtifactPath", readmeSource, StringComparison.Ordinal);
        Assert.DoesNotContain("launch-melonds-rom.ps1", readmeSource, StringComparison.Ordinal);
        Assert.DoesNotContain("-RomPath", readmeSource, StringComparison.Ordinal);
    }
}
