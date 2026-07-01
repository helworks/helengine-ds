using System.Drawing;
using System.Text;
using System.Runtime.Versioning;

namespace helengine.ds.builder;

/// <summary>
/// Decodes source image files into raw RGBA texture assets for Nintendo DS builder-owned texture cooking.
/// </summary>
[SupportedOSPlatform("windows")]
public sealed class NintendoDsSourceTextureDecoder {
    /// <summary>
    /// Stores the serialized asset magic header used by helengine asset payloads.
    /// </summary>
    static readonly byte[] AssetMagicHeader = Encoding.ASCII.GetBytes("HELE");

    /// <summary>
    /// Decodes one source image file into one uncooked raw texture asset.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute source image path.</param>
    /// <returns>Raw texture asset containing RGBA pixel bytes.</returns>
    public TextureAsset Decode(string sourceAssetPath) {
        if (string.IsNullOrWhiteSpace(sourceAssetPath)) {
            throw new ArgumentException("Source asset path is required.", nameof(sourceAssetPath));
        } else if (!File.Exists(sourceAssetPath)) {
            throw new FileNotFoundException("Source texture file was not found.", sourceAssetPath);
        }
        if (TryDecodeSerializedTextureAsset(sourceAssetPath, out TextureAsset serializedTextureAsset)) {
            return serializedTextureAsset;
        }

        using Bitmap bitmap = CreateBitmap(sourceAssetPath);
        if (bitmap.Width < 1 || bitmap.Height < 1) {
            throw new InvalidOperationException("Source textures must have positive dimensions.");
        }

        byte[] rgbaBytes = new byte[bitmap.Width * bitmap.Height * 4];
        int writeOffset = 0;
        for (int y = 0; y < bitmap.Height; y++) {
            for (int x = 0; x < bitmap.Width; x++) {
                Color pixel = bitmap.GetPixel(x, y);
                rgbaBytes[writeOffset++] = pixel.R;
                rgbaBytes[writeOffset++] = pixel.G;
                rgbaBytes[writeOffset++] = pixel.B;
                rgbaBytes[writeOffset++] = pixel.A;
            }
        }

        return new TextureAsset {
            Width = checked((ushort)bitmap.Width),
            Height = checked((ushort)bitmap.Height),
            Colors = rgbaBytes,
            PaletteColors = Array.Empty<byte>(),
            ColorFormat = TextureAssetColorFormat.Rgba32,
            AlphaPrecision = TextureAssetAlphaPrecision.A8
        };
    }

    /// <summary>
    /// Attempts to decode one serialized helengine texture asset directly from the source path.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute source asset path.</param>
    /// <param name="textureAsset">Decoded texture asset when the source file contains one serialized texture payload.</param>
    /// <returns>True when the source file contains one serialized texture asset payload; otherwise false.</returns>
    static bool TryDecodeSerializedTextureAsset(string sourceAssetPath, out TextureAsset textureAsset) {
        if (string.IsNullOrWhiteSpace(sourceAssetPath)) {
            throw new ArgumentException("Source asset path is required.", nameof(sourceAssetPath));
        }

        textureAsset = null;
        using FileStream stream = new(sourceAssetPath, FileMode.Open, FileAccess.Read, FileShare.Read);
        if (!HasSerializedAssetMagicHeader(stream)) {
            return false;
        }

        stream.Position = 0;
        textureAsset = global::helengine.files.AssetSerializer.Deserialize(stream) as TextureAsset
            ?? throw new InvalidOperationException($"Serialized texture source '{sourceAssetPath}' did not contain a TextureAsset payload.");
        return true;
    }

    /// <summary>
    /// Determines whether one source stream begins with the helengine serialized asset magic header.
    /// </summary>
    /// <param name="stream">Readable source stream to inspect.</param>
    /// <returns>True when the stream begins with the serialized asset magic header; otherwise false.</returns>
    static bool HasSerializedAssetMagicHeader(Stream stream) {
        if (stream == null) {
            throw new ArgumentNullException(nameof(stream));
        } else if (!stream.CanRead) {
            throw new InvalidOperationException("Source stream must be readable.");
        } else if (!stream.CanSeek) {
            throw new InvalidOperationException("Source stream must be seekable.");
        }

        if (stream.Length < AssetMagicHeader.Length) {
            return false;
        }

        long originalPosition = stream.Position;
        byte[] headerBuffer = new byte[AssetMagicHeader.Length];
        int bytesRead = stream.Read(headerBuffer, 0, headerBuffer.Length);
        stream.Position = originalPosition;
        if (bytesRead != headerBuffer.Length) {
            return false;
        }

        for (int index = 0; index < AssetMagicHeader.Length; index++) {
            if (headerBuffer[index] != AssetMagicHeader[index]) {
                return false;
            }
        }

        return true;
    }

    /// <summary>
    /// Opens one bitmap source file and rewrites the thrown exception to include the failing path.
    /// </summary>
    /// <param name="sourceAssetPath">Absolute bitmap source path.</param>
    /// <returns>Decoded bitmap instance.</returns>
    static Bitmap CreateBitmap(string sourceAssetPath) {
        if (string.IsNullOrWhiteSpace(sourceAssetPath)) {
            throw new ArgumentException("Source asset path is required.", nameof(sourceAssetPath));
        }

        try {
            return new Bitmap(sourceAssetPath);
        } catch (ArgumentException exception) {
            throw new InvalidOperationException($"Nintendo DS could not decode bitmap source '{sourceAssetPath}'.", exception);
        }
    }
}
