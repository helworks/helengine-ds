namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS renderer source so generated-core staging and native compilation keep the same cooked-material contract.
/// </summary>
public class NintendoDsRenderManager3DSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS renderer always compiles its cooked platform-owned material override when generated core is enabled.
    /// </summary>
    [Fact]
    public void Source_whenDsUsesGeneratedCore_doesNotGateCookedMaterialOverrideBehindUndefinedSymbol() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterial* BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset)", sourceCode, StringComparison.Ordinal);
    }
}
