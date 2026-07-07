#ifdef DrawText
#undef DrawText
#endif
#include "TextLayoutUtils.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/text/string-builder.hpp"
#include "FontChar.hpp"
#include "TextLayoutUtils.hpp"
#include "FontAsset.hpp"
#include "float4.hpp"
#include "FontInfo.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_string.hpp"
#include "system/text/string-builder.hpp"

std::string TextLayoutUtils::WrapText(std::string text, ::FontAsset* font, int32_t maxWidth)
{
    if (font == nullptr)
    {
throw new ArgumentNullException("font");
    }
    if (String::IsNullOrEmpty(text) || maxWidth <= 0)
    {
return text;    }
StringBuilder *wrappedText = new StringBuilder(static_cast<int32_t>(static_cast<int32_t>(text.size()) + 16));
auto __localDeleteGuard_00000184 = he_cpp_make_scope_exit([&]() {
delete wrappedText;
});
int32_t lineStartIndex = 0;
int32_t lastWrapIndex = -1;
double lineWidth = 0.0;
double widthAtLastWrap = 0.0;
int32_t wrappedLength = 0;
for (int32_t index = 0; index < static_cast<int32_t>(text.size()); index++) {
const char character = text[index];
    if (character == '\r')
    {
continue;
    }
    if (character == '\n')
    {
wrappedText->Append(static_cast<char>('\n'));
wrappedLength++;
lineStartIndex = wrappedLength;
lastWrapIndex = -1;
widthAtLastWrap = 0.0;
lineWidth = 0.0;
continue;
    }
const double characterWidth = TextLayoutUtils::GetCharacterAdvanceWidth(font, static_cast<char>(character));
    if (lineWidth > 0.0 && lineWidth + characterWidth > maxWidth)
    {
    if (lastWrapIndex >= lineStartIndex)
    {
const int32_t suffixStartIndex = lastWrapIndex + 1;
const std::string currentText = wrappedText->ToString();
const std::string suffix = String::Substring(currentText, suffixStartIndex);
auto __reassignValue_00000185 = new StringBuilder(String::Substring(currentText, 0, lastWrapIndex));
delete wrappedText;
wrappedText = __reassignValue_00000185;
wrappedText->Append(static_cast<char>('\n'));
wrappedText->Append(suffix);
wrappedLength = lastWrapIndex + 1 + static_cast<int32_t>(suffix.size());
lineStartIndex = wrappedLength - static_cast<int32_t>(suffix.size());
lineWidth -= widthAtLastWrap;
    }
else {
wrappedText->Append(static_cast<char>('\n'));
wrappedLength++;
lineStartIndex = wrappedLength;
lineWidth = 0.0;
}
lastWrapIndex = -1;
widthAtLastWrap = 0.0;
    if (character == ' ')
    {
continue;
    }
index--;
continue;
    }
wrappedText->Append(static_cast<char>(character));
wrappedLength++;
lineWidth += characterWidth;
    if (character == ' ')
    {
lastWrapIndex = wrappedLength - 1;
widthAtLastWrap = lineWidth;
    }
}
return wrappedText->ToString();}

double TextLayoutUtils::GetCharacterAdvanceWidth(::FontAsset* font, char character)
{
    if (character == ' ')
    {
return font->FontInfo->SpaceWidth;    }
::FontChar glyph;
    if (!font->Characters->TryGetValue(static_cast<char>(character), glyph))
    {
return 0.0;    }
const double pixelWidth = glyph.SourceRect.Z * font->AtlasWidth;
    if (glyph.AdvanceWidth > 0.0f)
    {
return glyph.AdvanceWidth;    }
return pixelWidth;}

