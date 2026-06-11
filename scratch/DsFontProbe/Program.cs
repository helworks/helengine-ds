namespace helengine.ds.scratch;

/// <summary>
/// Reads one cooked DS font and atlas texture so DS text failures can be debugged without relying on emulator visuals.
/// </summary>
public static class Program {
    /// <summary>
    /// Entry point that prints the cooked font and atlas texture diagnostics for the supplied DS build workspace.
    /// </summary>
    /// <param name="args">Command-line arguments where the first argument is the DS build workspace root.</param>
    public static void Main(string[] args) {
        if (args.Length < 1 || string.IsNullOrWhiteSpace(args[0])) {
            throw new ArgumentException("Expected the DS build workspace root path as the first argument.", nameof(args));
        }

        string cookedFontPath;
        string cookedAtlasPath;
        if (string.Equals(Path.GetExtension(args[0]), ".hefont", StringComparison.OrdinalIgnoreCase)) {
            cookedFontPath = Path.GetFullPath(args[0]);
            cookedAtlasPath = args.Length >= 2 && !string.IsNullOrWhiteSpace(args[1])
                ? Path.GetFullPath(args[1])
                : string.Empty;
        } else {
            string buildRootPath = Path.GetFullPath(args[0]);
            cookedFontPath = Path.Combine(buildRootPath, "package", "cooked", "fonts", "ds-debug.hefont");
            cookedAtlasPath = Path.Combine(buildRootPath, "builder", "ds", "nitrofs", "cooked", "fonts", "ds-debug.dsfonttex");
        }
        FontAsset fontAsset = ReadFontAsset(cookedFontPath);
        TextureAsset textureAsset = ResolveTextureAsset(fontAsset, cookedAtlasPath);

        Console.WriteLine("FontPath={0}", cookedFontPath);
        Console.WriteLine("FontInfo={0}", fontAsset.FontInfo == null ? "<null>" : fontAsset.FontInfo.Name);
        Console.WriteLine("LineHeight={0}", fontAsset.LineHeight);
        Console.WriteLine("Atlas={0}x{1}", fontAsset.AtlasWidth, fontAsset.AtlasHeight);
        Console.WriteLine("CookedAtlasReference={0}", fontAsset.CookedAtlasTextureRelativePath ?? "<null>");
        Console.WriteLine("CharacterCount={0}", fontAsset.Characters == null ? 0 : fontAsset.Characters.Count);

        Console.WriteLine("TexturePath={0}", string.IsNullOrWhiteSpace(cookedAtlasPath) ? "<embedded>" : cookedAtlasPath);
        Console.WriteLine("TextureFormat={0}", textureAsset.ColorFormat);
        Console.WriteLine("TextureAlpha={0}", textureAsset.AlphaPrecision);
        Console.WriteLine("TextureSize={0}x{1}", textureAsset.Width, textureAsset.Height);
        Console.WriteLine("TextureColors={0}", textureAsset.Colors == null ? 0 : textureAsset.Colors.Length);
        Console.WriteLine("TexturePaletteColors={0}", textureAsset.PaletteColors == null ? 0 : textureAsset.PaletteColors.Length);

        int maxGlyphWidth = 0;
        int maxGlyphHeight = 0;
        if (fontAsset.Characters != null) {
            foreach (KeyValuePair<char, FontChar> entry in fontAsset.Characters) {
                int glyphWidth = (int)Math.Round(entry.Value.SourceRect.Z * fontAsset.AtlasWidth);
                int glyphHeight = (int)Math.Round(entry.Value.SourceRect.W * fontAsset.AtlasHeight);
                if (glyphWidth > maxGlyphWidth) {
                    maxGlyphWidth = glyphWidth;
                }
                if (glyphHeight > maxGlyphHeight) {
                    maxGlyphHeight = glyphHeight;
                }
            }
        }

        Console.WriteLine("MaxGlyph={0}x{1}", maxGlyphWidth, maxGlyphHeight);
        PrintGlyph(fontAsset, 'B');
        PrintGlyph(fontAsset, 'A');
        PrintGlyph(fontAsset, 'C');
        PrintGlyph(fontAsset, 'K');
        PrintGlyph(fontAsset, 'P');
        PrintGlyph(fontAsset, '1');
        PrintGlyph(fontAsset, '2');
        PrintGlyph(fontAsset, 'R');
        PrintGlyph(fontAsset, 'o');
        PrintGlyph(fontAsset, 't');
        PrintGlyph(fontAsset, 'a');
        PrintGlyph(fontAsset, 'e');
        PrintGlyph(fontAsset, 'U');
        PrintGlyph(fontAsset, 'p');
        PrintGlyph(fontAsset, 'd');
        PrintGlyph(fontAsset, 'F');
        PrintGlyph(fontAsset, 'S');
        PrintGlyph(fontAsset, ':');
        PrintGlyph(fontAsset, '-');
        PrintGlyph(fontAsset, '(');
        PrintGlyph(fontAsset, ')');
        PrintGlyph(fontAsset, 'm');
        PrintGlyph(fontAsset, 's');
        PrintGlyph(fontAsset, 'H');
        PrintGlyph(textureAsset, fontAsset, 'H');
        DumpGlyphPixels(textureAsset, fontAsset, 'H');
        DumpRuntimeTile(textureAsset, fontAsset, 'H');
        PrintGlyph(fontAsset, 'T');
        PrintGlyph(fontAsset, 'g');
        PrintGlyph(fontAsset, 'l');
    }

