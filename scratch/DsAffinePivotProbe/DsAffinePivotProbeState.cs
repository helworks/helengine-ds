namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Holds the active scratch-probe options and animation parameters.
/// </summary>
public sealed class DsAffinePivotProbeState {
    /// <summary>
    /// Number of tile columns in the checker grid.
    /// </summary>
    public int GridColumns { get; set; } = 5;

    /// <summary>
    /// Number of tile rows in the checker grid.
    /// </summary>
    public int GridRows { get; set; } = 5;

    /// <summary>
    /// Width of one logical tile in pixels.
    /// </summary>
    public int TileWidth { get; set; } = 32;

    /// <summary>
    /// Height of one logical tile in pixels.
    /// </summary>
    public int TileHeight { get; set; } = 32;

    /// <summary>
    /// Parent pivot X coordinate in client-space pixels.
    /// </summary>
    public double ParentX { get; set; } = 220.35;

    /// <summary>
    /// Parent pivot Y coordinate in client-space pixels.
    /// </summary>
    public double ParentY { get; set; } = 180.65;

    /// <summary>
    /// Shared rotation angle in radians.
    /// </summary>
    public double RotationRadians { get; set; }

    /// <summary>
    /// Whether the orbit math uses the negative angle convention that currently best matches DS behavior.
    /// </summary>
    public bool UseNegativeOrbitAngle { get; set; } = true;

    /// <summary>
    /// Whether the parent center should be rounded before orbit math is applied.
    /// </summary>
    public bool RoundParentCenter { get; set; }

    /// <summary>
    /// Whether the tile anchor offset should use one expanded full-width or full-height rule.
    /// </summary>
    public bool UseExpandedAnchorOffset { get; set; }
}
