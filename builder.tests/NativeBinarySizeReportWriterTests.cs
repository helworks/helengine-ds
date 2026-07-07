namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies GNU linker-map parsing and native binary size report formatting for builder-owned native platforms.
/// </summary>
public sealed class NativeBinarySizeReportWriterTests {
    /// <summary>
    /// Verifies the report writer aggregates linked section sizes by normalized section root and object contributor.
    /// </summary>
    [Fact]
    public void Build_aggregates_linked_section_sizes_and_object_contributions() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-native-size-report-" + Guid.NewGuid().ToString("N"));

        try {
            Directory.CreateDirectory(rootPath);
            string mapPath = Path.Combine(rootPath, "sample.map");
            string elfPath = Path.Combine(rootPath, "sample.elf");
            string packagePath = Path.Combine(rootPath, "sample.nds");
            File.WriteAllText(
                mapPath,
                " .text.Foo\n"
                + "                0x02000000       0x10 helengine_core_unity.o\n"
                + " .rodata.Foo\n"
                + "                0x02000010        0x8 helengine_core_unity.o\n"
                + " .ARM.exidx.text.Foo\n"
                + "                0x02000018        0x8 helengine_core_unity.o\n"
                + " .text.Bar\n"
                + "                0x02000020        0xc NintendoDsRenderManager2D.o\n"
                + " .data.Bar\n"
                + "                0x0200002c        0x4 NintendoDsRenderManager2D.o\n"
                + " .bss.Bar\n"
                + "                0x02000030        0x6 NintendoDsRenderManager2D.o\n");
            File.WriteAllBytes(elfPath, new byte[128]);
            File.WriteAllBytes(packagePath, new byte[256]);

            NativeBinarySizeReportWriter writer = new();
            NativeBinarySizeReport report = writer.Build(mapPath, elfPath, packagePath);

            Assert.Equal(256, report.PackageSizeBytes);
            Assert.Equal(128, report.LinkedElfSizeBytes);
            Assert.Equal(48, report.AccountedLinkedSizeBytes);
            Assert.Equal(28, report.SectionSizes.Single(section => section.SectionName == ".text").SizeBytes);
            Assert.Equal(8, report.SectionSizes.Single(section => section.SectionName == ".rodata").SizeBytes);
            Assert.Equal(4, report.SectionSizes.Single(section => section.SectionName == ".data").SizeBytes);
            Assert.DoesNotContain(report.SectionSizes, section => section.SectionName == ".bss");
            Assert.Equal(8, report.SectionSizes.Single(section => section.SectionName == ".ARM.exidx").SizeBytes);
            Assert.Equal("helengine_core_unity.o", report.ObjectContributions[0].ObjectDisplayName);
            Assert.Equal(32, report.ObjectContributions[0].TotalSizeBytes);
            Assert.Equal("NintendoDsRenderManager2D.o", report.ObjectContributions[1].ObjectDisplayName);
            Assert.Equal(16, report.ObjectContributions[1].TotalSizeBytes);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the report writer emits a human-readable report that includes the package size, section totals, and object contributions.
    /// </summary>
    [Fact]
    public void WriteReport_writes_human_readable_text_report() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-native-size-report-" + Guid.NewGuid().ToString("N"));

        try {
            Directory.CreateDirectory(rootPath);
            string mapPath = Path.Combine(rootPath, "sample.map");
            string elfPath = Path.Combine(rootPath, "sample.elf");
            string packagePath = Path.Combine(rootPath, "sample.nds");
            string reportPath = Path.Combine(rootPath, "sample-report.txt");
            File.WriteAllText(
                mapPath,
                " .text.Foo\n"
                + "                0x02000000       0x10 helengine_core_unity.o\n");
            File.WriteAllBytes(elfPath, new byte[128]);
            File.WriteAllBytes(packagePath, new byte[256]);

            NativeBinarySizeReportWriter writer = new();
            writer.WriteReport(mapPath, elfPath, packagePath, reportPath);

            string reportText = File.ReadAllText(reportPath);
            Assert.Contains("Package Size: 256 bytes", reportText, StringComparison.Ordinal);
            Assert.Contains(".text: 16 bytes", reportText, StringComparison.Ordinal);
            Assert.Contains("helengine_core_unity.o", reportText, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }
}
