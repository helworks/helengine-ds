"""Recreate original synthetic native DS archives with an independent ndspy implementation."""
from pathlib import Path
import struct

from ndspy import color
from ndspy.texture import (
    NSBTX, Palette, Texture, TextureCoordinatesTransformationMode,
    TextureFormat, renderTextureData,
)

color.prepareLUTs()
destination = Path(__file__).resolve().parent
textures = []
palettes = []

for code, name, capacity in [
    (1, "a3i5", 32), (2, "indexed2", 4), (3, "indexed4", 16),
    (4, "indexed8", 256), (5, "compressed4x4", 16),
    (6, "a5i3", 8), (7, "bgr5551", 0),
]:
    words = [(i * 7 % 32) | (i * 13 % 32) << 5 | (i * 19 % 32) << 10 for i in range(capacity)]
    selectors, descriptors = bytearray(), bytearray()
    if code == 5:
        for block in range(4):
            selectors.extend(bytes([0xe4, 0x39, 0x4e, 0x93]))
            descriptors.extend(struct.pack("<H", block * 2 | block << 14))
    elif code in (1, 6):
        for y in range(8):
            for x in range(8):
                index = (x + y * 3) % capacity
                alpha = (x + y) % (8 if code == 1 else 32)
                selectors.append(index | alpha << (5 if code == 1 else 3))
    elif code in (2, 3, 4):
        bits = {2: 2, 3: 4, 4: 8}[code]
        for pixel in range(0, 64, 8 // bits):
            value = sum(((pixel + p) % capacity) << (p * bits) for p in range(8 // bits))
            selectors.append(value)
    else:
        for y in range(8):
            for x in range(8):
                word = x * 4 | y * 4 << 5 | ((x + y) * 2 % 32) << 10 | (0x8000 if (x + y) % 3 else 0)
                selectors.extend(struct.pack("<H", word))

    texture = Texture.fromFlags(
        0, 0, False, False, False, False, 8, 8, TextureFormat(code),
        code in (2, 3, 4), TextureCoordinatesTransformationMode.NONE,
        0, bytes(selectors), bytes(descriptors),
    )
    palette = Palette(0, 0, 0, struct.pack("<" + "H" * capacity, *words)) if capacity else None
    archive = NSBTX.fromTexturesAndPalettes(
        0, 0, 0, 0, 0, 0, [(name, texture)], [(name, palette)] if palette else [],
    )
    (destination / (name + ".nsbtx")).write_bytes(archive.save())
    rendered = renderTextureData(
        texture.data1, texture.data2, texture.format, texture.width, texture.height,
        words if palette else None, texture.isColor0Transparent,
    )
    (destination / (name + ".rgba")).write_bytes(bytes(v << 3 | v >> 2 for pixel in rendered for v in pixel))
    textures.append((name, texture))
    if palette:
        palettes.append((name, palette))

combined = NSBTX.fromTexturesAndPalettes(0, 0, 0, 0, 0, 0, textures, palettes)
(destination / "multi.nsbtx").write_bytes(combined.save())
