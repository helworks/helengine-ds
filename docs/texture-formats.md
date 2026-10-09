# Nintendo DS texture formats

The DS platform publishes all seven 3D hardware texture formats and the shared 4-bit/8-bit OBJ/BG tile layouts. Image and font-atlas cook capabilities expose these settings alongside the existing `Rgba4444`, `Indexed4` and `Indexed8` defaults.

| Cook format ID | Native storage | Palette | Intrinsic alpha |
| --- | --- | --- | --- |
| `NintendoDs.A3I5` | 8-bit linear texels: A3 + I5 | 32 RGB555 entries | `A3` |
| `NintendoDs.Indexed2` | 2-bit linear indices | 4 RGB555 entries | `Binary` |
| `NintendoDs.Indexed4` | 4-bit linear indices | 16 RGB555 entries | `Binary` |
| `NintendoDs.Indexed8` | 8-bit linear indices | 256 RGB555 entries | `Binary` |
| `NintendoDs.Compressed4x4` | 2-bit block selectors + 16-bit descriptor per block | Variable RGB555 palette | `Binary` |
| `NintendoDs.A5I3` | 8-bit linear texels: A5 + I3 | 8 RGB555 entries | `A5` |
| `NintendoDs.Bgr5551` | 16-bit red-low RGB555 + high-bit alpha | None | `Binary` |
| `NintendoDs.Tiled.Indexed4` | 4-bit indices in 8-by-8 tiles | 16 RGB555 entries | `Binary` |
| `NintendoDs.Tiled.Indexed8` | 8-bit indices in 8-by-8 tiles | 256 RGB555 entries | `Binary` |

Every native ID also supports `Opaque`. Other format/alpha combinations are rejected. Binary cooking thresholds source alpha at 128. Indexed binary cooking reserves palette entry zero for transparency. A3 expands into native five-bit alpha before the RGBA preview expands to eight bits; its preview levels are 0, 33, 74, 107, 148, 181, 222 and 255. A5 preserves all 32 native alpha levels.

## Storage and cooking

Logical dimensions are 1..1024 on each axis. Native storage rounds each axis independently to a power of two, with a minimum of eight pixels. Padding repeats the last logical row/column. `MaxResolution` accepts zero (no resize) or 1..1024; resizing retains the platform's existing center-sampled nearest-neighbor rule.

RGB555 palettes and direct words store red in the low five bits. Linear indices use the least-significant bits/nibble first. The tiled formats use row-major 8-by-8 tiles and row-major pixels within each tile. Compressed textures use row-major 4-by-4 blocks, four selector bytes per block, and a separate contiguous descriptor plane. Each descriptor stores a palette base in units of two colors and one of four palette/interpolation modes.

The decoder supports all four compressed modes, including truncated native five-bit averaging and 5:3/3:5 interpolation. The encoder uses deterministic local palettes with three explicit colors plus transparency, or four explicit colors. It reuses equal palette groups. It does not optimize blocks into interpolated modes; compression and palette quantization are lossy. Palette exhaustion produces an error.

Texture validation rejects compressed selector planes larger than one 128 KiB slot, ordinary 3D texels exceeding the hardware's total 512 KiB texture VRAM, invalid descriptors, inconsistent dimensions, unsupported alpha, and palette references outside the stored palette. Compressed palettes contain at most 32768 RGB555 entries. These bounds do not guarantee that an asset fits the platform's current VRAM mapping.

The editor estimates padded texels, compressed descriptors and palette bytes. Compressed palette estimates use the maximum possible size for that image, capped at 64 KiB. The estimate excludes the serialized header and CPU-side decoded pixels.

## NSBTX import

The portable importer accepts raw, uncompressed `.nsbtx` archives with a little-endian `BTX0` header and one `TEX0` section. It validates file/section/dictionary/data bounds and limits buffered containers to 16 MiB. It accepts readable non-seekable streams.

`NintendoDsNsbtxTextureImporter.ImportTexture(stream)` imports the first texture as RGBA for ordinary editor previews. Its indexed overload selects a texture and palette explicitly. Automatic palette selection matches the texture name, then accepts a sole palette; multiple unmatched palettes require an explicit selection.

