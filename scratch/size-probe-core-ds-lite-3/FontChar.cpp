#ifdef DrawText
#undef DrawText
#endif
#include "FontChar.hpp"
#include "float4.hpp"
#include "FontChar.hpp"

FontChar::FontChar() : SourceRect(), OffsetY(), AdvanceWidth(), BearingX(), BearingY()
{
}

FontChar::FontChar(::float4 sourceRect, float offsetY, float advanceWidth, float bearingX, float bearingY) : SourceRect(), OffsetY(), AdvanceWidth(), BearingX(), BearingY()
{
this->SourceRect = sourceRect;
this->OffsetY = offsetY;
this->AdvanceWidth = advanceWidth;
this->BearingX = bearingX;
this->BearingY = bearingY;
}

