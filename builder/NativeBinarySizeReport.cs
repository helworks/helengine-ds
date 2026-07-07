namespace helengine.ds.builder;

/// <summary>
/// Represents one linked native binary size report derived from the final package and linker-map artifacts.
/// </summary>
public sealed class NativeBinarySizeReport {
    /// <summary>
    /// Initializes one native binary size report with explicit package, linked-binary, and contribution data.
    /// </summary>
    /// <param name="packageSizeBytes">Final packaged native binary size in bytes.</param>
    /// <param name="linkedElfSizeBytes">Linked ELF size in bytes.</param>
    /// <param name="accountedLinkedSizeBytes">Total linked bytes accounted for by parsed linker-map rows.</param>
    /// <param name="sectionSizes">Normalized linked section totals.</param>
    /// <param name="objectContributions">Sorted object-file linked size contributions.</param>
    public NativeBinarySizeReport(
        long packageSizeBytes,
        long linkedElfSizeBytes,
        long accountedLinkedSizeBytes,
        IReadOnlyList<NativeBinarySectionSize> sectionSizes,
        IReadOnlyList<NativeBinaryObjectContribution> objectContributions) {
        PackageSizeBytes = packageSizeBytes < 0
            ? throw new ArgumentOutOfRangeException(nameof(packageSizeBytes), packageSizeBytes, "Package size must be zero or greater.")
            : packageSizeBytes;
        LinkedElfSizeBytes = linkedElfSizeBytes < 0
            ? throw new ArgumentOutOfRangeException(nameof(linkedElfSizeBytes), linkedElfSizeBytes, "Linked ELF size must be zero or greater.")
            : linkedElfSizeBytes;
        AccountedLinkedSizeBytes = accountedLinkedSizeBytes < 0
            ? throw new ArgumentOutOfRangeException(nameof(accountedLinkedSizeBytes), accountedLinkedSizeBytes, "Accounted linked size must be zero or greater.")
            : accountedLinkedSizeBytes;
        SectionSizes = sectionSizes ?? throw new ArgumentNullException(nameof(sectionSizes));
        ObjectContributions = objectContributions ?? throw new ArgumentNullException(nameof(objectContributions));
    }

    /// <summary>
    /// Gets the final packaged native binary size in bytes.
    /// </summary>
    public long PackageSizeBytes { get; }

    /// <summary>
    /// Gets the linked ELF size in bytes.
    /// </summary>
    public long LinkedElfSizeBytes { get; }

    /// <summary>
    /// Gets the total linked bytes accounted for by parsed linker-map rows.
    /// </summary>
    public long AccountedLinkedSizeBytes { get; }

    /// <summary>
    /// Gets the normalized linked section totals.
    /// </summary>
    public IReadOnlyList<NativeBinarySectionSize> SectionSizes { get; }

    /// <summary>
    /// Gets the sorted object-file linked size contributions.
    /// </summary>
    public IReadOnlyList<NativeBinaryObjectContribution> ObjectContributions { get; }
}
