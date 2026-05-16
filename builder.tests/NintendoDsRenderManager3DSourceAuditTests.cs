namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS renderer source so generated-core staging and native compilation keep the same cooked-material contract.
/// </summary>
public class NintendoDsRenderManager3DSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS frame renderer traverses every camera so menu scenes can target both screens through 2D camera viewports.
    /// </summary>
    [Fact]
    public void Source_whenCameraListContains2dViewports_dispatchesEveryCameraThroughNintendoDs2DRenderer() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("renderManager2D->BeginFrame();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("for (int32_t cameraIndex = 0; cameraIndex < cameras->Count(); cameraIndex++)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("IRenderQueue2D* renderQueue2D = camera->get_RenderQueue2D();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->DrawCamera(camera);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRenderManager2D* renderManager2D", sourceCode, StringComparison.Ordinal);
    }

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

    /// <summary>
    /// Verifies the Nintendo DS renderer source includes the full constant-buffer asset definition before reading generated-core buffer metadata.
    /// </summary>
    [Fact]
    public void Source_whenResolvingStandardMaterialBaseColor_includesMaterialConstantBufferAssetDefinition() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"MaterialConstantBufferAsset.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("constantBuffer->get_Name()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("constantBuffer->get_Data()", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 3D renderer resolves top-first hardware 3D ownership and still hands 2D presentation back to the DS 2D renderer.
    /// </summary>
    [Fact]
    public void Source_whenBothScreensContain3d_topScreenWinsAnd2dPresentationStillRunsForBothScreens() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"platform/ds/NintendoDsScreenTarget.hpp\"", headerSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsScreenTarget ResolveHardware3DScreenTarget", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool topScreenHas3D = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool bottomScreenHas3D = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (topScreenHas3D) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return NintendoDsScreenTarget::Top;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (bottomScreenHas3D) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return NintendoDsScreenTarget::Bottom;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->DrawCamera(camera);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
    }
}
