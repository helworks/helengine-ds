using System.Drawing;

namespace helengine.ds.builder;

/// <summary>
/// Represents one temporary glyph definition produced during Nintendo DS source font atlas import.
/// </summary>
public readonly struct NintendoDsFontAtlasGlyph {
    /// <summary>
    /// Initializes one temporary glyph definition.
    /// </summary>
    /// <param name="sourceRect">Source rectangle for the glyph.</param>
    /// <param name="bitmap">Bitmap containing the glyph pixels.</param>
    public NintendoDsFontAtlasGlyph(int4 sourceRect, Bitmap bitmap) {
        SourceRect = sourceRect;
        Bitmap = bitmap ?? throw new ArgumentNullException(nameof(bitmap));
    }

    /// <summary>
    /// Gets the source rectangle for the glyph within its temporary bitmap.
    /// </summary>
    public int4 SourceRect { get; }

    /// <summary>
    /// Gets the bitmap containing the glyph pixels.
    /// </summary>
    public Bitmap Bitmap { get; }
}
