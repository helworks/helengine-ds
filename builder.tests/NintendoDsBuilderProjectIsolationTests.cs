namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS builder project keeps DS-specific implementation isolated within the helengine-ds repository.
/// </summary>
public sealed class NintendoDsBuilderProjectIsolationTests {
    /// <summary>
    /// Ensures the builder project no longer takes a direct compile-time dependency on helengine.editor.
    /// </summary>
    [Fact]
    public void BuilderProject_doesNotReferenceHelengineEditor() {
        string projectFilePath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "builder", "helengine.ds.builder.csproj"));
        string projectFileSource = File.ReadAllText(projectFilePath);

        Assert.DoesNotContain("helengine.editor", projectFileSource, StringComparison.OrdinalIgnoreCase);
    }
}
