using helengine.editor;

namespace helengine.ds.scratch.tilepreview;

/// <summary>
/// Loads one cooked DS font and atlas pair, then reproduces the runtime 8x8 tile packing logic for one glyph.
/// </summary>
public sealed class DsTilePreviewLoader {
    /// <summary>
    /// Reads one cooked DS font and atlas pair from disk and returns the packed tile preview for one character.
    /// </summary>
    /// <param name="inputPath">Workspace root or direct cooked font path.</param>
    /// <param name="character">Character to preview.</param>
    /// <returns>Preview data containing raw atlas rows and packed DS tile rows.</returns>
    public DsTilePreviewResult Load(string inputPath, char character) {
        if (string.IsNullOrWhiteSpace(inputPath)) {
            throw new ArgumentException("A build workspace or cooked font path must be provided.", nameof(inputPath));
        }

        DsTilePreviewPaths paths = ResolvePaths(inputPath);
        FontAsset fontAsset = ReadFontAsset(paths.FontPath);
        TextureAsset textureAsset = ResolveTextureAsset(fontAsset, paths.AtlasPath);
        return BuildGlyphPreview(paths.FontPath, paths.AtlasPath, fontAsset, textureAsset, character);
    }

    /// <summary>
    /// Builds one preview from a directly imported GDI font without going through the cooked DS asset pipeline.
    /// </summary>
    /// <param name="fontFamilyName">Installed font family to import.</param>
    /// <param name="pixelSize">Direct GDI import size in pixels.</param>
    /// <param name="character">Character to preview.</param>
    /// <returns>Preview data containing raw atlas rows and packed DS tile rows.</returns>
    public DsTilePreviewResult LoadDirectGdi(string fontFamilyName, float pixelSize, char character) {
        if (string.IsNullOrWhiteSpace(fontFamilyName)) {
            throw new ArgumentException("A font family name must be provided.", nameof(fontFamilyName));
        } else if (pixelSize <= 0.0f) {
            throw new ArgumentOutOfRangeException(nameof(pixelSize), "The direct GDI pixel size must be positive.");
        }

        using Font font = new Font(fontFamilyName, pixelSize, FontStyle.Regular, GraphicsUnit.Pixel);
        FontAsset fontAsset = GDIFontProcessor.ImportFont(font);
        TextureAsset textureAsset = fontAsset.SourceTextureAsset;
        if (textureAsset == null) {
            throw new InvalidOperationException("The direct GDI font import did not produce a source atlas texture.");
        }

        string sourceLabel = $"{fontFamilyName} {pixelSize:0.###}px (direct GDI)";
        return BuildGlyphPreview(sourceLabel, "<direct-gdi>", fontAsset, textureAsset, character);
    }

    /// <summary>
    /// Builds one packed-glyph preview from already loaded font and atlas assets.
    /// </summary>
    /// <param name="fontPathLabel">Label describing the font source.</param>
    /// <param name="atlasPathLabel">Label describing the atlas source.</param>
    /// <param name="fontAsset">Font asset providing glyph metadata.</param>
    /// <param name="textureAsset">Texture asset providing atlas pixel data.</param>
    /// <param name="character">Character to preview.</param>
    /// <returns>Preview data containing raw atlas rows and packed DS tile rows.</returns>
    DsTilePreviewResult BuildGlyphPreview(string fontPathLabel, string atlasPathLabel, FontAsset fontAsset, TextureAsset textureAsset, char character) {
        if (fontAsset.Characters == null || !fontAsset.Characters.TryGetValue(character, out FontChar glyph)) {
            throw new InvalidOperationException($"Cooked font does not contain glyph '{character}'.");
        }

        int sourceX = (int)Math.Round(glyph.SourceRect.X * fontAsset.AtlasWidth);
        int sourceY = (int)Math.Round(glyph.SourceRect.Y * fontAsset.AtlasHeight);
        int sourceWidth = (int)Math.Round(glyph.SourceRect.Z * fontAsset.AtlasWidth);
        int sourceHeight = (int)Math.Round(glyph.SourceRect.W * fontAsset.AtlasHeight);
        if (sourceWidth < 1 || sourceHeight < 1) {
            throw new InvalidOperationException($"Cooked glyph '{character}' resolved to an empty atlas rectangle.");
        }

        DsTilePreviewResult result = new DsTilePreviewResult();
        result.FontPath = fontPathLabel;
        result.AtlasPath = atlasPathLabel;
        result.Character = character;
        result.AtlasWidth = fontAsset.AtlasWidth;
        result.AtlasHeight = fontAsset.AtlasHeight;
        result.SourceX = sourceX;
        result.SourceY = sourceY;
        result.SourceWidth = sourceWidth;
        result.SourceHeight = sourceHeight;
        result.AdvanceWidth = glyph.AdvanceWidth;
        result.BearingX = glyph.BearingX;
        result.BearingY = glyph.BearingY;
        result.AtlasRows = BuildAtlasRows(textureAsset, sourceX, sourceY, sourceWidth, sourceHeight);
        result.TileBytes = BuildPackedTileBytes(textureAsset, sourceX, sourceY, sourceWidth, sourceHeight);
        result.TileRows = BuildTileRows(result.TileBytes);
        return result;
    }

