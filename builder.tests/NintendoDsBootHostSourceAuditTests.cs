namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS boot host source so release startup enters the generated-core runtime loop instead of the diagnostic smoke test.
/// </summary>
public class NintendoDsBootHostSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS boot host run path enters checkpointed startup and does not stop in the status-console smoke test.
    /// </summary>
    [Fact]
    public void Source_whenGeneratedCoreIsEnabled_runUsesCheckpointedStartupInsteadOfSmokeTest() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains(
            "#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE\n"
            + "            RunCheckpointedStartup();\n"
            + "#else\n"
            + "            RunStatusConsoleSmokeTest();\n"
            + "#endif",
            sourceCode,
            StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host distinguishes menu startup presentation from the default 3D startup path.
    /// </summary>
    [Fact]
    public void Source_whenCheckpointedStartupCompletes_usesConfiguredTopScreenPresentationPath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("PrepareMainScreenForConfiguredStartupScene();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsBootHost::PrepareMainScreenForMenu2D()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsBootHost::PrepareMainScreenFor3D()", sourceCode, StringComparison.Ordinal);
    }
}
