namespace helengine.ds.builder;

/// <summary>
/// Represents one object-file contribution to a final linked native binary.
/// </summary>
public sealed class NativeBinaryObjectContribution {
    /// <summary>
    /// Initializes one object-file contribution record with explicit linked byte totals.
    /// </summary>
    /// <param name="objectDisplayName">Display name for the contributing object file or archive member.</param>
    /// <param name="totalSizeBytes">Total linked bytes attributed to the object contribution.</param>
    /// <param name="sectionSizes">Normalized linked section breakdown attributed to the object contribution.</param>
    public NativeBinaryObjectContribution(string objectDisplayName, long totalSizeBytes, IReadOnlyList<NativeBinarySectionSize> sectionSizes) {
        ObjectDisplayName = string.IsNullOrWhiteSpace(objectDisplayName)
            ? throw new ArgumentException("Object display name must be provided.", nameof(objectDisplayName))
            : objectDisplayName;
        TotalSizeBytes = totalSizeBytes < 0
            ? throw new ArgumentOutOfRangeException(nameof(totalSizeBytes), totalSizeBytes, "Object contribution size must be zero or greater.")
            : totalSizeBytes;
        SectionSizes = sectionSizes ?? throw new ArgumentNullException(nameof(sectionSizes));
    }

    /// <summary>
    /// Gets the display name for the contributing object file or archive member.
    /// </summary>
    public string ObjectDisplayName { get; }

    /// <summary>
    /// Gets the total linked bytes attributed to the object contribution.
    /// </summary>
    public long TotalSizeBytes { get; }

    /// <summary>
    /// Gets the normalized linked section breakdown attributed to the object contribution.
    /// </summary>
    public IReadOnlyList<NativeBinarySectionSize> SectionSizes { get; }
}
