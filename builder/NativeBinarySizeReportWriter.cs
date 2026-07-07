using System.Text;
using System.Text.RegularExpressions;

namespace helengine.ds.builder;

/// <summary>
/// Parses GNU linker-map output and writes one human-readable native binary size report backed by final linked artifacts.
/// </summary>
public sealed class NativeBinarySizeReportWriter {
    /// <summary>
    /// Regular expression used to parse one inline GNU linker-map contribution row.
    /// </summary>
    static readonly Regex InlineContributionPattern = new(
        @"^\s*(\.[^\s]+)\s+0x[0-9A-Fa-f]+\s+0x([0-9A-Fa-f]+)\s+(.+?)\s*$",
        RegexOptions.Compiled);

    /// <summary>
    /// Regular expression used to parse one GNU linker-map subsection header row.
    /// </summary>
    static readonly Regex SectionHeaderPattern = new(
        @"^\s*(\.[^\s]+)\s*$",
        RegexOptions.Compiled);

    /// <summary>
    /// Regular expression used to parse one continuation row that carries address, size, and source object after a subsection header.
    /// </summary>
    static readonly Regex ContinuationContributionPattern = new(
        @"^\s*0x[0-9A-Fa-f]+\s+0x([0-9A-Fa-f]+)\s+(.+?)\s*$",
        RegexOptions.Compiled);

    /// <summary>
    /// Builds one native binary size report from the final linker-map, ELF, and packaged binary outputs.
    /// </summary>
    /// <param name="mapPath">Path to the GNU linker map emitted by the native build.</param>
    /// <param name="elfPath">Path to the linked ELF emitted by the native build.</param>
    /// <param name="packagePath">Path to the final packaged native binary.</param>
    /// <returns>Aggregated linked native binary size report.</returns>
    public NativeBinarySizeReport Build(string mapPath, string elfPath, string packagePath) {
        if (string.IsNullOrWhiteSpace(mapPath)) {
            throw new ArgumentException("Map path must be provided.", nameof(mapPath));
        } else if (string.IsNullOrWhiteSpace(elfPath)) {
            throw new ArgumentException("ELF path must be provided.", nameof(elfPath));
        } else if (string.IsNullOrWhiteSpace(packagePath)) {
            throw new ArgumentException("Package path must be provided.", nameof(packagePath));
        } else if (!File.Exists(mapPath)) {
            throw new FileNotFoundException("Native linker map was not found.", mapPath);
        } else if (!File.Exists(elfPath)) {
            throw new FileNotFoundException("Linked native ELF was not found.", elfPath);
        } else if (!File.Exists(packagePath)) {
            throw new FileNotFoundException("Packaged native binary was not found.", packagePath);
        }

        Dictionary<string, long> sectionTotals = new(StringComparer.Ordinal);
        Dictionary<string, Dictionary<string, long>> objectSectionTotals = new(StringComparer.Ordinal);
        string pendingSectionName = string.Empty;

        foreach (string line in File.ReadLines(mapPath)) {
            if (TryParseInlineContribution(line, out string inlineSectionName, out long inlineSizeBytes, out string inlineObjectPath)) {
                AddContribution(sectionTotals, objectSectionTotals, inlineSectionName, inlineSizeBytes, inlineObjectPath);
                pendingSectionName = string.Empty;
                continue;
            }

            if (TryParseSectionHeader(line, out string sectionHeaderName)) {
                pendingSectionName = sectionHeaderName;
                continue;
            }

            if (!string.IsNullOrEmpty(pendingSectionName) && TryParseContinuationContribution(line, out long continuationSizeBytes, out string continuationObjectPath)) {
                AddContribution(sectionTotals, objectSectionTotals, pendingSectionName, continuationSizeBytes, continuationObjectPath);
                pendingSectionName = string.Empty;
                continue;
            }

            pendingSectionName = string.Empty;
        }

        List<NativeBinarySectionSize> normalizedSectionSizes = BuildSortedSectionSizes(sectionTotals);
        List<NativeBinaryObjectContribution> normalizedObjectContributions = BuildSortedObjectContributions(objectSectionTotals);
        long accountedLinkedSizeBytes = normalizedSectionSizes.Sum(section => section.SizeBytes);

        return new NativeBinarySizeReport(
            new FileInfo(packagePath).Length,
            new FileInfo(elfPath).Length,
            accountedLinkedSizeBytes,
            normalizedSectionSizes,
            normalizedObjectContributions);
    }