`ImportNativeTexture(stream, textureIndex, paletteIndex)` returns independently owned native storage and preserves source compressed blocks without recompression. The ordinary DS source-image/font-atlas build path decodes the selected first texture and recooks it using the requested output settings. It does not automatically pass source compressed blocks through unchanged.

This importer does not extract textures embedded in `.nsbmd`, unpack LZ-wrapped archives, or combine separate `.ncgr`/`.nclr` files. The tiled formats can be cooked from ordinary imported images. There is no mip-chain or cubemap container support.

## Serialized native payload

The existing texture asset envelope uses `TextureAssetColorFormat.NintendoDsNative` (18). `Colors` begins with a 48-byte, little-endian `DST1` descriptor; the texel plane follows it, then the optional compressed descriptor plane. `PaletteColors` contains independent little-endian RGB555 words.

| Byte offset | Unsigned 32-bit field |
| --- | --- |
| 0 | Magic `0x31545344` (`DST1`) |
| 4 | Version 1 |
| 8 | Native format code 1..9, in table order |
| 12 / 16 | Logical width / height |
| 20 / 24 | Storage width / height |
| 28 | Texel byte count |
| 32 | Compressed descriptor byte count; zero for other formats |
| 36 | RGB555 palette entry count |
| 40 | Flags: bit 0 enables indexed color-zero transparency |
| 44 | Reserved; must be zero |

Color-zero transparency is valid only for the linear and tiled indexed formats. Packaged texture readers and both font readers recognize native DS storage and A3/A5 alpha without changing previously serialized enum values. Preview decoding is shared with other engine consumers.

## Runtime

The 3D uploader sends original native texels and RGB555 palettes to libnds. It untile-converts tiled indices to the corresponding linear indexed GPU format. Compressed selectors and their descriptor plane remain contiguous as required by `glTexImage2D`.

The current platform mapping provides 256 KiB of texture VRAM in banks A/B and 16 KiB of palette VRAM in bank G. Existing OBJ mappings in E/F remain in use. These capacities are shared between live textures, and compressed slot placement adds further constraints. Texture-name, texel and palette allocation failures are reported explicitly.

The runtime owns a separate copy of native storage and an RGBA view for existing OBJ/software drawing. This consumes additional main RAM; the editor's VRAM estimate does not include it. Native alpha is preserved in that view, while OBJ and text BG rendering retain their existing hardware coverage rules. Text BG glyph conversion accepts the native decoded view and reduces coverage to its existing binary mask.

Native textures reject region updates because the preserved GPU payload is immutable. Generic dynamic textures retain the existing update path.

## Validation and current build limitation

Validation on 2026-10-08 passed 50 targeted DS builder tests, six native font-reader cases and one host importer-factory test. The corpus includes independently authored NSBTX files and pixel expectations produced by ndspy, all seven 3D formats, all four compressed decode modes, both tile formats, native alpha levels, invalid metadata, image/font cooking, serialization and real NitroFS staging. A separate ndspy decode of actual cooked payloads also matched every RGBA channel for all nine formats (117 logical pixels per format).

The complete core C#-to-C++ generation passed. The six new generated codec units, native uploader and runtime texture unit compiled with devkitARM/libnds and warnings treated as errors. The 3D renderer also compiled with the platform's normal warning policy.

The complete player build remains unverified: the existing 2D renderer references removed `get_RenderOrder2D` methods and dictionary pair members `first`/`second` absent from the current generated runtime. Compiling its original `HEAD` source reproduced the same eight errors, independently of the texture changes. Logs and generated outputs are in `builds/helengine-ds/texture-formats-20261008` under the workspace root. No emulator or hardware run was performed.

Native layout/API references: [libnds videoGL implementation](https://github.com/devkitPro/libnds/blob/master/source/arm9/videoGL.c) and [ndspy texture implementation](https://github.com/RoadrunnerWMC/ndspy/blob/master/ndspy/texture.py).
