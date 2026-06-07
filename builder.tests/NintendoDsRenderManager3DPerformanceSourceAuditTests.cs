namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS 3D renderer source so the performance-sensitive frame path stays hardware-only.
/// </summary>
public class NintendoDsRenderManager3DPerformanceSourceAuditTests {
    /// <summary>
    /// Verifies drawable transforms are submitted through the Nintendo DS matrix hardware instead of rotating every vertex on the ARM9.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingDrawable_usesHardwareMatrixForEntityTransform() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void ApplyDrawableTransformToHardwareMatrix(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void BuildDrawableTransformMatrix(", headerSource, StringComparison.Ordinal);
        Assert.Contains("glPushMatrix();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyDrawableTransformToHardwareMatrix(entityPosition, entityScale, entityOrientation);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glPopMatrix(1);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("m4x3 transformMatrix;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BuildDrawableTransformMatrix(transformMatrix, entityPosition, entityScale, entityOrientation);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glMultMatrix4x3(&transformMatrix);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glVertex3v16(floattov16((*positions)[indexA].X)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("glRotatef32i(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::acos(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::sqrt(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TransformVertex((*positions)[indexA], entityPosition, entityScale, entityOrientation)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies hardware-3D frames no longer preserve software 2D present plumbing after the DS software compositor removal.
    /// </summary>
    [Fact]
    public void Source_whenFrameHasOnlyHardware3d_removesSoftware2dPresentPath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("bool ShouldPresent2DFrame(NintendoDsScreenTarget hardware3DScreenTarget, NintendoDsRenderManager2D* renderManager2D) const;", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("bool shouldPresent2DFrame = ShouldPresent2DFrame(hardware3DScreenTarget, renderManager2D);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("LastPresentNetByteDelta", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies Nintendo DS render-target creation rejects placeholder support instead of fabricating unsupported off-screen capability.
    /// </summary>
    [Fact]
    public void Source_whenCreatingDsRenderTarget_rejectsPlaceholderCapabilityLie() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("RenderTarget* CreateRenderTarget(int32_t width, int32_t height) override;", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("Builds one placeholder render target", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("RenderTarget* renderTarget = new RenderTarget();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("throw new InvalidOperationException(", sourceCode, StringComparison.Ordinal);
    }
}
