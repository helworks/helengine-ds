namespace helengine.ds.builder;

/// <summary>
/// Stores the schema and field identifiers used by Nintendo DS fixed-pipeline material cooking.
/// </summary>
public static class NintendoDsMaterialSchemaIds {
    /// <summary>
    /// Standard textured DS schema id.
    /// </summary>
    public const string StandardTexturedSchemaId = "ds-standard-textured";

    /// <summary>
    /// Texture-relative-path field id.
    /// </summary>
    public const string TextureRelativePathFieldId = "texture-relative-path";

    /// <summary>
    /// Double-sided field id.
    /// </summary>
    public const string DoubleSidedFieldId = "double-sided";

    /// <summary>
    /// Vertex-color-mode field id.
    /// </summary>
    public const string VertexColorModeFieldId = "vertex-color-mode";

    /// <summary>
    /// Base-color field id.
    /// </summary>
    public const string BaseColorFieldId = "base-color";

    /// <summary>
    /// Lighting-mode field id.
    /// </summary>
    public const string LightingModeFieldId = "lighting-mode";
}
