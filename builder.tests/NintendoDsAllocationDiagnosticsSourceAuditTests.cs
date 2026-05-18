namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS allocation diagnostics source so live heap growth can be observed from the native diagnostics console.
/// </summary>
public class NintendoDsAllocationDiagnosticsSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS allocation diagnostics track current and peak allocated bytes through the global allocation hook.
    /// </summary>
    [Fact]
    public void Source_whenTrackingNintendoDsHeapUsage_exposesCurrentAndPeakAllocatedBytes() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsAllocationDiagnostics.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsAllocationDiagnostics.cpp");
        string headerCode = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("static std::size_t GetCurrentAllocatedSize();", headerCode, StringComparison.Ordinal);
        Assert.Contains("static std::size_t GetPeakAllocatedSize();", headerCode, StringComparison.Ordinal);
        Assert.Contains("std::size_t CurrentAllocatedSizeValue = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::size_t PeakAllocatedSizeValue = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("CurrentAllocatedSizeValue += size;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (CurrentAllocatedSizeValue > PeakAllocatedSizeValue) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("CurrentAllocatedSizeValue -= allocationSize;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::size_t NintendoDsAllocationDiagnostics::GetCurrentAllocatedSize()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::size_t NintendoDsAllocationDiagnostics::GetPeakAllocatedSize()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("static std::size_t GetTotalAllocatedSize();", headerCode, StringComparison.Ordinal);
        Assert.Contains("static std::size_t GetTotalFreedSize();", headerCode, StringComparison.Ordinal);
        Assert.Contains("std::size_t TotalAllocatedSizeValue = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::size_t TotalFreedSizeValue = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TotalAllocatedSizeValue += size;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TotalFreedSizeValue += allocationSize;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::size_t NintendoDsAllocationDiagnostics::GetTotalAllocatedSize()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::size_t NintendoDsAllocationDiagnostics::GetTotalFreedSize()", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS allocation diagnostics cover aligned and nothrow global allocation forms so transient standard-library allocations do not escape live-byte accounting.
    /// </summary>
    [Fact]
    public void Source_whenTrackingNintendoDsHeapUsage_coversAlignedAndNothrowAllocationForms() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsAllocationDiagnostics.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void* operator new(std::size_t size, std::align_val_t alignment)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void* operator new[](std::size_t size, std::align_val_t alignment)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void* operator new(std::size_t size, const std::nothrow_t&", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void* operator new[](std::size_t size, const std::nothrow_t&", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void operator delete(void* memory, std::align_val_t alignment) noexcept", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void operator delete[](void* memory, std::align_val_t alignment) noexcept", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void operator delete(void* memory, const std::nothrow_t&", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void operator delete[](void* memory, const std::nothrow_t&", sourceCode, StringComparison.Ordinal);
    }
}
