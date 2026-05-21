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
        int opaqueMethodStart = sourceCode.IndexOf("uint16_t NintendoDsColorPacker::PackOpaqueColor(float red, float green, float blue)", StringComparison.Ordinal);
        int registerMethodStart = sourceCode.IndexOf("uint16_t NintendoDsColorPacker::PackRegisterColor(float red, float green, float blue)", StringComparison.Ordinal);

        Assert.True(opaqueMethodStart >= 0);
        Assert.True(registerMethodStart > opaqueMethodStart);

        string opaqueMethodSource = sourceCode.Substring(opaqueMethodStart, registerMethodStart - opaqueMethodStart);
        Assert.Contains("return static_cast<uint16_t>((1u << 15) | packedRed | packedGreen | packedBlue);", opaqueMethodSource, StringComparison.Ordinal);
        Assert.DoesNotContain("return static_cast<uint16_t>(packedRed | packedGreen | packedBlue);", opaqueMethodSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies register-color packing leaves the high material-control bit clear for DS light and material registers.
    /// </summary>
    [Fact]
    public void Source_whenPackingRegisterColor_leavesMaterialControlBitClear() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsColorPacker.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsColorPacker.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("static uint16_t PackRegisterColor(float red, float green, float blue);", headerSource, StringComparison.Ordinal);
        Assert.Contains("static uint16_t PackRegisterColor(const float3& color);", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint16_t NintendoDsColorPacker::PackRegisterColor(float red, float green, float blue)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return static_cast<uint16_t>(packedRed | packedGreen | packedBlue);", sourceCode, StringComparison.Ordinal);
    }
}
