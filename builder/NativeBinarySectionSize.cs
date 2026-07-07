namespace helengine.ds.builder;

/// <summary>
/// Represents one normalized linked-section size contribution inside one native binary report.
/// </summary>
public sealed class NativeBinarySectionSize {
    /// <summary>
    /// Initializes one linked-section size record with explicit section name and byte count.
    /// </summary>
    /// <param name="sectionName">Normalized linked section name such as <c>.text</c> or <c>.rodata</c>.</param>
    /// <param name="sizeBytes">Total linked bytes attributed to the section.</param>
    public NativeBinarySectionSize(string sectionName, long sizeBytes) {
        SectionName = string.IsNullOrWhiteSpace(sectionName)
            ? throw new ArgumentException("Section name must be provided.", nameof(sectionName))
            : sectionName;
        SizeBytes = sizeBytes < 0
            ? throw new ArgumentOutOfRangeException(nameof(sizeBytes), sizeBytes, "Section size must be zero or greater.")
            : sizeBytes;
    }

    /// <summary>
    /// Gets the normalized linked section name.
    /// </summary>
    public string SectionName { get; }

    /// <summary>
    /// Gets the total linked bytes attributed to the section.
    /// </summary>
    public long SizeBytes { get; }
}
