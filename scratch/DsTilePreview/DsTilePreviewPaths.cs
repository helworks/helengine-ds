namespace helengine.ds.scratch.tilepreview;

/// <summary>
/// Stores the cooked font and atlas file paths used by the preview tool for one load request.
/// </summary>
public sealed class DsTilePreviewPaths {
    /// <summary>
    /// Gets or sets the cooked font asset path.
    /// </summary>
    public string FontPath { get; set; }

    /// <summary>
    /// Gets or sets the cooked atlas texture asset path.
    /// </summary>
    public string AtlasPath { get; set; }
}
