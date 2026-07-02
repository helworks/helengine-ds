using helengine.ds.scratch.affinepivotprobe;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the scratch affine-pivot probe keeps tiled checker placement rigid around one shared parent pivot.
/// </summary>
public sealed class DsAffinePivotProbeMathTests {
    /// <summary>
    /// Verifies zero rotation keeps the tiles in their authored grid positions.
    /// </summary>
    [Fact]
    public void ComputeTileTransforms_whenAngleIsZero_keepsTilesInAuthoredGrid() {
        DsAffinePivotProbeState state = new DsAffinePivotProbeState {
            GridColumns = 2,
            GridRows = 2,
            TileWidth = 32,
            TileHeight = 32,
            ParentX = 100.0,
            ParentY = 100.0,
            RotationRadians = 0.0
        };

        DsAffinePivotProbeMath math = new DsAffinePivotProbeMath();
        IReadOnlyList<DsAffinePivotProbeTileTransform> transforms = math.ComputeTileTransforms(state);

        Assert.Equal(4, transforms.Count);
        Assert.Equal(68.0f, transforms[0].DrawLeft, 3);
        Assert.Equal(68.0f, transforms[0].DrawTop, 3);
        Assert.Equal(116.0f, transforms[3].CenterX, 3);
        Assert.Equal(116.0f, transforms[3].CenterY, 3);
    }

    /// <summary>
    /// Verifies quarter-turn placement preserves each tile center radius from the shared pivot.
    /// </summary>
    [Fact]
    public void ComputeTileTransforms_whenQuarterTurn_preservesTileCenterDistanceFromPivot() {
        DsAffinePivotProbeState state = new DsAffinePivotProbeState {
            GridColumns = 2,
            GridRows = 2,
            TileWidth = 32,
            TileHeight = 32,
            ParentX = 100.0,
            ParentY = 100.0,
            RotationRadians = Math.PI / 2.0,
            UseNegativeOrbitAngle = true
        };

        DsAffinePivotProbeMath math = new DsAffinePivotProbeMath();
        IReadOnlyList<DsAffinePivotProbeTileTransform> transforms = math.ComputeTileTransforms(state);

        for (int index = 0; index < transforms.Count; index++) {
            double deltaX = transforms[index].CenterX - state.ParentX;
            double deltaY = transforms[index].CenterY - state.ParentY;
            double radius = Math.Sqrt((deltaX * deltaX) + (deltaY * deltaY));
            Assert.Equal(22.627416997969522, radius, 6);
        }
    }

    /// <summary>
    /// Verifies changing the anchor rule changes the final top-left placement without changing the logical tile centers.
    /// </summary>
    [Fact]
    public void ComputeTileTransforms_whenAnchorRuleChanges_keepsCenterButMovesTopLeft() {
        DsAffinePivotProbeState state = new DsAffinePivotProbeState {
            GridColumns = 2,
            GridRows = 2,
            TileWidth = 32,
            TileHeight = 32,
            ParentX = 100.0,
            ParentY = 100.0,
            RotationRadians = Math.PI / 6.0
        };

        DsAffinePivotProbeMath math = new DsAffinePivotProbeMath();
        IReadOnlyList<DsAffinePivotProbeTileTransform> centeredAnchor = math.ComputeTileTransforms(state);

        state.UseExpandedAnchorOffset = true;
        IReadOnlyList<DsAffinePivotProbeTileTransform> expandedAnchor = math.ComputeTileTransforms(state);

        Assert.Equal(centeredAnchor[0].CenterX, expandedAnchor[0].CenterX, 6);
        Assert.Equal(centeredAnchor[0].CenterY, expandedAnchor[0].CenterY, 6);
        Assert.NotEqual(centeredAnchor[0].DrawLeft, expandedAnchor[0].DrawLeft);
        Assert.NotEqual(centeredAnchor[0].DrawTop, expandedAnchor[0].DrawTop);
    }
}
