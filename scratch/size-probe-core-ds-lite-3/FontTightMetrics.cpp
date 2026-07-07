#ifdef DrawText
#undef DrawText
#endif
#include "FontTightMetrics.hpp"
#include "FontTightMetrics.hpp"
#include "system/math.hpp"

FontTightMetrics::FontTightMetrics() : Width(), MinTop(), MaxBottom()
{
}

float FontTightMetrics::get_Height()
{
return Math::Max(1.0f, this->MaxBottom - this->MinTop);
}

FontTightMetrics::FontTightMetrics(float width, float minTop, float maxBottom) : Width(), MinTop(), MaxBottom()
{
this->Width = width;
this->MinTop = minTop;
this->MaxBottom = maxBottom;
}