    /// <summary>
    /// Writes one human-readable native binary size report from the final linker-map, ELF, and packaged binary outputs.
    /// </summary>
    /// <param name="mapPath">Path to the GNU linker map emitted by the native build.</param>
    /// <param name="elfPath">Path to the linked ELF emitted by the native build.</param>
    /// <param name="packagePath">Path to the final packaged native binary.</param>
    /// <param name="reportPath">Destination path for the text report.</param>
    public void WriteReport(string mapPath, string elfPath, string packagePath, string reportPath) {
        if (string.IsNullOrWhiteSpace(reportPath)) {
            throw new ArgumentException("Report path must be provided.", nameof(reportPath));
        }

        NativeBinarySizeReport report = Build(mapPath, elfPath, packagePath);
        string reportDirectoryPath = Path.GetDirectoryName(reportPath)
            ?? throw new InvalidOperationException("Unable to resolve the native binary size report directory.");
        Directory.CreateDirectory(reportDirectoryPath);
        File.WriteAllText(reportPath, RenderReportText(report));
    }

    /// <summary>
    /// Renders one human-readable text report from aggregated native binary size data.
    /// </summary>
    /// <param name="report">Aggregated native binary size report to render.</param>
    /// <returns>Human-readable text report.</returns>
    static string RenderReportText(NativeBinarySizeReport report) {
        if (report == null) {
            throw new ArgumentNullException(nameof(report));
        }

        StringBuilder builder = new();
        builder.AppendLine("Native Binary Size Report");
        builder.AppendLine("=========================");
        builder.AppendLine("Package Size: " + report.PackageSizeBytes + " bytes");
        builder.AppendLine("Linked ELF Size: " + report.LinkedElfSizeBytes + " bytes");
        builder.AppendLine("Accounted Native Binary Size: " + report.AccountedLinkedSizeBytes + " bytes");
        builder.AppendLine("Note: package size includes non-binary payload data; the accounted native binary size excludes cooked assets, debug sections, linker aggregate rows, and RAM-only sections such as .bss.");
        builder.AppendLine();
        builder.AppendLine("Section Totals");
        builder.AppendLine("--------------");

        foreach (NativeBinarySectionSize sectionSize in report.SectionSizes) {
            builder.AppendLine(sectionSize.SectionName + ": " + sectionSize.SizeBytes + " bytes");
        }

        builder.AppendLine();
        builder.AppendLine("Object Contributions");
        builder.AppendLine("--------------------");

        foreach (NativeBinaryObjectContribution objectContribution in report.ObjectContributions) {
            builder.AppendLine(
                objectContribution.ObjectDisplayName
                + ": "
                + objectContribution.TotalSizeBytes
                + " bytes ["
                + BuildSectionBreakdownText(objectContribution.SectionSizes)
                + "]");
        }

        return builder.ToString();
    }

    /// <summary>
    /// Builds one compact inline section breakdown string for a single object contribution.
    /// </summary>
    /// <param name="sectionSizes">Normalized section sizes attributed to one object contribution.</param>
    /// <returns>Compact inline section breakdown string.</returns>
    static string BuildSectionBreakdownText(IReadOnlyList<NativeBinarySectionSize> sectionSizes) {
        if (sectionSizes == null) {
            throw new ArgumentNullException(nameof(sectionSizes));
        }

        StringBuilder builder = new();

        for (int sectionIndex = 0; sectionIndex < sectionSizes.Count; sectionIndex++) {
            if (sectionIndex > 0) {
                builder.Append(", ");
            }

            NativeBinarySectionSize sectionSize = sectionSizes[sectionIndex];
            builder.Append(sectionSize.SectionName);
            builder.Append('=');
            builder.Append(sectionSize.SizeBytes);
        }

        return builder.ToString();
    }

