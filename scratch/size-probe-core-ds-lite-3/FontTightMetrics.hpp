#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class FontTightMetrics
{
public:
    FontTightMetrics();

    float Width;

    float MinTop;

    float MaxBottom;

    float get_Height();

    FontTightMetrics(float width, float minTop, float maxBottom);
};
