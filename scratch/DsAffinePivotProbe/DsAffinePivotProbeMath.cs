namespace helengine.ds.scratch.affinepivotprobe;

/// <summary>
/// Reproduces the current DS tiled-sprite orbit and anchor math so it can be debugged quickly on Windows.
/// </summary>
public sealed class DsAffinePivotProbeMath {
    /// <summary>
    /// Computes the final draw transforms for one logical checker grid.
    /// </summary>
    /// <param name="state">Current probe settings.</param>
    /// <returns>Ordered per-tile draw transforms.</returns>
    public IReadOnlyList<DsAffinePivotProbeTileTransform> ComputeTileTransforms(DsAffinePivotProbeState state) {
        if (state == null) {
            throw new ArgumentNullException(nameof(state));
        }

        if (state.GridColumns <= 0 || state.GridRows <= 0 || state.TileWidth <= 0 || state.TileHeight <= 0) {
            throw new InvalidOperationException("Grid and tile sizes must be positive.");
        }

        double parentX = state.RoundParentCenter ? Math.Round(state.ParentX) : state.ParentX;
        double parentY = state.RoundParentCenter ? Math.Round(state.ParentY) : state.ParentY;
        double totalWidth = state.GridColumns * state.TileWidth;
        double totalHeight = state.GridRows * state.TileHeight;
        double orbitAngle = state.UseNegativeOrbitAngle ? -state.RotationRadians : state.RotationRadians;
        double sine = Math.Sin(orbitAngle);
        double cosine = Math.Cos(orbitAngle);
        double rotationDegrees = state.RotationRadians * (180.0 / Math.PI);

        List<DsAffinePivotProbeTileTransform> transforms = new List<DsAffinePivotProbeTileTransform>(state.GridColumns * state.GridRows);
        for (int row = 0; row < state.GridRows; row++) {
            for (int column = 0; column < state.GridColumns; column++) {
                double localCenterX = ((column * state.TileWidth) + (state.TileWidth * 0.5)) - (totalWidth * 0.5);
                double localCenterY = ((row * state.TileHeight) + (state.TileHeight * 0.5)) - (totalHeight * 0.5);
                double rotatedCenterX = (localCenterX * cosine) - (localCenterY * sine);
                double rotatedCenterY = (localCenterX * sine) + (localCenterY * cosine);
                double centerX = parentX + rotatedCenterX;
                double centerY = parentY + rotatedCenterY;
                double anchorOffsetX = state.UseExpandedAnchorOffset ? state.TileWidth : (state.TileWidth * 0.5);
                double anchorOffsetY = state.UseExpandedAnchorOffset ? state.TileHeight : (state.TileHeight * 0.5);
                double drawLeft = Math.Round(centerX - anchorOffsetX);
                double drawTop = Math.Round(centerY - anchorOffsetY);

                transforms.Add(new DsAffinePivotProbeTileTransform(
                    column,
                    row,
                    (float)centerX,
                    (float)centerY,
                    (float)drawLeft,
                    (float)drawTop,
                    (float)rotationDegrees));
            }
        }

        return transforms;
    }
}