    /// <summary>
    /// Builds a sorted linked-section total list from one mutable section-size dictionary.
    /// </summary>
    /// <param name="sectionTotals">Mutable section totals keyed by normalized section name.</param>
    /// <returns>Sorted linked-section totals.</returns>
    static List<NativeBinarySectionSize> BuildSortedSectionSizes(Dictionary<string, long> sectionTotals) {
        if (sectionTotals == null) {
            throw new ArgumentNullException(nameof(sectionTotals));
        }

        return sectionTotals
            .Select(sectionTotal => new NativeBinarySectionSize(sectionTotal.Key, sectionTotal.Value))
            .OrderByDescending(sectionSize => sectionSize.SizeBytes)
            .ThenBy(sectionSize => sectionSize.SectionName, StringComparer.Ordinal)
            .ToList();
    }

    /// <summary>
    /// Builds a sorted object-contribution list from one mutable per-object section-size dictionary.
    /// </summary>
    /// <param name="objectSectionTotals">Mutable per-object section totals keyed by object display name.</param>
    /// <returns>Sorted object contribution list.</returns>
    static List<NativeBinaryObjectContribution> BuildSortedObjectContributions(Dictionary<string, Dictionary<string, long>> objectSectionTotals) {
        if (objectSectionTotals == null) {
            throw new ArgumentNullException(nameof(objectSectionTotals));
        }

        List<NativeBinaryObjectContribution> objectContributions = [];

        foreach (KeyValuePair<string, Dictionary<string, long>> objectSectionTotal in objectSectionTotals) {
            List<NativeBinarySectionSize> sectionSizes = BuildSortedSectionSizes(objectSectionTotal.Value);
            long totalSizeBytes = sectionSizes.Sum(sectionSize => sectionSize.SizeBytes);
            objectContributions.Add(new NativeBinaryObjectContribution(objectSectionTotal.Key, totalSizeBytes, sectionSizes));
        }

        return objectContributions
            .OrderByDescending(objectContribution => objectContribution.TotalSizeBytes)
            .ThenBy(objectContribution => objectContribution.ObjectDisplayName, StringComparer.Ordinal)
            .ToList();
    }

    /// <summary>
    /// Adds one parsed linker-map contribution into mutable section and per-object aggregation dictionaries.
    /// </summary>
    /// <param name="sectionTotals">Mutable section totals keyed by normalized section name.</param>
    /// <param name="objectSectionTotals">Mutable per-object section totals keyed by object display name.</param>
    /// <param name="rawSectionName">Raw linker-map section name.</param>
    /// <param name="sizeBytes">Linked byte size attributed to the contribution.</param>
    /// <param name="rawObjectPath">Raw source object path or archive-member token from the linker map.</param>
    static void AddContribution(
        Dictionary<string, long> sectionTotals,
        Dictionary<string, Dictionary<string, long>> objectSectionTotals,
        string rawSectionName,
        long sizeBytes,
        string rawObjectPath) {
        if (sectionTotals == null) {
            throw new ArgumentNullException(nameof(sectionTotals));
        } else if (objectSectionTotals == null) {
            throw new ArgumentNullException(nameof(objectSectionTotals));
        } else if (sizeBytes <= 0) {
            return;
        }

        string normalizedSectionName = NormalizeSectionName(rawSectionName);
        if (!ShouldIncludeSection(normalizedSectionName) || !IsConcreteObjectToken(rawObjectPath)) {
            return;
        }

        string normalizedObjectDisplayName = NormalizeObjectDisplayName(rawObjectPath);
        AddBytes(sectionTotals, normalizedSectionName, sizeBytes);

        if (!objectSectionTotals.TryGetValue(normalizedObjectDisplayName, out Dictionary<string, long> perObjectSections)) {
            perObjectSections = new Dictionary<string, long>(StringComparer.Ordinal);
            objectSectionTotals.Add(normalizedObjectDisplayName, perObjectSections);
        }

        AddBytes(perObjectSections, normalizedSectionName, sizeBytes);
    }

