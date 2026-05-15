namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS renderer source so performance-sensitive draw-path contracts stay intact.
/// </summary>
public class NintendoDsRenderManager3DPerformanceSourceAuditTests {
    /// <summary>
    /// Verifies frame lighting is resolved once per draw from runtime-managed light collections.
    /// </summary>
    [Fact]
    public void Source_whenResolvingFrameLighting_usesObjectManagerLightCollectionsOutsideDrawableLoop() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("float3 FrameLightDirection;", headerSource, StringComparison.Ordinal);
        Assert.Contains("float3 FrameDirectionalRadiance;", headerSource, StringComparison.Ordinal);
        Assert.Contains("float3 FrameAmbientRadiance;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ResolveFrameLighting(ObjectManager* objectManager);", headerSource, StringComparison.Ordinal);
        Assert.Contains("ResolveFrameLighting(objectManager);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("objectManager->get_DirectionalLights()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("objectManager->get_AmbientLights()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("ResolveSceneLighting(lightDirection, directionalRadiance, ambientRadiance);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies drawable-scope transform inputs are cached outside the triangle loop.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingTriangles_cachesEntityTransformOutsidePerTriangleWork() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains(
            "void SubmitLitTriangle(\n"
            + "            NintendoDsRuntimeMaterial* runtimeMaterial,\n"
            + "            Array<float3>* positions,\n"
            + "            int32_t indexA,\n"
            + "            int32_t indexB,\n"
            + "            int32_t indexC,\n"
            + "            const float3& entityPosition,\n"
            + "            const float3& entityScale,\n"
            + "            const float4& entityOrientation);",
            headerSource,
            StringComparison.Ordinal);
        Assert.Contains("Entity* entity = drawable->get_Parent();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float3 entityPosition = float3::get_Zero();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float3 entityScale = float3::get_One();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float4 entityOrientation = float4::get_Identity();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TransformVertex((*positions)[indexA], entityPosition, entityScale, entityOrientation)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TransformVertex(drawable, (*positions)[indexA])", sourceCode, StringComparison.Ordinal);
    }
}