    /// <summary>
    /// Builds one single-line text preview by packing each glyph into one DS 8x8 tile and composing the tiles side by side.
    /// </summary>
    /// <param name="inputPath">Workspace root or direct cooked font path.</param>
    /// <param name="text">Text to preview through packed DS 8x8 tiles.</param>
    /// <returns>Composed preview rows for the requested text.</returns>
    public DsTilePreviewTextResult LoadTextPreview(string inputPath, string text) {
        if (string.IsNullOrWhiteSpace(inputPath)) {
            throw new ArgumentException("A build workspace or cooked font path must be provided.", nameof(inputPath));
        } else if (text == null) {
            throw new ArgumentNullException(nameof(text));
        }

        DsTilePreviewPaths paths = ResolvePaths(inputPath);
        FontAsset fontAsset = ReadFontAsset(paths.FontPath);
        TextureAsset textureAsset = ResolveTextureAsset(fontAsset, paths.AtlasPath);
        return BuildTextPreview(text, fontAsset, textureAsset);
    }

    /// <summary>
    /// Builds one composed text preview from a directly imported GDI font without going through the cooked DS asset pipeline.
    /// </summary>
    /// <param name="fontFamilyName">Installed font family to import.</param>
    /// <param name="pixelSize">Direct GDI import size in pixels.</param>
    /// <param name="text">Text to preview through packed DS 8x8 tiles.</param>
    /// <returns>Composed preview rows and full font-map rows for the requested text.</returns>
    public DsTilePreviewTextResult LoadDirectGdiTextPreview(string fontFamilyName, float pixelSize, string text) {
        if (string.IsNullOrWhiteSpace(fontFamilyName)) {
            throw new ArgumentException("A font family name must be provided.", nameof(fontFamilyName));
        } else if (pixelSize <= 0.0f) {
            throw new ArgumentOutOfRangeException(nameof(pixelSize), "The direct GDI pixel size must be positive.");
        } else if (text == null) {
            throw new ArgumentNullException(nameof(text));
        }

        using Font font = new Font(fontFamilyName, pixelSize, FontStyle.Regular, GraphicsUnit.Pixel);
        FontAsset fontAsset = GDIFontProcessor.ImportFont(font);
        TextureAsset textureAsset = fontAsset.SourceTextureAsset;
        if (textureAsset == null) {
            throw new InvalidOperationException("The direct GDI font import did not produce a source atlas texture.");
        }

        return BuildTextPreview(text, fontAsset, textureAsset);
    }