    /// <summary>
    /// Reads one cooked font asset from disk.
    /// </summary>
    /// <param name="fontPath">Absolute path to the cooked font asset.</param>
    /// <returns>Deserialized font asset.</returns>
    static FontAsset ReadFontAsset(string fontPath) {
        if (!File.Exists(fontPath)) {
            throw new FileNotFoundException("Cooked font asset was not found.", fontPath);
        }

        using MemoryStream stream = new MemoryStream(File.ReadAllBytes(fontPath), false);
        return helengine.files.FontAssetBinarySerializer.Deserialize(stream);
    }

    /// <summary>
    /// Reads one cooked texture asset from disk.
    /// </summary>
    /// <param name="texturePath">Absolute path to the cooked texture asset.</param>
    /// <returns>Deserialized texture asset.</returns>
    static TextureAsset ReadTextureAsset(string texturePath) {
        if (!File.Exists(texturePath)) {
            throw new FileNotFoundException("Cooked texture asset was not found.", texturePath);
        }

        object asset = helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(texturePath));
        if (asset is not TextureAsset textureAsset) {
            throw new InvalidOperationException("Cooked DS debug atlas did not deserialize as TextureAsset.");
        }

        return textureAsset;
    }

    /// <summary>
    /// Resolves the texture asset used by the font, preferring the explicit cooked texture path when provided.
    /// </summary>
    /// <param name="fontAsset">Font asset whose texture should be inspected.</param>
    /// <param name="texturePath">Optional explicit cooked texture path.</param>
    /// <returns>Texture asset backing the font glyph atlas.</returns>
    static TextureAsset ResolveTextureAsset(FontAsset fontAsset, string texturePath) {
        if (!string.IsNullOrWhiteSpace(texturePath)) {
            return ReadTextureAsset(texturePath);
        }

        if (fontAsset?.SourceTextureAsset == null) {
            throw new InvalidOperationException("Font probe requires either an explicit cooked texture path or an embedded SourceTextureAsset.");
        }

        return fontAsset.SourceTextureAsset;
    }

    /// <summary>
    /// Prints one glyph's atlas dimensions.
    /// </summary>
    /// <param name="fontAsset">Font asset being inspected.</param>
    /// <param name="character">Character whose glyph should be reported.</param>
    static void PrintGlyph(FontAsset fontAsset, char character) {
        if (fontAsset == null) {
            throw new ArgumentNullException(nameof(fontAsset));
        }

        if (fontAsset.Characters == null || !fontAsset.Characters.TryGetValue(character, out FontChar glyph)) {
            Console.WriteLine("Glyph {0}=missing", character);
            return;
        }

        int glyphWidth = (int)Math.Round(glyph.SourceRect.Z * fontAsset.AtlasWidth);
        int glyphHeight = (int)Math.Round(glyph.SourceRect.W * fontAsset.AtlasHeight);
        Console.WriteLine("Glyph {0}={1}x{2} Advance={3} BearingX={4} BearingY={5}", character, glyphWidth, glyphHeight, glyph.AdvanceWidth, glyph.BearingX, glyph.BearingY);
    }

    /// <summary>
    /// Prints one glyph's cooked indexed pixel occupancy and palette-alpha breakdown.
    /// </summary>
    /// <param name="textureAsset">Cooked DS atlas texture asset.</param>
    /// <param name="fontAsset">Font asset that owns the glyph metadata.</param>
    /// <param name="character">Character whose cooked atlas pixels should be summarized.</param>
    static void PrintGlyph(TextureAsset textureAsset, FontAsset fontAsset, char character) {
        if (textureAsset == null) {
            throw new ArgumentNullException(nameof(textureAsset));
        } else if (fontAsset == null) {
            throw new ArgumentNullException(nameof(fontAsset));
        }

        if (fontAsset.Characters == null || !fontAsset.Characters.TryGetValue(character, out FontChar glyph)) {
            Console.WriteLine("GlyphPixels {0}=missing", character);
            return;
        }

        int sourceX = (int)Math.Round(glyph.SourceRect.X * fontAsset.AtlasWidth);
        int sourceY = (int)Math.Round(glyph.SourceRect.Y * fontAsset.AtlasHeight);
        int sourceWidth = (int)Math.Round(glyph.SourceRect.Z * fontAsset.AtlasWidth);
        int sourceHeight = (int)Math.Round(glyph.SourceRect.W * fontAsset.AtlasHeight);
        if (sourceWidth < 1 || sourceHeight < 1) {
            Console.WriteLine("GlyphPixels {0}=emptyRect", character);
            return;
        }
        Console.WriteLine(
            "GlyphRect {0}=X:{1} Y:{2} W:{3} H:{4} Right:{5} Bottom:{6}",
            character,
            sourceX,
            sourceY,
            sourceWidth,
            sourceHeight,
            sourceX + sourceWidth,
            sourceY + sourceHeight);

        int opaquePixelCount = 0;
        int transparentPixelCount = 0;
        HashSet<int> opaquePaletteIndices = new HashSet<int>();
        HashSet<int> transparentPaletteIndices = new HashSet<int>();
        for (int y = 0; y < sourceHeight; y++) {
            for (int x = 0; x < sourceWidth; x++) {
                int pixelIndex = ((sourceY + y) * textureAsset.Width) + (sourceX + x);
                int paletteIndex;
                int alpha;
                if (textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4) {
                    int packedByte = textureAsset.Colors[pixelIndex / 2];
                    paletteIndex = (pixelIndex & 1) == 0
                        ? packedByte & 0x0F
                        : (packedByte >> 4) & 0x0F;
                    int paletteOffset = paletteIndex * 4;
                    alpha = textureAsset.PaletteColors[paletteOffset + 3];
                } else {
                    paletteIndex = textureAsset.Colors[(pixelIndex * 4) + 0] > 0 ? 1 : 0;
                    alpha = textureAsset.Colors[(pixelIndex * 4) + 3];
                }

                if (alpha > 0) {
                    opaquePixelCount++;
                    opaquePaletteIndices.Add(paletteIndex);
                } else {
                    transparentPixelCount++;
                    transparentPaletteIndices.Add(paletteIndex);
                }
            }
        }

        string opaqueIndices = opaquePaletteIndices.Count == 0
            ? "<none>"
            : string.Join(",", opaquePaletteIndices.OrderBy(value => value));
        string transparentIndices = transparentPaletteIndices.Count == 0
            ? "<none>"
            : string.Join(",", transparentPaletteIndices.OrderBy(value => value));
        Console.WriteLine(
            "GlyphPixels {0}=Opaque:{1} Transparent:{2} OpaquePalette:{3} TransparentPalette:{4}",
            character,
            opaquePixelCount,
            transparentPixelCount,
            opaqueIndices,
            transparentIndices);
    }

    /// <summary>
    /// Prints one glyph's raw atlas pixels row by row before the DS runtime upload logic repacks it into an 8x8 tile.
    /// </summary>
    /// <param name="textureAsset">Atlas texture asset that stores the glyph pixels.</param>
    /// <param name="fontAsset">Font asset that owns the glyph metadata.</param>
    /// <param name="character">Character whose atlas pixels should be printed.</param>
    static void DumpGlyphPixels(TextureAsset textureAsset, FontAsset fontAsset, char character) {
        if (textureAsset == null) {
            throw new ArgumentNullException(nameof(textureAsset));
        } else if (fontAsset == null) {
            throw new ArgumentNullException(nameof(fontAsset));
        }

        if (fontAsset.Characters == null || !fontAsset.Characters.TryGetValue(character, out FontChar glyph)) {
            Console.WriteLine("GlyphRows {0}=missing", character);
            return;
        }

        int sourceX = (int)Math.Round(glyph.SourceRect.X * fontAsset.AtlasWidth);
        int sourceY = (int)Math.Round(glyph.SourceRect.Y * fontAsset.AtlasHeight);
        int sourceWidth = (int)Math.Round(glyph.SourceRect.Z * fontAsset.AtlasWidth);
        int sourceHeight = (int)Math.Round(glyph.SourceRect.W * fontAsset.AtlasHeight);
        Console.WriteLine("GlyphRows {0} Size={1}x{2}", character, sourceWidth, sourceHeight);
        for (int y = 0; y < sourceHeight; y++) {
            char[] row = new char[sourceWidth];
            for (int x = 0; x < sourceWidth; x++) {
                int sourcePixelIndex = ((sourceY + y) * textureAsset.Width) + (sourceX + x);
                byte alpha;
                if (textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4) {
                    byte paletteIndex = (byte)(textureAsset.Colors[sourcePixelIndex / 2] & 15);
                    if ((sourcePixelIndex & 1) != 0) {
                        paletteIndex = (byte)((textureAsset.Colors[sourcePixelIndex / 2] >> 4) & 15);
                    }

                    int paletteOffset = paletteIndex * 4;
                    alpha = textureAsset.PaletteColors[paletteOffset + 3];
                } else {
                    alpha = textureAsset.Colors[(sourcePixelIndex * 4) + 3];
                }

                row[x] = alpha > 0 ? '#' : '.';
            }

            Console.WriteLine("GlyphRows {0} Row{1}={2}", character, y, new string(row));
        }
    }

    /// <summary>
    /// Simulates the DS runtime glyph upload for one character and prints the resulting 8x8 tile payload.
    /// </summary>
    /// <param name="textureAsset">Cooked DS atlas texture asset.</param>
    /// <param name="fontAsset">Font asset that owns the glyph metadata.</param>
    /// <param name="character">Character whose runtime tile should be synthesized.</param>
    static void DumpRuntimeTile(TextureAsset textureAsset, FontAsset fontAsset, char character) {
        if (textureAsset == null) {
            throw new ArgumentNullException(nameof(textureAsset));
        } else if (fontAsset == null) {
            throw new ArgumentNullException(nameof(fontAsset));
        }

        if (fontAsset.Characters == null || !fontAsset.Characters.TryGetValue(character, out FontChar glyph)) {
            Console.WriteLine("RuntimeTile {0}=missing", character);
            return;
        }

        int sourceX = (int)Math.Round(glyph.SourceRect.X * fontAsset.AtlasWidth);
        int sourceY = (int)Math.Round(glyph.SourceRect.Y * fontAsset.AtlasHeight);
        int sourceWidth = (int)Math.Round(glyph.SourceRect.Z * fontAsset.AtlasWidth);
        int sourceHeight = (int)Math.Round(glyph.SourceRect.W * fontAsset.AtlasHeight);
        int availablePaletteEntries = Math.Min(textureAsset.PaletteColors?.Length / 4 ?? 0, 16);
        byte[] paletteIndexRemap = new byte[16];
        if (textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4) {
            for (int paletteIndex = 0; paletteIndex < availablePaletteEntries; paletteIndex++) {
                int paletteOffset = paletteIndex * 4;
                byte alpha = textureAsset.PaletteColors[paletteOffset + 3];
                paletteIndexRemap[paletteIndex] = alpha > 0 ? (byte)1 : (byte)0;
            }
        }

        byte[] tilePixels = new byte[32];
        int tileCopyWidth = Math.Min(sourceWidth, 8);
        int tileCopyHeight = Math.Min(sourceHeight, 8);
        for (int y = 0; y < tileCopyHeight; y++) {
            for (int x = 0; x < tileCopyWidth; x++) {
                int sourcePixelIndex = ((sourceY + y) * textureAsset.Width) + (sourceX + x);
                byte paletteIndex;
                if (textureAsset.ColorFormat == TextureAssetColorFormat.Indexed4) {
                    paletteIndex = (byte)(textureAsset.Colors[sourcePixelIndex / 2] & 15);
                    if ((sourcePixelIndex & 1) != 0) {
                        paletteIndex = (byte)((textureAsset.Colors[sourcePixelIndex / 2] >> 4) & 15);
                    }

                    paletteIndex = paletteIndex >= availablePaletteEntries
                        ? (byte)0
                        : paletteIndexRemap[paletteIndex];
                } else {
                    byte alpha = textureAsset.Colors[(sourcePixelIndex * 4) + 3];
                    paletteIndex = alpha > 0 ? (byte)1 : (byte)0;
                }

                int tileByteIndex = (y * 4) + (x / 2);
                if ((x & 1) == 0) {
                    tilePixels[tileByteIndex] = (byte)((tilePixels[tileByteIndex] & 0xF0) | (paletteIndex & 0x0F));
                } else {
                    tilePixels[tileByteIndex] = (byte)((tilePixels[tileByteIndex] & 0x0F) | ((paletteIndex & 0x0F) << 4));
                }
            }
        }

        Console.WriteLine("RuntimeTile {0} Bytes={1}", character, string.Join(",", tilePixels.Select(value => value.ToString("X2"))));
        for (int y = 0; y < 8; y++) {
            char[] row = new char[8];
            for (int x = 0; x < 8; x++) {
                int tileByteIndex = (y * 4) + (x / 2);
                int paletteIndex = (x & 1) == 0
                    ? tilePixels[tileByteIndex] & 0x0F
                    : (tilePixels[tileByteIndex] >> 4) & 0x0F;
                row[x] = paletteIndex == 0 ? '.' : '#';
            }

            Console.WriteLine("RuntimeTile {0} Row{1}={2}", character, y, new string(row));
        }
    }
}
