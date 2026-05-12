namespace helengine.ds.builder;

/// <summary>
/// Stores the Nintendo DS-native startup manifest payload written into NitroFS.
/// </summary>
public sealed class NintendoDsStartupManifest {
    /// <summary>
    /// Initializes one startup manifest with already-packed Nintendo DS screen colors.
    /// </summary>
    /// <param name="topScreenColor">Packed top-screen color.</param>
    /// <param name="bottomScreenColor">Packed bottom-screen color.</param>
    public NintendoDsStartupManifest(ushort topScreenColor, ushort bottomScreenColor) {
        TopScreenColor = topScreenColor;
        BottomScreenColor = bottomScreenColor;
    }

    /// <summary>
    /// Gets the packed top-screen color.
    /// </summary>
    public ushort TopScreenColor { get; }

    /// <summary>
    /// Gets the packed bottom-screen color.
    /// </summary>
    public ushort BottomScreenColor { get; }
}