    /// <summary>
    /// Builds one composed text preview and one full font-map preview from already loaded font and atlas assets.
    /// </summary>
    /// <param name="text">Text to preview through packed DS 8x8 tiles.</param>
    /// <param name="fontAsset">Font asset providing glyph metadata.</param>
    /// <param name="textureAsset">Texture asset providing atlas pixel data.</param>
    /// <returns>Composed preview rows and full font-map rows for the requested text.</returns>
    DsTilePreviewTextResult BuildTextPreview(string text, FontAsset fontAsset, TextureAsset textureAsset) {
        int characterCount = text.Length;
        if (characterCount <= 0) {
            return new DsTilePreviewTextResult {
                Text = string.Empty,
                Rows = BuildTileRows(new byte[32])
            };
        }

        char[][] composedRows = new char[8][];
        for (int rowIndex = 0; rowIndex < composedRows.Length; rowIndex++) {
            composedRows[rowIndex] = new char[characterCount * 8];
            Array.Fill(composedRows[rowIndex], '.');
        }

        for (int characterIndex = 0; characterIndex < characterCount; characterIndex++) {
            char character = text[characterIndex];
            byte[] tileBytes = BuildCharacterTileBytes(fontAsset, textureAsset, character);
            string[] tileRows = BuildTileRows(tileBytes);
            for (int rowIndex = 0; rowIndex < 8; rowIndex++) {
                for (int columnIndex = 0; columnIndex < 8; columnIndex++) {
                    composedRows[rowIndex][(characterIndex * 8) + columnIndex] = tileRows[rowIndex][columnIndex];
                }
            }
        }

        string[] rows = new string[8];
        for (int rowIndex = 0; rowIndex < rows.Length; rowIndex++) {
            rows[rowIndex] = new string(composedRows[rowIndex]);
        }

        const int fontMapTileColumns = 32;
        const int fontMapTileRows = 24;
        char[][] fullFontMapRows = new char[fontMapTileRows * 8][];
        for (int rowIndex = 0; rowIndex < fullFontMapRows.Length; rowIndex++) {
            fullFontMapRows[rowIndex] = new char[fontMapTileColumns * 8];
            Array.Fill(fullFontMapRows[rowIndex], '.');
        }

        List<char> fontCharacters = BuildOrderedFontCharacterList(fontAsset);
        int fontMapGlyphCount = Math.Min(fontCharacters.Count, fontMapTileColumns * fontMapTileRows);
        for (int glyphIndex = 0; glyphIndex < fontMapGlyphCount; glyphIndex++) {
            byte[] tileBytes = BuildCharacterTileBytes(fontAsset, textureAsset, fontCharacters[glyphIndex]);
            string[] tileRows = BuildTileRows(tileBytes);
            int tileRow = glyphIndex / fontMapTileColumns;
            int tileColumn = glyphIndex % fontMapTileColumns;

            for (int tileRowPixel = 0; tileRowPixel < 8; tileRowPixel++) {
                for (int tileColumnPixel = 0; tileColumnPixel < 8; tileColumnPixel++) {
                    int destinationRowIndex = (tileRow * 8) + tileRowPixel;
                    int destinationColumnIndex = (tileColumn * 8) + tileColumnPixel;
                    fullFontMapRows[destinationRowIndex][destinationColumnIndex] = tileRows[tileRowPixel][tileColumnPixel];
                }
            }
        }

        string[] fullFontMapRowStrings = new string[fullFontMapRows.Length];
        for (int rowIndex = 0; rowIndex < fullFontMapRowStrings.Length; rowIndex++) {
            fullFontMapRowStrings[rowIndex] = new string(fullFontMapRows[rowIndex]);
        }

        return new DsTilePreviewTextResult {
            Text = text,
            Rows = rows,
            FullFontMapRows = fullFontMapRowStrings,
            FullFontMapGlyphCount = fontMapGlyphCount,
            FullFontMapTileColumns = fontMapTileColumns
        };
    }

    /// <summary>
    /// Builds the ordered character list that should populate the full font map preview.
    /// </summary>
    /// <param name="fontAsset">Cooked font asset whose glyph inventory should be previewed.</param>
    /// <returns>Ordered character list for the full font map preview.</returns>
    List<char> BuildOrderedFontCharacterList(FontAsset fontAsset) {
        if (fontAsset.Characters == null) {
            return new List<char>();
        }

        List<char> orderedCharacters = new List<char>(fontAsset.Characters.Keys);
        orderedCharacters.Sort(static (left, right) => left.CompareTo(right));
        return orderedCharacters;
    }

    /// <summary>
    /// Resolves the cooked font and atlas file paths from either one workspace root or one direct font path.
    /// </summary>
    /// <param name="inputPath">Workspace root or direct cooked font path.</param>
    /// <returns>Resolved font and atlas file paths.</returns>
    DsTilePreviewPaths ResolvePaths(string inputPath) {
        string fullInputPath = Path.GetFullPath(inputPath);
        DsTilePreviewPaths paths = new DsTilePreviewPaths();
        if (string.Equals(Path.GetExtension(fullInputPath), ".hefont", StringComparison.OrdinalIgnoreCase)) {
            paths.FontPath = fullInputPath;
            paths.AtlasPath = Path.ChangeExtension(fullInputPath, ".dsfonttex");
            return paths;
        }

        paths.FontPath = Path.Combine(fullInputPath, "package", "cooked", "fonts", "ds-debug.hefont");
        paths.AtlasPath = Path.Combine(fullInputPath, "builder", "ds", "nitrofs", "cooked", "fonts", "ds-debug.dsfonttex");
        return paths;
    }

