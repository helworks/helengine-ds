namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS transform-matrix packing so hardware-submitted mesh rotation matches the engine quaternion matrix layout exactly.
/// </summary>
public class NintendoDsTransformMatrixSourceAuditTests {
    /// <summary>
    /// Verifies the DS affine transform matrix matches the engine quaternion-to-matrix layout that drives all other runtime backends.
    /// </summary>
    [Fact]
    public void Source_buildDrawableTransformMatrix_matchesEngineQuaternionMatrixLayout() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string renderSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string renderSource = File.ReadAllText(renderSourcePath);

        Assert.Contains("transformMatrix.m[0] = floattof32((1.0f - yy - zz) * entityScale.X);", renderSource, StringComparison.Ordinal);
        Assert.Contains("transformMatrix.m[1] = floattof32((xy + wz) * entityScale.X);", renderSource, StringComparison.Ordinal);
        Assert.Contains("transformMatrix.m[2] = floattof32((xz - wy) * entityScale.X);", renderSource, StringComparison.Ordinal);
        Assert.Contains("transformMatrix.m[3] = floattof32((xy - wz) * entityScale.Y);", renderSource, StringComparison.Ordinal);
        Assert.Contains("transformMatrix.m[4] = floattof32((1.0f - xx - zz) * entityScale.Y);", renderSource, StringComparison.Ordinal);
        Assert.Contains("transformMatrix.m[5] = floattof32((yz + wx) * entityScale.Y);", renderSource, StringComparison.Ordinal);
        Assert.Contains("transformMatrix.m[6] = floattof32((xz + wy) * entityScale.Z);", renderSource, StringComparison.Ordinal);
        Assert.Contains("transformMatrix.m[7] = floattof32((yz - wx) * entityScale.Z);", renderSource, StringComparison.Ordinal);
        Assert.Contains("transformMatrix.m[8] = floattof32((1.0f - xx - yy) * entityScale.Z);", renderSource, StringComparison.Ordinal);
    }
}
