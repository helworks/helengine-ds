namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS native makefile so release-oriented builds strip debug-only host and renderer checks.
/// </summary>
public sealed class NintendoDsMakefileSourceAuditTests {
    /// <summary>
    /// Verifies DS builds without runtime diagnostics define <c>NDEBUG</c> so libnds assert paths and local debug-only renderer helpers can be compiled out.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeDiagnosticsAreDisabled_definesNdebugForReleaseOrientedBuilds() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefilePath = Path.Combine(repositoryRootPath, "Makefile");
        string makefile = File.ReadAllText(makefilePath);

        Assert.Contains("ifeq ($(strip $(HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS)),0)", makefile, StringComparison.Ordinal);
        Assert.Contains("CFLAGS += -DNDEBUG", makefile, StringComparison.Ordinal);
    }
}