    /// <summary>
    /// Reads one cooked font asset from disk.
    /// </summary>
    /// <param name="fontPath">Absolute cooked font path.</param>
    /// <returns>Deserialized font asset.</returns>
    FontAsset ReadFontAsset(string fontPath) {
        if (!File.Exists(fontPath)) {
            throw new FileNotFoundException("Cooked font asset was not found.", fontPath);
        }

        using MemoryStream stream = new MemoryStream(File.ReadAllBytes(fontPath), false);
        return helengine.files.FontAssetBinarySerializer.Deserialize(stream);
    }

    /// <summary>
    /// Resolves the cooked atlas texture from disk or falls back to the embedded source texture when present.
    /// </summary>
    /// <param name="fontAsset">Cooked font asset whose atlas should be previewed.</param>
    /// <param name="atlasPath">Explicit cooked atlas path.</param>
    /// <returns>Resolved texture asset backing the previewed glyph.</returns>
    TextureAsset ResolveTextureAsset(FontAsset fontAsset, string atlasPath) {
        if (File.Exists(atlasPath)) {
            object asset = helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(atlasPath));
            if (asset is not TextureAsset textureAsset) {
                throw new InvalidOperationException("Cooked DS atlas did not deserialize as TextureAsset.");
            }

            return textureAsset;
        } else if (fontAsset.SourceTextureAsset != null) {
            return fontAsset.SourceTextureAsset;
        } else {
            throw new FileNotFoundException("Cooked atlas texture was not found.", atlasPath);
        }
    }

    /// <summary>
    /// Builds one packed 8x8 DS tile payload for the requested character.
    /// </summary>
    /// <param name="fontAsset">Cooked font asset.</param>
    /// <param name="textureAsset">Cooked atlas texture asset.</param>
    /// <param name="character">Character to pack into one DS tile.</param>
    /// <returns>Packed 32-byte DS tile payload for the selected character.</returns>
    byte[] BuildCharacterTileBytes(FontAsset fontAsset, TextureAsset textureAsset, char character) {
        if (character == ' ') {
            return new byte[32];
        }
        if (fontAsset.Characters == null || !fontAsset.Characters.TryGetValue(character, out FontChar glyph)) {
            throw new InvalidOperationException($"Cooked font does not contain glyph '{character}'.");
        }

        int sourceX = (int)Math.Round(glyph.SourceRect.X * fontAsset.AtlasWidth);
        int sourceY = (int)Math.Round(glyph.SourceRect.Y * fontAsset.AtlasHeight);
        int sourceWidth = (int)Math.Round(glyph.SourceRect.Z * fontAsset.AtlasWidth);
        int sourceHeight = (int)Math.Round(glyph.SourceRect.W * fontAsset.AtlasHeight);
        return BuildPackedTileBytes(textureAsset, sourceX, sourceY, sourceWidth, sourceHeight);
    }

    /// <summary>
    /// Builds one printable atlas-row preview for the selected glyph.
    /// </summary>
    /// <param name="textureAsset">Cooked atlas texture.</param>
    /// <param name="sourceX">Glyph left edge in atlas pixels.</param>
    /// <param name="sourceY">Glyph top edge in atlas pixels.</param>
    /// <param name="sourceWidth">Glyph width in atlas pixels.</param>
    /// <param name="sourceHeight">Glyph height in atlas pixels.</param>
    /// <returns>Atlas rows using <c>#</c> for opaque pixels and <c>.</c> for empty pixels.</returns>
    string[] BuildAtlasRows(TextureAsset textureAsset, int sourceX, int sourceY, int sourceWidth, int sourceHeight) {
        string[] rows = new string[sourceHeight];
        for (int y = 0; y < sourceHeight; y++) {
            char[] row = new char[sourceWidth];
            for (int x = 0; x < sourceWidth; x++) {
                int sourcePixelIndex = ((sourceY + y) * textureAsset.Width) + (sourceX + x);
                int alpha;
                if (textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4) {
                    int paletteIndex = (textureAsset.Colors[sourcePixelIndex / 2] & 0x0F);
                    if ((sourcePixelIndex & 1) != 0) {
                        paletteIndex = (textureAsset.Colors[sourcePixelIndex / 2] >> 4) & 0x0F;
                    }

                    int paletteOffset = paletteIndex * 4;
                    alpha = textureAsset.PaletteColors[paletteOffset + 3];
                } else {
                    alpha = textureAsset.Colors[(sourcePixelIndex * 4) + 3];
                }

                row[x] = alpha > 0 ? '#' : '.';
            }

            rows[y] = new string(row);
        }

        return rows;
    }

    /// <summary>
    /// Reproduces the Nintendo DS runtime glyph-packing logic and builds one packed 8x8 tile payload.
    /// </summary>
    /// <param name="textureAsset">Cooked atlas texture.</param>
    /// <param name="sourceX">Glyph left edge in atlas pixels.</param>
    /// <param name="sourceY">Glyph top edge in atlas pixels.</param>
    /// <param name="sourceWidth">Glyph width in atlas pixels.</param>
    /// <param name="sourceHeight">Glyph height in atlas pixels.</param>
    /// <returns>Packed 32-byte DS tile payload.</returns>
    byte[] BuildPackedTileBytes(TextureAsset textureAsset, int sourceX, int sourceY, int sourceWidth, int sourceHeight) {
        byte[] tileBytes = new byte[32];
        int availablePaletteEntries = Math.Min(textureAsset.PaletteColors == null ? 0 : textureAsset.PaletteColors.Length / 4, 16);
        byte[] paletteIndexRemap = new byte[16];
        for (int paletteIndex = 0; paletteIndex < availablePaletteEntries; paletteIndex++) {
            int paletteOffset = paletteIndex * 4;
            byte alpha = textureAsset.PaletteColors[paletteOffset + 3];
            paletteIndexRemap[paletteIndex] = alpha > 0 ? (byte)1 : (byte)0;
        }

        int tileCopyWidth = Math.Min(sourceWidth, 7);
        int tileCopyHeight = Math.Min(sourceHeight, 7);
        int sourceStartX = 0;
        int sourceStartY = Math.Max(sourceHeight - tileCopyHeight, 0);
        int destinationOffsetX = Math.Max(7 - tileCopyWidth, 0);
        int destinationOffsetY = Math.Max(7 - tileCopyHeight, 0);
        for (int y = 0; y < tileCopyHeight; y++) {
            for (int x = 0; x < tileCopyWidth; x++) {
                int sourcePixelIndex = ((sourceY + sourceStartY + y) * textureAsset.Width) + (sourceX + sourceStartX + x);
                byte paletteIndex;
                if (textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4) {
                    paletteIndex = (byte)(textureAsset.Colors[sourcePixelIndex / 2] & 0x0F);
                    if ((sourcePixelIndex & 1) != 0) {
                        paletteIndex = (byte)((textureAsset.Colors[sourcePixelIndex / 2] >> 4) & 0x0F);
                    }

                    paletteIndex = paletteIndex >= availablePaletteEntries
                        ? (byte)0
                        : paletteIndexRemap[paletteIndex];
                } else {
                    byte alpha = textureAsset.Colors[(sourcePixelIndex * 4) + 3];
                    paletteIndex = alpha > 0 ? (byte)1 : (byte)0;
                }

                int destinationX = destinationOffsetX + x;
                int destinationY = destinationOffsetY + y;
                int tileByteIndex = (destinationY * 4) + (destinationX / 2);
                if ((destinationX & 1) == 0) {
                    tileBytes[tileByteIndex] = (byte)((tileBytes[tileByteIndex] & 0xF0) | (paletteIndex & 0x0F));
                } else {
                    tileBytes[tileByteIndex] = (byte)((tileBytes[tileByteIndex] & 0x0F) | ((paletteIndex & 0x0F) << 4));
                }
            }
        }

        return tileBytes;
    }

    /// <summary>
    /// Builds one printable 8x8 row preview from one packed DS tile payload.
    /// </summary>
    /// <param name="tileBytes">Packed DS tile payload.</param>
    /// <returns>Tile rows using <c>#</c> for visible pixels and <c>.</c> for empty pixels.</returns>
    string[] BuildTileRows(byte[] tileBytes) {
        string[] rows = new string[8];
        for (int y = 0; y < 8; y++) {
            char[] row = new char[8];
            for (int x = 0; x < 8; x++) {
                int tileByteIndex = (y * 4) + (x / 2);
                int paletteIndex = (x & 1) == 0
                    ? tileBytes[tileByteIndex] & 0x0F
                    : (tileBytes[tileByteIndex] >> 4) & 0x0F;
                row[x] = paletteIndex == 0 ? '.' : '#';
            }

            rows[y] = new string(row);
        }

        return rows;
    }
}
