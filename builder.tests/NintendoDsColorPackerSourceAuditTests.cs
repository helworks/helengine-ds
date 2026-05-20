namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS color packer source so opaque colors keep the visible bit enabled for bitmap presentation.
/// </summary>
public class NintendoDsColorPackerSourceAuditTests {
    /// <summary>
    /// Verifies the opaque packed-color helper sets the Nintendo DS visible bit instead of returning an all-black transparent texel.
    /// </summary>
    [Fact]
    public void Source_whenPackingOpaqueColor_setsVisibleBitInReturnedBgr5a1Value() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsColorPacker.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("return static_cast<uint16_t>((1u << 15) | packedRed | packedGreen | packedBlue);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("return static_cast<uint16_t>(packedRed | packedGreen | packedBlue);", sourceCode, StringComparison.Ordinal);
    }
}
