#ifdef DrawText
#undef DrawText
#endif
#include "TextLayoutAlignmentUtils.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "FontChar.hpp"
#include "TextAlignment.hpp"
#include "FontAsset.hpp"
#include "float4.hpp"
#include "FontInfo.hpp"
#include "system/math.hpp"
#include "runtime/native_dictionary.hpp"
#include "system/math.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

double TextLayoutAlignmentUtils::MeasureVisibleLineWidth(std::string line, ::FontAsset* font, double fontScale, double textureWidth)
{
    if (String::IsNullOrEmpty(line))
    {
return 0.0;    }
else {
    if (font == nullptr)
    {
throw new ArgumentNullException("font");
    }
else {
    if (fontScale <= 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000180 = "fontScale";
auto __ctor_arg_00000181 = "Font scale must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_00000180, __ctor_arg_00000181);
})();
    }
else {
    if (textureWidth <= 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000182 = "textureWidth";
auto __ctor_arg_00000183 = "Texture width must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_00000182, __ctor_arg_00000183);
})();
    }
}
}
}
double visibleWidth = 0.0;
double offsetX = 0.0;
const double spaceWidth = font->FontInfo != nullptr ? Math::Max(font->FontInfo->SpaceWidth * fontScale, 1.0) : Math::Max(font->LineHeight * fontScale * 0.25, 1.0);
for (int32_t index = 0; index < static_cast<int32_t>(line.size()); index++) {
const char character = line[index];
    if (character == ' ')
    {
offsetX += spaceWidth;
visibleWidth = Math::Max(visibleWidth, offsetX);
continue;
    }
::FontChar glyph;
    if (!font->Characters->TryGetValue(static_cast<char>(character), glyph))
    {
continue;
    }
const double glyphWidth = Math::Max(1.0, glyph.SourceRect.Z * textureWidth * fontScale);
visibleWidth = Math::Max(visibleWidth, offsetX + glyphWidth);
offsetX += glyph.AdvanceWidth > 0.0f ? glyph.AdvanceWidth * fontScale : glyphWidth;
}
return visibleWidth;}

double TextLayoutAlignmentUtils::ResolveHorizontalOffset(::TextAlignment alignment, int32_t boxWidth, double visibleWidth)
{
    if (boxWidth <= 0 || visibleWidth <= 0.0)
    {
return 0.0;    }
const double availableWidth = boxWidth - visibleWidth;
    if (availableWidth <= 0.0)
    {
return 0.0;    }
    if (alignment == TextAlignment::Center)
    {
return availableWidth * 0.5;    }
else {
    if (alignment == TextAlignment::Right)
    {
return availableWidth;    }
}
return 0.0;}

