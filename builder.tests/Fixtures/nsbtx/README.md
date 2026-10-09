# Independent NSBTX fixtures

These files use original, synthetic 8-by-8 pixel/index/palette patterns. They were packed with [ndspy](https://github.com/RoadrunnerWMC/ndspy) at commit `00165f177d03c6ca9e15e37decf54d9c6754bf54`.

There is one archive for each of the seven DS 3D texture formats. `multi.nsbtx` combines all seven textures and six named palettes. The compressed texture contains one block for each of the four native palette/interpolation modes. Alpha/index patterns exercise native coverage and color-zero transparency.

Each `.rgba` file contains 64 row-major RGBA pixels, expanded from ndspy's independent five-bit native output by `(value << 3) | (value >> 2)`. The importer tests compare every channel, including RGB under transparent alpha.

The fixtures are self-contained during ordinary test runs. To regenerate them, install the pinned ndspy revision and run `generate-fixtures.py` from any working directory. It writes into its own fixture directory. The generator passes raw RGB555 words to ndspy's `renderTextureData` API, avoiding the upstream `Texture.render` palette tuple mismatch at that revision.
