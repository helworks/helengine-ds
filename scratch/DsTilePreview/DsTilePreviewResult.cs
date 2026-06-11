namespace helengine.ds.scratch.tilepreview;

/// <summary>
/// Stores one previewed glyph together with the raw atlas rows and packed DS tile rows.
/// </summary>
public sealed class DsTilePreviewResult {
    /// <summary>
    /// Gets or sets the cooked font asset path that produced this preview.
    /// </summary>
    public string FontPath { get; set; }

    /// <summary>
    /// Gets or sets the cooked atlas texture asset path that produced this preview.
    /// </summary>
    public string AtlasPath { get; set; }

    /// <summary>
    /// Gets or sets the previewed character.
    /// </summary>
    public char Character { get; set; }

    /// <summary>
    /// Gets or sets the font atlas width in pixels.
    /// </summary>
    public int AtlasWidth { get; set; }

    /// <summary>
    /// Gets or sets the font atlas height in pixels.
    /// </summary>
    public int AtlasHeight { get; set; }

    /// <summary>
    /// Gets or sets the glyph source X position in the cooked atlas.
    /// </summary>
    public int SourceX { get; set; }

    /// <summary>
    /// Gets or sets the glyph source Y position in the cooked atlas.
    /// </summary>
    public int SourceY { get; set; }

    /// <summary>
    /// Gets or sets the glyph width in atlas pixels.
    /// </summary>
    public int SourceWidth { get; set; }

    /// <summary>
    /// Gets or sets the glyph height in atlas pixels.
    /// </summary>
    public int SourceHeight { get; set; }

    /// <summary>
    /// Gets or sets the glyph advance width from the cooked font metadata.
    /// </summary>
    public float AdvanceWidth { get; set; }

    /// <summary>
    /// Gets or sets the glyph left-side bearing from the cooked font metadata.
    /// </summary>
    public float BearingX { get; set; }

    /// <summary>
    /// Gets or sets the glyph top-side bearing from the cooked font metadata.
    /// </summary>
    public float BearingY { get; set; }

    /// <summary>
    /// Gets or sets the raw atlas rows for the selected glyph, using <c>#</c> for opaque pixels and <c>.</c> for empty pixels.
    /// </summary>
    public string[] AtlasRows { get; set; }

    /// <summary>
    /// Gets or sets the packed 8x8 DS tile rows for the selected glyph, using <c>#</c> for visible pixels and <c>.</c> for empty pixels.
    /// </summary>
    public string[] TileRows { get; set; }

    /// <summary>
    /// Gets or sets the packed DS tile bytes for the selected glyph.
    /// </summary>
    public byte[] TileBytes { get; set; }
}
