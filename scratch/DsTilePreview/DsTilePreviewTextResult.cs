namespace helengine.ds.scratch.tilepreview;

/// <summary>
/// Stores one composed text preview built from consecutive packed 8x8 DS glyph tiles.
/// </summary>
public sealed class DsTilePreviewTextResult {
    /// <summary>
    /// Gets or sets the preview text that produced this composed tile view.
    /// </summary>
    public string Text { get; set; }

    /// <summary>
    /// Gets or sets the composed preview rows using <c>#</c> for visible pixels and <c>.</c> for empty pixels.
    /// </summary>
    public string[] Rows { get; set; }

    /// <summary>
    /// Gets or sets the full 32x24 font tile-map preview rows using <c>#</c> for visible pixels and <c>.</c> for empty pixels.
    /// </summary>
    public string[] FullFontMapRows { get; set; }

    /// <summary>
    /// Gets or sets the number of glyph tiles placed into the full font map preview.
    /// </summary>
    public int FullFontMapGlyphCount { get; set; }

    /// <summary>
    /// Gets or sets the number of tile columns used by the full font map preview.
    /// </summary>
    public int FullFontMapTileColumns { get; set; }
}
