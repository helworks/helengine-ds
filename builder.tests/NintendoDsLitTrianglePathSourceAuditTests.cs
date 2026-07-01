namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the DS lit-geometry submission path so flat-lit meshes stay on triangles instead of the quad path while investigating hardware quad-lighting artifacts.
/// </summary>
public class NintendoDsLitTrianglePathSourceAuditTests {
    /// <summary>
    /// Verifies the lit display-list builder skips the quad shortcut and the immediate fallback always emits lit triangles.
    /// </summary>
    [Fact]
    public void Source_litGeometry_usesTrianglePathsInsteadOfHardwareQuadShortcuts() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string renderSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string renderSource = File.ReadAllText(renderSourcePath);

        Assert.DoesNotContain("uint32_t* quadDisplayList = BuildHardwareLitQuadDisplayList(runtimeModel, displayListWordCount);", renderSource, StringComparison.Ordinal);
        Assert.DoesNotContain("if (!useHardwareTexture && runtimeModel->UsesHardwareLitQuadDisplayList)", renderSource, StringComparison.Ordinal);
        Assert.Contains("glBegin(GL_TRIANGLES);", renderSource, StringComparison.Ordinal);
        Assert.Contains("SubmitHardwareLitTriangle(positions, indexA, indexB, indexC);", renderSource, StringComparison.Ordinal);
    }
}
