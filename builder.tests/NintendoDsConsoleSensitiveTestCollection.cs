namespace helengine.ds.builder.tests;

/// <summary>
/// Serializes tests that mutate process-wide console streams or current-directory state.
/// </summary>
[CollectionDefinition(CollectionName)]
public sealed class NintendoDsConsoleSensitiveTestCollection {
    /// <summary>
    /// Stores the shared collection name for console-sensitive Nintendo DS builder tests.
    /// </summary>
    public const string CollectionName = "NintendoDsConsoleSensitive";
}