    /// <summary>
    /// Adds one byte count into a mutable aggregation dictionary keyed by exact string name.
    /// </summary>
    /// <param name="totals">Mutable aggregation dictionary keyed by exact string name.</param>
    /// <param name="key">Exact aggregation key to update.</param>
    /// <param name="sizeBytes">Byte count to add.</param>
    static void AddBytes(Dictionary<string, long> totals, string key, long sizeBytes) {
        if (totals == null) {
            throw new ArgumentNullException(nameof(totals));
        } else if (string.IsNullOrWhiteSpace(key)) {
            throw new ArgumentException("Aggregation key must be provided.", nameof(key));
        } else if (sizeBytes < 0) {
            throw new ArgumentOutOfRangeException(nameof(sizeBytes), sizeBytes, "Byte count must be zero or greater.");
        }

        if (totals.TryGetValue(key, out long existingSizeBytes)) {
            totals[key] = existingSizeBytes + sizeBytes;
        } else {
            totals.Add(key, sizeBytes);
        }
    }

    /// <summary>
    /// Parses one inline GNU linker-map contribution row that includes the section, address, size, and source object on a single line.
    /// </summary>
    /// <param name="line">Raw linker-map line.</param>
    /// <param name="sectionName">Parsed raw section name.</param>
    /// <param name="sizeBytes">Parsed linked byte size.</param>
    /// <param name="objectPath">Parsed raw source object path or archive-member token.</param>
    /// <returns><c>true</c> when the line matched the inline contribution pattern; otherwise <c>false</c>.</returns>
    static bool TryParseInlineContribution(string line, out string sectionName, out long sizeBytes, out string objectPath) {
        sectionName = string.Empty;
        sizeBytes = 0;
        objectPath = string.Empty;

        if (string.IsNullOrWhiteSpace(line)) {
            return false;
        }

        Match match = InlineContributionPattern.Match(line);

        if (!match.Success) {
            return false;
        }

        sectionName = match.Groups[1].Value;
        sizeBytes = Convert.ToInt64(match.Groups[2].Value, 16);
        objectPath = match.Groups[3].Value.Trim();
        return true;
    }

    /// <summary>
    /// Parses one GNU linker-map subsection header row.
    /// </summary>
    /// <param name="line">Raw linker-map line.</param>
    /// <param name="sectionName">Parsed raw subsection name.</param>
    /// <returns><c>true</c> when the line matched the subsection-header pattern; otherwise <c>false</c>.</returns>
    static bool TryParseSectionHeader(string line, out string sectionName) {
        sectionName = string.Empty;

        if (string.IsNullOrWhiteSpace(line)) {
            return false;
        }

        Match match = SectionHeaderPattern.Match(line);

        if (!match.Success) {
            return false;
        }

        sectionName = match.Groups[1].Value;
        return true;
    }

    /// <summary>
    /// Parses one GNU linker-map continuation row that carries address, size, and source object after a subsection header.
    /// </summary>
    /// <param name="line">Raw linker-map line.</param>
    /// <param name="sizeBytes">Parsed linked byte size.</param>
    /// <param name="objectPath">Parsed raw source object path or archive-member token.</param>
    /// <returns><c>true</c> when the line matched the continuation contribution pattern; otherwise <c>false</c>.</returns>
    static bool TryParseContinuationContribution(string line, out long sizeBytes, out string objectPath) {
        sizeBytes = 0;
        objectPath = string.Empty;

        if (string.IsNullOrWhiteSpace(line)) {
            return false;
        }

        Match match = ContinuationContributionPattern.Match(line);

        if (!match.Success) {
            return false;
        }

        sizeBytes = Convert.ToInt64(match.Groups[1].Value, 16);
        objectPath = match.Groups[2].Value.Trim();
        return true;
    }

    /// <summary>
    /// Normalizes one raw linker-map section name into a stable root section bucket for report aggregation.
    /// </summary>
    /// <param name="rawSectionName">Raw linker-map section name.</param>
    /// <returns>Normalized root section name.</returns>
    static string NormalizeSectionName(string rawSectionName) {
        if (string.IsNullOrWhiteSpace(rawSectionName)) {
            throw new ArgumentException("Raw section name must be provided.", nameof(rawSectionName));
        } else if (rawSectionName.StartsWith(".ARM.exidx", StringComparison.Ordinal)) {
            return ".ARM.exidx";
        } else if (rawSectionName.StartsWith(".ARM.extab", StringComparison.Ordinal)) {
            return ".ARM.extab";
        } else if (rawSectionName.StartsWith(".text", StringComparison.Ordinal)) {
            return ".text";
        } else if (rawSectionName.StartsWith(".rodata", StringComparison.Ordinal)) {
            return ".rodata";
        } else if (rawSectionName.StartsWith(".data", StringComparison.Ordinal)) {
            return ".data";
        } else if (rawSectionName.StartsWith(".bss", StringComparison.Ordinal)) {
            return ".bss";
        }

        int secondDotIndex = rawSectionName.IndexOf('.', 1);

        if (secondDotIndex > 0) {
            return rawSectionName.Substring(0, secondDotIndex);
        }

        return rawSectionName;
    }

