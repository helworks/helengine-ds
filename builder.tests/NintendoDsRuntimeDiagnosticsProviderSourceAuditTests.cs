namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS runtime diagnostics provider source so release-oriented builds can strip diagnostic-only formatting paths.
/// </summary>
public class NintendoDsRuntimeDiagnosticsProviderSourceAuditTests {
    /// <summary>
    /// Verifies scene-transition trace formatting is compiled only when native DS runtime diagnostics are enabled, leaving release builds with a zero-cost no-op implementation.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeDiagnosticsAreDisabled_guardsSceneTransitionTraceFormattingBehindCompileTimeFlag() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeDiagnosticsProvider.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void NintendoDsRuntimeDiagnosticsProvider::ReportSceneTransitionStage(std::string stage, std::string sceneId, int32_t loadedSceneCount, int32_t pendingOperationCount) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS\n        std::string line = \"[helengine-ds] stage=\" + stage", sourceCode, StringComparison.Ordinal);
        Assert.Contains("+ \" loaded=\" + std::to_string(loadedSceneCount)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("+ \" pending=\" + std::to_string(pendingOperationCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#else\n        (void)stage;\n        (void)sceneId;\n        (void)loadedSceneCount;\n        (void)pendingOperationCount;\n#endif", sourceCode, StringComparison.Ordinal);
    }
}
