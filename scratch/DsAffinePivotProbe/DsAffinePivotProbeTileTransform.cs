namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Stores the draw placement and visual rotation for one logical checker tile.
/// </summary>
public sealed class DsAffinePivotProbeTileTransform {
    /// <summary>
    /// Creates one tile-transform result.
    /// </summary>
    /// <param name="tileColumn">Tile column index inside the logical checker grid.</param>
    /// <param name="tileRow">Tile row index inside the logical checker grid.</param>
    /// <param name="centerX">Final client-space tile center X coordinate.</param>
    /// <param name="centerY">Final client-space tile center Y coordinate.</param>
    /// <param name="drawLeft">Final client-space tile top-left X coordinate.</param>
    /// <param name="drawTop">Final client-space tile top-left Y coordinate.</param>
    /// <param name="rotationDegrees">Shared visual rotation in degrees.</param>
    public DsAffinePivotProbeTileTransform(int tileColumn, int tileRow, float centerX, float centerY, float drawLeft, float drawTop, float rotationDegrees) {
        TileColumn = tileColumn;
        TileRow = tileRow;
        CenterX = centerX;
        CenterY = centerY;
        DrawLeft = drawLeft;
        DrawTop = drawTop;
        RotationDegrees = rotationDegrees;
    }

    /// <summary>
    /// Tile column index inside the logical checker grid.
    /// </summary>
    public int TileColumn { get; }

    /// <summary>
    /// Tile row index inside the logical checker grid.
    /// </summary>
    public int TileRow { get; }

    /// <summary>
    /// Final client-space tile center X coordinate.
    /// </summary>
    public float CenterX { get; }

    /// <summary>
    /// Final client-space tile center Y coordinate.
    /// </summary>
    public float CenterY { get; }

    /// <summary>
    /// Final client-space tile top-left X coordinate.
    /// </summary>
    public float DrawLeft { get; }

    /// <summary>
    /// Final client-space tile top-left Y coordinate.
    /// </summary>
    public float DrawTop { get; }

    /// <summary>
    /// Shared visual rotation in degrees.
    /// </summary>
    public float RotationDegrees { get; }
}