    /// <summary>
    /// Normalizes one raw linker-map object token into a stable display name for report output.
    /// </summary>
    /// <param name="rawObjectPath">Raw source object path or archive-member token from the linker map.</param>
    /// <returns>Stable object contribution display name.</returns>
    static string NormalizeObjectDisplayName(string rawObjectPath) {
        if (string.IsNullOrWhiteSpace(rawObjectPath)) {
            throw new ArgumentException("Raw object path must be provided.", nameof(rawObjectPath));
        }

        string trimmedObjectPath = rawObjectPath.Trim();
        int archiveMemberSeparatorIndex = trimmedObjectPath.IndexOf('(', StringComparison.Ordinal);
        bool hasArchiveMember = archiveMemberSeparatorIndex > 0 && trimmedObjectPath.EndsWith(")", StringComparison.Ordinal);

        if (hasArchiveMember) {
            string archivePath = trimmedObjectPath.Substring(0, archiveMemberSeparatorIndex);
            string archiveMemberName = trimmedObjectPath.Substring(archiveMemberSeparatorIndex + 1, trimmedObjectPath.Length - archiveMemberSeparatorIndex - 2);
            return Path.GetFileName(archivePath) + "(" + archiveMemberName + ")";
        }

        return Path.GetFileName(trimmedObjectPath);
    }

    /// <summary>
    /// Returns whether one normalized section name represents bytes that actually ship inside the packaged native binary.
    /// </summary>
    /// <param name="normalizedSectionName">Normalized root section name.</param>
    /// <returns><c>true</c> when the section should be counted toward packaged binary attribution; otherwise <c>false</c>.</returns>
    static bool ShouldIncludeSection(string normalizedSectionName) {
        if (string.IsNullOrWhiteSpace(normalizedSectionName)) {
            throw new ArgumentException("Normalized section name must be provided.", nameof(normalizedSectionName));
        }

        return normalizedSectionName == ".text"
            || normalizedSectionName == ".rodata"
            || normalizedSectionName == ".data"
            || normalizedSectionName == ".ARM.exidx"
            || normalizedSectionName == ".ARM.extab"
            || normalizedSectionName == ".init_array"
            || normalizedSectionName == ".fini_array"
            || normalizedSectionName == ".init"
            || normalizedSectionName == ".fini"
            || normalizedSectionName == ".eh_frame"
            || normalizedSectionName == ".dldi"
            || normalizedSectionName == ".secure"
            || normalizedSectionName == ".itcm"
            || normalizedSectionName == ".dtcm"
            || normalizedSectionName == ".vectors"
            || normalizedSectionName == ".loadlist"
            || normalizedSectionName == ".crt0"
            || normalizedSectionName == ".bootstub"
            || normalizedSectionName == ".twl"
            || normalizedSectionName == ".tinfo";
    }

    /// <summary>
    /// Returns whether one raw linker-map object token represents a concrete object file or archive member instead of a linker-generated aggregate row.
    /// </summary>
    /// <param name="rawObjectPath">Raw source object path or aggregate token from the linker map.</param>
    /// <returns><c>true</c> when the token represents a concrete object contribution; otherwise <c>false</c>.</returns>
    static bool IsConcreteObjectToken(string rawObjectPath) {
        if (string.IsNullOrWhiteSpace(rawObjectPath)) {
            throw new ArgumentException("Raw object path must be provided.", nameof(rawObjectPath));
        }

        string trimmedObjectPath = rawObjectPath.Trim();
        return trimmedObjectPath.EndsWith(".o", StringComparison.Ordinal)
            || Regex.IsMatch(trimmedObjectPath, @"\.a\([^)]+\.o\)$", RegexOptions.CultureInvariant);
    }
}
